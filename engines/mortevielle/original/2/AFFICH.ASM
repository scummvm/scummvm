title affiche_le_dessin

; Pile : [bp]    = old bp
;        [bp+2]  = old ds
;        [bp+4]  = ret
;        [bp+6]  = y affichage
;        [bp+8]  = x affichage
;        [bp+10] = offset dessin
;        [bp+12] = segment dessin
;        [bp+14] = Gd

PROG segment para
       assume CS:prog
affich proc near


       aff: push ds                  ; sauve DS
            push bp                  ; sauve BP
            mov bp,sp
            push bp                  ; sauve SP
            sub sp,112h              ; 112h octets de locales

            mov ax,[bp+14]           ; Graph Device
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


     ams01: call trpal
            call trtaycoor
            call trvarrel
            mov bx,[bp+8]      ; X
            shl bx,1           ; 2*X car un point du dessin pour deux pixels
            mov [bp+8],bx
            mov cl,3
            call calc01
            call calcpoidep
     ams03:
     ams04:                  ; calcul du point d‚part [bp-106] :
                             ; poi_dep est un octet tel que le seul bit a 1
                             ; soit la position du premier pixel d'affichage
            mov ax,[bp+8]
            and ax,7         ; reste de la div de X par 8
            mov cl,al
            mov bx,80h       ; 1000 0000b position par defaut pour une
                             ; division par 8 entiere
            shr bx,cl
            mov [bp-106],bx

            shr ax,1                 ; 4 - ( X mod 8 ) div 2 =
            neg ax                   ; nbre de points pour le 1er octet .
            add ax,4                 ; 4 points/couleurs par octets (8 pixels)
            mov [bp-108],ax


    ams05:  call deblig

            mov ax,[bp-106]        ; AX := masque ecran
            mov di,[bp-100]        ; DI := offset dans mem video de l'octet de debut de ligne
            mov dx,di
            call amslit
            mov bx,[bp-102]        ; booleen = ligne courante paire/impaire
            or bx,bx
            jz ams06
            dec word ptr [bp-102]  ; la ligne suivante sera impaire
            sub dx,2000h
            add dx,80              ; DX = octet du debut de la ligne suivante
            jmp ams07
ams06:
            inc word ptr [bp-102]
            add dx,2000h
ams07:
            mov [bp-100],dx        ; DX = octet du debut de la ligne suivante
            jmp ams09

     ams08: shr al,1
            jmp ams25

ams09:                                ; 1er bit/pixel du point
            mov dl,es:[si]         ; no de couleur du point dans la palette
            and dl,15
            cmp dl,[bp-142]
            je ams08
            shl dl,1
            xor dh,dh
            push si
            mov si,dx
            mov dx,[bp+si-64]      ; DL => 1ere couleur
                                   ; DH => 2nde couleur
            pop si

            mov ah,al              ; AL masque positif, AH masque negatif
            not ah

            test dl,1              ; Bit BLEU
            jz ams10
            or [bp-246],al
            jmp ams15
     ams10: and [bp-246],ah

     ams15: test dl,2              ; Bit VERT
            jz ams11
            or [bp-248],al
            jmp ams14
     ams11: and [bp-248],ah

     ams14: test dl,4              ; Bit ROUGE
            jz ams12
            or [bp-250],al
            jmp ams13
     ams12: and [bp-250],ah

     ams13: test dl,8              ; Bit INTENSITE
            jz ams16
            or [bp-252],al
            jmp ams17
     ams16: and [bp-252],ah
     ams17: shr al,1                 ; 2nd bit/pixel du point
                                     ; decalage du masque
            mov ah,al
            not ah

            test dh,1              ; Bit BLEU
            jz ams18
            or [bp-246],al
            jmp ams19
     ams18: and [bp-246],ah

     ams19: test dh,2              ; Bit VERT
            jz ams20
            or [bp-248],al
            jmp ams21
     ams20: and [bp-248],ah

     ams21: test dh,4              ; Bit ROUGE
            jz ams22
            or [bp-250],al
            jmp ams23
     ams22: and [bp-250],ah

     ams23: test dh,8              ; Bit INTENSITE
            jz ams24
            or [bp-252],al
            jmp ams25
     ams24: and [bp-252],ah

     ams25: shr al,1                 ; decalage du masque
            inc word ptr [bp-112]    ; compteur x:= compteur x + 1
            inc si
            mov dx,[bp-112]
            cmp dx,[bp-132]
            loopnz ams26              ; boucle tant que cx<>0 et compteur x < taix
            jmp ams27

     ams26: jmp ams09

