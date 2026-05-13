AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

ASFLAGS_BIN = -f bin
ASFLAGS_ELF = -f elf32
CFLAGS = -m32 -ffreestanding -fno-stack-protector -fno-leading-underscore \
         -ffunction-sections -mgeneral-regs-only -mno-red-zone -I./include -c \
		 -fno-pic -fno-asynchronous-unwind-tables
LDFLAGS = -m elf_i386 -T linker.ld --nostdlib --static

OBJ = kernel.o cmos.o stdio.o mouse2.o utils.o keyboard.o font.o io.o inout.o mouse.o irq_hndlr.o idt.o isr.o task.o ata.o fat.o

all: os-image.img

os-image.img: boot.bin kernel.bin 
	dd if=/dev/zero of=os-image.img bs=512 count=2880

	cat boot.bin kernel.bin > temp_image.bin
	dd if=temp_image.bin of=os-image.img conv=notrunc
	
	
	@rm temp_image.bin

boot.bin: cpu/boot/entry.asm
	$(AS) $(ASFLAGS_BIN) $< -o $@

io.o: drivers/keyboard/asm/io.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

inout.o: utils/asm/inout.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

mouse.o: drivers/mouse/asm/mouse.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

kernel.o: kernel/main/kernel.c
	$(CC) $(CFLAGS) $< -o $@

cmos.o: drivers/cmos/cmos.c
	$(CC) $(CFLAGS) $< -o $@

stdio.o: drivers/video/video.c
	$(CC) $(CFLAGS) $< -o $@

mouse2.o: drivers/mouse/mouse.c
	$(CC) $(CFLAGS) $< -o $@

utils.o: utils/utils.c
	$(CC) $(CFLAGS) $< -o $@

keyboard.o: drivers/keyboard/keyboard.c
	$(CC) $(CFLAGS) $< -o $@

font.o: drivers/video/font/font.c
	$(CC) $(CFLAGS) $< -o $@

idt.o: cpu/idt/idt.c
	$(CC) $(CFLAGS) $< -o $@

task.o: cpu/idt/tasks/task.c
	$(CC) $(CFLAGS) $< -o $@

irq_hndlr.o: cpu/idt/asm/irq_hndlr.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

isr.o: cpu/idt/asm/isr.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

ata.o: drivers/ata/ata.c
	$(CC) $(CFLAGS) $< -o $@

fat.o: drivers/fat/fat.c
	$(CC) $(CFLAGS) $< -o $@

kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJ)
	$(OBJCOPY) -O binary kernel.elf kernel.bin


clean:
	rm -f *.o *.bin *.elf 

cleane:
	rm -f *.o *.bin *.elf *.img

run: os-image.img
	qemu-system-i386 -drive file=os-image.img,format=raw -display gtk

boch:
	bochs -f bochsrc.txt