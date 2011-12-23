title lit_phrase ; version 5.0 du 16/10/88


prog segment para
     assume cs:prog

lit_ph proc near       ; table en [bp+12]
                       ; freq en [bp+10]
                       ; typlec en [bp+8]
                       ; tempo en [bp+6]
; On calcule les adresses des echantillons puis on met la suite de ces
; adresses a partir de 60000h . Le lecteur n'aura plus qu'a lire les
; adresses => gain en rapidite .
; Une adresse d'echantillon est composee d'un segment, d'un offset de
; depart puis d'un offset de fin .
; Au maximum il y a moins de 16384 words . [7/88]

         push bp
         push ds
         mov bp,sp
         sub sp,28    ; 10 word de locales

         mov ax,[bp+6]
         sub ax,3
         mov [bp+6],ax

         xor ax,ax
         mov [bp-10],ax   ; en [bp-10] l'offset dans le segment 6000h

         mov bx,[bp+8]
         or bx,bx
         je adclas
         mov ax,2C00h
         jmp suclas

 adclas: mov ax,4300h
 suclas: mov ds,ax
         mov ax,406Bh
         mov es,ax     ; adtroct
         xor di,di
         xor si,si      ; ptroct


   blab: xor ax,ax
         mov al,es:[si]
         inc si
         ; (...)
   parf: cmp ax,0
         je iparv
         cmp ax,2
         je iparc
         cmp ax,6
         je iparc2
         cmp ax,4
         je ipari

; pauses ou fin => 1 word seulement .
         cmp ax,62
         je blab             ; pause transparente
         push ds
         mov cx,6000h
         mov ds,cx
         mov di,[bp-10]
         mov [di],ax
         inc di
         inc di
         mov [bp-10],di
         pop ds
         cmp ax,35
         je p1lit
         jmp blab

  p1lit: jmp lecteur

  iparc: jmp parc
 iparc2: jmp parc2
  ipari: jmp pari

;-----------------------------------------------------------------------------
;       Les adresses des bruits, ranges pour correspondre a la procedure
;       ech_suivant :
;-----------------------------------------------------------------------------
; segment , depart, arrivee

 iparv: call IPCX

adbrui dw 2C00h,     0, 17224
       dw 3034h,     8, 16460
       dw 3438h,    12, 17350
       dw 3874h,     6,  8388
       dw 3A80h,     4,  1893
       dw 3AF6h,     6,  8595

vadson: call IPCX2
adson dw 2C00h, 0, 14020
      dw 2C00h, 14020, 18994
      dw 2C00h, 18994, 19630
      dw 2C00h, 19630, 22258
      dw 2C00h, 22258, 37322
      dw 2C00h, 37322, 44472
      dw 2C00h, 44472, 52324
      dw 2C00h, 52324, 59598
      dw 2E00h, 51406, 61556

IPCX2: pop cx
       jmp duson

   parv: ; cx contient l'offset de adbrui dans cs
         mov al,es:[si]
         xor ah,ah
         inc si

         mov bx,[bp+8]   ; typlec
         or bx,bx
         je vclas
         cmp bx,2
         je vadson

; ***  BRUIT  &  SON ***

  duson:
         inc si
         push si
         push ds
         push cs
         pop ds
         mov si,cx
         mov bl,6
         mul bl
         add si,ax
         mov bx,[si]
         mov di,[si+2]
         mov dx,[si+4]
         pop ds
         pop si
         call ech_suivant
         jmp blab

  vclas: mov bl,12
         mul bl
         mov dl,es:[si]
         xor dh,dh
         inc si

  reech: mov di,ax
         or dx,dx
         je ver0_12
         dec dx
         je ver0_8
         dec dx
         je ver4_12
         dec dx
         je ver0_4
         dec dx
         je ver4_8
         dec dx
         je ver8_12
         jmp fin

 ver0_12: jmp _0_12
  ver0_8: jmp _0_8
 ver4_12: jmp _4_12
  ver0_4: jmp _0_4
  ver4_8: jmp _4_8
 ver8_12: jmp _8_12

   _0_12:
         mov ax,[di]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[di+2]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[di+12]
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[di+14]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

         jmp leno

    _0_8:
         mov ax,[di]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[di+2]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[di+8]
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[di+10]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

         jmp leno

   _4_12:
         mov ax,[di+4]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[di+6]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[di+12]
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[di+14]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

         jmp leno

    _0_4:
         mov ax,[di]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[di+2]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[di+4]
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[di+6]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

         jmp leno

    _4_8:
         mov ax,[di+4]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[di+6]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[di+8]
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[di+10]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

         jmp leno

   _8_12:
         mov ax,[di+8]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[di+10]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[di+12]
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[di+14]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

         jmp leno

   parc: mov al,es:[si]
         xor ah,ah
         inc si
         mov bl,12
         mul bl
         mov dl,es:[si]
         xor dh,dh
         inc si
         add ax,268
         jmp reech

  parc2: mov al,es:[si]
         xor ah,ah
         inc si
         mov bl,12
         mul bl
         mov dl,es:[si]
         xor dh,dh
         inc si
         add ax,692
         jmp reech

   pari: mov bx,[bp+8]   ; typlec
         or bx,bx
         je inacce
         add si,2        ; bruit => ignore l'interphoneme cree par le traitement
         jmp blab

 inacce: mov bl,es:[si]
         xor bh,bh
         inc si
         mov al,es:[si]
         xor ah,ah
         inc si
         mov dl,14     ; 14 voyelles par consonne
         mul dl
         add bx,ax
         shl bx,1
         shl bx,1      ; 4 octets par case de la matrice

