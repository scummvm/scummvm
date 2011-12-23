title affiche_le_dessin_sur_hercule

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
       aff: push ds                  ; sauve DS
            push bp                  ; sauve BP
            mov bp,sp
            push bp                  ; sauve SP
            sub sp,112h              ; 112h octets de locales


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
            mov cx,16                ; 16 bytes de palette
      tr01:
            mov al,[si]
            inc si
            inc si
            mov es:[di],al
            inc di
            loop tr01

            mov si,4138h
            mov ax,[si]              ; taille x
            mov [bp-132],ax
            mov ax,[si+2]            ; taille y
            mov [bp-130],ax

; additionne les coordonnees relatives dans les parametres de passage :

            mov ax,[si+4]
            add [bp+8],ax
            mov ax,[si+6]
            ;shl ax,1
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

            mov ax,6000h
            mov es,ax                ; ES = segment du dessin decompresse
            mov ax,0B000h
            mov ds,ax                ; DS = segment de la memoire video

            xor ax,ax
            mov [bp-110],ax          ; cmpt y
            mov [bp-114],ax          ; offset source

            mov ax,[bp+6]
            mov [bp-120],ax
            mov ax,[bp+8]
            mov [bp-122],ax

debut_lig:
            call calcaddlig
            mov ax,[bp-114]
            mov si,ax
            add ax,320
            mov [bp-114],ax

            xor bx,bx
            mov [bp-112],bx        ; compteur en x
            mov cl,[bp-106]        ; decalage pixels dans byte
            mov ch,[bp-104]        ; masque noir

un1:
            mov al,[di]
            add di,dx
            mov ah,[di]
            sub di,dx

un:
            mov bl,es:[si]         ; 1er bit/pixel du point =
            inc si
            and bx,15              ; no de couleur du point dans la palette
            cmp bl,[bp-142]
            je nonaff
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

    nonaff: or cl,cl
            jz outoct
            dec cl
            dec cl
            ror ch,1
            ror ch,1

     siutd:
            inc word ptr [bp-112]    ; compteur x:= compteur x + 1

            mov bx,[bp-112]
            cmp bx,[bp-132]
            jc un
            inc word ptr [bp-110]
            mov bx,[bp-110]
            cmp bx,[bp-130]
            jc inter_deb

            mov sp,bp
            pop bp
            pop ds
            ret 8

     ; affichage de l'octet precedement calcule .
     outoct:
            mov [di],al
            ;mov al,[bp-120]
            ;and al,3
            ;jz nonaff2l
            add di,dx
            mov [di],ah
            inc di                  ; octet video suivant
            mov ah,[di]
            sub di,dx
            ;jmp aff2l
  nonaff2l: ;inc di
     aff2l: mov al,[di]
            mov ch,3Fh
            mov cl,6
            jmp siutd

inter_deb:  mov di,[bp-100]
            add di,dx
            inc word ptr [bp-120]
            ;mov al,[bp-120]
            ;and al,3
            ;cmp al,1
            ;je s1l
            call calcaddlig
            add di,dx
            inc word ptr [bp-120]
       s1l: mov [bp-100],di
            jmp debut_lig

affich endp
calcaddlig proc near
      mov dx,[bp-120]
      and dx,3
      cmp dx,3
      je sub6000h
      mov dx,2000h
      ret
sub6000h:
      mov dx,6000h
      neg dx
      add dx,80
      ret
calcaddlig endp

PROG ENDS
     END aff



