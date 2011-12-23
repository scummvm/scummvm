prog segment para
     assume cs:prog
     org 100h

music proc near

debut: push bp
       push ds
       mov bp,sp

       push ds
       pop es
       mov ax,5000h
       mov ds,ax
       add [bp+8],ax
       mov ax,[bp+6]
       sub ax,3
       mov [bp+6],ax      ; voir plus loin =>  loop 3 en plus
       xor si,si

       cli
       in al,61h
       or al,3
       out 61h,al

bouc:  mov bl,[si]
       inc si
       xor bh,bh
       neg bx
       add bx,255
       shl bx,1
       add bx,10
       mov di,bx

       mov dx,[bp+di]

       mov al,0B6h
       out 43h,al

       mov al,dl
       out 42h,al
       mov al,dh
       out 42h,al

suit:  mov cx,[bp+6]
boucl: loop boucl

       cmp si,16
       jz nattent
       mov cx,3
 rien: loop rien
       jmp bouc

nattent: xor si,si
       mov ax,ds
       inc ax
       mov ds,ax
       cmp ax,[bp+8]
       jc bouc

       in al,61h
       and al,0FCh
       out 61h,al
       sti

       pop ds
       pop bp
       ret 516
music endp
prog ends
     end music



