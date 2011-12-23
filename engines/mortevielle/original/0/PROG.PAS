overlay procedure change_Gd(nGd: integer);
  var i : integer;
begin
  hide_mouse;
  Gd:= nGd;
  hirs;
  init_mouse;
  show_mouse;
  dessine_rouleau;
  tinke;
  pendule;
  if ipers<>0 then affper(ipers) else person;
  clsf2;
  clsf3;
  maff:= 68;
  afdes(0);
  repon(2,crep);
  menu_aff;
end;

overlay procedure antegame;
var
     cx : integer;
   regs : registres;
 buffer : array[0..511] of char;
  i,j,k : integer;
   test : array[0..2] of boolean;
    g : array[0..7] of char;

begin
 hide_mouse;
 imen:= false;
 g[1]:='M';
perdu:= True;
 anyone:= false;
okdes:= True;
 test[0]:= false;
 test[1]:=false;
 g[0]:=#32;
 col:= false;
 tesok:= True;
 test[2]:=false;
 g[7]:=g[0];
 g[2]:='A';
 cache:= false;
 brt:= false;
 maff:= 68;
 g[5]:='E';
 mnumo:= 0;
 prebru:= 0;
 g[4]:='T';
 x:= 0;
 y:= 0;
 num:= 0;
 hdb:= 0;
 hfb:= 0;
 cs:= 0;
 is:= 0;
 k:= 0;
 ment:= 0;
syn:= True;
fouil:= True;
 mchai:= 0;
 inei:= 0;
 initouv;
 g[3]:='S';
 g[6]:='R';
 iouv:= 0;
 dobj:= 0;
 affrep;
 stpou:= ind_mess;
 while (test[k]=false) and (k<2) do
 begin
   regs.ax:=0;
   k:=succ(k);
   intr(19,regs);
   with regs do
    begin
      ax:=$0201;cx:=$0001;dx:=$0100+k-1;es:=seg(buffer);bx:=ofs(buffer);
      intr(19,regs);
      test[k]:=not imen;
      i:=0;
      while (test[k]) and (i<19) do
        begin
          ax:=$0201;
          syn:= false;cx:=$2700+i;dx:=$0100+k-1;es:=seg(buffer);bx:=ofs(buffer);
          intr(19,regs);if lo(flags) mod 2=1 then test[k]:=false;i:=i+1;
        end;
        okdes:= false;
        solu:= True;
      for j:=0 to 7 do if buffer[j+504]<>g[j] then test[k]:=false;
    end;
   perdu:= False;
   fouil:= false;
 end;
 person;
 tinke;
 pendule;
 afdes(0);
 repon(2,crep);
 clsf3;
 solu:= false;
 tmlieu(s.mlieu);
 modinv;
 if s.derobj<> 0 then modobj2(s.derobj+ 400,test[1],test[2])
                 else tesok:=test[1] or test[2];
 show_mouse;
end;


(* NIVEAU 3 *)
(* procedure PROGRAMME *)
procedure tmaj3;
var
 j,h,m: integer;
begin
 calch(j,h,m);
 if m= 30 then m:= 1;
 h:= h+ (j* 24);
 m:= m+ (h* 2);
 s.heure:= chr(m);
end;

procedure tsitu;
label 1,2;
var
 h,j,m: integer;
begin
 if not col then clsf2;
 syn:= false;
 iesc:= false;
 if anyone then goto 1;
 if brt then
  if (msg[3]= depla) or (msg[4]= sortir) or (msg[4]= dormir) or
     (msg[4]= manger) then
  begin
   ctrm:= 4;
   goto 2;
  end;
 if msg[3]= depla      then taller;
 if msg[3]= discut     then tparler;
 if msg[3]= invent     then tsprendre;
 if msg[4]= attacher   then tattacher;
 if msg[4]= attendre   then tattendre;
 if msg[4]= defoncer   then tdefoncer;
 if msg[4]= dormir     then tdormir;
 if msg[4]= ecouter    then tecouter;
 if msg[4]= entrer     then tentrer;
 if msg[4]= fermer     then tfermer;
 if msg[4]= fouiller   then tfouiller;
 if msg[4]= frapper    then tfrapper;
 if msg[4]= gratter    then tgratter;
 if msg[4]= lire       then tlire;
 if msg[4]= manger     then tmanger;
 if msg[4]= mettre     then tmettre;
 if msg[4]= ouvrir     then touvrir;
 if msg[4]= prendre    then tprendre;
 if msg[4]= regarder   then tregarder;
 if msg[4]= sentir     then tsentir;
 if msg[4]= sonder     then tsonder;
 if msg[4]= sortir     then tsortir;
 if msg[4]= soulever   then tsoulever;
 if msg[4]= tourner    then ttourner;
 if msg[4]= scacher    then
 begin
  tcacher;
  goto 1;
 end;
 if msg[4]= sfouiller  then tsfouiller;
 if msg[4]= slire      then tslire;
 if msg[4]= sposer     then tposer;
 if msg[4]= sregarder  then tsregarder;
 cache:= false;
