; Secondary boot loader
org 0x7E00
	jmp entry_point
; Data
a20_check_value dw 0xC0DE
reboot_msg db "Press any key to reboot...",13,10,0
label boot_disk_id byte at $$
; Print string from DS:SI
print_str:
	push ax si
	mov ah, 0x0E
@@:
	lodsb
	test al, al
	jz @f
	int 0x10
	jmp @b
@@:
	pop si ax
	ret
; Error
error:
	pop si
	call print_str
; Reboot
reboot:
	mov si, reboot_msg
	call print_str
	xor ah, ah
	int 0x16
	jmp 0xFFFF:0
; Entry point
entry_point:
	; Save boot disk id to stack
	push dx
	; Calculate CRC16
	mov dx, 0xFFFF
	mov si, $$ shr 4
	mov cx, boot_size shr 4
.crc16_paragraph:
	push cx si ds
	mov ds, si
	xor si, si
	mov cx, 16
@@:
	lodsb
	movzx bx, dh
	xor bl, al
	shl bx, 1
	mov ax, [es:crc16_table + bx]
	shl dx, 8
	xor dx, ax
	loop @b
	pop ds si cx
	inc si
	loop .crc16_paragraph
	xor dx, 0xFFFF
	; Check calculated CRC16
	cmp dx, [boot_crc16]
	je @f
	call error
	db "WRONG SECONDARY BOOT LOADER CHECKSUM",13,10,0
@@:
	; Save boot disk id to memory
	pop dx
	mov [boot_disk_id], dl
	; Fetch memory map
	call fetch_memory_map
	; Check if processor supports protected mode
	mov dx, 0x7202
	push dx
	popf
	pushf
	pop ax
	cmp ax, dx
	je @f
	call error
	db "REQUIRED I386 OR BETTER",13,10,0
@@:
	; Enable A20
	call check_a20
	jnc @f
	call enable_a20_via_bios
	jnc @f
	call enable_a20_via_keyboard
	jnc @f
	call enable_a20_fast
	jnc @f
	call error
	db "FAILED TO ENABLE A20 ADDRESS LINE",13,10,0
@@:
	; Disable interrupts
	cli
	; Load GDTR
	lgdt [gdtr]
	; Switch to the protected mode
	mov eax, cr0
	or al, 1
	mov cr0, eax
	; Jump to the protected mode entry point
	jmp 8:start32
; Check if A20 enabled
check_a20:
	push ax si di es
	mov ax, 0xFFFF
	mov es, ax
	mov si, a20_check_value
	lea di, [a20_check_value + 0x10]
	mov ax, [ds:si]
	cmp ax, [es:di]
	jne .ok
	not word[ds:si]
	mov ax, [ds:si]
	cmp ax, [es:di]
	jne .ok
	stc
	jmp .exit
.ok:
	clc
.exit:
	pop es di si ax
	ret
; Try enable A20 using BIOS
enable_a20_via_bios:
	push ax
	mov ax, 0x2403
	int 0x15
	jc .exit
	mov ax, 0x2401
	int 0x15
	call check_a20
.exit:
	pop ax
	ret
; Try enable A20 using keyboard controller
enable_a20_via_keyboard:
	push ax
	cli
	call .wait2
	mov al, 0xAD
	out 0x64, al
	call .wait2
	mov al, 0xD0
	out 0x64, al
	call .wait1
	in al, 0x60
	push ax
	call .wait2
	mov al, 0xD1
	out 0x64, al
	call .wait2
	pop ax
	or al, 2
	out 0x60, al
	call .wait2
	mov al, 0xAE
	out 0x64, al
	call .wait2
	sti
	pop ax
	call check_a20
	ret
.wait1:
	in al, 0x64
	test al, 1
	jz .wait1
	ret
.wait2:
	in al, 0x64
	test al, 2
	jnz .wait2
	ret
; Try fast enable A20
enable_a20_fast:
	push ax
	in al, 0x92
	test al, 2
	jnz @f
	or al, 2
	and al, 0xFE
	out 0x92, al
@@:
	pop ax
	call check_a20
	ret
; 32-bit entry point
use32
align 16
start32:
	; Setup segment registers and stack
	mov ax, 16
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	movzx esp, sp
	; Jump to main boot code
	push pm_functions_table
	call boot_main
	add esp, 4
	jmp reboot32
; Free space and signature
rb 510 - ($ - $$)
db 0xAA,0x55
; CRC16 table
crc16_table:
	repeat 256
		.r = (% - 1) shl 8
		repeat 8
			if .r and (1 shl 15)
				.r = (.r shl 1) xor 0x8005
			else
				.r = .r shl 1
			end if
			.r = .r and 0xFFFF
		end repeat
		dw .r
	end repeat
; 32-bit data
align 16
gdt:
	dq 0x0000000000000000 ; 0: null
	dq 0x00CF9A000000FFFF ; 8: 32-bit code
	dq 0x00CF92000000FFFF ; 16: 32-bit data
	dq 0x00009A000000FFFF ; 24: 16-bit code
	dq 0x000092000000FFFF ; 32: 16-bit data
