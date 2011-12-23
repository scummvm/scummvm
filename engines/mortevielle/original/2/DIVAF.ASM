TITLE affiche_un_caractere_ou_un_point

prog segment para
     assume cs:prog,ds:prog

deb: push bp
     mov bp,sp
     mov ax,[bp+14]
     pop bp
     or ax,ax
     jz point
     dec ax
     jnz deb00
     jmp car
deb00:
     jmp mou

afpoint proc near
point: push ds
       push bp
       mov bp,sp
       sub sp,20

;       *********  PARAMETRES  ********
; Pile (bp) :
;             [bp]    = old bp
;             [bp+2]  = old ds
;             [bp+4]  = retour
;             [bp+6]  = rien
;             [bp+8]  = couleur
;             [bp+10] = y
;             [bp+12] = x
;             [bp+14] = Graph Device  ( 0 asm, 1 cga, 2 ega, 3 her, 4 tan )
;             [bp+16] = Choix

         mov bx,[bp+14]
         or bx,bx
         jz Gdams
         dec bx
         jz Gdcga
         dec bx
         jz Gdega
         dec bx
         jz Gdher
         dec bx
         jz Gdtan
         jmp fin

  Gdams: jmp ams01
  Gdcga: jmp cga01
  Gdega: jmp ega01
  Gdher: jmp her01
  Gdtan: jmp tan01

  ams01:
         mov cx,[bp+12]
         and cx,7
         mov ax,80h
         shr ax,cl
         mov [bp-2],ax       ; Bit Mask
         not al
         mov [bp-1],al

         mov ax,0B800h
         mov ds,ax
         mov ax,[bp+10]
         mov bl,40
         push ax
         and ax,0FFFEh
         mul bl
         mov di,ax
         pop ax
         test ax,1
         jz ams02
         add di,2000h

  ams02: mov ax,[bp+12]
         mov cl,3
         shr ax,cl
         add di,ax
         mov bx,[bp+8]


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

  cga01: mov ah,12
         mov dx,[bp+10]
         mov cx,[bp+12]
         mov al,[bp+8]
         int 16
         ;mov ax,[bp+10]
         ;mov cx,ax
         ;and ax,1
         ;mov bx,2000h
         ;mul bx
         ;and cx,0FFFEh
         ;mov bx,ax
         ;mov ax,80
         ;mul cx
         ;add ax,bx
         ;mov bx,[bp+12]
         ;mov cx,bx
         ;shr bx,1
         ;shr bx,1
         ;add ax,bx
         ;mov di,ax
         ;and cx,3
         ;mov ax,[bp+8]
         ;and ax,3
         ;push cx
         ;mov cl,6
         ;shl ax,cl
         ;pop cx
         ;shr ax,cl
         ;mov bx,
         ;shr bx,cl
         ;not bl
         ;mov cl,[di]

         ;and cl,bl       ; Not bit mask ( met 0 )
         ;or cl,al        ; Bit Mask ( met 1 )

         ;mov [di],cl
         jmp fin

  ega01: mov ax,0a000h
         mov ds,ax

         mov ax,[bp+10]
         mov bx,80
         mul bx
         mov di,ax

         mov ax,[bp+12]
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

         mov al,[bp+8]
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

  tan01: mov ax,[bp+10]
         and ax,11b
         mov bx,2000h
         mul bx
         mov cx,ax
         mov ax,[bp+10]
         shr ax,1
         shr ax,1
         mov bx,160
         mul bx
         add cx,ax
         mov ax,[bp+12]
         shr ax,1
         add ax,cx
         mov di,ax
         mov ax,0B800h
         mov ds,ax
         mov dl,[di]
         mov bx,[bp+8]
         mov cl,4
         mov ax,[bp+12]
         and ax,1
         jz tan02
         and dl,0F0h
         jmp tan03
  tan02: and dl,0Fh
         shl bl,cl
  tan03: or dl,bl
         mov [di],dl
         jmp fin

  her01: mov ax,[bp+10]
         shl ax,1            ; 400 lines => y * 2
         mov cl,2
         push ax
         shr ax,cl
         mov bx,80
         mul bx
         mov cx,ax
         pop ax
         and ax,3
         mov bx,2000h
         mul bx
         add cx,ax
         mov ax,[bp+12]
         shr ax,1
         shr ax,1
         shr ax,1
         add ax,cx
         mov di,ax
         mov ax,0B000h
         mov ds,ax
         mov dl,[di]
         mov cl,[bp+12]
         and cl,7
         neg cl
         add cl,7
         mov al,1
         shl al,cl
         mov bl,al
         not al
         and dl,al
         mov ah,[bp+8]
         or ah,ah
         je her02
         or dl,bl
  her02: mov [di],dl
         add di,2000h
         mov dl,[di]
         and dl,al
         or ah,ah
         je her03
         or dl,bl
  her03: mov [di],dl

    fin: mov sp,bp
         pop bp
         pop ds
         ret 12
