const ams=0; cga=1; ega=2;
      NbMenu=6;

const newlet : array[0..29] of byte
             = ( $00, $00, $00,
                 $1F, $DC, $00,
                 $0E, $00, $00,
                 $0E, $78, $F8,
                 $1F, $39, $DC,
                 $1C, $73, $80,
                 $38, $73, $B8,
                 $7B, $FB, $E0,
                 $00, $00, $00,
                 $00, $00, $00 );

     lettr_ang : array[1..6,0..23] of byte
             = ((   (* Inv *)
                  $1C, $00, $00,
                  $08, $00, $00,
                  $12, $C7, $70,
                  $11, $22, $20,
                  $22, $42, $40,
                  $22, $42, $80,
                  $EE, $C3, $00,
                  $00, $00, $00 ),

                (   (* Mov *)
                  $30, $C0, $00,
                  $11, $80, $00,
                  $3B, $0E, $77,
                  $2D, $12, $22,
                  $42, $22, $24,
                  $42, $24, $28,
                  $C6, $38, $30,
                  $00, $00, $00 ),

                (   (* Act *)
                  $02, $00, $20,
                  $06, $00, $20,
                  $0A, $1E, $F0,
                  $12, $22, $40,
                  $3E, $40, $80,
                  $42, $44, $80,
                  $EE, $38, $C0,
                  $00, $00, $00 ),

                (   (* Self *)
                  $0F, $80, $E7,
                  $10, $80, $49,
                  $20, $38, $88,
                  $1E, $44, $9C,
                  $02, $F9, $10,
                  $42, $81, $20,
                  $FC, $E7, $70,
                  $00, $00, $00 ),

                (   (* Dis *)
                  $3E, $10, $00,
                  $11, $00, $00,
                  $21, $63, $C0,
                  $21, $24, $00,
                  $42, $47, $00,
                  $44, $40, $80,
                  $F9, $DF, $00,
                  $00, $00, $00 ),

                (  (* File *)
                  $1F, $C9, $80,
                  $08, $40, $80,
                  $12, $31, $1C,
                  $1E, $11, $22,
                  $24, $22, $3C,
                  $20, $22, $40,
                  $E0, $EE, $70,
                  $00, $00, $00 ));

     lettr_all : array[1..6,0..23] of byte
             = ((   (* Inv *)
                  $1C, $00, $00,
                  $08, $00, $00,
                  $12, $C7, $70,
                  $11, $22, $20,
                  $22, $42, $40,
                  $22, $42, $80,
                  $EE, $C3, $00,
                  $00, $00, $00 ),

                (   (* Dep *)
                  $3E, $00, $00,
                  $11, $00, $00,
                  $21, $39, $70,
                  $21, $44, $88,
                  $42, $79, $10,
                  $44, $81, $E0,
                  $F8, $E2, $00,
                  $00, $07, $00 ),

                (   (* Akt *)
                  $02, $18, $10,
                  $0E, $08, $10,
                  $0A, $13, $78,
                  $12, $14, $20,
                  $3E, $38, $40,
                  $42, $24, $40,
                  $EE, $EC, $60,
                  $00, $00, $00 ),

                (  (* Self *)
                  $0F, $80, $E7,
                  $10, $80, $49,
                  $20, $38, $88,
                  $1E, $44, $9C,
                  $02, $F9, $10,
                  $42, $81, $20,
                  $FC, $E7, $70,
                  $00, $00, $00 ),

                (   (* Dis *)
                  $3E, $10, $00,
                  $11, $00, $00,
                  $21, $63, $C0,
                  $21, $24, $00,
                  $42, $47, $00,
                  $44, $40, $80,
                  $F9, $DF, $00,
                  $00, $00, $00 ),

                (   (* Fic *)
                  $1F, $C8, $00,
                  $08, $40, $00,
                  $12, $31, $E0,
                  $1E, $12, $20,
                  $24, $24, $00,
                  $20, $24, $40,
                  $E0, $E3, $80,
                  $00, $00, $00 ));

     lettr_fra : array[1..6,0..23] of byte
             = ((   (* Inv *)
                  $1C, $00, $00,
                  $08, $00, $00,
                  $12, $C7, $70,
                  $11, $22, $20,
                  $22, $42, $40,
                  $22, $42, $80,
                  $EE, $C3, $00,
                  $00, $00, $00 ),

                (   (* D‚p *)
                  $3E, $0C, $00,
                  $11, $10, $00,
                  $21, $39, $70,
                  $21, $44, $88,
                  $42, $79, $10,
                  $44, $81, $E0,
                  $F8, $E2, $00,
                  $00, $07, $00 ),

                (   (* Act *)
                  $02, $00, $20,
                  $06, $00, $20,
                  $0A, $1E, $F0,
                  $12, $22, $40,
                  $3E, $40, $80,
                  $42, $44, $80,
                  $EE, $38, $C0,
                  $00, $00, $00 ),

                (  (* Self *)
                  $0F, $80, $E7,
                  $10, $80, $49,
                  $20, $38, $88,
                  $1E, $44, $9C,
                  $02, $F9, $10,
                  $42, $81, $20,
                  $FC, $E7, $70,
                  $00, $00, $00 ),

                (   (* Dis *)
                  $3E, $10, $00,
                  $11, $00, $00,
                  $21, $63, $C0,
                  $21, $24, $00,
                  $42, $47, $00,
                  $44, $40, $80,
                  $F9, $DF, $00,
                  $00, $00, $00 ),

                (   (* Fic *)
                  $1F, $C8, $00,
                  $08, $40, $00,
                  $12, $31, $E0,
                  $1E, $12, $20,
                  $24, $24, $00,
                  $20, $24, $40,
                  $E0, $E3, $80,
                  $00, $00, $00 ));

