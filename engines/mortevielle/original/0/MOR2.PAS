const  men : array[1..11] of integer
             = ( scacher, attacher, defoncer, dormir,
                 entrer,  fermer,   frapper,  manger,
                 mettre,  ouvrir,   sortir );

procedure tinke;
const
 m1= 'Mince! Vous entendez du bruit...';
 d1= ' | Vous devriez avoir remarqu‚|       ';
 d2= '% des indices...';
 d3= '[';
 d4= ']';
 d5= '1';
 d6= 'OK';
var
 cx,cy,haz,nh,cf,j,h,m,cd: integer;
    stpo : string[3];
      am : boolean;
begin
 anyone:= false;
 calch(j,h,m);
 if j<> jou then
 begin
  jou:= j;
  cx:=0;
  repeat
    cx:=cx+1;
    if nbrepm[cx]<> 0 then nbrepm[cx]:= nbrepm[cx]- 1;
    nbrep[cx]:= 0;
  until cx=8;
 end;
 if (h> heu) or ((h= 0) and (heu= 23)) then
 begin
  heu:= h;
  min:= 0;
  pendule;
  cf:= 0;
  for cx:= 1 to 10 do if s.pourc[cx]= '*' then cf:= cf+ 1;
  if cf=10 then stpo:='10'
           else stpo:=chr(cf+48);
  stpou:= d3+ d5+ d4+ d3+ d1+ stpo+ '0'+ d2+ d4+ d3+ d6+ d4;
 end;
 if m> min then
 begin
  min:= 30;
  pendule;
 end;
 if Y_S<12 then exit;
 if not blo then
 begin
  if (h= 12) or ((h> 18) and (h< 21)) or ((h>= 0) and (h< 7)) then
   t:= ti2 else t:= ti1;
  cf:= s.conf;
  if (cf> 33) and (cf< 66) then t:= t- (t div 3);
  if cf> 65 then t:= t- ((t div 3)* 2);
  nh:= readclock;
  if (nh- mh)> t then
  begin
    am:= active_menu;
    erase_menu;
    jh:= jh+ ((nh- mh)div t);
    mh:= nh;
    case li of
            1,4 : pl1(cf);
              2 : pl2(cf);
              5 : pl5(cf);
            6,8 : pl6(cf);
              9 : pl9(cf);
             10 : pl10(cf);
             11 : pl11(cf);
             12 : pl12(cf);
          13,14 : pl13(cf);
          15,26 : pl15(cf);
             20 : pl20(cf);
        end;
   if (mpers<> 0) and (ipers<> 10) then mpers:= ipers;
   if (mpers= 0) and (ipers> 0) then
    if (s.mlieu= 13) or (s.mlieu= 14) then cavegre else
     if ipers= 10 then
     begin
      ipers:= 0;
      if not brt then
      begin
       brt:= true;
       hdb:= readclock;
       haz:= hazard(1,5);
       if haz< 5 then
       begin
        clsf3;
        ecrf2;
        ecr3(m1);
        haz:= (hazard(0, 4))- 2;
        parole(1, haz, 1);
        clsf3;
       end;
      end;
     end;
    if am then draw_menu;
  end;
 end;
 hfb:= readclock;
 if (brt) and ((hfb- hdb)> 17) then
 begin
  nouvp(li,cx);
  brt:= false;
  hdb:= 0;
  if (s.mlieu> 0) and (s.mlieu< 10) then anyone:= true;
 end;
end;

procedure fenat(ans: char);
  var coul : integer;
begin
  hide_mouse;
  if Gd=cga then coul:=2
            else if Gd=Her then coul:=1 else coul:=12;
  affcar(Gd,306,193,coul,ord(ans));
  boite(300, 191, 16, 8, 15);
  show_mouse;
end;


(* NIVEAU 8 *)
procedure afdes(ad: integer);
begin
  taffich;
  dessin(ad);
  okdes:= false;
end;