ams28:      add dx,4
            cmp dx,[bp-132]
            jc ams30
ams29:      call amslit
            jmp ams09
ams30:      mov dl,[bp-140]
            or dl,dl
            jnz ams29
ams31:
            call annul
            jmp ams09
     ams32: jmp ams05

     ams27:                     ; affichage de l'octet precedement calcule .
                                ; les masques B, V, R et I sont stoque dans
                                ; les locales en BP-246, 248, 250 et 252
            mov cx,4           ; 4 plans couleur
            mov ax,80h         ; prepare le masque pour l'octet suivant
            push cx            ; sauvegarde des registres utilises pour l'affichage
            push ax
            push ds
            mov ax,0B800h
            mov ds,ax          ; segment video

            mov dx,3DDh        ; registre de selectiondes couleurs

            mov al,1
            out dx,al          ; selecte BLEU
            mov cl,[bp-246]
            mov [di],cl     ; affiche

            inc al
            out dx,al          ; selecte ROUGE
            mov cl,[bp-248]
            mov [di],cl

            shl al,1
            out dx,al          ; selecte VERT
            mov cx,[bp-250]
            mov [di],cl

            shl al,1
            out dx,ax          ; selecte INTENSITE
            mov cl,[bp-252]
            mov [di],cl

            pop ds             ; Affichage termine . Restaure les registres
            pop ax
            pop cx
            inc di                  ; octet video suivant
            mov dx,[bp-112]
            cmp dx,[bp-132]
            jc ams28
            inc word ptr [bp-110]
            mov dx,[bp-110]
            cmp dx,[bp-130]
            jc ams32
            jmp fin

; --------------------------------------------------------------------------
;      			affichage sur CGA
; --------------------------------------------------------------------------

     cga01: ;push ds                  ; sauve DS
            ;push es                  ; sauve ES
            mov ax,7000h
            mov ds,ax                ; DS = segment de la palette

; calcul des addresses des motifs :
; - a partir de 6CE40 h et par pas de 404
; - chaque addresse est le deplacement dans le segment 6000h
; - bp-64 : nø15,   bp-62 : nø14,   ... ,   bp-34 : nø0

            mov cx,16
            mov bx,404;
     cga02: mov ax,cx
            dec ax
            mov di,ax
            mul bx
            add ax,0D700h
            shl di,1
            add di,34
            neg di
            mov [bp+di],ax
            loop cga02

            call trtaycoor
            call trvarrel
            mov cl,2
            call calc01

            mov ax,[bp+8]
            and ax,3         ; reste de la div de X par 4
                                     ; 4 - ( X mod 4 )  =
            neg ax                   ; nbre de points pour le 1er octet .
            add ax,4                 ; 4 points/couleurs par octets
            mov [bp-108],ax


     cga05: call deblig

            mov di,[bp-100]        ; DI := offset dans mem video de l'octet de debut de ligne
            mov dx,di
            call cgalit
            mov bx,[bp-102]        ; booleen = ligne courante paire/impaire
            or bx,bx
            jz cga07
            dec word ptr [bp-102]  ; la ligne suivante sera impaire
            sub dx,2000h
            add dx,80              ; DX = octet du debut de la ligne suivante
            jmp cga08

cga06:
            jmp cga14

cga07:
            inc word ptr [bp-102]
            add dx,2000h
cga08:
            mov [bp-100],dx        ; DX = octet du debut de la ligne suivante

