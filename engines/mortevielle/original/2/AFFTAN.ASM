title affiche_le_dessin_sur_tandy

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
;                [bp- 110] compteur relatif en y
;                [bp- 112] compteur relatif en x
;                [bp- 114] indice dans le dessin source
;                [bp- 120] compteur absolu en y
;                [bp- 122] compteur absolu en x
;                [bp- 142] numero du masque
;                [bp- 250] couleur de l'octet a effacer
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

            mov ax,6000h
            mov es,ax                ; ES = segment du dessin decompresse
            mov ax,0B800h
            mov ds,ax                ; DS = segment de la memoire video

            xor ax,ax
            mov [bp-110],ax          ; cmpt y
            mov [bp-114],ax          ; offset source

            mov ax,[bp+6]
            mov [bp-120],ax
            mov ax,[bp+8]
            mov [bp-122],ax

debut_lig:  mov si,[bp-114]
            mov ax,si
            add ax,320
            mov [bp-114],ax

            xor bx,bx
            mov [bp-112],bx        ; compteur en x
            mov bx,[bp-100]
            mov di,bx
            inc word ptr [bp-120]
            mov ax,[bp-120]
            and ax,3
            je sub6000h
            add bx,2000h
            jmp majbp100
  sub6000h: sub bx,6000h
            add bx,160
  majbp100: mov [bp-100],bx

un1:
            xor dx,dx
            mov [bp-250],dx
            xor bx,bx
            jmp un

     aff01: pop di
            mov al,[di]
            jmp siutd

        un: mov cl,4
            push di
            mov di,es:[si]         ; 1er bit/pixel du point =
            and di,15              ; no de couleur du point dans la palette
            cmp di,[bp-142]
            jz aff01
            shl di,1
            mov dx,[bp+di-64]      ; DL => 1ere couleur
            pop di
            xor al,al
            mov al,dl
            shl dh,cl
            add al,dh
            mov [bp-250],al

     siutd:
            inc word ptr [bp-112]    ;
            inc word ptr [bp-112]    ; compteur x:= compteur x + 2
            inc si
            inc si       ; on perd un point !!!

     ; affichage de l'octet precedement calcule .

            mov [di],al
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

inter_deb:  jmp debut_lig
 inter_un:  jmp un1

affich endp

PROG ENDS
     END aff



----