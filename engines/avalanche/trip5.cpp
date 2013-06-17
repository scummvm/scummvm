#include "ptoc.h"

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 TRIP5            Trippancy V */

#define __trip5_implementation__
            /* Trippancy V */ /* Define NOASM to use Pascal instead of asm. */
/*$S- Fast!*/
#include "trip5.h"


/*#include "Scrolls.h"*/
/*#include "Lucerna.h"*/
/*#include "Dropdown.h"*/
/*#include "Visa.h"*/
/*#include "Celer.h"*/
/*#include "Timeout.h"*/
/*#include "Sequence.h"*/
/*#include "Enid.h"*/


void loadtrip()
{
    byte gm;

 for( gm=1; gm <= numtr; gm ++) tr[gm].original();
 fillchar(aa,sizeof(aa),'\0');
}

byte checkfeet(integer x1,integer x2,integer oy,integer y, byte yl)
{
    byte a,c; integer fv,ff;

/* if not alive then begin checkfeet:=0; exit; end;*/
 byte checkfeet_result;
 a=0; setactivepage(2); if (x1<0)  x1=0; if (x2>639)  x2=639;
 if (oy<y) 
  for( fv=x1; fv <= x2; fv ++)
   for( ff=oy+yl; ff <= y+yl; ff ++)
   {
    c=getpixel(fv,ff);
    if (c>a)  a=c;
   } else
  for( fv=x1; fv <= x2; fv ++)
   for( ff=y+yl; ff <= oy+yl; ff ++)
   {
    c=getpixel(fv,ff);
    if (c>a)  a=c;
   }
 checkfeet_result=a; setactivepage(1-cp);
 return checkfeet_result;
}

byte geida_ped(byte which)
{
 byte geida_ped_result;
 switch (which) {
    case 1: geida_ped_result= 7; break;
  case 2:case 6: geida_ped_result= 8; break;
  case 3:case 5: geida_ped_result= 9; break;
    case 4: geida_ped_result=10; break;
 }
 return geida_ped_result;
}

void catamove(byte ped)
 /* When you enter a new position in the catacombs, this procedure should
   be called. It changes the Also codes so that they may match the picture
   on the screen. (Coming soon: It draws up the screen, too.) */
{
 longint here;
 word xy_word;
 byte fv,ff;

/* XY_word is cat_x+cat_y*256. Thus, every room in the
  catacombs has a different number for it. */


 {
  xy_word=cat_x+cat_y*256;
  geida_spin=0;
 }
 switch (xy_word) {
  case 1801: { /* Exit catacombs */
         fliproom(r__lustiesroom,4);
         display("Phew! Nice to be out of there!");
         return;
        }
        break;
  case 1033: { /* Oubliette */
         fliproom(r__oubliette,1);
         display("Oh, NO!\231\2");
         return;
        }
        break;
     case 4: {
         fliproom(r__geidas,1);
         return;
        }
        break;
  case 2307: {
         fliproom(r__lusties,5);
         display("Oh no... here we go again...");
         dna.user_moves_avvy=false;
         tr[1].iy=1; tr[1].ix=0;
         return;
        }
        break;
 }

 if (~ dna.enter_catacombs_from_lusties_room)  load(29);
             here=catamap[cat_y][cat_x];

 switch (here & 0xf) { /* West. */
  case 0: { /* no connection (wall) */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=nix;  /* Door. */
      show_one(28);
     }
     break;
  case 0x1: { /* no connection (wall + shield), */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=nix;  /* Door. */
      show_one(28); /* Wall, plus... */
      show_one(29); /* ...shield. */
     }
     break;
  case 0x2: { /* wall with door */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=special;  /* Door. */
      show_one(28); /* Wall, plus... */
      show_one(30); /* ...door. */
     }
     break;
  case 0x3: { /* wall with door and shield */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=special;  /* Door. */
      show_one(28); /* Wall, plus... */
      show_one(30); /* ...door, and... */
      show_one(29); /* ...shield. */
     }
     break;
  case 0x4: { /* no connection (wall + window), */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=nix;  /* Door. */
      show_one(28); /* Wall, plus... */
      show_one(5);  /* ...window. */
     }
     break;
  case 0x5: { /* wall with door and window */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=special;  /* Door. */
      show_one(28); /* Wall, plus... */
      show_one(30); /* ...door, and... */
      show_one(5); /* ...window. */
     }
     break;
  case 0x6: { /* no connection (wall + torches), */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=nix;  /* No door. */
      show_one(28); /* Wall, plus... */
      show_one(7); /* ...torches. */
     }
     break;
  case 0x7: { /* wall with door and torches */
      magics[2].op=bounces;  /* Sloping wall. */
      magics[3].op=nix;  /* Straight wall. */
      portals[13].op=special;  /* Door. */
      show_one(28); /* Wall, plus... */
      show_one(30); /* ...door, and... */
      show_one(7); /* ...torches. */
     }
     break;
  case 0xf: { /* straight-through corridor. */
      magics[2].op=nix;  /* Sloping wall. */
      magics[3].op=special;  /* Straight wall. */
     }
     break;
 }

                         /*  ---- */

 switch ((cardinal)(here & 0xf0) >> 4) { /* East */
  case 0: { /* no connection (wall) */
      magics[5].op=bounces;  /* Sloping wall. */
      magics[6].op=nix;  /* Straight wall. */
      portals[15].op=nix;  /* Door. */
      show_one(19);
     }
     break;
  case 0x1: { /* no connection (wall + window), */
      magics[5].op=bounces;  /* Sloping wall. */
      magics[6].op=nix;  /* Straight wall. */
      portals[15].op=nix;  /* Door. */
      show_one(19); /* Wall, plus... */
      show_one(20); /* ...window. */
     }
     break;
  case 0x2: { /* wall with door */
      magics[5].op=bounces;  /* Sloping wall. */
      magics[6].op=nix;  /* Straight wall. */
      portals[15].op=special;  /* Door. */
      show_one(19); /* Wall, plus... */
      show_one(21); /* ...door. */
     }
     break;
  case 0x3: { /* wall with door and window */
      magics[5].op=bounces;  /* Sloping wall. */
      magics[6].op=nix;  /* Straight wall. */
      portals[15].op=special;  /* Door. */
      show_one(19); /* Wall, plus... */
      show_one(20); /* ...door, and... */
      show_one(21); /* ...window. */
     }
     break;
  case 0x6: { /* no connection (wall + torches), */
      magics[5].op=bounces;  /* Sloping wall. */
      magics[6].op=nix;  /* Straight wall. */
      portals[15].op=nix;  /* No door. */
      show_one(19); /* Wall, plus... */
      show_one(18); /* ...torches. */
     }
     break;
  case 0x7: { /* wall with door and torches */
      magics[5].op=bounces;  /* Sloping wall. */
      magics[6].op=nix;  /* Straight wall. */
      portals[15].op=special;  /* Door. */
      show_one(19); /* Wall, plus... */
      show_one(21); /* ...door, and... */
      show_one(18); /* ...torches. */
     }
     break;
  case 0xf: { /* straight-through corridor. */
      magics[5].op=nix;  /* Sloping wall. */
      magics[6].op=special;  /* Straight wall. */
      portals[15].op=nix;  /* Door. */
     }
     break;
 }

                         /*  ---- */

 switch ((cardinal)(here & 0xf00) >> 8) { /* South */
  case 0: { /* No connection. */
      magics[7].op=bounces;
      magics[12].op=bounces;
      magics[13].op=bounces;
     }
     break;
  case 0x1: {
       show_one(22);
       { void& with = magics[13]; 
        if ((xy_word==2051) & (dna.geida_follows)) 
         op=exclaim;
        else op=special;} /* Right exit south. */
       magics[7].op=bounces;
       magics[12].op=bounces;
     }
     break;
  case 0x2: {
      show_one(23);
      magics[7].op=special;  /* Middle exit south. */
       magics[12].op=bounces;
       magics[13].op=bounces;
     }
     break;
  case 0x3: {
       show_one(24);
       magics[12].op=special;  /* Left exit south. */
       magics[7].op=bounces;
       magics[13].op=bounces;
      }
      break;
 }

 switch ((cardinal)(here & 0xf000) >> 12) { /* North */
  case 0: { /* No connection */
      magics[1].op=bounces;
      portals[12].op=nix;  /* Door. */
     }
     break;
   /* LEFT handles: */
/*  $1: begin
      show_one(4);
      magics[1].op:=bounces; { Left exit north. } { Change magic number! }
      portals[12].op:=special; { Door. }
     end;*/
  case 0x2: {
      show_one(4);
      magics[1].op=bounces;  /* Middle exit north. */
      portals[12].op=special;  /* Door. */
     }
     break;
/*  $3: begin
      show_one(4);
      magics[1].op:=bounces; { Right exit north. } { Change magic number! }
      portals[12].op:=special; { Door. }
     end;
  { RIGHT handles: }
  $4: begin
      show_one(3);
      magics[1].op:=bounces; { Left exit north. } { Change magic number! }
      portals[12].op:=special; { Door. }
     end;*/
  case 0x5: {
      show_one(3);
      magics[1].op=bounces;  /* Middle exit north. */
      portals[12].op=special;  /* Door. */
     }
     break;
/*  $6: begin
      show_one(3);
      magics[1].op:=bounces; { Right exit north. }
      portals[12].op:=special; { Door. }
     end;*/
 /* ARCHWAYS: */
  case 0x7:case 0x8:case 0x9: {
      show_one(6);

      if (((cardinal)(here & 0xf000) >> 12)>0x7)  show_one(31);
      if (((cardinal)(here & 0xf000) >> 12)==0x9)  show_one(32);

      magics[1].op=special;  /* Middle arch north. */
      portals[12].op=nix;  /* Door. */
     }
     break;
  /* DECORATIONS: */
  case 0xd: { /* No connection + WINDOW */
      magics[1].op=bounces;
      portals[12].op=nix;  /* Door. */
      show_one(14);
     }
     break;
  case 0xe: { /* No connection + TORCH */
      magics[1].op=bounces;
      portals[12].op=nix;  /* Door. */
      show_one(8);
     }
     break;
 /* Recessed door: */
  case 0xf: {
      magics[1].op=nix;  /* Door to Geida's room. */
      show_one(1);
      portals[12].op=special;  /* Door. */
     }
     break;
 }

 switch (xy_word) {
   case 514: show_one(17); break;     /* [2,2] : "Art Gallery" sign over door. */
   case 264: show_one(9); break;      /* [8,1] : "The Wrong Way!" sign. */
  case 1797: show_one(2); break;      /* [5,7] : "Ite Mingite" sign. */
   case 258: for( fv=0; fv <= 2; fv ++) /* [2,1] : Art gallery - pictures */
         {
          show_one_at(15,130+fv*120,70);
          show_one_at(16,184+fv*120,78);
         }
         break;
  case 1287: for( fv=10; fv <= 13; fv ++) show_one(fv); break; /* [7,5] : 4 candles. */
   case 776: show_one(10); break;     /* [8,3] : 1 candle. */
  case 2049: show_one(11); break;     /* [1,8] : another candle. */
   case 257: { show_one(12); show_one(13); } break; /* [1,1] : the other two. */
 }

 if ((dna.geida_follows) & (ped>0)) 
 {
   triptype& with = tr[2]; 

   if (! with.quick)  /* If we don't already have her... */
    tr[2].init(5,true); /* ...Load Geida. */
   apped(2,geida_ped(ped));
   tr[2].call_eachstep =true;
   tr[2].eachstep=procgeida_procs;
 }
}

