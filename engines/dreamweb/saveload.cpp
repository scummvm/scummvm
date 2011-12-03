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
#include "engines/metaengine.h"
#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace DreamGen {

void DreamGenContext::loadGame() {
	if (data.byte(kCommandtype) != 246) {
		data.byte(kCommandtype) = 246;
		commandOnly(41);
	}
	if (data.word(kMousebutton) == data.word(kOldbutton))
		return; // "noload"
	if (data.word(kMousebutton) == 1) {
		ax = 0xFFFF;
		doLoad();
	}
}

// input: ax = savegameId
// if -1, open menu to ask for slot to load
// if >= 0, directly load from that slot
void DreamGenContext::doLoad() {
	int savegameId = (int16)ax;

	data.byte(kLoadingorsave) = 1;

	if (ConfMan.getBool("dreamweb_originalsaveload") && savegameId == -1) {
		showOpBox();
		showLoadOps();
		data.byte(kCurrentslot) = 0;
		showSlots();
		showNames();
		data.byte(kPointerframe) = 0;
		workToScreenM();
		namesToOld();
		data.byte(kGetback) = 0;

		while (true) {
			if (quitRequested())
				return;
			delPointer();
			readMouse();
			showPointer();
			vSync();
			dumpPointer();
			dumpTextLine();
			RectWithCallback loadlist[] = {
				{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamGenContext::getBackToOps },
				{ kOpsx+128,kOpsx+190,kOpsy+12,kOpsy+100,&DreamGenContext::actualLoad },
				{ kOpsx+2,kOpsx+92,kOpsy+4,kOpsy+81,&DreamGenContext::selectSlot },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(loadlist);
			if (data.byte(kGetback) == 1)
				break;
			if (data.byte(kGetback) == 2)
				return; // "quitloaded"
		}
	} else {

		if (savegameId == -1) {
			// Open dialog to get savegameId

			const EnginePlugin *plugin = NULL;
			Common::String gameId = ConfMan.get("gameid");
			EngineMan.findGame(gameId, &plugin);
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"));
			dialog->setSaveMode(false);
			savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
			delete dialog;
		}

		if (savegameId < 0) {
			data.byte(kGetback) = 0;
			return;
		}

		loadPosition(savegameId);

		data.byte(kGetback) = 1;
	}

	// kTempgraphics might not have been allocated if we bypassed all menus
	if (data.word(kTempgraphics) != 0xFFFF)
		getRidOfTemp();

	dx = data;
	es = dx;
	const Room *room = (const Room *)cs.ptr(kMadeuproomdat, sizeof(Room));
	startLoading(*room);
	loadRoomsSample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearSprites();
	initMan();
	initRain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	workToScreen();
	data.byte(kGetback) = 4;
}


void DreamGenContext::saveGame() {
	if (data.byte(kMandead) == 2) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 247) {
		data.byte(kCommandtype) = 247;
		commandOnly(44);
	}
	if (data.word(kMousebutton) != 1)
		return;

	data.byte(kLoadingorsave) = 2;

	if (ConfMan.getBool("dreamweb_originalsaveload")) {
		showOpBox();
		showSaveOps();
		data.byte(kCurrentslot) = 0;
		showSlots();
		showNames();
		workToScreenM();
		namesToOld();
		data.word(kBufferin) = 0;
		data.word(kBufferout) = 0;
		data.byte(kGetback) = 0;

		while (true) {
			if (quitRequested())
				return;
			delPointer();
			checkInput();
			readMouse();
			showPointer();
			vSync();
			dumpPointer();
			dumpTextLine();

			RectWithCallback savelist[] = {
				{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamGenContext::getBackToOps },
				{ kOpsx+128,kOpsx+190,kOpsy+12,kOpsy+100,&DreamGenContext::actualSave },
				{ kOpsx+2,kOpsx+92,kOpsy+4,kOpsy+81,&DreamGenContext::selectSlot },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(savelist);
			_cmp(data.byte(kGetback), 0);
			if (flags.z())
				continue;
			break;
		}
		return;
	} else {
		const EnginePlugin *plugin = NULL;
		Common::String gameId = ConfMan.get("gameid");
		EngineMan.findGame(gameId, &plugin);
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
		dialog->setSaveMode(true);
		int savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
		Common::String game_description = dialog->getResultString();
		if (game_description.empty())
			game_description = "Untitled";
		delete dialog;

		if (savegameId < 0) {
			data.byte(kGetback) = 0;
			return;
		}

		// TODO: Check if this 2 is a constant
		uint8 descbuf[17] = { 2, 0 };
		strncpy((char*)descbuf+1, game_description.c_str(), 16);
		unsigned int desclen = game_description.size();
		if (desclen > 15)
			desclen = 15;
		// zero terminate, and pad with ones
		descbuf[++desclen] = 0;
		while (desclen < 17)
			descbuf[++desclen] = 1;
		if (savegameId < 7)
			memcpy(data.ptr(kSavenames + 17*savegameId, 17), descbuf, 17);

		savePosition(savegameId, descbuf);

		// TODO: The below is copied from actualsave
		getRidOfTemp();
		restoreAll(); // reels
		data.word(kTextaddressx) = 13;
		data.word(kTextaddressy) = 182;
		data.byte(kTextlen) = 240;
		redrawMainScrn();
		workToScreenM();
		data.byte(kGetback) = 4;
	}
}

void DreamGenContext::namesToOld() {
	memcpy(getSegment(data.word(kBuffers)).ptr(kZoomspace, 0), cs.ptr(kSavenames, 0), 17*4);
}

void DreamGenContext::oldToNames() {
	memcpy(cs.ptr(kSavenames, 0), getSegment(data.word(kBuffers)).ptr(kZoomspace, 0), 17*4);
}

void DreamGenContext::saveLoad() {
	if (data.word(kWatchingtime) || (data.byte(kPointermode) == 2)) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 253) {
		data.byte(kCommandtype) = 253;
		commandOnly(43);
	}
	if ((data.word(kMousebutton) != data.word(kOldbutton)) && (data.word(kMousebutton) & 1))
		doSaveLoad();
}

