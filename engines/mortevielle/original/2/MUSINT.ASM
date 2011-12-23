
prog segment para
     assume cs:prog,ds:prog

music proc near

       push bp
       push ds
       mov bp,sp
       sub sp,10

       mov ax,5000h
       mov ds,ax
       add [bp+8],ax
       xor si,si

       in al,61h
       or al,3
       out 61h,al
       cli
       ;mov al,00110110b
       ;out 43h,al
       ;in al,40h
       ;mov [bp-2],al
       ;in al,40h
       ;mov [bp-1],al

       mov al,00110000b
       out 43h,al
       mov ax,[bp+6]
       out 40h,al
       xchg ah,al
       out 40h,al

bouc:  mov bl,[si]
       inc si
       xor bh,bh
       neg bx
       add bx,255
       shl bx,1
       add bx,10
       mov di,bx
       mov dx,[bp+di]
  bcl1:
       mov al,0
       out 43h,al
       in al,40h
       in al,40h
       or al,al
       je bcl1

       mov al,0B0h
       out 43h,al
       mov al,dl
       out 42h,al
       mov al,dh
       out 42h,al

       mov al,00110000b
       out 43h,al
       mov ax,[bp+6]
       out 40h,al
       xchg ah,al
       out 40h,al

       cmp si,16
       jz nattent
       jmp bouc

nattent:
       xor si,si
       mov ax,ds
       inc ax
       mov ds,ax
       cmp ax,[bp+8]
       jc bouc

       in al,61h
       and al,0FCh
       out 61h,al

       mov al,00110110b
       out 43h,al
       mov ax,0
       out 40h,al
       xchg ah,al
       out 40h,al
       sti
       mov sp,bp
       pop ds
       pop bp
       ret 516


music endp
prog ends
     end music


