title sauve_ecran_graphique

prog segment para
     assume cs:prog
; Pile : [bp]    = old bp
;        [bp+2]  = old ds
;        [bp+4]  = ret
;        [bp+6]  = dy
;        [bp+8]  = y
;        [bp+10] = Gd

sav proc near
         push ds
         push bp
         mov bp,sp
         push bp
         sub sp,10
         mov ax,6000h
         mov es,ax
         xor ax,ax
         mov [bp-8],ax   ; numero du plan en lecture
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
         mov ds,ax
         mov cx,40
         mov ax,[bp+8]
         mul cl
         mov [bp-4],ax
         mov ax,[bp+6]
         inc ax
         mul cl
         shr ax,1
         mov [bp-6],ax
         xor di,di
         mov dx,3deh
         mov cx,4
  ams02: push cx
         mov ax,[bp-8]
         out dx,al
         inc ax
         mov [bp-8],ax
         mov cx,[bp-6]
         mov si,[bp-4]
        rep movsw
         mov si,[bp-4]
         add si,2000h
         mov cx,[bp-6]
        rep movsw
         pop cx
         loop ams02
         jmp fin

  cga01: mov ax,0b800h
         mov ds,ax
         mov cx,40
         mov ax,[bp+8]
         mul cl
         mov bx,ax
         mov ax,[bp+6]
         inc ax
         mul cl
         xor di,di
         mov cx,ax
         shr cx,1
         mov si,bx
        rep movsw
         mov si,bx
         add si,2000h
         mov cx,ax
        rep movsw
         jmp fin

  ega01: mov ax,0a000h
         mov ds,ax
         mov cx,80
         mov ax,[bp+8]
         mul cl
         mov [bp-4],ax
         mov ax,[bp+6]
         inc ax
         mul cl
         shr ax,1
         mov [bp-6],ax
         xor di,di
         mov dx,3ceh
         mov al,4
         out dx,al
         inc dx
         mov cx,4
  ega02: push cx
         mov ax,[bp-8]
         out dx,al
         inc ax
         mov [bp-8],ax
         mov cx,[bp-6]
         mov si,[bp-4]
        rep movsw
         pop cx
         loop ega02
         jmp fin

  tan01: mov ax,0b800h
         mov ds,ax
         mov ax,[bp+8]
         shr ax,1
         shr ax,1
         mov si,ax
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
         xor di,di
         mov ax,si
  tan02: push cx
         mov si,ax
         add ax,2000h
         mov cx,[bp-2]
        rep movsw
         pop cx
         loop tan02
         jmp fin

  her01: mov ax,0b000h
         mov ds,ax
         mov ax,[bp+8]
         ;shr ax,1
         shr ax,1
         mov bx,80
         mul bx
         mov si,ax
         mov ax,[bp+6]
         ;shr ax,1
         shr ax,1
         add ax,3
         mul bx
         mov [bp-2],ax
         mov dx,4
         xor di,di
         mov ax,si
  her02:
         mov si,ax
         add ax,2000h
         mov cx,[bp-2]
        rep movsw
         dec dx
         jnz her02

    fin: mov sp,bp
         pop bp
         pop ds
         ret 6
sav endp
prog ends
     end sav


