(* NIVEAU 4 *)
const stouinon = '[2][ ][OUI][NON]';

overlay procedure taller;
label 2;
var
 mx,cx,cy: integer;
begin
 if (s.mlieu= 26) and (msg[4]= depl[6]) then
 begin
  s.mlieu:= 15;
  caff:= s.mlieu;
  afdes(0);
  repon(2,s.mlieu);
 end;
 if (s.mlieu= 15) and (msg[4]= depl[6]) then
 begin
  if not syn then ecr3('aller');
  tfleche;
  if iesc then okdes:= false;
  if (anyone) or (iesc) then exit;
  tcoord(1);
  if num= 0 then exit;
  if num= 1 then
  begin
   s.mlieu:= 0;
   tmlieu(0);
  end
  else
   if num= 7 then
   begin
    s.mlieu:= 13;
    tmlieu(13);
   end
   else
    if num<> 6 then s.mlieu:= 26;
  if (num> 1) and (num< 6) then ment:= num- 1;
  if num> 7 then ment:= num- 3;
  if num<> 6 then affrep else aldepl;
  exit;
 end;
 tsort;
 cx:= 0;
 repeat
  cx:= cx+ 1;
 until depl[cx]= msg[4];
 if s.mlieu= 19 then
 begin
  if cx= 1 then t1deva;
  if cx= 2 then t1neig;
  goto 2;
 end;
 if s.mlieu= 23 then
 begin
  if cx= 1 then t1deau;
  if cx= 2 then t1derr;
  goto 2;
 end;
 if (s.mlieu= 11) and (cx= 1) then cx:= 6;
 if s.mlieu= 12 then
 begin
  if cx= 2 then cx:= 6;
  if cx= 5 then cx:= 16;
 end;
 if (s.mlieu= 14) and (cx= 3) then cx:= 6;
 if ((s.mlieu= 15) or (s.mlieu= 26)) and (cx= 4) then cx:= 6;
 if (s.mlieu> 19) and (s.mlieu<> 26) then cx:= cx+ 10;
 if (s.mlieu= 20) and (cx= 13) then cx:= 16;
 if s.mlieu= 21 then
 begin
  if cx= 12 then cx:= 16 else
  if cx> 13 then cx:= 15;
 end;
 if (s.mlieu= 22) and (cx> 14) then cx:= 15;
 if (s.mlieu= 24) and (cx<> 17) then
  if cx> 13 then cx:= 15;
 if cx= 1 then s.mlieu:= 11 else
 if cx= 2 then s.mlieu:= 12 else
 if cx= 3 then s.mlieu:= 14 else
 if cx= 4 then s.mlieu:= 15 else
 if cx= 5 then cx:= 12;
 if cx= 6 then cx:= 11;
 if cx= 11 then t1sama else
 if cx= 12 then t1deva else
 if cx= 13 then s.mlieu:= 20 else
 if cx= 14 then s.mlieu:= 24 else
 if cx= 15 then t1neig else
 if cx= 16 then t1derr else
 if cx= 17 then
 begin
  if (s.ipuit<> 120) and (s.ipuit<> 140) then crep:= 997 else
   if s.ipuit= 120 then crep:= 181 else
    if s.conf> 80 then
    begin
     crep:= 1505;
     tperd;
    end
    else
    begin
     s.mlieu:= 23;
     affrep;
    end;
 end;
 if (cx< 5) or (cx= 13) or (cx= 14) then affrep;
 debloc(s.mlieu);
2:
 tmlieu(s.mlieu);
end;

overlay procedure tprendre;
var
 cx, cy, cz: integer;
