/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

/*
 * Readline and command completion support by Tom Dunstan <tommyd@senet.com.au>
 */

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "debug.h"

#ifdef HAVE_READLINE
#include "debugrl.h"
#endif

enum {
	CMD_INVALID,
	CMD_HELP,
	CMD_CONTINUE,
	CMD_GO,
	CMD_ACTOR,
	CMD_SCRIPTS,
	CMD_LOAD_ROOM,
	CMD_DUMPBOX,
	CMD_VAR,
	CMD_WATCH,
	CMD_QUIT
};

void ScummDebugger::attach(Scumm *s)
{
	if (_s)
		detach();

	_welcome = true;
	_s = s;
	s->_debugger = this;

	_go_amount = 1;
#ifdef HAVE_READLINE
	initialize_readline();
#endif
}

bool ScummDebugger::do_command()
{
	switch (get_command()) {
	case CMD_HELP:
		printf("Debugger commands:\n"
					 "(a)ctor [actornum] -> show actor information\n"
					 "(b)ox [boxnum] -> list and draw the specified (or all) boxes\n"
					 "(c)ontinue -> exit the debugger and continue the program\n"
					 "(h)elp -> display this help text\n"
					 "(g)o [numframes] -> increase frame\n"
					 "(r)oom [roomnum] -> load room\n"
					 "(s)cripts -> show running scripts\n"
					 "(v)ariable -> set or show a variable value\n"
					 "(w)atch [varnum] -> set a variable watch. 0 means all variables.\n"
					 "(q)uit -> exit game\n");
		return true;

	case CMD_CONTINUE:
		detach();
		return false;

	case CMD_GO:
		if (!_parameters[0])
			_go_amount = 1;
		else
			_go_amount = atoi(_parameters);
		return false;
	case CMD_ACTOR:
		if (!_parameters[0])
			printActors(-1);
		else
			printActors(atoi(_parameters));
		return true;
	case CMD_SCRIPTS:
		printScripts();
		return true;
	case CMD_LOAD_ROOM:
		if (!_parameters[0]) {
			printf("Current room: %d [%d]\n", _s->_currentRoom, _s->_roomResource);
		} else {
			int room = atoi(_parameters);
			_s->_actors[_s->_vars[_s->VAR_EGO]].room = room;
			_s->startScene(room, 0, 0);
			_s->_fullRedraw = 1;
		}
		return true;
	case CMD_DUMPBOX:
		if (!_parameters[0])
			printBoxes();
		else
			printBox(atoi(_parameters));
		return true;
	case CMD_VAR:
		if (!_parameters[0]) {
			printf("v 123 will show the value of 123, v 123 456 will set the value of 123 to 456.\n");
		} else {
			char *tok = strtok(_parameters, " ");
			int var = atoi(tok);
			tok = strtok(NULL, "");
			if (tok)
				_s->writeVar(var, atoi(tok));

			printf("Var[%d] = %d\n", var, _s->readVar(var));
		}
		return true;

	case CMD_WATCH:
		if (!_parameters[0]) {
			printf("Clearing all watches..\n");
			_s->_varwatch = -1;
		} else {
			_s->_varwatch = atoi(_parameters);
			if (_s->_varwatch == 0)
				printf("Watching all variables\n");
			else
				printf("Watching vars[%d]\n", _s->_varwatch);
		}
		return true;
	case CMD_QUIT:
		_s->_system->quit();

	default:											/* this line is never reached */
		error("Unknown debug command");
		return true;
	}
}

void ScummDebugger::enter()
{
	if (_welcome) {
		_welcome = false;
		printf
			("Debugging Mode entered!, please switch to this console for input.\n"
			 "Enter h to list all the debug commands\n");
	}
	while (do_command()) {
	}
}


void ScummDebugger::on_frame()
{
	if (_go_amount == 0)
		return;
	if (!--_go_amount)
		enter();
}


void ScummDebugger::detach()
{
	_s->_debugger = NULL;
	_s = NULL;
}

struct DebuggerCommands {
	char text[8];
	byte len;
	byte id;
};

static const DebuggerCommands debugger_commands[] = {
	{"h", 1, CMD_HELP},
	{"c", 1, CMD_CONTINUE},
	{"g", 1, CMD_GO},
	{"a", 1, CMD_ACTOR},
	{"s", 1, CMD_SCRIPTS},
	{"r", 1, CMD_LOAD_ROOM},
	{"b", 1, CMD_DUMPBOX},
	{"v", 1, CMD_VAR},
	{"w", 1, CMD_WATCH},
	{"q", 1, CMD_QUIT},
	{"", 0, 0}
};

int ScummDebugger::get_command()
{
	const DebuggerCommands *dc;
	char *s;
	int i;
	char *buf;

	do {
#ifndef HAVE_READLINE
		buf = _cmd_buffer;
		printf("debug> ");
		if (!fgets(_cmd_buffer, sizeof(_cmd_buffer), stdin))
			return CMD_CONTINUE;

		i = strlen(_cmd_buffer);
		while (i > 0 && _cmd_buffer[i - 1] == 10)
			_cmd_buffer[--i] = 0;

		if (i == 0)
			continue;

#else	// yes we do have readline
		if (buf) {
			free(buf);
		}
		buf = readline("debug> ");
		if (!buf) {
			printf("\n");
			return CMD_CONTINUE;
		}
		if (strlen(buf) == 0) {
			continue;
		}
		add_history(buf);
#endif

		dc = debugger_commands;
		do {
			if (!strncmp(buf, dc->text, dc->len)) {
				for (s = buf; *s; s++) {
					if (*s == 32) {
						s++;
						break;
					}
				}
				_parameters = s;
				return _command = dc->id;
			}
		} while ((++dc)->text[0]);

		for (s = buf; *s; s++)
			if (*s == 32) {
				*s = 0;
				break;
			}
		printf("Invalid command '%s'. Type 'help' for a list of available commands.\n", buf);
	} while (1);
}

void ScummDebugger::printActors(int act)
{
	int i;
	Actor *a;

	printf("+------------------------------------------------------------------+\n");
	printf("|# |room|  x y   |elev|cos|width|box|mov|zp|frame|scale|spd|dir|cls|\n");
	printf("+--+----+--------+----+---+-----+---+---+--+-----+-----+---+---+---+\n");
	for (i = 1; i < _s->NUM_ACTORS; i++) {
		if (act == -1 || act == i) {
			a = &_s->_actors[i];
			if (a->visible)
				printf("|%2d|%4d|%3d  %3d|%4d|%3d|%5d|%3d|%3d|%2d|%5d|%5d|%3d|%3d|$%02x|\n",
							 a->number, a->room, a->x, a->y, a->elevation, a->costume,
							 a->width, a->walkbox, a->moving, a->forceClip, a->frame,
							 a->scalex, a->speedx, a->facing, int(_s->_classData[a->number]&0xFF));
		}
	}
	printf("+--------------------------------------------------------------+\n");
}

void ScummDebugger::printScripts()
{
	int i;
	ScriptSlot *ss;

	printf("+---------------------------------+\n");
	printf("|# |num|sta|typ|un1|un2|fc|cut|un5|\n");
	printf("+--+---+---+---+---+---+--+---+---+\n");
	for (i = 0; i < 25; i++) {
		ss = &_s->vm.slot[i];
		if (ss->number) {
			printf("|%2d|%3d|%3d|%3d|%3d|%3d|%2d|%3d|%3d|\n",
						 i, ss->number, ss->status, ss->where, ss->unk1, ss->unk2,
						 ss->freezeCount, ss->cutsceneOverride, ss->unk5);
		}
	}
	printf("+-------------------------------------+\n");
}


void ScummDebugger::printBoxes()
{
	int num, i = 0;
	byte *boxm = _s->getBoxMatrixBaseAddr();
	num = _s->getNumBoxes();

	printf("Walk matrix:\n");
	for (i = 0; i < num; i++) {
		while (*boxm != 0xFF) {
			printf("[%d] ", *boxm);
			boxm++;
		}
		boxm++;
		printf("\n");
	}

	printf("\nWalk boxes:\n");
	for (i = 0; i < num; i++)
		printBox(i);
}

void ScummDebugger::printBox(int box)
{
	BoxCoords coords;
	int flags = _s->getBoxFlags(box);
	int mask = _s->getMaskFromBox(box);

	_s->getBoxCoordinates(box, &coords);
	
	// Draw the box
	boxTest(box);

	// Print out coords, flags, zbuffer mask
	printf("%d: [%d x %d] [%d x %d] [%d x %d] [%d x %d], flags=0x%02x, mask=%d\n",
				box,
				 coords.ul.x, coords.ul.y, coords.ll.x, coords.ll.y,
				 coords.ur.x, coords.ur.y, coords.lr.x, coords.lr.y,
				 flags, mask);
}


/************ ENDER: Temporary debug code for boxen **************/
/*
int hlineColor(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color)
*/

static int gfxPrimitivesCompareInt(const void *a, const void *b);

static int *gfxPrimitivesPolyInts = NULL;
static int gfxPrimitivesPolyAllocated = 0;


static byte *getBasePtr(Scumm *_s, int x, int y)
{
	VirtScreen *vs = _s->findVirtScreen(y);

	if (vs == NULL)
		return NULL;

	return vs->screenPtr + x + (y - vs->topline) * _s->_realWidth +
		_s->_screenStartStrip * 8 + (_s->camera._cur.y - (_s->_realHeight / 2)) * _s->_realWidth;
}

static void hline(Scumm *scumm, int x1, int x2, int y, byte color)
{
	byte *ptr;

	if (x2 < x1)
		x2 ^= x1 ^= x2 ^= x1;				// Swap x2 and x1

	ptr = getBasePtr(scumm, x1, y);

	if (ptr == NULL)
		return;

	while (x1++ <= x2) {
		*ptr++ = color;
	}
}


static void filledPolygonColor(Scumm *scumm, int16 *vx, int16 *vy, int n, int color)
{
	int i;
	int y;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;

	/* Sanity check */
	if (n < 3) {
		return;
	}

	/* Allocate temp array, only grow array */
	if (!gfxPrimitivesPolyAllocated) {
		gfxPrimitivesPolyInts = (int *)malloc(sizeof(int) * n);
		gfxPrimitivesPolyAllocated = n;
	} else {
		if (gfxPrimitivesPolyAllocated < n) {
			gfxPrimitivesPolyInts = (int *)realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
			gfxPrimitivesPolyAllocated = n;
		}
	}

	/* Determine Y maxima */
	miny = vy[0];
	maxy = vy[0];
	for (i = 1; (i < n); i++) {
		if (vy[i] < miny) {
			miny = vy[i];
		} else if (vy[i] > maxy) {
			maxy = vy[i];
		}
	}

	/* Draw, scanning y */
	for (y = miny; (y <= maxy); y++) {
		ints = 0;
		for (i = 0; (i < n); i++) {
			if (!i) {
				ind1 = n - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			y1 = vy[ind1];
			y2 = vy[ind2];
			if (y1 < y2) {
				x1 = vx[ind1];
				x2 = vx[ind2];
			} else if (y1 > y2) {
				y2 = vy[ind1];
				y1 = vy[ind2];
				x2 = vx[ind1];
				x1 = vx[ind2];
			} else {
				continue;
			}
			if ((y >= y1) && (y < y2)) {
				gfxPrimitivesPolyInts[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			} else if ((y == maxy) && (y > y1) && (y <= y2)) {
				gfxPrimitivesPolyInts[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			}
		}
		qsort(gfxPrimitivesPolyInts, ints, sizeof(int), gfxPrimitivesCompareInt);

		for (i = 0; (i < ints); i += 2) {
			hline(scumm, gfxPrimitivesPolyInts[i], gfxPrimitivesPolyInts[i + 1], y, color);
		}
	}

	return;
}

static int gfxPrimitivesCompareInt(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

void ScummDebugger::boxTest(int num)
{
	BoxCoords box;
	int16 rx1[4], ry1[4];

	_s->getBoxCoordinates(num, &box);
	rx1[0] = box.ul.x;
	ry1[0] = box.ul.y;
	rx1[1] = box.ur.x;
	ry1[1] = box.ur.y;
	rx1[2] = box.lr.x;
	ry1[2] = box.lr.y;
	rx1[3] = box.ll.x;
	ry1[3] = box.ll.y;

	// TODO - maybe use different colors for each box, and/or print the box number inside it?
	filledPolygonColor(_s, &rx1[0], &ry1[0], 4, 255);

	VirtScreen *vs = _s->findVirtScreen(box.ul.y);
	if (vs != NULL)
		_s->setVirtscreenDirty(vs, 0, 0, _s->_realWidth, _s->_realHeight);
	_s->drawDirtyScreenParts();
	_s->_system->update_screen();
}
