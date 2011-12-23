prog segment para
     assume cs:prog,ds:prog

music proc near

       push bp
       push ds
       mov bp,sp
       sub sp,10

       call interup
       mov ax,5000h
       mov ds,ax
       add [bp+8],ax
       mov bx,[bp+8]
       xor si,si

       mov al,10110000b
       out 43h,al
       xor al,al
       out 42h,al
       out 42h,al

       mov al,10010000b
       out 43h,al
       mov dx,20h

       in al,61h
       or al,3
       out 61h,al
       add bp,10
       xor ah,ah
bouc:  mov al,[si]
       inc si
       mov di,ax
       shl di,1
       mov al,[bp+di]
       sti
       hlt
labint:
       cmp si,16
       jne bouc

nattent:
       xor si,si
       mov cx,ds
       inc cx
       mov ds,cx
       cmp cx,bx
       jc bouc

       in al,61h
       and al,0FCh
       out 61h,al

       sub bp,10
       call finterup

       mov sp,bp
       pop ds
       pop bp
       ret 516

;-----------------------------------------------------------------------------
;                 Mise en place de l'interruption 8
;-----------------------------------------------------------------------------
interup: call intcode
;--------------------  Le code de l'interruption 8  --------------------------
          out 42h,al         ;
          mov al,dl          ; 2 .... 20h = end interrupt
          out dx,al          ;
          add sp,6           ;
          jmp labint
;-------------------  Fin du code de l'interruption  -------------------------

intcode: pop cx      ; recupere le decalage du code de l'interruption
         xor bx,bx
         mov ds,bx
         mov bx,32
         mov ax,[bx]
         mov dx,[bx+2]
         mov [bp-4],ax
         mov [bp-6],dx
         cli
         mov [bx],cx
         mov [bx+2],cs
         mov al,00110110b
         out 43h,al
         mov ax,[bp+6]
         out 40h,al
         xchg ah,al
         out 40h,al
         mov al,0FEh
         out 21h,al
         ret
;-----------------------------------------------------------------------------
;                       Sortie de l'interruption 8
;-----------------------------------------------------------------------------
finterup: cli
          xor ax,ax
          mov ds,ax
          mov bx,32
          mov ax,[bp-4]
          mov dx,[bp-6]
          mov [bx],ax
          mov [bx+2],dx
          mov al,00110110b
          out 43h,al
          xor ax,ax
          out 40h,al
          xchg ah,al
          out 40h,al
          out 21h,al
          sti
          ret
music endp
prog ends
     end music

