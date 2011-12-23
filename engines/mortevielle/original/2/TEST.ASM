
prog segment para
     assume cs:prog,ds:prog

debut: mov ax,5000h
       mov ds,ax
       xor di,di

       in al,61h
       or al,3
       out 61h,al
       mov cx,1000
       cli
       mov al,00110000b
       out 43h,al
       mov ax,0
       out 40h,al
       xchg ah,al
       out 40h,al

bouc:
       mov al,0
       out 43h,al
       mov al,00110000b
       out 43h,al
       in al,40h
       mov ah,al
       in al,40h
       mov [di],ax
       inc di
       inc di
       loop bouc


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
       mov al,0
       mov ah,4Ch
       int 21h
prog ends
     end debut