void DreamGenContext::showMainOps() {
	showFrame(tempGraphics(), kOpsx+10, kOpsy+10, 8, 0);
	showFrame(tempGraphics(), kOpsx+59, kOpsy+30, 7, 0);
	showFrame(tempGraphics(), kOpsx+128+4, kOpsy+12, 1, 0);
}

void DreamGenContext::showDiscOps() {
	showFrame(tempGraphics(), kOpsx+128+4, kOpsy+12, 1, 0);
	showFrame(tempGraphics(), kOpsx+10, kOpsy+10, 9, 0);
	showFrame(tempGraphics(), kOpsx+59, kOpsy+30, 10, 0);
	showFrame(tempGraphics(), kOpsx+176+2, kOpsy+60-4, 5, 0);
}

void DreamGenContext::actualSave() {
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(44);
	}

	if (!(data.word(kMousebutton) & 1))
		return;

	unsigned int slot = data.byte(kCurrentslot);

	const uint8 *desc = data.ptr(kSavenames + 17*slot, 16);
	if (desc[1] == 0) // The actual description string starts at desc[1]
		return;

	savePosition(slot, desc);

	getRidOfTemp();
	restoreAll(); // reels
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	redrawMainScrn();
	workToScreenM();
	data.byte(kGetback) = 4;
}

void DreamGenContext::actualLoad() {
	if (data.byte(kCommandtype) != 221) {
		data.byte(kCommandtype) = 221;
		commandOnly(41);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || data.word(kMousebutton) != 1)
		return;

	unsigned int slot = data.byte(kCurrentslot);

	const uint8 *desc = data.ptr(kSavenames + 17*slot, 16);
	if (desc[1] == 0) // The actual description string starts at desc[1]
		return;

	loadPosition(data.byte(kCurrentslot));
	data.byte(kGetback) = 1;
}

