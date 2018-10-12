include ../../commons.mk

TARGET_OBJCOPY?=objcopy
TARGET_LD?=lld-link
TARGET_CC?=clang
TARGET_CFLAGS?=-O2 -ggdb -Wall -Wpedantic -Werror
TARGET_CFLAGS+=-target x86_64-pc-win32-coff -ffreestanding -fno-exceptions -fno-stack-protector -mno-red-zone -fshort-wchar -flto -DHAVE_USE_MS_ABI
TARGET_LDFLAGS+=-subsystem:efi_application -nodefaultlib -dll

BOOT_CSRC+=entry_point.c

BOOT_CSRC_NOTDIR=$(notdir $(BOOT_CSRC))
BOOT_COBJ=$(BOOT_CSRC_NOTDIR:.c=.o)
BOOT_OBJ+=$(BOOT_COBJ)
INCLUDE_DIRS+=$(sort $(dir $(BOOT_CSRC)))

TARGET_CFLAGS+=$(addprefix -I,$(INCLUDE_DIRS))

VPATH=$(sort $(dir $(BOOT_CSRC)))

all: bootx64.efi boot

bootx64.efi: $(BOOT_OBJ) Makefile
	$(TARGET_LD) $(TARGET_LDFLAGS) -entry:efi_main $(BOOT_OBJ) -out:$@
	$(RM_RFV) ${@:.efi=.lib}

$(BOOT_COBJ): %.o: %.c Makefile
	$(TARGET_CC) -c $(TARGET_CFLAGS) -MD -MF $@.deps -o $@ $<

boot: bootx64.efi
	$(MKDIR_P) boot/EFI/BOOT
	$(CP_RV) bootx64.efi boot/EFI/BOOT/BOOTX64.EFI

test: boot
	qemu-system-x86_64 -bios /usr/share/ovmf/x64/OVMF_CODE.fd -net none -drive format=raw,file=fat:rw:boot

clean:
	$(RM_RFV) *.o *.deps *.efi boot

-include *.deps

.PHONY: all clean boot
