/*
 * Copyright (C) 2007 Ingenic Semiconductor Inc.
 * Author: Peter <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>

#include <asm/jzsoc.h>
#include <config.h>
/*
  BUS_WIDTH 0  --> 1 BIT 
  BUS_WIDTH 2  --> 4 BIT
*/

#define BUS_WIDTH          2 
/* change BOOT_FROM_P to 1 for alternative boot testing */
#define BOOT_FROM_P        0
/*
 * External routines
 */
extern void flush_cache_all(void);
extern int serial_init(void);
extern void serial_puts(const char *s);
extern void sdram_init(void);
#ifndef CONFIG_FPGA
extern void pll_init(void);
#endif

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
static int rca;
static int highcap = 0;

/*
 * GPIO definition
 */
#define MMC_IRQ_MASK()				\
do {						\
	REG_MSC_IMASK = 0xffff;			\
	REG_MSC_IREG = 0xffff;			\
} while (0)

static void wait_prg_done(void)
{
	while (!(REG_MSC_STAT & MSC_STAT_PRG_DONE))
		;
	REG_MSC_IREG |= MSC_IREG_PRG_DONE;	
}

static void wait_tran_done(void)
{
	while (!(REG_MSC_STAT & MSC_STAT_DATA_TRAN_DONE))
		;
	REG_MSC_IREG |= MSC_IREG_DATA_TRAN_DONE;	
}

static void mudelay(unsigned int usec)
{
    //unsigned int i = usec * (336000000 / 2000000);
	unsigned int i = usec  << 7;
    __asm__ __volatile__ (
        "\t.set noreorder\n"
        "1:\n\t"
        "bne\t%0, $0, 1b\n\t"
        "addi\t%0, %0, -1\n\t"
        ".set reorder\n"
        : "=r" (i)
        : "0" (i)
    );

}

static void sd_mdelay(int sdelay)
{
    mudelay(sdelay * 1000);	
}

/* Start the MMC clock and operation */
static  int jz_mmc_start_op(void)
{
	REG_MSC_STRPCL = MSC_STRPCL_START_OP;
	return 0;
}

static u8 * mmc_cmd(u16 cmd, unsigned int arg, unsigned int cmdat, u16 rtype)
{
	static u8 resp[20];
	u32 timeout = 0x3fffff;
	int words, i;

	REG_MSC_CMD   = cmd;
	REG_MSC_ARG   = arg;
	REG_MSC_CMDAT = cmdat;

	REG_MSC_IMASK = ~MSC_IMASK_END_CMD_RES;
	jz_mmc_start_op();

	while (timeout-- && !(REG_MSC_STAT & MSC_STAT_END_CMD_RES))
		;

	REG_MSC_IREG = MSC_IREG_END_CMD_RES;

	switch (rtype) {
	case MSC_CMDAT_RESPONSE_R1:
		case MSC_CMDAT_RESPONSE_R3:
			words = 3;
			break;
		case MSC_CMDAT_RESPONSE_R2:
			words = 8;
			break;
		default:
			return 0; 
	}

	for (i = words-1; i >= 0; i--) {
		u16 res_fifo = REG_MSC_RES;
		int offset = i << 1;
		
		resp[offset] = ((u8 *)&res_fifo)[0];
		resp[offset+1] = ((u8 *)&res_fifo)[1];
	}
	return resp;
}