procedure tkey1(d: boolean);
  var quest : boolean;
      x,y,c:integer;
      key : integer;
begin
 hide_mouse;
 fenat('K');
 while keypressed do key:=testou;
 repeat
   if d then tinke;
   quest:=keypressed;
   read_pos_mouse(x,y,c);
 until quest or (c<>0) or (d and anyone);
 if quest then key:=testou;
 clic:=false;
 show_mouse;
end;

procedure tmlieu(mli: integer);
var
 cx, j, i, tail : integer;
           nomp : str30;
             st : phrase;

begin
  if mli= 26 then mli:= 15;
  i:=1;
  while (i<8) and (v_lieu[i,mli]<>0)  do
     begin
       cx:=v_lieu[i,mli];
       DeLine(cx+c_tmlieu,st,tail);
       nomp:= delig;
       while length(nomp)<30 do nomp:=nomp+' ';
       menut( depl[i], nomp);
       i:=i+1;
     end;
  nomp:= '*                   ';
  for cx:= 7 downto i do menut( depl[cx], nomp);
end;


(* NIVEAU 7 *)
procedure tlu(af,ob: integer);
begin
  caff:= 32;
  afdes(0);
  repon(6,ob+ 4000);
  repon(2,999);
  tkey1(true);
  caff:= af;
  msg[3]:= no_choice;
  crep:= 998;
end;

procedure delin(n:integer);
  var s:phrase;
      t : integer;
  begin
    deline(n,s,t);
  end;

procedure affrep;
begin
  caff:= s.mlieu;
  crep:= s.mlieu;
end;

procedure mfouen;

var
 cx: integer;
begin
 tmlieu(s.mlieu);
 for cx:=1 to 11 do menu_enable( men[cx]);
(* menu_enable( scacher);
 menu_enable(menup, attacher);
 menu_enable(menup, defoncer);
 menu_enable(menup, dormir);
 menu_enable(menup, entrer);
 menu_enable(menup, fermer);
 menu_enable(menup, frapper);
 menu_enable(menup, manger);
 menu_enable(menup, mettre);
 menu_enable(menup, ouvrir);
 menu_enable(menup, sortir);  *)
 menut( sonder, ' sonder   ');
 menut( soulever, ' soulever ');
end;

procedure atf3f8(var key: integer);
 begin
   repeat
     key:= testou;
   until (key=61) or (key=66);
 end;

(* NIVEAU 6 *)

procedure tperd;
var
 cx: integer;
begin
 initouv;
 ment:= 0;
 iouv:= 0;
 mchai:= 0;
 mfouen;
 if not blo then t11(21, cx);
 perdu:= true;
 clsf1;
 boite( 60, 35, 400, 50, 15);
 repon(9,crep);
 clsf2;
 clsf3;
 col:= false;
 syn:= false;
 okdes:= false;
end;

procedure tsort;
var
 cx: integer;
begin
 if (iouv> 0) and (s.mlieu<> 0) then
  if s.conf< 50 then s.conf:= s.conf+ 2
                else s.conf:= s.conf+ (s.conf div 10);
 for cx:= 1 to 7 do touv[cx]:= chr(0);
 ment:= 0;
 iouv:= 0;
 mchai:= 0;
 debloc(s.mlieu);
end;

procedure st4(ob: integer);
begin
 crep:= 997;
 case ob of
                114 : crep:=109;
                110 : crep:=107;
                158 : crep:=113;
    152..156,150,
    100,157,160,161 : tlu(caff,ob);
 end;
end;

procedure cherjer(ob: integer; var d: boolean);
var
 cx: integer;
begin
 d:= false;
 for cx:= 1 to 6 do d:=(d or (ord(s.sjer[cx])= ob));
 if s.derobj= ob then d:= true;
end;

procedure st1sama;
begin
 s.mlieu:= 10;
 affrep;
end;

procedure modinv;
var
 cx, cy, tay, i, r: integer;
 nom: phrase;
 nomp: str30;
