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

#ifndef KYRA_GUI_MR_H
#define KYRA_GUI_MR_H

#include "kyra/gui/gui_v2.h"

namespace Kyra {

class KyraEngine_MR;
class Screen_MR;

class GUI_MR : public GUI_v2 {
friend class KyraEngine_MR;
public:
	GUI_MR(KyraEngine_MR *engine);

	void initStaticData() override;

	void flagButtonEnable(Button *button);
	void flagButtonDisable(Button *button);

	int redrawShadedButtonCallback(Button *button) override;
	int redrawButtonCallback(Button *button) override;

	int optionsButton(Button *button);

	void createScreenThumbnail(Graphics::Surface &dst) override;
private:
	Common::String getMenuTitle(const Menu &menu) override;
	Common::String getMenuItemTitle(const MenuItem &menuItem) override;
	Common::String getMenuItemLabel(const MenuItem &menuItem) override;
	Common::String getTableString(int id, bool) override;

	uint8 textFieldColor1() const override { return 0xFF; }
	uint8 textFieldColor2() const override { return 0xCF; }
	uint8 textFieldColor3() const override { return 0xBA; }

	uint8 defaultColor1() const override { return 0xF0; }
	uint8 defaultColor2() const override { return 0xD0; }

	void resetState(int item);

	int quitGame(Button *button);
	int loadMenu(Button *button);
	int loadSecondChance(Button *button);

	int gameOptions(Button *button);
	void setupOptionsButtons() override;

	int audioOptions(Button *button);

	int sliderHandler(Button *caller) override;
	void drawSliderBar(int slider, const uint8 *shape);

	int changeLanguage(Button *caller);
	int toggleStudioSFX(Button *caller);
	int toggleSkipSupport(Button *caller);
	int toggleHeliumMode(Button *caller);

	KyraEngine_MR *_vm;
	Screen_MR *_screen;
};

} // End of namespace Kyra

#endif
