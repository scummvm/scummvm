(* Niveau 14 suite *)
const
 m1= 'quelqu''un entre, parait ‚tonn‚ mais ne dit rien';


procedure testfi;
   begin
     if ioresult<> 0 then
        begin
          caff:= do_alert(err_mess,1);
          halt;
        end;
   end;


function readclock: integer;
     var       r : registres;
         s, m, h : integer;
begin
(* debug('readclock');*)
  r.ax := $2C00;
  intr($21,r);
  with r do
  begin
    h:=cx shr 8;
    m:=cx mod 256;
    s:=dx shr 8;
  end;
  m:= m * 60;
  h:= h* 3600;
  readclock:= h+ m+ s;
end;

procedure modif(var nu: integer);
begin
 (* debug('modif'); *)
 if nu= 26 then nu:= 25;
 if (nu> 29) and (nu< 36) then nu:= nu- 4;
 if (nu> 69) and (nu< 78) then nu:= nu- 37;
 if (nu> 99) and (nu< 194) then nu:= nu- 59;
 if (nu> 996) and (nu< 1000) then nu:= nu- 862;
 if (nu> 1500) and (nu< 1507) then nu:= nu- 1363;
 if (nu> 1507) and (nu< 1513) then nu:= nu- 1364;
 if (nu> 1999) and (nu< 2002) then nu:= nu- 1851;
 if nu= 2010 then nu:= 151;
 if (nu> 2011) and (nu< 2025) then nu:= nu- 1860;
 if nu= 2026 then nu:= 165;
 if (nu> 2029) and (nu< 2037) then nu:= nu- 1864;
 if (nu> 3000) and (nu< 3005) then nu:= nu- 2828;
 if nu= 4100 then nu:= 177;
 if nu= 4150 then nu:= 178;
 if (nu> 4151) and (nu< 4156) then nu:= nu- 3973;
 if nu= 4157 then nu:= 183;
 if (nu= 4160) or (nu= 4161) then nu:= nu- 3976;
end;


procedure dessine(ad, x, y: integer);
begin
  (* debug('dessine'); *)
  hide_mouse;
  writepal(numpal);
  PictOut(ad,0,x,y);
  show_mouse;
end;

procedure dessine_rouleau;
begin
  (* debug('dessine_rouleau'); *)
  writepal(89);
  if gd=Her then
     begin
       mem[$7000:14]:=15;
     end;
  hide_mouse;
  PictOut($73A2,0,0,0);
  show_mouse;
end;


procedure text_color(c:integer);
begin
  color_txt:= c;
end;

(* NIVEAU 13 *)


procedure text(x,y,nb,m: integer);
   var st : phrase;
       tay : integer;
       co : integer;

begin
  (* debug('text'); *)
  if res=1 then co:=10 else co:=6;
  DeLine(m,st,tay);
  if (y=182) and (tay*co>nb*6) then y:=176;
  Afftex(st,x,y,nb,20,color_txt);
end;

procedure initouv;
  var cx:integer;
 begin
   (* debug('initouv'); *)
   for cx:= 1 to 7 do touv[cx]:= chr(0)
 end;

procedure ecrf1;
begin
  boite(0, 11, 512, 163, 15);           (* grand dessin *)
end;

procedure clsf1;
  var i,j:integer;
begin
 (* debug('clsf1'); *)
 hide_mouse;
 box(0,Gd, 0,11, 514,175, 255);
(* if Gd=ams then port[$3DD]:=15;
 case Gd of
         ams,
         cga : begin
                 for j:=0 to 63 do
                    begin
                      for i:=6 to 86 do
                         begin
                           mem[$B800:i*80+j]:=0;
                           mem[$Ba00:i*80+j]:=0;
                         end;
                      mem[$b800:6960+j]:=0;
                      mem[$ba00:400+j]:=0;
                    end;
               end;
         ega : begin
                 port[$3C4]:= 2;
                 port[$3C5]:= 15;
                 port[$3CE]:= 8;
                 port[$3CF]:= 255;
                 for j:=0 to 63 do
                    for i:=11 to 174 do
                       mem[$A000:i*80+j]:=0;
               end;
         tan : begin
                 for j:=0 to 128 do
                    begin
                      for i:=3 to 43 do
                         begin
                           mem[$B800:i*160+j]:=0;
                           mem[$Ba00:i*160+j]:=0;
                           mem[$bc00:i*160+j]:=0;
                           if i<>43 then mem[$be00:i*160+j]:=0;
                         end;
                      mem[$be00:320+j]:=0;
                    end;
               end;
 end;
 droite(256*res,11,256*res,175,0);*)
 show_mouse;
end;

procedure clsf2;
   var i,j:integer;
begin
 (* debug('clsf2'); *)
 hide_mouse;
 if f2_all then
    begin
      box(0,Gd,1,176,633,199,255);
      boite(0,175,634,24,15);
      f2_all:=false;
    end
  else
    begin
      box(0,Gd,1,176,633,190,255);
      boite(0,175,634,15,15);
    end;
 show_mouse;
