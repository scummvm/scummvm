/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 VISA             The new Sez handler. (Replaces Access.) */

#define __visa_implementation__


#include "visa.h"


/*#include "Gyro.h"*/
/*#include "Scrolls.h"*/
/*#include "Acci.h"*/
/*#include "Trip5.h"*/
/*#include "Lucerna.h"*/


const boolean bubbling = false;
const boolean report_dixi_errors = true;

boolean went_ok;

void unskrimble()
{
    word fv;

 for( fv=1; fv <= bufsize; fv ++) buffer[fv]=(char)((~(ord(buffer[fv])-fv)) % 256);
}

void do_the_bubble()
{
 bufsize += 1;
 buffer[bufsize]='\2';
}

void dixi(char block, byte point)
{
 untyped_file indexfile,sezfile;
 word idx_offset,sez_offset;
 boolean error;

 error=false;

 assign(indexfile,"avalot.idx"); assign(sezfile,"avalot.sez");

 reset(indexfile,1);
 seek(indexfile,(ord(upcase(block))-65)*2);
 blockread(indexfile,idx_offset,2);
 if (idx_offset==0)  error=true;
 seek(indexfile,idx_offset+point*2);
 blockread(indexfile,sez_offset,2);
 if (sez_offset==0)  error=true;
 close(indexfile);

 went_ok=! error;

 if (error) 
 {
  if (report_dixi_errors) 
   display(string('\7')+"Error accessing scroll "+block+strf(point));
  return;
 }

 reset(sezfile,1);
 seek(sezfile,sez_offset);
 blockread(sezfile,bufsize,2);
 blockread(sezfile,buffer,bufsize);
 close(sezfile);
 unskrimble();

 if (bubbling)  do_the_bubble();

 calldrivers;
}

void speech(byte who, byte subject)
{
 untyped_file indexfile,sezfile;
 word idx_offset,sez_offset,next_idx_offset;

 if (subject==0) 
 {     /* No subject. */
  bubbling=true; report_dixi_errors=false;
  dixi('s',who);
  bubbling=false; report_dixi_errors=true;
 } else
 {     /* Subject given. */
  assign(indexfile,"converse.avd"); assign(sezfile,"avalot.sez");

  went_ok=false;  /* Assume that until we know otherwise. */
  reset(indexfile,1);
  seek(indexfile,who*2-2);
  blockread(indexfile,idx_offset,2);
  blockread(indexfile,next_idx_offset,2);

  if ((idx_offset==0) ||
    ((((next_idx_offset-idx_offset) / 2)-1) < subject))  return;

  seek(indexfile,idx_offset+subject*2);
  /*$I-*/
  blockread(indexfile,sez_offset,2);
  if ((sez_offset==0) || (ioresult!=0))  return;
  /*$I+*/
  close(indexfile);

  reset(sezfile,1);
  seek(sezfile,sez_offset);
  blockread(sezfile,bufsize,2);
  blockread(sezfile,buffer,bufsize);
  close(sezfile);

  unskrimble();
  do_the_bubble();

  calldrivers;
  went_ok=true;
 }
}

void talkto(byte whom)
{
 byte fv;
 boolean no_matches;

 if (person==pardon) 
 {
  person=chr(subjnumber);
  subjnumber=0;
 }

 if (subjnumber==0) 
 switch (chr(whom)) {
  case pspludwick:

    if ((dna.lustie_is_asleep) & (~ dna.obj[potion])) 
    {
     dixi('q',68);
     dna.obj[potion]=true;
     objectlist; points(3); return;
    } else
    {
     if (dna.talked_to_crapulus) 
      switch (dna.given2spludwick) { /* Spludwick - what does he need? */
         /* 0 - let it through to use normal routine. */
         case RANGE_2(1,2): {
                display(string("Can you get me ")+
                 get_better(spludwick_order[dna.given2spludwick])+", please?"+
                  "\232\2");
                return;
               }
               break;
         case 3: {
             dixi('q',30); /* need any help with the game? */
             return;
            }
            break;
       }
     else {
           dixi('q',42); /* Haven't talked to Crapulus. Go and talk to him. */
           return;
          }
    }
    break;

  case pibythneth: if (dna.givenbadgetoiby) 
              {
               dixi('q',33); /* Thanks a lot! */
               return; /* And leave the proc. */
              }
              break; /* Or... just continue, 'cos he hasn't got it. */
  case pdogfood: if (dna.wonnim) 
            {     /* We've won the game. */
             dixi('q',6); /* "I'm Not Playing!" */
             return; /* Zap back. */
            } else dna.asked_dogfood_about_nim=true;
            break;
  case payles: if (~ dna.ayles_is_awake) 
          {
           dixi('q',43); /* He's fast asleep! */
           return;
          } else
           if (~ dna.given_pen_to_ayles) 
           {
            dixi('q',44); /* Can you get me a pen, Avvy? */
            return;
           }
           break;

  case pjacques: { dixi('q',43); return; } break;
  case pgeida: if (dna.geida_given_potion) 
           dna.geida_follows=true; else
          {
           dixi('u',17);
           return;
          }
          break;
  case pspurge: if (~ dna.sitting_in_pub) 
           {
            dixi('q',71); /* Try going over and sitting down. */
            return;
           } else
            {
             if (spurge_talk<5)  spurge_talk += 1;
             if (spurge_talk>1) 
             {     /* no. 1 falls through */
              dixi('q',70+spurge_talk);
              return;
             }
            }
            break;
 } else    /* On a subject. Is there any reason to block it? */
   switch (chr(whom)) {
     case payles: if (~ dna.ayles_is_awake) 
          {
           dixi('q',43); /* He's fast asleep! */
           return;
          }
          break;
   }

 if (whom>149)  whom -= 149;

 no_matches=true;
 for( fv=1; fv <= numtr; fv ++)
  if (tr[fv].a.accinum==whom) 
  {
   display(string('\23')+chr(fv+48)+'\4');
   no_matches=false;
   flush();
  }

 if (no_matches)  display("\23\23\4");

 speech(whom,subjnumber);
 if (! went_ok)      /* File not found! */
  dixi('n',whom);

 if (subjnumber==0) 
 switch (chr(whom+149)) {
  case pcrapulus:
     {     /* Crapulus: get the badge - first time only */
      dna.obj[badge]=true;
      objectlist;
      dixi('q',1); /* Circular from Cardiff. */
      dna.talked_to_crapulus=true;

      whereis[pcrapulus]=177;  /* Crapulus walks off. */

      tr[2].vanishifstill=true;
      tr[2].walkto(4); /* Walks away. */

      points(2);
     }
     break;

 }
}


