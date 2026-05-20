[bits 16]
[org 0x7c00]

jmp start_code
nop
OEM_ID db "MSDOS5.0"
bytes_per_sector dw 512
sectors_per_cluster db 1
reserved_sectors dw 1
num_fats db 2
root_entries dw 224
total_sectors_short dw 2880
media_type db 0xF0
fat_size_sectors dw 9
sectors_per_track dw 18
num_heads dw 2
hidden_sectors dd 0
total_sectors_long dd 0
boot_drive_number db 0
reserved1 db 0
boot_signature db 0x29
volume_id dd 0x12345678
volume_label db "NO NAME    "
file_system_type db "FAT12   "

start_code:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    mov [boot_drive], dl

    in al, 0x92
    or al, 2
    out 0x92, al

    mov ax, 0x0011
    int 0x10

    mov eax, 33
    mov [disk_packet + 8], eax 

    mov ax, 0x1000
    mov es, ax
    mov bx, 0

    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, disk_packet
    int 0x13
    jc disk_error

find_ahci:
    mov ax, 0xB103
    mov ecx, 0x010601
    xor si, si
    int 0x1A
    jc ahci_not_found

read_bar5:
    mov ax, 0xB10A
    mov di, 0x24
    int 0x1A
    jc ahci_not_found

save_address:
    and ecx, 0xFFFFFFF0
    mov [0x8000], ecx
    jmp continue_boot

ahci_not_found:
    mov [0x8000], dword 0

continue_boot:
    cli
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:init_32bit

disk_error:
    cli
    hlt
    jmp disk_error

[bits 32]
init_32bit:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000 
    mov ebp, esp

    jmp 0x10000

boot_drive db 0

align 4
disk_packet:
    db 0x10      
    db 0             
    dw 60               
    dw 0x0000        
    dw 0x1000       
    dq 0            

gdt_start:
    dq 0
gdt_code:
    dw 0xffff, 0x0000, 0x9a00, 0x00cf
gdt_data:
    dw 0xffff, 0x0000, 0x9200, 0x00cf
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55