begin
 if caff> 99 then
 begin
  cx:= caff;
  avpoing(cx);
  if crep<> 139 then
  begin
   if ipers> 0 then s.conf:= s.conf+ 3;
   if obpart then
   begin
    if s.mlieu= 2 then s.iloic:= 0;
    if s.mlieu= 13 then
    begin
     if s.iboul= caff then s.iboul:= 0;
     if s.ibag= caff then s.ibag:= 0;
    end;
    if s.mlieu= 14 then s.icave:= 0;
    if s.mlieu= 16 then s.icryp:= 0;
    if s.mlieu= 17 then s.ivier:= 0;
    if s.mlieu= 24 then s.ipuit:= 0;
    mfouen;
    obpart:= false;
    affrep;
   end
   else
   begin
    tabdon[acha+ (pred(mchai)* 10)+ pred(cs)]:=0;
    tsuiv;
    dobj:= dobj+ 1;
    if dobj> 6 then
    begin
     s.conf:= s.conf+ 2;
     dobj:= 0;
    end;
   end;
  end;
  exit;
 end;
 if not syn then ecr3('prendre');
 tfleche;
 if (anyone) or (iesc) then exit;
 if caff= 3 then
 begin
  tcoord(2);
  if num= 1 then
  begin
   crep:= 152;
   exit;
  end;
 end;
 tcoord(5);
 if (num= 0) or ((num= 1) and (s.mlieu= 16)) then
 begin
  tcoord(8);
  if num<> 0 then
  begin
   if ipers> 0 then s.conf:= s.conf+ 3;
   crep:= 997;
   if (s.mlieu= 2) and (s.iloic<> 0) then avpoing(s.iloic);
   if (s.mlieu=13) and (num=1) and (s.iboul<>0) then
   begin
    avpoing(s.iboul);
    if (crep<> 997) and (crep<> 139) then aniof(2,7);
   end;
   if (s.mlieu=13) and (num=2) and (s.ibag<>0) then
   begin
    avpoing(s.ibag);
    if (crep<> 997) and (crep<> 139) then aniof(2,6);
   end;
   if (s.mlieu= 14) and (s.icave<> 0) then
   begin
    avpoing(s.icave);
    if (crep<> 997) and (crep<> 139) then aniof(2,2);
   end;
   if (s.mlieu= 16) and (s.icryp<> 0) then avpoing(s.icryp);
   if (s.mlieu= 17) and (s.ivier<> 0) then
   begin
    avpoing(s.ivier);
    if (crep<> 997) and (crep<> 139) then
    begin
     crep:= 182;
     aniof(2,1);
    end;
   end;
   if (s.mlieu= 24) and (s.ipuit<> 0) then
   begin
    avpoing(s.ipuit);
    if (crep<> 997) and (crep<> 139) then aniof(2,1);
   end;
   if (crep<> 997) and (crep<> 182) and (crep<> 139) then crep:= 999;
  end;
 end
 else
 begin
  if ((s.mlieu= 0) and (num= 3)) or ((s.mlieu= 1) and (num= 4))
  or ((s.mlieu= 2) and (num= 1)) or ((s.mlieu= 4) and (num= 3))
  or ((s.mlieu= 5) and (num= 6)) or ((s.mlieu= 6) and (num= 2))
  or ((s.mlieu= 7) and (num= 6)) or ((s.mlieu= 8) and (num= 4))
  or ((s.mlieu= 9) and (num= 4)) or ((s.mlieu= 10) and (num> 2))
  or ((s.mlieu= 11) and (num= 7)) or ((s.mlieu= 12) and (num= 6))
  or ((s.mlieu= 13) and (num> 4)) or ((s.mlieu> 13)
  and (s.mlieu<> 23)) then crep:= 997 else
  begin
   if s.mlieu= 23 then
   begin
    crep:= 1504;
    tperd;
   end
   else crep:= 120;
  end;
 end;
end;

overlay procedure tsprendre;
var
 cx,cy,cz: integer;
begin
 cx:= 0;
 repeat
  cx:= cx+ 1;
 until invt[cx]= msg[4];
 cz:= 0;
 cy:= 0;
 repeat
  cy:= cy+ 1;
  if ord(s.sjer[cy])<> 0 then cz:= cz+ 1;
 until cz= cx;
 cz:= ord(s.sjer[cy]);
 s.sjer[cy]:= chr(0);
 modinv;
 avpoing(cz);
 crep:= 998;
 clsf2;
end;

overlay procedure tsoulever;
label 1;
var
 cx: integer;
begin
 if not syn then ecr3('soulever');
 tfleche;
 if (anyone) or (iesc) then exit;
 tcoord(3);
 if num= 0 then
 begin
  tcoord(8);
  if num<> 0 then
  begin
   if ipers> 0 then s.conf:= s.conf+ 1;
   crep:= 997;
   if (s.mlieu= 2) and (s.iloic<> 0) then treg(s.iloic);
  end;
  exit;
 end;
 if ipers> 0 then s.conf:= s.conf+ 1;
 cx:= s.mlieu;
 if s.mlieu= 16 then cx:= 14;
 if s.mlieu= 19 then cx:= 15;
 crep:= tabdon[asoul+(cx shl 3)+pred(num)];
 if crep= 255 then crep:= 997;
end;

overlay procedure tlire;
var
 iaff: integer;
begin
 if caff> 99 then st4(caff) else
 begin
  if not syn then ecr3('lire');
  tfleche;
  if not (anyone) and not (iesc) then
  begin
   tcoord(4);
   if num<> 0 then crep:= 107;
  end;
 end;
end;

overlay procedure tslire;
begin
 if s.derobj= 0 then crep:= 186 else st4(s.derobj);
end;

overlay procedure tregarder;
var
 cx: integer;
