include ../../commons.mk

TARGET_OBJCOPY?=objcopy
TARGET_LD?=clang
TARGET_CC?=clang
TARGET_CFLAGS?=-O2 -ggdb -Wall -Wpedantic -Werror
TARGET_CFLAGS+=-target i386-pc-none -ffreestanding -fno-pie -fno-exceptions -fno-stack-protector -mno-red-zone -mno-sse
TARGET_LDFLAGS+=-Wl,-melf_i386 -lgcc -nostdlib

BOOT_CSRC+=entry_point.c mm.c
BOOT_OBJ=startup.o

BOOT_CSRC_NOTDIR=$(notdir $(BOOT_CSRC))
BOOT_COBJ=$(BOOT_CSRC_NOTDIR:.c=.o)
BOOT_OBJ+=$(BOOT_COBJ)
INCLUDE_DIRS+=$(sort $(dir $(BOOT_CSRC)))

TARGET_CFLAGS+=$(addprefix -I,$(INCLUDE_DIRS))

VPATH=$(sort $(dir $(BOOT_CSRC)))

all: bootsector.bin boot.bin boot-install disk.img

bootsector.bin: bootsector.asm Makefile
	$(FASM) $< $@

boot.bin: boot.asm boot_main.bin Makefile
	$(FASM) $< $@

boot_main.elf: $(BOOT_OBJ) script.ld Makefile
	$(TARGET_LD) $(TARGET_CFLAGS) -Wl,-Tscript.ld -o $@ $(BOOT_OBJ) $(TARGET_LDFLAGS)

boot_main.bin: boot_main.elf Makefile
	$(TARGET_OBJCOPY) -R .note -R .comment -R .interp -S -O binary $< $@

startup.o: startup.asm Makefile
	$(FASM) $< $@

$(BOOT_COBJ): %.o: %.c Makefile
	$(TARGET_CC) -c $(TARGET_CFLAGS) -MD -MF $@.deps -o $@ $<

boot-install: boot-install.c bootsector_data.h Makefile
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $<

bootsector_data.h: bootsector.bin ../../utils/bin2hex Makefile
	../../utils/bin2hex $< $@

disk.img: boot-install boot.bin Makefile
	$(DD) if=/dev/zero of=disk.img bs=512 count=2880 status=none
	./boot-install disk.img boot.bin

test: disk.img
	$(QEMU_X86_64) $<

qtcreator_project:
	$(RM_RFV) project.files project.includes project.config
	echo "" > project.creator
	$(foreach f,$(BOOT_CSRC),echo $(f) >> project.files;)
	$(foreach f,$(INCLUDE_DIRS),echo $(f) >> project.includes;)
	$(TARGET_CC) $(TARGET_CFLAGS) -dM -E - < /dev/null > project.config

clean:
	$(RM_RFV) bootsector.bin boot.bin boot_main.elf boot_main.bin *.o bootsector_data.h boot-install disk.img *.deps

-include *.deps

.PHONY: all clean qtcreator_project
