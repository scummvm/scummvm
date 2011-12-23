prog segment
     assume cs:prog
deb:
     xor di,di
     mov ax,6000h
     mov es,ax
     xor ax,ax
     mov cx,32768
bou: mov es:[di],ax
     inc di
     inc di
     loop bou
     mov ah,4Ch
     int 21h
prog ends
     end deb