void call_special(word which);

 /* This proc gets called whenever you touch a line defined as Special. */
static void dawndelay()
 { set_up_timer(2,procdawn_delay,reason_dawndelay); }

void call_special(word which)
{
 switch (which) {
  case 1: { /* Special 1: Room 22: top of stairs. */
      show_one(1);
      dna.brummie_stairs=1;
      magics[10].op=nix;
      set_up_timer(10,procstairs,reason_brummiestairs);
      stopwalking();
      dna.user_moves_avvy=false;
     }
     break;
  case 2: { /* Special 2: Room 22: bottom of stairs. */
      dna.brummie_stairs=3;
      magics[11].op=nix;
      magics[12].op=exclaim;
      magics[12].data=5;
      magics[4].op=bounces;  /* Now works as planned! */
      stopwalking();
      dixi('q',26);
      dna.user_moves_avvy=true;
     }
     break;
  case 3: { /* Special 3: Room 71: triggers dart. */
      tr[1].bounce(); /* Must include that. */

      if (~ dna.arrow_triggered) 
      {
       dna.arrow_triggered=true;
       apped(2,4); /* The dart starts at ped 4, and... */
       tr[2].walkto(5); /* flies to ped 5. */
       tr[2].face=0;  /* Only face. */
       /* Should call some kind of Eachstep procedure which will deallocate
         the sprite when it hits the wall, and replace it with the chunk
         graphic of the arrow buried in the plaster. */
       /* OK! */
       tr[2].call_eachstep=true;
       tr[2].eachstep=procarrow_procs;
      }
     }
     break;

  case 4: { /* This is the ghost room link. */
      dusk;
      tr[1].turn(right); /* you'll see this after we get back from bootstrap */
      set_up_timer(1,procghost_room_phew,reason_ghost_room_phew);
      back_to_bootstrap(3);
     }
     break;

  case 5: if (dna.friar_will_tie_you_up) 
     {     /* Special 5: Room 42: touched tree, and get tied up. */
      magics[4].op=bounces;  /* Boundary effect is now working again. */
      dixi('q',35);
      tr[1].done();
      /*tr[1].vanishifstill:=true;*/
      show_one(2);
      dixi('q',36);
      dna.tied_up=true;
      dna.friar_will_tie_you_up=false;
      tr[2].walkto(3);
      tr[2].vanishifstill=true;
      tr[2].check_me=true;  /* One of them must have Check_Me switched on. */
      whereis[pfriartuck]=177;  /* Not here, then. */
      set_up_timer(364,prochang_around,reason_hanging_around);
     }
     break;

   case 6: { /* Special 6: fall down oubliette. */
       dna.user_moves_avvy=false;
       tr[1].ix=3;
       tr[1].iy=0;
       tr[1].face=right;
       set_up_timer(1,procfall_down_oubliette,reason_falling_down_oubliette);
      }
      break;

   case 7: { /* Special 7: stop falling down oubliette. */
       tr[1].visible=false;
       magics[10].op=nix;
       stopwalking();
       lose_timer(reason_falling_down_oubliette);
       mblit(12,80,38,160,3,0);
       mblit(12,80,38,160,3,1);
       display("Oh dear, you seem to be down the bottom of an oubliette.");
       set_up_timer(200,procmeet_avaroid,reason_meeting_avaroid);
      }
      break;

   case 8:        /* Special 8: leave du Lustie's room. */
       if ((geida_follows) & (~ lustie_is_asleep)) 
       {
        dixi('q',63);
        tr[2].turn(down); tr[2].stopwalk(); tr[2].call_eachstep=false; /* Geida */
        gameover;
       }
       break;

   case 9: { /* Special 9: lose Geida to Robin Hood... */
       if (~ dna.geida_follows)  return;   /* DOESN'T COUNT: no Geida. */
       tr[2].call_eachstep=false;  /* She no longer follows Avvy around. */
       tr[2].walkto(4); /* She walks to somewhere... */
       tr[1].done();     /* Lose Avvy. */
       dna.user_moves_avvy=false;
       set_up_timer(40,procrobin_hood_and_geida,reason_robin_hood_and_geida);
      }
      break;

  case 10: { /* Special 10: transfer north in catacombs. */
       if ((dna.cat_x==4) && (dna.cat_y==1)) 
       {     /* Into Geida's room. */
        if (dna.obj[key])  dixi('q',62); else
        {
         dixi('q',61);
         return;
        }
       }
       dusk;
       dna.cat_y -= 1;
       catamove(4); if (dna.room!=r__catacombs)  return;
       delavvy;
       switch ((cardinal)(catamap[cat_y][cat_x] & 0xf00) >> 8) {
        case 0x1: apped(1,12); break;
        case 0x3: apped(1,11); break;
        default: apped(1,4);
       }
       getback();
       dawndelay();
      }
      break;
  case 11: { /* Special 11: transfer east in catacombs. */
       dusk;
       dna.cat_x += 1;
       catamove(1); if (dna.room!=r__catacombs)  return;
       delavvy;
       apped(1,1);
       getback();
       dawndelay();
      }
      break;
  case 12: { /* Special 12: transfer south in catacombs. */
       dusk;
       dna.cat_y += 1;
       catamove(2); if (dna.room!=r__catacombs)  return;
       delavvy;
       apped(1,2);
       getback();
       dawndelay();
      }
      break;
  case 13: { /* Special 13: transfer west in catacombs. */
       dusk;
       dna.cat_x -= 1;
       catamove(3); if (dna.room!=r__catacombs)  return;
       delavvy;
       apped(1,3);
       getback();
       dawndelay();
      }
      break;
 }
}

