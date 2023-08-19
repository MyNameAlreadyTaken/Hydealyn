
OBJS	= bootpack.o nasmfunc.o ui.o dsctbl.o int.o fifo.o mouse.o mem.o sheet.o timer.o process.o console.o win.o

KOBJS	= kernel/hdisk.o kernel/time.o kernel/hdisk_request.o kernel/file_system.o

BINS	= ipl10.bin asmhead.bin

HYDAELYNKERNEL	= Hydaelyn.bin

DASMOUTPUT	= Hydaelyn.asm

KERNEL	= kernel.bin

CC	= gcc
CFLAGS	= -I ./include/ -fno-builtin -fno-stack-protector -Wall -c -m32 -lc
CKFLAGS	= -I ./include/ -fno-builtin -fno-stack-protector -Wall -c -m32 -lc
ASM	= nasm
ASMKFLAGS	= -I include/ -f elf32
DASM	= objdump
DASMFLAGS	= -D
LD	= ld
LDFLAGS	= -m elf_i386 -e main -s -Ttext 0x300400 -Map krnl.map --no-dynamic-linker -L /usr/lib32 -lc #-T rule.lds
#LDFLAGS	+= $(call lmZ-option, --no-dynamic-linker)

everything : $(BINS) $(KERNEL)

make :
	everything

clean :
	rm -f $(OBJS)
	rm -f $(KOBJS)

buildimg :
	dd if=ipl10.bin of=b.img bs=512 count=1 conv=notrunc
	sudo mount -o loop b.img /mnt/floppy/
	sudo cp -fv asmhead.bin /mnt/floppy/
	sudo cp -fv Hydaelyn.bin /mnt/floppy
	sudo umount /mnt/floppy

ipl10.bin : ipl10.asm
	$(ASM) -o $@ $<

#kernel.bin : do.o
#	$(LD) $(LDFLAGS) -o $@ $^
	
#do.o : do.asm
#	$(ASM) -f elf -o $@ $<

#kernel.bin : main.o nasmfunc.o
#	$(LD) $(LDFLAGS) -o $@ $^
	
#main.o : main.c
#	$(CC) $(CFLAGS) -o $@ $<

kernel.bin : $(OBJS) $(KOBJS)
	$(LD) $(LDFLAGS) -o $@ $^

nasmfunc.o : nasmfunc.asm
	$(ASM) -o $@ -f elf $<

asmhead.bin : asmhead.asm
	$(ASM) -o $@ $<

bootpack.o : bootpack.c
	$(CC) $(CFLAGS) -o $@ $<

ui.o : ui.c 
	$(CC) $(CFLAGS) -o $@ $<

dsctbl.o : dsctbl.c
	$(CC) $(CFLAGS) -o $@ $<

int.o : int.c
	$(CC) $(CFLAGS) -o $@ $<

fifo.o : fifo.c
	$(CC) $(CFLAGS) -o $@ $<

mouse.o : mouse.c
	$(CC) $(CFLAGS) -o $@ $<

mem.o : mem.c
	$(CC) $(CFLAGS) -o $@ $<

sheet.o : sheet.c
	$(CC) $(CFLAGS) -o $@ $<

timer.o : timer.c
	$(CC) $(CFLAGS) -o $@ $<

process.o : process.c
	$(CC) $(CFLAGS) -o $@ $<

win.o : win.c
	$(CC) $(CFLAGS) -o $@ $<

console.o : console.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/hdisk.o : kernel/hdisk.c
	$(CC) $(CKFLAGS) -o $@ $<

kernel/time.o : kernel/time.c
	$(CC) $(CKFLAGS) -o $@ $<

kernel/hdisk_request.o : kernel/hdisk_request.c
	$(CC) $(CKFLAGS) -o $@ $<

kernel/file_system.o : kernel/file_system.c
	$(CC) $(CKFLAGS) -o $@ $<
