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

#include "graph.h"
/*#include "Celer.h"*/
/*#include "Lucerna.h"*/

integer gd,gm;
int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 gd=3; gm=0; initgraph(gd,gm,"");
 setvisualpage(3);
 load_chunks('1');

 for( gd=0; gd <= num_chunks; gd ++)
  show_one_at(gd,0,gd*40);

 mblit(0,0,79,200,3,0);

 gd=getpixel(0,0);
 setvisualpage(0); setactivepage(0);

 settextstyle(0,0,4); setcolor(15);
 outtextxy(100,50,"Chunk1");
 input >> NL;
 return EXIT_SUCCESS;
}