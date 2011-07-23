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

Common::String getFilename(Context &context) {
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	return name;
}

void DreamGenContext::multiget() {
	multiget(di, bx, cl, ch);
}

void DreamGenContext::multiget(uint16 x, uint16 y, uint8 w, uint8 h) {
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
	multiput(di, bx, cl, ch);
}

void DreamGenContext::multiput(uint16 x, uint16 y, uint8 w, uint8 h) {
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

void DreamGenContext::multidump(uint16 x, uint16 y, uint8 width, uint8 height) {
	ds = data.word(kWorkspace);
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multidump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)ds, x, y, offset);
	engine->blit(ds.ptr(offset, width * height), kScreenwidth, x, y, width, height);
}

void DreamGenContext::multidump() {
	multidump(di, bx, cl, ch);
	unsigned offset = di + bx * kScreenwidth;
	si = di = offset + ch * kScreenwidth;
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

void DreamGenContext::frameoutnm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	dst += pitch * y + x;

	for (uint16 j = 0; j < height; ++j) {
		memcpy(dst, src, width);
		dst += pitch;
		src += width;
	}
}

void DreamGenContext::frameoutbh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	uint16 stride = pitch - width;
	dst += y * pitch + x;

	for (uint16 i = 0; i < height; ++i) {
		for (uint16 j = 0; j < width; ++j) {
			if (*dst == 0xff) {
				*dst = *src;
			}
			++src;
			++dst;
		}
		dst += stride;
	}
}

void DreamGenContext::frameoutfx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	uint16 stride = pitch - width;
	dst += y * pitch + x;
	dst -= width;

	for (uint16 j = 0; j < height; ++j) {
		for (uint16 i = 0; i < width; ++i) {
			uint8 pixel = src[width - i - 1];
			if (pixel)
				*dst = pixel;
			++dst;
		}
		src += width;
		dst += stride;
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
	engine->mouseCall();
}

void DreamGenContext::setmouse() {
	data.word(kOldpointerx) = 0xffff;
}

void DreamGenContext::printboth() {
	uint16 x = di;
	printboth(es, ds, &x, bx, al);
	di = x;
}

void DreamGenContext::printboth(uint16 dst, uint16 src, uint16 *x, uint16 y, uint8 c) {
	uint16 newX = *x;
	uint8 width, height;
	printchar(dst, src, &newX, y, c, &width, &height);
	multidump(*x, y, width, height);
	*x = newX;
}

uint8 DreamGenContext::getnextword(const uint8 *string, uint8 *totalWidth, uint8 *charCount) {
	*totalWidth = 0;
	*charCount = 0;
	while(true) {
		uint8 firstChar = *string;
		++string;
		++*charCount;
		if ((firstChar == ':') || (firstChar == 0)) { //endall
			*totalWidth += 6;
			return 1;
		}
		if (firstChar == 32) { //endword
			*totalWidth += 6;
			return 0;
		}
		firstChar = engine->modifyChar(firstChar);
		if (firstChar != 255) {
			uint8 secondChar = *string;
			uint8 width = ds.byte(6*(firstChar - 32 + data.word(kCharshift)));
			width = kernchars(firstChar, secondChar, width);
			*totalWidth += width;
		}
	}
}

void DreamGenContext::getnextword() {
	uint8 totalWidth, charCount;
	al = getnextword(es.ptr(di, 0), &totalWidth, &charCount);
	bl = totalWidth;
	bh = charCount;
	di += charCount;
}

void DreamGenContext::printchar() {
	uint16 x = di;
	uint8 width, height;
	printchar(es, ds, &x, bx, al, &width, &height);
	di = x;
	cl = width;
	ch = height;
}

void DreamGenContext::printchar(uint16 dst, uint16 src, uint16* x, uint16 y, uint8 c, uint8 *width, uint8 *height) {
	if (c == 255)
		return;
	push(si);
	push(di);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	showframe(dst, src, *x, y, c - 32 + data.word(kCharshift), 0, width, height);
	di = pop();
	si = pop();
	_cmp(data.byte(kKerning), 0);
	if (flags.z())
		kernchars();
	(*x) += *width;
}