begin
 cy:= 0;
 for cx:=1 to 6 do
  if s.sjer[cx]<> chr(0) then
    begin
      cy:= succ(cy);
      r:= (ord(s.sjer[cx])+ 400);
      DeLine(r-501+c_st41,nom,tay);
      nomp:= delig;
      menut( invt[cy], nomp);
      menu_enable( invt[cx]);
    end;
 if cy<6 then
    for cx:=cy+1 to 6 do
      begin
        menut( invt[cx], '                       ');
        menu_disable( invt[cx]);
      end;
end;

procedure sparl(adr, rep: long_integer);
const haut: array[1..8] of integer
           =(0,1,-3,6,-2,2,7,-1);
var
 key, ad, tay, repint : integer;
           st : phrase;
begin
  repint:=trunc(rep);
  hide_mouse;
  DeLine( repint+c_paroles,st,tay);
  Afftex(st,230,4,65,24,5);
  f3f8;
  (*boite(43,30,90,19,15);
  gotoxy(7,5);
  write('F3: Encore');
  gotoxy(7,6);
  write('F8: Stop');*)
  key:= 0;
  repeat
    parole(repint, haut[caff- 69], 0);
    atf3f8(key);
  until key= 66;
  hirs;
  show_mouse;
end;

procedure finfouil;
  begin
    fouil:= false;
    obpart:= false;
    cs:= 0;
    is:= 0;
    mfouen;
  end;

procedure mfoudi;
var
 cx: integer;
begin
 for cx:= 1 to 7 do menu_disable( depl[cx]);
 for cx:=1 to 11 do menu_disable( men[cx]);
(* menu_disable(menup, scacher);
 menu_disable(menup, attacher);
 menu_disable(menup, defoncer);
 menu_disable(menup, dormir);
 menu_disable(menup, entrer);
 menu_disable(menup, fermer);
 menu_disable(menup, frapper);
 menu_disable(menup, manger);
 menu_disable(menup, mettre);
 menu_disable(menup, ouvrir);
 menu_disable(menup, sortir);  *)
 menut( sonder, ' -SUITE- ');
 menut( soulever, ' -STOP-  ');
end;

procedure mennor;
begin
 menu_up(msg[3]);
end;

procedure premtet;
begin
 dessine(ades, 10, 80);
 boite(18, 79, 155, 91, 15);
end;

(* NIVEAU 5 *)
procedure ajchai;
  var cx,cy,lderobj: integer;

begin
  cy:= (acha+ pred(pred(mchai)* 10));
  cx:= 0;
  repeat
   cx:= cx+ 1;
  until (cx> 9) or (tabdon[cy+ cx]= 0);
  if tabdon[cy+ cx]= 0 then
    begin
      lderobj:= s.derobj;
      tabdon[cy+ cx]:=lderobj;
    end
    else crep:= 192;
end;

procedure ajjer(ob: integer);
var
 cx: integer;
begin
 cx:= 0;
 repeat
  cx:= cx+ 1;
 until (cx> 5) or (ord(s.sjer[cx])= 0);
 if ord(s.sjer[cx])= 0 then
 begin
  s.sjer[(cx)]:= chr(ob);
  modinv;
 end
 else crep:= 139;
end;

procedure t1sama;  (* On entre dans le manoir *)
var
 j,h,m: integer;
 d: boolean;
