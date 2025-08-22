TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -O0 -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com

COPY     = cmd /c copy
DEL      = cmd /c del /Q

default :
	$(MAKE) img

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

console.gas : console.c Makefile
	$(CC1) -o console.gas console.c

console.nas : console.gas Makefile
	$(GAS2NASK) console.gas console.nas

console.obj : console.nas Makefile
	$(NASK) console.nas console.obj console.lst

command.gas : command.c Makefile
	$(CC1) -o command.gas command.c

command.nas : command.gas Makefile
	$(GAS2NASK) command.gas command.nas

command.obj : command.nas Makefile
	$(NASK) command.nas command.obj command.lst

cal.gas : cal.c Makefile
	$(CC1) -o cal.gas cal.c

cal.nas : cal.gas Makefile
	$(GAS2NASK) cal.gas cal.nas

cal.obj : cal.nas Makefile
	$(NASK) cal.nas cal.obj cal.lst

task_console.gas : task_console.c Makefile
	$(CC1) -o task_console.gas task_console.c

task_console.nas : task_console.gas Makefile
	$(GAS2NASK) task_console.gas task_console.nas

task_console.obj : task_console.nas Makefile
	$(NASK) task_console.nas task_console.obj task_console.lst

timer.gas : timer.c Makefile
	$(CC1) -o timer.gas timer.c

timer.nas : timer.gas Makefile
	$(GAS2NASK) timer.gas timer.nas

timer.obj : timer.nas Makefile
	$(NASK) timer.nas timer.obj timer.lst

task.gas : task.c Makefile
	$(CC1) -o task.gas task.c

task.nas : task.gas Makefile
	$(GAS2NASK) task.gas task.nas

task.obj : task.nas Makefile
	$(NASK) task.nas task.obj task.lst

window.gas : window.c Makefile
	$(CC1) -o window.gas window.c

window.nas : window.gas Makefile
	$(GAS2NASK) window.gas window.nas

window.obj : window.nas Makefile
	$(NASK) window.nas window.obj window.lst


snake.gas : snake.c Makefile
	$(CC1) -o snake.gas snake.c

snake.nas : snake.gas Makefile
	$(GAS2NASK) snake.gas snake.nas

snake.obj : snake.nas Makefile
	$(NASK) snake.nas snake.obj snake.lst


fifo.gas : fifo.c Makefile
	$(CC1) -o fifo.gas fifo.c

fifo.nas : fifo.gas Makefile
	$(GAS2NASK) fifo.gas fifo.nas

fifo.obj : fifo.nas Makefile
	$(NASK) fifo.nas fifo.obj fifo.lst

bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

font.gas : font.c Makefile
	$(CC1) -o font.gas font.c

font.obj : font.nas Makefile
	$(NASK) font.nas font.obj font.lst

mouse.gas : mouse.c Makefile
	$(CC1) -o mouse.gas mouse.c

mouse.nas : mouse.gas Makefile
	$(GAS2NASK) mouse.gas mouse.nas

mouse.obj : mouse.nas Makefile
	$(NASK) mouse.nas mouse.obj mouse.lst

main.gas : main.c Makefile
	$(CC1) -o main.gas main.c

main.nas : main.gas Makefile
	$(GAS2NASK) main.gas main.nas

main.obj : main.nas Makefile
	$(NASK) main.nas main.obj main.lst

dsctbl.gas : dsctbl.c Makefile
	$(CC1) -o dsctbl.gas dsctbl.c

dsctbl.nas : dsctbl.gas Makefile
	$(GAS2NASK) dsctbl.gas dsctbl.nas

dsctbl.obj : dsctbl.nas Makefile
	$(NASK) dsctbl.nas dsctbl.obj dsctbl.lst

memory.gas : memory.c Makefile
	$(CC1) -o memory.gas memory.c

memory.nas : memory.gas Makefile
	$(GAS2NASK) memory.gas memory.nas

memory.obj : memory.nas Makefile
	$(NASK) memory.nas memory.obj memory.lst

int.gas : int.c Makefile
	$(CC1) -o int.gas int.c

int.nas : int.gas Makefile
	$(GAS2NASK) int.gas int.nas

int.obj : int.nas Makefile
	$(NASK) int.nas int.obj int.lst

graphic.gas : graphic.c Makefile
	$(CC1) -o graphic.gas graphic.c

graphic.nas : graphic.gas Makefile
	$(GAS2NASK) graphic.gas graphic.nas

graphic.obj : graphic.nas Makefile
	$(NASK) graphic.nas graphic.obj graphic.lst

sheet.gas : sheet.c Makefile
	$(CC1) -o sheet.gas sheet.c

sheet.nas : sheet.gas Makefile
	$(GAS2NASK) sheet.gas sheet.nas

sheet.obj : sheet.nas Makefile
	$(NASK) sheet.nas sheet.obj sheet.lst

fs.gas : fs.c Makefile
	$(CC1) -o fs.gas fs.c

fs.nas : fs.gas Makefile
	$(GAS2NASK) fs.gas fs.nas

fs.obj : fs.nas Makefile
	$(NASK) fs.nas fs.obj 

naskfunc.obj : naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

bootpack.bim : bootpack.obj main.obj graphic.obj dsctbl.obj int.obj naskfunc.obj font.obj hankaku.obj fifo.obj mouse.obj memory.obj sheet.obj window.obj timer.obj task.obj console.obj command.obj task_console.obj fs.obj snake.obj cal.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj main.obj graphic.obj dsctbl.obj int.obj naskfunc.obj font.obj hankaku.obj fifo.obj mouse.obj memory.obj sheet.obj window.obj timer.obj task.obj console.obj command.obj task_console.obj fs.obj snake.obj cal.obj

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	$(COPY) /B asmhead.bin+bootpack.hrb haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	$(EDIMG) imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin *.lst *.gas *.obj *.map *.bim *.hrb haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