void hide_in_the_cupboard(); 

void open_the_door(byte whither,byte ped,byte magicnum)
 /* This slides the door open. (The data really ought to be saved in
   the Also file, and will be next time. However, for now, they're
   here.) */
{
 switch (dna.room) {
  case r__outsideyours:case r__outsidenottspub:case r__outsideducks:
   {
    first_show(1);
    then_show(2);
    then_show(3);
   }
   break;
  case r__insidecardiffcastle:
   {
    first_show(1);
    then_show(5);
   }
   break;
  case r__avvysgarden:case r__entrancehall:case r__insideabbey:
   {
    first_show(1);
    then_show(2);
   }
   break;
  case r__musicroom:case r__outsideargentpub:
   {
    first_show(5);
    then_show(6);
   }
   break;
  case r__lusties:
   switch (magicnum) {
    case 14: if (dna.avvys_in_the_cupboard) 
         {
          hide_in_the_cupboard();
          first_show(8); then_show(7);
          start_to_close;
          return;
         } else
         {
            apped(1,6);
            tr[1].face=right;  /* added by TT 12/3/1995 */
            first_show(8); then_show(9);
         }
         break;
    case 12: {
         first_show(4); then_show(5); then_show(6);
        }
        break;
   }
   break;
 }

 then_flip(whither,ped);
 start_to_open;
}

void newspeed()
 /* Given that you've just changed the speed in triptype.xs, this adjusts
   ix. */
{
       const bytefield lightspace = {40, 199, 47, 199};
     byte page_;

 {
   triptype& with = tr[1]; 

   with.ix=(with.ix / 3)*with.xs;
   setactivepage(3);

   setfillstyle(1,14);
   if (with.xs==run)  bar(371,199,373,199); else bar(336,199,338,199);
   setfillstyle(1,9);
   if (with.xs==run)  bar(336,199,338,199); else bar(371,199,373,199);

   setactivepage(1-cp);
   for( page_=0; page_ <= 1; page_ ++) getset[page_].remember(lightspace);
 }
}

triptype* triptype::init(byte spritenum, boolean do_check)
{
      const integer idshould = -1317732048;
 integer gd,gm; varying_string<2> xx;
 byte fv/*,nds*/;
 byte aa,bb;
 longint id; word soa;
 untyped_file inf;

 if (spritenum==177)  return; /* Already running! */
 str(spritenum,xx); assign(inf,string("sprite")+xx+".avd");
 reset(inf,1);
 seek(inf,177);
 blockread(inf,id,4);
 if (id!=idshould) 
 {
  output << '\7';
  close(inf);
  exit(0);
 }

 blockread(inf,soa,2);
 blockread(inf,a,soa);

 {
                        adxtype& with = a; 

  /*nds:=num div seq;*/ totalnum=1;
  xw=with.xl / 8; if ((with.xl % 8)>0)  xw += 1;
  for( aa=1; aa <= /*nds*seq*/with.num; aa ++)
  {
   getmem(sil[totalnum],11*(with.yl+1));
   getmem(mani[totalnum],with.size-6);
   for( fv=0; fv <= with.yl; fv ++)
   {
    blockread(inf,(*sil[totalnum])[fv],xw);
   }
   blockread(inf,*mani[totalnum],with.size-6);
   totalnum += 1;
  }
 }

 /* on; */
 x=0; y=0; quick=true; visible=false; xs=3; ys=1;
/* if spritenum=1 then newspeed; { Just for the lights. }*/

 homing=false; ix=0; iy=0; step=0; check_me=do_check;
 count=0; whichsprite=spritenum; vanishifstill=false;
 call_eachstep=false;
 close(inf);
 return this;
}

void triptype::original()
{
 quick=false; whichsprite=177;
}

