;x64 Length Disassembler
;Copyright (C) 2020 Slek
;
;rsi (in)  - pointer to opcode
;eax (out) - opcode length or 0xffffffff if error

;format PE64 GUI 5.0
;entry start

;section '.text' code readable executable

use64

start:
    jmp LD64

align 8
LD64_tables:
prefix_t     dq 0x4040404000000000, 0x000000f00000ffff, 0x0000000000000000, 0x000d000000000000
supported_t  dq 0x3f3f3f3f3f3fbf3f, 0xffffff08ffff0000, 0xfffffffffbfffffb, 0xffe0fbffff8fbfcf
modrm_t      dq 0x0f0f0f0f0f0f0f0f, 0x00000a0800000000, 0x000000000000fffb, 0xc0c00000ff0f00c3
data0_t      dq 0x0f0f0f0f0f0f0f0f, 0x0000f008ffff0000, 0x0000fcf0fbfffff0, 0xff20f000ff8f9a08
data1_t      dq 0x1010101010101010, 0xffff0c0000000000, 0x00ff010000000009, 0x000008ff00002043

supported2_t dq 0x00000000800028a0, 0x000000000000ffff, 0xfcfffb3fffffffff, 0x000000000000ff83
modrm2_t     dq 0x0000000080002000, 0x000000000000ffff, 0xfcfff838ffff0000, 0x0000000000000083

LD64:
;---------------Initial adjustment----------------
    push rcx
    push rdx
    push rbx
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12

    lea rbp,[rip+LD64_tables-.rip]
  .rip:
    xor eax,eax
    xor r8d,r8d
    xor r9d,r9d
    mov r10w,4
    mov r11w,8
    xor r12d,r12d
    push rsi

;---------------Prefixes processing---------------
  .prefix:
    lodsb
    mov ebx,eax
    shr bl,6
    mov cl,al
    and ecx,0x3f
    mov rdx,[rbp+prefix_t-LD64_tables+8*rbx]
    bt rdx,rcx
    jnc .opcode

    ;check REX.W
    mov dl,al
    and dl,0xf8
    cmp dl,0x48
    sete r9b
    jne @f
    mov r10b,4
    jmp .prefix
  @@:
    ;check prefix 66
    mov dx,2
    cmp al,0x66
    cmove r10w,dx
    
    ;check prefix 67
    shl dl,1
    cmp al,0x67
    cmove r11w,dx
    jmp .prefix

;--------------Opcode type checking---------------
  .opcode:
    cmp al,0x0f
    je .two_byte

    ;one_byte
    mov rdx,[rbp+supported_t-LD64_tables+8*rbx]
    bt rdx,rcx
    jnc .error

    ;check modrm
    mov rdx,[rbp+modrm_t-LD64_tables+8*rbx]
    bt rdx,rcx
    setc r8b

    ;check test
    mov dl,al
    and dl,0xfe
    cmp dl,0xf6
    jne @f
    mov dl,byte [rsi]
    and dl,0x30
    jnz .modrm
    xor edx,edx
    inc dl
    bt ax,0
    cmovc r12w,r10w
    cmovnc r12w,dx
    jmp .modrm
  @@:
    ;check data0
    mov rdx,[rbp+data0_t-LD64_tables+8*rbx]
    bt rdx,rcx
    jc .modrm

    ;check data1
    mov rdx,[rbp+data1_t-LD64_tables+8*rbx]
    bt rdx,rcx
    jnc @f
    inc r12b
    jmp .modrm
  @@:
    ;check data48
    test r9b,r9b
    jz @f
    mov dl,al
    and dl,0xf8
    cmp dl,0xb8
    jne @f
    mov r12b,8
    jmp .modrm
  @@:
    ;check data66
    mov dl,al
    and dl,0xc7
    cmp dl,0x05
    je @f
    mov dl,al
    and dl,0xf8
    cmp dl,0xb8
    je @f
    mov dl,al
    and dl,0xfe
    cmp dl,0x68
    je @f
    cmp al,0x81
    je @f
    cmp al,0xa9
    je @f
    cmp al,0xc7
    jne .check_mem67
  @@:
    mov r12b,r10b
    jmp .modrm

  .check_mem67:
    mov dl,al
    and dl,0xfc
    cmp dl,0xa0
    jne @f
    mov r12b,r11b
    jmp .modrm
  @@:
    ;check data4
    mov dl,al
    and dl,0xfe
    cmp dl,0xe8
    jne @f
    mov r12b,4
    jmp .modrm
  @@:
    ;check data2
    mov dl,al
    and dl,0xf7
    cmp dl,0xc2
    jne @f
    mov r12b,2
    jmp .modrm
 @@:
    ;check_enter
    mov cl,3
    cmp al,0xc8
    cmove r12w,cx
    jmp .modrm

  .two_byte:
    lodsb
    mov ebx,eax
    shr bl,6
    mov cl,al
    and ecx,0x3f
    mov rdx,[rbp+supported2_t-LD64_tables+8*rbx]
    bt rdx,rcx
    jnc .error

    ;check modrm
    mov rdx,[rbp+modrm2_t-LD64_tables+8*rbx]
    bt rdx,rcx
    setc r8b

    ;check_data42
    mov dl,al
    and dl,0xf0
    cmp dl,0x80
    jne @f
    mov r12b,4
    jmp .modrm
  @@:
    ;check data1
    mov cl,1
    mov dl,al
    and dl,0xf7
    cmp dl,0xa4
    cmove r12w,cx
    cmp al,0xba
    cmove r12w,cx
    jmp .modrm

;----------------Setting the error----------------
  .error:
    pop rsi
    xor eax,eax
    dec eax
    jmp .exit

;--------------ModR/M byte processing-------------
  .modrm:
    test r8b,r8b ;modrm flag
    jz .compute_length

    ;parse modrm
    lodsb
    mov cl,al
    and cl,0xc0 ;mod
    cmp cl,0xc0
    je .compute_length

    ;disp8
    cmp cl,0x40
    jne @f
    inc r12b
    jmp .check_sib
  @@:
    ;disp32
    cmp cl,0x80
    jne .check_sib
    add r12b,4

  .check_sib:
    and al,0x07 ;rm
    cmp al,0x04
    jne @f
    lodsb
    and al,0x07
  @@:
    ;check rip+disp32
    test cl,cl
    jnz .compute_length
    lea ecx,[r12+4]
    cmp al,0x05
    cmove r12w,cx

;-----------Command length calculation------------
  .compute_length:
    lea rax,[rsi+r12]
    pop rsi
    sub rax,rsi

;---------Restore the registers and exit----------
  .exit:
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    ;ret
