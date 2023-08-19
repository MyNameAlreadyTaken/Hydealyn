; naskfunc
; TAB=4

;[FORMAT "WCOFF"]				
;[INSTRSET "i486p"]				
;[BITS 32]						
;[FILE "naskfunc.nas"]			

INT_VECTOR_SYS_CALL equ 0x90
_NR_sendrec	equ	1
INT_M_CTL	equ	0x20	; I/O port for interrupt controller         <Master>
INT_M_CTLMASK	equ	0x21	; setting bits in this port disables ints   <Master>
INT_S_CTL	equ	0xA0	; I/O port for second interrupt controller  <Slave>
INT_S_CTLMASK	equ	0xA1	; setting bits in this port disables ints   <Slave>

EOI		equ	0x20

		GLOBAL	do_hdisk, unexpected_hdisk_interrupt

		GLOBAL	_io_delay, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8, _io_in16, _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_port_read, _port_write
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_io_hlt, _write_mem8
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler20, _asm_inthandler21, _asm_inthandler2c, _asm_inthandler2e, _asm_inthandler27
		EXTERN	_inthandler20, _inthandler21, _inthandler2c, _inthandler2e, _inthandler27
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_load_tr
		GLOBAL	_memtest_sub
		GLOBAL	_farjmp
		GLOBAL	_sendrec
		;GLOBAL	_asm_sys_putchar
		global	_disable_irq, _enable_irq

;[SECTION .text]

_io_delay:
	NOP
	NOP
	NOP
	NOP
	RET

_io_cli:		;void _io_cli(void) 关中断
	CLI
	RET

_io_sti:		;void _io_sti(void) 开中断
	STI
	RET

_io_stihlt:		;void _io_stihlt(void) 开中断后执行一个空指令
	STI
	HLT
	RET

_io_in8:		;int _io_in8(int port)
	MOV	EDX, [ESP + 4]
	MOV	EAX, 0
	IN	AL, DX
	NOP
	NOP
	RET

_io_in16:		;int _io_in16(int port)
	MOV	EDX, [ESP + 4]
	MOV	EAX, 0
	IN	AX, DX
	RET

_io_in32:		;int _io_in32(int port)
	MOV	EDX, [ESP + 4]
	IN	EAX, DX
	RET

_io_out8:		; void _io_out8(int port, int data)
	MOV	EDX, [ESP + 4]
	;MOV	EAX, [ESP + 8]
	;OUT	DX, AX
	MOV	AL, [ESP + 8]
	OUT	DX, AL
	NOP
	NOP
	RET

_io_out16:	; void _io_out16(int port, int data)
	MOV	EDX, [ESP + 4]
	MOV	EAX, [ESP + 8]
	OUT 	DX, AX
	RET

_io_out32:	; void _io_out32(int port, int data)
	MOV	EDX, [ESP + 4]
	MOV 	EAX, [ESP + 8]
	OUT	DX, EAX
	RET

_port_read:	; void _port_read(unsigned short port, void *buf, int n)
	MOV	EDX, [ESP + 4]		; port
	MOV	EDI, [ESP + 4 + 4]		; buf
	MOV	ECX, [ESP + 4 + 4 + 4]	; n
	;SHR	ECX, 1
	CLD				; 将DF清零
	REP	INSW			; 重复输入字符串
	RET

_port_write:	; void _port_write(unsigned short port, void *buf, int n)
	MOV	EDX, [ESP + 4]
	MOV	ESI, [ESP + 4 + 4]
	MOV	ECX, [ESP + 4 + 4 + 4]
	;SHR	ECX, 1
	CLD
	REP	OUTSW
	RET

_io_load_eflags:	; int _io_load_eflags(void)
	PUSHFD
	POP	EAX
	RET

_io_store_eflags:	; void _io_store_eflags(int eflags)
	MOV	EAX, [ESP + 4]
	PUSH	EAX
	POPFD
	RET
	