begin
 if caff> 99 then
 begin
  crep:= 103;
  exit;
 end;
 if not syn then ecr3('regarder');
 tfleche;
 if (anyone) or (iesc) then exit;
 tcoord(5);
 if num= 0 then
 begin
  tcoord(8);
  crep:= 131;
  if num<> 0 then
  begin
   if s.mlieu= 13 then
   begin
    if num= 1 then
    begin
     crep:= 164;
     if s.ibag<> 0 then treg(s.ibag) else
      if s.iboul<> 0 then treg(s.iboul);
    end
    else
    begin
     crep:= 193;
     if s.ibag<> 0 then treg(s.ibag);
    end;
   end;
   if s.mlieu= 14 then
   begin
    crep:= 164;
    if s.icave<> 0 then treg(s.icave);
   end;
   if s.mlieu= 17 then
   begin
    crep:= 174;
    if s.ivier<> 0 then treg(s.ivier);
   end;
   if s.mlieu= 24 then
   begin
    crep:= 131;
    if s.ipuit<> 0 then treg(s.ipuit);
   end;
  end;
  exit;
 end;
 cx:= s.mlieu;
 if s.mlieu= 20 then cx:= 17;
 if (s.mlieu> 21) and (s.mlieu< 25) then cx:= cx- 4;
 if s.mlieu= 26 then cx:= 21;
 crep:= tabdon[arega+(cx*7)+pred(num)];
 if (s.mlieu= 13) and (num= 8) then crep:= 126;
 if s.mlieu= 19 then crep:= 103;
 if crep= 255 then crep:= 131;
 if (s.mlieu= 1) and (num= 1) then treg(144);
 if (s.mlieu= 5) and (num= 3) then treg(147);
 if (s.mlieu= 8) and (num= 3) then treg(149);
 if (s.mlieu= 9) and (num= 2) then treg(30);
 if (s.mlieu= 10) and (num= 3) then treg(31);
end;

overlay procedure tsregarder;
begin
 if s.derobj<> 0 then treg(s.derobj) else crep:= 186;
end;

overlay procedure tfouiller;
const r:array[0..13] of byte=(123,104,123,131,131,123,104,131,123,123,106,123,123,107);
var
 cx: integer;
begin
 if caff> 99 then
 begin
  st7(caff);
  exit;
 end;
 if not syn then ecr3('fouiller');
 tfleche;
 if (anyone or iesc) then exit;
 if s.mlieu=23 then
 begin
  crep:= 1504;
  tperd;
  exit;
 end;
 tcoord(6);
 if num= 0 then
 begin
  tcoord(7);
  if num<>0 then
  begin
   cx:=0;
   repeat
    cx:=cx+1;
   until (cx>6) or (num= ord(touv[cx]));
   if num<>ord(touv[cx]) then crep:=187 else
   begin
    if ipers>0 then s.conf:= s.conf+3;
    rechai(mchai);
    if mchai<>0 then
    begin
     cs:=0;
     is:=0;
     fouil:=true;
     mfoudi;
     tsuiv;
    end
    else crep:=997;
   end;
  end
  else
  begin
   tcoord(8);
   crep:= 997;
   if num<>0 then
   begin
    if ipers>0 then s.conf:=s.conf+3;
    if (s.mlieu<>24) and (s.mlieu<>17) and (s.mlieu<>13) then
    begin
     if s.mlieu=2 then
     begin
      crep:= 123;
      if s.iloic<>0 then treg(s.iloic);
     end;
     if s.mlieu=16 then
     begin
      crep:= 123;
      if s.icryp<>0 then treg(s.icryp);
     end;
    end;
   end;
  end;
 end
 else
 begin
  if ipers>0 then s.conf:=s.conf+ 3;
  crep:= 997;
  if s.mlieu < 14 then crep:= r[s.mlieu];
  if (s.mlieu= 3) and (num= 2) then crep:=162;
  if (s.mlieu= 12) then
  begin
   if (num= 3) or (num= 4) then crep:=162;
   if num= 5 then crep:= 159;
  end;
  if s.mlieu=19 then crep:=104;
  if s.mlieu=16 then crep:=155;
 end;
end;

overlay procedure tsfouiller;
begin
 if s.derobj<>0 then st7(s.derobj) else crep:= 186;
end;

overlay procedure touvrir;
var
 cx,haz:integer;
