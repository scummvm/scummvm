#include "dreamweb/dreamweb.h"
#include "engines/util.h"
#include "graphics/surface.h"

namespace DreamGen {

Common::String getFilename(Context &context) {
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	return name;
}

void DreamGenContext::multiget() {
	unsigned w = (uint8)cl, h = (uint8)ch;
	unsigned x = (uint16)di, y = (uint16)bx;
	unsigned src = x + y * kScreenwidth;
	unsigned dst = (uint16)si;
	es = ds;
	ds = data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiget %u,%u %ux%u -> segment: %04x->%04x", x, y, w, h, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = ds.ptr(src + kScreenwidth * l, w);
		uint8 *dst_p = es.ptr(dst + w * l, w);
		memcpy(dst_p, src_p, w);
	}
	si += w * h;
	di = src + kScreenwidth * h;
	cx = 0;
}

void DreamGenContext::multiput() {
	unsigned w = (uint8)cl, h = (uint8)ch;
	unsigned x = (uint16)di, y = (uint16)bx;
	unsigned src = (uint16)si;
	unsigned dst = x + y * kScreenwidth;
	es = data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiput %ux%u -> segment: %04x->%04x", w, h, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = ds.ptr(src + w * l, w);
		uint8 *dst_p = es.ptr(dst + kScreenwidth * l, w);
		memcpy(dst_p, src_p, w);
	}
	si += w * h;
	di = dst + kScreenwidth * h;
	cx = 0;
}

void DreamGenContext::multidump() {
	ds = data.word(kWorkspace);
	int w = (uint8)cl, h = (uint8)ch;
	int x = (int16)di, y = (int16)bx;
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multidump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)ds, x, y, offset);
	engine->blit(ds.ptr(offset, w * h), kScreenwidth, x, y, w, h);
	si = di = offset + h * kScreenwidth;
	cx = 0;
}

void DreamGenContext::worktoscreen() {
	ds = data.word(kWorkspace);
	uint size = 320 * 200;
	engine->blit(ds.ptr(0, size), 320, 0, 0, 320, 200);
	di = si = size;
	cx = 0;
}

void DreamGenContext::printundermon() {
	engine->printUnderMonitor();
}

void DreamGenContext::cls() {
	engine->cls();
}

void DreamGenContext::frameoutnm() {
	unsigned w = (uint8)cl, h = (uint8)ch;
	unsigned pitch = (uint16)dx;
	unsigned src = (uint16)si;
	int x = (uint16)di, y = (uint16)bx;
	unsigned dst = x + y * pitch;
	//debug(1, "framenm %ux%u[pitch: %u]-> %d,%d, segment: %04x->%04x", w, h, pitch, x, y, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = ds.ptr(src + w * l, w);
		uint8 *dst_p = es.ptr(dst + pitch * l, w);
		memcpy(dst_p, src_p, w);
	}
	di += dst + pitch * h;
	si += w * h;
	cx = 0;
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
	uint size = (bx + 2) * 16;
	debug(1, "allocate mem, %u bytes", size);
	flags._c = false;
	SegmentRef seg = allocateSegment(size);
	ax = (uint16)seg;
	debug(1, "\tsegment address -> %04x", (uint16)ax);
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

void DreamGenContext::doshake() {
	uint8 &counter = data.byte(kShakecounter);
	_cmp(counter, 48);
	if (flags.z())
		return;

	_add(counter, 1);
	static const int shakeTable[] = {
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,

		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,

		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,

		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  0,
	};
	int offset = shakeTable[counter];
	engine->setShakePos(offset >= 0? offset: -offset);
}

void DreamGenContext::vsync() {
	engine->waitForVSync();
}

void DreamGenContext::setmode() {
	vsync();
	initGraphics(320, 200, false);
}

void DreamGenContext::readoneblock() {
	ds = data.word(kWorkspace);
	cx = 30000;
	dx = 0;
	readfromfile();
}

void DreamGenContext::showpcx() {
	Common::String name = getFilename(*this);
	Common::File pcxFile;

	if (!pcxFile.open(name)) {
		warning("showpcx: Could not open '%s'", name.c_str());
		return;
	}

	uint8 *maingamepal;
	int i, j;

	// Read the 16-color palette into the 'maingamepal' buffer. Note that
	// the color components have to be adjusted from 8 to 6 bits.

	pcxFile.seek(16, SEEK_SET);
	es = data.word(kBuffers);
	maingamepal = es.ptr(4782, 768); //fixme: hardcoded offset
	pcxFile.read(maingamepal, 48);

	memset(maingamepal + 48, 0xff, 720);
	for (i = 0; i < 48; i++) {
		maingamepal[i] >>= 2;
	}

	// Decode the image data.

	Graphics::Surface *s = g_system->lockScreen();
	Common::Rect rect(640, 480);

	s->fillRect(rect, 0);
	pcxFile.seek(128, SEEK_SET);

	for (int y = 0; y < 480; y++) {
		byte *dst = (byte *)s->getBasePtr(0, y);
		int decoded = 0;

		while (decoded < 320) {
			byte col = pcxFile.readByte();
			byte len;

			if ((col & 0xc0) == 0xc0) {
				len = col & 0x3f;
				col = pcxFile.readByte();
			} else {
				len = 1;
			}

			// The image uses 16 colors and is stored as four bit
			// planes, one for each bit of the color, least
			// significant bit plane first.

			for (i = 0; i < len; i++) {
				int plane = decoded / 80;
				int pos = decoded % 80;

				for (j = 0; j < 8; j++) {
					byte bit = (col >> (7 - j)) & 1;
					dst[8 * pos + j] |= (bit << plane);
				}

				decoded++;
			}
		}
	}

	g_system->unlockScreen();
	pcxFile.close();
}

} /*namespace dreamgen */
