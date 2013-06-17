/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __closing_h__
#define __closing_h__


const integer scr_bugalert = 1;
const integer scr_ramcram = 2;
const integer scr_nagscreen = 3;
const integer scr_twocopies = 5;


void quit_with(byte which,byte errorlev);

void end_of_program();

#endif