void triptype::andexor()
{
 byte picnum; /* Picnum, Picnic, what ye heck */
 byte lay,laz; /* Limits for Qaz and Qay or equivs. (Laz always = 3). */
#ifdef NOASM
 word offs,fv;
 byte qax,qay,qaz;
#else
 word segmani,ofsmani;
 word ofsaa,realofsaa;
 word segsil,ofssil;
 word z; byte xwidth;

#endif

 if ((vanishifstill) && (ix==0) && (iy==0))  return;
 picnum=face*a.seq+step+1;

 {
  adxtype& with = a; 

  getimage(x,y,x+with.xl,y+with.yl,aa); /* Now loaded into our local buffer. */

  /* Now we've got to modify it! */

   /* Qaz ranges over the width of the sprite/8.
     Qay    "    "   the height.
     Qax    "    "   1 to 4 (the planes). */

  {
    adxtype& with1 = a; 

    #ifdef NOASM
    laz=xw-1; lay=with1.yl;  /* -1's only for Pascal. */
    #else
    laz=xw; lay=with1.yl+1;  /* +1's only for ASM! */
    #endif
  }

  /* ASSEMBLERISED: */
#ifdef NOASM
  for( qax=0; qax <= 3; qax ++) /* 3 */
   for( qay=0; qay <= lay; qay ++) /* 35 */
    for( qaz=0; qaz <= laz; qaz ++) /* 4 */
    {
     offs=5+qay*xw*4+xw*qax+qaz;
     aa[offs]=aa[offs] & (*sil[picnum])[qay][qaz];
    }

  for( fv=5; fv <= with.size-2; fv ++)
   aa[fv]=aa[fv] ^ (*mani[picnum])[fv];

#else
  /* OK, here's the same thing in assembler...

         AL is Qax,
         BL is Qay,
         CL is Qaz,
         DX is Offs */

 /* For the first part: */
 xwidth=xw;
 segsil=seg(*sil[picnum]);
 ofssil=ofs(*sil[picnum]);

 /* For the first and second parts: */
 segmani=seg(*mani[picnum]);  /* It's easier to do this in Pascal, and */
 ofsmani=ofs(*mani[picnum])+1;  /* besides it only goes round once here. */
 /* Segment of AA is always the current data segment. */
 ofsaa=ofs(aa)+5;
 realofsaa=ofs(aa)-1;  /* We may not need this. */
 z=with.size-7;
/*
 asm

  xor ax,ax;             { Initialise ax, bx, and cx, using a rather }
  @QAXloop: { AL }

   xor bx,bx;            { nifty speed trick. }
   @QAYloop: { BL }

    xor cx,cx;
    @QAZloop: { CL }

     { Right, well, here we are. We have to do some rather nifty array
       manipulation, stuff like that. We're trying to assemblerise:
         DX:= 5 + BX * xw * 4 + xw * AX + CX;
         aa[DX]:=aa[DX] and sil[picnum]^[BL,CL]; }

     push ax;  {AXcdx}   { OK, we're going to do some strange things }
                         { with ax, so we'd better save it. }
     mov dx,5;           { First of all... set dx to 5. }
     add dx,cx;          { DX now = 5+CX }
     mul xwidth;         { Multiply ax by xw (the Pascal variable.) }
     add dx,ax;          { DX now = 5+CX+xw*AX }

     { Right. Mul only works on ax. Don't ask me why. Ask Intel. Anyway,
     since ax is saved on the stack, we can move bx over it. Note that
     if xw was a word, using mul would have destroyed the contents of
     dx. NOT a good idea! }

     push cx;  {CXmul}   { We must just borrow cx for a second. }
     mov ax,bx;          { Make ax = bx. }
     mul xwidth;         { Do the same to it as we did to ax before. }
     mov cl,2;
     shl ax,cl;          { And multiply it by 4 (i.e., shl it by 2.) }
     add dx,ax;          { DX now = 5+CX+xw*AX+xw*BX*4. That's OK. }

     pop cx;   {CXmul}
     pop ax;   {AXcdx}   { Now we have to get ax and cx back again. }

   { Registers are now returned to original status. }

     { Righto. DX is now all worked out OK. We must now find out the
     contents of: 1) aa[dx], and 2) (harder) sil[picnum]^[BL,CL]. Gulp. }

     { DS already points to the segment of AA. So... let's use CL to
     put aa[dx] in, and use BX for the offset. Looks like we're going
     to have to push a few registers! }

     push ax; { AXaa. Saving loop value of AX. }
      { Let's use ax to do our dirty work with. }
     push dx; { Saving Offs(DX.) }

     push bx; { BXaa. Saving loop value of BX. }

     mov bx,realofsaa; { aa is now pointed to by [ds:bx]. }
     add bx,dx;        { Offset is dx bytes. }
     mov dl,[bx];      { cl now holds the contents of aa[dx]. }

     pop bx; { BXaa. Restoring loop value of BX. }

   { Stack now holds: Offs(DX). }

     { ^^^ That works. Now to find sil[picnum]^[BL,CL]. Our first task is
     to find the address of sil[picnum]^. Since it's dynamic, we must
     push and pop ds. }

     push ds; { DXaa. Saving value of Trip5's data segment. }
      { Push ds. Now we can put the segment of sil[picnum]^... }
     mov ds,segsil; { ...into ds, and... }
     mov ax,ofssil; { ...its offset into ax. }
      { Addr of sil[picnum]^ is now in [ds:ax]. BUT we want a particular
      offset: to wit, [BL,CL]. Now, siltype is defined so that this offset
      will be at an offset of (BL*11)+CL. }

     push bx; { Saving loop value of BX. }
     push cx; { Saving loop value of CX. }
     push ax; { Saving offset of sil[picnum]^. }
       { Save them for a bit (!) }
     mov al,bl;          { Put bl into al. }
     mov bl,11;          { ... }
     mul bl;             { ...times it by 11. }
     mov bx,ax;          { Put it back into bx (now = bx*11) }
     pop ax; { Restoring offset of sil[picnum]^. }
                         { Get ax back again. }
     add ax,bx;          { Add (original bl)*11 to al. }
     add ax,cx;          { Add cl to al. }
      { AX now holds the offset of sil[picnum]^[bx,cl]. }

   { Stack now holds: loop value of AX, Trip5's DS, lv of BX, lv of CX. }

     { Right. Now the address of sil[picnum]^[bl,cl] is in [ds:ax]. Let's
       get the elusive byte itself, and put it into ax. Firstly, we must
       swap ax and bx. }

     xchg ax,bx;
     mov al,[bx]; { AX now contains sil[picnum]^[bl,cl], AT LAST!! }

      { So now AL contains the sil stuff, and DL holds aa[offs]. }

     and al,dl; { aa[offs]:=aa[offs] and sil[picnum]^[Qay,Qaz]. }

     pop cx; { Restoring loop value of CX. }
     pop bx; { Restoring loop value of BX. }
     pop ds; { Restore value of Trip5's data segment. }

     { Right. AL contains the byte we need to replace aa[offs] with.
       All that's left to do is to put it back. Remember that we already
       have the segment of aa in the current DS, so... }

     pop dx; { Restore Offs(DX). }

     { Stack is now as when we entered the loop. Since this copy of DX
       is now being used for the last time, we can muck around with it. }

     { Recap: DX now holds the offset from the start of AA. If we add
       the offset of aa to it, we'll get the offset of the byte we want.
       DS is already set up. }

     push bx; { I'm just borrowing bx for a sec. I'll put it back in 5 lines.}
     mov bx,realofsaa;
     add dx,bx; { Now aa[offs] is at aa[ds:dx]. }
     mov bx,dx; { But we can't address memory with dx, so use bx. }
     mov [bx],al;   { Shove it into the memory! }
     pop bx; { See! I said I would. }

     pop ax; { Restore loop value of AX. }

     { Right, the fancy stuff's all done now. Finish off the loop code. }

     inc cl;
     cmp cl,laz;           { CL must not reach 5. Does it?  }
     jnz @QAZloop;       { no, keep going around the QAZloop. }

    inc bl;
    cmp bl,lay;           { BL must not reach 36. Does it? }
    jnz @QAYloop;        { no, keep going around the QAYloop. }

   inc al;
   cmp al,4;             { AL must not reach 4. Does it? }
   jnz @QAXloop;         { no, keep going around the QAXloop. }

  { al, bl and cl are now at their maxima, so we can stop the loops. }

  { *** SECOND ASSEMBLER BIT. *** }

  mov cx,z;             { Find the size of the array, -7. }
  mov bx,ofsmani;       { Now find the offset and put that into bx. }
  mov dx,ofsaa;         { We'll use dx to be the same as fv, +5. }

  { DS should already hold the segment of aa. }

  @nextbyte:            { Main loop... }

   { Firstly, we must get hold of aa[fv] (here called aa[dx].)}
   push bx;             { We need to "borrow" bx for a second... }
   mov bx,dx;           { Wrong register- shove it into bx. }
   mov al,[bx];         { Get aa[fv] and put it into al. }
   pop bx;              { Right, you can have bx back again. }

   { Secondly, we must get hold of mani[picnum]^[fv]. }
   push cx;             { Let's borrow cx for this one. }
   push ds;             { we must use ds to point to segmani. }
   mov ds,segmani;      { Find the segment of mani[picnum]^, }
   mov cl,[bx];         { now get the byte at [ds:bx] and put it into cl. }
   pop ds;              { Put ds back to being the current data segment. }

   { Right: now we can do what we came here for in the first place.
     AL contains aa[fv], CL contains mani[picnum]^[fv]. }

   xor al,cl;           { Xor al with bl. That's it! }

   pop cx;              { We don't need cx any more for this now. }

   push bx;             { Borrow bx... }
   mov bx,dx;           { Put dx into bx. }
   mov [bx],al;         { Put the changed al back into [ds:bx] (ie, [ds:dx].}
   pop bx;              { Get it back. }

   inc bx;              { Add one to bx, for the next char. }
   inc dx;              { And dx, for the same reason. }

  loop @nextbyte;       { Keep going round until cx=0. }
*/
{;
}
#endif

  /* Now.. let's try pasting it back again! */

  putimage(x,y,aa,0);
 }
}

