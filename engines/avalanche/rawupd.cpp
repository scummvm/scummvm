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

/*#include "Dos.h"*/
/*#include "Crt.h"*/

searchrec s;
string x,y;
longint hash_time,s_time;
boolean s_exists;

void find_out_about_s(string name)
{
    searchrec ss;
;
 findfirst(name,anyfile,ss);
 s_exists=doserror==0;

 if (s_exists) 
  s_time=ss.time;
}

void get_y()
{
 y=x;
 if (set::of(range('0','9'), eos).has(x[2])) 
  y[1]='h'; else
  if (x[3]=='K') 
   y[1]='a'; else
    y[1]='s';
}

void rename_it()
{
    untyped_file f;

 output << x << " -> " << y;
 assign(f,x); reset(f); rename(f,y); close(f);
 output << " ...done." << NL;
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 output << NL;
 findfirst("#*.*",anyfile,s);
 while (doserror==0) 
 {
  x=s.name;
  get_y();
  hash_time=s.time;
  output << format(x,15); clreol;
  find_out_about_s(y);
  if (s_exists) 
  {
   output << ": s exists and is ";
   if (s_time<hash_time) 
    output << "NEWER!" << NL;
    else if (s_time==hash_time) 
     output << string("equal.")+'\15';
      else output << "older." << NL;
  } else
  {
   output << " ... NO S FOUND! Renaming...";
   rename_it();
  }

  findnext(s);
 }
 return EXIT_SUCCESS;
}