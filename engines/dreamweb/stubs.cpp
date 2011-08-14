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
#include "graphics/surface.h"

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

	while (true) {

		scanfornames();

		bool startNewGame = true;

		if (al == 0 && firstLoop) {

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
				cx = 200;
				hangon();
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
		cx = 100;
		hangon();

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
	engine->mouseCall();
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
	//fixme: this is a bit hackish, set16colpalette called after initialization and nearly before main loop.
	engine->enableSavingOrLoading();
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

void DreamGenContext::turnpathonCPP(uint8 param) {
	al = param;
	push(es);
	push(bx);
	turnpathon();
	bx = pop();
	es = pop();
}

void DreamGenContext::turnpathoffCPP(uint8 param) {
	al = param;
	push(es);
	push(bx);
	turnpathoff();
	bx = pop();
	es = pop();
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

void DreamGenContext::getroomspaths() {
	es = data.word(kReels);
	bx = data.byte(kRoomnum) * 144;
}

uint8 *DreamGenContext::getroomspathsCPP() {
	void *result = segRef(data.word(kReels)).ptr(data.byte(kRoomnum) * 144, 144);
	return (uint8 *)result;
}

void DreamGenContext::makebackob() {
	if (data.byte(kNewobs) == 0)
		return;
	uint8 priority = es.byte(si+5);
	uint8 type = es.byte(si+8);
	Sprite *sprite = makesprite(data.word(kObjectx), data.word(kObjecty), addr_backobject, data.word(kSetframes), 0);

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += sizeof(Sprite) * (sprite - sprites);
	//
	sprite->setObjData(si);
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

void DreamGenContext::getroomdata(uint8 roomIndex) {
	getroomdata(roomIndex);
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
		al = secondParam;
		placesetobject();
		data.byte(kHavedoneobs) = 1;
	} else if (type == 1) {
		al = secondParam;
		removesetobject();
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
		movemap();
	}
}

void DreamGenContext::plotreel() {
	Reel *reel = getreelstartCPP();
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
	soundonreels();
}

void DreamGenContext::crosshair() {
	uint8 frame;
	if ((data.byte(kCommandtype) != 3) && (data.byte(kCommandtype) < 10)) {
		frame = 9;
	} else {
		frame = 29;
	}
	const Frame *src = (const Frame *)segRef(data.word(kIcons1)).ptr(0, 0);
	uint8 width, height;
	showframe(src, kZoomx + 24, kZoomy + 19, frame, 0, &width, &height);
}

void DreamGenContext::deltextline() {
	uint16 x = data.word(kTextaddressx);
	uint16 y = data.word(kTextaddressy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	multiput(segRef(data.word(kBuffers)).ptr(kTextunder, 0), x, y, kUndertextsizex, kUndertextsizey);
}

void DreamGenContext::autosetwalk() {
	al = data.byte(kManspath);
	if (data.byte(kFinaldest) == al)
		return;
	const uint8 *roomsPaths = getroomspathsCPP();
	checkdest(roomsPaths);
	data.word(kLinestartx) = roomsPaths[data.byte(kManspath) * 8 + 0] - 12;
	data.word(kLinestarty) = roomsPaths[data.byte(kManspath) * 8 + 1] - 12;
	data.word(kLineendx) = roomsPaths[data.byte(kDestination) * 8 + 0] - 12;
	data.word(kLineendy) = roomsPaths[data.byte(kDestination) * 8 + 1] - 12;
	bresenhams();
	if (data.byte(kLinedirection) != 0) {
		data.byte(kLinepointer) = data.byte(kLinelength) - 1;
		data.byte(kLinedirection) = 1;
		return;
	}
	data.byte(kLinepointer) = 0;
}

void DreamGenContext::checkdest(const uint8 *roomsPaths) {
	const uint8 *p = roomsPaths + 12 * 8;
	ah = data.byte(kManspath) << 4;
	al = data.byte(kDestination);
	uint8 destination = data.byte(kDestination);
	for (size_t i = 0; i < 24; ++i) {
		dh = p[0] & 0xf0;
		dl = p[0] & 0x0f;
		if (ax == dx) {
			data.byte(kDestination) = p[1] & 0x0f;
			return;
		}
		dl = (p[0] & 0xf0) >> 4;
		dh = (p[0] & 0x0f) << 4;
		if (ax == dx) {
			destination = p[1] & 0x0f;
		}
		p += 2;
	}
	data.byte(kDestination) = destination;
}

} /*namespace dreamgen */
