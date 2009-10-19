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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

#define ip	(_game.logics[lognum].cIP)
#define code	(_game.logics[lognum].data)

#ifdef _L
#undef _L
#endif

#define _L(a,b,c) { a, b, c }

struct AgiLogicnames logicNamesTest[] = {
	_L("", 0, 0x00),
	_L("equaln", 2, 0x80),
	_L("equalv", 2, 0xC0),
	_L("lessn", 2, 0x80),
	_L("lessv", 2, 0xC0),
	_L("greatern", 2, 0x80),
	_L("greaterv", 2, 0xC0),
	_L("isset", 1, 0x00),
	_L("issetv", 1, 0x80),
	_L("has", 1, 0x00),
	_L("obj.in.room", 2, 0x40),
	_L("posn", 5, 0x00),
	_L("controller", 1, 0x00),
	_L("have.key", 0, 0x00),

	// Not 0 args. Has variable number.
	_L("said", 0, 0x00),

	_L("compare.strings", 2, 0x00),
	_L("obj.in.box", 5, 0x00),
	_L("center.posn", 5, 0x00),
	_L("right.posn", 5, 0x00),

	// Haven't seen an official name for this command so tried to name it descriptively.
	_L("in.motion.using.mouse", 0, 0x00)
};

struct AgiLogicnames logicNamesIf[] = {
	_L("OR", 0, 0x00),
	_L("NOT", 0, 0x00),
	_L("ELSE", 0, 0x00),
	_L("IF", 0, 0x00)
};

