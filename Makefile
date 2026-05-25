AS      := nasm
CC      := gcc
LD      := ld
OBJCOPY := objcopy
RM      := rm -f

ASFLAGS_BIN := -f bin
ASFLAGS_ELF := -f elf32
CFLAGS      := -m32 -ffreestanding -fno-stack-protector -fno-leading-underscore \
               -ffunction-sections -mgeneral-regs-only -mno-red-zone -I./include -c \
               -fno-pic -fno-asynchronous-unwind-tables
LDFLAGS     := -m elf_i386 -T linker.ld --nostdlib --static

OBJ := kernel.o cmos.o video.o mouse_asm.o utils.o keyboard.o font.o io.o inout.o \
       mouse.o irq_hndlr.o idt.o isr.o task.o ata.o fat.o read.o write.o \
       sound.o pci.o rtl8139.o

vpath %.c kernel/main drivers/cmos drivers/video drivers/mouse utils drivers/keyboard \
          drivers/video/font cpu/idt cpu/idt/tasks drivers/ata drivers/fat drivers/sound drivers/pci \
		  drivers/rtl8139
		  
vpath %.asm cpu/boot drivers/keyboard/asm utils/asm drivers/mouse/asm cpu/idt/asm cpu/mm/asm

.PHONY: all clean cleane run boch help push

all: os-image.img

os-image.img: boot.bin KERNEL.SYS
	dd if=/dev/zero of=$@ bs=512 count=2880
	mformat -i $@ -f 1440 ::
	dd if=$< of=$@ conv=notrunc bs=512 count=1
	mcopy -i $@ KERNEL.SYS ::KERNEL.SYS

boot.bin: cpu/boot/entry.asm
	$(AS) $(ASFLAGS_BIN) $< -o $@

KERNEL.SYS: $(OBJ)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJ)
	$(OBJCOPY) -O binary kernel.elf $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS_ELF) $< -o $@

clean:
	$(RM) *.o *.bin *.elf KERNEL.SYS

cleane:
	$(RM) *.o *.bin *.elf *.img *.vdi KERNEL.SYS

run: os-image.img
	qemu-system-i386 -drive file=os-image.img,format=raw \
	-audiodev pa,id=snd0 -machine pcspk-audiodev=snd0 \
	-netdev user,id=net0 \
    -object filter-dump,id=f1,netdev=net0,file=traffic.pcap \
    -device rtl8139,netdev=net0

boch:
	bochs -f bochsrc.txt

help:
	@echo boch - start with bochs
	@echo run - start
	@echo clean - clear files
	@echo cleane - clear all files

push:
	git add .
	git commit -m "CalcOS"
	git push origin main --force

test:
	tcpdump -XX -r traffic.pcap