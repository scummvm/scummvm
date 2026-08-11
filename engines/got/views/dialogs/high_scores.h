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

#ifndef GOT_VIEWS_DIALOGS_HIGH_SCORES_H
#define GOT_VIEWS_DIALOGS_HIGH_SCORES_H

#include "got/views/dialogs/dialog.h"

namespace Got {
namespace Views {
namespace Dialogs {

class HighScores : public Dialog {
private:
	int _currentArea = 0;
	bool _showAll = false;
	int _timeoutCtr = 0;

	void goToMainMenu();
	void goToNextArea();

public:
	HighScores();
	virtual ~HighScores() {
	}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool tick() override;
};

} // namespace Dialogs
} // namespace Views
} // namespace Got

#endif
