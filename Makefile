# Makefile

CROSS_COMPILE := /opt/gcc-arm-none-eabi-6-2017-q1-update/bin/arm-none-eabi-

COMPILER := gcc

CPU := rpi1
BLD_TARGET := raycast
BLD_TYPE := debug

PROJ_DIRS := src

LNK_SCR := link.ld

include makefiles/main.mk

EXTRA_LIBS += -lm
BLD_OPTOMISE := -Os

distclean:
	rm -rf build
