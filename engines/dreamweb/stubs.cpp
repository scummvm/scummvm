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
#include "engines/util.h"
#include "common/config-manager.h"

namespace DreamGen {

void DreamGenContext::dreamweb() {
	STACK_CHECK;

	switch(engine->getLanguage()) {
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		// Implicit data.byte(kForeignrelease) = 0
		break;
	default:
		data.byte(kForeignrelease) = 1;
		break;
	}

	seecommandtail();
	soundstartup();
	setkeyboardint();
	setupemm();
	allocatebuffers();
	setmouse();
	fadedos();
	gettime();
	clearbuffers();
	clearpalette();
	set16colpalette();
	readsetdata();
	data.byte(kWongame) = 0;

	dx = 1909;
	loadsample();
	setsoundoff();

	bool firstLoop = true;

	int savegameId = Common::ConfigManager::instance().getInt("save_slot");

	while (true) {

		scanfornames();

		bool startNewGame = true;

		if (firstLoop && savegameId >= 0) {

			// loading a savegame requested from launcher/command line

			cls();
			setmode();
			loadpalfromiff();
			clearpalette();

			ax = savegameId;
			doload();
			worktoscreen();
			fadescreenup();
			startNewGame = false;

		} else if (al == 0 && firstLoop) {

			// no savegames found, and we're not restarting.

			setmode();
			loadpalfromiff();

		} else {
			// "dodecisions"

			// Savegames found, so ask if we should load one.
			// (If we're restarting after game over, we also always show these
			// options.)

			cls();
			setmode();
			decide();
			if (data.byte(kQuitrequested))
				return; // exit game

			if (data.byte(kGetback) == 4)
				startNewGame = false; // savegame has been loaded

		}

		firstLoop = false;

		if (startNewGame) {
			// "playgame"

			titles();
			if (data.byte(kQuitrequested))
				return; // exit game
			credits();

			if (data.byte(kQuitrequested))
				return; // exit game

			clearchanges();
			setmode();
			loadpalfromiff();
			data.byte(kLocation) = 255;
			data.byte(kRoomafterdream) = 1;
			data.byte(kNewlocation) = 35;
			data.byte(kVolume) = 7;
			loadroom();
			clearsprites();
			initman();
			entrytexts();
			entryanims();
			data.byte(kDestpos) = 3;
			initialinv();
			data.byte(kLastflag) = 32;
			startup1();
			data.byte(kVolumeto) = 0;
			data.byte(kVolumedirection) = (uint8)-1;
			data.byte(kCommandtype) = 255;

		}

		// main loop
		while (true) {

			if (data.byte(kQuitrequested))
				return; // exit game

			screenupdate();

			if (data.byte(kQuitrequested))
				return; // exit game

			if (data.byte(kWongame) != 0) {
				// "endofgame"
				clearbeforeload();
				fadescreendowns();
				hangon(200);
				endgame();
				quickquit2();
				return;
			}

			if (data.byte(kMandead) == 1 || data.byte(kMandead) == 2)
				break;

			if (data.word(kWatchingtime) > 0) {
				if (data.byte(kFinaldest) == data.byte(kManspath))
					data.word(kWatchingtime)--;
			}

			if (data.word(kWatchingtime) == 0) {
				// "notwatching"

				if (data.byte(kMandead) == 4)
					break;

				if (data.byte(kNewlocation) != 255) {
					// "loadnew"
					clearbeforeload();
					loadroom();
					clearsprites();
					initman();
					entrytexts();
					entryanims();
					data.byte(kNewlocation) = 255;
					startup();
					data.byte(kCommandtype) = 255;
					worktoscreenm();
				}
			}
		}

		// "gameover"
		clearbeforeload();
		showgun();
		fadescreendown();
		hangon(100);

	}
}

static Common::String getFilename(Context &context) {
	const char *name = (const char *)context.cs.ptr(context.dx, 0);
	return Common::String(name);
}

uint8 *DreamGenContext::textUnder() {
	return segRef(data.word(kBuffers)).ptr(kTextunder, 0);
}

uint16 DreamGenContext::standardload(const char *fileName) {
	engine->openFile(fileName);
	engine->readFromFile(cs.ptr(kFileheader, kHeaderlen), kHeaderlen);
	uint16 sizeInBytes = cs.word(kFiledata);
	uint16 result = allocatemem((sizeInBytes + 15) / 16);
	engine->readFromFile(segRef(result).ptr(0, 0), sizeInBytes);
	engine->closeFile();
	return result;
}

void DreamGenContext::standardload() {
	ax = standardload((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadintotemp() {
	loadintotemp((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadintotemp2() {
	loadintotemp2((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadintotemp3() {
	loadintotemp3((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadintotemp(const char *fileName) {
	data.word(kTempgraphics) = standardload(fileName);
}

void DreamGenContext::loadintotemp2(const char *fileName) {
	data.word(kTempgraphics2) = standardload(fileName);
}

void DreamGenContext::loadintotemp3(const char *fileName) {
	data.word(kTempgraphics3) = standardload(fileName);
}

void DreamGenContext::loadtempcharset() {
	loadtempcharset((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadtempcharset(const char *fileName) {
	data.word(kTempcharset) = standardload(fileName);
}

void DreamGenContext::printcurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiget(textUnder(), x, y, 6, height);
	++data.word(kMaintimer);
	if ((data.word(kMaintimer) & 16) == 0)
		showframe((Frame *)segRef(data.word(kTempcharset)).ptr(0, 0), x, y, '/' - 32, 0);
	multidump(x - 6, y, 12, height);
}

void DreamGenContext::delcurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 width = 6;
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiput(textUnder(), x, y, width, height);
	multidump(x, y, width, height);
}

void DreamGenContext::hangoncurs() {
	hangoncurs(cx);
}

void DreamGenContext::hangoncurs(uint16 frameCount) {
	for (uint16 i = 0; i < frameCount; ++i) {
		printcurs();
		vsync();
		delcurs();
	}
}

void DreamGenContext::seecommandtail() {
	data.word(kSoundbaseadd) = 0x220;
	data.byte(kSoundint) = 5;
	data.byte(kSounddmachannel) = 1;
	data.byte(kBrightness) = 1;
	data.word(kHowmuchalloc) = 0x9360;
}

void DreamGenContext::randomnumber() {
	al = engine->randomNumber();
}

void DreamGenContext::quickquit() {
	engine->quit();
}

void DreamGenContext::quickquit2() {
	engine->quit();
}

void DreamGenContext::keyboardread() {
	::error("keyboardread"); //this keyboard int handler, must never be called
}

void DreamGenContext::resetkeyboard() {
}

void DreamGenContext::setkeyboardint() {
}

void DreamGenContext::readfromfile() {
	uint16 dst_offset = dx;
	uint16 size = cx;
	debug(1, "readfromfile(%04x:%u, %u)", (uint16)ds, dst_offset, size);
	ax = engine->readFromFile(ds.ptr(dst_offset, size), size);
	flags._c = false;
}

void DreamGenContext::closefile() {
	engine->closeFile();
	data.byte(kHandle) = 0;
}

void DreamGenContext::openforsave() {
	const char *name = (const char *)ds.ptr(dx, 13);
	debug(1, "openforsave(%s)", name);
	engine->openSaveFileForWriting(name);
}

void DreamGenContext::openfilenocheck() {
	const char *name = (const char *)ds.ptr(dx, 13);
	debug(1, "checksavefile(%s)", name);
	bool ok = engine->openSaveFileForReading(name);
	flags._c = !ok;
}

void DreamGenContext::openfilefromc() {
	openfilenocheck();
}

void DreamGenContext::openfile() {
	Common::String name = getFilename(*this);
	debug(1, "opening file: %s", name.c_str());
	engine->openFile(name);
	cs.word(kHandle) = 1; //only one handle
	flags._c = false;
}

void DreamGenContext::createfile() {
	::error("createfile");
}

void DreamGenContext::dontloadseg() {
	ax = es.word(di);
	_add(di, 2);
	dx = ax;
	cx = 0;
	unsigned pos = engine->skipBytes(dx);
	dx = pos >> 16;
	ax = pos & 0xffff;
	flags._c = false;
}

void DreamGenContext::mousecall() {
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	cx = x;
	dx = y;
	bx = state;
}

void DreamGenContext::readmouse() {
	data.word(kOldbutton) = data.word(kMousebutton);
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	data.word(kMousebutton) = state;
}

void DreamGenContext::readmouse1() {
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	data.word(kMousebutton1) = state;
}

void DreamGenContext::readmouse2() {
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	data.word(kMousebutton2) = state;
}

void DreamGenContext::readmouse3() {
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	data.word(kMousebutton3) = state;
}

void DreamGenContext::readmouse4() {
	data.word(kOldbutton) = data.word(kMousebutton);
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	data.word(kMousebutton) = state | data.word(kMousebutton1) | data.word(kMousebutton2) | data.word(kMousebutton3);
}

void DreamGenContext::setmouse() {
	data.word(kOldpointerx) = 0xffff;
}

void DreamGenContext::dumptextline() {
	if (data.byte(kNewtextline) != 1)
		return;
	data.byte(kNewtextline) = 0;
	uint16 x = data.word(kTextaddressx);
	uint16 y = data.word(kTextaddressy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	multidump(x, y, 228, 13);
}

void DreamGenContext::getundertimed() {
	uint16 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease))
		y -= 3;
	ds = data.word(kBuffers);
	si = kUndertimedtext;
	multiget(ds.ptr(si, 0), data.byte(kTimedx), y, 240, kUndertimedysize);
}

void DreamGenContext::putundertimed() {
	uint16 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease))
		y -= 3;
	ds = data.word(kBuffers);
	si = kUndertimedtext;
	multiput(ds.ptr(si, 0), data.byte(kTimedx), y, 240, kUndertimedysize);
}

void DreamGenContext::usetimedtext() {
	if (data.word(kTimecount) == 0)
		return;
	--data.word(kTimecount);
	if (data.word(kTimecount) == 0) {
		putundertimed();
		data.byte(kNeedtodumptimed) = 1;
		return;
	}

	if (data.word(kTimecount) == data.word(kCounttotimed))
		getundertimed();
	else if (data.word(kTimecount) > data.word(kCounttotimed))
		return;

	es = data.word(kTimedseg);
	si = data.word(kTimedoffset);
	const uint8 *string = es.ptr(si, 0);
	uint16 y = data.byte(kTimedy);
	printdirect(&string, data.byte(kTimedx), &y, 237, true);
	data.byte(kNeedtodumptimed) = 1;
}

void DreamGenContext::setuptimedtemp() {
	setuptimedtemp(al, ah, bl, bh, cx, dx);
}

void DreamGenContext::setuptimedtemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount) {
#if 1 // if cd
	if (voiceIndex != 0) {
		push(ax);
		push(bx);
		push(cx);
		push(dx);
		dl = 'T';
		dh = voiceIndex;
		cl = 'T';
		ah = 0;
		al = textIndex;
		loadspeech();
		if (data.byte(kSpeechloaded) == 1) {
			al = 50+12;
			playchannel1();
		}
		dx = pop();
		cx = pop();
		bx = pop();
		ax = pop();

		// FIXME: This fallthrough does not properly support subtitles+speech
		// mode. The parameters to setuptimedtemp() are sometimes different
		// for speech and for subtitles. See e.g., madmantext()
		if ((data.byte(kSpeechloaded) == 1) && (data.byte(kSubtitles) != 1))
			return;
	}
#endif

	if (data.word(kTimecount) != 0)
		return;
	data.byte(kTimedy) = y;
	data.byte(kTimedx) = x;
	data.word(kCounttotimed) = countToTimed;
	data.word(kTimecount) = timeCount + countToTimed;
	data.word(kTimedseg) = data.word(kTextfile1);
	data.word(kTimedoffset) = kTextstart + segRef(data.word(kTextfile1)).word(textIndex * 2);
	const uint8 *string = segRef(data.word(kTextfile1)).ptr(data.word(kTimedoffset), 0);
	debug(1, "setuptimedtemp: (%d, %d) => '%s'", textIndex, voiceIndex, string);
}

void DreamGenContext::dumptimedtext() {
	if (data.byte(kNeedtodumptimed) != 1)
		return;
	uint8 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;

	multidump(data.byte(kTimedx), y, 240, kUndertimedysize);
	data.byte(kNeedtodumptimed) = 0;
}

void DreamGenContext::gettime() {
	TimeDate t;
	g_system->getTimeAndDate(t);
	debug(1, "\tgettime: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	ch = t.tm_hour;
	cl = t.tm_min;
	dh = t.tm_sec;
	data.byte(kSecondcount) = dh;
	data.byte(kMinutecount) = cl;
	data.byte(kHourcount) = ch;
}

void DreamGenContext::allocatemem() {
	ax = allocatemem(bx);
}

uint16 DreamGenContext::allocatemem(uint16 paragraphs) {
	uint size = (paragraphs + 2) * 16;
	debug(1, "allocate mem, %u bytes", size);
	flags._c = false;
	SegmentRef seg = allocateSegment(size);
	uint16 result = (uint16)seg;
	debug(1, "\tsegment address -> %04x", result);
	return result;
}

void DreamGenContext::deallocatemem() {
	uint16 id = (uint16)es;
	debug(1, "deallocating segment %04x", id);
	deallocateSegment(id);

	//fixing invalid entries in the sprite table
	es = data;
	uint tsize = 16 * 32;
	uint16 bseg = data.word(kBuffers);
	if (!bseg)
		return;
	SegmentRef buffers(this);
	buffers = bseg;
	uint8 *ptr = buffers.ptr(kSpritetable, tsize);
	for(uint i = 0; i < tsize; i += 32) {
		uint16 seg = READ_LE_UINT16(ptr + i + 6);
		//debug(1, "sprite segment = %04x", seg);
		if (seg == id)
			memset(ptr + i, 0xff, 32);
	}
}

void DreamGenContext::removeemm() {
	::error("removeemm");
}

void DreamGenContext::setupemm() {
}

void DreamGenContext::pitinterupt() {
	::error("pitinterupt");
}

void DreamGenContext::getridofpit() {
	::error("getridofpit");
}

void DreamGenContext::setuppit() {
	::error("setuppit");
}

void DreamGenContext::startdmablock() {
	::error("startdmablock");
}

void DreamGenContext::dmaend() {
	::error("dmaend");
}

void DreamGenContext::restoreems() {
	::error("restoreems");
}

void DreamGenContext::saveems() {
	::error("saveems");
}

void DreamGenContext::bothchannels() {
	::error("bothchannels");
}

void DreamGenContext::channel1only() {
	::error("channel1only");
}

void DreamGenContext::channel0only() {
	::error("channel0only");
}

void DreamGenContext::out22c() {
	::error("out22c");
}

void DreamGenContext::soundstartup() {}
void DreamGenContext::soundend() {}
void DreamGenContext::interupttest() {}
void DreamGenContext::disablesoundint() {}
void DreamGenContext::enablesoundint() {}
void DreamGenContext::checksoundint() {
	data.byte(kTestresult) = 1;
}

void DreamGenContext::setsoundoff() {
	warning("setsoundoff: STUB");
}

void DreamGenContext::loadsample() {
	engine->loadSounds(0, (const char *)data.ptr(dx, 13));
}

void DreamGenContext::loadsecondsample() {
	uint8 ch0 = data.byte(kCh0playing);
	if (ch0 >= 12 && ch0 != 255)
		cancelch0();
	uint8 ch1 = data.byte(kCh1playing);
	if (ch1 >= 12)
		cancelch1();
	engine->loadSounds(1, (const char *)data.ptr(dx, 13));
}

void DreamGenContext::loadspeech() {
	cancelch1();
	data.byte(kSpeechloaded) = 0;
	createname();
	const char *name = (const char *)data.ptr(di, 13);
	//warning("name = %s", name);
	if (engine->loadSpeech(name))
		data.byte(kSpeechloaded) = 1;
}

void DreamGenContext::saveseg() {
	cx = es.word(di);
	_add(di, 2);
	savefilewrite();
}

void DreamGenContext::savefilewrite() {
	ax = engine->writeToSaveFile(ds.ptr(dx, cx), cx);
}

void DreamGenContext::savefileread() {
	ax = engine->readFromSaveFile(ds.ptr(dx, cx), cx);
}

void DreamGenContext::loadseg() {
	ax = es.word(di);
	di += 2;

	uint16 dst_offset = dx;
	uint16 size = ax;

	debug(1, "loadseg(%04x:%u, %u)", (uint16)ds, dst_offset, size);
	ax = engine->readFromFile(ds.ptr(dst_offset, size), size);
	flags._c = false;
}

void DreamGenContext::error() {
	::error("error");
}

void DreamGenContext::generalerror() {
	::error("generalerror");
}

void DreamGenContext::dosreturn() {

	_cmp(data.byte(kCommandtype), 250);
	if (!flags.z()) {
		data.byte(kCommandtype) = 250;
		al = 46;
		commandonly();
	}

	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return;

	data.word(kMousebutton) = 0;
	engine->quit();
}

void DreamGenContext::set16colpalette() {
}

void DreamGenContext::mode640x480() {
	// Video mode 12h: 640x480 pixels, 16 colors, I believe
	al = 0x12 + 128;
	ah = 0;
	initGraphics(640, 480, true);
}

void DreamGenContext::showgroup() {
	engine->setPalette();
}

void DreamGenContext::fadedos() {
	engine->fadeDos();
}

void DreamGenContext::eraseoldobs() {
	if (data.byte(kNewobs) == 0)
		return;

	Sprite *sprites = spritetable();
	for (size_t i = 0; i < 16; ++i) {
		Sprite &sprite = sprites[i];
		if (sprite.objData() != 0xffff) {
			memset(&sprite, 0xff, sizeof(Sprite));
		}
	}
}

void DreamGenContext::modifychar() {
	al = engine->modifyChar(al);
}

void DreamGenContext::lockmon() {
	// Pressing space pauses text output in the monitor. We use the "hard"
	// key because calling readkey() drains characters from the input
	// buffer, we we want the user to be able to type ahead while the text
	// is being printed.
	if (data.byte(kLasthardkey) == 57) {
		// Clear the keyboard buffer. Otherwise the space that caused
		// the pause will be read immediately unpause the game.
		do {
			readkey();
		} while (data.byte(kCurrentkey) != 0);

		locklighton();
		while (!engine->shouldQuit()) {
			engine->waitForVSync();
			readkey();
			if (data.byte(kCurrentkey) == ' ')
				break;
		}
		// Forget the last "hard" key, otherwise the space that caused
		// the unpausing will immediately re-pause the game.
		data.byte(kLasthardkey) = 0;
		locklightoff();
	}
}

void DreamGenContext::cancelch0() {
	data.byte(kCh0repeat) = 0;
	data.word(kCh0blockstocopy) = 0;
	data.byte(kCh0playing) = 255;
	engine->stopSound(0);
}

void DreamGenContext::cancelch1() {
	data.word(kCh1blockstocopy) = 0;
	data.byte(kCh1playing) = 255;
	engine->stopSound(1);
}

void DreamGenContext::makebackob(SetObject *objData) {
	if (data.byte(kNewobs) == 0)
		return;
	uint8 priority = objData->priority;
	uint8 type = objData->type;
	Sprite *sprite = makesprite(data.word(kObjectx), data.word(kObjecty), addr_backobject, data.word(kSetframes), 0);

	uint16 objDataOffset = (uint8 *)objData - segRef(data.word(kSetdat)).ptr(0, 0);
	assert(objDataOffset % sizeof(SetObject) == 0);
	assert(objDataOffset < 128 * sizeof(SetObject));
	sprite->setObjData(objDataOffset);
	if (priority == 255)
		priority = 0;
	sprite->priority = priority;
	sprite->type = type;
	sprite->b16 = 0;
	sprite->delay = 0;
	sprite->frame = 0;
}

void DreamGenContext::getroomdata() {
	Room *room = getroomdata(al);
	bx = (uint8 *)room - cs.ptr(0, 0);
}

Room *DreamGenContext::getroomdata(uint8 room) {
	return (Room *)cs.ptr(kRoomdata, 0) + room;
}

void DreamGenContext::readheader() {
	ax = engine->readFromFile(cs.ptr(kFileheader, kHeaderlen), kHeaderlen);
	es = cs;
	di = kFiledata;
}

void DreamGenContext::startloading(const Room *room) {
	data.byte(kCombatcount) = 0;
	data.byte(kRoomssample) = room->roomsSample;
	data.byte(kMapx) = room->mapX;
	data.byte(kMapy) = room->mapY;
	data.byte(kLiftflag) = room->liftFlag;
	data.byte(kManspath) = room->b21;
	data.byte(kDestination) = room->b21;
	data.byte(kFinaldest) = room->b21;
	data.byte(kFacing) = room->b22;
	data.byte(kTurntoface) = room->b22;
	data.byte(kCounttoopen) = room->countToOpen;
	data.byte(kLiftpath) = room->liftPath;
	data.byte(kDoorpath) = room->doorPath;
	data.byte(kLastweapon) = (uint8)-1;
	al = room->b27;
	push(ax);
	al = room->b31;
	ah = data.byte(kReallocation);
	data.byte(kReallocation) = al;
	Common::String name = room->name;
	engine->openFile(name);
	cs.word(kHandle) = 1; //only one handle
	flags._c = false;
	readheader();
	allocateload();
	ds = ax;
	data.word(kBackdrop) = ax;
	dx = kFlags;
	loadseg();
	ds = data.word(kWorkspace);
	dx = kMap;
	cx = 132*66;
	al = 0;
	fillspace();
	loadseg();
	sortoutmap();
	allocateload();
	data.word(kSetframes) = ax;
	ds = ax;
	dx = kFramedata;
	loadseg();
	ds = data.word(kSetdat);
	dx = 0;
	cx = kSetdatlen;
	al = 255;
	fillspace();
	loadseg();
	allocateload();
	data.word(kReel1) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel2) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel3) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReels) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kPeople) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kSetdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kBlockdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kRoomdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kFreeframes) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	ds = data.word(kFreedat);
	dx = 0;
	cx = kFreedatlen;
	al = 255;
	fillspace();
	loadseg();
	allocateload();
	data.word(kFreedesc) = ax;
	ds = ax;
	dx = kFreetextdat;
	loadseg();
	closefile();
	findroominloc();
	deletetaken();
	setallchanges();
	autoappear();
	Room *newRoom = getroomdata(data.byte(kNewlocation));
	bx = (uint8 *)newRoom - cs.ptr(0, 0);
	data.byte(kLastweapon) = (uint8)-1;
	data.byte(kMandead) = 0;
	data.word(kLookcounter) = 160;
	data.byte(kNewlocation) = 255;
	data.byte(kLinepointer) = 254;
	ax = pop();
	if (al != 255) {
		data.byte(kManspath) = al;
		push(bx);
		autosetwalk();
		bx = pop();
	}
	findxyfrompath();
}

void DreamGenContext::fillspace() {
	memset(ds.ptr(dx, cx), al, cx);
}

void DreamGenContext::dealwithspecial(uint8 firstParam, uint8 secondParam) {
	uint8 type = firstParam - 220;
	if (type == 0) {
		placesetobject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 1) {
		removesetobject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 2) {
		al = secondParam;
		placefreeobject();
		data.byte(kHavedoneobs) = 1;
	} else if (type == 3) {
		al = secondParam;
		removefreeobject();
		data.byte(kHavedoneobs) = 1;
	} else if (type == 4) {
		switchryanoff();
	} else if (type == 5) {
		data.byte(kTurntoface) = secondParam;
		data.byte(kFacing) = secondParam;
		switchryanon();
	} else if (type == 6) {
		data.byte(kNewlocation) = secondParam;
	} else {
		movemap(secondParam);
	}
}

void DreamGenContext::plotreel() {
	Reel *reel = getreelstart();
	while (reel->x >= 220 && reel->x != 255) {
		dealwithspecial(reel->x, reel->y);
		++data.word(kReelpointer);
		reel += 8;
	}

	for (size_t i = 0; i < 8; ++i) {
		if (reel->frame() != 0xffff)
			showreelframe(reel);
		++reel;
	}
	push(es);
	push(bx);
	soundonreels();
	bx = pop();
	es = pop();
}

void DreamGenContext::crosshair() {
	uint8 frame;
	if ((data.byte(kCommandtype) != 3) && (data.byte(kCommandtype) < 10)) {
		frame = 9;
	} else {
		frame = 29;
	}
	const Frame *src = (const Frame *)segRef(data.word(kIcons1)).ptr(0, 0);
	showframe(src, kZoomx + 24, kZoomy + 19, frame, 0);
}

void DreamGenContext::deltextline() {
	uint16 x = data.word(kTextaddressx);
	uint16 y = data.word(kTextaddressy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	multiput(textUnder(), x, y, kUndertextsizex, kUndertextsizey);
}

void DreamGenContext::commandonly() {
	commandonly(al);	
}

void DreamGenContext::commandonly(uint8 command) {
	deltextline();
	uint16 index = command * 2;
	uint16 offset = kTextstart + segRef(data.word(kCommandtext)).word(index);
	uint16 y = data.word(kTextaddressy);
	const uint8 *string = segRef(data.word(kCommandtext)).ptr(offset, 0);
	printdirect(&string, data.word(kTextaddressx), &y, data.byte(kTextlen), (bool)(data.byte(kTextlen) & 1));
	data.byte(kNewtextline) = 1;
}

void DreamGenContext::checkifperson() {
	flags._z = !checkifperson(al, ah);
}

bool DreamGenContext::checkifperson(uint8 x, uint8 y) {
	People *people = (People *)segRef(data.word(kBuffers)).ptr(kPeoplelist, 0);

	for (size_t i = 0; i < 12; ++i, ++people) {
		if (people->b4 == 255)
			continue;
		data.word(kReelpointer) = people->reelPointer();
		Reel *reel = getreelstart();
		if (reel->frame() == 0xffff)
			++reel;
		const Frame *frame = getreelframeax(reel->frame());
		uint8 xmin = reel->x + frame->x;
		uint8 ymin = reel->y + frame->y;
		uint8 xmax = xmin + frame->width;
		uint8 ymax = ymin + frame->height;
		if (x < xmin)
			continue;
		if (y < ymin)
			continue;
		if (x >= xmax)
			continue;
		if (y >= ymax)
			continue;
		data.word(kPersondata) = people->routinePointer();
		obname(people->b4, 5);
		return true;
	}
	return false;
}

void DreamGenContext::checkiffree() {
	flags._z = !checkiffree(al, ah);
}

bool DreamGenContext::checkiffree(uint8 x, uint8 y) {
	const ObjPos *freeList = (const ObjPos *)segRef(data.word(kBuffers)).ptr(kFreelist, 80 * sizeof(ObjPos));
	for (size_t i = 0; i < 80; ++i) {
		const ObjPos *objPos = freeList + 79 - i;
		if (objPos->index == 0xff)
			continue;
		if (x < objPos->xMin)
			continue;
		if (x >= objPos->xMax)
			continue;
		if (y < objPos->yMin)
			continue;
		if (y >= objPos->yMax)
			continue;
		obname(objPos->index, 2);
		return true;
	}
	return false;
}

void DreamGenContext::checkifex() {
	flags._z = !checkifex(al, ah);
}

bool DreamGenContext::checkifex(uint8 x, uint8 y) {
	const ObjPos *exList = (const ObjPos *)segRef(data.word(kBuffers)).ptr(kExlist, 100 * sizeof(ObjPos));
	for (size_t i = 0; i < 100; ++i) {
		const ObjPos *objPos = exList + 99 - i;
		if (objPos->index == 0xff)
			continue;
		if (x < objPos->xMin)
			continue;
		if (x >= objPos->xMax)
			continue;
		if (y < objPos->yMin)
			continue;
		if (y >= objPos->yMax)
			continue;
		obname(objPos->index, 4);
		return true;
	}
	return false;
}

const uint8 *DreamGenContext::findobname(uint8 type, uint8 index) {
	if (type == 5) {
		uint16 i = 64 * 2 * (index & 127);
		uint16 offset = segRef(data.word(kPeople)).word(kPersontxtdat + i) + kPersontext;
		return segRef(data.word(kPeople)).ptr(offset, 0);
	} else if (type == 4) {
		uint16 offset = segRef(data.word(kExtras)).word(kExtextdat + index * 2) + kExtext;
		return segRef(data.word(kExtras)).ptr(offset, 0);
	} else if (type == 2) {
		uint16 offset = segRef(data.word(kFreedesc)).word(kFreetextdat + index * 2) + kFreetext;
		return segRef(data.word(kFreedesc)).ptr(offset, 0);
	} else if (type == 1) {
		uint16 offset = segRef(data.word(kSetdesc)).word(kSettextdat + index * 2) + kSettext;
		return segRef(data.word(kSetdesc)).ptr(offset, 0);
	} else {
		uint16 offset = segRef(data.word(kBlockdesc)).word(kBlocktextdat + index * 2) + kBlocktext;
		return segRef(data.word(kBlockdesc)).ptr(offset, 0);
	}
}

void DreamGenContext::copyname() {
	copyname(ah, al, cs.ptr(di, 0));
}

void DreamGenContext::copyname(uint8 type, uint8 index, uint8 *dst) {
	const uint8 *src = findobname(type, index);
	size_t i;
	for (i = 0; i < 28; ++i) { 
		char c = src[i];
		if (c == ':')
			break;
		if (c == 0)
			break;
		dst[i] = c;
	}
	dst[i] = 0;
}

void DreamGenContext::commandwithob() {
	commandwithob(al, bh, bl); 
}

void DreamGenContext::commandwithob(uint8 command, uint8 type, uint8 index) {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";
	deltextline();
	uint16 commandText = kTextstart + segRef(data.word(kCommandtext)).word(command * 2);
	uint8 textLen = data.byte(kTextlen);
	{
		const uint8 *string = segRef(data.word(kCommandtext)).ptr(commandText, 0);
		printdirect(string, data.word(kTextaddressx), data.word(kTextaddressy), textLen, (bool)(textLen & 1));
	}
	copyname(type, index, commandLine);
	uint16 x = data.word(kLastxpos);
	if (command != 0)
		x += 5;
	printdirect(commandLine, x, data.word(kTextaddressy), textLen, (bool)(textLen & 1));
	data.byte(kNewtextline) = 1;
}

void DreamGenContext::examineobtext() {
	commandwithob(1, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamGenContext::showpanel() {
	Frame *frame = (Frame *)segRef(data.word(kIcons1)).ptr(0, sizeof(Frame));
	showframe(frame, 72, 0, 19, 0);
	showframe(frame, 192, 0, 19, 0);
}

void DreamGenContext::blocknametext() {
	commandwithob(0, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamGenContext::personnametext() {
	commandwithob(2, data.byte(kCommandtype), data.byte(kCommand) & 127);
}

void DreamGenContext::walktotext() {
	commandwithob(3, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamGenContext::findormake() {
	uint8 b0 = al;
	uint8 b2 = cl;
	uint8 b3 = ch;
	findormake(b0, b2, b3);
}

void DreamGenContext::findormake(uint8 index, uint8 value, uint8 type) {
	Change *change = (Change *)segRef(data.word(kBuffers)).ptr(kListofchanges, sizeof(Change));
	for (; change->index != 0xff; ++change) {
		if (index == change->index && data.byte(kReallocation) == change->location && type == change->type) {
			change->value = value;
			return;
		}
	}

	change->index = index;
	change->location = data.byte(kReallocation);
	change->value = value;
	change->type = type;
}

void DreamGenContext::setallchanges() {
	Change *change = (Change *)segRef(data.word(kBuffers)).ptr(kListofchanges, sizeof(Change));
	for (; change->index != 0xff; ++change)
		if (change->location == data.byte(kReallocation))
			dochange(change->index, change->value, change->type);
}

DynObject *DreamGenContext::getfreead(uint8 index) {
	return (DynObject *)segRef(data.word(kFreedat)).ptr(0, 0) + index;
}

DynObject *DreamGenContext::getexad(uint8 index) {
	return (DynObject *)segRef(data.word(kExtras)).ptr(kExdata, 0) + index;
}

DynObject *DreamGenContext::geteitheradCPP() {
	if (data.byte(kObjecttype) == 4)
		return getexad(data.byte(kItemframe));
	else
		return getfreead(data.byte(kItemframe));
}

void *DreamGenContext::getanyad(uint8 *value1, uint8 *value2) {
	if (data.byte(kObjecttype) == 4) {
		DynObject *exObject = getexad(data.byte(kCommand));
		*value1 = exObject->b7;
		*value2 = exObject->b8;
		return exObject;
	} else if (data.byte(kObjecttype) == 2) {
		DynObject *freeObject = getfreead(data.byte(kCommand));
		*value1 = freeObject->b7;
		*value2 = freeObject->b8;
		return freeObject;
	} else {
		SetObject *setObject = getsetad(data.byte(kCommand));
		*value1 = setObject->b4;
		*value2 = setObject->priority;
		return setObject;
	}
}

void *DreamGenContext::getanyaddir(uint8 index, uint8 flag) {
	if (flag == 4)
		return getexad(index);
	else if (flag == 2)
		return getfreead(index);
	else
		return getsetad(index);
}

SetObject *DreamGenContext::getsetad(uint8 index) {
	return (SetObject *)segRef(data.word(kSetdat)).ptr(0, 0) + index;
}

void DreamGenContext::dochange() {
	dochange(al, cl, ch);
}

void DreamGenContext::dochange(uint8 index, uint8 value, uint8 type) {
	if (type == 0) { //object
		getsetad(index)->mapad[0] = value;
	} else if (type == 1) { //freeobject
		DynObject *freeObject = getfreead(index);
		if (freeObject->mapad[0] == 0xff)
			freeObject->mapad[0] = value;
	} else { //path
		bx = kPathdata + (type - 100) * 144 + index * 8;
		es = data.word(kReels);
		es.byte(bx+6) = value;
	}
}

void DreamGenContext::deletetaken() {
	const DynObject *extraObjects = (const DynObject *)segRef(data.word(kExtras)).ptr(kExdata, 0);
	DynObject *freeObjects = (DynObject *)segRef(data.word(kFreedat)).ptr(0, 0);
	for(size_t i = 0; i < kNumexobjects; ++i) {
		uint8 location = extraObjects[i].initialLocation;
		if (location == data.byte(kReallocation)) {
			uint8 index = extraObjects[i].index;
			freeObjects[index].mapad[0] = 0xfe;
		}
	}
}

void DreamGenContext::getexpos() {
	es = data.word(kExtras);
	const DynObject *objects = (const DynObject *)segRef(data.word(kExtras)).ptr(kExdata, sizeof(DynObject));
	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (objects[i].mapad[0] == 0xff) {
			data.byte(kExpos) = i;
			di = kExdata + i * sizeof(DynObject);
			return;
		}
	}
	data.byte(kExpos) = kNumexobjects;
	di = kExdata + kNumexobjects * sizeof(DynObject);
}

void DreamGenContext::placesetobject() {
	placesetobject(al);
}

void DreamGenContext::placesetobject(uint8 index) {
	findormake(index, 0, 0);
	getsetad(index)->mapad[0] = 0;
}

void DreamGenContext::removesetobject() {
	removesetobject(al);
}

void DreamGenContext::removesetobject(uint8 index) {
	findormake(index, 0xff, 0);
	getsetad(index)->mapad[0] = 0xff;
}

void DreamGenContext::finishedwalking() {
	flags._z = finishedwalkingCPP();
}

bool DreamGenContext::finishedwalkingCPP() {
	return (data.byte(kLinepointer) == 254) && (data.byte(kFacing) == data.byte(kTurntoface));
}

void DreamGenContext::getflagunderp() {
	uint8 flag, flagEx;
	getflagunderp(&flag, &flagEx);
	cl = flag;
	ch = flagEx;
}

void DreamGenContext::getflagunderp(uint8 *flag, uint8 *flagEx) {
	uint8 type, flagX, flagY;
	checkone(data.word(kMousex) - data.word(kMapadx), data.word(kMousey) - data.word(kMapady), flag, flagEx, &type, &flagX, &flagY);
	cl = data.byte(kLastflag) = *flag;
	ch = data.byte(kLastflagex) = *flagEx;
}

void DreamGenContext::walkandexamine() {
	if (!finishedwalkingCPP())
		return;
	data.byte(kCommandtype) = data.byte(kWalkexamtype);
	data.byte(kCommand) = data.byte(kWalkexamnum);
	data.byte(kWalkandexam) = 0;
	if (data.byte(kCommandtype) != 5)
		examineob();
}

void DreamGenContext::obname() {
	obname(al, ah);
}

void DreamGenContext::obname(uint8 command, uint8 commandType) {
	if (data.byte(kReasseschanges) == 0) {
		if ((commandType == data.byte(kCommandtype)) && (command == data.byte(kCommand))) {
			if (data.byte(kWalkandexam) == 1) {
				walkandexamine();
				return;
			} else if (data.word(kMousebutton) == 0)
				return;
			else if ((data.byte(kCommandtype) == 3) && (data.byte(kLastflag) < 2))
				return;
			else if ((data.byte(kManspath) != data.byte(kPointerspath)) || (data.byte(kCommandtype) == 3)) {
				setwalk();
				data.byte(kReasseschanges) = 1;
				return;
			} else if (! finishedwalkingCPP())
				return;
			else if (data.byte(kCommandtype) == 5) {
				if (data.word(kWatchingtime) == 0)
					talk();
				return;
			} else {
				if (data.word(kWatchingtime) == 0)
					examineob();
				return;
			}
		}
	} else
		data.byte(kReasseschanges) = 0;

	data.byte(kCommand) = command;
	data.byte(kCommandtype) = commandType;
	if ((data.byte(kLinepointer) != 254) || (data.word(kWatchingtime) != 0) || (data.byte(kFacing) != data.byte(kTurntoface))) {
		blocknametext();
		return;
	} else if (data.byte(kCommandtype) != 3) {
		if (data.byte(kManspath) != data.byte(kPointerspath)) {
			walktotext();
			return;
		} else if (data.byte(kCommandtype) == 3) {
			blocknametext();
			return;
		} else if (data.byte(kCommandtype) == 5) {
			personnametext();
			return;
		} else {
			examineobtext();
			return;
		}
	}
	if (data.byte(kManspath) == data.byte(kPointerspath)) {
		uint8 flag, flagEx, type, flagX, flagY;
		checkone(data.byte(kRyanx) + 12, data.byte(kRyany) + 12, &flag, &flagEx, &type, &flagX, &flagY);
		if (flag < 2) {
			blocknametext();
			return;
		}
	}

	getflagunderp();
	if (data.byte(kLastflag) < 2) {
		blocknametext();
		return;
	} else if (data.byte(kLastflag) >= 128) {
		blocknametext();
		return;
	} else {
		walktotext();
		return;
	}
}

void DreamGenContext::delpointer() {
	if (data.word(kOldpointerx) == 0xffff)
		return;
	data.word(kDelherex) = data.word(kOldpointerx);
	data.word(kDelherey) = data.word(kOldpointery);
	data.byte(kDelxs) = data.byte(kPointerxs);
	data.byte(kDelys) = data.byte(kPointerys);
	multiput(segRef(data.word(kBuffers)).ptr(kPointerback, 0), data.word(kDelherex), data.word(kDelherey), data.byte(kPointerxs), data.byte(kPointerys));
}

void DreamGenContext::showblink() {
	if (data.byte(kManisoffscreen) == 1)
		return;
	++data.byte(kBlinkcount);
	if (data.byte(kShadeson) != 0)
		return;
	if (data.byte(kReallocation) >= 50) // eyesshut
		return;
	if (data.byte(kBlinkcount) != 3)
		return;
	data.byte(kBlinkcount) = 0;
	uint8 blinkFrame = data.byte(kBlinkframe);
	++blinkFrame; // Implicit %256
	data.byte(kBlinkframe) = blinkFrame;
	if (blinkFrame > 6)
		blinkFrame = 6;
	static const uint8 blinkTab[] = { 16,18,18,17,16,16,16 };
	uint8 width, height;
	showframe((Frame *)segRef(data.word(kIcons1)).ptr(0, 0), 44, 32, blinkTab[blinkFrame], 0, &width, &height);
}

void DreamGenContext::dumpblink() {
	if (data.byte(kShadeson) != 0)
		return;
	if (data.byte(kBlinkcount) != 0)
		return;
	if (data.byte(kBlinkframe) >= 6)
		return;
	multidump(44, 32, 16, 12);
}

void DreamGenContext::dumppointer() {
	dumpblink();
	multidump(data.word(kDelherex), data.word(kDelherey), data.byte(kDelxs), data.byte(kDelys));
	if ((data.word(kOldpointerx) != data.word(kDelherex)) || (data.word(kOldpointery) != data.word(kDelherey)))
		multidump(data.word(kOldpointerx), data.word(kOldpointery), data.byte(kPointerxs), data.byte(kPointerys));
}

void DreamGenContext::checkcoords() {

	// FIXME: Move all these lists to the callers

	switch ((uint16)bx) {
	case offset_talklist: {
		RectWithCallback talklist[] = {
			{ 273,320,157,198,&DreamGenContext::getback1 },
			{ 240,290,2,44,&DreamGenContext::moretalk },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(talklist);
		break;
	}
	case offset_quitlist: {
		RectWithCallback quitlist[] = {
			{ 273,320,157,198,&DreamGenContext::getback1 },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(quitlist);
		break;
	}
	case offset_destlist: {
		RectWithCallback destlist[] = {
			{ 238,258,4,44,&DreamGenContext::nextdest },
			{ 104,124,4,44,&DreamGenContext::lastdest },
			{ 280,308,4,44,&DreamGenContext::lookatplace },
			{ 104,216,138,192,&DreamGenContext::destselect },
			{ 273,320,157,198,&DreamGenContext::getback1 },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(destlist);
		break;
	}
	case offset_keypadlist: {
		RectWithCallback keypadlist[] = {
			{ kKeypadx+9,kKeypadx+30,kKeypady+9,kKeypady+22,&DreamGenContext::buttonone },
			{ kKeypadx+31,kKeypadx+52,kKeypady+9,kKeypady+22,&DreamGenContext::buttontwo },
			{ kKeypadx+53,kKeypadx+74,kKeypady+9,kKeypady+22,&DreamGenContext::buttonthree },
			{ kKeypadx+9,kKeypadx+30,kKeypady+23,kKeypady+40,&DreamGenContext::buttonfour },
			{ kKeypadx+31,kKeypadx+52,kKeypady+23,kKeypady+40,&DreamGenContext::buttonfive },
			{ kKeypadx+53,kKeypadx+74,kKeypady+23,kKeypady+40,&DreamGenContext::buttonsix },
			{ kKeypadx+9,kKeypadx+30,kKeypady+41,kKeypady+58,&DreamGenContext::buttonseven },
			{ kKeypadx+31,kKeypadx+52,kKeypady+41,kKeypady+58,&DreamGenContext::buttoneight },
			{ kKeypadx+53,kKeypadx+74,kKeypady+41,kKeypady+58,&DreamGenContext::buttonnine },
			{ kKeypadx+9,kKeypadx+30,kKeypady+59,kKeypady+73,&DreamGenContext::buttonnought },
			{ kKeypadx+31,kKeypadx+74,kKeypady+59,kKeypady+73,&DreamGenContext::buttonenter },
			{ kKeypadx+72,kKeypadx+86,kKeypady+80,kKeypady+94,&DreamGenContext::quitkey },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(keypadlist);
		break;
	}
	case offset_menulist: {
		RectWithCallback menulist[] = {
			{ kMenux+54,kMenux+68,kMenuy+72,kMenuy+88,&DreamGenContext::quitkey },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(menulist);
		break;
	}
	case offset_folderlist: {
		RectWithCallback folderlist[] = {
			{ 280,320,160,200,&DreamGenContext::quitkey },
			{ 143,300,6,194,&DreamGenContext::nextfolder },
			{ 0,143,6,194,&DreamGenContext::lastfolder },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(folderlist);
		break;
	}
	case offset_symbollist: {
		RectWithCallback symbollist[] = {
			{ kSymbolx+40,kSymbolx+64,kSymboly+2,kSymboly+16,&DreamGenContext::quitsymbol },
			{ kSymbolx,kSymbolx+52,kSymboly+20,kSymboly+50,&DreamGenContext::settopleft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+20,kSymboly+50,&DreamGenContext::settopright },
			{ kSymbolx,kSymbolx+52,kSymboly+50,kSymboly+80,&DreamGenContext::setbotleft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+50,kSymboly+80,&DreamGenContext::setbotright },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(symbollist);

		break;
	}
	case offset_diarylist: {
		RectWithCallback diarylist[] = {
			{ kDiaryx+94,kDiaryx+110,kDiaryy+97,kDiaryy+113,&DreamGenContext::diarykeyn },
			{ kDiaryx+151,kDiaryx+167,kDiaryy+71,kDiaryy+87,&DreamGenContext::diarykeyp },
			{ kDiaryx+176,kDiaryx+192,kDiaryy+108,kDiaryy+124,&DreamGenContext::quitkey },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(diarylist);
		break;
	}
	case offset_opslist: {
		RectWithCallback opslist[] = {
			{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamGenContext::getbackfromops },
			{ kOpsx+10,kOpsx+77,kOpsy+10,kOpsy+59,&DreamGenContext::dosreturn },
			{ kOpsx+128,kOpsx+190,kOpsy+16,kOpsy+100,&DreamGenContext::discops },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(opslist);
		break;
	}
	case offset_discopslist: {
		RectWithCallback discopslist[] = {
			{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamGenContext::loadgame },
			{ kOpsx+10,kOpsx+79,kOpsy+10,kOpsy+59,&DreamGenContext::savegame },
			{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamGenContext::getbacktoops },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(discopslist);
		break;
	}
	case offset_decidelist: {
		RectWithCallback decidelist[] = {
			{ kOpsx+69,kOpsx+124,kOpsy+30,kOpsy+76,&DreamGenContext::newgame },
			{ kOpsx+20,kOpsx+87,kOpsy+10,kOpsy+59,&DreamGenContext::dosreturn },
			{ kOpsx+123,kOpsx+190,kOpsy+10,kOpsy+59,&DreamGenContext::loadold },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(decidelist);
		break;
	}
	default:
		::error("Unimplemented checkcoords() call");
	}
}


void DreamGenContext::checkcoords(const RectWithCallback *rectWithCallbacks) {
	if (data.byte(kNewlocation) != 0xff)
		return;

	const RectWithCallback *r;
	for (r = rectWithCallbacks; r->_xMin != 0xffff; ++r) {
		if (r->contains(data.word(kMousex), data.word(kMousey))) {
			(this->*(r->_callback))();
			return;
		}
	}
}


void DreamGenContext::showpointer() {
	showblink();
	const Frame *icons1 = ((const Frame *)segRef(data.word(kIcons1)).ptr(0, 0));
	uint16 x = data.word(kMousex);
	data.word(kOldpointerx) = data.word(kMousex);
	uint16 y = data.word(kMousey);
	data.word(kOldpointery) = data.word(kMousey);
	if (data.byte(kPickup) == 1) {
		const Frame *frames;
		if (data.byte(kObjecttype) != 4)
			frames = (const Frame *)segRef(data.word(kFreeframes)).ptr(0, 0);
		else
			frames = (const Frame *)segRef(data.word(kExtras)).ptr(0, 0);
		const Frame *frame = frames + (3 * data.byte(kItemframe) + 1);
		uint8 width = frame->width;
		uint8 height = frame->height;
		if (width < 12)
			width = 12;
		if (height < 12)
			height = 12;
		data.byte(kPointerxs) = width;
		data.byte(kPointerys) = height;
		uint16 xMin = (x >= width / 2) ? x - width / 2 : 0;
		uint16 yMin = (y >= height / 2) ? y - height / 2 : 0;
		data.word(kOldpointerx) = xMin;
		data.word(kOldpointery) = yMin;
		multiget(segRef(data.word(kBuffers)).ptr(kPointerback, 0), xMin, yMin, width, height);
		showframe(frames, x, y, 3 * data.byte(kItemframe) + 1, 128);
		showframe(icons1, x, y, 3, 128);
	} else {
		const Frame *frame = icons1 + (data.byte(kPointerframe) + 20);
		uint8 width = frame->width;
		uint8 height = frame->height;
		if (width < 12)
			width = 12;
		if (height < 12)
			height = 12;
		data.byte(kPointerxs) = width;
		data.byte(kPointerys) = height;
		multiget(segRef(data.word(kBuffers)).ptr(kPointerback, 0), x, y, width, height);
		showframe(icons1, x, y, data.byte(kPointerframe) + 20, 0);
	}
}

void DreamGenContext::animpointer() {

	if (data.byte(kPointermode) == 2) {
		data.byte(kPointerframe) = 0;
		if ((data.byte(kReallocation) == 14) && (data.byte(kCommandtype) == 211))
			data.byte(kPointerframe) = 5;
		return;
	} else if (data.byte(kPointermode) == 3) {
		if (data.byte(kPointerspeed) != 0) {
			--data.byte(kPointerspeed);
		} else {
			data.byte(kPointerspeed) = 5;
			++data.byte(kPointercount);
			if (data.byte(kPointercount) == 16)
				data.byte(kPointercount) = 0;
		}
		static const uint8 flashMouseTab[] = { 1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2 };
		data.byte(kPointerframe) = flashMouseTab[data.byte(kPointercount)];
		return;
	}
	if (data.word(kWatchingtime) != 0) {
		data.byte(kPointerframe) = 11;
		return;
	}
	data.byte(kPointerframe) = 0;
	if (data.byte(kInmaparea) == 0)
		return;
	if (data.byte(kPointerfirstpath) == 0)
		return;
	uint8 flag, flagEx;
	getflagunderp(&flag, &flagEx);
	if (flag < 2)
		return;
	if (flag >= 128)
		return;
	if (flag & 4) {
		data.byte(kPointerframe) = 3;
		return;
	}
	if (flag & 16) {
		data.byte(kPointerframe) = 4;
		return;
	}
	if (flag & 2) {
		data.byte(kPointerframe) = 5;
		return;
	}
	if (flag & 8) {
		data.byte(kPointerframe) = 6;
		return;
	}
	data.byte(kPointerframe) = 8;
}

void DreamGenContext::printmessage() {
	printmessage(di, bx, al, dl, (bool)(dl & 1));
}

void DreamGenContext::printmessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered) {
	uint16 offset = kTextstart + segRef(data.word(kCommandtext)).word(index * 2);
	const uint8 *string = segRef(data.word(kCommandtext)).ptr(offset, 0);
	printdirect(&string, x, &y, maxWidth, centered);
}

void DreamGenContext::compare() {
	char id[4] = { cl, ch, dl, dh };
	flags._z = compare(al, ah, id);
}

bool DreamGenContext::compare(uint8 index, uint8 flag, const char id[4]) {
	void *ptr = getanyaddir(index, flag);
	const char *objId = (const char *)(((const uint8 *)ptr) + 12); // whether it is a DynObject or a SetObject
	for (size_t i = 0; i < 4; ++i) {
		if(id[i] != objId[i] + 'A')
			return false;
	}
	return true;
}

bool DreamGenContext::isitdescribed(const ObjPos *pos) {
	uint16 offset = segRef(data.word(kSetdesc)).word(kSettextdat + pos->index * 2);
	uint8 result = segRef(data.word(kSetdesc)).byte(kSettext + offset);
	return result != 0;
}

bool DreamGenContext::isCD() {
	// The original sources has two codepaths depending if the game is 'if cd' or not
	// This is a hack to guess which version to use with the assumption that if we have a cd version
	// we managed to load the speech. At least it is isolated in this function and can be changed.
	// Maybe detect the version during game id?
	return (data.byte(kSpeechloaded) == 1);
}

void DreamGenContext::showicon() {
	if (data.byte(kReallocation) < 50) {
		showpanel();
		showman();
		roomname();
		panelicons1();
		zoomicon();
	} else {
		Frame *tempSprites = (Frame *)segRef(data.word(kTempsprites)).ptr(0, 0);
		showframe(tempSprites, 72, 2, 45, 0);
		showframe(tempSprites, 72+47, 2, 46, 0);
		showframe(tempSprites, 69-10, 21, 49, 0);
		showframe(tempSprites, 160+88, 2, 45, 4 & 0xfe);
		showframe(tempSprites, 160+43, 2, 46, 4 & 0xfe);
		showframe(tempSprites, 160+101, 21, 49, 4 & 0xfe);
		middlepanel();
	}
}

void DreamGenContext::checkifset() {
	flags._z = !checkifset(al, ah);
}

bool DreamGenContext::checkifset(uint8 x, uint8 y) {
	const ObjPos *setList = (const ObjPos *)segRef(data.word(kBuffers)).ptr(kSetlist, sizeof(ObjPos) * 128);
	for (size_t i = 0; i < 128; ++i) {
		const ObjPos *pos = setList + 127 - i;
		if (pos->index == 0xff)
			continue;
		if (x < pos->xMin)
			continue;
		if (x >= pos->xMax)
			continue;
		if (y < pos->yMin)
			continue;
		if (y >= pos->yMax)
			continue;
		if (! pixelcheckset(pos, x, y))
			continue;
		if (! isitdescribed(pos))
			continue;
		obname(pos->index, 1);
		return true;
	}
	return false;
}

void DreamGenContext::showryanpage() {
	Frame *icons1 = (Frame *)segRef(data.word(kIcons1)).ptr(0, 0);
	showframe(icons1, kInventx + 167, kInventy - 12, 12, 0);
	showframe(icons1, kInventx + 167 + 18 * data.byte(kRyanpage), kInventy - 12, 13 + data.byte(kRyanpage), 0);
}

void DreamGenContext::findallryan() {
	findallryan(es.ptr(di, 60));
}

void DreamGenContext::findallryan(uint8 *inv) {
	memset(inv, 0xff, 60);
	for (size_t i = 0; i < kNumexobjects; ++i) {
		DynObject *extra = getexad(i);
		if (extra->mapad[0] != 4)
			continue;
		if (extra->mapad[1] != 0xff)
			continue;
		uint8 slot = extra->mapad[2];
		assert(slot < 30);
		inv[2 * slot + 0] = i;
		inv[2 * slot + 1] = 4;
	}
}

void DreamGenContext::hangon() {
	hangon(cx);
}

void DreamGenContext::hangon(uint16 frameCount) {
	while (frameCount) {
		vsync();
		--frameCount;
		if (data.byte(kQuitrequested))
			break;
	}
}

void DreamGenContext::hangonp() {
	hangonp(cx);
}

void DreamGenContext::hangonp(uint16 count) {
	data.word(kMaintimer) = 0;
	uint8 pointerFrame = data.byte(kPointerframe);
	uint8 pickup = data.byte(kPickup);
	data.byte(kPointermode) = 3;
	data.byte(kPickup) = 0;
	data.byte(kCommandtype) = 255;
	readmouse();
	animpointer();
	showpointer();
	vsync();
	dumppointer();
	uint32 counter = count;	// silence a signed/unsigned MSVC warning below
	for (uint32 i = 0; i < counter * 3; ++i) {
		delpointer();
		readmouse();
		animpointer();
		showpointer();
		vsync();
		dumppointer();
		if (data.byte(kQuitrequested))
			break;
		if (data.word(kMousebutton) == 0)
			continue;
		if (data.word(kMousebutton) != data.word(kOldbutton))
			break;
	}

	delpointer();
	data.byte(kPointerframe) = pointerFrame;
	data.byte(kPickup) = pickup;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::findnextcolon() {
	uint8 *initialString = es.ptr(si, 0);
	uint8 *string = initialString;
	al = findnextcolon(&string);
	si += (string - initialString);
}

uint8 DreamGenContext::findnextcolon(uint8 **string) {
	uint8 c;
	do {
		c = **string;
		++(*string);
	} while ((c != 0) && (c != ':'));
	return c;
}

uint8 *DreamGenContext::getobtextstartCPP() {
	push(es);
	push(si);
	getobtextstart();
	uint8 *result = es.ptr(si, 0);
	si = pop();
	es = pop();
	return result;
}

void DreamGenContext::zoomonoff() {
	if (data.word(kWatchingtime) != 0) {
		blank();
		return;
	}
	if (data.byte(kPointermode) == 2) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandonly(39);
	}
	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;
	if ((data.word(kMousebutton) & 1) == 0)
		return;
	data.byte(kZoomon) ^= 1;
	createpanel();
	data.byte(kNewobs) = 0;
	drawfloor();
	printsprites();
	reelsonscreen();
	showicon();
	getunderzoom();
	undertextline();
	commandonly(39);
	readmouse();
	worktoscreenm();
}

void DreamGenContext::sortoutmap() {
	const uint8 *src = (const uint8 *)segRef(data.word(kWorkspace)).ptr(0, 0);
	uint8 *dst = (uint8 *)segRef(data.word(kMapdata)).ptr(0, 0);
	for (uint16 y = 0; y < kMaplength; ++y) {
		memcpy(dst, src, kMapwidth);
		dst += kMapwidth;
		src += 132;
	}
}

void DreamGenContext::showcity() {
	clearwork();
	Frame *tempGraphics = (Frame *)segRef(data.word(kTempgraphics)).ptr(0, 0);
	showframe(tempGraphics, 57, 32, 0, 0);
	showframe(tempGraphics, 120+57, 32, 1, 0);
}

void DreamGenContext::mainscreen() {
	data.byte(kInmaparea) = 0;
	if (data.byte(kWatchon) == 1) {
		RectWithCallback mainlist[] = {
			{ 44,70,32,46,&DreamGenContext::look },
			{ 0,50,0,180,&DreamGenContext::inventory },
			{ 226,244,10,26,&DreamGenContext::zoomonoff },
			{ 226,244,26,40,&DreamGenContext::saveload },
			{ 240,260,100,124,&DreamGenContext::madmanrun },
			{ 0,320,0,200,&DreamGenContext::identifyob },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(mainlist);
	} else {
		RectWithCallback mainlist2[] = {
			{ 44,70,32,46,&DreamGenContext::look },
			{ 0,50,0,180,&DreamGenContext::inventory },
			{ 226+48,244+48,10,26,&DreamGenContext::zoomonoff },
			{ 226+48,244+48,26,40,&DreamGenContext::saveload },
			{ 240,260,100,124,&DreamGenContext::madmanrun },
			{ 0,320,0,200,&DreamGenContext::identifyob },
			{ 0xFFFF,0,0,0,0 }
		};
		checkcoords(mainlist2);
	}

	if (data.byte(kWalkandexam) != 0)
		walkandexamine();
}

void DreamGenContext::showwatch() {
	if (data.byte(kWatchon)) {
		showframe((Frame *)segRef(data.word(kIcons1)).ptr(0, 0), 250, 1, 6, 0);
		showtime();
	}
}

void DreamGenContext::showtime() {
	if (data.byte(kWatchon) == 0)
		return;
	Frame *charset = (Frame *)segRef(data.word(kCharset1)).ptr(0, 0);

	int seconds = data.byte(kSecondcount);
	int minutes = data.byte(kMinutecount);
	int hours = data.byte(kHourcount);

	showframe(charset, 282+5, 21, 91*3+10 + seconds / 10, 0);
	showframe(charset, 282+9, 21, 91*3+10 + seconds % 10, 0);

	showframe(charset, 270+5, 21, 91*3 + minutes / 10, 0);
	showframe(charset, 270+11, 21, 91*3 + minutes % 10, 0);

	showframe(charset, 256+5, 21, 91*3 + hours / 10, 0);
	showframe(charset, 256+11, 21, 91*3 + hours % 10, 0);

	showframe(charset, 267+5, 21, 91*3+20, 0);
}

void DreamGenContext::watchcount() {
	if (data.byte(kWatchon) == 0)
		return;
	++data.byte(kTimercount);
	if (data.byte(kTimercount) == 9) {
		showframe((Frame *)segRef(data.word(kCharset1)).ptr(0, 0), 268+4, 21, 91*3+21, 0);
		data.byte(kWatchdump) = 1;
	} else if (data.byte(kTimercount) == 18) {
		data.byte(kTimercount) = 0;
		++data.byte(kSecondcount);
		if (data.byte(kSecondcount) == 60) {
			data.byte(kSecondcount) = 0;
			++data.byte(kMinutecount);
			if (data.byte(kMinutecount) == 60) {
				data.byte(kMinutecount) = 0;
				++data.byte(kHourcount);
				if (data.byte(kHourcount) == 24)
					data.byte(kHourcount) = 0;
			}
		}
		showtime();
		data.byte(kWatchdump) = 1;
	}
}

void DreamGenContext::roomname() {
	printmessage(88, 18, 53, 240, false);
	uint16 textIndex = data.byte(kRoomnum);
	if (textIndex >= 32)
		textIndex -= 32;
	data.word(kLinespacing) = 7;
	uint8 maxWidth = (data.byte(kWatchon) == 1) ? 120 : 160;
	uint16 descOffset = segRef(data.word(kRoomdesc)).word(kIntextdat + textIndex * 2);
	const uint8 *string = segRef(data.word(kRoomdesc)).ptr(kIntext + descOffset, 0);
	printdirect(string, 88, 25, maxWidth, false);
	data.word(kLinespacing) = 10;
	usecharset1();
}

void DreamGenContext::zoomicon() {
	if (data.byte(kZoomon) == 0)
		return;
	showframe((Frame *)segRef(data.word(kIcons1)).ptr(0, 0), kZoomx, kZoomy-1, 8, 0);
}

void DreamGenContext::loadroom() {
	data.byte(kRoomloaded) = 1;
	data.word(kTimecount) = 0;
	data.word(kMaintimer) = 0;
	data.word(kMapoffsetx) = 104;
	data.word(kMapoffsety) = 38;
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	data.byte(kLocation) = data.byte(kNewlocation);
	Room *room = getroomdata(data.byte(kNewlocation));
	startloading(room);
	loadroomssample();
	switchryanon();
	drawflags();
	getdimension();
}

void DreamGenContext::loadroomssample() {
	uint8 sample = data.byte(kRoomssample);

	if (sample == 255 || data.byte(kCurrentsample) == sample)
		return; // loaded already

	data.byte(kCurrentsample) = sample;
	cs.byte(kSamplename+10) = '0' + sample / 10;
	cs.byte(kSamplename+11) = '0' + sample % 10;
	dx = kSamplename;
	loadsecondsample();
}

void DreamGenContext::readsetdata() {
	data.word(kCharset1) = standardload((const char *)cs.ptr(kCharacterset1, 0));
	data.word(kIcons1) = standardload((const char *)cs.ptr(kIcongraphics0, 0));
	data.word(kIcons2) = standardload((const char *)cs.ptr(kIcongraphics1, 0));
	data.word(kMainsprites) = standardload((const char *)cs.ptr(kSpritename1, 0));
	data.word(kPuzzletext) = standardload((const char *)cs.ptr(kPuzzletextname, 0));
	data.word(kCommandtext) = standardload((const char *)cs.ptr(kCommandtextname, 0));
	ax = data.word(kCharset1);
	data.word(kCurrentset) = ax;
	if (data.byte(kSoundint) == 0xff)
		return;
	Common::String name((const char*)cs.ptr(kVolumetabname, 0));
	engine->openFile(name);
	uint8 *volumeTab = segRef(data.word(kSoundbuffer)).ptr(16384, 0);
	engine->readFromFile(volumeTab, 2048-256);
	engine->closeFile();
}

} /*namespace dreamgen */

