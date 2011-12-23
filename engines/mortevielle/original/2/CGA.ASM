title affiche_le_dessin_CGA

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


; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse absolue  7000h:xxxx
; --------------------------------------------------------------------------

            push ds                  ; sauve DS
            push es                  ; sauve ES
            mov ax,7000h
            mov ds,ax                ; DS = segment de la palette

; calcul des addresses des motifs :
; - a partir de 6CE40 h et par pas de 404
; - chaque addresse est le deplacement dans le segment 6000h
; - bp-64 : nø15,   bp-62 : nø14,   ... ,   bp-34 : nø0

            mov cx,16
            mov bx,404;
      cal0: mov ax,cx
            dec ax
            mov di,ax
            mul bx
            add ax,0D700h
            shl di,1
            add di,34
            neg di
            mov [bp+di],ax
            loop cal0

            mov si,4138h
            mov ax,[si]
            mov [bp-258],ax          ; taix du dessin
            mov ax,[si+2]
            mov [bp-260],ax          ; taiy du dessin

; additionne les coordonnees relatives dans les parametres de passage :

            mov ax,[si+4]            ; coorx
            add [bp+8],ax
            mov ax,[si+6]            ; coory
            add [bp+6],ax

; --------------------------------------------------------------------------
;      Calculs sur les variables d'adresse relative  yyyy:xxxx
; --------------------------------------------------------------------------

            mov ax,[bp+12]
            mov ds,ax                ; DS = segment du fichier dessin
            mov ax,6000h
            mov es,ax                ; ES = segment du dessin decompresse

            mov si,[bp+10]           ; offset dessin
            mov al,[si]
            mov [bp-275],al
            mov [bp-140],al

            mov bx,16
            mov [bp-142],bx          ; par d‚faut
            or al,al
            jz aff00
            mov bl,[si+2]            ; nø de couleur du masque
            mov [bp-142],bx
     aff00:
    datpal:
            mov ax,[bp+6]      ; Y
            shr ax,1           ; Y/2 lignes paires a afficher
            mov bx,80
            mul bx             ; 80 octets par lignes
            mov bx,[bp+8]
            mov cl,2
            shr bx,cl          ; BX := X div 4 , car 4 pixels par octet
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
prem_lig_paire:
            mov ax,[bp+8]
            and ax,3         ; reste de la div de X par 4
                                     ; 4 - ( X mod 4 )  =
            neg ax                   ; nbre de points pour le 1er octet .
            add ax,4                 ; 4 points/couleurs par octets
            mov [bp-108],ax

            xor si,si                ; init
            mov [bp-110],si          ; cmpt y
            mov [bp-114],si          ; Octet de d‚but de la ligne
debut_lig:
            mov ax,[bp-114]
            mov si,ax
            add ax,320
            mov [bp-114],ax          ; d‚but de la ligne suivante

            mov cx,[bp-108]          ; Nombre de point(s) a afficher
                                     ; vaut 4 lorsque tout l'octet va etre afficher
                                     ;      3 lorsque les 6 bits de droite vont etre re-afficher
                                     ;      2 lorsque les 4 bits de droite vont etre re-afficher
                                     ;      1 lorsque les 2 bits de droite vont etre re-afficher
            xor ax,ax
            mov [bp-112],ax        ; compteur en x
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

masque:
            jmp suit3

lig_paire:
            inc word ptr [bp-102]
            add dx,2000h
sui2:
            mov [bp-100],dx        ; DX = octet du debut de la ligne suivante

un:                                ; 1er bit/pixel du point
            mov dl,es:[si]         ; no de couleur du point dans la palette
            cmp dl,[bp-142]
            je masque

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
      xnok: cmp ax,dx
            jc xok
            sub ax,dx
            jmp xnok
       xok: mov bx,ax
            add si,2
            mov dx,es:[si]            ; tay du bloc
            mov ax,[bp-110]        ; cmpt y
      ynok: cmp ax,dx
            jc yok
            sub ax,dx
            jmp ynok
       yok: mov dx,es:[si-2]          ; y * tax
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

     suit3:
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
            jmp un
inter_deb:  jmp debut_lig

svte:                              ; affichage de l'octet precedement calcule .
            push ds
            mov ax,0B800h
            mov ds,ax              ; segment video

            mov al,[bp-246]
            mov [di],al            ; affiche

            pop ds                 ; Affichage termine .
            inc di                 ; octet video suivant
            mov cx,4
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
            mov ah,es:[di]
            mov [bp-246],ah
            pop dx
            pop ax
            pop es
            ret
litbyte endp
PROG ENDS
     END aff