struct AgiLogicnames logicNamesCmd[] = {
	_L("return", 0, 0x00),	// 00
	_L("increment", 1, 0x80),	// 01
	_L("decrement", 1, 0x80),	// 02
	_L("assignn", 2, 0x80),	// 03
	_L("assignv", 2, 0xC0),	// 04
	_L("addn", 2, 0x80),	// 05
	_L("addv", 2, 0xC0),	// 06
	_L("subn", 2, 0x80),	// 07
	_L("subv", 2, 0xC0),	// 08
	_L("lindirectv", 2, 0xC0),	// 09
	_L("rindirect", 2, 0xC0),	// 0A
	_L("lindirectn", 2, 0x80),	// 0B
	_L("set", 1, 0x00),	// 0C
	_L("reset", 1, 0x00),	// 0D
	_L("toggle", 1, 0x00),	// 0E
	_L("set.v", 1, 0x80),	// 0F
	_L("reset.v", 1, 0x80),	// 10
	_L("toggle.v", 1, 0x80),	// 11
	_L("new.room", 1, 0x00),	// 12
	_L("new.room.v", 1, 0x80),	// 13
	_L("load.logics", 1, 0x00),	// 14
	_L("load.logics.v", 1, 0x80),	// 15
	_L("call", 1, 0x00),	// 16
	_L("call.v", 1, 0x80),	// 17
	_L("load.pic", 1, 0x80),	// 18
	_L("draw.pic", 1, 0x80),	// 19
	_L("show.pic", 0, 0x00),	// 1A
	_L("discard.pic", 1, 0x80),	// 1B
	_L("overlay.pic", 1, 0x80),	// 1C
	_L("show.pri.screen", 0, 0x00),	// 1D
	_L("load.view", 1, 0x00),	// 1E
	_L("load.view.v", 1, 0x80),	// 1F
	_L("discard.view", 1, 0x00),	// 20
	_L("animate.obj", 1, 0x00),	// 21
	_L("unanimate.all", 0, 0x00),	// 22
	_L("draw", 1, 0x00),	// 23
	_L("erase", 1, 0x00),	// 24
	_L("position", 3, 0x00),	// 25
	_L("position.v", 3, 0x60),	// 26
	_L("get.posn", 3, 0x60),	// 27
	_L("reposition", 3, 0x60),	// 28
	_L("set.view", 2, 0x00),	// 29
	_L("set.view.v", 2, 0x40),	// 2A
	_L("set.loop", 2, 0x00),	// 2B
	_L("set.loop.v", 2, 0x40),	// 2C
	_L("fix.loop", 1, 0x00),	// 2D
	_L("release.loop", 1, 0x00),	// 2E
	_L("set.cel", 2, 0x00),	// 2F
	_L("set.cel.v", 2, 0x40),	// 30
	_L("last.cel", 2, 0x40),	// 31
	_L("current.cel", 2, 0x40),	// 32
	_L("current.loop", 2, 0x40),	// 33
	_L("current.view", 2, 0x40),	// 34
	_L("number.of.loops", 2, 0x40),	// 35
	_L("set.priority", 2, 0x00),	// 36
	_L("set.priority.v", 2, 0x40),	// 37
	_L("release.priority", 1, 0x00),	// 38
	_L("get.priority", 2, 0x40),	// 39
	_L("stop.update", 1, 0x00),	// 3A
	_L("start.update", 1, 0x00),	// 3B
	_L("force.update", 1, 0x00),	// 3C
	_L("ignore.horizon", 1, 0x00),	// 3D
	_L("observe.horizon", 1, 0x00),	// 3E
	_L("set.horizon", 1, 0x00),	// 3F
	_L("object.on.water", 1, 0x00),	// 40
	_L("object.on.land", 1, 0x00),	// 41
	_L("object.on.anything", 1, 0x00),	// 42
	_L("ignore.objs", 1, 0x00),	// 43
	_L("observe.objs", 1, 0x00),	// 44
	_L("distance", 3, 0x20),	// 45
	_L("stop.cycling", 1, 0x00),	// 46
	_L("start.cycling", 1, 0x00),	// 47
	_L("normal.cycle", 1, 0x00),	// 48
	_L("end.of.loop", 2, 0x00),	// 49
	_L("reverse.cycle", 1, 0x00),	// 4A
	_L("reverse.loop", 2, 0x00),	// 4B
	_L("cycle.time", 2, 0x40),	// 4C
	_L("stop.motion", 1, 0x00),	// 4D
	_L("start.motion", 1, 0x00),	// 4E
	_L("step.size", 2, 0x40),	// 4F
	_L("step.time", 2, 0x40),	// 50
	_L("move.obj", 5, 0x00),	// 51
	_L("move.obj.v", 5, 0x70),	// 52
	_L("follow.ego", 3, 0x00),	// 53
	_L("wander", 1, 0x00),	// 54
	_L("normal.motion", 1, 0x00),	// 55
	_L("set.dir", 2, 0x40),	// 56
	_L("get.dir", 2, 0x40),	// 57
	_L("ignore.blocks", 1, 0x00),	// 58
	_L("observe.blocks", 1, 0x00),	// 59
	_L("block", 4, 0x00),	// 5A
	_L("unblock", 0, 0x00),	// 5B
	_L("get", 1, 0x00),	// 5C
	_L("get.v", 1, 0x80),	// 5D
	_L("drop", 1, 0x00),	// 5E
	_L("put", 2, 0x00),	// 5F
	_L("put.v", 2, 0x40),	// 60
	_L("get.room.v", 2, 0xC0),	// 61
	_L("load.sound", 1, 0x00),	// 62
	_L("sound", 2, 0x00),	// 63
	_L("stop.sound", 0, 0x00),	// 64
	_L("print", 1, 0x00),	// 65
	_L("print.v", 1, 0x80),	// 66
	_L("display", 3, 0x00),	// 67
	_L("display.v", 3, 0xE0),	// 68
	_L("clear.lines", 3, 0x00),	// 69
	_L("text.screen", 0, 0x00),	// 6A
	_L("graphics", 0, 0x00),	// 6B
	_L("set.cursor.char", 1, 0x00),	// 6C
	_L("set.text.attribute", 2, 0x00),	// 6D
	_L("shake.screen", 1, 0x00),	// 6E
	_L("configure.screen", 3, 0x00),	// 6F
	_L("status.line.on", 0, 0x00),	// 70
	_L("status.line.off", 0, 0x00),	// 71
	_L("set.string", 2, 0x00),	// 72
	_L("get.string", 5, 0x00),	// 73
	_L("word.to.string", 2, 0x00),	// 74
	_L("parse", 1, 0x00),	// 75
	_L("get.num", 2, 0x40),	// 76
	_L("prevent.input", 0, 0x00),	// 77
	_L("accept.input", 0, 0x00),	// 78
	_L("set.key", 3, 0x00),	// 79
	_L("add.to.pic", 7, 0x00),	// 7A
	_L("add.to.pic.v", 7, 0xFE),	// 7B
	_L("status", 0, 0x00),	// 7C
	_L("save.game", 0, 0x00),	// 7D
	_L("restore.game", 0, 0x00),	// 7E
	_L("init.disk", 0, 0x00),	// 7F
	_L("restart.game", 0, 0x00),	// 80
	_L("show.obj", 1, 0x00),	// 81
	_L("random", 3, 0x20),	// 82
	_L("program.control", 0, 0x00),	// 83
	_L("player.control", 0, 0x00),	// 84
	_L("obj.status.v", 1, 0x80),	// 85
	// 0 args for AGI version 2.089
	_L("quit", 1, 0x00),	// 86

	_L("show.mem", 0, 0x00),	// 87
	_L("pause", 0, 0x00),	// 88
	_L("echo.line", 0, 0x00),	// 89
	_L("cancel.line", 0, 0x00),	// 8A
	_L("init.joy", 0, 0x00),	// 8B
	_L("toggle.monitor", 0, 0x00),	// 8C
	_L("version", 0, 0x00),	// 8D
	_L("script.size", 1, 0x00),	// 8E
	_L("set.game.id", 1, 0x00),	// 8F
	_L("log", 1, 0x00),	// 90
	_L("set.scan.start", 0, 0x00),	// 91
	_L("reset.scan.start", 0, 0x00),	// 92
	_L("reposition.to", 3, 0x00),	// 93
	_L("reposition.to.v", 3, 0x60),	// 94
	_L("trace.on", 0, 0x00),	// 95
	_L("trace.info", 3, 0x00),	// 96

