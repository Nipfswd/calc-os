[bits 32]
[extern timer_handler] 
[extern keyboard_handler]
[extern stub_mouse_handler]
[extern ata_handler]

global timer_wrapper
global keyboard_wrapper
global mouse_wrapper
global ata_wrapper
timer_wrapper:
    cli
    push dword 0   
    push dword 32  
    pusha
    mov ax, ds
    push eax         

    mov ax, 0x10        
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp          
    call timer_handler  
    
    add esp, 4         

    mov esp, eax       

    pop eax          
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa                
    add esp, 8          
    iret
    
keyboard_wrapper:
    pusha
    call keyboard_handler
    popa
    iret

mouse_wrapper:
    pusha
    call stub_mouse_handler
    popa
    iret   

ata_wrapper:
    pusha
    call ata_handler
    popa
    iret