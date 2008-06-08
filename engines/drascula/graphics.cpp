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

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::allocMemory() {
	screenSurface = (byte *)malloc(64000);
	assert(screenSurface);
	frontSurface = (byte *)malloc(64000);
	assert(frontSurface);
	backSurface = (byte *)malloc(64000);
	assert(backSurface);
	drawSurface1 = (byte *)malloc(64000);
	assert(drawSurface1);
	drawSurface2 = (byte *)malloc(64000);
	assert(drawSurface2);
	drawSurface3 = (byte *)malloc(64000);
	assert(drawSurface3);
	tableSurface = (byte *)malloc(64000);
	assert(tableSurface);
	extraSurface = (byte *)malloc(64000);
	assert(extraSurface);
}

void DrasculaEngine::freeMemory() {
	free(screenSurface);
	free(drawSurface1);
	free(backSurface);
	free(drawSurface2);
	free(tableSurface);
	free(drawSurface3);
	free(extraSurface);
	free(frontSurface);
}

void DrasculaEngine::moveCursor() {
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	updateRefresh_pre();
	moveCharacters();
	updateRefresh();

	if (!strcmp(textName, "hacker") && hasName == 1) {
		if (_color != kColorRed && menuScreen == 0)
			color_abc(kColorRed);
	} else if (menuScreen == 0 && _color != kColorLightGreen)
		color_abc(kColorLightGreen);
	if (hasName == 1 && menuScreen == 0)
		centerText(textName, mouseX, mouseY);
	if (menuScreen == 1)
		showMenu();
	else if (menuBar == 1)
		clearMenu();

	int cursorPos[6] = { 0, 0, mouseX - 20, mouseY - 17, OBJWIDTH, OBJHEIGHT };
	copyRectClip(cursorPos, drawSurface3, screenSurface);
}

void DrasculaEngine::setCursorTable() {
	int cursorPos[6] = { 225, 56, mouseX - 20, mouseY - 12, 40, 25 };
	copyRectClip(cursorPos, tableSurface, screenSurface);
}

void DrasculaEngine::loadPic(const char *NamePcc, byte *targetSurface, int colorCount) {
	unsigned int con, x = 0;
	unsigned int fExit = 0;
	byte ch, rep;

	_arj.open(NamePcc);
	if (!_arj.isOpen())
		error("missing game data %s %c", NamePcc, 7);

	_arj.seek(128);
	while (!fExit) {
		ch = _arj.readByte();
		rep = 1;
		if ((ch & 192) == 192) {
			rep = (ch & 63);
			ch = _arj.readByte();
		}
		for (con = 0; con < rep; con++) {
			x++;
			if (x > 64000) {
				fExit = 1;
				break;
			}
			*targetSurface++ = ch;
		}
	}

	_arj.read(cPal, 768);
	_arj.close();

	setRGB((byte *)cPal, colorCount);
}

void DrasculaEngine::showFrame(bool firstFrame) {
	int dataSize = _arj.readSint32LE();
	byte *pcxData = (byte *)malloc(dataSize);
	_arj.read(pcxData, dataSize);
	_arj.read(cPal, 768);
	byte *prevFrame = (byte *)malloc(64000);
	memcpy(prevFrame, VGA, 64000);

	decodeRLE(pcxData, VGA);

	free(pcxData);

	if (!firstFrame)
		mixVideo(VGA, prevFrame);

	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
	if (firstFrame)
		setPalette(cPal);

	free(prevFrame);
}

void DrasculaEngine::copyBackground(int xorg, int yorg, int xdes, int ydes, int width,
								  int height, byte *src, byte *dest) {
	dest += xdes + ydes * 320;
	src += xorg + yorg * 320;
	for (int x = 0; x < height; x++) {
		memcpy(dest, src, width);
		dest += 320;
		src += 320;
	}
}

void DrasculaEngine::copyRect(int xorg, int yorg, int xdes, int ydes, int width,
								   int height, byte *src, byte *dest) {
	int y, x;

	dest += xdes + ydes * 320;
	src += xorg + yorg * 320;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			if (src[x + y * 320] != 255)
				dest[x + y * 320] = src[x + y * 320];
}