void triptype::turn(byte whichway)
{
 if (whichway==8)  face=0; else face=whichway;
}

void triptype::appear(integer wx,integer wy, byte wf)
{
 x=(wx / 8)*8; y=wy; ox[cp]=wx; oy[cp]=wy; turn(wf);
 visible=true; ix=0; iy=0;
}




static boolean collision_check()
{
    byte fv;

 boolean collision_check_result;
 for( fv=1; fv <= numtr; fv ++)
  if (tr[fv].quick && (tr[fv].whichsprite!=whichsprite) &&
     ((x+a.xl)>tr[fv].x) &&
      (x<(tr[fv].x+tr[fv].a.xl)) &&
       (tr[fv].y==y)) 
       {
        collision_check_result=true;
        return collision_check_result;
       }
 collision_check_result=false;
 return collision_check_result;
}

void triptype::walk()
{
    byte tc; bytefield r;


 if (visible) 
 {
  {
   x1=(x / 8)-1;
   if (x1==255)  x1=0;
   y1=y-2;
   x2=((x+a.xl) / 8)+1;
   y2=y+a.yl+2;
  }
  getset[1-cp].remember(r);
 }

 if (~ doing_sprite_run) 
 {
  ox[cp]=x; oy[cp]=y;
  if (homing)  homestep();
  x=x+ix; y=y+iy;
 }

  if (check_me) 
  {
   if (collision_check()) 
   {
    bounce();
    return;
   }

   tc=checkfeet(x,x+a.xl,oy[cp],y,a.yl);

   if ((tc!=0) & (~ doing_sprite_run)) 
    { void& with = magics[tc]; 
     switch (op) {
      case exclaim: {
                bounce(); mustexclaim=true; saywhat=data;
               }
               break;
      case bounces: bounce(); break;
      case transport: fliproom(hi(data),lo(data)); break;
      case unfinished: {
                   bounce();
                   display("\7Sorry.\3\rThis place is not available yet!");
                  }
                  break;
      case special: call_special(data); break;
      case mopendoor: open_the_door(hi(data),lo(data),tc); break;
     }}
  }

 if (~ doing_sprite_run) 
 {
  count += 1;
  if (((ix!=0) || (iy!=0)) && (count>1)) 
  {
   step += 1; if (step==a.seq)  step=0; count=0;
  }
 }

}

void triptype::bounce()
{
 x=ox[cp]; y=oy[cp];
 if (check_me)  stopwalking(); else stopwalk();
 oncandopageswap=false;
 showrw;
 oncandopageswap=true;
}

shortint sgn(integer x)
{
 shortint sgn_result;
 if (x>0)  sgn_result=1; else
  if (x<0)  sgn_result=-1; else
   sgn_result=0; /* x=0 */
 return sgn_result;
}

void triptype::walkto(byte pednum)
{
 speed(sgn(peds[pednum].x-x)*4,sgn(peds[pednum].y-y));
 hx=peds[pednum].x-a.xl / 2;
 hy=peds[pednum].y-a.yl; homing=true;
}

void triptype::stophoming()
{
 homing=false;
}

void triptype::homestep()
{
    integer temp;

 if ((hx==x) && (hy==y)) 
 {     /* touching the target */
  stopwalk();
  return;
 }
 ix=0; iy=0;
 if (hy!=y) 
 {
  temp=hy-y; if (temp>4)  iy=4; else if (temp<-4)  iy=-4; else iy=temp;
 }
 if (hx!=x) 
 {
  temp=hx-x; if (temp>4)  ix=4; else if (temp<-4)  ix=-4; else ix=temp;
 }
}

void triptype::speed(shortint xx,shortint yy)
{
 ix=xx; iy=yy;
 if ((ix==0) && (iy==0))  return; /* no movement */
 if (ix==0) 
 {     /* No horz movement */
  if (iy<0)  turn(up); else turn(down);
 } else
 {
  if (ix<0)  turn(left); else turn(right);
 }
}

void triptype::stopwalk()
{
 ix=0; iy=0; homing=false;
}

void triptype::chatter()
{
 talkx=x+a.xl / 2; talky=y; talkf=a.fgc; talkb=a.bgc;
}

