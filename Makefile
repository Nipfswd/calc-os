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

OBJ = kernel.o cmos.o stdio.o mouse2.o utils.o keyboard.o font.o io.o inout.o mouse.o irq_hndlr.o idt.o isr.o task.o ata.o fat.o read.o write.o page.o page2.o sound.o pci.o

all: os-image.img

os-image.img: boot.bin KERNEL.SYS
	dd if=/dev/zero of=os-image.img bs=512 count=2880
	
	mformat -i os-image.img -f 1440 ::
	
	dd if=boot.bin of=os-image.img conv=notrunc bs=512 count=1
	
	mcopy -i os-image.img KERNEL.SYS ::KERNEL.SYS

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

read.o: drivers/fat/read.c
	$(CC) $(CFLAGS) $< -o $@

write.o: drivers/fat/write.c
	$(CC) $(CFLAGS) $< -o $@

page.o: cpu/mm/page.c
	$(CC) $(CFLAGS) $< -o $@

page2.o: cpu/mm/asm/page.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

sound.o: drivers/sound/sound.c
	$(CC) $(CFLAGS) $< -o $@

pci.o: drivers/pci/pci.c
	$(CC) $(CFLAGS) $< -o $@

KERNEL.SYS: $(OBJ)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJ)
	$(OBJCOPY) -O binary kernel.elf KERNEL.SYS


clean:
	rm -f *.o *.bin *.elf KERNEL.SYS	 

cleane:
	rm -f *.o *.bin *.elf *.img KERNEL.SYS

run: os-image.img
	qemu-system-i386 -drive file=os-image.img,format=raw -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0

boch:
	bochs -f bochsrc.txt

help:
	echo boch - start with bochs
	echo run - start
	echo clean - clear files
	echo cleane - clear all files

push:
	git add .
	git commit -m "CalcOS"
	git push origin main --force