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

#include "asylum/video.h"

namespace Asylum {

VideoPlayer::VideoPlayer(Graphics::VideoDecoder *decoder) : 
							Graphics::VideoPlayer(decoder) {
	_text = new VideoText();
	ResourcePack *resPack = new ResourcePack(1);
	_text->loadFont(resPack, 57);	// video font
	delete resPack;
}

VideoPlayer::~VideoPlayer() {
	delete _text;
}

bool VideoPlayer::playVideoWithSubtitles(Common::List<Common::Event> &stopEvents, int videoNumber) {
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

	// We hardcode all the text resources here. It makes the resulting code easier,
	// otherwise we'll have to read the text resources in the same obscure way they're stored
	// in vids.cap
	// -1 means that the video has no subtitles, -2 that it doesn't exist
	// The negative values aren't used in the code, they just make the table easier to
	// understand.
	int textRes[49] = {   -1, 1088, 1279, 1122, 1286, 1132, 1133, 1134, 1135, 1136,	//  0 - 9
						  -1,   -2, 1140, 1141,   -2,   -1, 1142,   -1,   -2, 1155,	// 10 - 19
						1157, 1159, 1162, 1164,   -2, 1171, 1177, 1184, 1190, 1201,	// 20 - 29
						  -2,   -2,   -2, 1207, 1213, 1217, 1223, 1227,   -2, 1228,	// 30 - 39
						  -2, 1244, 1247, 1250, 1256, 1120, 1127,   -1,   -1};    	// 40 - 48

	if (start) {
		start += 20;	// skip token, newline and "CAPTION = "

		int count = strcspn(start, "\r\n");
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
			newSubtitle.textRes = atoi(tok) + textRes[videoNumber];
			tok = strtok(NULL, " ");

			_subtitles.push_back(newSubtitle);
		}

		delete line;
	}

	delete buffer;

	return playVideo(stopEvents);
}

void VideoPlayer::performPostProcessing(byte *screen) {
	int curFrame = _decoder->getCurFrame();

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

Video::Video(Audio::Mixer *mixer) {
	Common::Event stopEvent;
	_stopEvents.clear();
	stopEvent.type = Common::EVENT_KEYDOWN;
	stopEvent.kbd  = Common::KEYCODE_ESCAPE;
	_stopEvents.push_back(stopEvent);

	_smkDecoder  = new Graphics::SmackerDecoder(mixer);
	_player      = new VideoPlayer(_smkDecoder);
}

Video::~Video() {
	delete _player;
	delete _smkDecoder;
}

bool Video::playVideo(int number, VideoSubtitles subtitles) {
	char filename[20];
	sprintf(filename, "mov%03d.smk", number);

	bool result = _smkDecoder->loadFile(filename);

	g_system->showMouse(false);
	if (result) {
		if (subtitles == kSubtitlesOff)
			_player->playVideo(_stopEvents);
		else
			_player->playVideoWithSubtitles(_stopEvents, number);
	}
	_smkDecoder->closeFile();
	g_system->showMouse(true);

	return result;
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

void VideoText::loadFont(ResourcePack *resPack, uint32 resId) {
	delete _fontResource;

	ResourceEntry *fontRes = resPack->getResource(resId);
	_fontResource = new GraphicResource(fontRes->data, fontRes->size);

    if (resId > 0) {
        // load font flag data
        _curFontFlags = (_fontResource->getFlags() >> 4) & 0x0F; 
    }
}

void VideoText::drawMovieSubtitle(byte *screenBuffer, uint32 resId) {
	Common::String textLine[4];
	Common::String tmpLine;
	int curLine = 0;
	ResourceEntry *textRes = _textPack->getResource(resId);
	char *text = strdup((const char *)textRes->data);	// for strtok
	char *tok = strtok(text, " ");
	int startY = 420;	// starting y for up to 2 subtitles
	int spacing = 30;	// spacing for up to 2 subtitles

	// Videos can have up to 4 lines of text
	while (tok) {
		tmpLine += tok;
		tmpLine += " ";
		if (getTextWidth(tmpLine.c_str()) > 640) {
			tmpLine = tok;
			curLine++;
			if (curLine >= 2) {
				startY = 410;	// starting Y for 3 subtitles
				spacing = 20;	// spacing for 3-4 subtitles
			}
			if (curLine >= 3) {
				startY = 402;	// starting Y for 4 subtitles
			}
		}
		textLine[curLine] += tok;
		textLine[curLine] += " ";
		tok = strtok(NULL, " ");
	}

	for (int i = 0; i < curLine + 1; i++) {
		int textWidth = getTextWidth(textLine[i].c_str());
		drawText(screenBuffer, 0 + (640 - textWidth) / 2, startY + i * spacing, textLine[i].c_str());
	}

	free(text);
}

uint32 VideoText::getTextWidth(const char *text) {
	assert (_fontResource);

    int width = 0;
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

void VideoText::drawText(byte *screenBuffer, int x, int y, const char *text) {
	assert (_fontResource);
	const char *curChar = text;
	int curX = x;

    while (*curChar) {
		GraphicFrame *fontLetter = _fontResource->getFrame(*curChar);
		copyToVideoFrame(screenBuffer, fontLetter, curX, y + fontLetter->y);
		curX += fontLetter->surface.w + fontLetter->x - _curFontFlags;
        curChar++;
    }
}

void VideoText::copyToVideoFrame(byte *screenBuffer, GraphicFrame *frame, int x, int y) {
	int h = frame->surface.h;
    int w = frame->surface.w;
	int screenBufferPitch = 640;
	byte *buffer = (byte *)frame->surface.pixels;
	byte *dest = screenBuffer + y * screenBufferPitch + x;

	while (h--) {
		memcpy(dest, buffer, w);
		dest += screenBufferPitch;
		buffer += frame->surface.w;
	}
}

} // end of namespace Asylum
