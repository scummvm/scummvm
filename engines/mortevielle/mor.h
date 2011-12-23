/* Niveau 14 suite */
const char m1[] = "quelqu'un entre, parait ‚tonn‚ mais ne dit rien";


void testfi()
   {
     if (ioresult!= 0) 
        {
          caff= do_alert(err_mess,1);
          exit(0);
        }
   }


integer readclock()
{
               registres r;
         integer s, m, h;

/* debug('readclock');*/
  integer readclock_result;
  r.ax = 0x2c00;
  intr(0x21,r);
  {
    h=(cardinal)r.cx >> 8;
    m=r.cx % 256;
    s=(cardinal)r.dx >> 8;
  }
  m= m * 60;
  h= h* 3600;
  readclock_result= h+ m+ s;
  return readclock_result;
}

void modif(integer& nu)
{
 /* debug('modif'); */
 if (nu== 26)  nu= 25;
 if ((nu> 29) && (nu< 36))  nu= nu- 4;
 if ((nu> 69) && (nu< 78))  nu= nu- 37;
 if ((nu> 99) && (nu< 194))  nu= nu- 59;
 if ((nu> 996) && (nu< 1000))  nu= nu- 862;
 if ((nu> 1500) && (nu< 1507))  nu= nu- 1363;
 if ((nu> 1507) && (nu< 1513))  nu= nu- 1364;
 if ((nu> 1999) && (nu< 2002))  nu= nu- 1851;
 if (nu== 2010)  nu= 151;
 if ((nu> 2011) && (nu< 2025))  nu= nu- 1860;
 if (nu== 2026)  nu= 165;
 if ((nu> 2029) && (nu< 2037))  nu= nu- 1864;
 if ((nu> 3000) && (nu< 3005))  nu= nu- 2828;
 if (nu== 4100)  nu= 177;
 if (nu== 4150)  nu= 178;
 if ((nu> 4151) && (nu< 4156))  nu= nu- 3973;
 if (nu== 4157)  nu= 183;
 if ((nu== 4160) || (nu== 4161))  nu= nu- 3976;
}


void dessine(integer ad, integer x, integer y)
{
  /* debug('dessine'); */
  hide_mouse();
  writepal(numpal);
  pictout(ad,0,x,y);
  show_mouse();
}

void dessine_rouleau()
{
  /* debug('dessine_rouleau'); */
  writepal(89);
  if (gd==her) 
     {
       mem[0x7000+14]=15;
     }
  hide_mouse();
  pictout(0x73a2,0,0,0);
  show_mouse();
}


void text_color(integer c)
{
  color_txt= c;
}

/* NIVEAU 13 */


void text1(integer x,integer y,integer nb,integer m)
{
       phrase st;
       integer tay;
       integer co;


  /* debug('text'); */
  if (res==1)  co=10; else co=6;
  deline(m,st,tay);
  if ((y==182) && (tay*co>nb*6))  y=176;
  afftex(st,x,y,nb,20,color_txt);
}

void initouv()
 {
      integer cx;

   /* debug('initouv'); */
   for( cx= 1; cx <= 7; cx ++) touv[cx]= chr(0);
 }

void ecrf1()
{
  boite(0, 11, 512, 163, 15);           /* grand dessin */
}

void clsf1()
{
      integer i,j;

 /* debug('clsf1'); */
 hide_mouse();
 box(0,gd, 0,11, 514,175, 255);
/* if Gd=ams then port[$3DD]:=15;
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
 droite(256*res,11,256*res,175,0);*/
 show_mouse();
}

void clsf2()
{
       integer i,j;

 /* debug('clsf2'); */
 hide_mouse();
 if (f2_all) 
    {
      box(0,gd,1,176,633,199,255);
      boite(0,175,634,24,15);
      f2_all=false;
    }
  else
    {
      box(0,gd,1,176,633,190,255);
      boite(0,175,634,15,15);
    }
 show_mouse();
}

void ecrf2()
{
  /* debug('ecrf2'); */
  text_color(5);
  /*boite(0,175,630,15,15);*/
}

void ecr2(str255 str_)
{
      integer tab;
      integer tlig;

 /* debug('ecr2 : '+str_);*/
 if (res==1)  tab=10; else tab=6;
 putxy(8,177);
 tlig=59+pred(integer,res)*36;
 if (length(str_)<tlig)  writeg(str_,5);
  else if (length(str_)<(tlig << 1)) 
          {
            putxy(8,176);
            writeg(copy(str_,1,pred(integer,tlig)),5);
            putxy(8,182);
            writeg(copy(str_,tlig,tlig << 1),5);
          }
   else {
          f2_all=true;
          clsf2();
          putxy(8,176);
          writeg(copy(str_,1,pred(integer,tlig)),5);
          putxy(8,182);
          writeg(copy(str_,tlig,pred(integer,tlig << 1)),5);
          putxy(8,190);
          writeg(copy(str_,tlig << 1,tlig*3),5);
        }
}