afpoint endp

afmou proc near
  mou: push ds
       push bp
       mov bp,sp
       sub sp,20

;       *********  PILE  ********
; Pile (bp) :
;             [bp]    = old bp
;             [bp+2]  = old ds
;             [bp+4]  = retour
;             [bp+6]  = rien
;             [bp+8]  = couleur
;             [bp+10] = y
;             [bp+12] = x
;             [bp+14] = Graph Device  ( 0 asm, 1 cga, 2 ega, 3 her )
;             [bp+16] = Choix

       mov ax,[bp+12]
       mov dx,ax
       mov bx,0F00h
       mov cx,[bp+10]
       call point1
       inc ax
       inc ax
       call point1
       dec ax
       xchg bh,bl    ; blanc
       call point1
       inc cx        ; new line
       call point1
       inc ax
       call point1
       xchg bh,bl    ; noir
       inc ax
       call point1
       mov ax,dx
       call point1
       inc cx        ; new line
       call point1
       xchg bh,bl    ; blanc
       inc ax
       call point1
       inc ax
       call point1
       inc ax
       call point1
       inc ax
       xchg bh,bl    ; noir
       call point1
       inc ax
       inc cx        ; new line
       call point1
       mov ax,dx
       call point1
       inc ax
       xchg bh,bl    ; blanc
       call point1
       inc ax
       call point1
       inc ax
       call point1
       inc ax
       call point1
       inc cx        ; new line
       call point1
       inc ax
       call point1
       xchg bh,bl    ; noir
       inc ax
       call point1
       mov ax,dx
       call point1
       xchg bh,bl    ; blanc
       inc ax
       call point1
       inc ax
       call point1
       inc ax
       call point1
       inc cx
       call point1
       inc ax
       call point1
       mov ax,dx
       xchg bh,bl
       call point1
       inc ax
       xchg bh,bl
       call point1
       inc cx
       inc ax
       mov dx,ax
       xchg bh,bl
       call point1
       xchg bh,bl
       inc ax
       call point1
       inc ax
       call point1
       xchg bh,bl
       inc ax
       call point1
       inc cx
       call point1
       mov ax,dx
       call point1
       xchg bh,bl
       inc ax
       call point1
       inc ax
       call point1

       mov sp,bp
       pop bp
       pop ds
       ret 12
afmou endp

point1 proc near
       push ax
       push bx
       push cx
       push dx

       push [bp+16]          ; Choix
       push [bp+14]          ; Gd
       push ax               ; x
       push cx               ; y
       push bx               ; couleur
       push bx               ;
       call afpoint

       pop dx
       pop cx
       pop bx
       pop ax
       ret
point1 endp



afcar proc near
car:
     call debut

