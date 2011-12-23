title recharge_ecran_graphique

prog segment para
     assume cs:prog
; Pile : [bp]    = old bp
;        [bp+2]  = old ds
;        [bp+4]  = ret
;        [bp+6]  = dy
;        [bp+8]  = y
;        [bp+10] = Gd

char proc near
         push ds
         push bp
         mov bp,sp
         push bp
         sub sp,10
         mov ax,6000h
         mov ds,ax
         mov ax,1
         mov [bp-8],ax   ; numero du plan en ecriture
         mov ax,[bp+10]
         or ax,ax
         jz ams01
         dec ax
         jz cga01
         dec ax
         jz Gdega
         dec ax
         jz Gdher
         dec ax
         jz Gdtan
         jmp fin

  Gdega: jmp ega01
  Gdher: jmp her01
  Gdtan: jmp tan01

  ams01: mov ax,0b800h
         mov es,ax
         mov cx,40
         mov ax,[bp+8]
         mul cl
         mov [bp-4],ax
         mov ax,[bp+6]
         inc ax
         mul cl
         shr ax,1
         mov [bp-6],ax
         xor si,si
         mov dx,3ddh
         mov cx,4
  ams02: push cx
         mov ax,[bp-8]
         out dx,al
         shl ax,1
         mov [bp-8],ax
         mov cx,[bp-6]
         mov di,[bp-4]
        rep movsw
         mov di,[bp-4]
         add di,2000h
         mov cx,[bp-6]
        rep movsw
         pop cx
         loop ams02
         jmp fin

  cga01: mov ax,0b800h
         mov es,ax
         mov cx,40
         mov ax,[bp+8]
         mul cl
         mov bx,ax
         mov ax,[bp+6]
         inc ax
         mul cl
         xor si,si
         mov cx,ax
         shr cx,1
         mov di,bx
        rep movsw
         mov di,bx
         add di,2000h
         mov cx,ax
        rep movsw
         jmp fin

  ega01: mov ax,0a000h
         mov es,ax
         mov cx,80
         mov ax,[bp+8]
         mul cl
         mov [bp-4],ax
         mov ax,[bp+6]
         inc ax
         mul cl
         shr ax,1
         mov [bp-6],ax
         xor si,si
         mov dx,3ceh
         mov ax,0FF08h
         out dx,al
         inc dx
         xchg ah,al
         out dx,al
         mov dl,0c4h
         mov al,2
         out dx,al
         inc dx
         mov cx,4
  ega02: push cx
         mov ax,[bp-8]
         out dx,al
         shl ax,1
         mov [bp-8],ax
         mov cx,[bp-6]
         mov di,[bp-4]
        rep movsw
         pop cx
         loop ega02
         jmp fin

  tan01: mov ax,0b800h
         mov es,ax
         mov ax,[bp+8]
         shr ax,1
         shr ax,1
         mov di,ax
         mov bx,160
         mul bx
         mov cx,[bp+6]
         shr cx,1
         shr cx,1
         add cx,4
         mov ax,80
         mul cx
         mov [bp-2],ax
         mov cx,4
         xor si,si
         mov ax,di
  tan02: push cx
         mov di,ax
         add ax,2000h
         mov cx,[bp-2]
        rep movsw
         pop cx
         loop tan02
         jmp fin

    her01: mov ax,0b000h
         mov es,ax
         mov ax,[bp+8]
         ;shr ax,1
         shr ax,1
         mov bx,80
         mul bx
         mov di,ax
         mov ax,[bp+6]
         ;shr ax,1
         shr ax,1
         add ax,3
         mul bx
         mov [bp-2],ax
         mov dx,4
         xor si,si
         mov ax,di
  her02:
         mov di,ax
         add ax,2000h
         mov cx,[bp-2]
        rep movsw
         dec dx
         jnz her02

    fin: mov sp,bp
         pop bp
         pop ds
         ret 6
char endp
prog ends
     end char


