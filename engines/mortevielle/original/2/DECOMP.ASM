title decompresseur_pour_le_pascal_Turbo  ;  decomp v 2.0

prog segment para
       assume CS:prog
; Pile durant la proc : bp    => bp old
;                       bp+2  => ds old
;                       bp+4  => ret
;                       bp+6  => offset fichier dessin
;                       bp+8  => segment fichier dessin

decp proc near
     debut:
            push ds
            push bp
            mov bp,sp
            push bp
            sub sp,276

            mov ax,[bp+8]
            mov ds,ax
            mov si,[bp+6]

            xor ax,ax
            mov [bp-50],ax   ; somme des taix
            mov [bp-52],ax   ; somme des taiy

            mov al,[si]
            mov [bp-1],al    ; type dessin/ani
            inc si
            mov ah,[si]      ; nb rectangles > 0
            inc si
            mov [bp-2],ah
            inc si
            inc si
            or al,al
            jnz dec00
            add si,30
     dec00: mov ax,[si+4]
            xchg ah,al
            mov [bp-54],ax   ; abscisse de d‚part
            mov ax,[si+6]
            xchg ah,al
            mov [bp-56],ax   ; ordonn‚e de d‚part

;----------------------------------------------------------------------------
;              Premier parcours du fichier pour calculer
;                   les coordonn‚es d'affichage .
;----------------------------------------------------------------------------
     dec30: push si
            mov cl,[bp-2]
            xor ch,ch
     dec32: mov ax,[si+4]
            xchg ah,al
            cmp [bp-54],ax
            jc dec33
            mov [bp-54],ax
     dec33: mov ax,[si+6]
            xchg ah,al
            cmp [bp-56],ax
            jc dec34
            mov [bp-56],ax
     dec34: mov ax,[si]
            xchg ah,al
            mov bx,[si+2]
            xchg bh,bl
            add ax,bx
            test ax,1
            jz dec31
            inc ax
     dec31: add si,ax
            add si,14
            loop dec32
            pop si
     dec01: mov ax,[si]
            xchg ah,al
            mov [bp-4],ax    ; tcol

            mov ax,[si+2]
            xchg ah,al
            mov [bp-6],ax    ; tval

            mov ax,[si+4]
            xchg ah,al
            sub ax,[bp-54]
            mov [bp-8],ax    ; abscisse rectangle

            mov ax,[si+6]
            xchg ah,al
            sub ax,[bp-56]
            mov [bp-10],ax   ; ordonnee rectangle
            add si,8

            mov ax,[si]
            xchg ah,al
            mov [bp-12],ax   ; ncom et code

            mov dx,[si+2]
            xchg dh,dl
            inc dx
            mov [bp-14],dx   ; taix

            mov dx,[si+4]
            xchg dh,dl
            inc dx
            mov [bp-16],dx   ; taiy

            call maj_ttx_tty
            add si,6
            xor di,di
            mov [bp-22],di
            mov [bp-24],di

            or ah,ah         ; ncom
            jnz dec15
            jmp decom0

     dec15: mov bx,4140h
            mov dx,7000h
            mov es,dx
            test al,1        ; code
            jnz anacol

            mov cx,[bp-4]
            shl cx,1
     dec02: call suiv
            mov es:[bx+di],dl
            inc di
            loop dec02
            jmp dec12

    anacol: call desanalyse
            mov [bp-18],ax   ; nrep
            call desanalyse
            mov [bp-20],ax   ; taiseq
            mov al,[bp-21]
            push ax
            push word ptr [bp-24]
            push si
     dec05: pop si
            pop word ptr [bp-24]
            pop ax
            mov [bp-21],al
            push ax
            push word ptr [bp-24]
            push si
            mov cx,[bp-20]

     dec06: call suiv
            mov es:[bx+di],dl
            inc di
            loop dec06
            dec word ptr [bp-18]
            jnz dec05

            add sp,6
            mov ax,[bp-4]
            dec ax
            cmp [bp-24],ax
            jc anacol

     dec12: mov al,[bp-21]
            or al,al
            jz dec13
            inc si
            xor al,al
            mov [bp-21],al

     dec13: mov ax,[bp-4]
            add ax,[bp-6]
            and ax,1
            jz anaval
            inc si

    anaval:
            mov bx,7110h
            xor di,di
            mov [bp-24],di   ; nbre d'octets traites par 'suiv'

            mov al,[bp-12]
            and al,2
            jnz dec07

            mov cx,[bp-6]

     dec08: mov al,[si]
            inc si
            mov es:[bx+di],al
            inc di
            loop dec08
            jmp decomp

     dec07: call desanalyse
            mov [bp-18],ax   ; nrep
            call desanalyse
            mov [bp-20],ax   ; taiseq
            add [bp-24],ax

            mov al,[bp-21]
            test al,1
            jz dec09
            inc si
            inc word ptr [bp-24]
            xor al,al
            mov [bp-21],al
     dec09: mov ax,si

     dec10: mov si,ax
            mov cx,[bp-20]
     dec11: mov dl,[si]
            inc si
            mov es:[bx+di],dl
            inc di
            loop dec11
            dec word ptr [bp-18]
            jnz dec10

            mov ax,[bp-6]
            dec ax
            cmp [bp-24],ax
            jc dec07


    decomp: nop
            mov al,[bp-21]
            or al,al
            jz dec16
            inc si
     dec16: mov [bp-44],si
            push es
            pop ds
            mov ax,6000h
            mov es,ax
            mov di,[bp-8]    ; indice de destination
            mov bx,320
            mov ax,[bp-10]
            mul bx
            add di,ax
            mov bx,4140h     ; indice source des couleurs
            dec bx           ; est incremente la 1ere fois
            mov si,7110h     ; indice source des quantit‚s
            xor ax,ax
            mov [bp-26],ax   ; quantite couleur actuelle
            mov [bp-24],ax   ; compteur de 1/2 octets lus par 'suiv' (inutile)
            mov [bp-22],ax   ; bp-21 => 0 leture gauche , 1 droite

            mov al,[bp-11]
            mov ah,al
            shl al,1
            add al,ah
            xor ah,ah
            call dec14

            jmp decom0
            jmp decom1
            jmp decom2
            jmp decom3
            jmp decom4
            jmp decom5
            jmp decom6
            jmp decom7
            jmp decom8
            jmp decom9
            jmp decom10
            jmp decom11
            jmp decom12
            jmp decom13
            jmp decom14
            jmp decom15
            jmp decom16
            jmp decom17
            jmp decom18

     dec14: pop cx           ; offset du 'jmp decom0'
            add cx,ax
            push cx
            ret              ; branchement sur le jmp correspondant au numero
                             ; de compression


    decom0: mov ax,6000h
            mov es,ax
            mov di,[bp-8]    ; indice de destination
            mov bx,320
            mov ax,[bp-10]
            mul bx
            add di,ax
            mov [bp-44],si
            call incr_taix
            mov ax,[bp-16]
            
     com01: mov cx,[bp-14]
            push di
     com00: call suiv
            mov es:[di],dl
            inc di
            loop com00
            pop di
            add di,320
            dec ax
            jnz com01

            mov ax,[bp-44]
            add ax,[bp-4]
            test ax,1
            jz com02
            inc ax
     com02: mov [bp-44],ax
            jmp fin

    decom1: call incr_taix
            mov ax,[bp-16]       ; taiy
     com11: mov cx,[bp-14]       ; taix
     com12: call csuiv
            mov es:[di],dl
            inc di
            loop com12
            add di,320
            dec ax
            jz ko
            mov cx,[bp-14]
     com13: call csuiv
            dec di
            mov es:[di],dl
            loop com13
            add di,320
            dec ax
            jnz com11
        ko: jmp fin

    decom2: ;call correrr
            mov ax,[bp-14]
     com21: mov cx,[bp-16]
     com22: call csuiv
            mov es:[di],dl
            add di,320
            loop com22
            inc di
            dec ax
            jz com24
            mov cx,[bp-16]
     com23: call csuiv
            sub di,320
            mov es:[di],dl
            loop com23
            inc di
            dec ax
            jnz com21
     com24: jmp fin


    decom3: mov word ptr [bp-28],2
            call horizontal
            jmp fin

    decom4: mov word ptr [bp-28],2
            call vertical
            jmp fin

    decom5: mov word ptr [bp-28],3
            call horizontal
            jmp fin


    decom6: mov word ptr [bp-28],4
            call vertical
            jmp fin

    decom7: call incr_taix
            mov ax,[bp-16]   ; taiy
     com70: push di
            mov cx,[bp-14]   ; taix
     com71: call csuiv
            mov es:[di],dl
            inc di
            loop com71
            pop di
            add di,320
            dec ax
            jnz com70
            jmp fin

    decom8: ;call correrr
            mov ax,[bp-14]
     com80: push di
            mov cx,[bp-16]
     com81: call csuiv
            mov es:[di],dl
            add di,320
            loop com81
            pop di
            inc di
            dec ax
            jnz com80
            jmp fin

    decom9: mov word ptr [bp-28],4
            call horizontal
            jmp fin

   decom10: mov word ptr [bp-28],6
            call horizontal
            jmp fin

   decom11: xor ax,ax
            mov [bp-38],ax   ; compteur vertical
            mov [bp-40],ax   ; compteur horizontal
            mov [bp-30],320
            mov word ptr [bp-34],-1
            dec word ptr [bp-14]
            dec word ptr [bp-16]
            jmp dec21

     dec17: call csuiv
            mov es:[di],dl
     dec27: call increments
            mov ax,[bp-38]
            or ax,ax
            jz dec18
            mov ax,[bp-40]
           cmp ax,[bp-14]
            jz dec19
            inc ax
            mov [bp-40],ax
            dec word ptr [bp-38]
            jmp dec17

     dec18: call NIH
            call NIV
            mov ax,[bp-40]
            cmp ax,[bp-14]
            jz dec20
            inc ax
            mov [bp-40],ax
            add di,320
            jmp dec21

     dec19: add di,320
            inc word ptr [bp-38]
            call NIH
            call NIV
            call increments
            call csuiv
            mov es:[di],dl
            mov ax,[bp-38]
            cmp ax,[bp-16]
            je dec22
            jmp dec26

     dec20: call increments
            add di,320
            inc word ptr [bp-38]
            jmp dec21

     dec21: call csuiv
            mov es:[di],dl
     dec26: call increments
            mov ax,[bp-40]
            or ax,ax
            jz dec23
            mov ax,[bp-38]
            cmp ax,[bp-16]
            jz dec24
            inc ax
            mov [bp-38],ax
            dec word ptr [bp-40]
            jmp dec21

     dec22: jmp fin

     dec23: call NIH
            call NIV
            mov ax,[bp-38]
            cmp ax,[bp-16]
            je dec25
            inc ax
            mov [bp-38],ax
            inc di
            jmp dec17

     dec24: call NIH
            call NIV
            call increments
            inc di
            inc word ptr [bp-40]
            call csuiv
            mov es:[di],dl
            mov ax,[bp-40]
            cmp ax,[bp-14]
            je dec22
            jmp dec27

     dec25: call increments
            inc di
            inc word ptr [bp-40]
            jmp dec17

   decom12: call incr_taix
            mov ax,1
            mov [bp-28],ax
            mov [bp-34],ax
            mov ax,320
            mov [bp-30],ax
            mov ax,[bp-16]
            mov [bp-32],ax
            mov ax,[bp-14]
            mov [bp-36],ax
            call diag
            jmp fin

   decom13: call incr_taix
            mov ax,[bp-14]
            mov [bp-28],ax
            mov ax,1
            mov [bp-30],ax
            mov ax,[bp-14]
            mov [bp-32],ax
            mov ax,320
            mov [bp-34],ax
            mov ax,[bp-16]
            mov [bp-36],ax
            call diag
            jmp fin

   decom14: mov ax,1
            mov [bp-28],ax
            ;mov ax,1
            mov [bp-30],ax
            mov ax,[bp-14]
            mov [bp-32],ax
            mov ax,320
            mov [bp-34],ax
            mov ax,[bp-16]
            mov [bp-36],ax
            call diag
            jmp fin

   decom15: call incr_taix
            mov ax,2
            mov [bp-28],ax
            mov ax,320
            mov [bp-30],ax
            mov ax,[bp-16]
            mov [bp-32],ax
            mov ax,1
            mov [bp-34],ax
            mov ax,[bp-14]
            mov [bp-36],ax
            call diag
            jmp fin

   decom16: mov ax,3
            mov [bp-28],ax   ; profondeur
            mov ax,1
            mov [bp-30],ax   ; increment vertical
            mov ax,[bp-14]   ; taix
            mov [bp-32],ax   ; limite verticale
            mov ax,320
            mov [bp-34],ax   ; increment horizontal
            mov ax,[bp-16]   ; taiy
            mov [bp-36],ax   ; limite horizontale
            call diag
            jmp fin

   decom17: call incr_taix
            mov ax,3
            mov [bp-28],ax
            mov ax,320
            mov [bp-30],ax
            mov ax,[bp-16]
            mov [bp-32],ax
            mov ax,1
            mov [bp-34],ax
            mov ax,[bp-14]
            mov [bp-36],ax
            call diag
            jmp fin

   decom18: call incr_taix
            mov ax,5
            mov [bp-28],ax
            mov ax,320
            mov [bp-30],ax
            mov ax,[bp-16]
            mov [bp-32],ax
            mov ax,1
            mov [bp-34],ax
            mov ax,[bp-14]
            mov [bp-36],ax
            call diag
            jmp fin


       fin: dec byte ptr [bp-2]
            jz fin2
            mov si,[bp-44]
            mov ax,[bp+8]
            mov ds,ax
            jmp dec01

      fin2: mov ax,7000h
            mov ds,ax
            mov di,4138h
            mov ax,[bp-50]
            mov [di],ax
            mov ax,[bp-52]
            mov [di+2],ax
            mov ax,[bp-54]
            mov [di+4],ax
            mov ax,[bp-56]
            mov [di+6],ax
            mov sp,bp
            pop bp
            pop ds
            ret 4