void DreamGenContext::printslow() {
	al = printslow(di, bx, dl, (bool)(dl & 1));
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

uint8 DreamGenContext::printslow(uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	data.byte(kPointerframe) = 1;
	data.byte(kPointermode) = 3;
	ds = data.word(kCharset1);
	do {
		uint16 offset = x;
		uint16 charCount = getnumber(si, maxWidth, centered, &offset);
		do {
			push(si);
			push(es);
			uint8 c0 = es.byte(si);
			push(es);
			push(ds);
			c0 = engine->modifyChar(c0);
			printboth(es, ds, &offset, y, c0);
			ds = pop();
			es = pop();
			uint8 c1 = es.byte(si+1);
			++si;
			if ((c1 == 0) || (c1 == ':')) {
				es = pop();
				si = pop();
				return 0;
			}
			if (charCount != 1) {
				push(ds);
				push(es);
				c1 = engine->modifyChar(c1);
				data.word(kCharshift) = 91;
				uint16 offset2 = offset;
				printboth(es, ds, &offset2, y, c1);
				data.word(kCharshift) = 0;
				es = pop();
				ds = pop();
				for (int i=0; i<2; ++i) {
					waitframes();
					if (ax == 0)
						continue;
					if (ax != data.word(kOldbutton)) {
						es = pop();
						si = pop();
						return 1;
					}
				}
			}

			es = pop();
			si = pop();
			++si;
			--charCount;
		} while (charCount);
		y += 10;
	} while (true);
}

void DreamGenContext::printdirect() {
	uint16 y = bx;
	printdirect(di, &y, dl, (bool)(dl & 1));
	bx = y;
}

void DreamGenContext::printdirect(uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
	data.word(kLastxpos) = x;
	ds = data.word(kCurrentset);
	while (true) {
		uint16 offset = x;
		uint8 charCount = getnumber(si, maxWidth, centered, &offset);
		uint16 i = offset;
		do {
			uint8 c = es.byte(si);
			++si;
			if ((c == 0) || (c == ':')) {
				return;
			}
			c = engine->modifyChar(c);
			uint8 width, height;
			push(es);
			printchar(es, ds, &i, *y, c, &width, &height);
			es = pop();
			data.word(kLastxpos) = i;
			--charCount;
		} while(charCount);
		*y += data.word(kLinespacing);
	}
}

void DreamGenContext::getundertimed() {
	uint16 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease))
		y -= 3;
	ds = data.word(kBuffers);
	si = kUndertimedtext;
	multiget(data.byte(kTimedx), y, 240, kUndertimedysize);
}

void DreamGenContext::putundertimed() {
	uint16 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease))
		y -= 3;
	ds = data.word(kBuffers);
	si = kUndertimedtext;
	multiput(data.byte(kTimedx), y, 240, kUndertimedysize);
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
	uint16 y = data.byte(kTimedy);
	printdirect(data.byte(kTimedx), &y, 237, true);
	data.byte(kNeedtodumptimed) = 1;
}

void DreamGenContext::getnumber() {
	uint16 offset = di;
	cl = getnumber(si, dl, (bool)(dl & 1), &offset);
	di = offset;
}

uint8 DreamGenContext::getnumber(uint16 index, uint16 maxWidth, bool centered, uint16* offset) {
	uint8 totalWidth = 0;
	uint8 charCount = 0;
	while (true) {
		uint8 wordTotalWidth, wordCharCount;
		uint8 done = getnextword(es.ptr(index, 0), &wordTotalWidth, &wordCharCount);
		index += wordCharCount;

		if (done == 1) { //endoftext
			ax = totalWidth + wordTotalWidth - 10;
			if (ax < maxWidth) {
				totalWidth += wordTotalWidth;
				charCount += wordCharCount;
			}

			if (centered) {
				ax = (maxWidth & 0xfe) + 2 + 20 - totalWidth;
				ax /= 2;
			} else {
				ax = 0;
			}
			*offset += ax;
			return charCount;
		}
		ax = totalWidth + wordTotalWidth - 10;
		if (ax >= maxWidth) { //gotoverend
			if (centered) {
				ax = (maxWidth & 0xfe) - totalWidth + 20;
				ax /= 2;
			} else {
				ax = 0;
			}
			*offset += ax;
			return charCount;
		}
		totalWidth += wordTotalWidth;
		charCount += wordCharCount;
	}
}

uint8 DreamGenContext::kernchars(uint8 firstChar, uint8 secondChar, uint8 width) {
	if ((firstChar == 'a') || (al == 'u')) {
		if ((secondChar == 'n') || (secondChar == 't') || (secondChar == 'r') || (secondChar == 'i') || (secondChar == 'l'))
			return width-1;
	}
	return width;
}

void DreamGenContext::kernchars() {
	cl = kernchars(al, ah, cl);
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
	engine->setShakePos(offset >= 0 ? offset : -offset);
}

