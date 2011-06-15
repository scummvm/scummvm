#include "dreamweb/dreamweb.h"
#include "engines/util.h"
#include "graphics/surface.h"

namespace dreamgen {

Common::String getFilename(Context &context) {
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	return name;
}

void multiget(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned x = (uint16)context.di, y = (uint16)context.bx;
	unsigned src = x + y * kScreenwidth;
	unsigned dst = (uint16)context.si;
	context.es = context.ds;
	context.ds = context.data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiget %u,%u %ux%u -> segment: %04x->%04x", x, y, w, h, (uint16)context.ds, (uint16)context.es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = context.ds.ptr(src + kScreenwidth * l, w);
		uint8 *dst_p = context.es.ptr(dst + w * l, w);
		memcpy(dst_p, src_p, w);
	}
	context.si += w * h;
	context.di = src + kScreenwidth * h;
	context.cx = 0;
}

void multiput(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned x = (uint16)context.di, y = (uint16)context.bx;
	unsigned src = (uint16)context.si;
	unsigned dst = x + y * kScreenwidth;
	context.es = context.data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiput %ux%u -> segment: %04x->%04x", w, h, (uint16)context.ds, (uint16)context.es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = context.ds.ptr(src + w * l, w);
		uint8 *dst_p = context.es.ptr(dst + kScreenwidth * l, w);
		memcpy(dst_p, src_p, w);
	}
	context.si += w * h;
	context.di = dst + kScreenwidth * h;
	context.cx = 0;
}

void multidump(Context &context) {
	context.ds = context.data.word(kWorkspace);
	int w = (uint8)context.cl, h = (uint8)context.ch;
	int x = (int16)context.di, y = (int16)context.bx;
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multidump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)context.ds, x, y, offset);
	context.engine->blit(context.ds.ptr(offset, w * h), kScreenwidth, x, y, w, h);
	context.si = context.di = offset + h * kScreenwidth;
	context.cx = 0;
}

void worktoscreen(Context &context) {
	context.ds = context.data.word(kWorkspace);
	uint size = 320 * 200;
	context.engine->blit(context.ds.ptr(0, size), 320, 0, 0, 320, 200);
	context.di = context.si = size;
	context.cx = 0;
}

void printundermon(Context &context) {
	context.engine->printUnderMonitor();
}

void cls(Context &context) {
	context.engine->cls();
}

void frameoutnm(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned pitch = (uint16)context.dx;
	unsigned src = (uint16)context.si;
	int x = (uint16)context.di, y = (uint16)context.bx;
	unsigned dst = x + y * pitch;
	//debug(1, "framenm %ux%u[pitch: %u]-> %d,%d, segment: %04x->%04x", w, h, pitch, x, y, (uint16)context.ds, (uint16)context.es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = context.ds.ptr(src + w * l, w);
		uint8 *dst_p = context.es.ptr(dst + pitch * l, w);
		memcpy(dst_p, src_p, w);
	}
	context.di += dst + pitch * h;
	context.si += w * h;
	context.cx = 0;
}

void seecommandtail(Context &context) {
	context.data.word(kSoundbaseadd) = 0x220;
	context.data.byte(kSoundint) = 5;
	context.data.byte(kSounddmachannel) = 1;
	context.data.byte(kBrightness) = 1;
	context.data.word(kHowmuchalloc) = 0x9360;
}

void randomnumber(Context &context) {
	context.al = context.engine->randomNumber();
}

void quickquit(Context &context) {
	context.engine->quit();
}

void quickquit2(Context &context) {
	context.engine->quit();
}

void keyboardread(Context &context) {
	::error("keyboardread"); //this keyboard int handler, must never be called
}

void resetkeyboard(Context &context) {
}

void setkeyboardint(Context &context) {
}

void readfromfile(Context &context) {
	uint16 dst_offset = context.dx;
	uint16 size = context.cx;
	debug(1, "readfromfile(%04x:%u, %u)", (uint16)context.ds, dst_offset, size);
	context.ax = context.engine->readFromFile(context.ds.ptr(dst_offset, size), size);
	context.flags._c = false;
}