decp endp

desanalyse proc near
         xor ax,ax
         call suiv
         cmp dl,15
         jne des03
  des04: call suiv
         add ax,dx
         cmp dl,15
         je des04
         mov dl,15
         mul dl
         call suiv
  des03: add ax,dx
         ret
desanalyse endp

suiv proc near
       mov dl,[si]
       mov dh,[bp-21]
       or dh,dh
       jz suiv0
       and dl,15
       inc si
       inc word ptr [bp-24]
       xor dh,dh
       jmp suiv1
suiv0: shr dl,1
       shr dl,1
       shr dl,1
       shr dl,1
       not dh
suiv1: mov [bp-21],dh
       xor dh,dh
       ret
suiv endp

csuiv proc near      ; modifie bx,dx,[bp-26] + si,[bp-21],[bp-24]
csuiv02: mov dx,[bp-26]
         or dx,dx
         jz csuiv01
         dec dx
         mov [bp-26],dx
         mov dl,[bx]
         xor dh,dh
         ret
csuiv01: call suiv
         xor dh,dh
         add [bp-26],dx
         cmp dl,15
         jz csuiv01
         inc bx
         jmp csuiv02
csuiv endp

NIH proc near
         push ax
         mov ax,[bp-34]
         neg ax
         mov [bp-34],ax
         pop ax
         ret
