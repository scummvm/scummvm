title boite

; Pile (BP) =  ; DS save    [bp]
               ; BP old     [bp+2]
               ; Retour     [bp+4]
               ; Pattern    [bp+6]
               ; y fin      [bp+8]
               ; x fin      [bp+10]
               ; y debut    [bp+12]
               ; x debut    [bp+14]
               ; Graph Device  => 0 = Ams   [bp+16]
               ;               => 1 = Cga
               ;               => 2 = Ega
               ;               => 3 = Her
               ;               => 4 = Tan
               ; Couleur     [bp+18]

prog segment para
       assume CS:prog
boite proc near
    aff: push bp
         push ds
         mov bp,sp
         sub sp,20

         mov ax,[bp+16]
         or ax,ax
         jz Gdams
         dec ax
         jz Gdcga
         dec ax
         jz Gdega
         dec ax
         jz Gdher
         dec ax
         jz Gdtan
         jmp fin

  Gdams: jmp ams01
  Gdcga: jmp cga01
  Gdega: jmp ega01
  Gdher: jmp her01
  Gdtan: jmp tan01

  ams01: mov ax,0B800h
         mov ds,ax

         xor ax,ax
         mov [bp-2],ax       ; Plan couleur de lecture
         inc ax
         mov [bp-4],ax       ; Plan couleur d'ecriture
         call calad

  ams02: mov ax,[bp-2]
         inc word ptr [bp-2]
         mov dx,3DEh
         out dx,al
         mov ax,[bp-4]
         dec dx
         out dx,al
         mov bx,[bp+18]
         and bx,ax
         or bx,bx
         jz ams11
         mov bx,1
  ams11: mov [bp-18],bx      ; 0 si pas d'ecriture dans ce plan , 1 si ecriture
         shl ax,1
         mov [bp-4],ax       ; Plan ecriture suivant

         mov di,[bp-6]      ; offset debut
         mov dx,[bp-16]     ; Nombre de lignes
         mov al,[bp-11]
         mov [bp-12],al

  ams05: push di
         mov cx,[bp-14]     ; Nombre d'octets par ligne
         or cx,cx
         jz ams06
         mov bx,[bp-18]
         mov ax,[bp-8]
         not al
         and [di],al
         not al
         and ax,[bp+6]       ; Pattern
         mul bl              ; 0 ou 1
         or [di],al
         inc di
         jmp ams03

  ams04: mov ax,255
         and ax,[bp+6]       ; Pattern
         mul bl              ; 0 ou 1
  ams10: mov [di],al
         inc di
  ams03: loop ams04

  ams06: mov ax,[bp-10]
         not al
         and [di],al
         not al
         and ax,[bp+6]
         mul bl
         or [di],al

         dec dx
         jz ams07
         mov al,[bp-12]
         or al,al
         jz ams08
         dec byte ptr [bp-12]
         pop di
         sub di,2000h
         add di,80
         jmp ams05

  ams08: inc byte ptr [bp-12]
         pop di
         add di,2000h
         jmp ams05

  ams07: pop di
         mov ax,[bp-2]
         cmp ax,4
         jz ams09
         jmp ams02
  ams09: jmp fin

  cga01: mov ax,0B800h
         mov ds,ax
         call calad
         mov ax,[bp+18]
         or ax,ax
         jz cga08
         mov ax,1
  cga08: mov [bp-18],ax
         mov di,[bp-6]      ; offset debut
         mov dx,[bp-16]     ; Nombre de lignes
         mov bx,[bp-18]     ; Noir ou Blanc

  cga02: push di
         mov cx,[bp-14]     ; Nombre d'octets par ligne
         or cx,cx
         jz cga05
         mov ax,[bp-8]
         not al
         and [di],al
         not al
         and ax,[bp+6]       ; Pattern
         mul bl              ; 0 ou 1
         or [di],al
         inc di
         jmp cga04

  cga03: mov ax,255
         and ax,[bp+6]       ; Pattern
         mul bl
         mov [di],al
         inc di
  cga04: loop cga03

  cga05: mov ax,[bp-10]
         not al
         and [di],al
         not al
         and ax,[bp+6]
         mul bl
         or [di],al

         dec dx
         jz cga07
         mov al,[bp-12]
         or al,al
         jz cga06
         dec byte ptr [bp-12]
         pop di
         sub di,2000h
         add di,80
         jmp cga02

  cga06: inc byte ptr [bp-12]
         pop di
         add di,2000h
         jmp cga02

  cga07: pop di
         jmp fin

  ega01: mov ax,0A000h
         mov ds,ax

         mov dx,3CEh      ; => 3CFh pointe sur le Bit mask register
         mov al,8
         out dx,al
         mov dx,3C4h      ; => 3C5h pointe sur le Map mask register
         mov al,2
         out dx,al

         mov ax,[bp+12]
         mov bl,80
         mul bl
         mov bx,[bp+14]
         push bx
         mov cl,3
         shr bx,cl
         add ax,bx
         mov [bp-6],ax       ; offset 1er octet d'affichage

         pop bx
         call calad04
         mov di,[bp-6]      ; offset debut
         mov dx,[bp-16]     ; Nombre de lignes

  ega02: push di
         push dx
         mov cx,[bp-14]     ; Nombre d'octets par ligne
         or cx,cx
         jz ega05
         mov ax,[bp-8]
         jmp ega04

  ega03: mov ax,255
  ega04: call ppix
         inc di
         loop ega03

  ega05: mov ax,[bp-10]
         call ppix

         pop dx
         pop di
         add di,80
         dec dx
         jnz ega02

  ega07: jmp fin

  her01: mov ax,0B000h
         mov ds,ax

         mov ax,[bp+8]
         shl ax,1
         mov [bp+8],ax
         mov ax,[bp+12]     ; y deb
         shl ax,1
         mov [bp+12],ax
         push ax
         shr ax,1
         shr ax,1
         mov bx,80
         mul bx
         mov cx,ax
         pop ax
         and ax,3
         mov bx,2000h
         mul bx
         add ax,cx
         mov bx,[bp+14]     ; x deb
         push bx
         mov cl,3
         shr bx,cl
         add ax,bx
         mov [bp-6],ax      ; offset 1er octet d'affichage

         pop bx
         call calad04
         mov di,[bp-6]      ; offset debut
         mov dx,[bp-16]     ; Nombre de lignes

  her02: push di
         mov cx,[bp-14]     ; Nombre d'octets par ligne
         or cx,cx
         jz her05
         mov ax,[bp-8]
         jmp her04

  her03: mov ax,255
  her04: call putherbyte
         inc di
         loop her03

  her05: mov ax,[bp-10]
         call putherbyte

         pop di
         add di,2000h
         shl di,1
         jnc nonadd80
         add di,160