cga09:                             ; 1er bit/pixel du point
            mov dl,es:[si]         ; no de couleur du point dans la palette
            cmp dl,[bp-142]
            je cga06

; - bp-64 : nø15,   bp-62 : nø14,   ... ,   bp-34 : nø0

            push ax
            push si
            xor dh,dh
            shl dx,1
            mov si,dx
            add si,34
            neg si
            mov ax,[bp+si]
            mov si,ax              ; offset du bon bloc
            xor bx,bx
            mov ax,[bp-112]        ; cmpt x
            mov dx,es:[si]            ; tax du bloc
     cga10: cmp ax,dx
            jc cga11
            sub ax,dx
            jmp cga10
     cga11: mov bx,ax
            add si,2
            mov dx,es:[si]            ; tay du bloc
            mov ax,[bp-110]        ; cmpt y
     cga12: cmp ax,dx
            jc cga13
            sub ax,dx
            jmp cga12
     cga13: mov dx,es:[si-2]          ; y * tax
            mul dx
            add bx,ax
            add si,2

            mov dl,es:[bx+si]         ; et enfin : la couleur
            xor dh,dh
            pop si
            pop ax
            push cx
            dec cx
            shl cx,1
            shl dx,cl              ; ATTENTION Superbe !!!!!
            mov bx,3
            shl bx,cl
            not bl                 ; DL masque positif, BL masque negatif
            pop cx
            and [bp-246],bl
            or [bp-246],dl

     cga14:
            inc word ptr [bp-112]    ; compteur x:= compteur x + 1
            inc si
            mov dx,[bp-112]
            cmp dx,[bp-132]
            loopnz cga15              ; boucle tant que cx<>0 et compteur x < taix
            jmp cga21

cga15:      jmp cga09

cga16:      add dx,4
            cmp dx,[bp-132]
            jc cga18
cga17:      call cgalit
            jmp cga09
cga18:      mov dl,[bp-140]
            or dl,dl
            jnz cga17
cga19:      xor dx,dx
            mov [bp-246],dx
            jmp cga09
cga20:      jmp cga05

cga21:                             ; affichage de l'octet precedement calcule .
            push ds
            mov ax,0B800h
            mov ds,ax              ; segment video

            mov al,[bp-246]
            mov [di],al            ; affiche

            pop ds                 ; Affichage termine .
            inc di                 ; octet video suivant
            mov cx,4
            mov dx,[bp-112]
            cmp dx,[bp-132]
            jc cga16
            inc word ptr [bp-110]
            mov dx,[bp-110]
            cmp dx,[bp-130]
            jc cga20
            jmp fin

