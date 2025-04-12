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

#ifndef HODJNPODJ_VIEWS_MINIGAME_VIEW_H
#define HODJNPODJ_VIEWS_MINIGAME_VIEW_H

#include "common/archive.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/formats/winexe_ne.h"
#include "bagel/hodjnpodj/views/sprites_view.h"
#include "bagel/hodjnpodj/libs/settings.h"

namespace Bagel {
namespace HodjNPodj {

/**
 * Base view class for the main view for each minigame
 */
class MinigameView : public SpritesView {
private:
	int _showMenuCtr = 0;

protected:
	Settings::Domain &_settings;

	virtual void showMainMenu() = 0;

public:
	MinigameView(const Common::String &name, const Common::String &resFilename);
	virtual ~MinigameView() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgOpen(const OpenMessage &msg) override;
	bool msgGame(const GameMessage &msg) override {
		return true;
	}
	bool msgKeypress(const KeypressMessage &msg) override;
	bool tick() override;

	void close();
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
