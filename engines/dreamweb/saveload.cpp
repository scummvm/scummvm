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
 */

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamGenContext::loadgame() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 246);
	if (flags.z())
		goto alreadyload;
	data.byte(kCommandtype) = 246;
	al = 41;
	commandonly();
alreadyload:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noload) */;
	_cmp(ax, 1);
	if (flags.z())
		goto doload;
	return;
doload:
	data.byte(kLoadingorsave) = 1;
	showopbox();
	showloadops();
	data.byte(kCurrentslot) = 0;
	showslots();
	shownames();
	data.byte(kPointerframe) = 0;
	worktoscreenm();
	namestoold();
	data.byte(kGetback) = 0;
loadops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitloaded) */;
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = offset_loadlist;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto loadops;
	_cmp(data.byte(kGetback), 2);
	if (flags.z())
		return /* (quitloaded) */;
	getridoftemp();
	dx = data;
	es = dx;
	bx = 7979;
	startloading();
	loadroomssample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearsprites();
	initman();
	initrain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	worktoscreen();
	data.byte(kGetback) = 4;
}

void DreamGenContext::doload() {
	STACK_CHECK;
	data.byte(kLoadingorsave) = 1;
	showopbox();
	showloadops();
	data.byte(kCurrentslot) = 0;
	showslots();
	shownames();
	data.byte(kPointerframe) = 0;
	worktoscreenm();
	namestoold();
	data.byte(kGetback) = 0;
loadops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitloaded) */;
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = offset_loadlist;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto loadops;
	_cmp(data.byte(kGetback), 2);
	if (flags.z())
		return /* (quitloaded) */;
	getridoftemp();
	dx = data;
	es = dx;
	bx = 7979;
	startloading();
	loadroomssample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearsprites();
	initman();
	initrain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	worktoscreen();
	data.byte(kGetback) = 4;
}

void DreamGenContext::savegame() {
	STACK_CHECK;
	_cmp(data.byte(kMandead), 2);
	if (!flags.z())
		goto cansaveok;
	blank();
	return;
cansaveok:
	_cmp(data.byte(kCommandtype), 247);
	if (flags.z())
		goto alreadysave;
	data.byte(kCommandtype) = 247;
	al = 44;
	commandonly();
alreadysave:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (!flags.z())
		goto dosave;
	return;
dosave:
	data.byte(kLoadingorsave) = 2;
	showopbox();
	showsaveops();
	data.byte(kCurrentslot) = 0;
	showslots();
	shownames();
	worktoscreenm();
	namestoold();
	data.word(kBufferin) = 0;
	data.word(kBufferout) = 0;
	data.byte(kGetback) = 0;
saveops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitsavegame) */;
	delpointer();
	checkinput();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = offset_savelist;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto saveops;
}

} /*namespace dreamgen */