begin
 if not syn then ecr3('ouvrir');
 if caff= 26 then
 begin
  if ment<> 0 then
  begin
   msg[4]:= entrer;
   syn:= true;
  end
  else crep:= 997;
  exit;
 end;
 if caff= 15 then
 begin
  aldepl;
  exit;
 end;
 tfleche;
 if (anyone) or (iesc) then exit;
 tcoord(7);
 if num<> 0 then
 begin
  if ipers> 0 then s.conf:= s.conf+ 2;
  iouv:= iouv+ 1;
  cx:= 0;
  repeat
   cx:= cx+ 1;
  until (cx> 6) or (ord(touv[cx])= 0) or (ord(touv[cx])= num);
  if ord(touv[cx])<> num then
  begin
   if not
    (
     ( (num= 3) and ((s.mlieu= 0) or (s.mlieu= 9) or (s.mlieu= 5) or (s.mlieu= 7)) )
     or
      ((num= 4) and ((s.mlieu= 1) or (s.mlieu= 2) or (s.mlieu= 6))) or
      ((s.mlieu= 4) and (num= 5)) or
      ((num= 6) and ((s.mlieu= 7) or (s.mlieu= 10) or
                     (s.mlieu= 8) or (s.mlieu= 13))) or
      ((s.mlieu= 8) and (num= 2)) or
      ((s.mlieu= 12) and (num= 7))) then
      begin
         if ((s.mlieu> 10) and (s.mlieu< 14)) or
          ((s.mlieu> 6) and (s.mlieu< 10)) or
          (s.mlieu= 0) or (s.mlieu= 2) or (s.mlieu= 5) then
          begin
            haz:= hazard(1, 4);
            if haz= 3 then parole(7, 9, 1);
          end;
        touv[cx]:= chr(num);
        aniof(1,num);
      end;
   cx:= s.mlieu;
   if s.mlieu= 16 then cx:= 14;
   crep:= tabdon[aouvr+ (cx* 7)+ pred(num)];
   if crep= 254 then crep:= 999;
  end
  else crep:= 18;
 end;
end;

overlay procedure tmettre;
var
 quel: integer;
 entre : boolean;
 st : phrase;
 str : str255;
 i,tay : integer;
begin
 if s.derobj= 0 then
 begin
  crep:= 186;
  exit;
 end;
 if not syn then ecr3('mettre');
 tfleche;
 if iesc then crep:= 998;
 if (anyone) or (iesc) then exit;
 tcoord(8);
 if num<> 0 then
 begin
  crep:= 999;
  if caff= 13 then
  begin
   if num= 1 then
   begin
    if s.iboul<> 0 then crep:= 188 else
    begin
     s.iboul:= s.derobj;
     if s.derobj= 141 then aniof(1,7);
    end;
   end
   else
    if s.ibag<> 0 then crep:= 188 else
    begin
     s.ibag:= s.derobj;
     if s.derobj= 159 then aniof(1,6);
    end;
  end;
  if caff= 14 then
   if s.icave<>0 then crep:= 188 else
   begin
    s.icave:= s.derobj;
    if s.derobj= 151 then
    begin
     aniof(1,2);
     aniof(1,1);
     repon(2,165);
     maivid;
     parole(6, -9, 1);
     quel:= do_alert(stouinon,1);
     if quel=1 then begin
                      deline(582,st,tay);
                      i:= do_alert(delig,1);
                      tesok:=False;
                      entre:= ques;
                      hide_mouse;
                      hirs;
                      dessine_rouleau;
                      clsf2;
                      clsf3;
                      show_mouse;
                      tinke;
                      pendule;
                      if ipers<>0 then affper(ipers)
                                  else person;
                      menu_aff;
                      if entre then
                         begin
                           s.mlieu:= 17;
                           tmlieu(17);
                         end
                        else
                         begin
                           tmlieu(s.mlieu);
                           writepal(14);
                           dessin(0);
                           aniof(1,2);
                           aniof(1,1);
                           DeLine(577,st,tay);
                           i:= do_alert(delig,1);
                           aniof(2,1);
                           crep:= 166;
                         end;
                      affrep;
                    end
               else begin
                      aniof(2,1);
                      crep:= 166;
                      tesok:=True;
                    end;
     exit;
    end;
   end;
  if caff= 16 then
   if s.icryp= 0 then s.icryp:= s.derobj else crep:= 188;
  if caff= 17 then
   if s.ivier<>0 then crep:= 188 else
    if s.derobj= 143 then
    begin
     s.ivier:= 143;
     aniof(1,1);
    end
    else
    begin
     crep:= 1512;
     tperd;
    end;
  if caff= 24 then
   if s.ipuit<> 0 then crep:= 188 else
    if (s.derobj= 140) or (s.derobj= 120) then
    begin
     s.ipuit:= s.derobj;
     aniof(1,1);
    end
    else crep:= 185;
  if crep<> 188 then maivid;
 end;
end;

overlay procedure ttourner;
var
 quel: integer;