nonadd80:
         shr di,1
         dec dx
         jnz her02

  her07: jmp fin

;  her01: mov ax,0B000h
;         mov ds,ax
;         mov ax,[bp+8]
;         shl ax,1
;         mov [bp+8],ax
;         mov ax,[bp+14]
;         mov [bp-2],ax
;
;         mov ax,[bp+12]
;         shl ax,1
;         mov [bp+12],ax
;         mov bx,ax
;         shr ax,1
;         shr ax,1
;         mov cx,80
;         mul cx
;         and bx,3
;         mov cx,ax
;         mov ax,2000h
;         mul bx
;         add ax,cx
;         mov di,ax
;
;  her03: push di
;         mov ax,[bp-2]
;         mov cl,3
;         mov bx,ax
;         shr ax,cl
;         add di,ax
;         mov ax,[bp+18]
;         and ax,1
;         and bl,7
;         neg bl
;         add bl,7
;         mov cl,bl
;         mov ah,1
;         shl ah,cl
;         not ah
;         shl al,cl
;         mov bl,[di]
;         and bl,ah
;         or bl,al
;         mov [di],bl
;         mov ax,[bp-2]
;         cmp ax,[bp+10]
;         je her02
;         inc ax
;         mov [bp-2],ax
;         pop di                   ; debut de ligne
;         jmp her03
;
;  her02: pop di
;         mov ax,[bp+12]
;         cmp ax,[bp+8]
;         jne her04
;         jmp fin
;
;  her04: inc ax
;         mov [bp+12],ax
;         mov bx,[bp+14]
;         mov [bp-2],bx
;         and ax,3
;         jz her05
;         add di,2000h
;         jmp her03
;
;  her05: sub di,6000h
;         add di,80
;         jmp her03


  tan01: mov ax,0B800h
         mov ds,ax
         mov ax,[bp+10]
         shr ax,1
         mov [bp+10],ax
         mov ax,[bp+12]
         mov bx,ax
         shr bx,1
         shr bx,1
         mov cx,2000h
         and ax,3
         mul cx
         mov cx,ax
         mov ax,160
         mul bx
         add cx,ax
         mov ax,[bp+14]
         shr ax,1
         shr ax,1
         add cx,ax
         mov [bp-2],cx
         mov di,cx
         mov dx,[bp+18]
         mov dh,dl
         mov bh,dl
         mov cl,4
         shl dh,cl
         or bh,dh            ; dl = point droit, dh = gauche, bh = both
         mov ax,[bp+14]
         shr ax,1
  tan02: test ax,1
         jz tan06

  tan03: mov bl,[di]
         and bl,0F0h
         or bl,dl
         mov [di],bl
         inc di
         inc ax
         cmp ax,[bp+10]
         je tan04

  tan07: mov [di],bh
         inc di
         inc ax
         cmp ax,[bp+10]
         je tan04

  tan06: inc ax
         cmp ax,[bp+10]
         jne tan07
  tan08: mov bl,[di]
         and bl,15
         or bl,dh
         mov [di],bl

  tan04: mov ax,[bp+12]
         inc ax
         cmp ax,[bp+8]
         je tan09
         mov [bp+12],ax
         mov di,[bp-2]
         cmp di,6000h
         jc tan10
         sub di,6000h
         add di,160
         jmp tan11
  tan10: add di,2000h
  tan11: mov ax,[bp+14]
         shr ax,1
         mov [bp-2],di
         jmp tan02

  tan09:

    fin: mov sp,bp
         pop ds
         pop bp
         ret 14
