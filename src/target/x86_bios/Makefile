include ../../commons.mk

TARGET_OBJCOPY?=objcopy
TARGET_LD?=$(HOST_CC)
TARGET_CC?=$(HOST_CC)
TARGET_CFLAGS?=-O2 -ggdb -Wall -Wpedantic -Werror
TARGET_CFLAGS+=-m32 -ffreestanding -fno-pie -fno-exceptions -fno-stack-protector
TARGET_LDFLAGS+=-Wl,-melf_i386 -lgcc -nostdlib

BOOT_CSRC=entry_point.c ../../main.c ../../stdfunctions.c mm.c
BOOT_OBJ=startup.o

BOOT_CSRC_NOTDIR=$(notdir $(BOOT_CSRC))
BOOT_COBJ=$(BOOT_CSRC_NOTDIR:.c=.o)
BOOT_OBJ+=$(BOOT_COBJ)
INCLUDE_DIRS+=$(sort $(dir $(BOOT_CSRC)))

TARGET_CFLAGS+=$(addprefix -I,$(INCLUDE_DIRS))

VPATH=$(sort $(dir $(BOOT_CSRC)))

all: bootsector.bin boot.bin boot-install disk.img

bootsector.bin: bootsector.asm
	$(FASM) bootsector.asm bootsector.bin

boot.bin: boot.asm boot_main.bin
	$(FASM) boot.asm boot.bin

boot_main.elf: $(BOOT_OBJ) script.ld
	$(TARGET_LD) $(TARGET_CFLAGS) -T script.ld -o boot_main.elf $(BOOT_OBJ) $(TARGET_LDFLAGS)

boot_main.bin: boot_main.elf
	$(TARGET_OBJCOPY) -R .note -R .comment -R .interp -S -O binary boot_main.elf boot_main.bin

startup.o: startup.asm
	$(FASM) startup.asm startup.o

$(BOOT_COBJ): %.o: %.c
	$(TARGET_CC) -c $(TARGET_CFLAGS) -MD -MF $@.deps -o $@ $<

boot-install: boot-install.c bootsector_data.h
	$(HOST_CC) $(HOST_CFLAGS) -o boot-install boot-install.c

bootsector_data.h: bootsector.bin ../../utils/bin2hex
	../../utils/bin2hex bootsector.bin bootsector_data.h

disk.img: boot-install boot.bin
	$(DD) if=/dev/zero of=disk.img bs=512 count=2880 status=none
	./boot-install disk.img boot.bin

clean:
	$(RM_RFV) bootsector.bin boot.bin boot_main.elf boot_main.bin *.o bootsector_data.h boot-install disk.img *.deps

-include *.deps

.PHONY: all clean