; --------------------------------------------------------------------------
;                       affichage sur EGA
; --------------------------------------------------------------------------
     ega01:
            mov dx,3CEh      ; 3CFh pointe sur le Bit mask register
            mov al,8
            OUT dx,al

            mov dx,3C4h      ; 3C5h pointe sur le Map mask register
            mov al,2
            OUT dx,al

            call trpal
            call trtaycoor
            call trvarrel

            mov ax,[bp+6]      ; Y
            mov bx,80
            mul bx             ; 80 octets par lignes
            mov bx,[bp+8]      ; X
            shl bx,1           ; 2*X car un point du dessin pour deux pixels
            mov [bp+8],bx
            mov cl,3
            shr bx,cl          ; BX := X div 8 , car 8 pixels par octet
            add ax,bx          ; AX contient l'offset du premier octet d'affichage dans le segment video
            mov [bp-100],ax    ; Oct_dep en bp-100

            mov ax,0A000h
            mov ds,ax                ; DS = segment de la memoire video

            call calcpoidep

            xor ax,ax
            mov [bp-114],ax          ; init
            mov [bp-110],ax          ; cmpt y

     ega02: call deblig

            mov ax,[bp-100]
            mov di,ax
            add ax,80
            mov [bp-100],ax
            mov ax,[bp-106]        ; masque binaire tournant

     ega03: call annul
            xor bx,bx
            jmp ega04

     ega05: shr al,1
            pop di
            jmp ega06

     ega04: push di
            mov di,es:[si]         ; 1er bit/pixel du point =
            and di,15              ; no de couleur du point dans la palette
            cmp di,[bp-142]
            jz ega05
            shl di,1
            mov dx,[bp+di-64]      ; DL => 1ere couleur
                                   ; DH => 2nde couleur
            pop di

            or bl,al

            test dl,1              ; Bit BLEU
            jz ega07
            or [bp-246],al

     ega07: test dl,2              ; Bit VERT
            jz ega08
            or [bp-248],al

     ega08: test dl,4              ; Bit ROUGE
            jz ega09
            or [bp-250],al

     ega09: test dl,8              ; Bit INTENSITE
            jz ega10
            or [bp-252],al
     ega10: shr al,1                 ; 2nd bit/pixel du point
                                     ; decalage du masque
            or bl,al

            test dh,1              ; Bit BLEU
            jz ega11
            or [bp-246],al

     ega11: test dh,2              ; Bit VERT
            jz ega12
            or [bp-248],al

     ega12: test dh,4              ; Bit ROUGE
            jz ega13
            or [bp-250],al

     ega13: test dh,8              ; Bit INTENSITE
            jz ega06
            or [bp-252],al

     ega06: shr al,1                 ; decalage du masque
            inc word ptr [bp-112]    ; compteur x:= compteur x + 1
            inc si
            mov dx,[bp-112]
            cmp dx,[bp-132]
            loopnz ega04      ; boucle tant que cx<>0 et compteur x < taix

     ; affichage de l'octet precedement calcule .
     ; les masques B, V, R et I sont stoques dans
     ; les locales en BP-246, 248, 250 et 252
     ; Le masque Noir est en BL

            mov dx,3C5h      ; Selecte toutes les couleurs
            mov bh,dl
            mov al,15
            out dx,al
            mov dl,0CFh      ; Bit mask Noir
            mov al,bl
            out dx,al
            mov al,[di]
            xor al,al
            mov [di],al

            xchg bh,dl
            mov al,1
            out dx,al          ; selecte BLEU

            mov al,[bp-246]
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche

            xchg bh,dl
            mov al,2
            out dx,al          ; selecte VERT

            mov al,[bp-248]
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche

            xchg bh,dl
            mov al,4
            out dx,al          ; selecte ROUGE

            mov al,[bp-250]
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche

            xchg bh,dl
            mov al,8
            out dx,al          ; selecte INTENSITE

            mov al,[bp-252]
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche


            mov ax,80h         ; prepare le masque pour l'octet suivant
            mov cx,4
            inc di                  ; octet video suivant
            mov dx,[bp-112]
            cmp dx,[bp-132]
            jc ega14
            inc word ptr [bp-110]
            mov dx,[bp-110]
            cmp dx,[bp-130]
            jc ega15

            mov dx,3CFh
            mov al,255
            out dx,al
            mov dl,0C5h
            out dx,al

            jmp fin

     ega15: jmp ega02
     ega14: jmp ega03