begin
 if caff> 99 then
 begin
  crep:= 149;
  exit;
 end;
 if not syn then ecr3('tourner');
 tfleche;
 if (anyone) or (iesc) then exit;
 tcoord(9);
 if num<> 0 then
 begin
  crep:= 997;
  if (s.mlieu= 13) and (s.ibag= 159) and (s.iboul= 141) then
  begin
   repon(2,167);
   parole(7, 9, 1);
   quel:= do_alert(stouinon,1);
   if quel= 1 then solu:= true else crep:= 168;
  end;
  if (s.mlieu= 17) and (s.ivier= 143) then
  begin
   repon(2, 175);
   clsf3;
   parole(6, -9, 1);
   quel:= do_alert(stouinon,1);
   if quel= 1 then
   begin
    s.mlieu:= 16;
    affrep;
   end
   else crep:= 176;
  end;
 end;
end;

overlay procedure tcacher;
begin
 if not syn then ecr3('se cacher');
 tfleche;
 if not (anyone) and not (iesc) then
 begin
  tcoord(10);
  if num= 0 then cache:= false else
  begin
   cache:= true;
   crep:= 999;
  end;
 end;
end;

overlay procedure tattacher;
begin
 if s.derobj= 0 then crep:= 186 else
 begin
  if not syn then ecr3('attacher');
  tfleche;
  if not (anyone) and not (iesc) then
  begin
   tcoord(8);
   crep:= 997;
   if (num<> 0) and (s.mlieu= 24) then
   begin
    crep:= 999;
    if (s.derobj= 120) or (s.derobj= 140) then
    begin
     s.ipuit:= s.derobj;
     aniof(1,1);
    end
    else crep:= 185;
    maivid;
   end;
  end;
 end;
end;

overlay procedure tfermer;
var
 cx,chai: integer;
begin
 if not syn then ecr3('fermer');
 if caff< 26 then
 begin
  tfleche;
  if iesc then crep:= 998;
  if (anyone) or (iesc) then exit;
  tcoord(7);
  if num<> 0 then
  begin
   cx:= 0;
   repeat
    cx:= cx+ 1;
   until (cx> 6) or (num= ord(touv[cx]));
   if num= ord(touv[cx]) then
   begin
    aniof(2,num);
    crep:= 998;
    touv[cx]:= chr(0);
    iouv:= iouv- 1;
    if iouv< 0 then iouv:= 0;
    chai:= 9999;
    rechai(chai);
    if mchai= chai then mchai:= 0;
   end
   else crep:= 187;
  end;
 end;
 if caff= 26 then crep:= 999;
end;

overlay procedure tfrapper;
var
 l,p,haz: integer;
begin
 if not syn then ecr3('frapper');
 if s.mlieu= 15 then
 begin
  l:= do_alert('[1][ | Avant, utilisez le menu DEP...][ok]',1);
  exit;
 end;
 if s.mlieu< 25 then
 begin
  tfleche;
  if not (anyone) and not (iesc) then
   if (s.mlieu< 19) and (s.mlieu<> 15) then crep:= 133 else crep:= 997;
  exit;
 end;
 if s.mlieu= 26 then
 begin
  haz:= (hazard(0, 8))- 4;
  parole(11, haz, 1);
  ecfren(p,haz,s.conf,ment);
  l:= ment;
  if l<> 0 then
   if p<> -500 then
   begin
    if haz> p then crep:= 190 else
    begin
     becfren(l);
     frap;
    end;
   end
   else frap;
  if ment= 8 then crep:= 190;
 end;
end;

overlay procedure tposer;
var
 cx,chai: integer;
begin
 if not syn then ecr3('poser');
 if s.derobj= 0 then crep:= 186 else
 begin
  if caff> 99 then
  begin
    crep:= 999;
    ajchai;
    if crep<> 192 then maivid;
    exit;
  end;
  tfleche;
  if (anyone) or (iesc) then exit;
  tcoord(7);
  crep:= 124;
  if num<> 0 then
  begin
   rechai(chai);
   if chai= 0 then crep:= 997 else
   begin
    cx:= 0;
    repeat
     cx:= cx+ 1;
    until (cx> 6) or (num= ord(touv[cx]));
    if num<> ord(touv[cx]) then crep:= 187 else
    begin
     mchai:= chai;
     crep:= 999;
    end;
   end;
  end
  else
  begin
   tcoord(8);
   if num<> 0 then
   begin
    crep:= 998;
    if caff= 2 then
     if s.iloic<> 0 then crep:= 188 else s.iloic:= s.derobj;
    if caff= 13 then
    begin
     if num= 1 then
     begin
      if s.iboul<> 0 then crep:= 188 else s.iboul:= s.derobj;
     end
     else
      if s.ibag<> 0 then crep:= 188 else s.ibag:= s.derobj;
    end;
    if caff= 16 then
     if s.icryp<> 0 then crep:= 188 else s.icryp:= s.derobj;
    if caff= 24 then crep:= 185;
    if (caff= 14) or (caff= 17) then crep:= 124;
   end
   else
   begin
    crep:= 124;
    if caff= 24 then
    begin
     tcoord(5);
     if num<> 0 then crep:= 185;
    end;
   end;
  end;
  if caff= 23 then crep:= 185;
  if (crep= 999) or (crep= 185) or (crep= 998) then
  begin
   if crep= 999 then ajchai;
   if crep<> 192 then maivid;
  end;
 end;