letr db 0h,0h,0h,0h,0h,0h            ; ' '
     db 20h,70h,20h,0h,20h,0h        ; !
     db 50h,50h,0h,0h,0h,0h          ; "
     db 50h,0F0h,50h,0F0h,50h,0      ; #
     db 20h,70h,0A0h,70h,28h,20h     ; $
     db 88h,10h,20h,40h,88h,0h       ; %
     db 60h,90h,60h,0h,0h,0h         ; Attention: non ASCII ==> ¯
     db 20h,40h,0h,0h,0h,0h
     db 10h,20h,40h,40h,20h,10h
     db 40h,20h,10h,10h,20h,40h
     db 0h,0h,50h,0F8h,50h,0h
     db 0h,0h,20h,0F8h,20h,0h
     db 0h,0h,0h,20h,20h,40h
     db 0h,0h,0h,0F8h,0h,0h
     db 0h,0h,0h,0h,30h,0h
     db 8h,10h,20h,40h,80h,0h
     db 70h,90h,90h,90h,0E0h,0h    ; 0
     db 10h,30h,10h,10h,10h,0h
     db 0E0h,01h,60h,80h,0F0h,0h
     db 0E0h,10h,60h,10h,0E0h,0h
     db 10h,30h,50h,0F8h,10h,0h
     db 0F0h,80h,0E0h,10h,0E0h,0h
     db 30h,40h,0E0h,90h,60h,0h
     db 0F0h,10h,20h,40h,40h,0h
     db 60h,90h,60h,90h,60h,0h
     db 60h,90h,70h,20h,0C0h,0h    ; 9
     db 0h,0h,30h,0h,30h,0h
     db 0h,0h,30h,0h,30h,40h
     db 10h,20h,40h,20h,10h,0h
     db 0h,0h,0F8h,0h,0F8h,0h
     db 40h,20h,10h,20h,40h,0h
     db 0E0h,10h,60h,0h,40h,0h
     db 0h,0h,0h,0h,0h,0h         ; @
     db 60h,90h,90h,0F0h,90h,0h
     db 0E0h,90h,0E0h,90h,0E0h,0h
     db 70h,80h,80h,80h,70h,0h
     db 0E0h,90h,90h,90h,0E0h,0h
     db 0F0h,80h,0E0h,80h,0F0h,0h
     db 0F0h,80h,0E0h,80h,80h,0h
     db 70h,80h,90h,90h,70h,0h
     db 90h,90h,0F0h,90h,90h,0h
     db 70h,20h,20h,20h,70h,0h
     db 10h,10h,90h,90h,60h,0h
     db 90h,0A0h,0C0h,0A0h,90h,0h
     db 80h,80h,80h,80h,0F0h,0h
     db 88h,0D8h,0A8h,88h,88h,0h
     db 88h,0C8h,0A8h,98h,88h,0h
     db 60h,90h,90h,90h,60h,0h
     db 0E0h,90h,0E0h,80h,80h,0h
     db 60h,90h,90h,90h,60h,30h
     db 0E0h,90h,0E0h,90h,88h,0h
     db 70h,80h,60h,10h,0E0h,0h
     db 0F8h,20h,20h,20h,20h,0h
     db 90h,90h,90h,90h,0F0h,0h
     db 90h,90h,90h,0A0h,40h,0h
     db 88h,88h,0A8h,0D8h,88h,0h
     db 88h,50h,20h,50h,88h,0h
     db 88h,88h,50h,20h,20h,0h
     db 0F0h,20h,40h,80h,0F0h,0h
     db 60h,40h,40h,40h,60h,0h
     db 80h,40h,20h,10h,8h,0h
     db 60h,20h,20h,20h,60h,0h
     db 20h,50h,0h,0h,0h,0h
     db 0h,0h,0h,0h,0h,0F8h
     db 40h,20h,0h,0h,0h,0h
     db 0h,70h,90h,90h,78h,0h      ; a
     db 80h,0E0h,90h,90h,0E0h,0h   ; b
     db 0h,70h,80h,80h,70h,0h      ; c
     db 10h,70h,90h,90h,70h,0h     ; d
     db 0h,60h,0B0h,80h,70h,0h     ; e
     db 38h,40h,0F0h,40h,40h,0h    ; f
     db 0h,70h,90h,70h,10h,0E0h    ; g
     db 80h,0E0h,90h,90h,90h,0h    ; h
     db 20h,60h,20h,20h,0F0h,0h     ; i
     db 20h,0E0h,20h,20h,20h,0C0h  ;
     db 80h,90h,0A0h,0E0h,90h,0h   ;
     db 60h,20h,20h,20h,0F0h,0
     db 0h,0D0h,0A8h,0A8h,88h,0h
     db 0h,0E0h,90h,90h,90h,0
     db 0h,60h,90h,90h,60h,0
     db 0h,0E0h,90h,90h,0E0h,80h
     db 0h,70h,90h,90h,70h,10h
     db 0h,0B0h,0C0h,80h,80h,0
     db 0h,70h,0C0h,30h,0F0h,0
     db 40h,0F0h,40h,40h,30h,0
     db 0h,90h,90h,90h,70h,0
     db 0h,90h,90h,0A0h,40h,0
     db 0h,88h,0A8h,0A8h,50h,0
     db 0h,90h,60h,60h,90h,0
     db 0h,90h,90h,0A0h,40h,80h
     db 0h,0F0h,20h,40h,0F0h,0h     ; z
     db 6 dup (0) ; {
     db 20h,20h,20h,20h,20h,20h ; |
     db 6 dup (0) ; }
     db 6 dup (0) ; ~
     db 6 dup (0) ;
     db 70h,80h,80h,80h,70h,0C0h
     db 90h,0,90h,90h,70h,0
     db 10h,20h,0F0h,80h,70h,0
     db 20h,50h,70h,90h,70h,0
     db 90h,00h,70h,90h,70h,0         ; 132   Ñ
     db 80h,40h,70h,90h,70h,0         ; Ö
     db 60h,00h,70h,90h,70h,0         ; Ü
     db 00h,70h,80h,80h,70h,0C0h      ; á
     db 20h,50h,0F0h,80h,70h,00h       ;  à
     db 90h,00h,0F0h,80h,70h,00h       ; â
     db 80h,40h,0F0h,80h,70h,00h       ; ä
     db 90h,00h,60h,20h,0F0h,00h       ; ã
     db 60h,90h,60h,20h,0F0h,00h       ; å
     db 80h,40h,60h,20h,0F0h,00h       ; ç
     db 6 dup (0) ; é
     db 6 dup (0) ; è
     db 6 dup (0) ; ê
     db 6 dup (0) ; ë
     db 6 dup (0) ; í
     db 20h,50h,60h,90h,60h,00h       ; ì
     db 90h,00h,60h,90h,60h,00h       ; î
     db 80h,40h,60h,90h,60h,00h       ; ï
     db 20h,50h,90h,90h,70h,00h       ; ñ
     db 80h,40h,90h,90h,70h,00h       ; ó
     db 80h,0C0h,0E0h,90h,0A0h,30h    ; fleche

