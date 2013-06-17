#include "ptoc.h"

/*
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 CELER            The unit for updating the screen pics. */

#define __celer_implementation__


#include "celer.h"


#include "Graph.h"
#include "trip5.h"
#include "lucerna.h"
/*#include "Crt.h"*/


untyped_file f; /* Private variable- not accessible from elsewhere. */

const integer on_disk = -1; /* Value of memos[fv].x when it's not in memory. */

void pics_link()
{
    byte xx;

 if (ddmnow)  return; /* No animation when the menus are up. */

 { dnatype& with = dna; 
  switch (with.room) {

   case r__outsideargentpub: {
    if ((roomtime % longint(12))==0) 
     show_one(longint(1)+(roomtime / longint(12)) % longint(4));
   }
   break;

   case r__brummieroad: if ((roomtime % longint(2))==0) 
    show_one(longint(1)+(roomtime / longint(2)) % longint(4));
    break;

   case r__bridge: if ((roomtime % longint(2))==0) 
    show_one(longint(4)+(roomtime / longint(2)) % longint(4));
    break;

   case r__yours: if ((! with.avvy_is_awake) && ((roomtime % longint(4))==0)) 
              show_one(longint(1)+(roomtime / longint(12)) % longint(2));
              break;

   case r__argentpub: {
                  if (((roomtime % longint(7))==1) && (dna.malagauche!=177)) 
                  {     /* Malagauche cycle */
                   dna.malagauche += 1;
                   switch (dna.malagauche) {
                    case 1:case 11:case 21: show_one(12); break; /* Looks forwards. */
                    case 8:case 18:case 28:case 32: show_one(11); break; /* Looks at you. */
                    case 30: show_one(13); break; /* Winks. */
                    case 33: dna.malagauche=0; break;
                   }
                  }

                  switch (roomtime % longint(200)) {
                   case 179:case 197: show_one(5); break; /* Dogfood's drinking cycle */
                   case 182:case 194: show_one(6); break;
                   case 185: show_one(7); break;
                   case 199: dna.dogfoodpos=177; break; /* Impossible value for this. */
                   case 0 ... 178: { /* Normally. */
                            switch (bearing(2)) {/* Bearing of Avvy from Dogfood. */
                              case 1 ... 90:case RANGE_3(358,360): xx=3; break;
                              case 293 ... 357: xx=2; break;
                              case 271 ... 292: xx=4; break;
                            }
                            if (xx!=dna.dogfoodpos)    /* Only if it's changed.*/
                            {
                             show_one(xx);
                             dna.dogfoodpos=xx;
                            }
                           }
                           break;
                  }
                 }
                 break;

   case r__westhall: if ((roomtime % longint(3))==0) 
                 switch ((roomtime / longint(3)) % longint(6)) {
                  case 4: show_one(1); break;
                  case 1:case 3:case 5: show_one(2); break;
                  case 0:case 2: show_one(3); break;
                 }
                 break;
   case r__lustiesroom: if (! (dna.lustie_is_asleep))  {
                    if ((roomtime % longint(45))>42)  xx=4; /* du Lustie blinks */
                    else
                    switch (bearing(2)) {/* Bearing of Avvy from du Lustie. */
                      case 0 ... 45:case 315 ... 360: xx=1; break; /* Middle. */
                       case 45 ... 180: xx=2; break; /* Left. */
                      case 181 ... 314: xx=3; break; /* Right. */
                    }
                    if (xx!=dna.dogfoodpos)    /* Only if it's changed.*/
                    {
                     show_one(xx);
                     dna.dogfoodpos=xx;  /* We use DogfoodPos here too- why not? */
                    }
                   }
                   break;

   case r__aylesoffice: if ((! dna.ayles_is_awake) && (roomtime % longint(14)==0)) 
                   {
                    switch ((roomtime / longint(14)) % longint(2)) {
                     case 0: show_one(1); break; /* Frame 2: EGA. */
                     case 1: show_one(3); break; /* Frame 1: Natural. */
                    }
                   }
                   break;

   case r__robins: if (dna.tied_up) 
               switch (roomtime % longint(54)) {
                case 20: show_one(4); break; /* Frame 4: Avalot blinks. */
                case 23: show_one(2); break; /* Frame 1: Back to normal. */
               }
               break;

   case r__nottspub: {
                 switch (bearing(5)) { /* Bearing of Avvy from Port. */
                   case 0 ... 45:case 315 ... 360: xx=2; break; /* Middle. */
                    case 45 ... 180: xx=6; break; /* Left. */
                   case 181 ... 314: xx=8; break; /* Right. */
                 }

                 if ((roomtime % longint(60))>57)  xx -= 1; /* Blinks */

                 if (xx!=dna.dogfoodpos)    /* Only if it's changed.*/
                 {
                  show_one(xx);
                  dna.dogfoodpos=xx;  /* We use DogfoodPos here too- why not? */
                 }

                 switch (roomtime % longint(50)) {
                  case 45 : show_one(9); break; /* Spurge blinks */
                  case 49 : show_one(10); break;
                 }
                }
                break;

   case r__ducks: {
              if ((roomtime % longint(3))==0)  /* The fire flickers */
               show_one(longint(1)+(roomtime / longint(3)) % longint(3));

              switch (bearing(2)) {/* Bearing of Avvy from Duck. */
                case 0 ... 45:case 315 ... 360: xx=4; break; /* Middle. */
                 case 45 ... 180: xx=6; break; /* Left. */
                case 181 ... 314: xx=8; break; /* Right. */
              }

              if ((roomtime % longint(45))>42)  xx += 1; /* Duck blinks */

              if (xx!=dna.dogfoodpos)    /* Only if it's changed.*/
              {
               show_one(xx);
               dna.dogfoodpos=xx;  /* We use DogfoodPos here too- why not? */
              }
             }
             break;

  }}

 if ((dna.ringing_bells) && (flagset('B'))) 
   /* They're ringing the bells. */
  switch (roomtime % longint(4)) {
   case 1: {
       dnatype& with = dna; 

       if (with.nextbell<5)  with.nextbell=12;
       with.nextbell -= 1;
       note(notes[with.nextbell]);
           }
           break;
   case 2: nosound; break;
  }

}

