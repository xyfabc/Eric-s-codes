#ifndef __NAND_CONFIG_H
#define __NAND_CONFIG_H

/*
 * This file contains the nand configuration parameters for the cygnus board.
 */
/*-----------------------------------------------------------------------
 * HYNIX H8BES0UQ0MCR.h
 * MCP NAND FLASH configuration
 */
#define CFG_NAND_BCH_BIT	4		/* Specify the hardware BCH algorithm for nand (4|8) */
#define CFG_NAND_BW8		0		/* Data bus width: 0-16bit, 1-8bit */
#define CFG_NAND_PAGE_SIZE	2048
#define CFG_NAND_OOB_SIZE	64		/* Size of OOB space per page (e.g. 64 128 etc.) */
#define CFG_NAND_ROW_CYCLE	3
#define CFG_NAND_BLOCK_SIZE	(128 << 10)	/* NAND chip block size		*/
#define CFG_NAND_BADBLOCK_PAGE	0		/* NAND bad block was marked at this page in a block, starting from 0 */

#endif /* __NAND_CONFIG_H */
