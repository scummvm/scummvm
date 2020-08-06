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

#include "dsmain.h"
#include "dsoptions.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/widgets/list.h"
#include "gui/widgets/tab.h"
#include "osystem_ds.h"
#include "engines/scumm/scumm.h"
#include "gui/widgets/popup.h"

#include "common/translation.h"

#define ALLOW_CPU_SCALER

namespace DS {

static bool confGetBool(Common::String key, bool defaultVal) {
	if (ConfMan.hasKey(key, "ds"))
		return ConfMan.getBool(key, "ds");
	return defaultVal;
}

static int confGetInt(Common::String key, int defaultVal) {
	if (ConfMan.hasKey(key, "ds"))
		return ConfMan.getInt(key, "ds");
	return defaultVal;
}



DSOptionsDialog::DSOptionsDialog() : GUI::Dialog(0, 0, 320 - 10, 230 - 40) {

	new GUI::ButtonWidget(this, 10, 170, 72, 16, _("~C~lose"), U32String(), GUI::kCloseCmd);
	new GUI::ButtonWidget(this, 320 - 10 - 130, 170, 120, 16, _("ScummVM Main Menu"), U32String(), 0x40000000, 'M');

	_tab = new GUI::TabWidget(this, 10, 5, 300, 230 - 20 - 40 - 20);

	_tab->addTab(_("Graphics"), "");

	_showCursorCheckbox = new GUI::CheckboxWidget(_tab, 150, 5, 130, 20, _("Show mouse cursor"), U32String(), 0, 'T');

	new GUI::StaticTextWidget(_tab, 5, 67, 180, 15, _("Initial top screen scale:"), Graphics::kTextAlignLeft);

	_100PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 82, 80, 20, U32String("100%"), U32String("TODO: Add tooltip"), 0x30000001, 'T');
	_150PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 97, 80, 20, U32String("150%"), U32String("TODO: Add tooltip"), 0x30000002, 'T');
	_200PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 112, 80, 20, U32String("200%"), U32String("TODO: Add tooltip"), 0x30000003, 'T');

	new GUI::StaticTextWidget(_tab, 5, 5, 180, 15, _("Main screen scaling:"), Graphics::kTextAlignLeft);

	_hardScaler = new GUI::CheckboxWidget(_tab, 5, 20, 270, 20, _("Hardware scale (fast, but low quality)"), U32String(), 0x10000001, 'T');
	_cpuScaler = new GUI::CheckboxWidget(_tab, 5, 35, 270, 20, _("Software scale (good quality, but slower)"), U32String(), 0x10000002, 'S');
	_unscaledCheckbox = new GUI::CheckboxWidget(_tab, 5, 50, 270, 20, _("Unscaled (you must scroll left and right)"), U32String(), 0x10000003, 'S');

	new GUI::StaticTextWidget(_tab, 5, 125, 110, 15, _("Brightness:"), Graphics::kTextAlignLeft);
	_gammaCorrection = new GUI::SliderWidget(_tab, 130, 120, 130, 12, U32String("TODO: Add tooltip"), 1);
	_gammaCorrection->setMinValue(0);
	_gammaCorrection->setMaxValue(8);
	_gammaCorrection->setValue(0);


	_tab->setActiveTab(0);

	_radioButtonMode = false;

	_showCursorCheckbox->setState(confGetBool("showcursor", true));
	_unscaledCheckbox->setState(confGetBool("unscaled", false));


	if (ConfMan.hasKey("topscreenzoom", "ds")) {

		_100PercentCheckbox->setState(false);
		_150PercentCheckbox->setState(false);
		_200PercentCheckbox->setState(false);

		switch (ConfMan.getInt("topscreenzoom", "ds")) {
			case 100: {
				_100PercentCheckbox->setState(true);
				break;
			}

			case 150: {
				_150PercentCheckbox->setState(true);
				break;
			}

			case 200: {
				_200PercentCheckbox->setState(true);
				break;
			}
		}

	} else if (ConfMan.hasKey("twohundredpercent", "ds")) {
		_200PercentCheckbox->setState(ConfMan.getBool("twohundredpercent", "ds"));
	} else {
		// No setting
		_150PercentCheckbox->setState(true);
	}

	if (ConfMan.hasKey("gamma", "ds")) {
		_gammaCorrection->setValue(ConfMan.getInt("gamma", "ds"));
	} else {
		_gammaCorrection->setValue(0);
	}

    #ifdef ALLOW_CPU_SCALER
	_cpuScaler->setState(confGetBool("cpu_scaler", false));
    #endif

	if (!_cpuScaler->getState() && !_unscaledCheckbox->getState()) {
		_hardScaler->setState(true);
	}

	_radioButtonMode = true;
}

