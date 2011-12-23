title affiche_le_dessin_sur_ega_low

prog segment para
       assume CS:prog
affich proc near
; --------------------------------------------------------------------------
; Pile durant la proc affega :
;                       [bp+ 4] Retour
;                       [bp+ 6] y affichage
;                       [bp+ 8] x affichage
;                       [bp+10] offset dessin
;                       [bp+12] segment dessin
; --------------------------------------------------------------------------
       aff: push ds                  ; sauve DS
            push bp                  ; sauve BP
            mov bp,sp
            push bp                  ; sauve SP
            sub sp,112h              ; 112h octets de locales

            mov dx,3CEh      ; 3CFh pointe sur le Bit mask register
            mov al,8
            OUT dx,al

            mov dx,3C4h      ; 3C5h pointe sur le Map mask register
            mov al,2
            OUT dx,al


; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse absolue  7000h:xxxx
; --------------------------------------------------------------------------

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

            mov si,4138h
            mov ax,[si]
            mov [bp-258],ax
            mov ax,[si+2]
            mov [bp-260],ax

; additionne les coordonnees relatives dans les parametres de passage :

            mov ax,[si+4]
            add [bp+8],ax
            mov ax,[si+6]
            add [bp+6],ax

; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse relative [segment : offset]
; --------------------------------------------------------------------------

            mov ax,[bp+12]
            mov ds,ax                ; DS = segment du fichier dessin
            xor ah,ah
            mov si,[bp+10]           ; offset dessin
            mov al,[si]
            mov [bp-140],ax          ; 0 = dessin ; 1 = animation

            mov bx,16
            mov [bp-142],bx          ; par d‚faut
            or al,al
            jz aff00
            mov bl,[si+2]            ; nø de couleur du masque
            mov [bp-142],bx

     aff00: mov ax,[bp+6]      ; Y
            mov bx,80
            mul bx             ; 80 octets par lignes
            mov bx,[bp+8]      ; X
            shl bx,1           ; 2*X car un point du dessin pour deux pixels
            mov [bp+8],bx
            mov cl,3
            shr bx,cl          ; BX := X div 8 , car 8 pixels par octet
            add ax,bx          ; AX contient l'offset du premier octet d'affichage dans le segment video
            mov [bp-100],ax    ; Oct_dep en bp-100

            mov ax,6000h
            mov es,ax                ; ES = segment du dessin decompresse
            mov ax,0A000h
            mov ds,ax                ; DS = segment de la memoire video

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

            xor ax,ax
            mov [bp-114],ax          ; init
            mov [bp-110],ax          ; cmpt y

debut_lig:  mov si,[bp-114]
            mov ax,si
            add ax,320
            mov [bp-114],ax

            mov cx,[bp-108]    ; Nombre de point(s) a afficher
                               ; vaut 4 lorsque tout l'octet va etre afficher
                               ;      3 pour les 6 bits de droite
                               ;      2 pour les 4 bits de droite
                               ;      1 pour les 2 bits de droite

            xor ax,ax
            mov [bp-112],ax        ; compteur en x
            mov ax,[bp-100]
            mov di,ax
            add ax,80
            mov [bp-100],ax
            mov ax,[bp-106]        ; masque binaire tournant

un1:
            xor dx,dx
            mov [bp-246],dx
            mov [bp-248],dx
            mov [bp-250],dx
            mov [bp-252],dx
            xor bx,bx
            jmp un

     aff01: shr al,1
            pop di
            jmp siutd

        un: push di
            mov di,es:[si]         ; 1er bit/pixel du point =
            and di,15              ; no de couleur du point dans la palette
            cmp di,[bp-142]
            jz aff01
            shl di,1
            mov dx,[bp+di-64]      ; DL => 1ere couleur
                                   ; DH => 2nde couleur
            pop di

            or bl,al

            test dl,1              ; Bit BLEU
            jz deux
            or [bp-246],al

      deux: test dl,2              ; Bit VERT
            jz trois
            or [bp-248],al

     trois: test dl,4              ; Bit ROUGE
            jz quatre
            or [bp-250],al

    quatre: test dl,8              ; Bit INTENSITE
            jz siute
            or [bp-252],al
     siute: shr al,1                 ; 2nd bit/pixel du point
                                     ; decalage du masque
            or bl,al

            test dh,1              ; Bit BLEU
            jz deud
            or [bp-246],al

      deud: test dh,2              ; Bit VERT
            jz troid
            or [bp-248],al

     troid: test dh,4              ; Bit ROUGE
            jz quatrd
            or [bp-250],al

    quatrd: test dh,8              ; Bit INTENSITE
            jz siutd
            or [bp-252],al

     siutd: shr al,1                 ; decalage du masque
            inc word ptr [bp-112]    ; compteur x:= compteur x + 1
            inc si
            mov dx,[bp-112]
            cmp dx,[bp-258]
            loopnz un      ; boucle tant que cx<>0 et compteur x < taix

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

            ;mov dx,3C5h        ; registre de selection des couleurs
            xchg bh,dl
            mov al,1
            out dx,al          ; selecte BLEU

            mov al,[bp-246]
            ;mov dl,0CFh
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche

            ;mov dx,3C5h        ; registre de selection des couleurs
            xchg bh,dl
            mov al,2
            out dx,al          ; selecte VERT

            mov al,[bp-248]
            ;mov dl,0CFh
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche

            ;mov dx,3C5h        ; registre de selection des couleurs
            xchg bh,dl
            mov al,4
            out dx,al          ; selecte ROUGE

            mov al,[bp-250]
            ;mov dl,0CFh
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche

            ;mov dx,3C5h        ; registre de selection des couleurs
            xchg bh,dl
            mov al,8
            out dx,al          ; selecte INTENSITE

            mov al,[bp-252]
            ;mov dl,0CFh
            xchg bh,dl
            out dx,al

            mov al,[di]
            mov al,0FFh
            mov [di],al     ; affiche


            mov ax,80h         ; prepare le masque pour l'octet suivant
            mov cx,4
            inc di                  ; octet video suivant
            mov dx,[bp-112]
            cmp dx,[bp-258]
            jc inter_un
            inc word ptr [bp-110]
            mov dx,[bp-110]
            cmp dx,[bp-260]
            jc inter_deb

            mov dx,3CFh
            mov al,255
            out dx,al
            mov dl,0C5h
            out dx,al

            mov sp,bp
            pop bp
            pop ds
            ret 8

inter_deb:  jmp debut_lig
 inter_un: jmp un1

affich endp

PROG ENDS
     END aff



