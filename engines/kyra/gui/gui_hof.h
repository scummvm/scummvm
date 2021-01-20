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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KYRA_GUI_HOF_H
#define KYRA_GUI_HOF_H

#include "kyra/gui/gui_v2.h"

namespace Kyra {

class KyraEngine_HoF;
class Screen_HoF;

class GUI_HoF : public GUI_v2 {
friend class KyraEngine_HoF;
public:
	GUI_HoF(KyraEngine_HoF *engine);

	void initStaticData() override;

	int optionsButton(Button *button);

	void createScreenThumbnail(Graphics::Surface &dst) override;
private:
	Common::String getMenuTitle(const Menu &menu) override;
	Common::String getMenuItemTitle(const MenuItem &menuItem) override;
	Common::String getMenuItemLabel(const MenuItem &menuItem) override;

	uint8 defaultColor1() const override { return 0xCF; }
	uint8 defaultColor2() const override { return 0xF8; }

	uint8 textFieldColor1() const override { return 0xFD; }
	uint8 textFieldColor2() const override { return 0xFA; }
	uint8 textFieldColor3() const override { return 0xFE; }

	void setupPalette() override;
	void restorePalette() override;

	void resetState(int item);

	Common::String getTableString(int id, bool decode) override;

	KyraEngine_HoF *_vm;
	Screen_HoF *_screen;

	int quitGame(Button *caller);
	int loadMenu(Button *caller);
	int audioOptions(Button *caller);
	int gameOptions(Button *caller);
	int gameOptionsTalkie(Button *caller);

	int changeLanguage(Button *caller);

	void setupOptionsButtons() override;

	int sliderHandler(Button *caller) override;
	void drawSliderBar(int slider, const uint8 *shape);

	static const uint16 _menuStringsTalkie[];
	static const uint16 _menuStringsOther[];
};

} // End of namespace Kyra

#endif
