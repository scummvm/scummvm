prog segment para
     assume cs:prog

deb proc near
       push ds
       push bp
       mov bp,sp

       mov ax,[bp+10]   ; segment de depart
       mov ds,ax
       mov si,[bp+8]   ; offset de depart
       mov dx,[bp+6]   ; nombre de secteurs de 128 octets

 et01: mov cx,64

 et02: mov al,[si]
       xor ah,ah
       sub ax,128
       shl ax,1
       add ax,128
       test ax,8000h
       jz et03
       xor ax,ax
       jmp et05

 et03: or ah,ah
       jz et05
       mov ax,255

 et05: mov [si],al
       inc si
       jnz et04
       push ds
       pop bx
       add bx,1000h
       mov ds,bx
 et04: loop et02

       dec dx
       jnz et01

       mov sp,bp
       pop bp
       pop ds
       ret 6
deb endp
prog ends
     end deb