; --------------------------------------------------------------------------
;                       affichage sur TANDY
; --------------------------------------------------------------------------
; Variables locales :
;                [bp- 100] octet de deplacement du debut de la ligne courante
;                [bp- 110] compteur relatif en y
;                [bp- 112] compteur relatif en x
;                [bp- 114] indice dans le dessin source
;                [bp- 120] compteur absolu en y
;                [bp- 122] compteur absolu en x
;                [bp- 142] numero du masque
;                [bp- 250] couleur de l'octet a effacer
; --------------------------------------------------------------------------
     tan01: call trpal
            call trtaycoor
            call trvarrel

            mov ax,[bp+6]      ; Y
            shr ax,1
            shr ax,1
            mov bx,160
            mul bx             ; Y div 4 * 160
            mov bx,ax
            mov ax,[bp+6]      ; Y
            and ax,3
            mov cx,2000h
            mul cx
            add ax,bx
            mov bx,[bp+8]      ; X
            shr bx,1           ; X div 2 car 2 pixels par octets
            add ax,bx          ; AX contient l'offset du premier octet d'affichage dans le segment video
            mov [bp-100],ax    ; Oct_dep en bp-100

            mov ax,0B800h
            mov ds,ax                ; DS = segment de la memoire video

            xor ax,ax
            mov [bp-110],ax          ; cmpt y
            mov [bp-114],ax          ; offset source
            mov [bp-108],ax          ; compatibilite avec proc deblig

            mov ax,[bp+6]
            mov [bp-120],ax
            mov ax,[bp+8]
            mov [bp-122],ax

    tan02:  call deblig

            mov bx,[bp-100]
            mov di,bx
            inc word ptr [bp-120]
            mov ax,[bp-120]
            and ax,3
            je tan03
            add bx,2000h
            jmp tan04
     tan03: sub bx,6000h
            add bx,160
     tan04: mov [bp-100],bx

     tan05: xor dx,dx
            mov [bp-250],dx
            xor bx,bx
            jmp tan07

     tan06: pop di
            mov al,[di]
            jmp tan08

     tan07: mov cl,4
            push di
            mov di,es:[si]         ; 1er bit/pixel du point =
            and di,15              ; no de couleur du point dans la palette
            cmp di,[bp-142]
            jz tan06
            shl di,1
            mov dx,[bp+di-64]      ; DL => 1ere couleur
            pop di
            xor al,al
            mov al,dl
            shl dh,cl
            add al,dh
            mov [bp-250],al

     tan08:
            inc word ptr [bp-112]    ;
            inc word ptr [bp-112]    ; compteur x:= compteur x + 2
            inc si
            inc si       ; on perd un point !!!

     ; affichage de l'octet precedement calcule .

            mov [di],al
            inc di                  ; octet video suivant

            mov dx,[bp-112]
            cmp dx,[bp-132]
            jc tan10
            inc word ptr [bp-110]
            mov dx,[bp-110]
            cmp dx,[bp-130]
            jc tan09

            jmp fin

     tan09: jmp tan02
     tan10: jmp tan05
; --------------------------------------------------------------------------
;      			affichage sur HERCULE / AT&T400
; --------------------------------------------------------------------------
; Variables locales :
;                [bp- 100] octet de deplacement du debut de la ligne courante
;                [bp- 102] nombre a ajouter donnant la ligne de dessous
;                [bp- 104] masque de d‚but de ligne
;                [bp- 106] cl de d‚but de ligne
;                [bp- 110] compteur relatif en y
;                [bp- 112] compteur relatif en x
;                [bp- 114] indice dans le dessin source
;                [bp- 120] compteur absolu en y
;                [bp- 122] compteur absolu en x
;                [bp- 130] taille en y du dessin
;                [bp- 132] taille en x du dessin
;                [bp- 140] bool = est un dessin ? (./animation)
;                [bp- 142] numero du masque
; --------------------------------------------------------------------------


; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse absolue  7000h:xxxx
; --------------------------------------------------------------------------