NIH endp

NIV proc near
         push ax
         mov ax,[bp-30]
         neg ax
         mov [bp-30],ax
         pop ax
         ret
NIV endp

TF1 proc near
          mov ax,[bp-30]
          mov dx,[bp-28]
          inc dx
          add [bp-38],dx     ; Compteur vertical
          mul dx
          add di,ax
          ret
TF1 endp

TF2 proc near
          mov cx,[bp-28]
          inc cx
          add [bp-38],cx
    TF20: call csuiv
          mov es:[di],dl
          add di,[bp-30]
          Loop TF20
TF2 endp

TFP proc near
; Terminaison du bloc lorsque compteur y = taille y
        push ax
        mov ax,[bp-32]       ; Taille verticale
        sub ax,[bp-38]       ; Compteur vertical
        jz TFP0
        mov cx,[bp-28]       ; Profondeur
        inc cx
        cmp ax,cx
        jnc TFP1
        dec ax
        mov [bp-28],ax
  TFP1: pop ax
        ret

  TFP0: pop ax
        pop ax
        ret                  ; Retour au programme principal
TFP endp

increments proc near
           add di,[bp-34]
           add di,[bp-30]
           ret
increments endp

diag proc near
          xor ax,ax
          mov [bp-38],ax     ; Compteur vertical
          mov [bp-40],ax     ; Compteur horizontal
          ;mov ax,[bp-36]
          ;test ax,1
          ;jz diag30
          ;inc word ptr [bp-36]
  diag30: dec word ptr [bp-36]  ; d‚part de 0 donc on decremente les limites
          ;dec word ptr [bp-32]
  diag10: call TFP           ; Test Final de Profondeur
  diag00: mov cx,[bp-28]
          inc cx
          call NIH           ;
  diag03: call csuiv
          mov es:[di],dl
          call NIH           ; Negate Increment Horizontal
          call increments
          Loop diag03

          call NIV           ; Maintenant on remonte
          add di,[bp-30]
          mov cx,[bp-28]     ; Profondeur
          inc cx

  diag04: call csuiv
          mov es:[di],dl
          call NIH           ; Negate Increment Horizontal
          call increments
          Loop diag04

          call NIH
          call NIV
          call increments    ; DI retourne sur le point precedent
          inc word ptr [bp-40]  ; Compteur horizontal
          mov ax,[bp-40]
          inc ax             ; AX sur la premiere colonne suivante
          cmp [bp-36],ax
          jnc diag12
          call TF1
          jmp diag05
  diag12: add di,[bp-34]     ; DI sur le premier point suivant
          mov [bp-40],ax
          inc ax             ; AX sur la seconde colonne suivante
          cmp [bp-36],ax
          jnc diag13
          call TF2
          jmp diag05
  diag13: jmp diag00