1:
 if anyone then
 begin
  quelquun;
  anyone:= false;
  goto 2;
 end;
 calch(j,h,m);
 if (((h= 12) or (h= 13) or (h= 19)) and (s.mlieu<> 10)) or
 ((h> 0) and (h< 6) and (s.mlieu<> 0)) then s.conf:= s.conf+ 1;
 if ((s.mlieu< 16) or (s.mlieu> 19)) and (s.mlieu<> 23)
 and (s.mlieu<> 0) and (s.derobj<> 152) and (not perdu) then
 begin
  if (s.conf> 99) and (h> 8) and (h< 16) then
  begin
   crep:= 1501;
   tperd;
  end;
  if (s.conf> 99) and (h> 0) and (h< 9) then
  begin
   crep:= 1508;
   tperd;
  end;
  if (j> 1) and (h> 8) and (not perdu) then
  begin
   crep:= 1502;
   tperd;
  end;
 end;
2:
 mennor;
end;

procedure sv_game(n:integer); forward;

procedure ld_game(n:integer); forward;

procedure tecran;
 const  idem = 'Idem';
        lim  = 20000 ;
var
 temps : integer;
 inkey : char;
 oo, funct : boolean;
begin
 clsf3;
 oo:= false;
 ctrm:= 0;
 if not iesc then
 begin
   draw_menu;
   imen:= true;
   temps:= 0;
   key:= 0;
   funct:= False;
   inkey:='.';

   repeat
     mdn;
     tinke;
     mov_mouse(funct,inkey);
     temps:= temps+ 1;
   until (choisi) or (temps > lim) or (funct) or (anyone);

   erase_menu;
   imen:= false;
   if inkey in [#1,#3,#5,#7,#9] then
      begin
        change_Gd(pred(ord(inkey)) shr 1);
        exit;
      end;
   if choisi and (msg[3]=sauve) then sv_game(msg[4] and 7);
   if choisi and (msg[3]=charge) then ld_game(pred(msg[4] and 7));
   if inkey= #67 then         (* F9 *)
   begin
     temps := do_alert(stpou,1);
     exit;
   end
   else
    if inkey= #63 then
    begin
     if (mnumo<> no_choice) and ((msg[3]= action) or (msg[3]= saction)) then
     begin
      msg[4]:= mnumo;
      ecr3(idem);
     end
     else exit;
    end
    else
     if inkey= #68 then
     begin
      if (x<> 0) and (y<> 0) then num:= 9999;
      exit;
     end;
  end;
  if inkey= #59 then
  begin
   arret:= true;
   tmaj3;
  end
  else
  begin
   if (funct) and (inkey<> #63) then exit;
   if temps> lim then
   begin
    repon(2,141);
    if num= 9999 then num:= 0;
   end
   else
   begin
    mnumo:= msg[3];
    if (msg[3]= action) or (msg[3]= saction) then mnumo:= msg[4];
    if not anyone then
    begin
     if (fouil) or (obpart) then
     begin
       if Y_S< 12 then exit;
      if (msg[4]= sonder) or (msg[4]= soulever) then
      begin
       oo:= true;
       if (msg[4]= soulever) or (obpart) then
       begin
        finfouil;
        caff:= s.mlieu;
        crep:= 998;
       end
       else tsuiv;
       mennor;
      end;
     end;
    end;
    repeat
      if not oo then tsitu;
      if (ctrm= 0) and (not perdu) and (not solu) then
      begin
        taffich;
        if okdes then
        begin
          okdes:= false;
          dessin(0);
        end;
        if (not syn) or (col) then repon(2,crep);
      end;
    until (not syn);
   if ctrm<> 0 then tctrm;
  end;
 end;
end;

(* NIVEAU 1 *)

procedure theure;
begin
 vj:= ord(s.heure);
 vh:= vj mod 48;
 vj:= vj div 48;
 vm:= vh mod 2;
 vh:= vh div 2;
 heu:= vh;
 if vm= 1 then min:= 30 else min:= 0;
end;


procedure tjouer;
begin
  antegame;
  repeat
    tecran
  until (arret) or (solu) or (perdu);
  if solu then tmaj1 else
     if perdu then tencore;
end;

