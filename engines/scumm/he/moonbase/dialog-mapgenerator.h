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

#ifndef SCUMM_DIALOG_MAP_GENERATOR_H
#define SCUMM_DIALOG_MAP_GENERATOR_H

#include "gui/dialog.h"
#include "gui/widget.h"

namespace Scumm {

class MapGeneratorDialog : public Dialog {
public:
	MapGeneratorDialog(bool demo);

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleKeyDown(Common::KeyState state) override;

private:
	void refresh();
	bool _refreshing;

	GUI::StaticTextWidget *_dialogTitle;

	GUI::StaticTextWidget *_algorDesc;
	GUI::RadiobuttonGroup *_algorGroup;
	GUI::RadiobuttonWidget *_algorSpiff;
	GUI::CheckboxWidget *_algorRandom;

	GUI::StaticTextWidget *_sizeDesc;
	GUI::RadiobuttonGroup *_sizeGroup;
	GUI::RadiobuttonWidget *_sizeSmall;
	GUI::RadiobuttonWidget *_sizeMedium;
	GUI::RadiobuttonWidget *_sizeLarge;
	GUI::RadiobuttonWidget *_sizeHuge;
	GUI::RadiobuttonWidget *_sizeSAI;
	GUI::RadiobuttonWidget *_sizeRidiculous;
	GUI::RadiobuttonWidget *_sizeMax;
	GUI::CheckboxWidget *_sizeRandom;

	GUI::StaticTextWidget *_tileDesc;
	GUI::RadiobuttonGroup *_tileGroup;
	GUI::RadiobuttonWidget *_tileAblation;
	GUI::RadiobuttonWidget *_tileEmerau;
	GUI::RadiobuttonWidget *_tileKyanite;
	GUI::RadiobuttonWidget *_tileDrijim;
	GUI::RadiobuttonWidget *_tileZanateros;
	GUI::RadiobuttonWidget *_tileTerrandra;
	GUI::CheckboxWidget *_tileRandom;

	GUI::StaticTextWidget *_energyDesc;
	GUI::SliderWidget *_energySlider;
	GUI::StaticTextWidget *_energyLabel;
	GUI::CheckboxWidget *_energyRandom;

	GUI::StaticTextWidget *_terrainDesc;
	GUI::SliderWidget *_terrainSlider;
	GUI::StaticTextWidget *_terrainLabel;
	GUI::CheckboxWidget *_terrainRandom;

	GUI::StaticTextWidget *_waterDesc;
	GUI::SliderWidget *_waterSlider;
	GUI::StaticTextWidget *_waterLabel;
	GUI::CheckboxWidget *_waterRandom;

	GUI::ButtonWidget *_cancelButton;
	GUI::ButtonWidget *_generateButton;
};

} // End of namespace Scumm

#endif
