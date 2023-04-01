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

#ifndef GUI_CLOUDCONNECTIONWIZARD_H
#define GUI_CLOUDCONNECTIONWIZARD_H

#include "backends/networking/curl/request.h"
#include "common/str.h"
#include "common/ustr.h"
#include "gui/dialog.h"

namespace GUI {
class ScrollContainerWidget;
class StaticTextWidget;
class ButtonWidget;
class EditTextWidget;

class CloudConnectionWizard : public Dialog {
	enum class Step {
		NONE,
		MODE_SELECT,
		QUICK_MODE_STEP_1,
		QUICK_MODE_STEP_2,
		QUICK_MODE_SUCCESS,
		MANUAL_MODE_STEP_1,
		MANUAL_MODE_STEP_2,
		MANUAL_MODE_FAILURE,
		MANUAL_MODE_SUCCESS
	};

	// wizard flow
	Step _currentStep;
	bool _switchToSuccess;
	bool _switchToFailure;

	// state
	Networking::ErrorCallback _callback;
	bool _connecting;
	Common::U32String _errorMessage;

	// common and generic widgets
	StaticTextWidget *_headlineLabel;
	ButtonWidget *_closeButton;
	ButtonWidget *_prevStepButton;
	ButtonWidget *_nextStepButton;
	StaticTextWidget *_label0;
	StaticTextWidget *_label1;
	StaticTextWidget *_label2;
	StaticTextWidget *_label3;
	ButtonWidget *_button0;

	// specific widgets
	ScrollContainerWidget *_container;
	ButtonWidget *_quickModeButton;
	StaticTextWidget *_quickModeLabel;
	ButtonWidget *_manualModeButton;
	EditTextWidget *_codeBox;

	// wizard flow
	void showStep(Step newStep);

	void showStepModeSelect();
	void hideStepModeSelect();

	void showStepQuickMode1();
	void refreshStepQuickMode1(bool displayAsStopped = false);
	void hideStepQuickMode1();

	void showStepQuickMode2();
	void hideStepQuickMode2();

	void showStepQuickModeSuccess();
	void hideStepQuickModeSuccess();

	void showStepManualMode1();
	void hideStepManualMode1();

	void showStepManualMode2();
	void hideStepManualMode2();

	void showStepManualModeFailure();
	void hideStepManualModeFailure();

	void showStepManualModeSuccess();
	void hideStepManualModeSuccess();

	// widgets utils
	void showContainer(const Common::String &dialogName);
	void hideContainer();

	void showBackButton();
	void hideBackButton();

	void showNextButton();
	void hideNextButton();

	void removeWidgetChecked(ScrollContainerWidget *&widget);
	void removeWidgetChecked(ButtonWidget *&widget);
	void removeWidgetChecked(StaticTextWidget *&widget);
	void removeWidgetChecked(EditTextWidget *&widget);

	// logic
	void storageConnectionCallback(Networking::ErrorResponse response);
	void manualModeConnect();
	void manualModeStorageConnectionCallback(Networking::ErrorResponse response);

public:
	CloudConnectionWizard();
	~CloudConnectionWizard() override;

	void open() override;
	void close() override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleTickle() override;
};

} // End of namespace GUI

#endif
