#
# (C) Copyright 2009 Ingenic Semiconductor Inc.
# Author: Regen Huang <lhhuang@ingenic.cn>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk
include $(TOPDIR)/include/config.mk
include $(TOPDIR)/spi_spl/board/$(BOARDDIR)/config.mk

LDSCRIPT= $(TOPDIR)/spi_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS	= -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE)
AFLAGS	+= -DCONFIG_SPI_SPL
CFLAGS	+= -DCONFIG_SPI_SPL

SOBJS	= start.o
COBJS	= spi_boot_jz4780.o cpu.o jz4780.o jz_serial.o 

ifeq ($(CONFIG_USE_MDDR),y)
COBJS	+= jz4780_ddr.o
COBJS	+= jz_mddr_init.o
endif

ifeq ($(CONFIG_USE_DDR1),y)
COBJS	+= jz4780_ddr.o
COBJS	+= jz_ddr1_init.o
endif

ifeq ($(CONFIG_USE_DDR2),y)
COBJS	+= jz4780_ddr.o
COBJS	+= jz_ddr2_init.o
endif

ifeq ($(CONFIG_USE_LPDDR2),y)
COBJS	+= jz4780_ddr.o
COBJS	+= jz_lpddr2_init.o
endif

ifeq ($(CONFIG_USE_DDR3),y)
COBJS	+= jz4780_ddr.o
COBJS	+= jz_ddr3_init.o
endif

ifeq ($(CONFIG_USE_SDRAM),y)
COBJS	+= jz4780_sdram.o
endif

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/spi_spl/board/$(BOARDDIR)

spiobj	:= $(OBJTREE)/spi_spl/

ALL	= $(spiobj)u-boot-spl $(spiobj)u-boot-spl.bin $(spiobj)u-boot-spl-pad.bin
all:	$(obj).depend $(ALL)

$(spiobj)u-boot-spl-pad.bin: $(spiobj)u-boot-spl.bin
	dd bs=1024 count=14 if=/dev/zero of=$(spiobj)junk1
	cat $< $(spiobj)junk1 > $(spiobj)junk2
	dd bs=1024 count=14 if=$(spiobj)junk2 of=$@
	rm -f $(spiobj)junk*

$(spiobj)u-boot-spl.bin:	$(spiobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@
	gcc $(OBJTREE)/tools/spi_checksum/spi_checksum.c -o $(OBJTREE)/tools/spi_checksum/spi_checksum;
	$(OBJTREE)/tools/spi_checksum/spi_checksum $(spiobj)u-boot-spl.bin; 
	rm $(OBJTREE)/tools/spi_checksum/spi_checksum; 
	
$(spiobj)u-boot-spl:	$(OBJS)
	cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
		-Map $(spiobj)u-boot-spl.map \
		-o $(spiobj)u-boot-spl

# create symbolic links for common files

# from cpu directory
$(obj)start.S:
	@rm -f $(obj)start.S
	ln -s $(SRCTREE)/cpu/mips/start.S $(obj)start.S

$(obj)cpu.c:
	@rm -f $(obj)cpu.c
	ln -s $(SRCTREE)/cpu/mips/cpu.c $(obj)cpu.c

$(obj)jz4780.c:
	@rm -f $(obj)jz4780.c
	ln -s $(SRCTREE)/cpu/mips/jz4780.c $(obj)jz4780.c

$(obj)jz_serial.c:
	@rm -f $(obj)jz_serial.c
	ln -s $(SRCTREE)/cpu/mips/jz_serial.c $(obj)jz_serial.c

ifeq ($(CONFIG_USE_MDDR),y)
$(obj)jz4780_ddr.c:
	@rm -f $(obj)jz4780_ddr.c
	ln -s $(SRCTREE)/cpu/mips/jz4780_ddr.c $(obj)jz4780_ddr.c
$(obj)jz_mddr_init.c:
	@rm -f $(obj)jz_mddr_init.c
	ln -s $(SRCTREE)/cpu/mips/jz_mddr_init.c $(obj)jz_mddr_init.c
endif

ifeq ($(CONFIG_USE_DDR1),y)
$(obj)jz4780_ddr.c:
	@rm -f $(obj)jz4780_ddr.c
	ln -s $(SRCTREE)/cpu/mips/jz4780_ddr.c $(obj)jz4780_ddr.c
$(obj)jz_ddr1_init.c:
	@rm -f $(obj)jz_ddr1_init.c
	ln -s $(SRCTREE)/cpu/mips/jz_ddr1_init.c $(obj)jz_ddr1_init.c
endif

ifeq ($(CONFIG_USE_DDR2),y)
$(obj)jz4780_ddr.c:
	@rm -f $(obj)jz4780_ddr.c
	ln -s $(SRCTREE)/cpu/mips/jz4780_ddr.c $(obj)jz4780_ddr.c
$(obj)jz_ddr2_init.c:
	@rm -f $(obj)jz_ddr2_init.c
	ln -s $(SRCTREE)/cpu/mips/jz_ddr2_init.c $(obj)jz_ddr2_init.c
endif

ifeq ($(CONFIG_USE_LPDDR2),y)
$(obj)jz4780_ddr.c:
	@rm -f $(obj)jz4780_ddr.c
	ln -s $(SRCTREE)/cpu/mips/jz4780_ddr.c $(obj)jz4780_ddr.c
$(obj)jz_lpddr2_init.c:
	@rm -f $(obj)jz_lpddr2_init.c
	ln -s $(SRCTREE)/cpu/mips/jz_lpddr2_init.c $(obj)jz_lpddr2_init.c
endif

ifeq ($(CONFIG_USE_DDR3),y)
$(obj)jz4780_ddr.c:
	@rm -f $(obj)jz4780_ddr.c
	ln -s $(SRCTREE)/cpu/mips/jz4780_ddr.c $(obj)jz4780_ddr.c
$(obj)jz_ddr3_init.c:
	@rm -f $(obj)jz_ddr3_init.c
	ln -s $(SRCTREE)/cpu/mips/jz_ddr3_init.c $(obj)jz_ddr3_init.c
endif

ifeq ($(CONFIG_USE_SDRAM),y)
$(obj)jz4780_sdram.c:
	@rm -f $(obj)jz4780_sdram.c
	ln -s $(SRCTREE)/cpu/mips/jz4780_sdram.c $(obj)jz4780_sdram.c
endif

# from spi_spl directory
$(obj)spi_boot_jz4780.c:
	@rm -f $(obj)spi_boot_jz4780.c
	ln -s $(SRCTREE)/spi_spl/spi_boot_jz4780.c $(obj)spi_boot_jz4780.c

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