void DreamGenContext::savePosition(unsigned int slot, const uint8 *descbuf) {

	const Room *currentRoom = (const Room *)cs.ptr(kRoomdata + sizeof(Room)*data.byte(kLocation), sizeof(Room));
	Room *madeUpRoom = (Room *)cs.ptr(kMadeuproomdat, sizeof(Room));

	*madeUpRoom = *currentRoom;
	madeUpRoom->roomsSample = data.byte(kRoomssample);
	madeUpRoom->mapX = data.byte(kMapx);
	madeUpRoom->mapY = data.byte(kMapy);
	madeUpRoom->liftFlag = data.byte(kLiftflag);
	madeUpRoom->b21 = data.byte(kManspath);
	madeUpRoom->facing = data.byte(kFacing);
	madeUpRoom->b27 = 255;

	openForSave(slot);

	// fill length fields in savegame file header
	uint16 len[6] = { 17, kLengthofvars, kLengthofextra,
	                  4*kNumchanges, 48, kLenofreelrouts };
	for (int i = 0; i < 6; ++i)
		data.word(kFiledata + 2*i) = len[i];

	engine->writeToSaveFile(data.ptr(kFileheader, kHeaderlen), kHeaderlen);
	engine->writeToSaveFile(descbuf, len[0]);
	engine->writeToSaveFile(data.ptr(kStartvars, len[1]), len[1]);
	engine->writeToSaveFile(getSegment(data.word(kExtras)).ptr(kExframedata, len[2]), len[2]);
	engine->writeToSaveFile(getSegment(data.word(kBuffers)).ptr(kListofchanges, len[3]), len[3]);
	engine->writeToSaveFile(data.ptr(kMadeuproomdat, len[4]), len[4]);
	engine->writeToSaveFile(data.ptr(kReelroutines, len[5]), len[5]);
	closeFile();
}

void DreamGenContext::loadPosition(unsigned int slot) {
	data.word(kTimecount) = 0;
	clearChanges();

	openForLoad(slot);

	engine->readFromSaveFile(cs.ptr(kFileheader, kHeaderlen), kHeaderlen);

	// read segment lengths from savegame file header
	int len[6];
	for (int i = 0; i < 6; ++i)
		len[i] = cs.word(kFiledata + 2*i);
	if (len[0] != 17)
		::error("Error loading save: description buffer isn't 17 bytes");

	if (slot < 7) {
		engine->readFromSaveFile(data.ptr(kSavenames + 17*slot, len[0]), len[0]);
	} else {
		// The savenames buffer only has room for 7 descriptions
		uint8 namebuf[17];
		engine->readFromSaveFile(namebuf, 17);
	}
	engine->readFromSaveFile(data.ptr(kStartvars, len[1]), len[1]);
	engine->readFromSaveFile(getSegment(data.word(kExtras)).ptr(kExframedata, len[2]), len[2]);
	engine->readFromSaveFile(getSegment(data.word(kBuffers)).ptr(kListofchanges, len[3]), len[3]);
	engine->readFromSaveFile(data.ptr(kMadeuproomdat, len[4]), len[4]);
	engine->readFromSaveFile(cs.ptr(kReelroutines, len[5]), len[5]);

	closeFile();
}

void DreamGenContext::scanForNames() {
	STACK_CHECK;
	dx = data;
	es = dx;
	di = 8579;
	dx = data;
	ds = dx;
	dx = 8698;
	cx = 7;
scanloop:
	push(es);
	push(ds);
	push(di);
	push(dx);
	push(cx);
	openFileFromC();
	if (flags.c())
		goto notexist;
	cx = pop();
	_inc(ch);
	push(cx);
	push(di);
	push(es);
	dx = data;
	ds = dx;
	dx = 6091;
	cx = (6187-6091);
	saveFileRead();
	dx = data;
	es = dx;
	di = 6141;
	ds = pop();
	dx = pop();
	loadSeg();
	bx = data.word(kHandle);
	closeFile();
notexist:
	cx = pop();
	dx = pop();
	di = pop();
	ds = pop();
	es = pop();
	_add(dx, 13);
	_add(di, 17);
	_dec(cl);
	if (!flags.z())
		goto scanloop;
	al = ch;
}

} /*namespace dreamgen */

