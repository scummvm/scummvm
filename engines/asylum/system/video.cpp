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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/video.h"

#include "asylum/system//graphics.h"
#include "asylum/system/text.h"

#include "asylum/staticres.h"

namespace Asylum {

Video::Video(Audio::Mixer *mixer): _skipVideo(false) {
	Common::Event stopEvent;
	_stopEvents.clear();
	stopEvent.type = Common::EVENT_KEYDOWN;
	stopEvent.kbd.keycode  = Common::KEYCODE_ESCAPE;
	_stopEvents.push_back(stopEvent);

	_smkDecoder = new Graphics::SmackerDecoder(mixer);

	_text = new VideoText();
	ResourcePack *resPack = new ResourcePack(1);
	_text->loadFont(resPack, 57);	// video font
	delete resPack;
}

Video::~Video() {
	delete _smkDecoder;
	delete _text;
}

bool Video::playVideo(int32 videoNumber, bool showSubtitles) {
	bool lastMouseState = false;
	char filename[20];

	sprintf(filename, "mov%03d.smk", videoNumber);

	bool result = _smkDecoder->loadFile(filename);

	lastMouseState = g_system->showMouse(false);
	if (result) {
		_skipVideo = false;

		if (showSubtitles)
			loadSubtitles(videoNumber);

		uint16 x = (g_system->getWidth() - _smkDecoder->getWidth()) / 2;
		uint16 y = (g_system->getHeight() - _smkDecoder->getHeight()) / 2;

		while (!_smkDecoder->endOfVideo() && !_skipVideo) {
			processVideoEvents();
			if (_smkDecoder->needsUpdate()) {
				Graphics::Surface *frame = _smkDecoder->decodeNextFrame();

				if (frame) {
					g_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, x, y, frame->w, frame->h);

					if(showSubtitles) {
						Graphics::Surface *screen = g_system->lockScreen();
						performPostProcessing((byte *)screen->pixels);
						g_system->unlockScreen();
					}

					if (_smkDecoder->hasDirtyPalette())
						_smkDecoder->setSystemPalette();

					g_system->updateScreen();
				}
			}
			g_system->delayMillis(10);
		}
	}
	_smkDecoder->close();
	_subtitles.clear();
	g_system->showMouse(lastMouseState);

	return result;
}

void Video::performPostProcessing(byte *screen) {
	int32 curFrame = _smkDecoder->getCurFrame();

	// Reset subtitle area, by filling it with zeroes
	memset(screen + 640 * 400, 0, 640 * 80);

	for (uint32 i = 0; i < _subtitles.size(); i++) {
		VideoSubtitle curSubtitle = _subtitles[i];
		if (curFrame >= curSubtitle.frameStart &&
		        curFrame <= curSubtitle.frameEnd) {
			_text->drawMovieSubtitle(screen, curSubtitle.textRes);
			break;
		}
	}
}

void Video::loadSubtitles(int32 videoNumber) {
	// Read vids.cap

	char movieToken[10];
	sprintf(movieToken, "[MOV%03d]", videoNumber);

	Common::File subsFile;
	subsFile.open("vids.cap");
	char *buffer = new char[subsFile.size()];
	subsFile.read(buffer, subsFile.size());
	subsFile.close();

	char *start = strstr(buffer, movieToken);
	char *line = 0;

	if (start) {
		start += 20; // skip token, newline and "CAPTION = "

		int32 count = strcspn(start, "\r\n");
		line = new char[count + 1];

		strncpy(line, start, count);
		line[count] = 0;

		char *tok = strtok(line, " ");

		while (tok) {
			VideoSubtitle newSubtitle;
			newSubtitle.frameStart = atoi(tok);
			tok = strtok(NULL, " ");
			newSubtitle.frameEnd = atoi(tok);
			tok = strtok(NULL, " ");
			newSubtitle.textRes = atoi(tok) + video_subtitle_resourceIds[videoNumber];
			tok = strtok(NULL, " ");

			_subtitles.push_back(newSubtitle);
		}

		delete [] line;
	}

	delete [] buffer;
}