void load_chunks(string xx)
{
 chunkblocktype ch;
 byte fv;

 /*$I-*/
 assign(f,string("chunk")+xx+".avd"); /* strf is done for us by Lucerna. */
 reset(f,1);
 if (ioresult!=0)  return; /* no Chunk file. */
 seek(f,44);
 blockread(f,num_chunks,1);
 blockread(f,offsets,num_chunks*4);

 for( fv=1; fv <= num_chunks; fv ++)
 {
  seek(f,offsets[fv]);
  blockread(f,ch,sizeof(ch));
  {
   if (ch.memorise) 
   {

    {
     memotype& with1 = memos[fv]; 

     with1.x=ch.x; with1.xl=ch.xl;
     with1.y=ch.y; with1.yl=ch.yl;
     with1.flavour=ch.flavour;
     with1.size=ch.size;
    }

    getmem(memory[fv],ch.size);

    if (ch.natural) 
    {
     getimage(ch.x*8,ch.y,(ch.x+ch.xl)*8,ch.y+ch.yl,memory[fv]);
    } else
     blockread(f,memory[fv],ch.size);
   } else memos[fv].x=on_disk;
  }
 }

 close(f);
 /*$I+*/
}

void forget_chunks()
{
    byte fv;

 for( fv=1; fv <= num_chunks; fv ++)
  if (memos[fv].x>on_disk) 
   freemem(memory[fv],memos[fv].size);
 fillchar(memos,sizeof(memos),'\377'); /* x=-1, => on disk. */
}