void triptype::set_up_saver(trip_saver_type& v)
{
 v.whichsprite=whichsprite;
 v.face=face;
 v.step=step;
 v.x=x;
 v.y=y;
 v.ix=ix;
 v.iy=iy;
 v.visible=visible;
 v.homing=homing;
 v.check_me=check_me;
 v.count=count;
 v.xw=xw;
 v.xs=xs;
 v.ys=ys;
 v.totalnum=totalnum;
 v.hx=hx;
 v.hy=hy;
 v.call_eachstep=call_eachstep;
 v.eachstep=eachstep;
 v.vanishifstill=vanishifstill;
}

void triptype::unload_saver(trip_saver_type v)
{
 whichsprite=v.whichsprite;
 face=v.face;
 step=v.step;
 x=v.x;
 y=v.y;
 ix=v.ix;
 iy=v.iy;
 visible=v.visible;
 homing=v.homing;
 check_me=v.check_me;
 count=v.count;
 xw=v.xw;
 xs=v.xs;
 ys=v.ys;
 totalnum=v.totalnum;
 hx=v.hx;
 hy=v.hy;
 call_eachstep=v.call_eachstep;
 eachstep=v.eachstep;
 vanishifstill=v.vanishifstill;
}

void triptype::savedata(untyped_file& f)
{
 trip_saver_type tripsaver;

 set_up_saver(tripsaver);

 {
  blockwrite(f,tripsaver.whichsprite,1);
  blockwrite(f,tripsaver.face,1); blockwrite(f,tripsaver.step,1);
  blockwrite(f,tripsaver.x,2);    blockwrite(f,tripsaver.y,2);
  blockwrite(f,tripsaver.ix,1);   blockwrite(f,tripsaver.iy,1);
  blockwrite(f,tripsaver.visible,1);
  blockwrite(f,tripsaver.homing,1);
  blockwrite(f,tripsaver.check_me,1);
  blockwrite(f,tripsaver.count,1);
  blockwrite(f,tripsaver.xw,1);
  blockwrite(f,tripsaver.xs,1); blockwrite(f,tripsaver.ys,1);
  blockwrite(f,tripsaver.totalnum,1);
  blockwrite(f,tripsaver.hx,2); blockwrite(f,tripsaver.hy,2);
  blockwrite(f,tripsaver.call_eachstep,1);
  blockwrite(f,tripsaver.eachstep,1);
  blockwrite(f,tripsaver.vanishifstill,1);
 }
}

void triptype::loaddata(untyped_file& f)
{
 word spritewas,spriteis,saveface,savex,savey,savestep;
 boolean wasquick;
 trip_saver_type tripsaver;


 wasquick=quick;
 quick=true; spritewas=whichsprite;

 {
  blockread(f,tripsaver.whichsprite,1);
  blockread(f,tripsaver.face,1); blockread(f,tripsaver.step,1);
  blockread(f,tripsaver.x,2);    blockread(f,tripsaver.y,2);
  blockread(f,tripsaver.ix,1);   blockread(f,tripsaver.iy,1);

  if ((! wasquick) || ((unsigned char)tripsaver.whichsprite!=spritewas)) 
  {
   spriteis=tripsaver.whichsprite;
   savex=tripsaver.x; savey=tripsaver.y; saveface=tripsaver.face; savestep=tripsaver.step;

   if (wasquick)  done();

   init(spriteis,tripsaver.check_me);

   appear(savex,savey,saveface); tripsaver.step=savestep;
  }

  blockread(f,tripsaver.visible,1);
  blockread(f,tripsaver.homing,1);
  blockread(f,tripsaver.check_me,1);
  blockread(f,tripsaver.count,1);
  blockread(f,tripsaver.xw,1);
  blockread(f,tripsaver.xs,1); blockread(f,tripsaver.ys,1);
  blockread(f,tripsaver.totalnum,1);
  blockread(f,tripsaver.hx,2); blockread(f,tripsaver.hy,2);
  blockread(f,tripsaver.call_eachstep,1);
  blockread(f,tripsaver.eachstep,1);
  blockread(f,tripsaver.vanishifstill,1);
 }

 unload_saver(tripsaver);
}

triptype* triptype::done()
{
 integer gd,gm; varying_string<2> xx;
 byte fv/*,nds*/;
 byte aa,bb;
 longint id; word soa;

 {
  adxtype& with = a; 

/*  nds:=num div seq;*/
  xw=with.xl / 8; if ((with.xl % 8)>0)  xw += 1;
  for( aa=1; aa <= /*nds*seq*/ with.num; aa ++)
  {
   totalnum -= 1;
   freemem(mani[totalnum],with.size-6);
   freemem(sil[totalnum],11*(with.yl+1));  /* <<- Width of a siltype. */
  }
 }

 quick=false; whichsprite=177;
 return this;
}

getsettype* getsettype::init()
{
 numleft=0;  /* initialise array pointer */
 return this;
}

void getsettype::remember(bytefield r)
{
 numleft += 1;
 if (numleft>maxgetset)  runerror(runerr_getset_overflow);
 gs[numleft]=r;
}

void getsettype::recall(bytefield& r)
{
 r=gs[numleft];
 numleft -= 1;
}

void rwsp(byte t,byte r)
{
 { triptype& with = tr[t];  switch (r) {
      case up: speed(  0,-with.ys); break; case down: speed(  0, with.ys); break; case left: speed(-with.xs,  0); break;
   case right: speed( with.xs,  0); break;   case ul: speed(-with.xs,-with.ys); break;   case ur: speed( with.xs,-with.ys); break;
      case dl: speed(-with.xs, with.ys); break;   case dr: speed( with.xs, with.ys); break;
  }}
}

void apped(byte trn,byte np)
{
 { triptype& with = tr[trn]; 
  {
   void& with1 = peds[np]; 

   appear(with.x-with.a.xl / 2,with.y-with.a.yl,dir);
   rwsp(trn,dir);
  }}
}

void getback();

static bytefield r;


/*   function overlap(x1,y1,x2,y2,x3,y3,x4,y4:word):boolean;
   begin { By De Morgan's law: }
    overlap:=(x2>=x3) and (x4>=x1) and (y2>=y3) and (y4>=y1);
   end;*/
   /* x1,x2 - as bytefield, but *8. y1,y2 - as bytefield.
     x3,y3 = mx,my. x4,y4 = mx+16,my+16. */
static boolean overlaps_with_mouse()
{
  boolean overlaps_with_mouse_result;
  overlaps_with_mouse_result=
   (x2*8>=mx) && (mx+16>=x1*8) && (y2>=my) && (my+16>=y1);
  return overlaps_with_mouse_result;
}

void getback()
{
 byte fv;
 boolean endangered;


 endangered=false;
/* Super_Off;*/

  { getsettype& with = getset[1-cp]; 
   while (with.numleft>0) 
   {
    recall(r);

/*    if overlaps_with_mouse and not endangered then
    begin
     endangered:=true;
     blitfix;
     Super_Off;
    end;*/

    mblit(x1,y1,x2,y2,3,1-cp);
   }}

 blitfix;
 /*if endangered then*/ /*Super_On;*/
}