void clsf3()
{
       integer i,j;

  /* debug('clsf3'); */
  hide_mouse();
  box(0,gd,1,192,633,199,255);
  boite(0,191,634,8,15);
  show_mouse();
}

void ecr3(str255 text)
{
 /* debug('ecr3 : '+text);*/
 clsf3();
 putxy(8,192);
 writeg(text,5);
}

void ecrf6()
{
  /* debug('ecrf6'); */
  text_color(5);
  boite(62, 33, 363, 80, 15);
}

void ecrf7()
{
 text_color(4);
/* draw_mode(2);
 text_height(4*res);
 text_style(normal);*/
}

void clsf10()
{
      integer co,cod;
      varying_string<8> st;

 /* debug('clsf10'); */
 hide_mouse();
 if (res==1) 
    {
      co=634;
      cod=534;
    }
  else
    {
      co=600;
      cod=544;
    }
 box(15,gd, cod, 93, co, 98, 255);
 if (s.conf< 33)  st="Cool";
               else
 if (s.conf< 66)  st=" Lourde ";
               else
 if (s.conf> 65)  st="Malsaine";
 if (res==1)  co=10; else co=6;
 co= 574-((cardinal)co*length(st) >> 1);
 putxy(co,92);
 writeg(st,4);
 if (res==1)  co=620; else co=584;
 box(15,gd, 560, 24, co, 86, 255);
/* rempli(69,12,32,5,255);*/
 show_mouse();
}

   void stop()
     {
       clrscr;
       hirs();
       gd=ams;
       hirs();
       exit(0);
     }

void paint_rect(integer x, integer y, integer dx, integer dy)
   {
         integer co;

     /* debug('paint_rect'); */
     if (gd==cga)  co= 3;
               else co=11;
     box(co,gd,x,y,x+dx,y+dy,255);
   }

integer hazard(integer min,integer max)
   {
         integer ha1;
         real ha2;

     /* debug('hazard'); */
     integer hazard_result;
     ha2= Random();
     ha1= trunc(ha2*(max-min));
     hazard_result= min+ ha1;
     return hazard_result;
   }

void calch(integer& j,integer& h,integer& m)
{
 integer th,nh;

/* debug('calch');*/
 nh= readclock();
 th= jh+ ((nh- mh) / t);
 m= ((th % 2)+ vm)* 30;
 h= ((cardinal)th >> 1)+ vh;
 if (m== 60) 
 {
  m= 0;
  h= h+ 1;
 }
 j= (h / 24)+ vj;
 h= h- ((j- vj)* 24);
}

void conv(integer x, integer& y)
{
 integer cx;

 /* debug('conv'); */
 cx= 1;
 y= 128;
 while (cx< x) 
 {
  y= (cardinal)y >> 1;
  cx= succ(integer,cx);
 }
}

/* NIVEAU 12 */
void okpas()
{
  tesok=true;
}

void modobj(integer m)
{
 phrase str_;
 str30 strp;
 integer i,tay;

 /* debug('modobj'); */
 strp=' ';
 if (m!= 500) 
    {
      deline(m- 501+ c_st41,str_,tay);
      strp= delig;
    }
 menut( invt[8], strp);
 menu_disable( invt[8]);
}

void modobj2(integer m,boolean t1,boolean t2)
{
 phrase str_;
 str30 strp;
 integer i,tay;

 /* debug('modobj'); */
  strp=' ';
 if (t1 || t2)  okpas();
             else tesok=false;;
 if (m!= 500) 
    {
      deline(m- 501+ c_st41,str_,tay);
      strp= delig;
    }
 menut( invt[8], strp);
 menu_disable( invt[8]);
}


