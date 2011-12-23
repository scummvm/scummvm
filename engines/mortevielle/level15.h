
/* NIVEAU 15 */
void copcha()
{
      integer i;

  i= acha;
  do {
    tabdon[i]=tabdon[i+390];
    i=succ(integer,i);
  } while (!(i==acha +390));
}

boolean dans_rect( rectangle r )
   {
         integer x, y, c;

     boolean dans_rect_result;
     read_pos_mouse(x,y,c);
       if ((x>r.x1) &&
          (x<r.x2) &&
          (y>r.y1) &&
          (y<r.y2))  dans_rect_result= true;
                 else dans_rect_result= false;
     return dans_rect_result;
   }

void outbloc(integer n, pattern p, t_nhom pal)
   {
         integer i,j,ad;

     ad=n*404+0xd700;
       {
         memw[0x6000+ad]= p.tax;
         memw[0x6000+ad+2]= p.tay;
         ad=ad+4;
         for( i=1; i <= p.tax; i ++)
           for( j=1; j <= p.tay; j ++)
             mem[0x6000+ad+pred(integer,j)*p.tax+pred(integer,i)]= pal[n].hom[p.des[i][j]];
       }
   }


void writepal(integer n)
 {
         integer i;
       t_nhom pal;


   switch (gd) {
           case tan:
           case ega:
           case ams : for( i=1; i <= 16; i ++)
                   {
                     mem[0x7000+2*i]= tabpal[n][i].x;
                     mem[0x7000+succ(integer,2*i)]= tabpal[n][i].y;
                   }
                   break;
           case cga : {
                   pal= palcga[n].a;
                   if (n<89)  palette(palcga[n].p);
                   for( i=0; i <= 15; i ++) outbloc(i, tpt[pal[i].n],pal);
                 }
                 break;
   }
 }


void pictout(integer seg,integer dep,integer x,integer y)
   {
     decomp(seg,dep);
     if (gd==her) 
        {
          mem[0x7000+2]=0;
          mem[0x7000+32]=15;
        }
     if ((caff!=51) && (memw[0x7000+0x4138]>0x100))  memw[0x7000+0x4138]=0x100;
     afff(gd,seg,dep,x,y);
   }

void putxy(integer x,integer y)
{
  xwhere=x;
  ywhere=y;
}
/*
function calad(x,y:integer):integer;
   begin
     calad:=pred(x)+320*pred(y)
   end;
  */
void sauvecr(integer y,integer dy)
  {
    hide_mouse();
    s_sauv(gd,y,dy);
    show_mouse();
  }

void charecr(integer y,integer dy)
  {
    hide_mouse();
    s_char(gd,y,dy);
    show_mouse();
  }
/*
function peek(ad:integer):integer;
begin
  peek:=tabdon[ad];
end;

function peekw(ad:integer):integer;
begin
  peekw:=tabdon[ad] shl 8+tabdon[succ(ad)];
end;

function peekl(ad:integer):real;
begin
  peekl:=tabdon[ad+3]+tabdon[ad+2] shl 8;
end;


procedure poke (ad,n:integer);
begin
  tabdon[ad]:=lo(n);
end;
  */

void adzon()
   {
         untyped_file f;
         file<byte> g;
         byte a;
         integer i;

     assign(f,"don.mor");
     reset(f,256);
     blockread(f,tabdon,7);
     close(f);
     assign(f,"bmor.mor");
     reset(f,1916);
     blockread(f,tabdon[fleche],1);
     /*i:=fleche;
     repeat
       read(g,a);
       tabdon[i]:=a;
       i:=succ(i);
     until i=fleche + 1916;*/
     close(f);
     assign(f,"dec.mor");
     reset(f,1664);
     blockread(f,mem[0x73a2+0],1);
     /*i:=0;
     repeat
       read(g,a);
       mem[$73A2:i]:=a;
       i:=succ(i);
     until eof(g);*/
     close(f);
   }

integer animof (integer ouf,integer num)
   {
         integer nani, aux;

     integer animof_result;
     nani= mem[adani+1];
     aux= num;
     if (ouf!=1)  aux= aux+nani;
     animof_result=(nani << 2) + 2+swap(memw[adani+ (aux << 1)]);
     /*aux:= nani shl 2 + 2;
     if ouf=1 then aux:= aux+ swap(memw[adani: num shl 1])
              else aux:= aux+ swap(memw[adani: (nani+num) shl 1]);
     animof:=aux;*/
     return animof_result;
   }

/*procedure affgd(offs,dep,x,y:integer);
begin
  case Gd of
     Ams : affams(offs,dep,x,y);
     Cga : affcga(offs,dep,x,y);
     Ega : affega(offs,dep,x,y);
     Her : affher(offs,dep,x,y);
     Tan : afftan(offs,dep,x,y);
  end;
end;*/
