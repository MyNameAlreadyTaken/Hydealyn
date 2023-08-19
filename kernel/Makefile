OBJS = hdisk.o time.o hdisk_request.o file_system.o

INCLUDE_PATH = ../include/

TOOLPATH = E:/tolset/z_tools/
INCPATH  = E:/tolset/z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
CFLAGS	= -I ../include/ -fno-builtin -Wall
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

%.gas : %.c $(INCLUDE_PATH)$*.h ../head.h ../chr.h Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.o : %.nas Makefile
	$(NASK) $*.nas $*.o $*.lst

install:
	$(CC1) $(CFLAGS) -o hdisk.gas hdisk.c
	$(GAS2NASK) hdisk.gas hdisk.nas
	$(NASK) hdisk.nas hdisk.o hdisk.lst
	$(CC1) $(CFLAGS) -o time.gas time.c
	$(GAS2NASK) time.gas time.nas
	$(NASK) time.nas time.o time.lst
	$(CC1) $(CFLAGS) -o hdisk_request.gas hdisk_request.c
	$(GAS2NASK) hdisk_request.gas hdisk_request.nas
	$(NASK) hdisk_request.nas hdisk_request.o hdisk_request.lst
	$(CC1) $(CFLAGS) -o file_system.gas file_system.c
	$(GAS2NASK) file_system.gas file_system.nas
	$(NASK) file_system.nas file_system.o file_system.lst
