; haribote-os boot asm
; TAB=4

;[INSTRSET "i486p"]

ORG		0xc200		
jmp		LABEL_START

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit

;	0x100 :  640 x  400 x 8bit
;	0x101 :  640 x  480 x 8bit
;	0x103 :  800 x  600 x 8bit
;	0x105 : 1024 x  768 x 8bit
;	0x107 : 1280 x 1024 x 8bit

BOTPAK	EQU		0x00280000		
DSKCAC	EQU		0x00100000		
DSKCAC0	EQU		0x00008000		

CYLS	EQU		0x0ff0			
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			
SCRNX	EQU		0x0ff4			
SCRNY	EQU		0x0ff6			
VRAM	EQU		0x0ff8

DA_32		EQU	4000h	; 32 位段
DA_LIMIT_4K	EQU	8000h	; 段界限粒度为 4K 字节

DA_DPL0		EQU	  00h	; DPL = 0
DA_DPL1		EQU	  20h	; DPL = 1
DA_DPL2		EQU	  40h	; DPL = 2
DA_DPL3		EQU	  60h	; DPL = 3

DA_DR		EQU	90h	; 存在的只读数据段类型值
DA_DRW		EQU	92h	; 存在的可读写数据段属性值
DA_DRWA		EQU	93h	; 存在的已访问可读写数据段类型值
DA_C		EQU	98h	; 存在的只执行代码段属性值
DA_CR		EQU	9Ah	; 存在的可执行可读代码段属性值
DA_CCO		EQU	9Ch	; 存在的只执行一致代码段属性值
DA_CCOR		EQU	9Eh	; 存在的可执行可读一致代码段属性值

DA_LDT		EQU	  82h	; 局部描述符表段类型值
DA_TaskGate	EQU	  85h	; 任务门类型值
DA_386TSS	EQU	  89h	; 可用 386 任务状态段类型值
DA_386CGate	EQU	  8Ch	; 386 调用门类型值
DA_386IGate	EQU	  8Eh	; 386 中断门类型值
DA_386TGate	EQU	  8Fh	; 386 陷阱门类型值

SA_RPL0		EQU	0	; ┓
SA_RPL1		EQU	1	; ┣ RPL
SA_RPL2		EQU	2	; ┃
SA_RPL3		EQU	3	; ┛

SA_TIG		EQU	0	; ┓TI
SA_TIL		EQU	4	; ┛

PG_P		EQU	1	; 页存在属性位
PG_RWR		EQU	0	; R/W 属性位值, 读/执行
PG_RWW		EQU	2	; R/W 属性位值, 读/写/执行
PG_USS		EQU	0	; U/S 属性位值, 系统级
PG_USU		EQU	4	; U/S 属性位值, 用户级