debut: pop ax     ; ax = offset letr
       push ds
       push bp
       mov bp,sp
       sub sp,20

;       *********  PARAMETRES  ********
; Pile (bp) :
;             [bp]    = old bp
;             [bp+2]  = old ds
;             [bp+4]  = retour
;             [bp+6]  = caractere
;             [bp+8]  = couleur
;             [bp+10] = y
;             [bp+12] = x
;             [bp+14] = Graph Device  ( 0 asm, 1 cga, 2 ega, 3 her )
;             [bp+16] = 1 pour afcar

         mov [bp-6],ax       ; offset letr

         mov ax,cs
         mov ds,ax

         mov ax,[bp+10]
         add ax,6
         mov [bp-14],ax      ; y fin
         mov ax,[bp+12]
         add ax,5
         mov [bp-16],ax      ; x fin

         mov ax,[bp+6]
         cmp ax,32
         jc ctrl
         mov bx,[bp+14]
         or bx,bx
         jz Gdamscar
         dec bx
         jz Gdcgacar
         dec bx
         jz Gdegacar
         dec bx
         jz Gdhercar
         dec bx
         jz Gdtancar

   ctrl: jmp fincar
  Gdamscar: jmp amscar01
  Gdcgacar: ;jmp cgacar01
  Gdegacar: ;jmp egacar01
  Gdhercar: ;jmp hercar01
  Gdtancar: jmp tancar01

  amscar01:
         mov [bp-2],bx       ; Plan couleur de lecture
         inc bx
         mov [bp-4],bx       ; Plan couleur d'ecriture
         sub ax,32
         mov bl,6
         mul bl
         add ax,[bp-6]
         mov [bp-6],ax       ; offset du car a ecrire

  amscar02: mov si,[bp-6]
         mov dx,3DDh
         mov ax,[bp-4]
         out dx,al           ; plan d'ecriture
         xor bx,bx
         test ax,[bp+8]
         jz amscar04
         inc bx
  amscar04: mov [bp-8],bx
         shl ax,1
         mov [bp-4],ax
         inc dx
         mov ax,[bp-2]
         out dx,al           ; plan de lecture
         inc ax
         mov [bp-2],ax
         mov ax,[bp+10]
         mov [bp-10],ax      ; y courant
         mov ax,[bp+12]
         mov [bp-12],ax      ; x courant

         mov dx,[bp-10]
         mov ah,12
         mov al,[bp-8]

  amscar07: mov bl,[si]
         or bl,bl
         jz amscar06
         mov bh,80h
         mov cx,[bp-12]

  amscar03: test bl,bh
         jz amscar05
         int 16
  amscar05: inc cx
         shr bh,1
         cmp bh,4
         jne amscar03

  amscar06: inc si
         inc dx
         cmp dx,[bp-14]
         jne amscar07
         mov ax,[bp-4]
         cmp ax,10h
         je amscar08
         jmp amscar02

  amscar08: jmp fincar

  egacar01:
  hercar01:
  tancar01:
  cgacar01:
         sub ax,32
         mov bl,6
         mul bl
         add ax,[bp-6]
         mov si,ax           ; offset du car a ecrire

         mov dx,[bp+10]
         mov ah,12
         mov al,[bp+8]

  cgacar02: mov bl,[si]
         or bl,bl
         jz cgacar06
         mov bh,80h
         mov cx,[bp+12]

  cgacar03: test bl,bh
         jz cgacar05
         ;
         push cx
         push bx
         push si
         push dx
         push es

         mov ax,1
         push ax
         mov ax,[bp+14]
         push ax
         push cx
         push dx
         mov ax,[bp+8]
         push ax
         push ax
         call afpoint
         ;int 16

         pop es
         pop dx
         pop si
         pop bx
         pop cx
  cgacar05: inc cx
         shr bh,1
         cmp bh,4
         jne cgacar03

  cgacar06:  inc si
         inc dx
         cmp dx,[bp-14]
         jne cgacar02
         ;jmp fincar


 fincar: mov sp,bp
         pop bp
         pop ds
         ret 12
afcar endp
prog ends
     end deb