void repon(integer f,integer m)
{
 str255 str_;
 varying_string<40> str1;
 phrase st;
 text1 fic;
 integer i, xco, dx, caspe, tay;

 /* debug('repon fenetre nø'+chr(f+48));*/
 if ((m> 499) && (m< 563)) 
 {
  deline( m-501+c_st41,st,tay);
  if (tay>((58+pred(integer,res)*37) << 1))  f2_all= true;
                                 else f2_all= false;
  clsf2();
  afftex(st,8,176,85,3,5);
 }
 else
 {
  modif(m);
  if (f== 8)  f= 2;
  if (f== 1)  f= 6;
  if (f== 2) 
  {
   clsf2();
   ecrf2();
   text1(8, 182, 103, m);
   if ((m== 68) || (m== 69))  s.teauto[40]= '*';
   if ((m== 104) && (caff== 14)) 
   {
    s.teauto[36]= '*';
    if (s.teauto[39]== '*') 
    {
     s.pourc[3]= '*';
     s.teauto[38]= '*';
    }
   }
  }
  if (set::of(6,9, eos).has(f)) 
  {
   deline(m,st,tay);
   if (f==6)  i=4;
          else i=5;
   afftex(st,80,40,60,25,i);
   if (m== 180)  s.pourc[6]= '*';
   if (m== 179)  s.pourc[10]= '*';
  }
  if (f== 7)            /* messint */
  {
   ecrf7();
   deline(m,st,tay);
   if (res==1) 
      {
        xco=252-tay*5;
        caspe= 100;
        dx= 80;
      }
    else
      {
        xco=252-tay*3;
        caspe= 144;
        dx=50;
      }
   if (tay<40)  afftex(st,xco,86,dx,3,5);
             else afftex(st,caspe,86,dx,3,5);
  }
 }
}

void f3f8()
{
      integer co;

  if (res==1)  co=107; else co=64;
  putxy(3,44);
  writeg(f3,5);
  ywhere= 51;
  writeg(f8,5);
  boite(0,42,co,16,7);
}

void t5(integer cx)
{
 /* debug('t5'); */
 if (cx== 10)  blo= false;
 if (cx!= 1) 
 {
  bh1= false;
  bf1= false;
 }
 if (cx!= 2)  bh2= false;
 if (cx!= 4) 
 {
  bh4= false;
  bf4= false;
 }
 if (cx!= 5)  bh5= false;
 if (cx!= 6)  bh6= false;
 if (cx!= 8)  bh8= false;
 if (cx!= 3)  bt3= false;
 if (cx!= 7)  bt7= false;
 if (cx!= 9)  bh9= false;
}

void affper(integer per)
{
 integer cx;

 /* debug('affper'); */
 for( cx= 1; cx <= 8; cx ++) menu_disable( disc[cx]);
 clsf10();
 if ((per & 128)== 128) 
 {
   putxy(560,24);
   writeg("LEO",4);
   menu_enable( disc[1]);
 }
 if ((per & 64)== 64) 
 {
  putxy(560, 32);
  writeg("PAT",4);
  menu_enable( disc[2]);
 }
 if ((per & 32)== 32) 
 {
  putxy(560, 40);
  writeg("GUY",4);
  menu_enable( disc[3]);
 }
 if ((per & 16)== 16) 
 {
  putxy(560,48);
  writeg("EVA",4);
  menu_enable( disc[4]);
 }
 if ((per & 8)== 8) 
 {
  putxy(560,56);
  writeg("BOB",4);
  menu_enable( disc[5]);
 }
 if ((per & 4)== 4) 
 {
  putxy(560, 64);
  writeg("LUC",4);
  menu_enable( disc[6]);
 }
 if ((per & 2)== 2) 
 {
  putxy(560, 72);
  writeg("IDA",4);
  menu_enable( disc[7]);
 }
 if ((per & 1)== 1) 
 {
  putxy(560, 80);
  writeg("MAX",4);
  menu_enable( disc[8]);
 }
 ipers= per;
}

/* overlay */ void choix(integer min,integer max, integer& per)
{
 boolean i;
 integer haz,cx,cy,cz;
 long_integer per2,cz2;

 /* debug('o0 choix'); */
 haz= hazard(min,max);
 if (haz> 4) 
 {
  haz= 8- haz;
  i= true;
 }
 else i= false;
 cx= 0;
 per= 0;
 while (cx< haz) 
 {
  cy= hazard(1,8);
  conv(cy,cz);
  if ((per & cz)!= cz) 
  {
   cx= cx+ 1;
   per= (per | cz);
  }
 }
 if (i)  per= 255- per;
 i= false;
}

/* overlay */ void cpl1(integer& p)
{
 integer j,h,m;

 /* debug('o0 cpl1'); */
 calch(j,h,m);
 if ((h> 7) || (h< 11))  p= 25;
 if ((h> 10) && (h< 14))  p= 35;
 if ((h> 13) && (h< 16))  p= 50;
 if ((h> 15) && (h< 18))  p= 5;
 if ((h> 17) && (h< 22))  p= 35;
 if ((h> 21) && (h< 24))  p= 50;
 if ((h>= 0) && (h< 8))  p= 70;
 mdn();
}

/* overlay */ void cpl2(integer& p)
{
 integer j,h,m;

 /* debug('o0 cpl2'); */
 calch(j,h,m);
 if ((h> 7) && (h< 11))  p= -2;
 if (h== 11)  p= 100;
 if ((h> 11) && (h< 23))  p= 10;
 if (h== 23)  p= 20;
 if ((h>= 0) && (h< 8))  p= 50;
}

