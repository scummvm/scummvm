


#include "graph.h"
/*#include "Crt.h"*/

/*$R+*/

const array<1,5,shortint> adjustment = {{7,0,7,7,7}};

const array<0,3,byte> plane_to_use = {{2,2,2,3}};

const array<1,5,byte> waveorder = {{5,1,2,3,4}};

const array<1,26,byte> glerkfade = 
 {{1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,4,4,3,3,3,2,2,1}};

const array<1,18,byte> greldetfade = {{1,2,3,4,5,6,6,6,5,5,4,4,3,3,2,2,1,1}};

enum flavourtype {ch_ega,ch_bgi,ch_natural,ch_two,ch_one, last_flavourtype};

struct chunkblocktype {
                  flavourtype flavour;
                  integer x,y;
                  integer xl,yl;
                  longint size;
};

typedef array<1,6,0,3,0,34,1,9,byte> glerktype;


untyped_file f;
chunkblocktype cb;
array<1,5,2,3,0,65,0,25,byte> ghost;
byte fv;
pointer memlevel;
byte y,yy,bit,xofs;

array<0,1,pointer> eyes;
pointer exclamation;
array<1,6,pointer> aargh;
array<1,3,pointer> bat;
glerktype* glerk;
array<1,5,pointer> green_eyes;
matrix<1,6,false,true,pointer> greldet;

array<1,6,pointtype> aargh_where;

integer gd,gm; boolean gb;

byte glerkstage;

integer bat_x,bat_y; word bat_count; shortint aargh_count;

integer greldet_x,greldet_y; byte greldet_count; boolean red_greldet;

void plain_grab()
 /* Just grabs the next one and puts it where it's told to. */
{
 integer xx,yy,xofs;
;
 blockread(f,cb,sizeof(cb));

  switch (cb.flavour) {
   case ch_one: {;
            xofs=cb.x / 8;
            bit=3;
             for( yy=0; yy <= cb.yl; yy ++)
             {;
              port[0x3c4]=2; port[0x3ce]=4;
              port[0x3c5]=1 << bit; port[0x3cf]=bit;

              blockread(f,mem[0xa000*(yy+cb.y)*80+xofs],cb.xl / 8);
             }
           }
           break;
   case ch_ega: {;
            xofs=cb.x / 8;
            bit=3;
            for( bit=0; bit <= 3; bit ++)
             for( yy=0; yy <= cb.yl; yy ++)
             {;
              port[0x3c4]=2; port[0x3ce]=4;
              port[0x3c5]=1 << bit; port[0x3cf]=bit;

              blockread(f,mem[0xa000*(yy+cb.y)*80+xofs],cb.xl / 8);
             }
           }
           break;
  }
}

void get_me(pointer& p)
{;
 blockread(f,cb,sizeof(cb));
 /* Take it for granted that cb.flavour = ch_BGI! */

 {;
  getmem(p,cb.size);
  blockread(f,p,cb.size);
 }
}

void get_me_aargh(byte which)
{;
 blockread(f,cb,sizeof(cb));
 /* Take it for granted that cb.flavour = ch_BGI! */

 {;
  getmem(aargh[which],cb.size);
  blockread(f,aargh[which],cb.size);
 }

 {
  pointtype& with = aargh_where[which]; 

  with.x=cb.x;
  with.y=cb.y;
 }
}

void wait(word how_long)
{
    word i; char r;

 for( i=1; i <= how_long; i ++)
  if (keypressed()) 
  {
   sound(6177);
   while (keypressed())  r=readkey();
   delay(1);
   nosound;
  } else
   delay(1);
}

void do_bat();
static byte batimage; 
static pointtype batsize;

static void super_blank()
{
 pointtype oldsize;

 move(bat[batimage-1],oldsize,4);
 bar(bat_x+batsize.x,bat_y,bat_x+oldsize.x,bat_y+oldsize.y);
}