end;

overlay procedure tecouter;
var
 l,p,haz,j,h,m: integer;
begin
 if s.mlieu<> 26 then crep:= 101 else
 begin
  if ipers<> 0 then s.conf:= s.conf+ 1;
  ecfren(p,haz,s.conf,ment);
  l:= ment;
  if l<> 0 then
   if p<> -500 then
   begin
    if haz> p then crep:= 101 else
    begin
     becfren(l);
     calch(j,h,m);
     haz:= hazard(1,100);
     if (h>= 0) and (h< 8) then
     begin
      if haz> 30 then crep:= 101 else crep:= 178;
     end
     else
      if haz> 70 then crep:= 101 else crep:= 178;
    end;
   end
   else crep:= 178;
 end;
end;

overlay procedure tmanger;
var
 j,h,m: integer;
begin
 if (s.mlieu> 15) and (s.mlieu< 26) then
  crep:= 148
 else
 begin
  tsort;
  s.mlieu:= 10;
  caff:= 10;
  debloc(s.mlieu);
  tmlieu(s.mlieu);
  calch(j,h,m);
  if (h= 12) or (h= 13) or (h= 19) then
  begin
   s.conf:= s.conf- (s.conf div 7);
   if h= 12 then
    if m= 0 then h:= 4 else h:= 3;
   if (h= 13) or (h= 19) then
    if m= 0 then h:= 2 else h:= 1;
   jh:= jh+ h;
   crep:= 135;
   tinke;
  end
  else crep:= 134;
 end;
end;

overlay procedure tentrer;
var
 x, z: integer;
begin
 if (s.mlieu= 21) or (s.mlieu= 22) then
 begin
  t1sama;
  tmlieu(s.mlieu);
 end
 else
  if s.mlieu= 15 then aldepl else
   if ment= 0 then crep:= 997 else
   begin
    if (ment= 9) and (s.derobj<> 136) then
    begin
     crep:= 189;
     s.teauto[8]:= '*';
    end
    else
    begin
     if not blo then t11(ment, z);
     if z<> 0 then
     begin
      if (ment= 3) or (ment= 7) then crep:= 179 else
      begin
       x:= (hazard(0, 10))- 5;
       parole(7, x, 1);
       aniof(1,1);
{       tkey(5,32000);}
       tip(z,x);
       s.conf:= s.conf+ 1;
       s.mlieu:= 15;
       msg[3]:= discut;
       msg[4]:= disc[x];
       syn:= true;
       if ment= 9 then
       begin
        col:= true;
        caff:= 70;
        afdes(0);
        repon(2,caff);
       end
       else col:= false;
       debloc(ment);
       ment:= 0;
      end;
     end
     else
     begin
      x:= (hazard(0, 10))- 5;
      parole(7, x, 1);
      aniof(1,1);
{      tkey(1,32000);}
      s.mlieu:= ment;
      affrep;
      debloc(s.mlieu);
      tmlieu(s.mlieu);
      ment:= 0;
      mpers:= 0;
      ipers:= 0;
     end;
    end;
   end;
end;

overlay procedure tdormir;
const
 m1= 'D‚sirez-vous vous r‚veiller?';
var
 z,j,h,m,quel: integer;
begin
 if (s.mlieu> 15) and (s.mlieu< 26) then
 begin
  crep:= 148;
  exit;
 end;
 if s.mlieu <> 0 then
 begin
  tsort;
  s.mlieu:= 0;
  affrep;
  afdes(0);
  debloc(s.mlieu);
  tmlieu(s.mlieu);
 end;
 clsf3;
 clsf2;
 ecrf2;
 ecr2(m1);
 calch(j,h,m);
 repeat
   if h< 8 then
     begin
       s.conf:= s.conf- (s.conf div 20);
       z:= (7- h)* 2;
       if m= 30 then z:= z- 1;
       jh:= jh+ z;
       h:= 7;
     end;
   jh:= jh+ 2;
   h:= h+ 1;
   if h> 23 then h:= 0;
   tinke;
   quel:= do_alert(stouinon,1);
   anyone:= false;
 until quel= 1;
 crep:= 998;
 num:= 0;