gdtr:
	.limit dw gdt - gdtr - 1
	.base dd gdt
idtr16:
	.limit dw 0x3FF
	.base dd 0
; 32-bit to 16-bit mode bridge
align 16
call16:
	pushfd
	cli
	sidt [.saved_idtr]
	lidt [idtr16]
	push eax
	mov ax, 32
	mov ds, ax
	mov es, ax
	mov ss, ax
	jmp 24:@f
align 16
use16
@@:
	mov eax, cr0
	and eax, not 1
	mov cr0, eax
	jmp 0:@f
align 16
@@:
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	pop eax
	sti
	call [.func]
	cli
	push eax
	lgdt [gdtr]
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	jmp 8:@f
align 16
use32
@@:
	mov ax, 16
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	movzx esp, sp
	pop eax
	lidt [.saved_idtr]
	popfd
	ret
align 16
.func dw reboot
.saved_idtr: rb 6
; Table of functions wrappers
pm_functions_table:
	.reboot dd reboot32
	.print_str dd print_str32
	.get_memory_map dd get_memory_map32
; Reboot from the protected mode
align 16
reboot32:
	mov [call16.func], reboot
	jmp call16
; Print string from the protected mode
align 16
print_str32:
	push ebp esi edi
	mov ebp, esp
	mov esi, [ebp + 16]
	cmp esi, 0x100000
	jb .no_copy
	mov edi, esi
	xor ecx, ecx
	dec ecx
	xor al, al
	repne scasb
	not ecx
	sub esp, ecx
	mov edi, esp
	rep movsb
	mov esi, esp
.no_copy:
	mov [call16.func], .print_str16
	call call16
	mov esp, ebp
	pop edi esi ebp
	ret
use16
.print_str16:
	push ds
	mov eax, esi
	shr eax, 4
	mov ds, ax
	and si, 0x000F
	call print_str
	pop ds
	ret
use32
; Fetch memory map
align 16
get_memory_map32:
	mov eax, memory_map
	ret
use16
fetch_memory_map:
	mov di, memory_map.entries
	xor ebx, ebx
@@:
	mov dword[di + 20], 1
	mov edx, "PAMS"
	mov eax, 0xE820
	mov ecx, 24
	int 0x15
	jc @f
	cmp eax, "PAMS"
	jne @f
	cmp cl, 20
	jb @f
	test ebx, ebx
	jz @f
	add di, 24
	inc [memory_map.size]
	cmp di, memory_map.entries_end
	jb @b
	mov si, memory_map_too_big_msg
	call print_str
@@:
	cmp [memory_map.size], 0
	ja .exit
	xor cx, cx
	xor dx, dx
	mov ax, 0xE801
	int 0x15
	jc .e801_failed
	cmp ah, 0x86
	je .e801_failed
	cmp ah, 0x80
	je .e801_failed
	jcxz @f
	mov ax, cx
	mov dx, dx
@@:
	movzx eax, ax
	movzx ebx, bx
	shl eax, 10
	shl ebx, 16
.fill_memory_map:
	mov [memory_map.size], 1
	mov dword[memory_map.entries + 0], 0x100000
	mov dword[memory_map.entries + 4], 0
	mov dword[memory_map.entries + 8], eax
	mov dword[memory_map.entries + 12], 0
	mov dword[memory_map.entries + 16], 1
	mov dword[memory_map.entries + 20], 1
	test ebx, ebx
	jz .exit
	inc [memory_map.size]
	mov dword[memory_map.entries + 24 + 0], 0x1000000
	mov dword[memory_map.entries + 24 + 4], 0
	mov dword[memory_map.entries + 24 + 8], ebx
	mov dword[memory_map.entries + 24 + 12], 0
	mov dword[memory_map.entries + 24 + 16], 1
	mov dword[memory_map.entries + 24 + 20], 1
.exit:
	ret
.e801_failed:
	clc
	mov ah, 0x88
	int 0x15
	jnc .error
	test ax, ax
	jz .error
	movzx eax, ax
	shl eax, 10
	xor ebx, ebx
	jmp .fill_memory_map
.error:
	call error
	db "UNABLE TO QUERY MEMORY MAP FROM BIOS",13,10,0
; Place for memory map
align 16
memory_map:
	.size dd 0
	.entries_ptr dd .entries
	.entries: rb 24 * 64
	.entries_end:
memory_map_too_big_msg db "WARNING: Memory map returned by BIOS is too big (contains more than 64 entries)",13,10,0
; Main boot code
rb 0x9000 - $
boot_main file "boot_main.bin"
; Secondary loader size without CRC16
align 16
boot_size = $ - $$
; CRC16 calculation
.crc16 = 0xFFFF
repeat boot_size
	load .crc16_byte byte from $$ + (% - 1)
	load .crc16_word word from crc16_table + (((.crc16 shr 8) xor .crc16_byte) and 0xFF) * 2
	.crc16 = (.crc16_word xor (.crc16 shl 8)) and 0xFFFF
end repeat
..boot_crc16 = .crc16 xor 0xFFFF
boot_crc16 dw ..boot_crc16