/* Eachstep procedures: */
void follow_avvy_y(byte tripnum)
{
 {
  triptype& with = tr[tripnum]; 

  if (tr[1].face==left)  return;
  if (with.homing)  with.hy=tr[1].y; else
  {
   if (with.y<tr[1].y) 
    with.y += 1; else
     if (with.y>tr[1].y) 
      with.y -= 1; else
       return;
   if (with.ix==0)  { with.step += 1; if (with.step==with.a.seq)  with.step=0; with.count=0; }
  }
 }
}

void back_and_forth(byte tripnum)
{
 { triptype& with = tr[tripnum]; 
  if (! with.homing) 
  {
   if (with.face==right)  walkto(4); else walkto(5);
  }}
}

void face_avvy(byte tripnum)
{
 { triptype& with = tr[tripnum]; 
  if (! with.homing) 
  {
   if (tr[1].x>=with.x)  with.face=right;
    else with.face=left;
  }}
}

void arrow_procs(byte tripnum)
{
    byte fv;

 { triptype& with = tr[tripnum]; 
  if (with.homing) 
  {     /* Arrow is still in flight. */
   /* We must check whether or not the arrow has collided with Avvy's head.
     This is so if: a) the bottom of the arrow is below Avvy's head,
      b) the left of the arrow is left of the right of Avvy's head, and
      c) the right of the arrow is right of the left of Avvy's head. */
   if (((with.y+with.a.yl)>=tr[1].y) /* A */
    && (with.x<=(tr[1].x+tr[1].a.xl)) /* B */
     && ((with.x+with.a.xl)>=tr[1].x)) /* C */
         {     /* OK, it's hit him... what now? */

     tr[2].call_eachstep=false;  /* prevent recursion. */
     dixi('Q',47); /* Complaint! */
     done(); /* Deallocate the arrow. */
/*     tr[1].done; { Deallocate normal pic of Avvy. }

     off;
     for fv:=0 to 1 do
     begin
      cp:=1-cp;
      getback;
     end;
     on;*/

     gameover;

     dna.user_moves_avvy=false;  /* Stop the user from moving him. */
     set_up_timer(55,procnaughty_duke,reason_naughty_duke);
    }
  } else   /* Arrow has hit the wall! */
  {
   done(); /* Deallocate the arrow. */
   show_one(3); /* Show pic of arrow stuck into the door. */
   dna.arrow_in_the_door=true;  /* So that we can pick it up. */
  }}
}

/*procedure Spludwick_procs(tripnum:byte);
var fv:byte;
begin
 with tr[tripnum] do
  if not homing then { We only need to do anything if Spludwick *stops*
                       walking. }
  with dna do
   begin
    inc(DogfoodPos);
    if DogfoodPos=8 then DogfoodPos:=1;
    walkto(DogfoodPos);
   end;
end;*/

void grab_avvy(byte tripnum)       /* For Friar Tuck, in Nottingham. */
{
    byte fv; integer tox,toy;

 { triptype& with = tr[tripnum]; 
   {
    tox=tr[1].x + 17;
    toy=tr[1].y - 1;
    if ((with.x==tox) && (with.y==toy)) 
    {
     with.call_eachstep=false;
     with.face=left;
     stopwalk();
     /* ... whatever ... */
    } else
    {     /* Still some way to go. */
     if (with.x<tox) 
     {
      with.x += 5;
      if (with.x>tox)  with.x=tox;
     }
     if (with.y<toy)  with.y += 1;
     with.step += 1; if (with.step==with.a.seq)  with.step=0;
    }
   }}
}

void geida_procs(byte tripnum);

static void take_a_step(byte& tripnum)
{
 { triptype& with = tr[tripnum]; 
 if (with.ix==0) 
  { with.step += 1; if (with.step==with.a.seq)  with.step=0; with.count=0; }}
}



static void spin(byte whichway, byte& tripnum)
{
 { triptype& with = tr[tripnum]; 
 if (with.face!=whichway) 
 {
  with.face=whichway;
  if (with.whichsprite==2)  return; /* Not for Spludwick */

  dna.geida_spin += 1;
  dna.geida_time=20;
  if (dna.geida_spin==5) 
  {
   display("Steady on, Avvy, you'll make the poor girl dizzy!");
   dna.geida_spin=0; dna.geida_time=0;   /* knock out records */
  }
 }}
}

void geida_procs(byte tripnum)
{
 {
  triptype& with = tr[tripnum]; 

  if (dna.geida_time>0) 
  {
   dna.geida_time -= 1;
   if (dna.geida_time==0)  dna.geida_spin=0;
  }

  if (with.y<(tr[1].y-2)) 
  {     /* Geida is further from the screen than Avvy. */
   spin(down, tripnum);
   with.iy=1; with.ix=0;
   take_a_step(tripnum);
   return;
  } else
   if (with.y>(tr[1].y+2)) 
   {     /* Avvy is further from the screen than Geida. */
    spin(up, tripnum);
    with.iy=-1; with.ix=0;
    take_a_step(tripnum);
    return;
   }

  with.iy=0;
  if (with.x<tr[1].x-tr[1].xs*8) 
  {
   with.ix=tr[1].xs;
   spin(right, tripnum);
   take_a_step(tripnum);
  } else
    if (with.x>tr[1].x+tr[1].xs*8) 
    {
     with.ix=-tr[1].xs;
     spin(left, tripnum);
     take_a_step(tripnum);
    } else with.ix=0;
 }
}

/* That's all... */

void call_andexors()
{
 array<1,5,byte> order;
 byte fv,temp;
 boolean ok;

 fillchar(order,5,'\0');
 for( fv=1; fv <= numtr; fv ++) { triptype& with = tr[fv]; 
  if (with.quick && with.visible) 
   order[fv]=fv;}

 do {
  ok=true;
  for( fv=1; fv <= 4; fv ++)
   if (((order[fv]!=0) && (order[fv+1]!=0))
    && (tr[order[fv]].y>tr[order[fv+1]].y)) 
   {     /* Swap them! */
    temp=order[fv];
    order[fv]=order[fv+1];
    order[fv+1]=temp;
    ok=false;
   }
 } while (!ok);

 for( fv=1; fv <= 5; fv ++)
  if (order[fv]>0) 
   tr[order[fv]].andexor();
}

void trippancy_link()
{
 byte fv;

 if (ddmnow | ontoolbar | seescroll)  return;
 for( fv=1; fv <= numtr; fv ++) { triptype& with = tr[fv];  if (with.quick)  walk();}
 call_andexors();
 for( fv=1; fv <= numtr; fv ++) { triptype& with = tr[fv]; 
  if (with.quick && with.call_eachstep) 
  {
   switch (tr[fv].eachstep) {
    case procfollow_avvy_y : follow_avvy_y(fv); break;
    case procback_and_forth : back_and_forth(fv); break;
    case procface_avvy : face_avvy(fv); break;
    case procarrow_procs : arrow_procs(fv); break;
/*    PROCSpludwick_procs : spludwick_procs(fv);*/
    case procgrab_avvy : grab_avvy(fv); break;
    case procgeida_procs : geida_procs(fv); break;
   }
  }}
 if (mustexclaim) 
 {
  mustexclaim=false;
  dixi('x',saywhat);
 }
}