int mmc_block_readm(u32 src, u32 num, u8 *dst)
{
	u8 *resp;
	u32 stat, timeout, data, cnt, wait, nob;

	resp = mmc_cmd(16, 0x200, 0x1, MSC_CMDAT_RESPONSE_R1);
	REG_MSC_BLKLEN = 0x200;
	REG_MSC_NOB = num / 512;

	if (BUS_WIDTH == 2){
		if (highcap) 
			resp = mmc_cmd(18, src, 0x409, MSC_CMDAT_RESPONSE_R1); // for sdhc card
		else
			resp = mmc_cmd(18, src * 512, 0x409, MSC_CMDAT_RESPONSE_R1);
	}else{
		if (highcap) 
			resp = mmc_cmd(18, src, 0x9, MSC_CMDAT_RESPONSE_R1); // for sdhc card
		else
			resp = mmc_cmd(18, src * 512, 0x9, MSC_CMDAT_RESPONSE_R1);

	}
	nob  = num / 512;

	for (; nob >= 1; nob--) {
		timeout = 0x7ffffff;
		while (timeout) {
			timeout--;
			stat = REG_MSC_STAT;
		
			if (stat & MSC_STAT_TIME_OUT_READ) {
				serial_puts("\n TIME_OUT_READ\n\n");
				return -1;
			}
			else if (stat & MSC_STAT_CRC_READ_ERROR) {
				serial_puts("\n CRC_READ_ERROR\n\n");
				return -1;
			}
			else if (!(stat & MSC_STAT_DATA_FIFO_EMPTY)) {
				/* Ready to read data */
				break;
			}

			wait = 120;
			while (wait--)
				;
		}
		if (!timeout) {
			serial_puts("read timeout\n");
			return -1;
		}

		/* Read data from RXFIFO. It could be FULL or PARTIAL FULL */
		cnt = 128;
		while (cnt) {
			while (cnt && (REG_MSC_STAT & MSC_STAT_DATA_FIFO_EMPTY))
				;
			cnt --;
			data = REG_MSC_RXFIFO;
			{
				*dst++ = (u8)(data >> 0);
				*dst++ = (u8)(data >> 8);
				*dst++ = (u8)(data >> 16);
				*dst++ = (u8)(data >> 24);
			}
		}
	}

	resp = mmc_cmd(12, 0, 0x41, MSC_CMDAT_RESPONSE_R1);
	wait_tran_done();	

	return 0;
}

static void config_boot_partition(void)
{
	serial_puts("config boot partition\n");
	if(BUS_WIDTH == 2)
		mmc_cmd(6, 0x3b34901/* 0x3b30901 */, 0x441, MSC_CMDAT_RESPONSE_R1);   /* set boot from partition 1 with ACK*/
	else
		mmc_cmd(6, 0x3b34901/* 0x3b30901 */, 0x41, MSC_CMDAT_RESPONSE_R1);   /* set boot from partition 1 with ACK*/		
	wait_prg_done();	
	
	if(BUS_WIDTH == 2)
		mmc_cmd(6, 0x3b10101, 0x441, MSC_CMDAT_RESPONSE_R1);   /* set boot bus width -> 4 bit */
	else
		mmc_cmd(6, 0x3b10001, 0x41, MSC_CMDAT_RESPONSE_R1);   /* set boot bus width -> 1 bit */
	wait_prg_done();	
}

static void sd_found(void)
{

	int retries;
	u8 *resp;
	unsigned int cardaddr;
//	serial_puts("SD card found!\n");
#if 0
	resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	retries = 100;
	while (retries-- && resp && !(resp[4] & 0x80)) {
		resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
		sd_mdelay(10);
	}

	if (resp[4] & 0x80) 
		;//serial_puts("init ok\n");
	else 
		;//serial_puts("init fail\n");

	/* try to get card id */
	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0, 0x6, MSC_CMDAT_RESPONSE_R1);
	cardaddr = (resp[4] << 8) | resp[3]; 
	rca = cardaddr << 16;

	resp = mmc_cmd(9, rca, 0x2, MSC_CMDAT_RESPONSE_R2);
	highcap = (resp[14] & 0xc0) >> 6;
#ifndef CONFIG_FPGA
	REG_MSC_CLKRT = 2;
#else
	REG_MSC_CLKRT = 4;
#endif
	resp = mmc_cmd(7, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(6, BUS_WIDTH, 0x1 | (BUS_WIDTH << 9), MSC_CMDAT_RESPONSE_R1);
#endif
}