boite endp
calad proc near
           xor bx,bx
           mov [bp-12],bl    ; 0 ligne pair , 1 ligne impaire
           mov ax,[bp+12]    ; y debut
           test ax,1
           jz calad01
           add bx,2000h
           inc byte ptr [bp-12]  ; ligne impaire

  calad01: mov cl,[bp-12]
           mov [bp-11],cl        ; sauve la parite du depart
           and ax,0FFFEh         ; masque la parite
           push bx
           mov bl,40
           mul bl             ; AX = offset du debut de la ligne
           mov dx,[bp+14]     ; x debut
           mov bl,dl          ; sauvegarde des Bits faibles
           mov cl,3
           shr dx,cl          ; division par 8
           add ax,dx
           pop bx
           add ax,bx
           mov [bp-6],ax       ; Offset premier octet affichage
  calad04:
           mov ax,80h
           mov cl,bl
           and cl,7
           shr ax,cl
           mov bx,ax
           dec bx
           add ax,bx         ; Masque : tout les bits devant etre affiche
                             ; dans cet octet sont a 1 ( = 0..01..1b )
           mov [bp-8],ax     ; Masque Binaire d'affichage du premier octet

           mov ax,[bp+10]
           and ax,7
           mov cl,al
           mov ax,80h
           shr ax,cl
           mov bx,ax
           dec ax
           add ax,bx
           not ax
           mov [bp-10],ax      ; Masque Binaire d'affichage du dernier octet
                               ; ( 1..10..0b )
           mov ax,[bp+10]      ; x fin
           mov cl,3
           shr ax,cl
           mov bx,[bp+14]
           shr bx,cl
           sub ax,bx
           jz calad02        ; Octet fin = Octet debut
           mov [bp-14],ax
           jmp calad03

  calad02: mov [bp-14],ax
           mov ax,[bp-10]
           and ax,[bp-8]
           mov [bp-10],ax

  calad03: mov ax,[bp+8]
           sub ax,[bp+12]
           mov [bp-16],ax    ; Nombre de lignes
           ret
calad endp
ppix proc near
         mov dx,3CFh      ; Bit mask register
         out dx,al
         push ax

         mov dx,3C5h      ; Map mask register
         mov al,0Fh       ; permet d'ecrire dans tout les plans
         out dx,al

         mov al,[di]        ; Lecture preliminaire de l'octet a adresser
         sub al,al
         mov [di],al        ; Met 0 dans tout les plans

         mov ax,[bp+18]     ; couleur
         out dx,al

         pop ax
         and ax,[bp+6]       ; Pattern
         mov dx,3CFh         ; Bit mask register
         out dx,al

         mov al,[di]
         mov al,0FFh
         mov [di],al        ; Seuls les bits pre-selectionnes avec le bit mask
                            ; sont reelement adresses
         ret
ppix endp
putherbyte proc near

         mov bh,[di]        ; Lecture preliminaire de l'octet a adresser
         and al,[bp+6]      ; Pattern
         not al
         and bh,al

         mov bl,[bp+18]     ; couleur
         or bl,bl
         jz noir
         not al
         or bh,al
   noir: mov [di],bh
         ret

putherbyte endp
prog ends
     end aff