_io_hlt:	; void _io_hlt(void);空指令
		HLT
		RET

_write_mem8:	; void _write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		;地址读入ECX
		MOV		AL,[ESP+8]		;数据读入AL
		MOV		[ECX],AL
		RET

_load_gdtr:	; void _load_gdtr(int limit, int addr)
	MOV	AX, [ESP + 4]
	MOV	[ESP + 6], AX
	LGDT	[ESP + 6]
	RET

_load_idtr:	; void _load_idtr(int limit, int addr)
	MOV	AX, [ESP + 4]
	MOV	[ESP + 6], AX
	LIDT	[ESP + 6]
	RET

_asm_inthandler20:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL	_inthandler20
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler21:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL	_inthandler21
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler27:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL	_inthandler27
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler2c:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL	_inthandler2c
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler2e:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL	_inthandler2e
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_load_cr0:		; int _load_cr0(void)
	MOV	EAX, CR0
	RET

_store_cr0:	; void _store_cr0(int cr0)
	MOV	EAX, [ESP + 4]
	MOV	CR0, EAX
	RET

_load_tr:		; void _load_tr(int tr)
	LTR	[ESP + 4]
	RET

_memtest_sub:	; unsigned int _memtest_sub(unsigned int start, unsigned int end)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV	ESI, 0xAA55AA55
	MOV	EDI, 0x55AA55AA
	MOV	EAX, [ESP + 12 + 4]
mts_loop:
	MOV	EBX, EAX
	ADD	EBX, 0xffc
	MOV	EDX, [EBX]
	MOV	[EBX], ESI
	XOR	DWORD [EBX], 0xFFFFFFFF
	CMP	EDI, [EBX]
	JNE	mts_fin
	XOR	DWORD [EBX], 0xFFFFFFFF
	CMP	ESI, [EBX]
	JNE	mts_fin
	MOV	[EBX], EDX
	ADD	EAX, 0x1000
	CMP	EAX, [ESP + 12 + 8]
	JBE	mts_loop
	POP	EBX
	POP	ESI
	POP	EDI
	RET
mts_fin:
	MOV	[EBX], EDX
	POP	EBX
	POP	ESI
	POP	EDI
	RET

_farjmp:		; void _farjmp(int eip, int cs)
	JMP	FAR [ESP + 4]
	RET

;_asm_sys_putchar:
	;STI
	;PUSH	1
	;AND	EAX, 0xFF
	;PUSH	EAX
	;PUSH
	;CALL	_sys_putchar
	;ADD	ESP, 12
	;IRETD

_sendrec:
	MOV	EAX, _NR_sendrec
	MOV	EBX, [ESP + 4]	; function
	MOV	ECX, [ESP + 8]	; src_dest
	MOV	EDX, [ESP + 12]	; p_msg
	INT	INT_VECTOR_SYS_CALL
	RET

;		   void _disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
;	}
_disable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, 1
	rol	ah, cl			; ah = (1 << (irq % 8))
	cmp	cl, 8
	jae	disable_8		; disable irq >= 8 at the slave 8259
disable_0:
	in	al, INT_M_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_M_CTLMASK, al	; set bit at master 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
disable_8:
	in	al, INT_S_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_S_CTLMASK, al	; set bit at slave 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
dis_already:
	popf
	xor	eax, eax		; already disabled
	ret

;		   void _enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;	}
;
_enable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, ~1
	rol	ah, cl			; ah = ~(1 << (irq % 8))
	cmp	cl, 8
	jae	enable_8		; enable irq >= 8 at the slave 8259
enable_0:
	in	al, INT_M_CTLMASK
	and	al, ah
	out	INT_M_CTLMASK, al	; clear bit at master 8259
	popf
	ret
enable_8:
	in	al, INT_S_CTLMASK
	and	al, ah
	out	INT_S_CTLMASK, al	; clear bit at slave 8259
	popf
	ret
	
