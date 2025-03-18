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
 * aint32 with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_MAZEDOOM_OPTIONS_H
#define HODJNPODJ_MAZEDOOM_OPTIONS_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/gfx/scrollbar.h"
#include "bagel/hodjnpodj/libs/settings.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

extern const int16 TIME_SCALES[10];

class Options : public View {
private:
	GfxSurface _background;
	OkButton _okButton;
	CancelButton _cancelButton;
	ScrollBar _difficultyScroll;
	ScrollBar _timeScroll;
	const Common::Rect _timeLeftRect;
	const Common::Rect _difficultyRect,
		_difficultyTitleRect;
	const Common::Rect _timeRect;
	Common::String _timeStr;
	Settings::Domain &_settings;
	int _time;
	int _difficulty;

	bool _hasChanges = false;

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

	void setTime(int nMinutes, int nSeconds);
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