%macro Descriptor 3
	dw	%2 & 0FFFFh				; 段界限 1				(2 字节)
	dw	%1 & 0FFFFh				; 段基址 1				(2 字节)
	db	(%1 >> 16) & 0FFh			; 段基址 2				(1 字节)
	dw	((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)	; 属性 1 + 段界限 2 + 属性 2		(2 字节)
	db	(%1 >> 24) & 0FFh			; 段基址 3				(1 字节)
%endmacro ; 共 8 字节

BaseOfLoader		equ	 0x00c00	; LOADER.BIN 被加载到的位置 ----  段地址
OffsetOfLoader		equ	  0x0200	; LOADER.BIN 被加载到的位置 ---- 偏移地址

BaseOfLoaderPhyAddr	equ	BaseOfLoader * 0x10	; LOADER.BIN 被加载到的位置 ---- 物理地址 (= BaseOfLoader * 10h)


BaseOfKernelFile	equ	 0x28000	; KERNEL.BIN 被加载到的位置 ----  段地址
OffsetOfKernelFile	equ	     0x0	; KERNEL.BIN 被加载到的位置 ---- 偏移地址

BaseOfKernelFilePhyAddr	equ	BaseOfKernelFile * 0x10
KernelEntryPointPhyAddr	equ	0x0300400	; 注意：1、必须与 MAKEFILE 中参数 -Ttext 的值相等!!
					;       2、这是个地址而非仅仅是个偏移，如果 -Ttext 的值为 0x400400，则它的值也应该是 0x400400。

LABEL_GDT:			Descriptor             0,                    0, 0						; 空描述符
LABEL_DESC_FLAT_C:		Descriptor             0,              0fffffh, DA_CR  | DA_32 | DA_LIMIT_4K			; 0 ~ 4G
LABEL_DESC_FLAT_RW:		Descriptor             0,              0fffffh, DA_DRW | DA_32 | DA_LIMIT_4K			; 0 ~ 4G
LABEL_DESC_VIDEO:		Descriptor	 0B8000h,               0ffffh, DA_DRW                         | DA_DPL3	; 显存首地址

GdtLen		equ	$ - LABEL_GDT
GdtPtr		dw	GdtLen - 1				; 段界限
		dd	BaseOfLoaderPhyAddr + LABEL_GDT		; 基地址

SelectorFlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3

		ALIGN	16, DB	0
GDT0:
		TIMES	8	DB	0
		DW		0xffff,0x0000,0x9200,0x00cf	
		DW		0xffff,0x0000,0x9a30,0x0047

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGN	16,	DB 0

TopOfStack	equ	0x390000

LABEL_START:
		MOV		AX, 0x0800
		MOV		DS, AX
		MOV		AH, 0x03
		XOR		BH, BH
		INT		0x10
		MOV		[0], DX
; get memory size
		MOV		AH, 0x88
		INT		0x15
		MOV		[2], AX
; get video-card data
		MOV		AH, 0x0f
		INT		0x10
		MOV		[4], BX
		MOV		[6], AX

		MOV		AH, 0x12
		MOV		BL, 0x10
		INT		0x10
		MOV		[8], AX
		MOV		[10], BX
		MOV		[12], CX
; get hd0 data
		MOV		AX, 0x0000
		MOV		DS, AX
		LDS		SI, [4 * 0x41]
		MOV		AX, 0x0800
		MOV		ES, AX
		MOV		DI, 0x0080
		MOV		CX, 0x10
		REP
		MOVSB
; get hd1 data
		MOV		AX, 0x0000
		MOV		DS, AX
		LDS		SI, [4 * 0x46]
		MOV		AX, 0x0800
		MOV		ES, AX
		MOV		DI, 0x0090
		MOV		CX, 0x10
		REP
		MOVSB

; check hd1
		MOV		AX, 0x01500
		MOV		DL, 0x81
		INT		0x13
		JC		no_hdisk1
		CMP		AH, 3
		JE		is_hdisk1
no_hdisk1:
		MOV		AX, 0x0800
		MOV		ES, AX
		MOV		DI, 0x0090
		MOV		CX, 0x10
		MOV		AX, 0x00
		REP
		STOSB
is_hdisk1:

		MOV		AX, 0
		MOV		DS, AX

		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]
		AND		AX,0x0080
		JZ		scrn320

		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13			
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL


		MOV		AL,0xff
		OUT		0x21,AL
		NOP						
		OUT		0xa1,AL
		CLI						

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf		;enable A20		
		OUT		0x60,AL
		CALL	waitkbdout
	
		;lgdt	[GdtPtr]

		LGDT	[GDTR0]			;设定临时GDT
		cli
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	;设bit31为0 禁止分页	
		OR		EAX,0x00000001	;设bit0为1 切换到保护模式
		MOV		CR0,EAX
		
		JMP		LABEL_PM_START

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		
		RET