void get_back_loretta()
{
    byte fv;

/* for fv:=1 to numtr do with tr[fv] do if quick then getback;*/
 for( fv=1; fv <= numtr; fv ++) if (tr[fv].quick) 
 {
  getback();
  return;
 }
/* for fv:=0 to 1 do begin cp:=1-cp; getback; end;*/
}

void stopwalking()
{
 tr[1].stopwalk(); dna.rw=stopped; if (alive)  tr[1].step=1;
}

void tripkey(char dir)
{
 if ((ctrl==cjoy) | (~ dna.user_moves_avvy))  return;

 { triptype& with = tr[1]; 
  {
   switch (dir) {
    case 'H': if (rw!=up)    
            { rw=up;    rwsp(1,rw); } else stopwalking();
            break;
    case 'P': if (rw!=down)  
            { rw=down;  rwsp(1,rw); } else stopwalking();
            break;
    case 'K': if (rw!=left)  
            { rw=left;  rwsp(1,rw); } else stopwalking();
            break;
    case 'M': if (rw!=right) 
            { rw=right; rwsp(1,rw); } else stopwalking();
            break;
    case 'I': if (rw!=ur)    
            { rw=ur;    rwsp(1,rw); } else stopwalking();
            break;
    case 'Q': if (rw!=dr)    
            { rw=dr;    rwsp(1,rw); } else stopwalking();
            break;
    case 'O': if (rw!=dl)    
            { rw=dl;    rwsp(1,rw); } else stopwalking();
            break;
    case 'G': if (rw!=ul)    
            { rw=ul;    rwsp(1,rw); } else stopwalking();
            break;
    case 'L': stopwalking(); break;
   }
 }}
}

void readstick()
{
    byte jw;

 if (ctrl==ckey)  return;

 jw=joyway;

 {
  triptype& with = tr[1]; 

  if (jw==stopped)  stopwalking(); else
  {
   dna.rw=jw; rwsp(1,dna.rw);
  }
 }

 if (jw!=oldjw) 
 {
  showrw;
  oldjw=jw;
 }

}

void getsetclear()
{
    byte fv;

 for( fv=0; fv <= 1; fv ++) getset[fv].init();
}

void hide_in_the_cupboard()
{
      const char nowt = '\372'; /* As in Acci. */

 {
  if (avvys_in_the_cupboard) 
  {
   if (wearing==nowt) 
    display("\6AVVY!\22 Get dressed first!");
   else
   {
    tr[1].visible=true;
    user_moves_avvy=true;
    apped(1,3); /* Walk out of the cupboard. */
    display("You leave the cupboard. Nice to be out of there!");
    avvys_in_the_cupboard=false;
    first_show(8); then_show(7); start_to_close;
   }
  } else
  {     /* Not hiding in the cupboard */
   tr[1].visible=false;
   user_moves_avvy=false;
   display(string("You walk into the room...\20It seems to be an empty, ")+
    "but dusty, cupboard. Hmmmm... you leave the door slightly open to "+
    "avoid suffocation.");
   avvys_in_the_cupboard=true;
   show_one(8);
  }
 }
}

void fliproom(byte room,byte ped);
static integer beforex,beforey;
static void tidy_after_mouse();


static void tidy_up(integer a,integer b,integer c,integer d)
{
    bytefield bf;

   {
      x1=a / 8;
      y1=b;
      x2=(c+7) / 8;
      y2=d;
      setactivepage(0);
      rectangle(x1*8,y1,x2*8+7,y2);
   }
   getset[0].remember(bf);
   getset[1].remember(bf);
}



static void tidy_after_mouse()
{
   tidy_up(beforex,beforey,beforex+15,beforey+15);
   xycheck;
   tidy_up(mx,my,mx+15,my+15);
}

void fliproom(byte room,byte ped)
{
    byte fv;


 if (~ alive) 
 {     /* You can't leave the room if you're dead. */
   tr[1].ix=0; tr[1].iy=0;   /* Stop him from moving. */
   return;
 }

 if ((ped==177) && (dna.room==r__lusties)) 
 {
  hide_in_the_cupboard();
  return;
 }

 if ((dna.jumpstatus>0) && (dna.room==r__insidecardiffcastle)) 
 {     /* You can't *jump* out of Cardiff Castle! */
  tr[1].ix=0;
  return;
 }

 xycheck; beforex=mx; beforey=my;

 exitroom(dna.room);
 dusk; getsetclear();


 for( fv=2; fv <= numtr; fv ++)
  { triptype& with = tr[fv]; 
  if (with.quick)  done();} /* Deallocate sprite */

 if (dna.room==r__lustiesroom) 
  dna.enter_catacombs_from_lusties_room=true;

 enterroom(room,ped); apped(1,ped);
 dna.enter_catacombs_from_lusties_room=false;
 oldrw=dna.rw; dna.rw=tr[1].face; showrw;

 for( fv=0; fv <= 1; fv ++)
 {
  cp=1-cp;
  getback();
 }
 dawn;

 /* Tidy up after mouse. I know it's a kludge... */
/*  tidy_after_mouse;*/
}

boolean infield(byte which)
 /* returns True if you're within field "which" */
{
    integer yy;

 boolean infield_result;
 { void& with = fields[which];  {
  triptype& with = tr[1]; 

  yy=with.y+with.a.yl;
  infield_result=(with.x>=x1) && (with.x<=x2) && (yy>=y1) && (yy<=y2);
                                }}
 return infield_result;
}

boolean neardoor()         /* returns True if you're near a door! */
{
    integer ux,uy; byte fv; boolean nd;

 boolean neardoor_result;
 if (numfields<9) 
 {     /* there ARE no doors here! */
  neardoor_result=false;
  return neardoor_result;
 }
 {
  triptype& with = tr[1]; 

  ux=with.x;
  uy=with.y+with.a.yl;
 } nd=false;
 for( fv=9; fv <= numfields; fv ++)
  {
   void& with = fields[fv]; 

   if ((ux>=x1) && (ux<=x2) && (uy>=y1) && (uy<=y2))  nd=true;
  }
 neardoor_result=nd;
 return neardoor_result;
}

void new_game_for_trippancy()     /* Called by gyro.newgame */
{
 tr[1].visible=false;
}

void triptype::save_data_to_mem(word& where)
{
 trip_saver_type tripsaver;

 set_up_saver(tripsaver);
 move(tripsaver,mem[storage_seg*where],sizeof(tripsaver));
 where += sizeof(tripsaver);
}

void triptype::load_data_from_mem(word& where)
{
 word spritewas,spriteis,saveface,savex,savey,savestep;
 boolean wasquick;
 trip_saver_type tripsaver;

 move(mem[storage_seg*where],tripsaver,sizeof(tripsaver));
 where += sizeof(tripsaver);
 unload_saver(tripsaver);

 spriteis=whichsprite;
 savex=x; savey=y; saveface=face; savestep=step;

 init(spriteis,check_me);

 appear(savex,savey,saveface); unload_saver(tripsaver);
 step=savestep;
}

class unit_trip5_initialize {
  public: unit_trip5_initialize();
};
static unit_trip5_initialize trip5_constructor;

unit_trip5_initialize::unit_trip5_initialize() {
 getsetclear(); mustexclaim=false;
}
