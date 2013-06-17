/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 STICKS           The joystick handler. */

#define __sticks_implementation__


#include "sticks.h"


enum lmc {l,m,r, last_lmc}; /* Left, middle & right */

file<joysetup> jf;

byte joyway();


static lmc getlmc(word n,word max,word min)
{lmc getlmc_result;
;
 if (n<min)  getlmc_result=l; else
  if (n>max)  getlmc_result=r; else
   getlmc_result=m;
return getlmc_result;
}

byte joyway()
{
 word x,y;
 lmc xx,yy;
byte joyway_result;
;
 if (~ dna.user_moves_avvy)  return joyway_result;
 if (use_joy_a)  readjoya(x,y); else readjoyb(x,y);

 {;
  xx=getlmc(x,cxmax,cxmin);
  yy=getlmc(y,cymax,cymin);
 }

 switch (xx) {
  case l: switch (yy) {
      case l: joyway_result=ul; break;
      case m: joyway_result=left; break;
      case r: joyway_result=dl; break;
     }
     break;
  case m: switch (yy) {
      case l: joyway_result=up; break;
      case m: joyway_result=stopped; break;
      case r: joyway_result=down; break;
     }
     break;
  case r: switch (yy) {
      case l: joyway_result=ur; break;
      case m: joyway_result=right; break;
      case r: joyway_result=dr; break;
     }
     break;
 }
return joyway_result;
}

void joykeys()
 /* The 2 joystick keys may be reprogrammed. This parses them. */
{
    byte v;
;
 if (use_joy_a) 
 {
   v=(byte)(buttona1); v += (byte)(buttona2)*2;
 } else
 {
   v=(byte)(buttonb1); v += (byte)(buttonb2)*2;
 }

 switch (v) {
  case 0: return; break; /* No buttons pressed. */
  case 1: opendoor; break; /* Button 1 pressed: open door. */
/*  2: blip; { Button 2 pressed: nothing (joylock?). }*/
  case 2:case 3: lookaround; break; /* Both buttons pressed: look around. */
 }
}

     /* No init code. */