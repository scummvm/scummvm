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
	seecommandtail();
	checkbasemem();
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
			data.byte(kVolumedirection) = -1;
			data.byte(kCommandtype) = 255;

		}

		// main loop
		while (true) {

			if (data.byte(kQuitrequested))
				return; // exit game

			screenupdate();

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
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	return name;
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
		loadspeech();
		if (data.byte(kSpeechloaded) == 1) {
			al = 50+12;
			playchannel1();
		}
		dx = pop();
		cx = pop();
		bx = pop();
		ax = pop();
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
	//good place for early initialization
	switch(engine->getLanguage()) {
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		return;
	default:
		data.byte(kForeignrelease) = 1;
	}
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
	for (size_t i=0; i < 16; ++i) {
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
	bx = kRoomdata + sizeof(Room) * al;
}

void DreamGenContext::startloading() {
	const Room *room = (Room *)cs.ptr(bx, sizeof(Room));
	startloading(room);
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
	data.byte(kLastweapon) = -1;
	al = room->b27;
	push(ax);
	al = room->b31;
	ah = data.byte(kReallocation);
	data.byte(kReallocation) = al;
	dx = bx;
	Common::String name = getFilename(*this);
	engine->openFile(name);
	cs.word(kHandle) = 1; //only one handle
	flags._c = false;
	readheader();
	allocateload();
	ds = ax;
	data.word(kBackdrop) = ax;
	dx = (0);
	loadseg();
	ds = data.word(kWorkspace);
	dx = (0);
	cx = 132*66;
	al = 0;
	fillspace();
	loadseg();
	sortoutmap();
	allocateload();
	data.word(kSetframes) = ax;
	ds = ax;
	dx = (0);
	loadseg();
	ds = data.word(kSetdat);
	dx = 0;
	cx = (64*128);
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
	cx = (16*80);
	al = 255;
	fillspace();
	loadseg();
	allocateload();
	data.word(kFreedesc) = ax;
	ds = ax;
	dx = (0);
	loadseg();
	closefile();
	findroominloc();
	deletetaken();
	setallchanges();
	autoappear();
	al = data.byte(kNewlocation);
	getroomdata();
	data.byte(kLastweapon) = -1;
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
	while (true) {
		if (reel->x < 220)
			break;
		if (reel->x == 255)
			break;
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
	multiput(segRef(data.word(kBuffers)).ptr(kTextunder, 0), x, y, kUndertextsizex, kUndertextsizey);
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
	flags._z = not checkifperson(al, ah);
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
	flags._z = not checkiffree(al, ah);
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
	flags._z = not checkifex(al, ah);
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
	while (true) {
		if (change->index == 0xff) {
			change->index = index;
			change->location = data.byte(kReallocation);
			change->value = value;
			change->type = type;
			return;
		}
		if ((index == change->index) && (data.byte(kReallocation) == change->location) && (type == change->type)) {
			change->value = value;
			return;
		}
		++change;
	}
}

void DreamGenContext::setallchanges() {
	Change *change = (Change *)segRef(data.word(kBuffers)).ptr(kListofchanges, sizeof(Change));
	while (change->index != 0xff) {
		if (change->location == data.byte(kReallocation))
			dochange(change->index, change->value, change->type);
		++change;
	}
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
	if (! finishedwalkingCPP())
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
	checkcoords((const RectWithCallback *)cs.ptr(bx, 0));
}

void DreamGenContext::checkcoords(const RectWithCallback *rectWithCallbacks) {
	if (data.byte(kNewlocation) != 0xff)
		return;

	const RectWithCallback *rectWithCallback = rectWithCallbacks;
	while (rectWithCallback->xMin() != 0xffff) {
		if (rectWithCallback->contains(data.word(kMousex), data.word(kMousey))) {
			uint16 callback = rectWithCallback->callback();

			// common
			if(callback == addr_blank)
				blank();
			else if(callback == addr_getbackfromob)
				getbackfromob();
			else if(callback == addr_incryanpage)
				incryanpage();
			else if(callback == addr_getback1)
				getback1();
			else if(callback == addr_quitkey)
				quitkey();
			else if(callback == addr_dosreturn)
				dosreturn();
			else if(callback == addr_getbacktoops)
				getbacktoops();
			else if(callback == addr_selectslot)
				selectslot();
			// examlist
			else if(callback == addr_useobject)
				useobject();
			else if(callback == addr_selectopenob)
				selectopenob();
			else if(callback == addr_setpickup)
				setpickup();
			else if(callback == addr_examinventory)
				examinventory();
			// invlist1
			else if(callback == addr_dropobject)
				dropobject();
			else if(callback == addr_useopened)
				useopened();
			else if(callback == addr_setpickup)
				setpickup();
			else if(callback == addr_intoinv)
				intoinv();
			// withlist1
			else if(callback == addr_selectob)
				selectob();
			// talklist
			else if(callback == addr_moretalk)
				moretalk();
			// quitlist
			// destlist
			else if(callback == addr_nextdest)
				nextdest();
			else if(callback == addr_lastdest)
				lastdest();
			else if(callback == addr_lookatplace)
				lookatplace();
			else if(callback == addr_destselect)
				destselect();
			// keypadlist
			else if(callback == addr_buttonone)
				buttonone();
			else if(callback == addr_buttontwo)
				buttontwo();
			else if(callback == addr_buttonthree)
				buttonthree();
			else if(callback == addr_buttonfour)
				buttonfour();
			else if(callback == addr_buttonfive)
				buttonfive();
			else if(callback == addr_buttonsix)
				buttonsix();
			else if(callback == addr_buttonseven)
				buttonseven();
			else if(callback == addr_buttoneight)
				buttoneight();
			else if(callback == addr_buttonnine)
				buttonnine();
			else if(callback == addr_buttonnought)
				buttonnought();
			else if(callback == addr_buttonenter)
				buttonenter();
			// menulist
			// folderlist
			else if(callback == addr_nextfolder)
				nextfolder();
			else if(callback == addr_lastfolder)
				lastfolder();
			// symbollist
			else if(callback == addr_quitsymbol)
				quitsymbol();
			else if(callback == addr_settopleft)
				settopleft();
			else if(callback == addr_settopright)
				settopright();
			else if(callback == addr_setbotleft)
				setbotleft();
			else if(callback == addr_setbotright)
				setbotright();
			// diarylist
			else if(callback == addr_diarykeyn)
				diarykeyn();
			else if(callback == addr_diarykeyp)
				diarykeyp();
			else if(callback == addr_quitkey)
				quitkey();
			// opslist
			else if(callback == addr_getbackfromops)
				getbackfromops();
			else if(callback == addr_discops)
				discops();
			// discopslist
			else if(callback == addr_loadgame)
				loadgame();
			else if(callback == addr_savegame)
				savegame();
			// mainlist, mainlist2
			else if(callback == addr_look)
				look();
			else if(callback == addr_inventory)
				inventory();
			else if(callback == addr_zoomonoff)
				zoomonoff();
			else if(callback == addr_saveload)
				saveload();
			else if(callback == addr_madmanrun)
				madmanrun();
			else if(callback == addr_identifyob)
				identifyob();
			// decidelist
			else if(callback == addr_newgame)
				newgame();
			else if(callback == addr_loadold)
				loadold();
			// loadlist
			else if(callback == addr_actualload)
				actualload();
			// savelist
			else if(callback == addr_actualsave)
				actualsave();
			else {
				debug("__dispatch_call remaining in checkcoords! %d", (int)callback);
				__dispatch_call(callback);
			}
			return;
		}
		++rectWithCallback;
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

void DreamGenContext::obpicture() {
	if (data.byte(kObjecttype) == 1)
		return;
	Frame *frames;
	if (data.byte(kObjecttype) == 4)
		frames = (Frame *)segRef(data.word(kExtras)).ptr(0, 0);
	else
		frames = (Frame *)segRef(data.word(kFreeframes)).ptr(0, 0);
	uint8 frame = 3 * data.byte(kCommand) + 1;
	showframe(frames, 160, 68, frame, 0x80);
}

void DreamGenContext::obicons() {
	uint8 value1, value2;
	getanyad(&value1, &value2);
	if (value1 == 0xff) {
		showframe((Frame *)segRef(data.word(kIcons2)).ptr(0, 0), 260, 1, 1, 0);
	} else {
		showframe((Frame *)segRef(data.word(kIcons2)).ptr(0, 0), 210, 1, 4, 0);
	}
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

void DreamGenContext::isitworn() {
	flags._z = isitworn((const DynObject *)es.ptr(bx, sizeof(DynObject)));
}

bool DreamGenContext::isitworn(const DynObject *object) {
	return (object->id[0] == 'W'-'A') && (object->id[1] == 'E'-'A');
}

void DreamGenContext::makeworn() {
	makeworn((DynObject *)es.ptr(bx, sizeof(DynObject)));
}

void DreamGenContext::makeworn(DynObject *object) {
	object->id[0] = 'W'-'A';
	object->id[1] = 'E'-'A';
}

void DreamGenContext::obtoinv() {
	obtoinv(al, ah, di, bx);
}

void DreamGenContext::obtoinv(uint8 index, uint8 flag, uint16 x, uint16 y) {
	Frame *icons1 = (Frame *)segRef(data.word(kIcons1)).ptr(0, 0);
	showframe(icons1, x - 2, y - 1, 10, 0);
	if (index == 0xff)
		return;

	Frame *extras = (Frame *)segRef(data.word(kExtras)).ptr(0, 0);
	Frame *frees = (Frame *)segRef(data.word(kFreeframes)).ptr(0, 0);
	Frame *frames = (flag == 4) ? extras : frees;
	showframe(frames, x + 18, y + 19, 3 * index + 1, 128);
	const DynObject *object = (const DynObject *)getanyaddir(index, flag);
	bool worn = isitworn(object);
	if (worn)
		showframe(icons1, x - 3, y - 2, 7, 0);
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

void DreamGenContext::fillryan() {
	uint8 *inv = segRef(data.word(kBuffers)).ptr(kRyaninvlist, 60);
	findallryan(inv);
	inv += data.byte(kRyanpage) * 2 * 10;
	for (size_t i = 0; i < 2; ++i) {
		for (size_t j = 0; j < 5; ++j) {
			uint8 objIndex = *inv++;
			uint8 objType = *inv++;
			obtoinv(objIndex, objType, kInventx + j * kItempicsize, kInventy + i * kItempicsize);
		}
	}
	showryanpage();
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
	for (size_t i = 0; i < count * 3; ++i) {
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

} /*namespace dreamgen */

