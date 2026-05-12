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
    pusha    
    push esp           
    call timer_handler  
    mov esp, eax
    popa                
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