end;

procedure ecrf2;
begin
  (* debug('ecrf2'); *)
  text_color(5);
  (*boite(0,175,630,15,15);*)
end;

procedure ecr2(str: str255);
  var tab : integer;
      tlig : integer;
begin
 (* debug('ecr2 : '+str);*)
 if res=1 then tab:=10 else tab:=6;
 putxy(8,177);
 tlig:=59+pred(res)*36;
 if length(str)<tlig then writeg(str,5)
  else if length(str)<tlig shl 1 then
          begin
            putxy(8,176);
            writeg(copy(str,1,pred(tlig)),5);
            putxy(8,182);
            writeg(copy(str,tlig,tlig shl 1),5);
          end
   else begin
          f2_all:=True;
          clsf2;
          putxy(8,176);
          writeg(copy(str,1,pred(tlig)),5);
          putxy(8,182);
          writeg(copy(str,tlig,pred(tlig shl 1)),5);
          putxy(8,190);
          writeg(copy(str,tlig shl 1,tlig*3),5);
        end;
end;

procedure clsf3;
   var i,j:integer;
begin
  (* debug('clsf3'); *)
  hide_mouse;
  box(0,Gd,1,192,633,199,255);
  boite(0,191,634,8,15);
  show_mouse;
end;

procedure ecr3(text: str255);
begin
 (* debug('ecr3 : '+text);*)
 clsf3;
 putxy(8,192);
 writeg(text,5);
end;

procedure ecrf6;
begin
  (* debug('ecrf6'); *)
  text_color(5);
  boite(62, 33, 363, 80, 15);
end;

procedure ecrf7;
begin
 text_color(4);
{ draw_mode(2);
 text_height(4*res);
 text_style(normal);}
end;

procedure clsf10;
  var co,cod : integer;
      st : string[8];
begin
 (* debug('clsf10'); *)
 hide_mouse;
 if res=1 then
    begin
      co:=634;
      cod:=534
    end
  else
    begin
      co:=600;
      cod:=544;
    end;
 box(15,Gd, cod, 93, co, 98, 255);
 if s.conf< 33 then st:='Cool'
               else
 if s.conf< 66 then st:=' Lourde '
               else
 if s.conf> 65 then st:='Malsaine';
 if res=1 then co:=10 else co:=6;
 co:= 574-co*length(st) shr 1;
 putxy(co,92);
 writeg(st,4);
 if res=1 then co:=620 else co:=584;
 box(15,Gd, 560, 24, co, 86, 255);
(* rempli(69,12,32,5,255);*)
 show_mouse;
end;

   procedure stop;
     begin
       clrscr;
       hirs;
       Gd:=ams;
       hirs;
       halt;
     end;

procedure paint_rect(x, y, dx, dy : integer);
     var co:integer;
   begin
     (* debug('paint_rect'); *)
     if gd=cga then co:= 3
               else co:=11;
     box(co,Gd,x,y,x+dx,y+dy,255);
   end;

function hazard(min,max: integer): integer;
     var ha1: integer;
         ha2: real;
   begin
     (* debug('hazard'); *)
     ha2:= random;
     ha1:= trunc(ha2*(max-min));
     hazard:= min+ ha1;
   end;

procedure calch(var j,h,m: integer);
var
 th,nh: integer;
begin
(* debug('calch');*)
 nh:= readclock;
 th:= jh+ ((nh- mh) div t);
 m:= ((th mod 2)+ vm)* 30;
 h:= (th shr 1)+ vh;
 if m= 60 then
 begin
  m:= 0;
  h:= h+ 1;
 end;
 j:= (h div 24)+ vj;
 h:= h- ((j- vj)* 24);
end;

procedure conv(x : integer; var y: integer);
var
 cx: integer;
begin
 (* debug('conv'); *)
 cx:= 1;
 y:= 128;
 while cx< x do
 begin
  y:= y shr 1;
  cx:= succ(cx);
 end;
end;

(* NIVEAU 12 *)
procedure okpas;
begin
  tesok:=True;
end;

procedure modobj(m: integer);
var
 str: phrase;
 strp: str30;
 i,tay: integer;
begin
 (* debug('modobj'); *)
 strp:=' ';
 if m<> 500 then
    begin
      DeLine(m- 501+ c_st41,str,tay);
      strp:= delig;
    end;
 menut( invt[8], strp);
 menu_disable( invt[8]);
end;

procedure modobj2(m: integer;t1,t2 : boolean);
var
 str: phrase;
 strp: str30;
 i,tay: integer;
begin
 (* debug('modobj'); *)
  strp:=' ';
 if t1 or t2 then okpas
             else tesok:=False;;
 if m<> 500 then
    begin
      DeLine(m- 501+ c_st41,str,tay);
      strp:= delig;
    end;
 menut( invt[8], strp);
 menu_disable( invt[8]);