begin
 calch(j,h,m);
 if (h< 5) and (s.mlieu> 18) then
 begin
  cherjer(137,d);
  if not d then     (* On n'a pas les cl‚s et il est tard *)
  begin
    crep:= 1511;
    tperd;
  end
  else st1sama;
 end
 else
 if not s.ipre then     (* c'est votre premiŠre fois ? *)
 begin
  ipers:= 255;
  affper(ipers);
  caff:= 77;
  afdes(0);
  boite(223, 47, 155, 91, 15);
  repon(2,33);
  tkey1(false);
  mennor;
  hide_mouse;
  hirs;
  premtet;
  sparl(0,140);
  dessine_rouleau;
  pendule;
  show_mouse;
  s.mlieu:= 0;
  affrep;
  t5(10);
  if not blo then t11(0, m);
  ipers:= 0;
  mpers:= 0;
  s.ipre:= true;
  (*chech;*)
 end
 else  st1sama;
end;

procedure t1vier;
begin
 s.mlieu:= 17;
 affrep;
end;

procedure t1neig;
begin
 inei:= inei+ 1;
 if inei> 2 then
 begin
   crep:= 1506;
   tperd;
 end
 else
 begin
   okdes:= true;
   s.mlieu:= 19;
   affrep;
 end;
end;

procedure t1deva;
begin
 inei:= 0;
 s.mlieu:= 21;
 affrep;
end;

procedure t1derr;
begin
 s.mlieu := 22;
 affrep;
end;

procedure t1deau;
begin
 crep := 1503;
 tperd;
end;

procedure tctrm;
begin
 repon(2,(3000+ ctrm));
 ctrm:= 0;
end;


procedure quelquun;
label 1;
var
 haz,cx: integer;
begin
 if imen then erase_menu;
 finfouil;
 crep:= 997;
1:
 if not cache then
 begin
  if crep= 997 then crep:= 138;
  repon(2,crep);
  if crep= 138 then parole(5, 2, 1) else parole(4, 4, 1);
  if iouv= 0 then s.conf:= s.conf+ 2 else
   if s.conf< 50 then s.conf:= s.conf+ 4 else
    s.conf:= s.conf+ (3* (s.conf div 10));
  tsort;
  tmlieu(15);
  tip(ipers,cx);
  caff:= 69+ cx;
  crep:= caff;
  msg[3]:= discut;
  msg[4]:= disc[cx];
  syn:= true;
  col:= true;
 end
 else
 begin
  haz:= hazard(1, 3);
  if haz= 2 then
  begin
   cache:= false;
   crep:= 137;
   goto 1;
  end
  else
  begin
   repon(2, 136);
   haz:= (hazard(0, 4))- 2;
   parole(3, haz, 1);
   clsf2;
   person;
   debloc(21);
   affrep;
  end;
 end;
 if imen then draw_menu;
end;

procedure tsuiv;
var
 cx,tbcl: integer;
 cl,cy: integer;
begin
 cy:= acha+ ((mchai- 1)* 10)- 1;
 cx:= 0;
 repeat
  cx:= cx+ 1;
  cs:= cs+ 1;
  cl:= cy+ cs;
  tbcl:= tabdon[cl];
 until (tbcl <> 0) or (cs> 9);
 if (tbcl <> 0) and (cs< 11) then
 begin
  is:= is+ 1;
  caff:= tbcl;
  crep:= caff+ 400;
  if ipers<> 0 then s.conf:= s.conf+ 2;
 end
 else
 begin
  affrep;
  finfouil;
  if cx> 9 then crep:= 131;
 end;
end;

procedure tfleche;
var
 qust : boolean;
 touch : char;
 dummy,cd,xmo,ymo: integer;
begin
 if num= 9999 then exit;
 fenat(chr(152));
 rect:= false;
 repeat
  touch:=#0;

  repeat
   mov_mouse(qust,touch);
   if clic then rect:=(X_S<256*res) and (Y_S<176) and (Y_S>12);
   tinke;
  until (qust or rect or anyone);

  if (qust and (touch=#67)) then dummy:=do_alert(stpou,1);
 until ((touch=#59) or ((touch= #68) and (x<> 0) and (y<>0)) or
       (anyone) or (rect));
 if touch=#59 then iesc:= true;
 if rect then
   begin
     x:=X_S;
     y:=Y_S;
   end;
end;

procedure tcoord(sx: integer);
label 1;
     var sy,ix,iy,cb,cy,ib: integer;
         a, b, atdon : integer;

   begin
     num:= 0;
     crep:= 999;
     a:=0;
     atdon:=amzon+3;
     cy:= 0;
     while cy<caff do
        begin
          a:=a+tabdon[atdon];
          atdon:=atdon+4;
          cy:= succ(cy);
        end;
(*     for cy:=0 to caff-1 do
          a:=a+ tabdon[amzon+3+4*cy];
     b:= peek(amzon+3+4*caff); *)
     b:= tabdon[atdon];
     if b= 0 then goto 1;
     a:= a+ fleche;
     cb:= 0;
     for cy:= 0 to (sx-2) do
      begin
        ib:= tabdon[a+cb] shl 8+ tabdon[succ(a+ cb)] ;
        cb:= cb+ (ib* 4)+ 2;
      end;
      ib:= tabdon[a+cb] shl 8+ tabdon[succ(a+ cb)] ;
      if ib= 0 then goto 1;
      cy:= 1;
      repeat
        cb:= cb+ 2;
        sx:= tabdon[a+ cb] * res;
        sy:= tabdon[succ(a+ cb)];
        cb:= cb+ 2;
        ix:= tabdon[a+ cb] * res;
        iy:= tabdon[succ(a+ cb)];
        cy:= cy+ 1;
      until ((x>= sx) and (x<= ix) and (y>= sy) and (y<= iy))
             or (cy> ib);
      if (x>= sx) and (x<= ix) and (y>= sy) and (y<= iy)
        then
          begin
            num:= cy- 1;
            exit;
          end;
1:
     crep:= 997;
   end;


procedure st7(ob: integer);
begin
 crep:= 183;
 if (ob= 116) or (ob= 144) then crep:= 104;
 if (ob= 126) or (ob= 111) then crep:= 108;
 if ob= 132 then crep:= 111;
 if ob= 142 then crep:= 112;
 if crep= 183 then st4(ob);
end;

procedure treg(ob: integer);
var
 mdes: integer;
begin
 mdes:= caff;
 caff:= ob;
 if ((caff> 29) and (caff< 33)) or (caff= 144) or
    (caff= 147) or (caff= 149) or (msg[4]= sregarder) then
 begin
  afdes(0);
  if ((caff> 29) and (caff< 33)) then repon(2,caff) else repon(2,(caff+ 400));
  tkey1(true);
  caff:= mdes;
  msg[3]:= 0;
  crep:= 998;
 end
 else
 begin
  obpart:= true;
  crep:= caff+ 400;
  mfoudi;
 end;
end;

procedure avpoing(var ob: integer);
begin
 crep:= 999;
 if s.derobj<> 0 then ajjer(s.derobj);
 if crep<> 139 then
 begin
  modobj(ob+ 400);
  s.derobj:= ob;
  ob:= 0;
 end;
end;

procedure rechai(var ch: integer);
var
 cx: integer;
begin
 cx:= s.mlieu;
 if s.mlieu= 16 then cx:= 14;
 ch:= tabdon[achai+ (cx* 7)+ pred(num)];
end;

procedure t23coul(var l: integer);
var
 d: boolean;
begin
 cherjer(143,d);
 l:= 14;
 if not d then
 begin
  crep:= 1512;
  tperd;
 end;
end;

procedure maivid;
begin
 s.derobj:= 0;
 modobj(500);
end;

procedure st13(ob: integer);
begin
 if (ob= 114) or (ob= 116) or (ob= 126) or (ob= 132) or
 (ob= 111) or (ob= 106) or (ob= 102) or (ob= 100) or
 (ob= 110) or (ob= 153) or (ob= 154) or (ob= 155) or
 (ob= 156) or (ob= 157) or (ob= 144) or (ob= 158) or
 (ob= 150) or (ob= 152) then crep:= 999 else crep:= 105;
end;

procedure aldepl;
  var dummy:integer;
begin
  dummy:=do_alert('[1][Alors, utilisez le menu DEP...][ok]',1);
end;