/* overlay */ void cpl3(integer& p)
{
 integer j,h,m;

 /* debug('o0 cpl3'); */
 calch(j,h,m);
 if (((h> 8) && (h< 10)) || ((h> 19) && (h< 24)))  p= 34;
 if (((h> 9) && (h< 20)) || ((h>= 0) && (h< 9)))  p= 0;
}

/* overlay */ void cpl5(integer& p)
{
 integer j,h,m;

 /* debug('o0 cpl5'); */
 calch(j,h,m);
 if ((h> 6) && (h< 10))  p= 0;
 if (h== 10)  p= 100;
 if ((h> 10) && (h< 24))  p= 15;
 if ((h>= 0) && (h< 7))  p= 50;
}

/* overlay */ void cpl6(integer& p)
{
 integer j,h,m;

 /* debug('o0 cpl6'); */
 calch(j,h,m);
 if (((h> 7) && (h< 13)) || ((h> 17) && (h< 20)))  p= -2;
 if (((h> 12) && (h< 17)) || ((h> 19) && (h< 24)))  p= 35;
 if (h== 17)  p= 100;
 if ((h>= 0) && (h< 8))  p= 60;
}

void person()
{
 const char m1[] = "Vous";
 const char m2[] = "ˆtes";
 const char m3[] = "SEUL";
 integer cf;
 str255 str_;

 /* debug('person'); */
 for( cf= 1; cf <= 8; cf ++) menu_disable( disc[cf]);
 clsf10();
 putxy(560, 30);
 writeg(m1,4);
 putxy(560, 50);
 writeg(m2,4);
 putxy(560, 70);
 writeg(m3,4);
 ipers= 0;
}

void chlm(integer& per)
{
  /* debug('chlm'); */
  per= hazard(1,2);
  if (per== 2)  per= 128;
}

void pendule()
{
 const matrix<1,2,1,12,integer> cv
    = {{{{ 5,  8, 10,  8,  5,  0, -5,  -8, -10,  -8,  -5,   0}},
      {{-5, -3,  0,  3,  5,  6,  5,   3,   0,  -3,  -5,  -6}} }};
 const integer x = 580;
 const integer y = 123;
 const integer rg = 9;
 integer h,co;


 /* debug('pendule'); */
 hide_mouse();
 /*paint_rect(572,114,18,20);
 paint_rect(577,114,6,20);*/
 paint_rect(570,118,20,10);
 paint_rect(578,114,6,18);
 if (set::of(cga,her, eos).has(gd))  co=0; else co=1;
 if (min== 0)  droite(((cardinal)x >> 1)*res, y, ((cardinal)x >> 1)*res, (y- rg), co);
           else droite(((cardinal)x >> 1)*res, y, ((cardinal)x >> 1)*res, (y+ rg), co);
 h= heu;
 if (h> 12)  h= h- 12;
 if (h== 0)  h= 12;
 droite(((cardinal)x >> 1)*res, y, ((cardinal)(x+ cv[1][h]) >> 1)*res, y+ cv[2][h], co);
 show_mouse();
 putxy( 568, 154);
 if (heu> 11)  writeg( "PM ",1);
            else writeg( "AM ",1);
 putxy( 550, 160);
 if (set::of(range(0,8), eos).has(jou))  writeg(string(chr(jou+49))+"& jour",1);
}

/*************
 * NIVEAU 11 *
 *************/

void debloc(integer l)
{
/* debug('debloc'); */
 num= 0;
 x= 0;
 y= 0;
 if ((l!= 26) && (l!= 15))  t5(l);
 mpers= ipers;
}

/* overlay */ void cpl10(integer& p,integer& h)
{
 integer j,m;

/* debug('o1 cpl10'); */
 calch(j,h,m);
 if (((h> 7) && (h< 11)) || ((h> 11) && (h< 14))
  || ((h> 18) && (h< 21)))  p= 100;
 if ((h== 11) || ((h> 20) && (h< 24)))  p= 45;
 if (((h> 13) && (h< 17)) || (h== 18))  p= 35;
 if (h== 17)  p= 60;
 if ((h>= 0) && (h< 8))  p= 5;
}

/* overlay */ void cpl11(integer& p,integer& h)
{
 integer j,m;

/* debug('o1 cpl11'); */
 calch(j,h,m);
 if (((h> 8) && (h< 12)) || ((h> 20) && (h< 24)))  p= 25;
 if (((h> 11) && (h< 14)) || ((h> 18) && (h< 21)))  p= 5;
 if ((h> 13) && (h< 17))  p= 55;
 if ((h> 16) && (h< 19))  p= 45;
 if ((h>= 0) && (h< 9))  p= 0;
}