end;


procedure repon(f,m: integer);
var
 str: str255;
 str1: string[40];
 st : phrase;
 fic: text;
 i, xco, dx, caspe, tay : integer;
begin
 (* debug('repon fenetre nø'+chr(f+48));*)
 if (m> 499) and (m< 563) then
 begin
  DeLine( m-501+c_st41,st,tay);
  if tay>(58+pred(res)*37) shl 1 then f2_all:= True
                                 else f2_all:= False;
  clsf2;
  Afftex(st,8,176,85,3,5);
 end
 else
 begin
  modif(m);
  if f= 8 then f:= 2;
  if f= 1 then f:= 6;
  if f= 2 then
  begin
   clsf2;
   ecrf2;
   text(8, 182, 103, m);
   if (m= 68) or (m= 69) then s.teauto[40]:= '*';
   if (m= 104) and (caff= 14) then
   begin
    s.teauto[36]:= '*';
    if s.teauto[39]= '*' then
    begin
     s.pourc[3]:= '*';
     s.teauto[38]:= '*';
    end;
   end;
  end;
  if f in [6,9] then
  begin
   DeLine(m,st,tay);
   if f=6 then i:=4
          else i:=5;
   Afftex(st,80,40,60,25,i);
   if m= 180 then s.pourc[6]:= '*';
   if m= 179 then s.pourc[10]:= '*';
  end;
  if f= 7 then          (* messint *)
  begin
   ecrf7;
   DeLine(m,st,tay);
   if res=1 then
      begin
        xco:=252-tay*5;
        caspe:= 100;
        dx:= 80;
      end
    else
      begin
        xco:=252-tay*3;
        caspe:= 144;
        dx:=50;
      end;
   if tay<40 then Afftex(st,xco,86,dx,3,5)
             else Afftex(st,caspe,86,dx,3,5);
  end;
 end;
end;

procedure f3f8;
  var co:integer;
begin
  if res=1 then co:=107 else co:=64;
  putxy(3,44);
  writeg(f3,5);
  ywhere:= 51;
  writeg(f8,5);
  boite(0,42,co,16,7);
end;

procedure t5(cx: integer);
begin
 (* debug('t5'); *)
 if cx= 10 then blo:= false;
 if cx<> 1 then
 begin
  bh1:= false;
  bf1:= false;
 end;
 if cx<> 2 then bh2:= false;
 if cx<> 4 then
 begin
  bh4:= false;
  bf4:= false;
 end;
 if cx<> 5 then bh5:= false;
 if cx<> 6 then bh6:= false;
 if cx<> 8 then bh8:= false;
 if cx<> 3 then bt3:= false;
 if cx<> 7 then bt7:= false;
 if cx<> 9 then bh9:= false;
end;

procedure affper(per: integer);
var
 cx: integer;
begin
 (* debug('affper'); *)
 for cx:= 1 to 8 do menu_disable( disc[cx]);
 clsf10;
 if (per and 128)= 128 then
 begin
   putxy(560,24);
   writeg('LEO',4);
   menu_enable( disc[1]);
 end;
 if (per and 64)= 64 then
 begin
  putxy(560, 32);
  writeg('PAT',4);
  menu_enable( disc[2]);
 end;
 if (per and 32)= 32 then
 begin
  putxy(560, 40);
  writeg('GUY',4);
  menu_enable( disc[3]);
 end;
 if (per and 16)= 16 then
 begin
  putxy(560,48);
  writeg('EVA',4);
  menu_enable( disc[4]);
 end;
 if (per and 8)= 8 then
 begin
  putxy(560,56);
  writeg('BOB',4);
  menu_enable( disc[5]);
 end;
 if (per and 4)= 4 then
 begin
  putxy(560, 64);
  writeg('LUC',4);
  menu_enable( disc[6]);
 end;
 if (per and 2)= 2 then
 begin
  putxy(560, 72);
  writeg('IDA',4);
  menu_enable( disc[7]);
 end;
 if (per and 1)= 1 then
 begin
  putxy(560, 80);
  writeg('MAX',4);
  menu_enable( disc[8]);
 end;
 ipers:= per;
end;

overlay procedure choix(min,max: integer; var per: integer);
var
 i: boolean;
 haz,cx,cy,cz: integer;
 per2,cz2: long_integer;
begin
 (* debug('o0 choix'); *)
 haz:= hazard(min,max);
 if haz> 4 then
 begin
  haz:= 8- haz;
  i:= true
 end
 else i:= false;
 cx:= 0;
 per:= 0;
 while cx< haz do
 begin
  cy:= hazard(1,8);
  conv(cy,cz);
  if (per and cz)<> cz then
  begin
   cx:= cx+ 1;
   per:= (per or cz);
  end;
 end;
 if i then per:= 255- per;
 i:= false;
