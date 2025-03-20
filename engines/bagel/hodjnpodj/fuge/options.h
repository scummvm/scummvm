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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_FUGE_OPTIONS_H
#define HODJNPODJ_FUGE_OPTIONS_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/gfx/scrollbar.h"
#include "bagel/hodjnpodj/libs/settings.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

class Options : public View {
private:
	GfxSurface _background;
	OkButton _okButton;
	CancelButton _cancelButton;
	DefaultsButton _defaultsButton;
	Common::Rect _numBallsRect;
	Common::Rect _startLevelRect;
	Common::Rect _ballSpeedRect;
	Common::Rect _paddleSizeRect;
	ScrollBar _numBallsScroll;
	ScrollBar _startLevelScroll;
	ScrollBar _ballSpeedScroll;
	ScrollBar _paddleSizeScroll;
	CheckButton _outerWallCheck;
	Settings::Domain &_settings;
	bool _hasChanges = false;
	int _numBalls = 0;
	int _startLevel = 0;
	int _ballSpeed = 0;
	int _paddleSize = 0;
	bool _outerWall = false;

	void reset();
	void putDialogData();
	void loadIniSettings();
	void saveIniSettings();

public:
	Options();
	virtual ~Options() {
	}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
