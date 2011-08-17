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

void DreamGenContext::loadgame() {
	STACK_CHECK;
	if (data.byte(kCommandtype) != 246) {
		data.byte(kCommandtype) = 246;
		al = 41;
		commandonly();
	}
	if (data.word(kMousebutton) == data.word(kOldbutton))
		return; // "noload"
	if (data.word(kMousebutton) == 1) {
		ax = 0xFFFF;
		doload();
	}
}

// input: ax = savegameId
// if -1, open menu to ask for slot to load
// if >= 0, directly load from that slot
void DreamGenContext::doload() {
	STACK_CHECK;
	int savegameId = (int16)ax;

	data.byte(kLoadingorsave) = 1;

	if (ConfMan.getBool("dreamweb_originalsaveload") && savegameId == -1) {
		showopbox();
		showloadops();
		data.byte(kCurrentslot) = 0;
		showslots();
		shownames();
		data.byte(kPointerframe) = 0;
		worktoscreenm();
		namestoold();
		data.byte(kGetback) = 0;

		while (true) {
			if (data.byte(kQuitrequested))
				return; // "quitloaded"
			delpointer();
			readmouse();
			showpointer();
			vsync();
			dumppointer();
			dumptextline();
			bx = offset_loadlist;
			checkcoords();
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


		// TODO: proper scheme for filename, in a separate function
		//Common::String filename = ConfMan.getActiveDomainName() + Common::String::format(".d%02d", savegameId);
		Common::String filename = Common::String::format("DREAMWEB.D%02d", savegameId);
		debug(1, "Loading from filename: %s", filename.c_str());
		engine->openSaveFileForReading(filename);

		// TODO: The below is duplicated from Loadposition
		data.word(kTimecount) = 0;
		clearchanges();

		ds = cs;
		dx = kFileheader;
		cx = kHeaderlen;
		savefileread();
		es = cs;
		di = kFiledata;
		ax = savegameId;
		if (savegameId < 7) {
			cx = 17;
			_mul(cx);
			ds = data;
			dx = kSavenames;
			_add(dx, ax);
			loadseg();
		} else {
			// For potential support of more than 7 savegame slots,
			// loading into the savenames buffer isn't always possible
			// Emulate a loadseg call:
			uint8 namebuf[17];
			engine->readFromFile(namebuf, 17);
			_add(di, 2);
		}
		ds = data; 
		dx = kStartvars;
		loadseg();
		ds = data.word(kExtras);
		dx = kExframedata;
		loadseg();
		ds = data.word(kBuffers);
		dx = kListofchanges;
		loadseg();
		ds = data;
		dx = kMadeuproomdat;
		loadseg();
		ds = cs;
		dx = kReelroutines;
		loadseg();
		closefile();
		data.byte(kGetback) = 1;
	}

	// kTempgraphics might not have been allocated if we bypassed all menus
	if (data.word(kTempgraphics) != 0xFFFF)
		getridoftemp();

	dx = data;
	es = dx;
	bx = kMadeuproomdat;
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
	if (data.byte(kMandead) == 2) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 247) {
		data.byte(kCommandtype) = 247;
		al = 44;
		commandonly();
	}
	if (data.word(kMousebutton) != 1)
		return;

	data.byte(kLoadingorsave) = 2;

	if (ConfMan.getBool("dreamweb_originalsaveload")) {
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

		while (true) {
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

		// TODO: The below is copied from actualsave
		al = data.byte(kLocation);
		ah = 0;
		cx = 32;
		_mul(cx);
		ds = cs;
		si = kRoomdata;
		_add(si, ax);
		di = kMadeuproomdat;
		bx = di;
		es = cs;
		cx = 16;
		_movsw(cx, true);
		al = data.byte(kRoomssample);
		es.byte(bx+13) = al;
		al = data.byte(kMapx);
		es.byte(bx+15) = al;
		al = data.byte(kMapy);
		es.byte(bx+16) = al;
		al = data.byte(kLiftflag);
		es.byte(bx+20) = al;
		al = data.byte(kManspath);
		es.byte(bx+21) = al;
		al = data.byte(kFacing);
		es.byte(bx+22) = al;
		al = 255;
		es.byte(bx+27) = al;

		// TODO: The below is copied from saveposition
		makeheader();

		//Common::String filename = ConfMan.getActiveDomainName() + Common::String::format(".d%02d", savegameId);
		Common::String filename = Common::String::format("DREAMWEB.D%02d", savegameId);
		debug(1, "Saving to filename: %s (%s)", filename.c_str(), game_description.c_str());

		engine->openSaveFileForWriting(filename.c_str());

		dx = data;
		ds = dx;
		dx = kFileheader;
		cx = kHeaderlen;
		savefilewrite();
		dx = data;
		es = dx;
		di = kFiledata;

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
		if (savegameId < 7) {
			ax = savegameId;
			cx = 17;
			_mul(cx);
			ds = data;
			dx = kSavenames;
			_add(dx, ax);
			memcpy(data.ptr(dx,17), descbuf, 17);
			saveseg();
		} else {
			// savenames only has room for descriptions for 7 slots
			uint16 len = es.word(di);
			_add(di, 2);
			assert(len == 17);
			engine->writeToSaveFile(descbuf, len);
		}

		ds = data;
		dx = kStartvars;
		saveseg();
		ds = data.word(kExtras);
		dx = kExframedata;
		saveseg();
		ds = data.word(kBuffers);
		dx = kListofchanges;
		saveseg();
		ds = data;
		dx = kMadeuproomdat;
		saveseg();
		ds = data;
		dx = kReelroutines;
		saveseg();
		closefile();

		getridoftemp();
		restoreall();
		data.word(kTextaddressx) = 13;
		data.word(kTextaddressy) = 182;
		data.byte(kTextlen) = 240;
		redrawmainscrn();
		worktoscreenm();
		data.byte(kGetback) = 4;
	}
}


} /*namespace dreamgen */
