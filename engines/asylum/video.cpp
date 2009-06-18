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
	_text->loadFont(resPack, 16);	// 0x80010010, yellow font
	// TODO: font color
	delete resPack;
}

VideoPlayer::~VideoPlayer() {
	delete _text;
}

bool VideoPlayer::playVideoWithSubtitles(Common::List<Common::Event> &stopEvents, int videoNumber) {
	// FIXME: this is a hack, only for video 1, with information taken from
	// the vids.cap file. The format of the vids.cap file is as follows:
	// [MOVxxx]
	// CAPTION = frameStart frameEnd subtitleIndex
	// TODO: We need to figure out where the index of the first subtitle is
	// loaded, as the subtitle indices are relative to the video being played,
	// and not to the actual subtitle indices inside res.000.
	// For now, we just hardcode the values from vids.cap and res.000

	if (videoNumber == 1) {
		VideoSubtitle newSubtitle;
		newSubtitle.frameStart = 287;	// hardcoded from vids.cap
		newSubtitle.frameEnd = 453;		// hardcoded from vids.cap
		newSubtitle.textRes = 1088;		// hardcoded from vids.cap

		_subtitles.push_back(newSubtitle);
	}

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
			_text->drawResTextCentered(screen, curSubtitle.textRes);
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
	_videoNumber = -1;
}

Video::~Video() {
	delete _player;
	delete _smkDecoder;
}

bool Video::playVideo(int number) {
	char filename[20];
	sprintf(filename, "mov%03d.smk", number);

	bool result = _smkDecoder->loadFile(filename);

	g_system->showMouse(false);
	if (result)
		_player->playVideoWithSubtitles(_stopEvents, number);
	_smkDecoder->closeFile();
	g_system->showMouse(true);

	return result;
}

VideoText::VideoText() {
    _posX = 0;
    _posY = 0;
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

void VideoText::drawResTextCentered(byte *screenBuffer, uint32 resId) {
    ResourceEntry *textRes = _textPack->getResource(resId);
    drawTextCentered(screenBuffer, 0, 400, 640, (char *)textRes->data);
}

void VideoText::setTextPos(uint32 x, uint32 y) {
    _posX = x;
    _posY = y;
}

void VideoText::drawTextCentered(byte *screenBuffer, uint32 x, uint32 y, uint32 width, char *text) {
	// HACK: make sure that the text fits on screen
	char textHack[80];
	strncpy(textHack, text, 80);
	textHack[79] = 0;

    int textWidth = getTextWidth(textHack);
    setTextPos(x + (width - textWidth) / 2, y);
    drawText(screenBuffer, textHack);
}

uint32 VideoText::getTextWidth(char *text) {
	assert (_fontResource);

    int width = 0;
    uint8 character = *text;
    while (character) {
		GraphicFrame *font = _fontResource->getFrame(character);
        width += font->surface.w + font->x - _curFontFlags;

        text++;
        character = *text;
    }
    return width;
}

uint32 VideoText::getResTextWidth(uint32 resId) {
    ResourceEntry *textRes = _textPack->getResource(resId);
    return getTextWidth((char*)textRes->data);
}

void VideoText::drawChar(byte *screenBuffer, char character) {
	assert (_fontResource);

	GraphicFrame *fontLetter = _fontResource->getFrame(character);
	copyToVideoFrame(screenBuffer, fontLetter, _posX, _posY + fontLetter->y);
    _posX += fontLetter->surface.w + fontLetter->x - _curFontFlags;
}

void VideoText::drawText(byte *screenBuffer, char *text) {
    while (*text) {
        drawChar(screenBuffer, *text);
        text++;
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