void do_bat()
{
 shortint dx,iy;

 bat_count += 1;

 if (odd(bat_count)) 
 {
  switch (bat_count) {
   case 1 ... 90: { dx=2; iy=1; batimage=1; } break;
   case 91 ... 240: { dx=1; iy=1; batimage=2; } break;
   case 241 ... 260: { dx=1; iy=4; batimage=3; } break;
  }

  move(bat[batimage],batsize,4);

  if ((bat_count==91) || (bat_count==241)) 
   super_blank(); /* When the bat changes, blank out the old one. */

  bar(bat_x,bat_y,bat_x+batsize.x,bat_y+iy);
  bar(bat_x+batsize.x,bat_y,bat_x+batsize.x-dx,bat_y+batsize.y);

  bat_x -= dx; bat_y += 1;
  putimage(bat_x,bat_y,bat[batimage],0);
 }
}

void big_green_eyes(byte how)
{
 putimage(330,103,green_eyes[how],0);
 putimage(376,103,green_eyes[how],0);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 if (paramstr(1)!="jsb")  exit(255);
 gd=3; gm=0; initgraph(gd,gm,"");
 fillchar(ghost,sizeof(ghost),'\0');

 assign(f,"spooky.avd");
 reset(f,1);
 seek(f,44);

 mark(memlevel);

 for( fv=1; fv <= 5; fv ++)
 {;
  blockread(f,cb,sizeof(cb));
  for( bit=2; bit <= 3; bit ++)
   for( y=0; y <= cb.yl; y ++)
    blockread(f,ghost[fv][bit][y],cb.xl / 8);
 }

 get_me(eyes[0]); /* Get BGI-based ones */
 get_me(eyes[1]);
 get_me(exclamation);

 plain_grab(); /* Grabs to screen (cobweb) */
 plain_grab(); /* Grabs to screen (Mark's signature) */
 plain_grab(); /* Grabs to screen (open door) */

 for( fv=1; fv <= 3; fv ++)
  get_me(bat[fv]);

/* new(glerk);
 for fv:=1 to 5 do { Grab the Glerk. }
 begin;
  blockread(f,cb,sizeof(cb));
  for bit:=0 to 3 do
   for y:=0 to 34 do
    blockread(f,glerk^[fv,bit,y],cb.xl div 8);
  inc(gd,gm);
 end;
*/

 glerk = new glerktype;

 for( fv=1; fv <= 6; fv ++)
  {;
   blockread(f,cb,sizeof(cb));
   bit=3;
   for( bit=0; bit <= 3; bit ++)
    for( yy=0; yy <= cb.yl; yy ++)
/*    begin;
     port[$3c4]:=2; port[$3ce]:=4;
     port[$3C5]:=1 shl bit; port[$3CF]:=bit;*/

     blockread(f,(*glerk)[fv][bit][yy],cb.xl / 8);
/*     move(glerk^[fv,bit,yy],mem[$A000:yy*80+177],xl div 8);*/
/*     blockread(f,mem[$A000:yy*80+177],xl div 8);*/

/*    end;*/
  }

 for( fv=1; fv <= 6; fv ++) get_me_aargh(fv);
 for( fv=1; fv <= 5; fv ++) get_me(green_eyes[fv]);
 for( gb=false; gb <= true; gb ++)
  for( fv=1; fv <= 6; fv ++) get_me(greldet[fv][gb]);

 close(f);

 /* Avvy walks over... */

 setfillstyle(1,0);
 glerkstage=0; bat_x=277; bat_y=40; bat_count=0;

 for( gd=500; gd >= 217; gd --)
 {;
  if (set::of(range(22,27), eos).has((gd % 30))) 
  {;
   if ((gd % 30)==27)  bar(gd,135,gd+16,136);
   putimage(gd,136,eyes[0],0);
   putpixel(gd+16,137,0);
  } else
  {;
   if (gd % 30==21)  bar(gd,137,gd+17,138);

   putimage(gd,135,eyes[0],0);
   putpixel(gd+16,136,0); /* eyes would leave a trail 1 pixel high behind them */
  }

  /* Plot the Glerk: */
  if (gd % 10==0) 
  {;
   glerkstage += 1;
   if (glerkstage>26)  flush();

   for( gm=0; gm <= 34; gm ++)
    for( bit=0; bit <= 3; bit ++)
    {;
     port[0x3c4]=2; port[0x3ce]=4;
     port[0x3c5]=1 << bit; port[0x3cf]=bit;

     move((*glerk)[glerkfade[glerkstage]][bit][gm],
      mem[0xa000*1177+gm*80],9);
    }
    bit=getpixel(0,0);
   }

   do_bat();

   wait(15);
 }

 setfillstyle(1,0);
 bar(456,14,530,50);

 /* It descends... */

 for( gm=-64; gm <= 103; gm ++)
 {;
  bit=getpixel(0,0);

  if (gm>0) 
   fillchar(mem[0xa000*(gm-1)*80],26,'\0');

  for( y=0; y <= 65; y ++)
   if ((y+gm)>=0) 
    for( bit=0; bit <= 3; bit ++)
     {;
      port[0x3c4]=2; port[0x3ce]=4;
      port[0x3c5]=1 << bit; port[0x3cf]=bit;

      move(ghost[2+(abs(gm / 7) % 2)*3][plane_to_use[bit]][y],
       mem[0xa000*(y+gm)*80],26);
     }

  wait(27);
 }

 /* And waves... */

 aargh_count=-14;

 for( gd=1; gd <= 4; gd ++)
  for( fv=1; fv <= 5; fv ++)
  {;

   y=getpixel(0,0);

   for( y=0; y <= 7; y ++)
    fillchar(mem[0xa000*7760+y*80],26,'\0');

   for( y=0; y <= 65; y ++)
    for( bit=0; bit <= 3; bit ++)
    {;
     port[0x3c4]=2; port[0x3ce]=4;
     port[0x3c5]=1 << bit; port[0x3cf]=bit;
     move(ghost[waveorder[fv]][plane_to_use[bit]][y],
      mem[0xa000*7760+(y+adjustment[fv])*80],26);
    }

   aargh_count += 1;

   if (aargh_count>0) 
    { pointtype& with = aargh_where[aargh_count]; 
     putimage(with.x,with.y,aargh[aargh_count],0);}

   wait(177);
 }

 /* ! appears */

 gd=getpixel(0,0);
 putimage(246,127,exclamation,0);
 wait(777);
 bar(172, 78,347,111); /* erase "aargh" */

 for( gm=5; gm >= 1; gm --)
 {
  wait(377);
  big_green_eyes(gm);
 }
 bar(246,127,251,133); /* erase the "!" */

 /* He hurries back. */

 glerkstage=1; greldet_count=18; red_greldet=false;

 for( gd=217; gd <= 479; gd ++)
 {;
  if (set::of(range(22,27), eos).has((gd % 30))) 
  {;
   if ((gd % 30)==22) 
    bar(gd+22,134,gd+38,137);
   putimage(gd+23,136,eyes[1],0);
   putpixel(gd+22,137,0);
  } else
  {;
   if (gd % 30==28) 
    bar(gd+22,135,gd+38,138);

   putimage(gd+23,135,eyes[1],0);
   putpixel(gd+22,136,0); /* eyes would leave a trail 1 pixel high behind them */
  }

  /* Plot the Green Eyes: */
  if (gd % 53==5) 
  {;
   big_green_eyes(glerkstage);
   glerkstage += 1;
  }

  /* Plot the Greldet: */

  if (greldet_count==18) 
  {     /* A new greldet. */
   greldet_x=Random(600);
   greldet_y=Random(80);
   greldet_count=0;
   red_greldet=! red_greldet;
  }

  greldet_count += 1;
  putimage
   (greldet_x,greldet_y,greldet[greldetfade[greldet_count]][red_greldet],0);

  wait(10);
 }

 release(memlevel);
 closegraph();
return EXIT_SUCCESS;
}
