/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __logger_h__
#define __logger_h__


void log_setup(string name, boolean printing);

void log_divider();    /* Prints the divider sign. */

void log_command(string x);      /* Prints a command */

void log_scrollchar(string x);      /* print one character */

void log_italic();

void log_roman();

void log_epsonroman();

void log_scrollline();    /* Set up a line for the scroll driver */

void log_scrollendline(boolean centred);

void log_bubbleline(byte linenum,byte whom, string x);

void log_newline();

void log_newroom(string where);

void log_aside(string what);

void log_score(word credit,word now);

#endif