type registres = record
                   ax, bx, cx, dx, bp, sp, di, si, ds, es, Flags : integer;
                 end;

var Gd, res,coul : integer;
    lettres : array[1..6,0..23] of byte;

procedure affput(Chx,Gd,x,y,coul,char:integer); external 'c:\usr\mort\asm\divaf.com';

procedure affcar(Gd,x,y,coul,char:integer);
begin
  if res=1 then affput(1,Gd,(x shr 1),y,coul,char)
           else affput(1,Gd,x,y,coul,char);
end;

procedure putpix(Gd,x,y,coul:integer);
begin
  affput(0,Gd,x,y,coul,0);
end;

procedure hirs;
   var i,  j : integer;
begin
  case Gd of
    cga :
     begin
       graphcolormode;
       graphbackground(0);
       Palette(1);
     end;
    ams :
     begin
       hires;
       inline($B8/6/0/        (*  =>  mov ax, 6 *)
              $CD/$10);       (*  =>  int 16   *)
       port[$3D9]:=15;
       port[$3DF]:=0;
       port[$3DD]:=15;
     end;
    ega :
     inline($B8 / 14 / 0 /     (*  MOV AX, 14   ; mode video 14 = 640*200 16 couleurs *)
            $CD / $10);        (*  INT 16  *)
  end;
end;


(* NIVEAU 14*)

procedure init_menu;
    var
        f : file;
  begin
    assign(f,'c:\usr\mort\menuan.mor');
    reset(f,144);
    blockread(f,lettres,1);
    close(f);
  end;

procedure save_menu;
    var
        f : file;
  begin
    assign(f,'c:\usr\mort\menual.mor');
    rewrite(f,144);
    blockwrite(f,lettr_all,1);
    close(f);
    assign(f,'c:\usr\mort\menuan.mor');
    rewrite(f,144);
    blockwrite(f,lettr_ang,1);
    close(f);

  end;


(*---------------------------------------------------------------------------*)
(*--------------------   PROCEDURES  EN  ASSEMBLEUR   -----------------------*)
(*---------------------------------------------------------------------------*)

procedure box(c,Gd,xo,yo,xi,yi,patt:integer);
     external 'c:\usr\mort\asm\boite.com';


procedure menu_aff;
     var
         ind_tabl,k,col:integer;
         c: char;
         pt,x,y,color,msk,
         num_letr: integer;
   begin
     (* debug('menu_aff'); *)
     box( 7, Gd, 0,0, 639,10, 255);
     col:= 28*res;
     if Gd = cga then color:=1
                 else color:=11;
     num_letr:=0;
     repeat     (* lettre par lettre *)
       num_letr:= num_letr +1;
       ind_tabl:=0;
       y:= 1;
       repeat    (* colonne par colonne *)
         k:=0;
         x:= col;
         repeat   (* ligne par ligne *)
           msk:=$80;
           for pt:=0 to 7 do
            begin
              if (lettres[num_letr,ind_tabl] and msk)<>0 then
                 begin
                   putpix(Gd,x+1,y+1,0);
                   putpix(Gd,x,y+1,0);
                   putpix(Gd,x,y,color);
                 end;
              msk:= msk shr 1;
              x:=x +1;
            end;
           ind_tabl:=succ(ind_tabl);
           k:=succ(k);
         until k=3;
         y:=y+1;
       until (y=9);
       col:=col+48*res;
     until num_letr=6;
   end;


begin
  gd:=ega;
  res:=2;
  init_menu;
  repeat
    hirs;
    menu_aff;
    gotoxy(1,20);
    write('Enter new Gd : ');
    readln(Gd);
    if gd=cga then res:=1 else res:=2;
  until gd>2;
end.
