title affiche_le_dessin

prog segment para
       assume CS:prog
affich proc near           ; pile appel :
                                       ; Retour
                                       ; y affichage
                                       ; x affichage
                                       ; offset dessin
                                       ; segment dessin


       aff: push ds                  ; sauve DS
            push bp                  ; sauve BP
            mov bp,sp
            push bp                  ; sauve SP
            sub sp,112h              ; 112h octets de locales



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
            mov ax,[si+4]
            add [bp+8],ax
            mov ax,[si+6]
            add [bp+6],ax

            mov ax,[bp+12]
            mov ds,ax                ; DS = segment du fichier dessin
            xor ah,ah
            mov si,[bp+10]           ; offset dessin
            mov al,[si]
            mov [bp-140],ax          ; 0 = dessin ; 1 = animation
            mov si,[bp+10]           ; offset dessin
            mov bx,16
            mov [bp-142],bx          ; par d‚faut
            or al,al
            jz aff00
            mov bl,[si+2]            ; nø de couleur du masque
            mov [bp-142],bx

     aff00: mov ax,6000h
            mov es,ax                ; ES = segment du dessin decompresse
    datpal:
            mov ax,[bp+6]      ; Y
            shr ax,1           ; Y/2 lignes paires a afficher
            mov bx,80
            mul bx             ; 80 octets par lignes
            mov bx,[bp+8]      ; X
            shl bx,1           ; 2*X car un point du dessin pour deux pixels
            mov [bp+8],bx
            mov cl,3
            shr bx,cl          ; BX := X div 8 , car 8 pixels par octet
            add ax,bx          ; AX contient l'offset du premier octet d'affichage dans le segment video
            mov [bp-100],ax    ; Oct_dep en bp-100
            mov [bp-102],dx    ; 0 si affichage courant sur ligne paire,
                               ; 1 si affichage sur ligne impaire

            mov ax,[bp+6]      ; Y
            and ax,1           ; masque l'imparite de Y
            jz prem_lig_paire
            inc word ptr [bp-102]    ; 1 car premier affichage sur ligne impaire
            mov ax,2000h
            add [bp-100],ax          ; memoire video pour lignes impaires
prem_lig_paire:              ; calcul du point d‚part [bp-106] :
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

            xor ax,ax                
            mov [bp-110],ax          ; cmpt y
            mov [bp-114],ax          ; init

debut_lig:  mov si,[bp-114]
            mov ax,si
            add ax,320
            mov [bp-114],ax

            mov cx,[bp-108]          ; Nombre de point(s) a afficher
                                     ; vaut 4 lorsque tout l'octet va etre afficher
                                     ;      3 lorsque les 6 bits de droite vont etre re-afficher
                                     ;      2 lorsque les 4 bits de droite vont etre re-afficher
                                     ;      1 lorsque les 2 bits de droite vont etre re-afficher
            xor ax,ax
            mov [bp-112],ax        ; compteur en x
            mov ax,[bp-106]        ; AX := masque ecran
            mov di,[bp-100]        ; DI := offset dans mem video de l'octet de debut de ligne
            mov dx,di
            call litbyte
            mov bx,[bp-102]        ; booleen = ligne courante paire/impaire
            or bx,bx
            jz lig_paire
            dec word ptr [bp-102]  ; la ligne suivante sera impaire
            sub dx,2000h
            add dx,80              ; DX = octet du debut de la ligne suivante
            jmp sui2
lig_paire:
            inc word ptr [bp-102]
            add dx,2000h
sui2:
            mov [bp-100],dx        ; DX = octet du debut de la ligne suivante
            jmp un

     aff01: shr al,1
            jmp siutd

