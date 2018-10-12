all: utils target_x86_bios

clean: clean_utils clean_target_x86_bios

utils:
	make -C utils

clean_utils:
	make -C utils clean

target_x86_bios: utils
	make -C target/x86_bios

clean_target_x86_bios:
	make -C target/x86_bios clean

.PHONY: all clean utils clean_utils target_x86_bios clean_target_x86_bios