	// 3 args for AGI versions before 2.440
	_L("print.at", 4, 0x00),	// 97
	_L("print.at.v", 4, 0x80),	// 98

	_L("discard.view.v", 1, 0x80),	// 99
	_L("clear.text.rect", 5, 0x00),	// 9A
	_L("set.upper.left", 2, 0x00),	// 9B
	_L("set.menu", 1, 0x00),	// 9C
	_L("set.menu.item", 2, 0x00),	// 9D
	_L("submit.menu", 0, 0x00),	// 9E
	_L("enable.item", 1, 0x00),	// 9F
	_L("disable.item", 1, 0x00),	// A0
	_L("menu.input", 0, 0x00),	// A1
	_L("show.obj.v", 1, 0x01),	// A2
	_L("open.dialogue", 0, 0x00),	// A3
	_L("close.dialogue", 0, 0x00),	// A4
	_L("mul.n", 2, 0x80),	// A5
	_L("mul.v", 2, 0xC0),	// A6
	_L("div.n", 2, 0x80),	// A7
	_L("div.v", 2, 0xC0),	// A8
	_L("close.window", 0, 0x00),	// A9

	_L("set.simple", 1, 0x00),	// AA
	_L("push.script", 0, 0x00),	// AB
	_L("pop.script", 0, 0x00),	// AC
	_L("hold.key", 0, 0x00),	// AD
	_L("set.pri.base", 1, 0x00),	// AE
	_L("discard.sound", 1, 0x00),	// AF

	// 1 arg for AGI version 3.002.086
	_L("hide.mouse", 0, 0x00),	// B0

	_L("allow.menu", 1, 0x00),	// B1
	_L("show.mouse", 0, 0x00),	// B2
	_L("fence.mouse", 4, 0x00),	// B3
	_L("mouse.posn", 2, 0x00),	// B4
	_L("release.key", 0, 0x00),	// B5

	// 2 args for at least the Amiga Gold Rush! (v2.05 1989-03-09) using Amiga AGI 2.316.
	_L("adj.ego.move.to.xy", 0, 0x00),	// B6
	_L(NULL, 0, 0x00)
};

void AgiEngine::debugConsole(int lognum, int mode, const char *str) {
	AgiLogicnames *x;
	uint8 a, c, z;

	if (str) {
		report("         %s\n", str);
		return;
	}

	report("%03d:%04x ", lognum, ip);

	switch (*(code + ip)) {
	case 0xFC:
	case 0xFD:
	case 0xFE:
	case 0xFF:
		x = logicNamesIf;

		if (_debug.opcodes) {
			report("%02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
			    "         ",
			    (uint8)*(code + (0 + ip)) & 0xFF,
			    (uint8)*(code + (1 + ip)) & 0xFF,
			    (uint8)*(code + (2 + ip)) & 0xFF,
			    (uint8)*(code + (3 + ip)) & 0xFF,
			    (uint8)*(code + (4 + ip)) & 0xFF,
			    (uint8)*(code + (5 + ip)) & 0xFF,
			    (uint8)*(code + (6 + ip)) & 0xFF,
			    (uint8)*(code + (7 + ip)) & 0xFF,
			    (uint8)*(code + (8 + ip)) & 0xFF);
		}
		report("%s ", (x + *(code + ip) - 0xFC)->name);
		break;
	default:
		x = mode == lCOMMAND_MODE ? logicNamesCmd : logicNamesTest;
		a = (unsigned char)(x + *(code + ip))->numArgs;
		c = (unsigned char)(x + *(code + ip))->argMask;

		if (_debug.opcodes) {
			report("%02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
			    "         ",
			    (uint8)*(code + (0 + ip)) & 0xFF,
			    (uint8)*(code + (1 + ip)) & 0xFF,
			    (uint8)*(code + (2 + ip)) & 0xFF,
			    (uint8)*(code + (3 + ip)) & 0xFF,
			    (uint8)*(code + (4 + ip)) & 0xFF,
			    (uint8)*(code + (5 + ip)) & 0xFF,
			    (uint8)*(code + (6 + ip)) & 0xFF,
			    (uint8)*(code + (7 + ip)) & 0xFF,
			    (uint8)*(code + (8 + ip)) & 0xFF);
		}
		report("%s ", (x + *(code + ip))->name);

		for (z = 1; a > 0;) {
			if (~c & 0x80) {
				report("%d", *(code + (ip + z)));
			} else {
				report("v%d[%d]", *(code + (ip + z)), getvar(*(code + (ip + z))));
			}
			c <<= 1;
			z++;
			if (--a > 0)
				report(",");
		}
		break;
	}

	report("\n");
}

} // End of namespace Agi