end;

overlay procedure cpl1(var p: integer);
var
 j,h,m: integer;
begin
 (* debug('o0 cpl1'); *)
 calch(j,h,m);
 if (h> 7) or (h< 11) then p:= 25;
 if (h> 10) and (h< 14) then p:= 35;
 if (h> 13) and (h< 16) then p:= 50;
 if (h> 15) and (h< 18) then p:= 5;
 if (h> 17) and (h< 22) then p:= 35;
 if (h> 21) and (h< 24) then p:= 50;
 if (h>= 0) and (h< 8) then p:= 70;
 mdn;
end;

overlay procedure cpl2(var p: integer);
var
 j,h,m: integer;
begin
 (* debug('o0 cpl2'); *)
 calch(j,h,m);
 if (h> 7) and (h< 11) then p:= -2;
 if h= 11 then p:= 100;
 if (h> 11) and (h< 23) then p:= 10;
 if h= 23 then p:= 20;
 if (h>= 0) and (h< 8) then p:= 50;
end;

overlay procedure cpl3(var p: integer);
var
 j,h,m: integer;
begin
 (* debug('o0 cpl3'); *)
 calch(j,h,m);
 if ((h> 8) and (h< 10)) or ((h> 19) and (h< 24)) then p:= 34;
 if ((h> 9) and (h< 20)) or ((h>= 0) and (h< 9)) then p:= 0;
end;

overlay procedure cpl5(var p: integer);
var
 j,h,m: integer;
begin
 (* debug('o0 cpl5'); *)
 calch(j,h,m);
 if (h> 6) and (h< 10) then p:= 0;
 if h= 10 then p:= 100;
 if (h> 10) and (h< 24) then p:= 15;
 if (h>= 0) and (h< 7) then p:= 50;
end;

overlay procedure cpl6(var p: integer);
var
 j,h,m: integer;
begin
 (* debug('o0 cpl6'); *)
 calch(j,h,m);
 if ((h> 7) and (h< 13)) or ((h> 17) and (h< 20)) then p:= -2;
 if ((h> 12) and (h< 17)) or ((h> 19) and (h< 24)) then p:= 35;
 if h= 17 then p:= 100;
 if (h>= 0) and (h< 8) then p:= 60;
end;

procedure person;
const
 m1= 'Vous';
 m2= 'ˆtes';
 m3= 'SEUL';
var
 cf: integer;
 str: str255;
begin
 (* debug('person'); *)
 for cf:= 1 to 8 do menu_disable( disc[cf]);
 clsf10;
 putxy(560, 30);
 writeg(m1,4);
 putxy(560, 50);
 writeg(m2,4);
 putxy(560, 70);
 writeg(m3,4);
 ipers:= 0;
end;

procedure chlm(var per: integer);
begin
  (* debug('chlm'); *)
  per:= hazard(1,2);
  if per= 2 then per:= 128;
end;

procedure pendule;
const
 cv: array[1..2,1..12] of integer
    =(( 5,  8, 10,  8,  5,  0, -5,  -8, -10,  -8,  -5,   0),
      (-5, -3,  0,  3,  5,  6,  5,   3,   0,  -3,  -5,  -6) );
 x= 580;
 y= 123;
 rg= 9;
var
 h,co: integer;

begin
 (* debug('pendule'); *)
 hide_mouse;
 (*paint_rect(572,114,18,20);
 paint_rect(577,114,6,20);*)
 paint_rect(570,118,20,10);
 paint_rect(578,114,6,18);
 if Gd in [Cga,Her] then co:=0 else co:=1;
 if min= 0 then droite(x shr 1*res, y, x shr 1*res, (y- rg), co)
           else droite(x shr 1*res, y, x shr 1*res, (y+ rg), co);
 h:= heu;
 if h> 12 then h:= h- 12;
 if h= 0 then h:= 12;
 droite(x shr 1*res, y, (x+ cv[1, h]) shr 1*res, y+ cv[2, h], co);
 show_mouse;
 putxy( 568, 154);
 if heu> 11 then writeg( 'PM ',1)
            else writeg( 'AM ',1);
 putxy( 550, 160);
 if jou in [0..8] then writeg(chr(jou+49)+'& jour',1);
end;

(*************
 * NIVEAU 11 *
 *************)

procedure debloc(l: integer);
begin
(* debug('debloc'); *)
 num:= 0;
 x:= 0;
 y:= 0;
 if (l<> 26) and (l<> 15) then t5(l);
 mpers:= ipers;
end;

overlay procedure cpl10(var p,h: integer);
var
 j,m: integer;