void Video::processVideoEvents() {
	Common::Event curEvent;
	while (g_system->getEventManager()->pollEvent(curEvent)) {
		if (curEvent.type == Common::EVENT_RTL || curEvent.type == Common::EVENT_QUIT) {
			_skipVideo = true;
		}

		for (Common::List<Common::Event>::const_iterator iter = _stopEvents.begin(); iter != _stopEvents.end(); ++iter) {
			if (curEvent.type == iter->type) {
				if (iter->type == Common::EVENT_KEYDOWN || iter->type == Common::EVENT_KEYUP) {
					if (curEvent.kbd.keycode == iter->kbd.keycode) {
						_skipVideo = true;
						break;
					}
				} else {
					_skipVideo = true;
					break;
				}
			}
		}
	}
}


VideoText::VideoText() {
	_curFontFlags = 0;
	_fontResource = 0;

	_textPack = new ResourcePack(0);
}

VideoText::~VideoText() {
	delete _textPack;
	delete _fontResource;
}

void VideoText::loadFont(ResourcePack *resPack, ResourceId resourceId) {
	delete _fontResource;

	_fontResource = new GraphicResource(resPack, resourceId);

	if (resourceId > 0) {
		// load font flag data
		_curFontFlags = (_fontResource->getFlags() >> 4) & 0x0F;
	}
}

void VideoText::drawMovieSubtitle(byte *screenBuffer, ResourceId resourceId) {
	Common::String textLine[4];
	Common::String tmpLine;
	int32 curLine = 0;
	ResourceEntry *textRes = _textPack->getResource(resourceId);
	char *text = strdup((const char *)textRes->data);	// for strtok
	char *tok  = strtok(text, " ");
	int32 startY  = 420; // starting y for up to 2 subtitles
	int32 spacing = 30;  // spacing for up to 2 subtitles

	// Videos can have up to 4 lines of text
	while (tok) {
		tmpLine += tok;
		tmpLine += " ";
		if (getTextWidth(tmpLine.c_str()) > 640) {
			tmpLine = tok;
			curLine++;
			if (curLine >= 2) {
				startY  = 410; // starting Y for 3 subtitles
				spacing = 20;  // spacing for 3-4 subtitles
			}
			if (curLine >= 3) {
				startY = 402;  // starting Y for 4 subtitles
			}
		}
		textLine[curLine] += tok;
		textLine[curLine] += " ";
		tok = strtok(NULL, " ");
	}

	for (int32 i = 0; i < curLine + 1; i++) {
		int32 textWidth = getTextWidth(textLine[i].c_str());
		drawText(screenBuffer, 0 + (640 - textWidth) / 2, startY + i * spacing, textLine[i].c_str());
	}

	free(text);
}

int32 VideoText::getTextWidth(const char *text) {
	assert(_fontResource);

	int32 width = 0;
	uint8 character = *text;
	const char *curChar = text;

	while (character) {
		GraphicFrame *font = _fontResource->getFrame(character);
		width += font->surface.w + font->x - _curFontFlags;

		curChar++;
		character = *curChar;
	}
	return width;
}

void VideoText::drawText(byte *screenBuffer, int16 x, int16 y, const char *text) {
	assert(_fontResource);
	const byte *curChar = (byte *)text;
	int16 curX = x;

	while (*curChar) {
		GraphicFrame *fontLetter = _fontResource->getFrame(*curChar);
		copyToVideoFrame(screenBuffer, fontLetter, curX, y + fontLetter->y);
		curX += fontLetter->surface.w + fontLetter->x - _curFontFlags;
		curChar++;
	}
}

void VideoText::copyToVideoFrame(byte *screenBuffer, GraphicFrame *frame, int32 x, int32 y) {
	uint16 h = frame->surface.h;
	uint16 w = frame->surface.w;
	int32 screenBufferPitch = 640;
	byte *buffer = (byte *)frame->surface.pixels;
	byte *dest   = screenBuffer + y * screenBufferPitch + x;

	while (h--) {
		memcpy(dest, buffer, w);
		dest   += screenBufferPitch;
		buffer += frame->surface.w;
	}
}

} // end of namespace Asylum
