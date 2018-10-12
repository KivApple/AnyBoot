; Bootsector
org 0x7C00
	; Jump to entry point
	jmp boot
; Space for FAT12/16/32 header
rb 90 - ($ - $$)
; Data
head_count db 0
label memory_size word at $$
label spt word at $$ + 2
label disk_id byte at $$ + 4
; Error handler
error:
	pop si
	mov ah, 0x0E
@@:
	lodsb
	test al, al
	jz @f
	int 0x10
	jmp @b
@@:
	xor ah, ah
	int 0x16
	jmp 0xFFFF:0
; Entry point
boot:
	; Setup CS
	jmp 0:@f
@@:
	; Disable interrupts
	cli
	; Setup DS, ES and stack
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, $$
	; Enable interrupts
	sti
	; Save disk id
	mov [disk_id], dl
	; Check EDD present
	mov ah, 0x41
	mov bx, 0x55AA
	int 0x13
	jc .use_chs
	cmp bx, 0xAA55
	je .use_lba
.use_chs:
	mov ah, 0x08
	push es
	xor di, di
	int 0x13
	pop es
	jc .disk_error
	inc dh
	mov [head_count], dh
	and cx, 0x003F
	mov [spt], cx
.use_lba:
	; Query memory size
	int 0x12
	shl ax, 10 - 4
	mov [memory_size], ax
	; Load secondary boot loader
	mov di, secondary_boot shr 4
.load_sector:
	cmp [secondary_boot_sector_count], 0
	je .load_done
	dec [secondary_boot_sector_count]
	mov ax, di
	add ax, 512 shr 4
	cmp ax, [memory_size]
	jb @f
	call error
	db "OUT OF MEMORY",13,10,0
@@:
	push es
	cmp [head_count], 0
	je .load_sector_lba
	mov ax, word[secondary_boot_start_sector]
	mov dx, word[secondary_boot_start_sector + 2]
	div [spt]
	mov cl, dl
	inc cl
	div [head_count]
	mov dh, ah
	mov ch, al
	mov dl, [disk_id]
	mov es, di
	xor bx, bx
	mov si, 3
.try_load_sector:
	mov ax, 0x0201
	int 0x13
	jnc .load_sector_ok
	xor ah, ah
	int 0x13
	dec si
	jnz .try_load_sector
.disk_error:
	call error
	db "DISK ERROR",13,10,0
.load_sector_ok:
	pop es
	add di, 512 shr 4
	xor ax, ax
	add word[secondary_boot_start_sector], 1
	adc word[secondary_boot_start_sector + 2], ax
	adc word[secondary_boot_start_sector + 4], ax
	adc word[secondary_boot_start_sector + 6], ax
	jmp .load_sector
.load_sector_lba:
	mov ah, 0x42
	mov dl, [disk_id]
	mov [dap.segment], di
	mov si, dap
	int 0x13
	jc .disk_error
	jmp .load_sector_ok
.load_done:
	; Check secondary bootloader signature
	cmp [secondary_boot.signature], 0x55AA
	je @f
	call error
	db "WRONG MAGIC",13,10,0
@@:
	; Jump to secondary boot loader entry point
	xor ax, ax
	xor bx, bx
	xor cx, cx
	xor dx, dx
	xor si, si
	xor di, di
	xor bp, bp
	push 0x202
	popf
	mov dl, [disk_id]
	jmp secondary_boot
; Free space
rb 418 - ($ - $$)
dap:
	.size db 16
	.zero db 0
	.count dw 1
	.offset dw 0
	.segment dw ?
	.sector:
; Parameters
secondary_boot_start_sector dq 1
secondary_boot_sector_count dw 1
; Space for partition table
rb 10 + 4 * 16
; Signature
db 0x55,0xAA
; Secondary boot loader
secondary_boot:
	rb 510
	.signature dw ?
