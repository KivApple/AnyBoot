DD?=dd
RM_RFV?=rm -rfv
MKDIR_P?=mkdir -p
CP_RV?=cp -rv

HOST_CC?=clang
HOST_CFLAGS?=-O2 -ggdb -Wall -Wextra -Wpedantic -Werror

FASM?=fasm

QEMU_X86_64?=qemu-system-x86_64

BOOT_CSRC+=../../main.c ../../stdfunctions.c 

define NEWLINE

endef