un:                                ; 1er bit/pixel du point
            mov dl,es:[si]         ; no de couleur du point dans la palette
            and dl,15
            cmp dl,[bp-142]
            je aff01
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
            jz deux1
            or [bp-246],al
            jmp deux
     deux1: and [bp-246],ah

      deux: test dl,2              ; Bit VERT
            jz trois1
            or [bp-248],al
            jmp trois
    trois1: and [bp-248],ah

     trois: test dl,4              ; Bit ROUGE
            jz quatre1
            or [bp-250],al
            jmp quatre
   quatre1: and [bp-250],ah

    quatre: test dl,8              ; Bit INTENSITE
            jz siute1
            or [bp-252],al
            jmp siute
    siute1: and [bp-252],ah
     siute: shr al,1                 ; 2nd bit/pixel du point
                                     ; decalage du masque
            mov ah,al
            not ah

            test dh,1              ; Bit BLEU
            jz deud1
            or [bp-246],al
            jmp deud
     deud1: and [bp-246],ah

      deud: test dh,2              ; Bit VERT
            jz troid1
            or [bp-248],al
            jmp troid
    troid1: and [bp-248],ah

     troid: test dh,4              ; Bit ROUGE
            jz quatrd1
            or [bp-250],al
            jmp quatrd
   quatrd1: and [bp-250],ah

    quatrd: test dh,8              ; Bit INTENSITE
            jz siutd1
            or [bp-252],al
            jmp siutd
    siutd1: and [bp-252],ah

     siutd: shr al,1                 ; decalage du masque
            inc word ptr [bp-112]    ; compteur x:= compteur x + 1
            inc si
            mov dx,[bp-112]
            cmp dx,[bp-258]
            loopnz i_un              ; boucle tant que cx<>0 et compteur x < taix
            jmp svte

i_un:       jmp un

inter_un:   add dx,4
            cmp dx,[bp-258]
            jc tani
clb:        call litbyte
            jmp un
tani:       mov dl,[bp-140]
            or dl,dl
            jnz clb
annul:      xor dx,dx
            mov [bp-246],dx
            mov [bp-248],dx
            mov [bp-250],dx
            mov [bp-252],dx
            jmp un
inter_deb:  jmp debut_lig

svte:                                ; affichage de l'octet precedement calcule .
                                     ; les masques B, V, R et I sont stoque dans
                                     ; les locales en BP-246, 248, 250 et 252
            mov cx,4           ; 4 plans couleur
            mov ax,80h         ; prepare le masque pour l'octet suivant
            push cx            ; sauvegarde des registres utilises pour l'affichage
            push ax
            push es
            mov ax,0B800h      
            mov es,ax          ; segment video

            mov dx,3DDh        ; registre de selectiondes couleurs

            mov al,1
            out dx,al          ; selecte BLEU
            mov cl,[bp-246]
            mov es:[di],cl     ; affiche

            inc al
            out dx,al          ; selecte ROUGE
            mov cl,[bp-248]
            mov es:[di],cl

            shl al,1
            out dx,al          ; selecte VERT
            mov cx,[bp-250]
            mov es:[di],cl

            shl al,1
            out dx,ax          ; selecte INTENSITE
            mov cl,[bp-252]
            mov es:[di],cl

            pop es             ; Affichage termine . Restaure les registres
            pop ax
            pop cx
            inc di                  ; octet video suivant
            mov dx,[bp-112]
            cmp dx,[bp-258]
            jc inter_un
            inc word ptr [bp-110]
            mov dx,[bp-110]
            cmp dx,[bp-260]
            jc inter_deb

            mov sp,bp
            pop bp
            pop ds
            ret 8

affich endp
litbyte proc near
            push es
            push ax
            push dx
            mov ax,0b800h
            mov es,ax
            mov dx,3DEh
            mov al,0
            out dx,al
            mov ah,es:[di]
            mov [bp-246],ah
            inc ax
            out dx,al
            mov ah,es:[di]
            mov [bp-248],ah
            inc ax
            out dx,al
            mov ah,es:[di]
            mov [bp-250],ah
            inc ax
            out dx,al
            mov ah,es:[di]
            mov [bp-252],ah
            pop dx
            pop ax
            pop es
            ret
litbyte endp
prog ends
     end aff

;******************************************************************************