begin
(* debug('o1 cpl10'); *)
 calch(j,h,m);
 if ((h> 7) and (h< 11)) or ((h> 11) and (h< 14))
  or ((h> 18) and (h< 21)) then p:= 100;
 if (h= 11) or ((h> 20) and (h< 24)) then p:= 45;
 if ((h> 13) and (h< 17)) or (h= 18) then p:= 35;
 if h= 17 then p:= 60;
 if (h>= 0) and (h< 8) then p:= 5;
end;

overlay procedure cpl11(var p,h: integer);
var
 j,m: integer;
begin
(* debug('o1 cpl11'); *)
 calch(j,h,m);
 if ((h> 8) and (h< 12)) or ((h> 20) and (h< 24)) then p:= 25;
 if ((h> 11) and (h< 14)) or ((h> 18) and (h< 21)) then p:= 5;
 if (h> 13) and (h< 17) then p:= 55;
 if (h> 16) and (h< 19) then p:= 45;
 if (h>= 0) and (h< 9) then p:= 0;
end;

overlay procedure cpl12(var p: integer);
var
 j,h,m: integer;
begin
(* debug('o1 cpl12'); *)
 calch(j,h,m);
 if ((h> 8) and (h< 15)) or ((h> 16) and (h< 22)) then p:= 55;
 if ((h> 14) and (h< 17)) or ((h> 21) and (h< 24)) then p:= 25;
 if (h>= 0) and (h< 5) then p:= 0;
 if (h> 4) and (h< 9) then p:= 15;
end;

overlay procedure cpl13(var p: integer);
begin
(* debug('o1 cpl13'); *)
 p:= 0;
end;

overlay procedure cpl15(var p: integer);
var
 j,h,m: integer;
begin
(* debug('o1 cpl15'); *)
 calch(j,h,m);
 if (h> 7) and (h< 12) then p:= 25;
 if (h> 11) and (h< 14) then p:= 0;
 if (h> 13) and (h< 18) then p:= 10;
 if (h> 17) and (h< 20) then p:= 55;
 if (h> 19) and (h< 22) then p:= 5;
 if (h> 21) and (h< 24) then p:= 15;
 if (h>= 0) and (h< 8) then p:= -15;
end;

overlay procedure cpl20(var p,h: integer);
var
 j,m: integer;
begin
(* debug('o1 cpl20'); *)
 calch(j,h,m);
 if h= 10 then p:= 65;
 if (h> 10) and (h< 21) then p:= 5;
 if (h> 20) and (h< 24) then p:= -15;
 if (h>= 0) and (h< 5) then p:= -300;
 if (h> 4) and (h< 10) then p:= -5;
end;

overlay procedure quelq1(l: integer);
var
 per: integer;
begin
(* debug('o1 quelq1'); *)
 per:= hazard(1,2);
 if l= 1 then
  if per= 1 then bh1:= true else bf1:= true;
 if l= 4 then
  if per= 1 then bh4:= true else bf4:= true;
 ipers:= 10;
end;

overlay procedure quelq2;
begin
(* debug('o1 quelq2'); *)
 if li= 2 then bh2:= true else bh9:= true;
 ipers:= 10;
end;

overlay procedure quelq5;
begin
(* debug('o1 quelq5'); *)
 bh5:= true;
 ipers:= 10;
end;

overlay procedure quelq6(l: integer);
var
 per: integer;
begin
(* debug('o1 quelq6'); *)
 if l= 6 then bh6:= true;
 if l= 8 then bh8:= true;
 ipers:= 10;
end;

overlay procedure quelq10(h: integer; var per: integer);
var
 min,max: integer;
begin
(* debug('o1 quelq10'); *)
 if (h>= 0) and (h< 8) then chlm(per) else
 begin
  if (h> 7) and (h< 10) then
  begin
   min:= 5;
   max:= 7;
  end;
  if (h> 9) and (h< 12) then
  begin
   min:= 1;
   max:= 4;
  end;
  if ((h> 11) and (h< 15)) or ((h> 18) and (h< 21)) then
  begin
   min:= 6;
   max:= 8;
  end;
  if ((h> 14) and (h< 19)) or ((h> 20) and (h< 24)) then
  begin
   min:= 1;
   max:= 5;
  end;
  choix(min,max,per);
 end;
 affper(per);
end;

overlay procedure quelq11(h: integer; var per: integer);
var
 min,max: integer;
begin
(* debug('o1 quelq11'); *)
 if (h>= 0) and (h< 8) then chlm(per) else
 begin
  if ((h> 7) and (h< 10)) or ((h>20) and (h< 24)) then
  begin
   min:= 1;
   max:= 3;
  end;
  if ((h> 9) and (h< 12)) or ((h> 13) and (h< 19)) then
  begin
   min:= 1;
   max:= 4;
  end;
  if ((h> 11) and (h< 14)) or ((h> 18) and (h< 21))then
  begin
   min:= 1;
   max:= 2;
  end;
  choix(min,max,per);
 end;
 affper(per);
end;

overlay procedure quelq12(var per: integer);
begin
(* debug('o1 quelq12'); *)
 chlm(per);
 affper(per);
