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

#ifndef GOT_VIEWS_SPLASH_SCREEN_H
#define GOT_VIEWS_SPLASH_SCREEN_H

#include "got/views/view.h"

namespace Got {
namespace Views {

class SplashScreen : public View {
private:
	const byte *_chunkSize = nullptr;
	const byte *_chunkPtr = nullptr;
	int _frameCount = 0;
	int _delayCtr = 0, _frameCtr = 0;

	void executeFrame(const byte *src, byte *dest);

public:
	SplashScreen() : View("SplashScreen") {}
	virtual ~SplashScreen() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Views
} // namespace Got

#endif
