format ELF

public _start
extrn entry_point

section ".text" executable

_start:
	jmp entry_point
