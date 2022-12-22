/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "neverhood/resource.h"
#include "neverhood/resourceman.h"
#include "neverhood/subtitles.h"

namespace Neverhood {

namespace {
void drawSubtitles(Graphics::Surface *surf, const Common::String &str, const SubtitleGlyph *subfont, int x0) {
	if (!surf || surf->h < SubtitlePlayer::kSubtitleCharHeight || !subfont)
		return;

	byte *dest0 = (byte*)surf->getBasePtr(0, 0);

	int lastx = MIN<int>(str.size() * SubtitlePlayer::kSubtitleCharWidth + x0, surf->w);
	for (int16 yc = 0; yc < SubtitlePlayer::kSubtitleCharHeight; yc++) {
		byte *dest = dest0 + yc * surf->pitch;
		memset(dest, SubtitlePlayer::kSubtitleAlpha, x0);
		memset(dest + lastx, SubtitlePlayer::kSubtitleAlpha, surf->w - lastx);
	}

	for (int i = 0; i < (int)str.size() && i * SubtitlePlayer::kSubtitleCharWidth < surf->w; i++) {
		byte c = str[i];
		byte *dest = dest0 + i * SubtitlePlayer::kSubtitleCharWidth + x0;
		for (int16 yc = 0; yc < SubtitlePlayer::kSubtitleCharHeight; yc++) {
			byte *row = dest;
			for (int16 xc = 0; xc < SubtitlePlayer::kSubtitleCharWidth; xc++, row++) {
				if ((subfont[c].bitmap[yc] << xc) & 0x80)
					*row = 0xff;
				else if ((subfont[c].outline[yc] << xc) & 0x80)
					*row = 0x00;
				else
					*row = SubtitlePlayer::kSubtitleAlpha;
			}
			dest += surf->pitch;
		}
	}
}
}
		
SubtitlePlayer::SubtitlePlayer(NeverhoodEngine *vm, uint32 fileHash, int width) :
	_vm(vm), _haveBottomSubs(false), _haveTopSubs(false), _currentFrame(-1), _isValid(false) {
	if (!vm->getSubfont())
		return;
	_isValid = true;
  	_bottomSubs.create(width, kSubtitleCharHeight, Graphics::PixelFormat::createFormatCLUT8());
  	_topSubs.create(width, kSubtitleCharHeight, Graphics::PixelFormat::createFormatCLUT8());

	Common::SeekableReadStream *s = vm->_res->createNhcStream(fileHash, kResNhcTypeSubtitles);
	while (s && !s->eos()) {
		Subtitle sub;
		sub.fromFrame = s->readUint32LE();
		sub.toFrame = s->readUint32LE();
		sub.text = s->readString('\0', 40);
		if (!sub.text.empty() && sub.text[0] == '^') {
			sub.isTop = true;
			sub.text = sub.text.substr(1);
		} else {
			sub.isTop = false;
		}
		_subtitles.push_back(sub);
	}
	delete s;
}

void SubtitlePlayer::renderFrame(uint frameNumber, int centerX) {
	// Reuse old rendering if no frame has passed
	if (_currentFrame == (int64)frameNumber)
		return;

	const SubtitleGlyph *subFont = _vm->getSubfont();
	if (!subFont)
		return;

	int screenWidth = _bottomSubs.w;

	_haveBottomSubs = false;
	_haveTopSubs = false;

	// TODO: Optimize this
	for (uint i = 0; i < _subtitles.size(); i++) {
		if (frameNumber < _subtitles[i].fromFrame || frameNumber > _subtitles[i].toFrame)
			continue;
		Common::String curStr = _subtitles[i].text;
		if ((int)curStr.size() > screenWidth / SubtitlePlayer::kSubtitleCharWidth)
			curStr = curStr.substr(0, screenWidth / SubtitlePlayer::kSubtitleCharWidth - 3) + "...";
		int width = curStr.size() * SubtitlePlayer::kSubtitleCharWidth;
		int startX = MAX(MIN(centerX - width / 2, screenWidth - width), 0);

		if (_subtitles[i].isTop) {
			drawSubtitles(&_topSubs, curStr, subFont, startX);
			_haveTopSubs = true;
		} else {
			drawSubtitles(&_bottomSubs, curStr, subFont, startX);
			_haveBottomSubs = true;
		}
	}
}

}