; transfert de la palette dans les locales en [BP-64] :

     her01: mov ax,7000h
            mov ds,ax                ; DS = segment de la palette
            mov si,2                 ; SI = deplacement de la palette
            push ss
            pop es                   ; ES = destination : locales sur la pile
            mov di,bp
            sub di,64                ; DI = offset dest.
            mov cx,16                ; 16 bytes de palette

     her02: mov al,[si]
            inc si
            inc si
            mov es:[di],al
            inc di
            loop her02

            call trtaycoor
            call trvarrel

            mov ax,[bp+6]      ; Y
            shl ax,1
            mov cx,ax
            shr ax,1
            shr ax,1
            mov bx,80
            mul bx             ; Y div 4 * 80
            mov bx,ax
            and cx,3
            mov ax,2000h
            mul cx
            add ax,bx
            mov bx,[bp+8]      ; X
            mov cl,2
            shr bx,cl          ; X div 4 car 4 points par octets
            add ax,bx          ; AX contient l'off du premier octet d'affichage
                               ; dans le segment video
            mov [bp-100],ax    ; Oct_dep en bp-100
            mov di,ax

            mov cx,[bp+8]      ; X
            and cx,3
            neg cx
            add cx,3
            shl cx,1
            mov [bp-106],cl
            mov al,3
            shl al,cl
            not al
            mov [bp-104],al

            mov ax,0B000h
            mov ds,ax                ; DS = segment de la memoire video

            xor ax,ax
            mov [bp-110],ax          ; cmpt y
            mov [bp-114],ax          ; offset source
            mov [bp-108],ax          ; compatibilite ...

            mov ax,[bp+6]
            shl ax,1
            mov [bp-120],ax
            mov ax,[bp+8]
            mov [bp-122],ax

     her03:
            call calcaddlig
            call deblig

            mov cl,[bp-106]        ; decalage pixels dans byte
            mov ch,[bp-104]        ; masque noir


     her04: mov al,[di]
            add di,dx
            mov ah,[di]
            sub di,dx


     her05: mov bl,es:[si]         ; 1er bit/pixel du point =
            inc si
            and bx,15              ; no de couleur du point dans la palette
            cmp bl,[bp-142]
            je her06
            push si
            mov si,bx
            mov bl,[bp+si-64]
            pop si
            and al,ch
            and ah,ch
            mov bh,bl
            and bl,3
            shl bl,cl
            or al,bl
            shr bh,1
            shr bh,1
            shl bh,cl
            or ah,bh

     her06: or cl,cl
            jz her08
            dec cl
            dec cl
            ror ch,1
            ror ch,1

     her07:
            inc word ptr [bp-112]    ; compteur x ++
            mov bx,[bp-112]
            cmp bx,[bp-132]
            jc her05
            call affher
     her10: inc word ptr [bp-110]
            mov bx,[bp-110]
            cmp bx,[bp-130]
            jc her09

            jmp fin

     her08: call affher
            mov ch,3Fh
            mov cl,6
            inc word ptr [bp-112]    ; compteur x ++
            mov bx,[bp-112]
            cmp bx,[bp-132]
            jnc her10
            jmp her05

     her09: mov di,[bp-100]
            add di,dx
            inc word ptr [bp-120]
            call calcaddlig
            add di,dx
            inc word ptr [bp-120]
            mov [bp-100],di
            jmp her03


       fin: mov sp,bp
            pop bp
            pop ds
            ret 10

affich endp

amslit proc near
            push ds
            push ax
            push dx
            mov ax,0b800h
            mov ds,ax
            mov dx,3DEh
            mov al,0
            out dx,al
            mov ah,[di]
            mov [bp-246],ah
            inc ax
            out dx,al
            mov ah,[di]
            mov [bp-248],ah
            inc ax
            out dx,al
            mov ah,[di]
            mov [bp-250],ah
            inc ax
            out dx,al
            mov ah,[di]
            mov [bp-252],ah
            pop dx
            pop ax
            pop ds
            ret
amslit endp

cgalit proc near
            push ds
            push ax
            push dx
            mov ax,0b800h
            mov ds,ax
            mov ah,[di]
            mov [bp-246],ah
            pop dx
            pop ax
            pop ds
            ret
cgalit endp

trpal proc near
; transfert de la palette dans les locales en [BP-64] :

            mov ax,7000h
            mov ds,ax                ; DS = segment de la palette
            mov si,2                 ; SI = deplacement de la palette
            push ss
            pop es                   ; ES = destination : locales sur la pile
            mov di,bp
            sub di,64                ; DI = offset dest.
            mov cx,16                ; 16 mots de palette
            cld
          rep movsw                  ; transfert
            ret
trpal endp

trtaycoor proc near
; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse absolue  7000h:xxxx
; --------------------------------------------------------------------------

            mov si,4138h
            mov ax,[si]
            mov [bp-132],ax          ; taix du dessin
            mov ax,[si+2]
            mov [bp-130],ax          ; taiy du dessin

