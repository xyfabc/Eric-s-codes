
obj-y += urboard-misc.o urboard-pm.o urboard-lcd.o

obj-$(CONFIG_I2C)		+= urboard-i2c.o
obj-$(CONFIG_MFD_ACT8600)	+= urboard-regulator.o
obj-$(CONFIG_MMC_JZ4780)	+= urboard-mmc.o
obj-$(CONFIG_JZ4780_INTERNAL_CODEC) += urboard-sound.o
obj-$(CONFIG_VIDEO_JZ4780_CIM_HOST)	+= urboard-cim.o

ifeq ($(CONFIG_BOARD_TROOPER), y)
EXTRA_CFLAGS+=-DCONFIG_BOARD_URBOARD_V_1_1
endif
obj-$(CONFIG_BCM4330_RFKILL) += urboard-bt.o
