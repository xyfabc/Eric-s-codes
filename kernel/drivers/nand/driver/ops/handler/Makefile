#
# Makefile
#

KBUILD_CFLAGS	+= -Idrivers/nand/driver/inc/

obj-$(CONFIG_NAND_CPU) += cpu_msg_handler.o
obj-$(CONFIG_NAND_DMA) += dma_msg_handler.o
obj-y += spl_rw.o bad_block.o