void mdrop(integer x,integer y,integer xl,integer yl, pointer p) /* assembler;
asm
  push ds;      { Strictly speaking, we shouldn't modify DS, so we'll save it.}
  push bp;      { Nor BP! }


  { DI holds the offset on this page. It starts at the top left-hand corner. }
  { (It should equal ch.y*80+ch.x. }

  mov ax,y;
  mov dl,80;
  mul dl; { Line offset now calculated. }
  mov di,ax; { Move it into DI. }
  mov ax,x;
  add di,ax; { Full offset now calculated. }

  mov bx,yl; { No. of times to repeat lineloop. }
  inc bx;        { "loop" doesn't execute the zeroth time. }
  mov bh,bl;     { Put it into BH. }

  { BP holds the length of the string to copy. It's equal to ch.xl.}

  mov ax,word(p);   { Data is held at DS:SI. }
  mov si,ax;
  mov ax,word(p+2); { This will be moved over into ds in just a tick... }

  mov bp,xl;

  mov ds,ax;


  cld;          { We're allowed to hack around with the flags! }

  mov ax,$AC00; { Top of the first EGA page. }
  mov es,ax;    { Offset on this page is calculated below... }


{    port[$3c4]:=2; port[$3ce]:=4; }

  mov dx,$3c4;
  mov al,2;
  out dx,al;
  mov dx,$3ce;
  mov al,4;
  out dx,al;

  mov cx,4;  { This loop executes for 3, 2, 1, and 0. }
  mov bl,0;


 @mainloop:

    push di;
    push cx;

{    port[$3C5]:=1 shl bit; }
    mov dx,$3C5;
    mov al,1;
    mov cl,bl; { BL = bit. }
    shl al,cl;
    out dx,al;
{     port[$3CF]:=bit; }
    mov dx,$3CF;
    mov al,bl; { BL = bit. }
    out dx,al;

    xor ch,ch;
    mov cl,bh; { BH = ch.yl. }

   @lineloop:

     push cx;

     mov cx,bp;

     repz movsb; { Copy the data. }

     sub di,bp;
     add di,80;

     pop cx;

   loop @lineloop;

    inc bl; { One more on BL. }

    pop cx;
    pop di;

 loop @mainloop;

  pop bp;
  pop ds;       { Get DS back again. }
*/
{;
}

void show_one(byte which);

static bytefield r;

static void display_it(integer x,integer y,integer xl,integer yl, flavourtype flavour, pointer p)
{
 {
  switch (flavour) {
   case ch_bgi : {
             putimage(x*8,y,p,0);
             r.x1=x; r.y1=y;
             r.x2=x+xl+1; r.y2=y+yl;
            }
            break;
   case ch_ega : {
             mdrop(x,y,xl,yl,p); blitfix();
             r.x1=x; r.y1=y; r.x2=x+xl; r.y2=y+yl;
            }
            break;
  }
}
}

void show_one(byte which)
{
 chunkblocktype ch;
 pointer p;
 byte fv;


 setactivepage(3);

 {
  memotype& with = memos[which]; 

  if (with.x>on_disk) 
  {
   display_it(with.x,with.y,with.xl,with.yl,with.flavour,memory[which]);
  } else
  {
   reset(f,1);
   seek(f,offsets[which]);
   blockread(f,ch,sizeof(ch));

   {
    getmem(p,ch.size);
    blockread(f,p,ch.size);

    display_it(ch.x,ch.y,ch.xl,ch.yl,ch.flavour,p);
    freemem(p,ch.size);

    close(f);
   }

  }

  setactivepage(1-cp);

  for( fv=0; fv <= 1; fv ++)
   getset[fv].remember(r);
 }

}

void show_one_at(byte which, integer xxx,integer yyy);

static bytefield r1;

static void display_it1(integer xl,integer yl, flavourtype flavour, pointer p, integer& xxx, integer& yyy)
{
 {
  switch (flavour) {
   case ch_bgi : {
             putimage(xxx,yyy,p,0);
             r1.x1=xxx; r1.y1=yyy;
             r1.x2=xxx+xl+1; r1.y2=yyy+yl;
            }
            break;
   case ch_ega : {
             mdrop(xxx / 8,yyy,xl,yl,p); blitfix();
             r1.x1=xxx / 8; r1.y1=yyy; r1.x2=(xxx / 8)+xl; r1.y2=yyy+yl;
            }
            break;
  }
}
}

void show_one_at(byte which, integer xxx,integer yyy)
{
 chunkblocktype ch;
 pointer p;
 byte fv;


 setactivepage(3);

 {
  memotype& with = memos[which]; 

  if (with.x>on_disk) 
  {
   display_it1(with.xl,with.yl,with.flavour,memory[which], xxx, yyy);
  } else
  {
   reset(f,1);
   seek(f,offsets[which]);
   blockread(f,ch,sizeof(ch));

   {
    getmem(p,ch.size);
    blockread(f,p,ch.size);

    display_it1(ch.xl,ch.yl,ch.flavour,p, xxx, yyy);
    freemem(p,ch.size);

    close(f);
   }

  }

  setactivepage(1-cp);

  for( fv=0; fv <= 1; fv ++)
   getset[fv].remember(r1);
 }
}

class unit_celer_initialize {
  public: unit_celer_initialize();
};
static unit_celer_initialize celer_constructor;

unit_celer_initialize::unit_celer_initialize() {
 num_chunks=0;
}