/* overlay */ void cpl12(integer& p)
{
 integer j,h,m;

/* debug('o1 cpl12'); */
 calch(j,h,m);
 if (((h> 8) && (h< 15)) || ((h> 16) && (h< 22)))  p= 55;
 if (((h> 14) && (h< 17)) || ((h> 21) && (h< 24)))  p= 25;
 if ((h>= 0) && (h< 5))  p= 0;
 if ((h> 4) && (h< 9))  p= 15;
}

/* overlay */ void cpl13(integer& p)
{
/* debug('o1 cpl13'); */
 p= 0;
}

/* overlay */ void cpl15(integer& p)
{
 integer j,h,m;

/* debug('o1 cpl15'); */
 calch(j,h,m);
 if ((h> 7) && (h< 12))  p= 25;
 if ((h> 11) && (h< 14))  p= 0;
 if ((h> 13) && (h< 18))  p= 10;
 if ((h> 17) && (h< 20))  p= 55;
 if ((h> 19) && (h< 22))  p= 5;
 if ((h> 21) && (h< 24))  p= 15;
 if ((h>= 0) && (h< 8))  p= -15;
}

/* overlay */ void cpl20(integer& p,integer& h)
{
 integer j,m;

/* debug('o1 cpl20'); */
 calch(j,h,m);
 if (h== 10)  p= 65;
 if ((h> 10) && (h< 21))  p= 5;
 if ((h> 20) && (h< 24))  p= -15;
 if ((h>= 0) && (h< 5))  p= -300;
 if ((h> 4) && (h< 10))  p= -5;
}

/* overlay */ void quelq1(integer l)
{
 integer per;

/* debug('o1 quelq1'); */
 per= hazard(1,2);
 if (l== 1) 
  if (per== 1)  bh1= true; else bf1= true;
 if (l== 4) 
  if (per== 1)  bh4= true; else bf4= true;
 ipers= 10;
}

/* overlay */ void quelq2()
{
/* debug('o1 quelq2'); */
 if (li== 2)  bh2= true; else bh9= true;
 ipers= 10;
}

/* overlay */ void quelq5()
{
/* debug('o1 quelq5'); */
 bh5= true;
 ipers= 10;
}

/* overlay */ void quelq6(integer l)
{
 integer per;

/* debug('o1 quelq6'); */
 if (l== 6)  bh6= true;
 if (l== 8)  bh8= true;
 ipers= 10;
}

/* overlay */ void quelq10(integer h, integer& per)
{
 integer min,max;

/* debug('o1 quelq10'); */
 if ((h>= 0) && (h< 8))  chlm(per); else
 {
  if ((h> 7) && (h< 10)) 
  {
   min= 5;
   max= 7;
  }
  if ((h> 9) && (h< 12)) 
  {
   min= 1;
   max= 4;
  }
  if (((h> 11) && (h< 15)) || ((h> 18) && (h< 21))) 
  {
   min= 6;
   max= 8;
  }
  if (((h> 14) && (h< 19)) || ((h> 20) && (h< 24))) 
  {
   min= 1;
   max= 5;
  }
  choix(min,max,per);
 }
 affper(per);
}

/* overlay */ void quelq11(integer h, integer& per)
{
 integer min,max;

/* debug('o1 quelq11'); */
 if ((h>= 0) && (h< 8))  chlm(per); else
 {
  if (((h> 7) && (h< 10)) || ((h>20) && (h< 24))) 
  {
   min= 1;
   max= 3;
  }
  if (((h> 9) && (h< 12)) || ((h> 13) && (h< 19))) 
  {
   min= 1;
   max= 4;
  }
  if (((h> 11) && (h< 14)) || ((h> 18) && (h< 21)))
  {
   min= 1;
   max= 2;
  }
  choix(min,max,per);
 }
 affper(per);
}

/* overlay */ void quelq12(integer& per)
{
/* debug('o1 quelq12'); */
 chlm(per);
 affper(per);
}

/* overlay */ void quelq15(integer& per)
{
 integer cx;
 boolean test;


/* debug('o1 quelq15'); */
 per= 0;
 if (per== 0) 
 {
   do {
     cx= hazard(1,8);
     test =(((cx== 1) && (bh2 || bh9)) ||
             ((cx== 2) && bh8) ||
             ((cx== 3) && bh4) ||
             ((cx== 4) && bf4) ||
             ((cx== 5) && bh6) ||
             ((cx== 6) && bh1) ||
             ((cx== 7) && bf1) ||
             ((cx== 8) && bh5));
  } while (!(! test));
  conv(cx,per);
  affper(per);
 }
}

