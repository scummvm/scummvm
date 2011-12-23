TITLE affiche_un_point

prog segment para
     assume cs:prog,ds:prog
afpoint proc near
deb: push ds
       push bp
       mov bp,sp
       sub sp,20

;       *********  PARAMETRES  ********
; Pile (bp) :
;             [bp]    = old bp
;             [bp+2]  = old ds
;             [bp+4]  = retour
;             [bp+6]  = couleur
;             [bp+8]  = y
;             [bp+10] = x
;             [bp+12] = Graph Device  ( 0 asm, 1 cga, 2 ega, 3 her )


         mov bx,[bp+12]
         or bx,bx
         jz Gdams
         dec bx
         jz Gdcga
         dec bx
         jz Gdega
         dec bx
         jz Gdher
         jmp fin

   ctrl: jmp fin
  Gdams: jmp ams01
  Gdcga: jmp cga01
  Gdega: jmp ega01
  Gdher: jmp her01

  ams01:
         mov cx,[bp+10]
         and cx,7
         mov ax,80h
         shr ax,cl
         mov [bp-2],ax       ; Bit Mask
         not al
         mov [bp-1],al

         mov ax,0B800h
         mov ds,ax
         mov ax,[bp+8]
         mov bl,40
         push ax
         and ax,0FFFEh
         mul bl
         mov di,ax
         pop ax
         test ax,1
         jz ams02
         add di,2000h

  ams02: mov ax,[bp+10]
         mov cl,3
         shr ax,cl
         add di,ax
         mov bx,[bp+6]


         mov dx,3DDh
         mov ax,1
         out dx,al           ; plan d'ecriture
         inc dx
         xchg ah,al
         out dx,al           ; plan de lecture
         mov cl,[di]
         and cl,[bp-1]       ; Not bit mask ( met 0 )
         and ah,bl
         jnz ams03

         or cl,[bp-2]        ; Bit Mask ( met 1 )

  ams03: mov [di],cl
         mov ax,0201h
         out dx,al           ; lecture
         dec dx
         xchg ah,al
         out dx,al           ; ecriture
         mov cl,[di]
         and cl,[bp-1]       ; Not bit mask
         and al,bl
         jz ams04

         or cl,[bp-2]        ; Bit Mask

  ams04: mov [di],cl
         mov ax,0204h
         out dx,al
         inc dx
         xchg ah,al
         out dx,al
         mov cl,[di]
         and cl,[bp-1]       ; Not bit mask
         and ah,bl
         jz ams05

         or cl,[bp-2]        ; Bit Mask
  ams05: mov [di],cl
         mov ax,0803h
         out dx,al
         dec dx
         xchg ah,al
         out dx,al
         mov cl,[di]
         and cl,[bp-1]       ; Not bit mask
         and al,bl
         jz ams06

         or cl,[bp-2]        ; Bit Mask

  ams06: mov [di],cl
         jmp fin

  cga01:
         mov cx,[bp+10]
         and cx,7
         mov ax,80h
         shr ax,cl
         mov [bp-2],ax       ; Bit Mask
         not al
         mov [bp-1],al

         mov ax,0B800h
         mov ds,ax
         mov ax,[bp+8]
         mov bl,40
         push ax
         and ax,0FFFEh
         mul bl
         mov di,ax
         pop ax
         test ax,1
         jz cga02
         add di,2000h

  cga02: mov ax,[bp+10]
         mov cl,3
         shr ax,cl
         add di,ax
         mov bx,[bp+6]

         mov cl,[di]
         and cl,[bp-1]       ; Not bit mask ( met 0 )
         or bx,bx
         jz cga03

         or cl,[bp-2]        ; Bit Mask ( met 1 )

  cga03: mov [di],cl
         jmp fin

  ega01: ;jmp cga01
         mov ax,0a000h
         mov ds,ax

         mov ax,[bp+8]
         mov bl,80
         mul bl
         mov di,ax

         mov ax,[bp+10]
         push ax
         mov cl,3
         shr ax,cl
         add di,ax

         pop cx
         and cx,7
         mov bl,80h
         shr bl,cl

         mov al,8
         mov dx,3CEh
         out dx,al      ; = bit mask register
         inc dx
         mov al,bl
         out dx,al

         mov ax,0F02h
         mov dx,3C4h
         out dx,al
         inc dx
         xchg al,ah
         out dx,al

         mov ah,[di]
         xor ah,ah
         mov [di],ah

         mov al,[bp+6]
         out dx,al
         not ah
         mov al,[di]
         mov [di],ah

         mov ax,0FF08h
         mov dx,3CEh
         out dx,al      ; = bit mask register
         inc dx
         xchg al,ah
         out dx,al

         jmp fin

  her01:
    fin: mov sp,bp
         pop bp
         pop ds
         ret 8
afpoint endp
prog ends
     end deb

