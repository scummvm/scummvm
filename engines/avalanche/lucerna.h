/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __lucerna_h__
#define __lucerna_h__


#include "gyro.h"


 void callverb(char n);

 void draw_also_lines();

 void mouse_init();

 void mousepage(word page_);

 void load(byte n);

 void exitroom(byte x);

 void enterroom(byte x,byte ped);

 void thinkabout(char z, boolean th);      /* Hey!!! Get it and put it!!! */

 void load_digits();    /* Load the scoring digits & rwlites */

 void toolbar();

 void showscore();

 void points(byte num);      /* Add on no. of points */

 void mouseway();

 void inkey();

 void posxy();

 void fxtoggle();

 void objectlist();

 void checkclick();

 void errorled();

 void dusk();

 void dawn();

 void showrw();

 void mblit(byte x1,byte y1,byte x2,byte y2, byte f,byte t); /* The Minstrel Blitter */

 void blitfix();

 void clock();

 void flip_page();

 void delavvy();

 void gameover();

 void minor_redraw();

 void major_redraw();

 word bearing(byte whichped);

 void flesh_colours();

 void sprite_run();

 void fix_flashers();

#endif