/* init mmc/sd card we assume that the card is in the slot */
int  mmc_found(void)
{

	int retries;
	u8 *resp;

#if 1
	resp = mmc_cmd(1, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	retries = 1000;
	while (retries-- && resp && !(resp[4] & 0x80)) {
		resp = mmc_cmd(1, 0x40300000, 0x3, MSC_CMDAT_RESPONSE_R3);
		sd_mdelay(10);
	}
		
	sd_mdelay(10);

	if ((resp[4] & 0x80 )== 0x80) 
		serial_puts("MMC init ok\n");
	else 
		serial_puts("MMC init fail\n");
	
	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;
	/* try to get card id */
	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);

#ifndef CONFIG_FPGA
	REG_MSC_CLKRT = 2;	/* 16/1 MHz */
#else
	REG_MSC_CLKRT = 4;	/* 16/1 MHz */
#endif
	resp = mmc_cmd(7, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);
	if(BUS_WIDTH == 2)
		resp = mmc_cmd(6, 0x3b70101, 0x441, MSC_CMDAT_RESPONSE_R1); /* 4 bit bus width */
	else
		resp = mmc_cmd(6, 0x3b70001, 0x41, MSC_CMDAT_RESPONSE_R1);  /* 1 bit bus width */
	wait_prg_done();	

	if(BOOT_FROM_P)
		config_boot_partition();

#endif
	return 0;
}

int  mmc_init(void)
{
	u8 *resp;

	__gpio_a_as_msc0_4bit();
	/* __gpio_e_as_msc0_4bit(); */
	__msc_reset();

	MMC_IRQ_MASK();	
	REG_MSC_CLKRT = 7;    //187k
	REG_MSC_RDTO = 0xffffffff;
	REG_MSC_LPM = 0x1;

	/* just for reading and writing, suddenly it was reset, and the power of sd card was not broken off */
	resp = mmc_cmd(12, 0, 0x41, MSC_CMDAT_RESPONSE_R1);

	/* reset */
	resp = mmc_cmd(0, 0, 0x80, 0);
	resp = mmc_cmd(8, 0x1aa, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);

	if(resp[5] == 0x37){
		resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
		if(resp[5] == 0x3f){
			sd_found();
		}else
			mmc_found();
	}else
		mmc_found();

	return 0;
}

static void gpio_init(void)
{
//#ifdef CONFIG_DISABLE_CONSOLE_YJT	// disable console, yjt, 20130813
	// do nothing
//#else	
	switch (CFG_UART_BASE) {
		case UART0_BASE:
			__gpio_as_uart0();
			__cpm_start_uart0();
			break;
		case UART1_BASE:
			__gpio_as_uart1();
			__cpm_start_uart1();
			break;
		case UART2_BASE:
			__gpio_as_uart2();
			__cpm_start_uart2();
			break;
		case UART3_BASE:
			__gpio_as_uart3();
			__cpm_start_uart3();
			break;
	}
//#endif
}

/*
 * Load kernel image from MMC/SD into RAM
 */
static int mmc_load(int uboot_size, u8 *dst)
{
	mmc_init();
	mmc_block_readm(56, uboot_size, dst);

	return 0;
}

void spl_boot(void)
{
	int msc_cdr;
	void (*uboot)(void);

	/*
	 * Init hardware
	 */
	gpio_init();
	serial_init();
	serial_puts("\n\nMSC Secondary Program Loader\n");

#ifndef CONFIG_FPGA
	pll_init();
#endif

	sdram_init();

#ifndef CONFIG_FPGA
	msc_cdr = CPM_MSCCDR_MPCS_SRC | CPM_MSCCDR_CE;
	/* msc_cdr = CPM_MSCCDR_MPCS_MPLL | CPM_MSCCDR_CE; */
	msc_cdr |= __cpm_get_xpllout(SCLK_APLL) % (25000000 / 2) ? __cpm_get_pllout2() / 25000000 / 2 : __cpm_get_pllout2() / 25000000 / 2 - 1;
	REG_CPM_MSCCDR = msc_cdr;
	while (REG_CPM_MSCCDR & CPM_MSCCDR_MSC_BUSY) ;
#endif

	/*
	 * Load U-Boot image from NAND into RAM
	 */
	mmc_load(CFG_MSC_U_BOOT_SIZE, (uchar *)CFG_MSC_U_BOOT_DST);

	uboot = (void (*)(void))CFG_MSC_U_BOOT_START;

//	serial_puts("Starting U-Boot ...\n");

	/*
	 * Flush caches
	 */
	flush_cache_all();

	/*
	 * Jump to U-Boot image
	 */
	(*uboot)();
}
