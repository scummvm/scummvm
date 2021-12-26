/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_LIVETEXT_H
#define BURIED_LIVETEXT_H

#include "common/str.h"

#include "buried/window.h"

namespace Graphics {
class Font;
}

namespace Buried {

class LiveTextWindow : public Window {
public:
	LiveTextWindow(BuriedEngine *vm, Window *parent);
	~LiveTextWindow();

	bool updateLiveText(const Common::String &text = "", bool notifyUser = true);
	bool updateTranslationText(const Common::String &text = "", bool notifyUser = true);

	void translateBiochipClosing();

	void onPaint();
	void onEnable(bool enable);

private:
	Graphics::Font *_font;
	int _fontHeight;
	bool _textTranslation;
	Common::String _text;
};

} // End of namespace Buried

#endif