;-----------------------------------------------------------------;
; TF1: et TF2: correspondent aux deux cas de terminaison de ligne ;
;-----------------------------------------------------------------;

  diag05: call TFP
  diag07: mov cx,[bp-28]
          inc cx
  diag08: call csuiv
          mov es:[di],dl
          call NIH
          call increments
          Loop diag08

          call NIV           ; Vers le haut
          add di,[bp-30]
          mov cx,[bp-28]
          inc cx
  diag09: call csuiv
          mov es:[di],dl
          call NIH
          call increments
          Loop diag09

          call NIH
          call NIV
          call increments
          dec word ptr [bp-40]
          mov ax,[bp-40]
          or ax,ax
          jz TF1_call
          add di,[bp-34]     ; DI sur le premier point suivant
          dec ax
          mov [bp-40],ax
          or ax,ax
          jz TF2_call
          call NIH
          jmp diag07

TF1_call: call TF1
          call NIH
          jmp diag10

TF2_call: call TF2
          call NIH
          jmp diag10
diag endp

horizontal proc near
            ;call correrr
            ;xor di,di
            ;xor si,si         deja initialises
            call incr_taix
            mov ax,[bp-16]
            dec ax
            mov cx,320
            mul cx
            add ax,[bp-14]
            add ax,di
            mov [bp-48],ax   ; Octet de limite
      newn: mov ax,[bp-28]
            dec ax
            mul cx
            add ax,di
       do1: cmp ax,[bp-48]
            jc hor00
        un: dec word ptr [bp-28]
            jnz newn
            ret
     hor00: mov cx,[bp-14]
    retour: push cx
            mov cx,[bp-28]
     sensd: call csuiv
            mov es:[di],dl
            add di,320
            loop sensd
            pop cx
            inc di
            dec cx
            jz flinf1
            push cx
            mov cx,[bp-28]
     sensm: call csuiv
            sub di,320
            mov es:[di],dl
            loop sensm
            pop cx
            dec cx
            jz flsup1
            inc di
            jmp retour
     nihil: push cx
            mov cx,[bp-28]
    sensd2: call csuiv
            mov es:[di],dl
            add di,320
            loop sensd2
            pop cx
            dec di
            dec cx
            jz flinf2
            push cx
            mov cx,[bp-28]
    sensm2: call csuiv
            sub di,320
            mov es:[di],dl
            loop sensm2
            dec di
            pop cx
            dec cx
            jnz nihil
            mov cx,320
            inc di
            mov ax,[bp-28]
            mul cx
            add di,ax
            jmp newn

    flinf2: mov cx,320
            inc di
            mov ax,[bp-28]
            dec ax
            mul cx
            add ax,di
            cmp ax,[bp-48]
            jc hor00
            jmp un

     hor01: mov cx,[bp-14]
            jmp nihil

    flinf1: mov cx,320
     newn2: mov ax,[bp-28]
            dec ax
            mul cx
            add ax,di
            cmp ax,[bp-48]
            jc hor01
      deux: dec word ptr [bp-28]
            jnz newn2
            ret
    flsup1: mov cx,320
            mov ax,[bp-28]
            mul cx
            add di,ax
            sub ax,cx
            add ax,di
            cmp [bp-48],ax
            jnc hor01
            jmp deux