void DreamGenContext::vsync() {
	push(ax);
	push(bx);
	push(cx);
	push(dx);
	push(si);
	push(di);
	push(es);
	push(ds);
	engine->waitForVSync();
	ds = pop();
	es = pop();
	di = pop();
	si = pop();
	dx = pop();
	cx = pop();
	bx = pop();
	ax = pop();
}

void DreamGenContext::setmode() {
	vsync();
	initGraphics(320, 200, false);
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
	maingamepal = es.ptr(kMaingamepal, 768);
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

/*
void DreamGenContext::frameoutv() {
	uint16 pitch = dx;
	uint16 width = cx & 0xff;
	uint16 height = cx >> 8;

	const uint8 *src = ds.ptr(si, width * height);
	uint8 *dst = es.ptr(0, pitch * height);

	frameoutv(dst, src, pitch, width, height, di, bx);
}
*/

void DreamGenContext::frameoutv(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	// NB : These resilience checks were not in the original engine, but did they result in undefined behaviour
	// or was something broken during porting to C++?
	assert(pitch == 320);

	if(x >= 320)
		return;
	if(y >= 200)
		return;
	if(x + width > 320) {
		width = 320 - x;
	}
	if(y + height > 200) {
		height = 200 - y;
	}

	uint16 stride = pitch - width;
	dst += pitch * y + x;

	for (uint16 j = 0; j < height; ++j) {
		for (uint16 i = 0; i < width; ++i) {
			uint8 pixel = *src++;
			if (pixel)
				*dst = pixel;
			++dst;
		}
		dst += stride;
	}
}

Sprite *DreamGenContext::spritetable() {
	push(es);
	push(bx);

	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprite = (Sprite *)es.ptr(bx, 16 * sizeof(Sprite));

	bx = pop();
	es = pop();

	return sprite;
}

void DreamGenContext::showframe(uint16 dst, uint16 src, uint16 x, uint16 y, uint8 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height) {
	es = dst;
	ds = src;
	di = x;
	bx = y;
	al = frameNumber;
	ah = effectsFlag;

	si = (ax & 0x1ff) * 6;
	if (ds.word(si) == 0) {
		*width = 0;
		*height = 0;
		return;
	}

//notblankshow:
	if ((effectsFlag & 128) == 0) {
		di += ds.byte(si + 4);
		bx += ds.byte(si + 5);
	}
//skipoffsets:
	cx = ds.word(si + 0);
	*width = cl;
	*height = ch;
	si = ds.word(si+2) + 2080;

	if (effectsFlag) {
		if (effectsFlag & 128) { //centred
			di -= *width / 2;
			bx -= *height / 2;
		}
		if (effectsFlag & 64) { //diffdest
			frameoutfx(es.ptr(0, dx * *height), ds.ptr(si, *width * *height), dx, *width, *height, di, bx);
			return;
		}
		if (effectsFlag & 8) { //printlist
			push(ax);
			ax = di - data.word(kMapadx);
			push(bx);
			bx -= data.word(kMapady);
			ah = bl;
			bx = pop();
			//addtoprintlist(); // NB: Commented in the original asm
			ax = pop();
		}
		if (effectsFlag & 4) { //flippedx
			es = data.word(kWorkspace);
			frameoutfx(es.ptr(0, 320 * *height), ds.ptr(si, *width * *height), 320, *width, *height, di, bx);
			return;
		}
		if (effectsFlag & 2) { //nomask
			es = data.word(kWorkspace);
			frameoutnm(es.ptr(0, 320 * *height), ds.ptr(si, *width * *height), 320, *width, *height, di, bx);
			return;
		}
		if (effectsFlag & 32) {
			es = data.word(kWorkspace);
			frameoutbh(es.ptr(0, 320 * *height), ds.ptr(si, *width * *height), 320, *width, *height, di, bx);
			return;
		}
	}
//noeffects:
	es = data.word(kWorkspace);
	frameoutv(es.ptr(0, 65536), ds.ptr(si, *width * *height), 320, *width, *height, di, bx);
	return;
}

void DreamGenContext::showframe() {
	uint8 width, height;
	showframe(es, ds, di, bx, al, ah, &width, &height);
	cl = width;
	ch = height;
}

void DreamGenContext::printsprites() {
	for (size_t priority = 0; priority < 7; ++priority) {
		Sprite *sprites = spritetable();
		for (size_t j = 0; j < 16; ++j) {
			const Sprite &sprite = sprites[j];
			if (READ_LE_UINT16(&sprite.updateCallback) == 0x0ffff)
				continue;
			if (priority != sprite.priority)
				continue;
			if (sprite.hidden == 1)
				continue;
			printasprite(&sprite);
		}
	}
}

void DreamGenContext::printasprite(const Sprite *sprite) {
	push(es);
	push(bx);
	ds = READ_LE_UINT16(&sprite->w6);
	ax = sprite->y;
	if (al >= 220) {
		bx = data.word(kMapady) - (256 - al);
	} else {
		bx = ax + data.word(kMapady);
	}

	ax = sprite->x;
	if (al >= 220) {
		di = data.word(kMapadx) - (256 - al);
	} else {
		di = ax + data.word(kMapadx);
	}
	
	uint8 c;
	if (sprite->b29 != 0)
		c = 8;
	else
		c = 0;
	uint8 width, height;
	showframe(es, ds, di, bx, sprite->b15, c, &width, &height);
	cl = width;
	ch = height;

	bx = pop();
	es = pop();
}

void DreamGenContext::eraseoldobs() {
	if (data.byte(kNewobs) == 0)
		return;

	Sprite *sprites = spritetable();
	for (size_t i=0; i < 16; ++i) {
		Sprite &sprite = sprites[i];
		if (READ_LE_UINT16(&sprite.obj_data) != 0xffff) {
			memset(&sprite, 0xff, sizeof(Sprite));
		}
	}
}

void DreamGenContext::clearsprites() {
	memset(spritetable(), 0xff, sizeof(Sprite) * 16);
}

Sprite *DreamGenContext::makesprite(uint8 x, uint8 y, uint16 updateCallback, uint16 somethingInDx, uint16 somethingInDi) {
	Sprite *sprite = spritetable();
	while (sprite->b15 != 0xff) { // NB: No boundchecking in the original code either
		++sprite;
	}

	WRITE_LE_UINT16(&sprite->updateCallback, updateCallback);
	sprite->x = x;
	sprite->y = y;
	WRITE_LE_UINT16(&sprite->w6, somethingInDx);
	WRITE_LE_UINT16(&sprite->w8, somethingInDi);
	sprite->w2 = 0xffff;
	sprite->b15 = 0;
	sprite->delay = 0;
	return sprite;
}

void DreamGenContext::makesprite() { // NB: returns new sprite in es:bx 
	Sprite *sprite = makesprite(si & 0xff, si >> 8, cx, dx, di);

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += sizeof(Sprite) * (sprite - sprites);
	//
}

void DreamGenContext::spriteupdate() {
	Sprite *sprites = spritetable();
	sprites[0].hidden = data.byte(kRyanon);

	Sprite *sprite = sprites;
	for (size_t i=0; i < 16; ++i) {
		uint16 updateCallback = READ_LE_UINT16(&sprite->updateCallback);
		if (updateCallback != 0xffff) {
			sprite->w24 = sprite->w2;
			if (updateCallback == addr_mainman) // NB : Let's consider the callback as an enum while more code is not ported to C++
				mainmanCPP(sprite);
			else {
				assert(updateCallback == addr_backobject);
				backobject(sprite);
			}
		}
	
		if (data.byte(kNowinnewroom) == 1)
			break;
		++sprite;
	}
}

void DreamGenContext::initman() {
	Sprite *sprite = makesprite(data.byte(kRyanx), data.byte(kRyany), addr_mainman, data.word(kMainsprites), 0);
	sprite->priority = 4;
	sprite->b22 = 0;
	sprite->b29 = 0;

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += 32 * (sprite - sprites);
	//
}

void DreamGenContext::mainmanCPP(Sprite *sprite) {
	push(es);
	push(ds);

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += 32 * (sprite - sprites);
	//

	if (data.byte(kResetmanxy) == 1) {
		data.byte(kResetmanxy) = 0;
		sprite->x = data.byte(kRyanx);
		sprite->y = data.byte(kRyany);
		sprite->b29 = 0;
	}
	--sprite->b22;
	if (sprite->b22 != 0xff) {
		ds = pop();
		es = pop();
		return;
	}
	sprite->b22 = 0;
	if (data.byte(kTurntoface) != data.byte(kFacing)) {
		aboutturn(sprite);
	} else {
		if ((data.byte(kTurndirection) != 0) && (data.byte(kLinepointer) == 254)) {
			data.byte(kReasseschanges) = 1;
			if (data.byte(kFacing) == data.byte(kLeavedirection))
				checkforexit();
		}
		data.byte(kTurndirection) = 0;
		if (data.byte(kLinepointer) == 254) {
			sprite->b29 = 0;
		} else {
			++sprite->b29;
			if (sprite->b29 == 11)
				sprite->b29 = 1;
			walking();
			if (data.byte(kLinepointer) != 254) {
				if ((data.byte(kFacing) & 1) == 0)
					walking();
				else if ((sprite->b29 != 2) && (sprite->b29 != 7))
					walking();
			}
			if (data.byte(kLinepointer) == 254) {
				if (data.byte(kTurntoface) == data.byte(kFacing)) {
					data.byte(kReasseschanges) = 1;
					if (data.byte(kFacing) == data.byte(kLeavedirection))
						checkforexit();
				}
			}
		}
	}
	static const uint8 facelist[] = { 0,60,33,71,11,82,22,93 };
	sprite->b15 = sprite->b29 + facelist[data.byte(kFacing)];
	data.byte(kRyanx) = sprite->x;
	data.byte(kRyany) = sprite->y;

	ds = pop();
	es = pop();
}

void DreamGenContext::walking() {
	Sprite *sprite = (Sprite *)es.ptr(bx, sizeof(Sprite));

	uint8 comp;
	if (data.byte(kLinedirection) != 0) {
		--data.byte(kLinepointer);
		comp = 200;
	} else {
		++data.byte(kLinepointer);
		comp = data.byte(kLinelength);
	}
	if (data.byte(kLinepointer) < comp) {
		sprite->x = data.byte(kLinedata + data.byte(kLinepointer) * 2 + 0);
		sprite->y = data.byte(kLinedata + data.byte(kLinepointer) * 2 + 1);
		return;
	}

	data.byte(kLinepointer) = 254;
	data.byte(kManspath) = data.byte(kDestination);
	if (data.byte(kDestination) == data.byte(kFinaldest)) {
		facerightway();
		return;
	}
	data.byte(kDestination) = data.byte(kFinaldest);
	push(es);
	push(bx);
	autosetwalk();
	bx = pop();
	es = pop();
}

void DreamGenContext::aboutturn(Sprite *sprite) {
	bool incdir = true;

	if (data.byte(kTurndirection) == 1)
		incdir = true;
	else if ((int8)data.byte(kTurndirection) == -1)
		incdir = false;
	else {
		if (data.byte(kFacing) < data.byte(kTurntoface)) {
			uint8 delta = data.byte(kTurntoface) - data.byte(kFacing);
			if (delta >= 4)
				incdir = false;
			else
				incdir = true;
		} else {
			uint8 delta = data.byte(kFacing) - data.byte(kTurntoface);
			if (delta >= 4)
				incdir = true;
			else
				incdir = false;
		}
	}

	if (incdir) {
		data.byte(kTurndirection) = 1;
		data.byte(kFacing) = (data.byte(kFacing) + 1) & 7;
		sprite->b29 = 0;
	} else {
		data.byte(kTurndirection) = -1;
		data.byte(kFacing) = (data.byte(kFacing) - 1) & 7;
		sprite->b29 = 0;
	}
}

void DreamGenContext::backobject(Sprite *sprite) {
	push(es);
	push(ds);

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += 32 * (sprite - sprites);
	//

	ds = data.word(kSetdat);
	di = READ_LE_UINT16(&sprite->obj_data);
	ObjData *objData = (ObjData *)ds.ptr(di, 0);

	if (sprite->delay != 0) {
		--sprite->delay;
		ds = pop();
		es = pop();
		return;
	}

	sprite->delay = objData->delay;
	if (objData->type == 6)
		widedoor(sprite, objData);
	else if (objData->type == 5)
		random(sprite, objData);
	else if (objData->type == 4)
		lockeddoorway();
	else if (objData->type == 3)
		liftsprite(sprite, objData);
	else if (objData->type == 2)
		doorway(sprite, objData);
	else if (objData->type == 1)
		constant(sprite, objData);
	else
		steady(sprite, objData);

	ds = pop();
	es = pop();
}

void DreamGenContext::constant(Sprite *sprite, ObjData *objData) {
	++sprite->frame;
	if (objData->b18[sprite->frame] == 255) {
		sprite->frame = 0;
	}
	uint8 b18 = objData->b18[sprite->frame];
	objData->b17 = b18;
	sprite->b15 = b18;
}

void DreamGenContext::random(Sprite *sprite, ObjData *objData) {
	randomnum1();
	uint16 r = ax;
	sprite->b15 = objData->b18[r&7];
}

void DreamGenContext::doorway(Sprite *sprite, ObjData *objData) {
	data.byte(kDoorcheck1) = -24;
	data.byte(kDoorcheck2) = 10;
	data.byte(kDoorcheck3) = -30;
	data.byte(kDoorcheck4) = 10;
	dodoor(sprite, objData);
}

void DreamGenContext::widedoor(Sprite *sprite, ObjData *objData) {
	data.byte(kDoorcheck1) = -24;
	data.byte(kDoorcheck2) = 24;
	data.byte(kDoorcheck3) = -30;
	data.byte(kDoorcheck4) = 24;
	dodoor(sprite, objData);
}

void DreamGenContext::dodoor() {
	Sprite *sprite = (Sprite *)es.ptr(bx, sizeof(Sprite));
	ObjData *objData = (ObjData *)ds.ptr(di, 0);
	dodoor(sprite, objData);
}

void DreamGenContext::dodoor(Sprite *sprite, ObjData *objData) {
	uint8 ryanx = data.byte(kRyanx);
	uint8 ryany = data.byte(kRyany);
	int8 deltax = ryanx - sprite->x;
	int8 deltay = ryany - sprite->y;
	if (ryanx < sprite->x) {
		if (deltax < (int8)data.byte(kDoorcheck1))
			goto shutdoor;
	} else {
		if (deltax >= data.byte(kDoorcheck2))
			goto shutdoor;
	}
	if (ryany < sprite->y) {
		if (deltay < (int8)data.byte(kDoorcheck3))
			goto shutdoor;
	} else {
		if (deltay >= data.byte(kDoorcheck4))
			goto shutdoor;
	}
//opendoor:
	if ((data.byte(kThroughdoor) == 1) && (sprite->frame == 0))
		sprite->frame = 6;

	++sprite->frame;
	if (sprite->frame == 1) { //doorsound2
		if (data.byte(kReallocation) == 5) //hoteldoor2
			al = 13;
		else
			al = 0;
		playchannel1();
	}
	if (objData->b18[sprite->frame] == 255) {
		--sprite->frame;
	}
	sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	data.byte(kThroughdoor) = 1;
	return;
shutdoor:
	if (sprite->frame == 5) { //doorsound1;
		if (data.byte(kReallocation) == 5) //hoteldoor1
			al = 13;
		else
			al = 1;
		playchannel1();
	}
	if (sprite->frame != 0) {
		--sprite->frame;
	}
	sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	if (sprite->frame == 5) //nearly
		data.byte(kThroughdoor) = 0;
}

void DreamGenContext::steady(Sprite *sprite, ObjData *objData) {
	uint8 b18 = objData->b18[0];
	objData->b17 = b18;
	sprite->b15 = b18;
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

void DreamGenContext::liftsprite() {
	Sprite *sprite = (Sprite *)es.ptr(bx, sizeof(Sprite));
	ObjData *objData = (ObjData *)ds.ptr(di, 0);
	liftsprite(sprite, objData);
}

void DreamGenContext::liftsprite(Sprite *sprite, ObjData *objData) {
	uint8 liftFlag = data.byte(kLiftflag);
	if (liftFlag == 0) { //liftclosed
		turnpathoffCPP(data.byte(kLiftpath));

		if (data.byte(kCounttoopen) != 0) {
			_dec(data.byte(kCounttoopen));
			if (data.byte(kCounttoopen) == 0)
				data.byte(kLiftflag) = 3;
		}
		sprite->frame = 0;
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	}
	else if (liftFlag == 1) {  //liftopen
		turnpathonCPP(data.byte(kLiftpath));

		if (data.byte(kCounttoclose) != 0) {
			_dec(data.byte(kCounttoclose));
			if (data.byte(kCounttoclose) == 0)
				data.byte(kLiftflag) = 2;
		}
		sprite->frame = 12;
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	}	
	else if (liftFlag == 3) { //openlift
		if (sprite->frame == 12) {
			data.byte(kLiftflag) = 1;
			return;
		}
		++sprite->frame;
		if (sprite->frame == 1) {
			al = 2;
			liftnoise();
		}
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	} else { //closeLift
		assert(liftFlag == 2);
		if (sprite->frame == 0) {
			data.byte(kLiftflag) = 0;
			return;
		}
		--sprite->frame;
		if (sprite->frame == 11) {
			al = 3;
			liftnoise();
		}
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
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

} /*namespace dreamgen */