end;

overlay procedure quelq15(var per: integer);
var
 cx: integer;
 test: boolean;

begin
(* debug('o1 quelq15'); *)
 per:= 0;
 if per= 0 then
 begin
   repeat
     cx:= hazard(1,8);
     test :=(((cx= 1) and (bh2 or bh9)) or
             ((cx= 2) and bh8) or
             ((cx= 3) and bh4) or
             ((cx= 4) and bf4) or
             ((cx= 5) and bh6) or
             ((cx= 6) and bh1) or
             ((cx= 7) and bf1) or
             ((cx= 8) and bh5))
  until not test;
  conv(cx,per);
  affper(per);
 end;
end;

overlay procedure quelq20(h: integer; var per: integer);
var
 min,max: integer;
begin
(* debug('o1 quelq20'); *)
 if ((h>= 0) and (h< 10)) or ((h> 18) and (h< 24)) then chlm(per)
 else
 begin
  if (h> 9) and (h< 12) then
  begin
   min:= 3;
   max:= 7;
  end;
  if (h> 11) and (h< 18) then
  begin
   min:= 1;
   max:= 2;
  end;
  if h= 18 then
  begin
   min:= 2;
   max:= 4;
  end;
  choix(min,max,per);
 end;
 affper(per);
end;


overlay procedure frap;
var
 j,h,m,haz: integer;
begin
(* debug('o1 frap'); *)
 calch(j,h,m);
 if (h>= 0) and (h< 8) then crep:= 190 else
 begin
  haz:= hazard(1,100);
  if haz> 70 then crep:= 190 else crep:= 147;
 end;
end;

overlay procedure nouvp(l: integer; var p: integer);
begin
(* debug('o1 nouvp'); *)
 p:= 0;
 if l= 1 then
 begin
  if bh1 then p:= 4;
  if bf1 then p:= 2;
 end;
 if ((l= 2) and (bh2)) or ((l= 9) and (bh9)) then p:= 128;
 if l= 4 then
 begin
  if bh4 then p:= 32;
  if bf4 then p:= 16;
 end;
 if (l= 5) and (bh5) then p:= 1;
 if (l= 6) and (bh6) then p:= 8;
 if (l= 8) and (bh8) then p:= 64;
 if ((l= 3) and (bt3)) or ((l= 7) and (bt7)) then p:= 9;
 if p<> 9 then affper(p);
end;



overlay procedure tip(ip: integer; var cx: integer);
begin
(* debug('o1 tip'); *)
 if ip= 128 then cx:= 1 else
 if ip= 64  then cx:= 2 else
 if ip= 32  then cx:= 3 else
 if ip= 16  then cx:= 4 else
 if ip= 8   then cx:= 5 else
 if ip= 4   then cx:= 6 else
 if ip= 2   then cx:= 7 else
 if ip= 1   then cx:= 8;
end;


overlay procedure ecfren(var p,haz: integer; cf,l: integer);
begin
(* debug('o1 ecfren'); *)
 if l= 0 then person;
 p:= -500;
 haz:= 0;
 if ((l= 1) and (not bh1) and (not bf1))
 or ((l= 4) and (not bh4) and (not bf4)) then cpl1(p);
 if (l= 2) and (not bh2) and (not bh9) then cpl2(p);
 if ((l= 3) and (not bt3)) or ((l= 7) and (not bt7)) then cpl3(p);
 if (l= 5) and (not bh5) then cpl5(p);
 if ((l= 6) and (not bh6)) or ((l= 8) and (not bh8)) then cpl6(p);
 if (l= 9) and (not bh9) and (not bh2) then p:= 10;
 if ((l= 2) and (bh9)) or ((l= 9) and (bh2)) then p:= -400;
 if p<> -500 then
 begin
  p:= p+ cf;
  haz:= hazard(1,100);
 end;
end;

overlay procedure becfren(l: integer);
var
 haz: integer;
begin
(* debug('o1 becfren'); *)
 if (l= 1) or (l= 4) then
 begin
  haz:= hazard(1,2);
  if l= 1 then
   if haz= 1 then bh1:= true else bf1:= true;
  if l= 4 then
   if haz= 1 then bh4:= true else bf4:= true;
 end;
 if l= 2 then bh2:= true else
 if l= 3 then bt3:= true else
 if l= 5 then bh5:= true else
 if l= 6 then bh6:= true else
 if l= 7 then bt7:= true else
 if l= 8 then bh8:= true else
 if l= 9 then bh9:= true;
end;

(* NIVEAU 10 *)
procedure init_nbrepm;
  const ipm: array[1..8] of byte
            =(4,5,6,7,5,6,5,8);
   var i : integer;
begin
(* debug('init_nbrepm'); *)
  i:= 0;
  repeat
    i:= succ(i);
    nbrepm[i]:= ipm[i];
  until i=8;