void DrasculaEngine::copyRectClip(int *Array, byte *src, byte *dest) {
	int y, x;
	int xorg = Array[0];
	int yorg = Array[1];
	int xdes = Array[2];
	int ydes = Array[3];
	int width = Array[4];
	int height = Array[5];

	if (ydes < 0) {
		yorg += -ydes;
		height += ydes;
		ydes = 0;
	}
	if (xdes < 0) {
		xorg += -xdes;
		width += xdes;
		xdes = 0;
	}
	if ((xdes + width) > 319)
		width -= (xdes + width) - 320;
	if ((ydes + height) > 199)
		height -= (ydes + height) - 200;

	dest += xdes + ydes * 320;
	src += xorg + yorg * 320;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			if (src[x + y * 320] != 255)
				dest[x + y * 320] = src[x + y * 320];
}

void DrasculaEngine::updateScreen(int xorg, int yorg, int xdes, int ydes, int width, int height, byte *buffer) {
	byte *ptr = VGA;

	ptr += xdes + ydes * 320;
	buffer += xorg + yorg * 320;
	for (int x = 0; x < height; x++) {
		memcpy(ptr, buffer, width);
		ptr += 320;
		buffer += 320;
	}

	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void DrasculaEngine::print_abc(const char *said, int screenX, int screenY) {
	int letterY = 0, letterX = 0, i;
	uint len = strlen(said);
	byte c;

	for (uint h = 0; h < len; h++) {
		c = toupper(said[h]);

		for (i = 0; i < _charMapSize; i++) {
			if (c == _charMap[i].inChar) {
				letterX = _charMap[i].mappedChar;

				switch (_charMap[i].charType) {
					case 0:		// letters
						letterY = (_lang == kSpanish) ? 149 : 158;
						break;
					case 1:		// signs
						letterY = (_lang == kSpanish) ? 160 : 169;
						break;
					case 2:		// accented
						letterY = 180;
						break;
				}	// switch
				break;
			}	// if
		}	// for

		int textPos[6] = { letterX, letterY, screenX, screenY, CHAR_WIDTH, CHAR_HEIGHT };
		copyRectClip(textPos, textSurface, screenSurface);

		screenX = screenX + CHAR_WIDTH;
		if (screenX > 317) {
			screenX = 0;
			screenY = screenY + CHAR_HEIGHT + 2;
		}
	}	// for
}

void DrasculaEngine::print_abc_opc(const char *said, int screenX, int screenY, int game) {
	int signY, letterY, letterX = 0;
	uint len = strlen(said);

	for (uint h = 0; h < len; h++) {
		if (game == 1) {
			letterY = 6;
			signY = 15;
		} else if (game == 3) {
			letterY = 56;
			signY = 65;
		} else {
			letterY = 31;
			signY = 40;
		}

		int c = toupper(said[h]);

		// WORKAROUND: Even original did not process it correctly
		// Fixes apostrophe rendering
		if (_lang != kSpanish)
			if (c == '\'')
				c = '\244';

		for (int i = 0; i < _charMapSize; i++) {
			if (c == _charMap[i].inChar) {
				// Convert the mapped char of the normal font to the
				// mapped char of the dialogue font

				int multiplier = (_charMap[i].mappedChar - 6) / 9;

				letterX = multiplier * 7 + 10;

				if (_charMap[i].charType > 0)
					letterY = signY;
				break;
			}	// if
		}	// for

		int textPos[6] = { letterX, letterY, screenX, screenY, CHAR_WIDTH_OPC, CHAR_HEIGHT_OPC };
		copyRectClip(textPos, backSurface, screenSurface);

		screenX = screenX + CHAR_WIDTH_OPC;
	}
}

void DrasculaEngine::centerText(const char *message, int textX, int textY) {
	char bb[200], m2[200], m1[200], mb[10][50];
	char m3[200];
	int h, fil, textX3, textX2, textX1, conta_f = 0, ya = 0;

	strcpy(m1, " ");
	strcpy(m2, " ");
	strcpy(m3, " ");
	strcpy(bb, " ");

	for (h = 0; h < 10; h++)
		strcpy(mb[h], " ");

	if (textX > 160)
		ya = 1;

	strcpy(m1, message);
	textX = CLIP<int>(textX, 60, 255);

	textX1 = textX;

	if (ya == 1)
		textX1 = 315 - textX;

	textX2 = (strlen(m1) / 2) * CHAR_WIDTH;

	while (true) {
		strcpy(bb, m1);
		scumm_strrev(bb);

		if (textX1 < textX2) {
			strcpy(m3, strrchr(m1, ' '));
			strcpy(m1, strstr(bb, " "));
			scumm_strrev(m1);
			m1[strlen(m1) - 1] = '\0';
			strcat(m3, m2);
			strcpy(m2, m3);
		};

		textX2 = (strlen(m1) / 2) * CHAR_WIDTH;

		if (textX1 < textX2)
			continue;

		strcpy(mb[conta_f], m1);

		if (!strcmp(m2, ""))
			break;

		scumm_strrev(m2);
		m2[strlen(m2) - 1] = '\0';
		scumm_strrev(m2);
		strcpy(m1, m2);
		strcpy(m2, "");
		conta_f++;
	}

	fil = textY - (((conta_f + 3) * CHAR_HEIGHT));

	for (h = 0; h < conta_f + 1; h++) {
		textX3 = strlen(mb[h]) / 2;
		print_abc(mb[h], ((textX) - textX3 * CHAR_WIDTH) - 1, fil);
		fil = fil + CHAR_HEIGHT + 2;
	}
}

void DrasculaEngine::screenSaver() {
	int xr, yr;
	byte *copia, *ghost;
	float coeff = 0, coeff2 = 0;
	int count = 0;
	int count2 = 0;
	int tempLine[320];
	int tempRow[200];

	clearRoom();

	loadPic("sv.alg", drawSurface1, HALF_PAL);

	// inicio_ghost();
	copia = (byte *)malloc(64000);
	ghost = (byte *)malloc(65536);

	// carga_ghost();
	_arj.open("ghost.drv");
	if (!_arj.isOpen())
		error("Cannot open file ghost.drv");

	_arj.read(ghost, 65536);
	_arj.close();

	updateEvents();
	xr = mouseX;
	yr = mouseY;

	for (;;) {
		// efecto(drawSurface1);

		memcpy(copia, drawSurface1, 64000);
		coeff += 0.1f;
		coeff2 = coeff;

		if (++count > 319)
			count = 0;

		for (int i = 0; i < 320; i++) {
			tempLine[i] = (int)(sin(coeff2) * 16);
			coeff2 += 0.02f;
			tempLine[i] = checkWrapY(tempLine[i]);
		}

		coeff2 = coeff;
		for (int i = 0; i < 200; i++) {
			tempRow[i] = (int)(sin(coeff2) * 16);
			coeff2 += 0.02f;
			tempRow[i] = checkWrapX(tempRow[i]);
		}

		if (++count2 > 199)
			count2 = 0;

		int x1_, y1_, off1, off2;

		for (int i = 0; i < 200; i++) {
			for (int j = 0; j < 320; j++) {
				x1_ = j + tempRow[i];
				x1_ = checkWrapX(x1_);

				y1_ = i + count2;
				y1_ = checkWrapY(y1_);

				off1 = 320 * y1_ + x1_;

				x1_ = j + count;
				x1_ = checkWrapX(x1_);

				y1_ = i + tempLine[j];
				y1_ = checkWrapY(y1_);
				off2 = 320 * y1_ + x1_;

				VGA[320 * i + j] = ghost[drawSurface1[off2] + (copia[off1] << 8)];
			}
		}
		_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
		_system->updateScreen();

		_system->delayMillis(20);

		// end of efecto()

		updateEvents();
		if (rightMouseButton == 1 || leftMouseButton == 1)
			break;
		if (mouseX != xr)
			break;
		if (mouseY != yr)
			break;
	}
	// fin_ghost();
	free(copia);
	free(ghost);

	loadPic(roomNumber, drawSurface1, HALF_PAL);
}

void DrasculaEngine::playFLI(const char *filefli, int vel) {
	// Open file
	MiVideoSSN = (byte *)malloc(64256);
	globalSpeed = 1000 / vel;
	FrameSSN = 0;
	UsingMem = 0;
	if (MiVideoSSN == NULL)
		return;
	_arj.open(filefli);
	mSession = TryInMem();
	LastFrame = _system->getMillis();

	while (playFrameSSN() && (!term_int)) {
		if (getScan() == Common::KEYCODE_ESCAPE)
			term_int = 1;
	}

	free(MiVideoSSN);
	if (UsingMem)
		free(memPtr);
	else
		_arj.close();
}

int DrasculaEngine::playFrameSSN() {
	int Exit = 0;
	uint32 Lengt;
	byte *BufferSSN;

	if (!UsingMem)
		_arj.read(&CHUNK, 1);
	else {
		memcpy(&CHUNK, mSession, 1);
		mSession += 1;
	}

	switch (CHUNK) {
	case kFrameSetPal:
		if (!UsingMem)
			_arj.read(dacSSN, 768);
		else {
			memcpy(dacSSN, mSession, 768);
			mSession += 768;
		}
		setPalette(dacSSN);
		break;
	case kFrameEmptyFrame:
		waitFrameSSN();
		break;
	case kFrameInit:
		if (!UsingMem) {
			CMP = _arj.readByte();
			Lengt = _arj.readUint32LE();
		} else {
			memcpy(&CMP, mSession, 1);
			mSession += 1;
			Lengt = READ_LE_UINT32(mSession);
			mSession += 4;
		}
		if (CMP == kFrameCmpRle) {
			BufferSSN = (byte *)malloc(Lengt);
			if (!UsingMem) {
				_arj.read(BufferSSN, Lengt);
			} else {
				memcpy(BufferSSN, mSession, Lengt);
				mSession += Lengt;
			}
			decodeRLE(BufferSSN, MiVideoSSN);
			free(BufferSSN);
			waitFrameSSN();
			if (FrameSSN)
				mixVideo(VGA, MiVideoSSN);			
			else
				memcpy(VGA, MiVideoSSN, 64000);
			_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
			_system->updateScreen();
			FrameSSN++;
		} else {
			if (CMP == kFrameCmpOff) {
				BufferSSN = (byte *)malloc(Lengt);
				if (!UsingMem) {
					_arj.read(BufferSSN, Lengt);
				} else {
					memcpy(BufferSSN, mSession, Lengt);
					mSession += Lengt;
				}
				decodeOffset(BufferSSN, MiVideoSSN, Lengt);
				free(BufferSSN);
				waitFrameSSN();
				if (FrameSSN)
					mixVideo(VGA, MiVideoSSN);
				else
					memcpy(VGA, MiVideoSSN, 64000);
				_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
				_system->updateScreen();
				FrameSSN++;
			}
		}
		break;
	case kFrameEndAnim:
		Exit = 1;
		break;
	default:
		Exit = 1;
		break;
	}

	return (!Exit);
}

byte *DrasculaEngine::TryInMem() {
	int Lengt;

	_arj.seek(0, SEEK_END);
	Lengt = _arj.pos();
	_arj.seek(0, SEEK_SET);
	memPtr = (byte *)malloc(Lengt);
	if (memPtr == NULL)
		return NULL;
	_arj.read(memPtr, Lengt);
	UsingMem = 1;
	_arj.close();

	return memPtr;
}

void DrasculaEngine::decodeOffset(byte *BufferOFF, byte *MiVideoOFF, int length) {
	int x = 0;
	unsigned char Reps;
	int Offset;

	memset(MiVideoSSN, 0, 64000);
	while (x < length) {
		Offset = BufferOFF[x] + BufferOFF[x + 1] * 256;
		Reps = BufferOFF[x + 2];
		memcpy(MiVideoOFF + Offset, &BufferOFF[x + 3], Reps);
		x += 3 + Reps;
	}
}

void DrasculaEngine::decodeRLE(byte* srcPtr, byte* dstPtr) {
	bool stopProcessing = false;
	byte pixel;
	uint repeat;
	int curByte = 0;

	while (!stopProcessing) {
		pixel = *srcPtr++;
		repeat = 1;
		if ((pixel & 192) == 192) {
			repeat = (pixel & 63);
			pixel = *srcPtr++;
		}
		for (uint j = 0; j < repeat; j++) {
			curByte++;
			if (curByte > 64000) {
				stopProcessing = true;
				break;
			}
			*dstPtr++ = pixel;
		}
	}
}

void DrasculaEngine::mixVideo(byte *OldScreen, byte *NewScreen) {
	for (int x = 0; x < 64000; x++)
		OldScreen[x] ^= NewScreen[x];
}

void DrasculaEngine::waitFrameSSN() {
	uint32 now;
	while ((now = _system->getMillis()) - LastFrame < ((uint32) globalSpeed))
		_system->delayMillis(globalSpeed - (now - LastFrame));
	LastFrame = LastFrame + globalSpeed;
}

bool DrasculaEngine::animate(const char *animationFile, int FPS) {
	int NFrames = 1;
	int cnt = 2;

	_arj.open(animationFile);

	if (!_arj.isOpen()) {
		error("Animation file %s not found", animationFile);
	}

	NFrames = _arj.readSint32LE();
	showFrame(true);
	_system->delayMillis(1000 / FPS);
	while (cnt < NFrames) {
		showFrame();
		_system->delayMillis(1000 / FPS);
		cnt++;
		byte key = getScan();
		if (key == Common::KEYCODE_ESCAPE)
			term_int = 1;
		if (key != 0)
			break;
	}
	_arj.close();

	return ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE));
}



} // End of namespace Drascula