/* overlay */ void quelq20(integer h, integer& per)
{
 integer min,max;

/* debug('o1 quelq20'); */
 if (((h>= 0) && (h< 10)) || ((h> 18) && (h< 24)))  chlm(per);
 else
 {
  if ((h> 9) && (h< 12)) 
  {
   min= 3;
   max= 7;
  }
  if ((h> 11) && (h< 18)) 
  {
   min= 1;
   max= 2;
  }
  if (h== 18) 
  {
   min= 2;
   max= 4;
  }
  choix(min,max,per);
 }
 affper(per);
}


/* overlay */void frap()
{
 integer j,h,m,haz;

/* debug('o1 frap'); */
 calch(j,h,m);
 if ((h>= 0) && (h< 8))  crep= 190; else
 {
  haz= hazard(1,100);
  if (haz> 70)  crep= 190; else crep= 147;
 }
}

/* overlay */ void nouvp(integer l, integer& p)
{
/* debug('o1 nouvp'); */
 p= 0;
 if (l== 1) 
 {
  if (bh1)  p= 4;
  if (bf1)  p= 2;
 }
 if (((l== 2) && (bh2)) || ((l== 9) && (bh9)))  p= 128;
 if (l== 4) 
 {
  if (bh4)  p= 32;
  if (bf4)  p= 16;
 }
 if ((l== 5) && (bh5))  p= 1;
 if ((l== 6) && (bh6))  p= 8;
 if ((l== 8) && (bh8))  p= 64;
 if (((l== 3) && (bt3)) || ((l== 7) && (bt7)))  p= 9;
 if (p!= 9)  affper(p);
}



/* overlay */ void tip(integer ip, integer& cx)
{
/* debug('o1 tip'); */
 if (ip== 128)  cx= 1; else
 if (ip== 64)   cx= 2; else
 if (ip== 32)   cx= 3; else
 if (ip== 16)   cx= 4; else
 if (ip== 8)    cx= 5; else
 if (ip== 4)    cx= 6; else
 if (ip== 2)    cx= 7; else
 if (ip== 1)    cx= 8;
}


/* overlay */ void ecfren(integer& p,integer& haz, integer cf,integer l)
{
/* debug('o1 ecfren'); */
 if (l== 0)  person();
 p= -500;
 haz= 0;
 if (((l== 1) && (! bh1) && (! bf1))
 || ((l== 4) && (! bh4) && (! bf4)))  cpl1(p);
 if ((l== 2) && (! bh2) && (! bh9))  cpl2(p);
 if (((l== 3) && (! bt3)) || ((l== 7) && (! bt7)))  cpl3(p);
 if ((l== 5) && (! bh5))  cpl5(p);
 if (((l== 6) && (! bh6)) || ((l== 8) && (! bh8)))  cpl6(p);
 if ((l== 9) && (! bh9) && (! bh2))  p= 10;
 if (((l== 2) && (bh9)) || ((l== 9) && (bh2)))  p= -400;
 if (p!= -500) 
 {
  p= p+ cf;
  haz= hazard(1,100);
 }
}

/* overlay */ void becfren(integer l)
{
 integer haz;

/* debug('o1 becfren'); */
 if ((l== 1) || (l== 4)) 
 {
  haz= hazard(1,2);
  if (l== 1) 
   if (haz== 1)  bh1= true; else bf1= true;
  if (l== 4) 
   if (haz== 1)  bh4= true; else bf4= true;
 }
 if (l== 2)  bh2= true; else
 if (l== 3)  bt3= true; else
 if (l== 5)  bh5= true; else
 if (l== 6)  bh6= true; else
 if (l== 7)  bt7= true; else
 if (l== 8)  bh8= true; else
 if (l== 9)  bh9= true;
}

/* NIVEAU 10 */
void init_nbrepm()
{
        const array<1,8,byte> ipm
            = {{4,5,6,7,5,6,5,8}};
       integer i;

/* debug('init_nbrepm'); */
  i= 0;
  do {
    i= succ(integer,i);
    nbrepm[i]= ipm[i];
  } while (!(i==8));
}

void phaz(integer& haz,integer& p, integer cf)
{
/* debug('phaz'); */
 p= p+ cf;
 haz= hazard(1,100);
}