; additionne les coordonnees relatives dans les parametres de passage :

            mov ax,[si+4]            ; coorx
            add [bp+8],ax
            mov ax,[si+6]            ; coory
            add [bp+6],ax
            ret
trtaycoor endp

trvarrel proc near
; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse relative  yyyy:xxxx
; --------------------------------------------------------------------------

            mov ax,[bp+12]
            mov ds,ax                ; DS = segment du fichier dessin
            mov ax,6000h
            mov es,ax                ; ES = segment du dessin decompresse

            mov si,[bp+10]           ; offset dessin
            mov al,[si]
            mov [bp-140],al          ; 0 = dessin ; 1 = animation

            mov bx,16
            mov [bp-142],bx          ; par d‚faut
            or al,al
            jz trv01
            mov bl,[si+2]            ; nø de couleur du masque
            mov [bp-142],bx

     trv01: ret

trvarrel endp

calc01 proc near
            mov ax,[bp+6]      ; Y
            shr ax,1           ; Y/2 lignes paires a afficher
            mov bx,80
            mul bx             ; 80 octets par lignes
            mov bx,[bp+8]      ; X
            shr bx,cl          ; BX := X div 4/8 , car 4/8 pixels par octet
            add ax,bx          ; AX contient l'offset du premier octet d'affichage dans le segment video
            mov [bp-100],ax    ; Oct_dep en bp-100
            mov [bp-102],dx    ; 0 si affichage courant sur ligne paire,
                               ; 1 si affichage sur ligne impaire

            mov ax,[bp+6]      ; Y
            and ax,1           ; masque l'imparite de Y
            jz cc01
            inc word ptr [bp-102]    ; 1 car premier affichage sur ligne impaire
            mov ax,2000h
            add [bp-100],ax          ; memoire video pour lignes impaires

      cc01: xor si,si
            mov [bp-110],si          ; cmpt y
            mov [bp-114],si          ; Octet de debut de la ligne
            ret
calc01 endp

calcpoidep proc near
;-----------------------------------------------------------------------------
;  - Calcul du point d‚part [bp-106] : poi_dep est un octet tel que le seul
;     bit a 1 soit la position du premier pixel d'affichage
;-----------------------------------------------------------------------------

            mov ax,[bp+8]
            and ax,7         ; reste de la div de X par 8
            mov cl,al
            mov bx,80h       ; 1000 0000b position par defaut pour une
                             ; division par 8 entiere
            shr bx,cl
            mov [bp-106],bx

            shr ax,1                 ; 4 - ( X mod 8 ) div 2 =
            neg ax                   ; nbre de points pour le 1er octet .
            add ax,4                 ; 4 points/couleurs par octets (8 pixels)
            mov [bp-108],ax

            ret
calcpoidep endp

annul proc near
        xor dx,dx
        mov [bp-246],dx
        mov [bp-248],dx
        mov [bp-250],dx
        mov [bp-252],dx
        ret
annul endp

deblig proc near
    mov si,[bp-114]
    mov ax,si
    add ax,320
    mov [bp-114],ax

    mov cx,[bp-108]          ; Nombre de point(s) a afficher
                             ; vaut 4 lorsque tout l'octet va etre afficher
                             ;      3 lorsque les 6 bits de droite vont etre re-afficher
                             ;      2 lorsque les 4 bits de droite vont etre re-afficher
                             ;      1 lorsque les 2 bits de droite vont etre re-afficher
    xor ax,ax
    mov [bp-112],ax          ; compteur en x
    ret
deblig endp

calcaddlig proc near
      mov dx,[bp-120]
      and dx,3
      cmp dx,3
      je cc02
      mov dx,2000h
      ret

cc02: mov dx,6000h
      neg dx
      add dx,80
      ret
calcaddlig endp

affher proc near
        mov [di],al
        add di,dx
        mov [di],ah
        inc di                  ; octet video suivant
        mov ah,[di]
        sub di,dx
        mov al,[di]
        ret
affher endp

PROG ENDS
     END aff