end;

overlay procedure tdefoncer;
begin
 if not syn then ecr3('d‚foncer');
 if caff< 25 then tfleche;
 if (not anyone) and (not iesc) then
    if s.mlieu<> 26 then crep:= 997 else
   begin
     crep:= 143;
     s.conf:= s.conf+ 2;
   end;
end;

overlay procedure tsortir;
var
 lx: integer;
begin
 tsort;
 crep:= 0;
 if (s.mlieu= 19) or (s.mlieu= 21) or (s.mlieu= 22)
 or (s.mlieu= 24) then crep:= 997 else
 begin
  if (s.mlieu< 16) or (s.mlieu= 26) then lx:= 10;
  if (s.mlieu= 10) or (s.mlieu= 20) then lx:= 21;
  if (s.mlieu< 10) or (s.mlieu= 13) then lx:= 15;
  if s.mlieu= 16 then
  begin
   lx:= 17;
   crep:= 176;
  end;
  if s.mlieu= 17 then t23coul(lx);
  if s.mlieu= 23 then lx:= 24;
  if crep<> 997 then s.mlieu:= lx;
  caff:= lx;
  if crep= 0 then crep:= lx;
  debloc(lx);
  tmlieu(lx);
 end;
end;

overlay procedure tattendre;
var
 quel: integer;
begin
 mpers:= 0;
 clsf3;
 repeat
   jh:= jh+ 1;
   tinke;
   if not blo then t11(s.mlieu, quel);
   if (ipers<> 0) and (mpers= 0) then
   begin
     crep:= 998;
     if (s.mlieu= 13) or (s.mlieu= 14) then cavegre;
     if (s.mlieu> 0) and (s.mlieu< 10) then anyone:= true;
     mpers:= ipers;
     if not anyone then tinke;
     exit;
   end;
   repon(2,102);
   quel:= do_alert(stouinon,1);
 until quel= 2;
 crep:= 998;
 if not anyone then tinke;
end;

overlay procedure tsonder;
begin
 if not syn then ecr3('sonder');
 if caff< 27 then
 begin
  tfleche;
  if not (anyone) and (not iesc) then crep:= 145;
  num:= 0;
 end;
end;

overlay procedure tparler;

var
                               te : array[1..46] of boolean;
       ix, cy, cx, max, haz, suj, co,lig,icm,
            i, tay, choi, x, y, c : integer;
                              tou : char;
                              lib : array[1..46] of string[40];
                               st : phrase;
                                f : boolean;


begin
 finfouil;
 if col then suj:= 128 else
  begin
    cx:= 0;
    repeat
      cx:= cx+ 1;
    until disc[cx]= msg[4];
    caff:= 69+ cx;
    afdes(0);
    repon(2,caff);
    suj:= caff+ 60;
  end;
 tkey1(false);
 mennor;
 hide_mouse;
 hirs;
 premtet;
 sparl(0,suj);
 hirs;
 for ix:= 1 to 46 do te[ix]:= false;
 for ix:=1 to 45 do
     begin
       DeLine(ix+c_tparler,st,tay);
       lib[ix]:= delig;
       for i:=tay to 40 do lib[ix]:=lib[ix]+' ';
     end;
 lib[46]:=lib[45];
 lib[45]:=' ';
 show_mouse;
 repeat
   choi:= 0;
   icm:= 0;
   co:= 0;
   lig:= 0;
   repeat
     icm:= succ( icm );
     putxy(co,lig);
     if (s.teauto[icm]='*') then
     if (te[icm]) then writetp(lib[icm],1)
                  else writetp(lib[icm],0);
     if icm=23 then begin
                      lig:= 0;
                      co:= 320;
                    end
               else lig:= lig + 8;
   until icm=42;
   putxy(320,176);
   writetp(lib[46],0);
   tou:= #0;
   repeat
     mov_mouse(f,tou);
