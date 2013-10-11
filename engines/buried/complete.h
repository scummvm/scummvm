/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_COMPLETE_H
#define BURIED_COMPLETE_H

#include "buried/global_flags.h"
#include "buried/window.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Buried {

class VideoWindow;

class CompletionWindow : public Window {
public:
	CompletionWindow(BuriedEngine *vm, Window *parent, const GlobalFlags &globalFlags);
	~CompletionWindow();

	void onPaint();
	bool onEraseBackground();
	void onTimer(uint timer);
	void onLButtonUp(const Common::Point &point, uint flags);

private:
	int _status;
	uint _timer;
	Graphics::Surface *_background;
	int _currentSoundEffectID;

	GlobalFlags _globalFlags;
	Graphics::Font *_textFontA;
	Graphics::Font *_textFontB;
	int _fontHeightA, _fontHeightB;
	bool _walkthroughMode;

	Common::String _scoringTextDescriptions;
	Common::String _scoringTextScores;
	Common::String _scoringTextFinalScore;

	VideoWindow *_gageVideo;
};

} // End of namespace Buried

#endif
