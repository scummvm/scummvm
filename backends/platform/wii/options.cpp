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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "common/config-manager.h"
#include "gui/dialog.h"
#include "gui/TabWidget.h"

#include "options.h"
#include "gfx.h"

WiiOptionsDialog::WiiOptionsDialog(const OSystem::GraphicsMode &gfxMode) :
	Dialog(180, 120, 304, 200) {

	_videoModePrefix = String("wii_video_") + gfxMode.name;

	new ButtonWidget(this, 56, 160, 108, 24, "Cancel", 'c');
	new ButtonWidget(this, 180, 160, 108, 24, "Ok", 'k');

	TabWidget *tab = new TabWidget(this, 0, 0, 304, 146);

	tab->addTab("Video");

	new StaticTextWidget(tab, 16, 16, 128, 16,
							"Current video mode:", Graphics::kTextAlignRight);
	new StaticTextWidget(tab, 160, 16, 128, 16,
							gfxMode.description, Graphics::kTextAlignLeft);

	new StaticTextWidget(tab, 16, 48, 128, 16,
							"Horizontal underscan:", Graphics::kTextAlignRight);
	_sliderUnderscanX = new SliderWidget(tab, 160, 47, 128, 18, 'x');
	_sliderUnderscanX->setMinValue(0);
	_sliderUnderscanX->setMaxValue(32);

	new StaticTextWidget(tab, 16, 80, 128, 16,
							"Vertical underscan:", Graphics::kTextAlignRight);
	_sliderUnderscanY = new SliderWidget(tab, 160, 79, 128, 18, 'y');
	_sliderUnderscanY->setMinValue(0);
	_sliderUnderscanY->setMaxValue(32);

	load();
}

WiiOptionsDialog::~WiiOptionsDialog() {
}

void WiiOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd,
										uint32 data) {
	switch (cmd) {
	case 'x':
	case 'y':
		gfx_set_underscan(_sliderUnderscanX->getValue(),
							_sliderUnderscanY->getValue());
		break;

	case 'k':
		save();
		close();
		break;

	case 'c':
		revert();
		close();
		break;

	default:
		Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

void WiiOptionsDialog::revert() {
	gfx_set_underscan(ConfMan.getInt(_videoModePrefix + "_underscan_x",
									Common::ConfigManager::kApplicationDomain),
						ConfMan.getInt(_videoModePrefix + "_underscan_y",
									Common::ConfigManager::kApplicationDomain));
}

void WiiOptionsDialog::load() {
	int x = ConfMan.getInt(_videoModePrefix + "_underscan_x",
							Common::ConfigManager::kApplicationDomain);
	int y = ConfMan.getInt(_videoModePrefix + "_underscan_y",
							Common::ConfigManager::kApplicationDomain);

	_sliderUnderscanX->setValue(x);
	_sliderUnderscanY->setValue(y);
}

void WiiOptionsDialog::save() {
	ConfMan.setInt(_videoModePrefix + "_underscan_x",
					_sliderUnderscanX->getValue(),
					Common::ConfigManager::kApplicationDomain);
	ConfMan.setInt(_videoModePrefix + "_underscan_y",
					_sliderUnderscanY->getValue(),
					Common::ConfigManager::kApplicationDomain);
	ConfMan.flushToDisk();
}