; Acces aux interphonemes :
; ( voyelle + 14 * consonne ) * 4

         add bx,1116   ; decalage pour debut de table

; BX contient l'adresse de la place memoire ou l'on peut trouver
; l'adresse de l'echantillon sur 4 octets = 32 bits

         mov ax,[bx]
         xchg ah,al
         mov [bp-2],ax  ;  [bp-2] => 16 bits debut poids fort
         mov ax,[bx+2]
         xchg ah,al
         mov [bp-4],ax  ;  [bp-4] => 16 bits debut poids faible

         mov ax,[bx+4]  ; => la fin de l'inter correspond au debut de la
                        ; suivante .
         xchg ah,al
         mov [bp-6],ax  ;  [bp-6] => 16 bits fin poids fort
         mov ax,[bx+6]
         xchg ah,al
         mov [bp-8],ax  ;  [bp-8] => 16 bits fin poids faible

;--------------------------------------------------------------------------
;      TRAITEMENT  DES  ADRESSES ledeb ET lefin POUR ALLER CHERCHER
;                        L'ECHANTILLON
;--------------------------------------------------------------------------
; ledeb est en [bp-4] dans l'ordre : O0, O1, O2, O3 .
; On a toujours : - O3=0
;                 - O2=0 ou 1
;                 - ledeb= O0 + O1*256 + O2*256^2 + O3*256^3
; equivalent pour lefin en [bp-8]

   leno: mov al,[bp-6]
         or al,al
         jz okseg
         mov al,[bp-2]
         or al,al
         jz gond          ; ledeb< 65536  et  lefin> 65536

         mov bx,53A1h     ; ledeb  et  lefin> 65536
   lbl1: mov di,[bp-4]
         mov dx,[bp-8]
   lbl2: call ech_suivant
         jmp blab

  okseg: mov bx,43A1h     ; ledeb  et  lefin< 65536
         jmp lbl1

   gond: mov ax,[bp-4]
         mov bx,ax
         not ax
         inc ax           ;  => ax = 65536 - ledeb
         add ax,[bp-8]    ;        = lefin - 65536
         mov di,bx
         and di,15
         mov cl,4
         shr bx,cl
         add bx,53A1h
         mov dx,di
         add dx,ax
         jmp lbl2

;-----------------------------------------------------------------------------
;                       Lecture des echantillons
;-----------------------------------------------------------------------------

lecteur: mov ax,6000h
         mov es,ax
         xor si,si
         mov al,0b6h
         out 43h,al
         mov ax,[bp+10]
         out 42h,al
         xchg al,ah
         out 42h,al
         call intr_install
    son: mov ax,es:[si]
         inc si
         inc si
         cmp ax,32
         jne pause
         mov cx,500
   paus:
   vide: push cx
         mov cx,[bp+6]      ; TEMPO
  vide2: loop vide2
         pop cx
         loop vide
         jmp son

  pause: cmp ax,46
         jne prl_ou_sort
         mov cx,5000
         jmp paus

prl_ou_sort: cmp ax,35
         je fin
         mov ds,ax
         mov di,es:[si]
         inc si
         inc si
         mov bx,es:[si]
         inc si
         inc si
         call blabla
         jmp son

    fin: call no_son
         call intr_desinstall
         mov sp,bp
         pop ds
         pop bp
         ret 518    ; table (512) + tempo(2) + typlec(2) +2
lit_ph endp

IPCX proc near
     pop cx
     jmp parv
IPCX endp

ech_suivant proc near    ; dx= fin
                         ; bx= pseg
                         ; di= deb
          push ds
          mov ax,6000h
          mov ds,ax
          mov ax,di
          mov di,[bp-10]
          mov [di],bx
          inc di
          inc di
          mov [di],ax
          inc di
          inc di
          mov [di],dx
          inc di
          inc di
          mov [bp-10],di
          pop ds
          ret
ech_suivant endp

intr_install proc near
              cli
              push ds
              xor ax,ax
              mov ds,ax
              mov bx,32
              mov ax,[bx]
              mov [bp-20],ax ; sauve dep de l'intr 8
              mov ax,[bx+2]
              mov [bp-22],ax
              call suite

intr:         push ax
              in al,61h
              and al,0FCh
              out 61h,al
              mov al,20h
              out 20h,al
              pop ax
              iret

suite:        pop ax
              mov [bx],ax
              push cs
              pop ax
              mov [bx+2],ax
              ;mov al,30h
              ;out 43h,al
              mov al,0FEh
              out 21h,al
              pop ds
              sti
              ret
intr_install endp

intr_desinstall proc near
               cli
               push ds
               xor ax,ax
               mov ds,ax
               mov bx,32
               mov ax,[bp-20]
               mov [bx],ax
               mov ax,[bp-22]
               mov [bx+2],ax
               pop ds
               sti
               mov al,0
               out 21h,al
               ret
intr_desinstall endp


blabla proc near         ; bx= fin
                         ; di= deb
            push si

            jmp suit1

      bcl2: mov cx,[bp+6]    ; TEMPO
      bcl1: loop bcl1

     suit1: mov dl,[di]
            inc di
            xor dh,dh
            mov si,dx

            shl si,1       ; 2 octets par element de la table
            add si,12

            in al,61h
            or al,3
            out 61h,al

            mov al,30h
            out 43h,al
            mov ax,[bp+si]
            out 40h,al
            xchg al,ah
            out 40h,al


            cmp di,bx
            jc bcl2

            pop si
            ret
blabla endp
no_son proc near
        in al,61h
        and al,0fch
        out 61h,al
        ret
no_son endp
son_on proc near
        in al,61h
        or al,3
        out 61h,al
        ret
son_on endp
prog ends
     end lit_ph