/* overlay */ void inzon()
{
 integer cx;

/* debug('o2 inzon'); */
 copcha();
 {
  s.ipre  = false;
  s.derobj= 0;
  s.icave = 0;
  s.iboul = 0;
  s.ibag  = 0;
  s.ipuit = 0;
  s.ivier = 0;
  s.iloic = 136;
  s.icryp = 141;
  s.conf  = hazard(4,10);
  s.mlieu = 21;
  for( cx= 2; cx <= 6; cx ++) s.sjer[cx]= chr(0);
  s.sjer[1]= chr(113);
  s.heure= chr(20);
  for( cx= 1; cx <= 10; cx ++) s.pourc[cx]= ' ';
  for( cx= 1; cx <= 6; cx ++) s.teauto[cx]= '*';
  for( cx= 7; cx <= 9; cx ++) s.teauto[cx]= ' ';
  for( cx= 10; cx <= 28; cx ++) s.teauto[cx]= '*';
  for( cx= 29; cx <= 42; cx ++) s.teauto[cx]= ' ';
  s.teauto[33]= '*';
 }
 for( cx= 1; cx <= 8; cx ++) nbrep[cx]= 0;
 init_nbrepm();
}

/* overlay */ void dprog()
{
/* debug('o2 dprog'); */
 li= 21;
 /* jh:= t_settime(0);*/
 jh= 0;
 if (! s.ipre)  blo= true;
 t= ti1;
 mh= readclock();
}

/* overlay */ void pl1(integer cf)
{
 integer p, haz;

/* debug('o2 pl1'); */
 if (((li== 1) && (! bh1) && (! bf1))
 || ((li== 4) && (! bh4) && (! bf4))) 
 {
  cpl1(p);
  phaz(haz,p,cf);
  if (haz> p)  person(); else quelq1(li);
 }
}

/* overlay */ void pl2(integer cf)
{
 integer p,haz;

/* debug('o2 pl2'); */
 if (! bh2) 
 {
  cpl2(p);
  phaz(haz,p,cf);
  if (haz> p)  person(); else quelq2();
 }
}

/* overlay */ void pl5(integer cf)
{
 integer p,haz;

/* debug('o2 pl5'); */
 if (! bh5) 
 {
  cpl5(p);
  phaz(haz,p,cf);
  if (haz> p)  person(); else quelq5();
 }
}

/* overlay */ void pl6(integer cf)
{
 integer p,haz;

/* debug('o2 pl6'); */
 if (((li== 6) && (! bh6)) || ((li== 8) && (! bh8))) 
 {
  cpl6(p);
  phaz(haz,p,cf);
  if (haz> p)  person(); else quelq6(li);
 }
}

/* overlayi */ void pl9(integer cf)
{
 integer p,haz;

/* debug('o2 pl9'); */
 if (! bh9) 
 {
  cf= -10;
  phaz(haz,p,cf);
  if (haz> p)  person(); else quelq2();
 }
}

/* overlayi */ void pl10(integer cf)
{
 integer p,h,haz;

/* debug('o2 pl10'); */
 cpl10(p,h);
 phaz(haz,p,cf);
 if (haz> p)  person(); else quelq10(h,p);
}

/* overlay */ void pl11(integer cf)
{
 integer p,h,haz;

/* debug('o2 pl11'); */
 cpl11(p,h);
 phaz(haz,p,cf);
 if (haz> p)  person(); else quelq11(h,p);
}

/* overlay */ void pl12(integer cf)
{
 integer p,haz;

/* debug('o2 pl12'); */
 cpl12(p);
 phaz(haz,p,cf);
 if (haz> p)  person(); else quelq12(p);
}

/* overlay */ void pl13(integer cf)
{
 integer p,haz;

/* debug('o2 pl13'); */
 cpl13(p);
 phaz(haz,p,cf);
 if (haz> p)  person(); else quelq12(p);
}

/* overlay */ void pl15(integer cf)
{
 integer p,haz;

/* debug('o2 pl15'); */
 cpl15(p);
 phaz(haz,p,cf);
 if (haz> p)  person(); else quelq15(p);
}

/* overlay */ void pl20(integer cf)
{
 integer p,h,haz;

/* debug('o2 pl20'); */
 cpl20(p,h);
 phaz(haz,p,cf);
 if (haz> p)  person(); else quelq20(h,p);
}

/* overlay */ void t11(integer l11, integer& a)
{
 integer p,haz,h;

/* debug('o2 t11'); */
 ecfren(p,haz,s.conf,l11);
 li= l11;
 if ((l11> 0) && (l11< 10)) 
  if (p!= -500) 
  {
   if (haz> p) 
   {
    person();
    a= 0;
   }
   else
   {
    becfren(li);
    nouvp(li,a);
   }
  }
  else nouvp(li,a);
 if (l11> 9) 
  if ((l11> 15) && (l11!= 20) && (l11!= 26))  person(); else
  {
   if (l11== 10)  cpl10(p,h);
   if (l11== 11)  cpl11(p,h);
   if (l11== 12)  cpl12(p);
   if ((l11== 13) || (l11== 14))  cpl13(p);
   if ((l11== 15) || (l11== 26))  cpl15(p);
   if (l11== 20)  cpl20(p,h);
   p= p+ s.conf;
   haz= hazard(1,100);
   if (haz> p) 
   {
    person();
    a= 0;
   }
   else
   {
    if (l11== 10)  quelq10(h,p);
    if (l11== 11)  quelq11(h,p);
    if ((l11== 12) || (l11== 13) || (l11== 14))  quelq12(p);
    if ((l11== 15) || (l11== 26))  quelq15(p);
    if (l11== 20)  quelq20(h,p);
    a= p;
   }
  }
}