void closefile(Context &context) {
	context.engine->closeFile();
	context.data.byte(kHandle) = 0;
}

void openforsave(Context &context) {
	const char *name = (const char *)context.ds.ptr(context.dx, 13);
	debug(1, "openforsave(%s)", name);
	context.engine->openSaveFileForWriting(name);
}

void openfilenocheck(Context &context) {
	const char *name = (const char *)context.ds.ptr(context.dx, 13);
	debug(1, "checksavefile(%s)", name);
	bool ok = context.engine->openSaveFileForReading(name);
	context.flags._c = !ok;
}

void openfilefromc(Context &context) {
	openfilenocheck(context);
}

void openfile(Context &context) {
	Common::String name = getFilename(context);
	debug(1, "opening file: %s", name.c_str());
	context.engine->openFile(name);
	context.cs.word(kHandle) = 1; //only one handle
	context.flags._c = false;
}

void createfile(Context &context) {
	::error("createfile");
}

void dontloadseg(Context &context) {
	context.ax = context.es.word(context.di);
	context._add(context.di, 2);
	context.dx = context.ax;
	context.cx = 0;
	unsigned pos = context.engine->skipBytes(context.dx);
	context.dx = pos >> 16;
	context.ax = pos & 0xffff;
	context.flags._c = false;
}

void mousecall(Context &context) {
	context.engine->mouseCall();
}

void setmouse(Context &context) {
	context.data.word(kOldpointerx) = 0xffff;
	//warning("setmouse: fixme: add range setting");
	//set vertical range to 15-184
	//set horizontal range to 15-298*2
}