DSOptionsDialog::~DSOptionsDialog() {
	ConfMan.flushToDisk();
}

void DSOptionsDialog::updateConfigManager() {
	ConfMan.setBool("unscaled", _unscaledCheckbox->getState(), "ds");
#ifdef ALLOW_CPU_SCALER
	ConfMan.setBool("cpu_scaler", _cpuScaler->getState(), "ds");
#endif
	ConfMan.setBool("showcursor", _showCursorCheckbox->getState(), "ds");
	ConfMan.setInt("gamma", _gammaCorrection->getValue(), "ds");

	int zoomLevel = 150;

	if (_100PercentCheckbox->getState()) {
		zoomLevel = 100;
	} else if (_150PercentCheckbox->getState()) {
		zoomLevel = 150;
	} else if (_200PercentCheckbox->getState()) {
		zoomLevel = 200;
	}

	printf("Saved zoom: %d\n", zoomLevel);

	ConfMan.setInt("topscreenzoom", zoomLevel, "ds");

	DS::setOptions();
}

void DSOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	static bool guard = false;

	if ((!guard) && (_radioButtonMode)) {
		guard = true;

		if ((cmd & 0xFF000000) == 0x10000000) {
			_cpuScaler->setState(false);
			_hardScaler->setState(false);
			_unscaledCheckbox->setState(false);

			if ((sender == _cpuScaler) && (cmd == 0x10000002)) {
				_cpuScaler->setState(true);
			}

			if ((sender == _hardScaler) && (cmd == 0x10000001)) {
				_hardScaler->setState(true);
			}

			if ((sender == _unscaledCheckbox) && (cmd == 0x10000003)) {
				_unscaledCheckbox->setState(true);
			}
		}

		guard = false;

	}


	if ((!guard) && (_radioButtonMode)) {

		guard = true;

		if (cmd == 0x30000001) {
			_100PercentCheckbox->setState(true);
			_150PercentCheckbox->setState(false);
			_200PercentCheckbox->setState(false);
			DS::setTopScreenZoom(100);
		}

		if (cmd == 0x30000002) {
			_100PercentCheckbox->setState(false);
			_150PercentCheckbox->setState(true);
			_200PercentCheckbox->setState(false);
			DS::setTopScreenZoom(150);
		}

		if (cmd == 0x30000003) {
			_100PercentCheckbox->setState(false);
			_150PercentCheckbox->setState(false);
			_200PercentCheckbox->setState(true);
			DS::setTopScreenZoom(200);
		}

		guard = false;

	}


	if (cmd == GUI::kCloseCmd) {
		updateConfigManager();
		close();
	}


	if ((!guard) && (cmd == 0x40000000)) {
		close();
		g_engine->openMainMenuDialog();
	}
}


void showOptionsDialog() {


	DS::displayMode16Bit();


	DSOptionsDialog *d = new DSOptionsDialog();
	d->runModal();
	delete d;

	DS::displayMode8Bit();

}

void setOptions() {
	static bool firstLoad = true;

	ConfMan.addGameDomain("ds");

	DS::setMouseCursorVisible(confGetBool("showcursor", true));

	DS::setUnscaledMode(confGetBool("unscaled", false));

	if (firstLoad) {
		if (ConfMan.hasKey("topscreenzoom", "ds")) {
			DS::setTopScreenZoom(ConfMan.getInt("topscreenzoom", "ds"));
		} else {
			if (ConfMan.hasKey("twohundredpercent", "ds")) {
				DS::setTopScreenZoom(200);
			} else {
				DS::setTopScreenZoom(150);
			}
		}
	}

#ifdef ALLOW_CPU_SCALER
	DS::setCpuScalerEnable(confGetBool("cpu_scaler", false));
#endif

	DS::setGamma(confGetInt("gamma", 0));

	firstLoad = false;
}

} // End of namespace DS