LABEL_PM_START:
		MOV		EAX, 1*8		;可读写的段 32bit	
		MOV		DS, EAX
		MOV		ES, EAX
		MOV		FS, EAX
		MOV		GS, EAX
		MOV		SS, EAX
		mov	esp, TopOfStack

		MOV		ESI,bootpack	;转送源
		MOV		EDI,BOTPAK	;转送目的地 0x280000
		MOV		ECX,512*1024/4
		;rep	movsd
		call	memcpy

		; memcpy(bootpack, BOTPAK, 512 * 1024 / 4);
		
		;mov	eax, 0x7c00
		;mov	ebx, DSKCAC
		;mov	esi, [
		MOV		ESI,0x7c00		
		MOV		EDI,DSKCAC	;0x100000
		MOV		ECX,512/4
		;rep	movsd
		call	memcpy
		; memcpy(0x7c00, DSKCAC, 512 / 4);

		MOV		ESI,DSKCAC0+512	;0x8000 + 512
		MOV		EDI,DSKCAC+512	
		MOV		ECX,10
		;MOV		ECX, dword [CYLS]
		IMUL	ECX,512*18*2/4	
		SUB		ECX,512/4
		;rep	movsd				;ds:si -> es:di
		call	memcpy
		; memcpy(DSKCAC0 + 512, DSKCAC + 512, cyls * 512 * 18 * 2 / 4 - 512 / 4);


		;jmp	dword 2*8:kernel_init
		
		mov	ebx, BaseOfKernelFilePhyAddr
		;mov	eax, [ebx]
		mov	al, byte [ebx + 400h]
		cmp	al, 0xf3
		;cmp	eax, 0x464c457f
;.die:
;	hlt
;	jmp	.die
		je	not
not_elf:
	hlt
	jmp	not_elf


memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4 
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy

		;cmp	esi, bootpack
		;je	mem_out1
		;cmp	esi, 0x7c00
		;je	mem_out2
		;cmp	esi, DSKCAC0 + 512
		;je	mem_out3
		;jmp	skip	
		RET
; ------------------------------------------------------------------------

;[SECTION .s32]

;ALIGN	32

;[BITS	32]

not:
		call init_kernel

		;call	SetupPaging
		;mov	esp, TopOfStack

		mov	ebx, 0x300000
		;mov	al, byte [ebx + 400h]
		;cmp	al, 0xf3
		mov	eax, [ebx + 400h]
		cmp	eax, 0xfb1e0ff3
		je	yes
die:
	hlt
	jmp	die
yes:
		;jmp	0x300400
		jmp	dword 2*8:0x00000400


		;JMP		DWORD 2*8:0x0000001b

init_kernel:
	xor	eax, eax
	xor	ebx, ebx
	xor	ecx, ecx
	xor	edx, edx
	
	mov	edx, BaseOfKernelFilePhyAddr
	mov	eax, [edx + 28]
	add	eax, edx
	mov	bx, [edx + 44]		;number of segments
	
.one_seg:
	cmp	byte [eax], 0
	je	.finish
	
	mov	esi, [eax + 4]
	add	esi, edx
	mov	edi, [eax + 8]
	mov	ecx, [eax + 16]
	cmp	ecx, 0
	jz	.finish
.cpy:
	push	eax
	mov	al, byte [ds:esi]
	add	esi, 1
	mov	byte [es:edi], al
	add	edi, 1
	sub	ecx, 1
	pop	eax
	cmp	ecx, 0
	jnz	.cpy
	;cld
	;rep	movsb

.finish:
	add	eax, 020h
	sub	bx, 1
	cmp	bx, 0
	jnz	.one_seg

	ret

MemCpy:
	cld
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
	;rep	movsb

.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环

.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	;jmp	MemCpy_finished
	ret			; 函数结束，返回
; MemCpy 结束-------------------------------------------------------------

; --------------------------------------------------------------------------------------------
InitKernel:
	;cld
	xor	ebx, ebx
        xor   esi, esi
        xor	edx, edx
        xor	ecx, ecx
        mov	ebx, BaseOfKernelFilePhyAddr
        mov   cx, word [ebx + 2Ch];`. ecx <- pELFHdr->e_phnum
        movzx ecx, cx                               ;/
        mov   esi, [ebx + 1Ch]  ; esi <- pELFHdr->e_phoff
        add   esi, ebx;	esi<-OffsetOfKernel+pELFHdr->e_phoff
        mov	dx, [ebx + 42]

Begin:
        mov   eax, [esi + 0]
        cmp   eax, 0                      ; PT_NULL
        jz    NoAction
        push  dword [esi + 010h]    ;size ;`.
        mov   eax, [esi + 04h]            ; |
        add   eax, ebx; | memcpy((void*)(pPHdr->p_vaddr),
        push  eax		    ;src  ; |      uchCode + pPHdr->p_offset,
        push  dword [esi + 08h]     ;dst  ; |      pPHdr->p_filesz;
        call  MemCpy                      ; |

;MemCpy_finished:
        add   esp, 12                     ;/
NoAction:
        add   esi, edx                   ; esi += pELFHdr->e_phentsize
        dec   ecx
        jnz   Begin

        ret
        ;jmp	InitKernel_finished
; InitKernel ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

kernel_init:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
	
	mov	edx, BaseOfKernelFilePhyAddr
    ;mov dx, [BaseOfKernelFilePhyAddr + 42]
    mov ebx, [edx + 28]

    add ebx, edx
    mov cx, [edx + 44]

.each_segment:
    cmp dword [ebx], 0
    je .PTNULL

    ; 准备mem_cpy参数
    push dword [ebx + 16]
    mov eax, [ebx + 4]
    add eax, edx
    push eax
    push dword [ebx + 8]
    call mem_cpy
    add esp, 12

.PTNULL:
    add ebx, 020h
    loop .each_segment
    ret

mem_cpy:
    cld
    push ebp
    mov ebp, esp
    push ecx

    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    rep movsb

    pop ecx
    pop ebp
    ret

		ALIGN	16,	DB 0
bootpack:
