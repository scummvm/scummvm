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

#ifdef HAVE_READLINE
#include "debugrl.h"
#endif

enum {
	CMD_INVALID,
	CMD_HELP,
	CMD_QUIT,
	CMD_GO,
	CMD_ACTOR,
	CMD_SCRIPTS,
	CMD_LOAD_ROOM,
	CMD_DUMPBOX,
	CMD_VAR,
	CMD_WATCH,
	CMD_EXIT
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

void BoxTest(int num);

bool ScummDebugger::do_command()
{
	switch (get_command()) {
	case CMD_HELP:
		printf("Debugger commands:\n"
					 "(h)elp -> display this help text\n"
					 "(q)uit -> quit the debugger\n"
					 "(g)o [numframes] -> increase frame\n"
					 "(a)ctor [actornum] -> show actor information\n"
					 "(r)oom [roomnum] -> load room\n"
					 "(s)cripts -> show running scripts\n"
					 "(b)oxes -> list and draw boxen\n"
					 "(v)ariable -> set or show a variable value\n"
					 "(w)atch [varnum] -> set a variable watch. 0 means all variables.\n" "(e)xit -> exit game\n");
		return true;

	case CMD_QUIT:
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
			_s->actor[_s->_vars[_s->VAR_EGO]].room = room;
			_s->startScene(room, 0, 0);
			_s->_fullRedraw = 1;
		}
		return true;
	case CMD_DUMPBOX:
		{
			int num, i = 0;
			BoxCoords box;
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
			for (i = 0; i < num; i++) {
				warning("BoxTest currently unimplemented in new graphics code\n");
				/*BoxTest(i); */
				_s->getBoxCoordinates(i, &box);
				printf("%d: [%d x %d] [%d x %d] [%d x %d] [%d x %d]\n", i,
							 box.ul.x, box.ul.y, box.ll.x, box.ll.y, box.ur.x, box.ur.y, box.lr.x, box.lr.y);
			}
		}
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
	case CMD_EXIT:
		exit(1);

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
	{"q", 1, CMD_QUIT},
	{"g", 1, CMD_GO},
	{"a", 1, CMD_ACTOR},
	{"s", 1, CMD_SCRIPTS},
	{"r", 1, CMD_LOAD_ROOM},
	{"b", 1, CMD_DUMPBOX},
	{"v", 1, CMD_VAR},
	{"w", 1, CMD_WATCH},
	{"e", 1, CMD_EXIT},
	{"", 0, 0}
};

int ScummDebugger::get_command()
{
	const DebuggerCommands *dc;
	char *s;
	int i;
	static char *buf;

	do {
#ifndef HAVE_READLINE
		buf = _cmd_buffer;
		printf("debug> ");
		if (!fgets(_cmd_buffer, sizeof(_cmd_buffer), stdin))
			return CMD_QUIT;

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
			return CMD_QUIT;
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

	printf("+--------------------------------------------------------------+\n");
	printf("|# |room|  x y   |elev|cos|width|box|mov|zp|frame|scale|spd|dir|\n");
	printf("+--+----+--------+----+---+-----+---+---+--+-----+-----+---+---+\n");
	for (i = 1; i < _s->NUM_ACTORS; i++) {
		if (act == -1 || act == i) {
			a = &_s->actor[i];
			if (a->visible)
				printf("|%2d|%4d|%3d  %3d|%4d|%3d|%5d|%3d|%3d|%2d|%5d|%5d|%3d|%3d|\n",
							 i, a->room, a->x, a->y, a->elevation, a->costume, a->width,
							 a->walkbox, a->moving, a->forceClip, a->frame, a->scalex, a->speedx, a->facing);
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
	printf("+---------------------------------+\n");
}