void gettime(Context &context) {
	TimeDate t;
	g_system->getTimeAndDate(t);
	debug(1, "\tgettime: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	context.ch = t.tm_hour;
	context.cl = t.tm_min;
	context.dh = t.tm_sec;
	context.data.byte(kSecondcount) = context.dh;
	context.data.byte(kMinutecount) = context.cl;
	context.data.byte(kHourcount) = context.ch;
}

void allocatemem(Context &context) {
	uint size = (context.bx + 2) * 16;
	debug(1, "allocate mem, %u bytes", size);
	context.flags._c = false;
	SegmentRef seg = context.allocateSegment(size);
	context.ax = (uint16)seg;
	debug(1, "\tsegment address -> %04x", (uint16)context.ax);
}

void deallocatemem(Context &context) {
	uint16 id = (uint16)context.es;
	debug(1, "deallocating segment %04x", id);
	context.deallocateSegment(id);

	//fixing invalid entries in the sprite table
	context.es = context.data;
	uint tsize = 16 * 32;
	uint16 bseg = context.data.word(kBuffers);
	if (!bseg)
		return;
	SegmentRef buffers(&context);
	buffers = bseg;
	uint8 *ptr = buffers.ptr(kSpritetable, tsize);
	for(uint i = 0; i < tsize; i += 32) {
		uint16 seg = READ_LE_UINT16(ptr + i + 6);
		//debug(1, "sprite segment = %04x", seg);
		if (seg == id)
			memset(ptr + i, 0xff, 32);
	}
}

void removeemm(Context &context) {
	::error("removeemm");
}

void setupemm(Context &context) {
	//fixme: double check this, but it seems that emm pages used only for sound
}

void pitinterupt(Context &context) {
	::error("pitinterupt");
}

void getridofpit(Context &context) {
	::error("getridofpit");
}

void setuppit(Context &context) {
	::error("setuppit");
}

void startdmablock(Context &context) {
	::error("startdmablock");
}

void dmaend(Context &context) {
	::error("dmaend");
}

void restoreems(Context &context) {
	::error("restoreems");
}

void saveems(Context &context) {
	::error("saveems");
}

void bothchannels(Context &context) {
	::error("bothchannels");
}

void channel1only(Context &context) {
	::error("channel1only");
}

void channel0only(Context &context) {
	::error("channel0only");
}

void out22c(Context &context) {
	::error("out22c");
}

void soundstartup(Context &context) {
}

void soundend(Context &context) {
}

void interupttest(Context &context) {
	::error("interupttest");
}

void disablesoundint(Context &context) {
	warning("disablesoundint: STUB");
}

void enablesoundint(Context &context) {
	warning("enablesoundint: STUB");
}

void checksoundint(Context &context) {
	context.data.byte(kTestresult) = 1;
	warning("checksoundint: STUB");
}

void setsoundoff(Context &context) {
	warning("setsoundoff: STUB");
}

void readheader(Context &context);

void loadsample(Context &context) {
	context.engine->loadSounds(0, (const char *)context.data.ptr(context.dx, 13));
}

void cancelch0(Context &context);
void cancelch1(Context &context);

void loadsecondsample(Context &context) {
	uint8 ch0 = context.data.byte(kCh0playing);
	if (ch0 >= 12 && ch0 != 255)
		cancelch0(context);
	uint8 ch1 = context.data.byte(kCh1playing);
	if (ch1 >= 12)
		cancelch1(context);
	context.engine->loadSounds(1, (const char *)context.data.ptr(context.dx, 13));
}

void createname(Context &context);

void loadspeech(Context &context) {
	cancelch1(context);
	context.data.byte(kSpeechloaded) = 0;
	createname(context);
	const char *name = (const char *)context.data.ptr(context.di, 13);
	//warning("name = %s", name);
	if (context.engine->loadSpeech(name))
		context.data.byte(kSpeechloaded) = 1;
}

void saveseg(Context &context) {
	context.cx = context.es.word(context.di);
	context._add(context.di, 2);
	savefilewrite(context);
}

void savefilewrite(Context &context) {
	context.ax = context.engine->writeToSaveFile(context.ds.ptr(context.dx, context.cx), context.cx);
}

void savefileread(Context &context) {
	context.ax = context.engine->readFromSaveFile(context.ds.ptr(context.dx, context.cx), context.cx);
}

void loadseg(Context &context) {
	context.ax = context.es.word(context.di);
	context.di += 2;

	uint16 dst_offset = context.dx;
	uint16 size = context.ax;

	debug(1, "loadseg(%04x:%u, %u)", (uint16)context.ds, dst_offset, size);
	context.ax = context.engine->readFromFile(context.ds.ptr(dst_offset, size), size);
	context.flags._c = false;
}

void error(Context &context) {
	::error("error");
}

void generalerror(Context &context) {
	::error("generalerror");
}

void commandonly(Context &context);

void dosreturn(Context &context) {
	context._cmp(context.data.byte(kCommandtype), 250);
	if (context.flags.z()) goto alreadydos;
	context.data.byte(kCommandtype) = 250;
	context.al = 46;
	commandonly(context);
alreadydos:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) return;

	quickquit2(context);
	quickquit(context);
}

void set16colpalette(Context &context) {
}

void mode640x480(Context &context) {
	// Video mode 12h: 640x480 pixels, 16 colors, I believe
	context.al = 0x12 + 128;
	context.ah = 0;
	initGraphics(640, 480, true);
}

void showgroup(Context &context) {
	context.engine->setPalette();
}

void fadedos(Context &context) {
	context.engine->fadeDos();
}

void doshake(Context &context) {
	uint8 &counter = context.data.byte(kShakecounter);
	context._cmp(counter, 48);
	if (context.flags.z())
		return;

	context._add(counter, 1);
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
	context.engine->setShakePos(offset >= 0? offset: -offset);
}

void vsync(Context &context) {
	context.engine->waitForVSync();
}

void setmode(Context &context) {
	vsync(context);
	initGraphics(320, 200, false);
}

void readoneblock(Context &context) {
	context.ds = context.data.word(kWorkspace);
	context.cx = 30000;
	context.dx = 0;
	readfromfile(context);
}

void showpcx(Context &context) {
	Common::String name = getFilename(context);
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
	context.es = context.data.word(kBuffers);
	maingamepal = context.es.ptr(4782, 768); //fixme: hardcoded offset
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
