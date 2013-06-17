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

#ifndef __trip5_h__
#define __trip5_h__


#include "graph.h"
/*#include "Crt.h"*/
/*#include "Gyro.h"*/
/*#include "Sticks.h"*/


const integer maxgetset = 35;

typedef array<5,2053,byte> manitype;

typedef matrix<0,50,0,10,byte> siltype; /* 35, 4 */

struct adxtype { /* Second revision of ADX type */
           varying_string<12> name; /* name of character */
           varying_string<16> comment; /* comment */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
           byte accinum; /* the number according to Acci (1=Avvy, etc.) */
};

struct trip_saver_type {
                   byte whichsprite;
                   byte face; byte step;
                   integer x;    integer y;
                   shortint ix;   shortint iy;
                   boolean visible;
                   boolean homing;
                   boolean check_me;
                   byte count;
                   byte xw,xs,ys;
                   byte totalnum;
                   integer hx; integer hy;
                   boolean call_eachstep;
                   byte eachstep;
                   boolean vanishifstill;
};

class triptype {
public:
            adxtype a; /* vital statistics */
            byte face,step;
            integer x,y; /* current xy coords */
            array<0,1,integer> ox,oy;     /* last xy coords */
            shortint ix,iy; /* amount to move sprite by, each step */
            array<1,24,manitype*> mani;
            array<1,24,siltype*> sil;
            byte whichsprite;
            boolean quick,visible,homing,check_me;
            integer hx,hy; /* homing x & y coords */
            byte count; /* counts before changing step */
            byte xw; /* x-width in bytes */
            byte xs,ys; /* x & y speed */
            byte totalnum; /* total number of sprites */
            boolean vanishifstill; /* Do we show this sprite if it's still? */

            boolean call_eachstep; /* Do we call the eachstep procedure? */
            byte eachstep;

            triptype* init(byte spritenum, boolean do_check);
             /* loads & sets up the sprite */
            void original();    /* just sets Quick to false */
            void andexor();    /* drops sprite onto screen */
            void turn(byte whichway);      /* turns him round */
            void appear(integer wx,integer wy, byte wf); /* switches him on */
            void bounce();    /* bounces off walls. */
            void walk();    /* prepares for andexor, etc. */
            void walkto(byte pednum);      /* home in on a point */
            void stophoming();    /* self-explanatory */
            void homestep();    /* calculates ix & iy for one homing step */
            void speed(shortint xx,shortint yy); /* sets ix & iy, non-homing, etc */
            void stopwalk();    /* Stops the sprite from moving */
            void chatter();    /* Sets up talk vars */
            void set_up_saver(trip_saver_type& v);
            void unload_saver(trip_saver_type v);
              void savedata(untyped_file& f); /* Self-explanatory, */
              void loaddata(untyped_file& f);  /* really. */
              void save_data_to_mem(word& where);
              void load_data_from_mem(word& where);
            triptype* done();
};

class getsettype {
public:
              array<1,maxgetset,bytefield> gs;
              byte numleft;

              getsettype* init();
              void remember(bytefield r);
              void recall(bytefield& r);
};


const integer up = 0;
const integer right = 1;
const integer down = 2;
const integer left = 3;
const integer ur = 4; const integer dr = 5; const integer dl = 6; const integer ul = 7;
const integer stopped = 8;

const integer numtr = 5; /* current max no. of sprites */



const integer procfollow_avvy_y = 1;

const integer procback_and_forth = 2;

const integer procface_avvy = 3;

const integer procarrow_procs = 4;

const integer procspludwick_procs = 5;

const integer procgrab_avvy = 6;

const integer procgeida_procs = 7;


void trippancy_link();

void get_back_loretta();

void loadtrip();

void call_special(word which);

void open_the_door(byte whither,byte ped,byte magicnum); /* Handles slidey-open doors. */

void catamove(byte ped);

void stopwalking();

void tripkey(char dir);

void rwsp(byte t,byte r);

void apped(byte trn,byte np);

void getback();

void fliproom(byte room,byte ped);

boolean infield(byte which);          /* returns True if you're within field "which" */

boolean neardoor();        /* returns True if you're near a door! */

void readstick();

void newspeed();

void new_game_for_trippancy();


#ifdef __trip5_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<1,numtr,triptype> tr;
EXTERN array<0,1,getsettype> getset;
EXTERN array<1,16000,byte> aa;

EXTERN boolean mustexclaim; EXTERN word saywhat;
#undef EXTERN
#define EXTERN extern


#endif