(*     if keypressed then read(kbd,tou);*)
     read_pos_mouse(x,y,c);
     x:= x *(3-res);
     if x>319 then cx:= 41 else cx:= 1;
     cy:= succ(y shr 3);                    { 0-199 => 1-25 }
     if (cy>23) or ((cx=41) and (cy in [20..22])) then
             begin
               if choi<>0 then
                  begin
                    lig:= ((choi-1) mod 23) shl 3 ;
                    if choi>23 then co:= 320 else co:= 0;
                    putxy(co,lig);
                    if te[choi] then writetp(lib[choi],0)
                                else writetp(lib[choi],1);
                    te[choi]:= not te[choi];
                    choi:= 0;
                  end;
             end
        else begin
               ix:= cy;
               if cx=41 then ix:= ix+23;
               if ix<>choi then
                  begin
                    if choi<>0 then
                       begin
                         lig:= ((choi-1) mod 23) shl 3 ;
                         if choi>23 then co:= 320 else co:= 0;
                         putxy(co,lig);
                         if te[choi] then writetp(lib[choi],0)
                                     else writetp(lib[choi],1);
                         te[choi]:= not te[choi];
                       end;
                    if (s.teauto[ix]='*') or (ix=46) then
                       begin
                         lig:= ((ix-1) mod 23) shl 3 ;
                         if ix>23 then co:= 320 else co:= 0;
                         putxy(co,lig);
                         if te[ix] then writetp(lib[ix],0)
                                   else writetp(lib[ix],1);
                         te[ix]:= not te[ix];
                         choi:= ix;
                       end
                      else choi:= 0;
                  end;
             end;
   until (tou=#13) or ( ((c<>0) or clic) and (choi<>0));
   clic:=false;
   if choi<>46 then
      begin
        ix:= choi-1;
        if col then
           begin
             col:= false;
             s.mlieu:= 15;
             if iouv> 0 then max:= 8 else max:= 4;
             haz:= hazard(1,max);
             if haz= 2 then suj:= 129 else
                begin
                  suj:= 138;
                  s.conf:= s.conf+ (3* (s.conf div 10));
                end;
           end
          else
           if nbrep[caff- 69]< nbrepm[caff- 69] then
           begin
             suj:= tabdon[arep+ (ix shl 3)+ (caff- 70)];
             s.conf:= s.conf+ tabdon[arcf+ ix];
             nbrep[caff- 69]:= nbrep[caff- 69]+ 1;
           end
          else
           begin
             s.conf:= s.conf+ 3;
             suj:= 139;
           end;
        hide_mouse;
        hirs;
        premtet;
        sparl(0, suj);
        show_mouse;
        if (suj= 84) or (suj= 86) then
           begin
             s.pourc[5]:= '*';
             s.teauto[7]:= '*';
           end;
        if (suj= 106) or (suj= 108) or (suj= 94) then
           begin
             for ix:= 29 to 31 do s.teauto[ix]:= '*';
             s.pourc[7]:= '*';
           end;
        if suj= 70 then
           begin
             s.pourc[8]:= '*';
             s.teauto[32]:= '*';
           end;
        hide_mouse;
        hirs;
        show_mouse;
      end;
    until (choi= 46) or (suj= 138);
 if col then
 begin
  s.conf:= s.conf+ (3* (s.conf div 10));
  hide_mouse;
  hirs;
  premtet;
  sparl(0, 138);
  show_mouse;
  col:= false;
  s.mlieu:= 15;
 end;
 ctrm:= 0;
 hide_mouse;
 hirs;
 dessine_rouleau;
 show_mouse;
 affper(ipers);
 tinke;
 pendule;
 affrep;
(* chech;*)
 tmlieu(s.mlieu);
 clsf3;
end;

overlay procedure tsentir;
begin
 crep:= 119;
 if caff< 26 then
 begin
  if not syn then ecr3('sentir');
  tfleche;
  if not (anyone) and not (iesc) then
   if caff= 16 then crep:= 153;
 end
 else
 if caff= 123 then crep:= 110;
 num:= 0;
end;

overlay procedure tgratter;
begin
 crep:= 155;
 if caff< 27 then
 begin
  if not syn then ecr3('gratter');
  tfleche;
 end;
 num:= 0;
end;

(* NIVEAU 2 *)
overlay procedure tmaj1;         { Le jeu est termin‚ !!! }
begin
 arret:= true;
 tlu(13,152);
 maivid;
 clsf1;
 clsf2;
 clsf3;
 repon(9,1509);
 tkey1(false);
 hide_mouse;
 caff:= 70;
 taffich;
 hirs;
 premtet;
 sparl(0,141);
 show_mouse;
 clsf1;
 repon(9,1509);
 repon(2,142);
 tkey1(false);
 caff:= 32;
 afdes(0);
 repon(6,34);
 repon(2,35);
 musique(0);
 tkey1(false);
 messint(2036);
 tkey1(false);
 inzon;
end;

overlay procedure tencore;       { Perdu !!! }
var
 quel: integer;
begin
 clsf2;
 musique(0);
 tkey1(false);
 maivid;
 inzon;
 dprog;
 vh:= 10;
 vm:= 0;
 vj:= 0;
 min:= 0;
 heu:= 10;
 jou:= 0;
 repon(2,180);
 quel:= do_alert(stouinon,1);
 arret:= (quel<>1);
end;
