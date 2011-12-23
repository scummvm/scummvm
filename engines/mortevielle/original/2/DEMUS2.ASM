
prog segment para
     assume cs:prog

deb proc near
       push ds
       push bp
       mov bp,sp
       sub sp,4

       mov ax,[bp+10]
       mov ds,ax
       mov ax,[bp+8]
       mov es,ax
       xor si,si
       xor di,di

       mov ax,[bp+6]
       shl ax,1
       mov [bp-2],ax
       call et00
tab  dw  -96, -72, -48, -32, -20, -12, -8, -4, 0, 4, 8, 12, 20, 32, 48, 72
 et00: pop ax
       mov [bp-4],ax
       mov dx,80h

 et01: mov cx,64

 et02: push cx
       xor ax,ax
       xor bx,bx
       mov al,[si]
       inc si
       jnz et04
       push ds
       pop bx
       add bx,1000h
       mov ds,bx
       xor bx,bx

 et04: push si
       push ds
       push cs
       pop ds
       mov bl,al

       mov cl,4
       shr ax,cl
       shl ax,1
       add ax,[bp-4]
       mov si,ax
       add dx,[si]
       mov es:[di],dl
       inc di

       and bl,15
       shl bx,1
       add bx,[bp-4]
       mov si,bx
       add dx,[si]
       mov es:[di],dl
       inc di
       jnz et03
       push es
       pop ax
       add ax,1000h
       mov es,ax
       xor ax,ax

 et03: pop ds
       pop si
       pop cx
       loop et02

       dec word ptr [bp-2]
       jnz et01

       mov sp,bp
       pop bp
       pop ds
       ret 6
deb endp
prog ends
     end deb