end;

procedure phaz(var haz,p: integer; cf: integer);
begin
(* debug('phaz'); *)
 p:= p+ cf;
 haz:= hazard(1,100);
end;

overlay procedure inzon;
var
 cx: integer;
begin
(* debug('o2 inzon'); *)
 copcha;
 with s do
 begin
  ipre  := false;
  derobj:= 0;
  icave := 0;
  iboul := 0;
  ibag  := 0;
  ipuit := 0;
  ivier := 0;
  iloic := 136;
  icryp := 141;
  conf  := hazard(4,10);
  mlieu := 21;
  for cx:= 2 to 6 do sjer[cx]:= chr(0);
  sjer[1]:= chr(113);
  heure:= chr(20);
  for cx:= 1 to 10 do pourc[cx]:= ' ';
  for cx:= 1 to 6 do teauto[cx]:= '*';
  for cx:= 7 to 9 do teauto[cx]:= ' ';
  for cx:= 10 to 28 do teauto[cx]:= '*';
  for cx:= 29 to 42 do teauto[cx]:= ' ';
  teauto[33]:= '*';
 end;
 for cx:= 1 to 8 do nbrep[cx]:= 0;
 init_nbrepm;
end;

overlay procedure dprog;
begin
(* debug('o2 dprog'); *)
 li:= 21;
 (* jh:= t_settime(0);*)
 jh:= 0;
 if not s.ipre then blo:= true;
 t:= ti1;
 mh:= readclock;
end;

overlay procedure pl1(cf: integer);
var
 p, haz: integer;
begin
(* debug('o2 pl1'); *)
 if ((li= 1) and (not bh1) and (not bf1))
 or ((li= 4) and (not bh4) and (not bf4)) then
 begin
  cpl1(p);
  phaz(haz,p,cf);
  if haz> p then person else quelq1(li);
 end;
end;