horizontal endp

vertical proc near
            ;CALL correrr
            xor ax,ax
            mov [bp-40],ax
            mov ax,[bp-16]
            dec ax
            mov cx,320
            mul cx
            add ax,[bp-14]
            add ax,di
            mov [bp-48],ax
       hcp: mov cx,[bp-16]
       lo1: mov ax,[bp-40]
            add ax,[bp-28]
            cmp ax,[bp-14]
            jbe rien
            dec word ptr [bp-28]
            jnz lo1
            ret
      rien: push cx
            mov cx,[bp-28]
            add [bp-40],cx
     sensp: call csuiv
            mov es:[di],dl
            inc di
            loop sensp
            pop cx
            dec cx
            jz fcp
            add di,320
            push cx
            mov cx,[bp-28]
            sub [bp-40],cx
     sensi: call csuiv
            dec di
            mov es:[di],dl
            loop sensi
            pop cx
            dec cx
            jz fci
            add di,320
            jmp rien
       fcp: mov cx,[bp-16]
       lo2: mov ax,[bp-40]
            add ax,[bp-28]
            cmp [bp-14],ax
            jnc noth
            dec word ptr [bp-28]
            jnz lo2
            ret
      noth: push cx
            mov cx,[bp-28]
            add [bp-40],cx
    sensp2: call csuiv
            mov es:[di],dl
            inc di
            loop sensp2
            pop cx
            dec cx
            jz hcp
            sub di,320
            push cx
            mov cx,[bp-28]
            sub [bp-40],cx
    sensi2: call csuiv
            dec di
            mov es:[di],dl
            loop sensi2
            pop cx
            dec cx
            jz hci
            sub di,320
            jmp noth
       hci: mov ax,[bp-28]
            add di,ax
            add [bp-40],ax
            jmp hcp
       fci: mov ax,[bp-28]
            add di,ax
            add [bp-40],ax
            jmp fcp
vertical endp

correrr proc near
            push si
            push ax
            mov si,7110h
        la: mov al,[si]
            inc si
            cmp al,0FFh
            jz la
            dec si
            cmp al,0FEh
            jz ci
            inc al
            mov [si],al
        ci: pop ax
            pop si
            ret
correrr endp

incr_taix proc near
           mov ax,[bp-14]
           and ax,1
           jz in00
           inc word ptr [bp-14]
     in00: ret
incr_taix endp

maj_ttx_tty proc near
             mov dx,[bp-10]  ;  Si  Yrect = Yd‚part alors
             or dx,dx        ;
             jne maj00       ;
             mov dx,[bp-14]  ;
             add [bp-50],dx  ;  TailDessin_X += Taille_X
      maj00: mov dx,[bp-8]
             or dx,dx        ;  Si  Xrect = Xd‚part alors
             jne maj01
             mov dx,[bp-16]
             add [bp-52],dx  ;  TailDessin_Y += Taille_Y
      maj01: ret
maj_ttx_tty endp

PROG   ends
       END decp

