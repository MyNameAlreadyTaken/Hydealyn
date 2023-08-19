#include "head.h"

void init_pic(void) {
	/*
	用ICW作初始化：确定是否需要级联、设置起始的中断向量号、设置中断结束模式等等

ICW1用来初始化8259A的连接方式：单片还是级联       中断信号触发方式：电平触发还是边沿触发

ICW2用来设置起始中断向量号。指定了 IRQ0 映射到的中断向量号，其他 IRQ 接口对应的中断向量号会顺着自动排列下去。

ICW3用来设置级联方式下主片和从片用哪个IRQ接口互连。

ICW4用来设置中断结束模式：自动还是手动。
*/
/*用OCW来操作控制：中断屏蔽、中断结束

OCW1用来屏蔽连接在8259A上的外部设备中的中断信号。这些没有屏蔽的中断最终还是要受到eflags的IF位管束，IF=0，不管8259A怎么样设置全部屏蔽。

OCW2用来设置中断结束方式：发EOI来终止某一个中断      优先级模式：循环还是固定

OCW3用来设置特殊屏蔽方式和查询方式。

ICW1和OCW2、OCW3是用偶地端口 0x20(主片)或 0xA0（从片）写入。

ICW2-ICW4是用奇地址端口 0x21(主片) 或0xA1（从片）写入。

4个ICW要保证一定的次序写入，8259A就知道写入端口的数据是什么意思了。

OCW的写入顺序无关，各控制字有唯一标识可以辨别。
*/

//	_io_out8(PIC0_IMR, 0xff);	/*禁止所有中断*/
//	_io_out8(PIC1_IMR, 0xff);		
	
//	_io_out8(PIC0_ICW1, 0x11);	/*边沿触发模式*/
//	_io_out8(PIC0_ICW2, 0x20);	/*IRQ0-7由INT20-27接收*/
//	_io_out8(PIC0_ICW3, 1 << 2);	/*PIC1由IRQ2连接*/
//	_io_out8(PIC0_ICW4, 0x01);	/*无缓冲区模式*/

//	_io_out8(PIC1_ICW1, 0x11);	/*边沿触发模式*/
//	_io_out8(PIC1_ICW2, 0x28);	/*IRQ8-15由INT28-2f接收*/
//	_io_out8(PIC1_ICW3, 2);	/*PIC1由IRQ2连接*/
//	_io_out8(PIC1_ICW4, 0x01);	/*无缓冲区模式*/
	
//	_io_out8(PIC0_IMR, 0xff);	/*11111011 PIC1以外全部禁止*/
//	_io_delay();
//	_io_out8(PIC1_IMR, 0xff);	/*11111111 禁止所有中断*/
//	_io_delay();

	_io_out8(PIC0_IMR, 0xff);	/*禁止所有中断*/
	_io_delay();
	_io_out8(PIC1_IMR, 0xff);	
	_io_delay();
	
	_io_out8(PIC0_ICW1, 0x11);
	_io_delay();
	_io_out8(PIC1_ICW1, 0x11);
	_io_delay();
	
	_io_out8(PIC0_ICW2, 0x20);
	_io_delay();
	_io_out8(PIC1_ICW2, 0x28);
	_io_delay();
	
	_io_out8(PIC0_ICW3, 0x4);
	_io_delay();
	_io_out8(PIC1_ICW3, 0x2);
	_io_delay();
	
	_io_out8(PIC0_ICW4, 0x1);
	_io_delay();
	_io_out8(PIC1_ICW4, 0x1);
	_io_delay();
	
	_io_out8(PIC0_IMR, 0xfe);	/*11111011 PIC1以外全部禁止*/
	_io_delay();
	_io_out8(PIC1_IMR, 0xff);	/*11111111 禁止所有中断*/
	_io_delay();
	
	return;
}

void wait_KBC_sendready(void) {
	while (1) {
		if ((_io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

struct FIFO16 *keyfifo;

void enable_keyboard(struct FIFO16 *fifo) {		/*激活键盘控制电路*/
	keyfifo = fifo;
	wait_KBC_sendready();
	_io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	_io_out8(PORT_KEYDAT, KBC_MODE);
	//_io_out8(PIC0_IMR, 0xf9);
	_disable_irq(1, _inthandler21);
	_enable_irq(1);
	return;
}

struct FIFO16 *mousefifo;

void enable_mouse(struct FIFO16 *fifo, struct MOUSE_DEC *mdec) {		/*激活鼠标*/
	mousefifo = fifo;
	wait_KBC_sendready();
	_io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);	/*向键盘控制电路发送0xd4后*/
	wait_KBC_sendready();				/*下一数据会发送给鼠标*/
	_io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	//_disable_irq(12, _inthandler2c);
	_enable_irq(2);
	//_enable_irq(12);
	return;
}

void _inthandler21(int *esp) {		/*键盘中断*/
	unsigned short data;
	_io_out8(PIC0_OCW2, 0x61);
	data = _io_in8(PORT_KEYDAT);
	fifo16_put(keyfifo, 256);
	//fifo16_put(keyfifo, data + 256);
	return;
}

void _inthandler27(int *esp) {
	_io_out8(PIC0_OCW2, 0x67);
	return;
}

void _inthandler2c(int *esp) {		/*鼠标中断*/
	unsigned short data;
	_io_out8(PIC1_OCW2, 0x64);
	_io_out8(PIC0_OCW2, 0x62);
	data = _io_in8(PORT_KEYDAT);
	fifo16_put(mousefifo, data + 512);
	return;
}