overlay procedure pl2(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl2'); *)
 if not bh2 then
 begin
  cpl2(p);
  phaz(haz,p,cf);
  if haz> p then person else quelq2;
 end;
end;

overlay procedure pl5(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl5'); *)
 if not bh5 then
 begin
  cpl5(p);
  phaz(haz,p,cf);
  if haz> p then person else quelq5;
 end;
end;

overlay procedure pl6(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl6'); *)
 if ((li= 6) and (not bh6)) or ((li= 8) and (not bh8)) then
 begin
  cpl6(p);
  phaz(haz,p,cf);
  if haz> p then person else quelq6(li);
 end;
end;

overlay procedure pl9(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl9'); *)
 if not bh9 then
 begin
  cf:= -10;
  phaz(haz,p,cf);
  if haz> p then person else quelq2;
 end;
end;

overlay procedure pl10(cf: integer);
var
 p,h,haz: integer;
begin
(* debug('o2 pl10'); *)
 cpl10(p,h);
 phaz(haz,p,cf);
 if haz> p then person else quelq10(h,p);
end;

overlay procedure pl11(cf: integer);
var
 p,h,haz: integer;
begin
(* debug('o2 pl11'); *)
 cpl11(p,h);
 phaz(haz,p,cf);
 if haz> p then person else quelq11(h,p);
end;

overlay procedure pl12(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl12'); *)
 cpl12(p);
 phaz(haz,p,cf);
 if haz> p then person else quelq12(p);
end;

overlay procedure pl13(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl13'); *)
 cpl13(p);
 phaz(haz,p,cf);
 if haz> p then person else quelq12(p);
end;

overlay procedure pl15(cf: integer);
var
 p,haz: integer;
begin
(* debug('o2 pl15'); *)
 cpl15(p);
 phaz(haz,p,cf);
 if haz> p then person else quelq15(p);
end;

overlay procedure pl20(cf: integer);
var
 p,h,haz: integer;
begin
(* debug('o2 pl20'); *)
 cpl20(p,h);
 phaz(haz,p,cf);
 if haz> p then person else quelq20(h,p);
end;

overlay procedure t11(l11: integer; var a: integer);
var
 p,haz,h: integer;
begin
(* debug('o2 t11'); *)
 ecfren(p,haz,s.conf,l11);
 li:= l11;
 if (l11> 0) and (l11< 10) then
  if (p<> -500) then
  begin
   if (haz> p) then
   begin
    person;
    a:= 0;
   end
   else
   begin
    becfren(li);
    nouvp(li,a);
   end;
  end
  else nouvp(li,a);
 if l11> 9 then
  if (l11> 15) and (l11<> 20) and (l11<> 26) then person else
  begin
   if l11= 10 then cpl10(p,h);
   if l11= 11 then cpl11(p,h);
   if l11= 12 then cpl12(p);
   if (l11= 13) or (l11= 14) then cpl13(p);
   if (l11= 15) or (l11= 26) then cpl15(p);
   if l11= 20 then cpl20(p,h);
   p:= p+ s.conf;
   haz:= hazard(1,100);
   if haz> p then
   begin
    person;
    a:= 0;
   end
   else
   begin
    if l11= 10 then quelq10(h,p);
    if l11= 11 then quelq11(h,p);
    if (l11= 12) or (l11= 13) or (l11= 14) then quelq12(p);
    if (l11= 15) or (l11= 26) then quelq15(p);
    if l11= 20 then quelq20(h,p);
    a:= p;
   end;
  end;
end;

overlay procedure cavegre;

var
 haz: integer;
begin
(* debug('cavegre'); *)
 s.conf:= s.conf+ 2;
 if s.conf> 69 then s.conf:= s.conf+ (s.conf div 10);
 clsf3;
 ecrf2;
 ecr3(m1);
 haz:= (hazard(0, 4))- 2;
 parole(2, haz, 1);
 for haz:=0 to 3000 do;
 clsf3;
 person;
end;

procedure writetp(s:str255;t:integer);
begin
  if res=2 then writeg(s,t)
           else writeg(copy(s,1,25),t);
end;

procedure messint(nu: integer);
begin
(* debug('messint'); *)
 clsf1;
 clsf2;
 clsf3;
 decomp( $73A2, 1008);
 memw[$7413:12]:= 80;
 memw[$7413:14]:=  40;
 writepal(90);
 afff(Gd,$73A2, 1008,0,0);
 afff(Gd,$73A2, 1008,0,70);
 repon(7,nu);
end;

procedure aniof(ouf,num: integer);
     var  ad, offset : integer;
            c:char;
   begin
   (* debug('aniof'); *)
     if (caff= 7) and ((num= 4) or (num= 5)) then exit;
     if (caff= 10) and (num= 7) then num:= 6;
     if (caff= 12) then if (num= 3) then num:= 4
                                    else if (num= 4) then num:= 3;
     ad:= adani;
     offset:= animof(ouf,num);
     decomp(ad,offset);
     hide_mouse;
     afff(Gd,ad,offset,0,12);
     ecrf1;
     show_mouse;
   end;

procedure musique(so: integer);
var
 i: boolean;
 haz: integer;
(* dep: array[1..5] of long_integer;*)
begin
(* debug('musique'); *)
  if so= 0 then (*musik(0)*) begin end
   else
  if (prebru= 0) and (not s.ipre) then
  begin
   parole(10, 1, 1);
   prebru:= prebru+ 1;
  end
  else
  begin
   i:= false;
   if (s.mlieu= 19) or (s.mlieu= 21) or (s.mlieu= 22) then
   begin
    haz:= hazard(1,3);
    if haz= 2 then
    begin
     haz:= hazard(2,4);
     parole(9,haz,1);
     i:= true;
    end;
   end;
   if s.mlieu= 20 then
   begin
    haz:= hazard(1,2);
    if haz= 1 then
    begin
     parole(8,1,1);
     i:= true;
    end;
   end;
   if s.mlieu= 24 then
   begin
    haz:= hazard(1,2);
    if haz= 2 then
    begin
     parole(12,1,1);
     i:= true;
    end;
   end;
   if s.mlieu= 23 then
   begin
    parole(13,1,1);
    i:= true;
   end;
   if not i then
   begin
   (* dep[1]:= 1416;
    dep[2]:= 1512;
    dep[3]:= 1692;
    dep[4]:= 1884;
    dep[5]:= 2046;
    haz:= hazard(1,5);
    musik(dep[haz]);*)
    haz:= hazard(1,17);
    parole(haz,1,2);
   end;
  end;
end;

(* NIVEAU 9 *)
procedure dessin(ad:integer);
var
 cx: integer;
begin
(* debug('dessin'); *)
 if ad<> 0 then dessine(ades,((ad mod 160)* 2),(ad div 160))
           else
 begin
  clsf1;
  if caff> 99 then
  begin
    dessine(ades,60,33);
    boite(118, 32, 291, 121, 15);         (* cadre moyen *)
  end
  else
   if caff> 69 then
   begin
    dessine(ades,112,48);             (* tˆtes *)
    boite(222, 47, 155, 91, 15);
   end
   else
   begin
    dessine(ades,0,12);
    ecrf1;
    if (caff< 30) or (caff> 32) then
    begin
     for cx:= 1 to 6 do
      if ord(touv[cx])<> 0 then aniof(1,ord(touv[cx]));
     if caff= 13 then
     begin
      if s.iboul= 141 then aniof(1,7);
      if s.ibag= 159 then aniof(1,6);
     end;
     if (caff= 14) and (s.icave= 151) then aniof(1,2);
     if (caff= 17) and (s.ivier= 143) then aniof(1,1);
     if (caff= 24) and (s.ipuit<> 0) then aniof(1,1);
    end;
    if caff< 26 then musique(1);
   end;
 end;
end;