/* overlay */ void cavegre()

{
 integer haz;

/* debug('cavegre'); */
 s.conf= s.conf+ 2;
 if (s.conf> 69)  s.conf= s.conf+ (s.conf / 10);
 clsf3();
 ecrf2();
 ecr3(m1);
 haz= (hazard(0, 4))- 2;
 parole(2, haz, 1);
 for( haz=0; haz <= 3000; haz ++);
 clsf3();
 person();
}

void writetp(str255 s,integer t)
{
  if (res==2)  writeg(s,t);
           else writeg(copy(s,1,25),t);
}

void messint(integer nu)
{
/* debug('messint'); */
 clsf1();
 clsf2();
 clsf3();
 decomp( 0x73a2, 1008);
 memw[0x7413+12]= 80;
 memw[0x7413+14]=  40;
 writepal(90);
 afff(gd,0x73a2, 1008,0,0);
 afff(gd,0x73a2, 1008,0,70);
 repon(7,nu);
}

void aniof(integer ouf,integer num)
   {
          integer ad, offset;
            char c;

   /* debug('aniof'); */
     if ((caff== 7) && ((num== 4) || (num== 5)))  return;
     if ((caff== 10) && (num== 7))  num= 6;
     if (caff== 12)  if (num== 3)  num= 4;
                                    else if (num== 4)  num= 3;
     ad= adani;
     offset= animof(ouf,num);
     decomp(ad,offset);
     hide_mouse();
     afff(gd,ad,offset,0,12);
     ecrf1();
     show_mouse();
   }

void musique(integer so)
{
 boolean i;
 integer haz;
/* dep: array[1..5] of long_integer;*/

/* debug('musique'); */
  if (so== 0)   /*musik(0)*/ {; }
   else
  if ((prebru== 0) && (! s.ipre)) 
  {
   parole(10, 1, 1);
   prebru= prebru+ 1;
  }
  else
  {
   i= false;
   if ((s.mlieu== 19) || (s.mlieu== 21) || (s.mlieu== 22)) 
   {
    haz= hazard(1,3);
    if (haz== 2) 
    {
     haz= hazard(2,4);
     parole(9,haz,1);
     i= true;
    }
   }
   if (s.mlieu== 20) 
   {
    haz= hazard(1,2);
    if (haz== 1) 
    {
     parole(8,1,1);
     i= true;
    }
   }
   if (s.mlieu== 24) 
   {
    haz= hazard(1,2);
    if (haz== 2) 
    {
     parole(12,1,1);
     i= true;
    }
   }
   if (s.mlieu== 23) 
   {
    parole(13,1,1);
    i= true;
   }
   if (! i) 
   {
   /* dep[1]:= 1416;
    dep[2]:= 1512;
    dep[3]:= 1692;
    dep[4]:= 1884;
    dep[5]:= 2046;
    haz:= hazard(1,5);
    musik(dep[haz]);*/
    haz= hazard(1,17);
    parole(haz,1,2);
   }
  }
}

/* NIVEAU 9 */
void dessin(integer ad)
{
 integer cx;

/* debug('dessin'); */
 if (ad!= 0)  dessine(ades,((ad % 160)* 2),(ad / 160));
           else
 {
  clsf1();
  if (caff> 99) 
  {
    dessine(ades,60,33);
    boite(118, 32, 291, 121, 15);         /* cadre moyen */
  }
  else
   if (caff> 69) 
   {
    dessine(ades,112,48);             /* tˆtes */
    boite(222, 47, 155, 91, 15);
   }
   else
   {
    dessine(ades,0,12);
    ecrf1();
    if ((caff< 30) || (caff> 32)) 
    {
     for( cx= 1; cx <= 6; cx ++)
      if (ord(touv[cx])!= 0)  aniof(1,ord(touv[cx]));
     if (caff== 13) 
     {
      if (s.iboul== 141)  aniof(1,7);
      if (s.ibag== 159)  aniof(1,6);
     }
     if ((caff== 14) && (s.icave== 151))  aniof(1,2);
     if ((caff== 17) && (s.ivier== 143))  aniof(1,1);
     if ((caff== 24) && (s.ipuit!= 0))  aniof(1,1);
    }
    if (caff< 26)  musique(1);
   }
 }
}



