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

#include "gui/cloudconnectionwizard.h"

#include "backends/cloud/cloudmanager.h"

#ifdef USE_SDL_NET
#include "backends/networking/sdl_net/localwebserver.h"
#endif // USE_SDL_NET

#include "common/formats/json.h"
#include "common/memstream.h"
#include "common/translation.h"

#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/scrollcontainer.h"

namespace GUI {

enum {
	kCloudConnectionWizardQuickModeButtonCmd = 'WQMb',
	kCloudConnectionWizardManualModeButtonCmd = 'WMMb',
	kCloudConnectionWizardBackButtonCmd = 'WSBb',
	kCloudConnectionWizardNextButtonCmd = 'WSNb',
	kCloudConnectionWizardRunServerButtonCmd = 'WRSb',
	kCloudConnectionWizardOpenUrlStorageCmd = 'WOUb',
	kCloudConnectionWizardReflowCmd = 'WRFb',
	kCloudConnectionWizardPasteCodeCmd = 'WPCb',
};

CloudConnectionWizard::CloudConnectionWizard() :
	Dialog("GlobalOptions_Cloud_ConnectionWizard"),	
	_currentStep(Step::NONE), _switchToSuccess(false), _switchToFailure(false),
	_connecting(false) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	_headlineLabel = new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.Headline", Common::U32String());
	_closeButton = new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.CancelButton", _("Cancel"), Common::U32String(), kCloseCmd);
	_prevStepButton = nullptr;
	_nextStepButton = nullptr;
	_label0 = nullptr;
	_label1 = nullptr;
	_label2 = nullptr;
	_label3 = nullptr;
	_button0 = nullptr;

	_container = nullptr;
	_quickModeButton = nullptr;
	_quickModeLabel = nullptr;
	_manualModeButton = nullptr;
	_codeBox = nullptr;

	showStep(Step::MODE_SELECT);

	_callback = new Common::Callback<CloudConnectionWizard, Networking::ErrorResponse>(this, &CloudConnectionWizard::storageConnectionCallback);
}

CloudConnectionWizard::~CloudConnectionWizard() {
#ifdef USE_SDL_NET
	LocalServer.setStorageConnectionCallback(nullptr);
#endif // USE_SDL_NET

	delete _callback;
}

void CloudConnectionWizard::showStep(Step newStep) {
	if (newStep == _currentStep)
		return;

	switch (_currentStep) {
	case Step::NONE:
		break;

	case Step::MODE_SELECT:
		hideStepModeSelect();
		break;

	case Step::QUICK_MODE_STEP_1:
		hideStepQuickMode1();
		break;

	case Step::QUICK_MODE_STEP_2:
		hideStepQuickMode2();
		break;

	case Step::QUICK_MODE_SUCCESS:
		hideStepQuickModeSuccess();
		break;

	case Step::MANUAL_MODE_STEP_1:
		hideStepManualMode1();
		break;

	case Step::MANUAL_MODE_STEP_2:
		hideStepManualMode2();
		break;

	case Step::MANUAL_MODE_FAILURE:
		hideStepManualModeFailure();
		break;

	case Step::MANUAL_MODE_SUCCESS:
		hideStepManualModeSuccess();
		break;
	}

	_currentStep = newStep;

	switch (_currentStep) {
	case Step::NONE:
		break;

	case Step::MODE_SELECT:
		showStepModeSelect();
		break;

	case Step::QUICK_MODE_STEP_1:
		showStepQuickMode1();
		break;

	case Step::QUICK_MODE_STEP_2:
		showStepQuickMode2();
		break;

	case Step::QUICK_MODE_SUCCESS:
		showStepQuickModeSuccess();
		break;

	case Step::MANUAL_MODE_STEP_1:
		showStepManualMode1();
		break;

	case Step::MANUAL_MODE_STEP_2:
		showStepManualMode2();
		break;

	case Step::MANUAL_MODE_FAILURE:
		showStepManualModeFailure();
		break;

	case Step::MANUAL_MODE_SUCCESS:
		showStepManualModeSuccess();
		break;
	}

	reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

// mode select

void CloudConnectionWizard::showStepModeSelect() {
	_headlineLabel->setLabel(_("Cloud Connection Wizard"));
	showContainer("ConnectionWizard_ModeSelect");

	_quickModeButton = new ButtonWidget(_container, "ConnectionWizard_ModeSelect.QuickModeButton", _("Quick mode"), Common::U32String(), kCloudConnectionWizardQuickModeButtonCmd);
	_manualModeButton = new ButtonWidget(_container, "ConnectionWizard_ModeSelect.ManualModeButton", _("Manual mode"), Common::U32String(), kCloudConnectionWizardManualModeButtonCmd);

#ifdef USE_SDL_NET
	_quickModeLabel = new StaticTextWidget(_container, "ConnectionWizard_ModeSelect.QuickModeHint", _("Will ask you to run Local Webserver"));
#else
	_quickModeLabel = new StaticTextWidget(_container, "ConnectionWizard_ModeSelect.QuickModeHint", _("Requires Local Webserver feature"), Common::U32String(), ThemeEngine::kFontStyleNormal);
	_quickModeLabel->setEnabled(false);
	_quickModeButton->setEnabled(false);
#endif // USE_SDL_NET
}

void CloudConnectionWizard::hideStepModeSelect() {
	hideContainer();
	removeWidgetChecked(_quickModeButton);
	removeWidgetChecked(_manualModeButton);
	removeWidgetChecked(_quickModeLabel);
}

// quick mode

void CloudConnectionWizard::showStepQuickMode1() {
	_headlineLabel->setLabel(_("Quick Mode: Step 1"));
	showContainer("ConnectionWizard_QuickModeStep1");
	showBackButton();
	showNextButton();

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep1.Line1", _("In this mode, Local Webserver must be running,"));
	_label1 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep1.Line2", _("so your browser could forward data to ScummVM"));

	_button0 = new ButtonWidget(_container, "ConnectionWizard_QuickModeStep1.RunServerButton", Common::U32String(), Common::U32String(), kCloudConnectionWizardRunServerButtonCmd);
	_label2 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep1.ServerInfoLabel", Common::U32String());

	refreshStepQuickMode1();
}

void CloudConnectionWizard::refreshStepQuickMode1(bool displayAsStopped) {
	bool serverIsRunning = false;
#ifdef USE_SDL_NET
	serverIsRunning = LocalServer.isRunning();
#endif // USE_SDL_NET
	if (displayAsStopped)
		serverIsRunning = false;

	if (_nextStepButton)
		_nextStepButton->setEnabled(serverIsRunning);

	if (_button0) {
		_button0->setLabel(serverIsRunning ? _("Stop server") : _("Run server"));
		_button0->setTooltip(serverIsRunning ? _("Stop local webserver") : _("Run local webserver"));
	}

	if (_label2) {
		Common::U32String address;
#ifdef USE_SDL_NET
		address = LocalServer.getAddress();
#endif // USE_SDL_NET
		_label2->setLabel(serverIsRunning ? address : _("Not running"));
	}
}

void CloudConnectionWizard::hideStepQuickMode1() {
	hideContainer();
	hideBackButton();
	hideNextButton();
	removeWidgetChecked(_label0);
	removeWidgetChecked(_label1);
	removeWidgetChecked(_button0);
	removeWidgetChecked(_label2);
}

void CloudConnectionWizard::showStepQuickMode2() {
	_headlineLabel->setLabel(_("Quick Mode: Step 2"));
	showContainer("ConnectionWizard_QuickModeStep2");
	showBackButton();

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep2.Line1", _("Now, open this link in your browser:"));
	_button0 = new ButtonWidget(_container, "ConnectionWizard_QuickModeStep2.OpenLinkButton", Common::U32String("https://cloud.scummvm.org/"), _("Open URL"), kCloudConnectionWizardOpenUrlStorageCmd);

	_label1 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep2.Line2", _("It will automatically pass the data to ScummVM,"));
	_label2 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep2.Line3", _("and warn you should there be any errors."));
	_label3 = new StaticTextWidget(_container, "ConnectionWizard_QuickModeStep2.Line4", Common::U32String(), Common::U32String(), ThemeEngine::kFontStyleNormal);

#ifdef USE_SDL_NET
	_label3->setLabel(_("Local Webserver address: ") + Common::U32String(LocalServer.getAddress()));
	_label3->setEnabled(false);
#endif // USE_SDL_NET
}

void CloudConnectionWizard::hideStepQuickMode2() {
	hideContainer();
	hideBackButton();
	removeWidgetChecked(_label0);
	removeWidgetChecked(_button0);
	removeWidgetChecked(_label1);
	removeWidgetChecked(_label2);
	removeWidgetChecked(_label3);
}

void CloudConnectionWizard::showStepQuickModeSuccess() {
	_headlineLabel->setLabel(_("Quick Mode: Success"));
	showContainer("ConnectionWizard_Success");
	_closeButton->setVisible(false);

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_Success.Line1", _("You cloud storage has been connected!"));
	_button0 = new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.FinishButton", _("Finish"), Common::U32String(), kCloseCmd);
}

void CloudConnectionWizard::hideStepQuickModeSuccess() {
	hideContainer();
	_closeButton->setVisible(true);
	removeWidgetChecked(_label0);
	removeWidgetChecked(_button0);
}

// manual mode

void CloudConnectionWizard::showStepManualMode1() {
	_headlineLabel->setLabel(_("Manual Mode: Step 1"));
	showContainer("ConnectionWizard_ManualModeStep1");
	showBackButton();
	showNextButton();

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_ManualModeStep1.Line1", _("Open this link in your browser:"));
	_button0 = new ButtonWidget(_container, "ConnectionWizard_ManualModeStep1.OpenLinkButton", Common::U32String("https://cloud.scummvm.org/"), _("Open URL"), kCloudConnectionWizardOpenUrlStorageCmd);

	_label1 = new StaticTextWidget(_container, "ConnectionWizard_ManualModeStep1.Line2", _("When it fails to pass JSON code to ScummVM,"));
	_label2 = new StaticTextWidget(_container, "ConnectionWizard_ManualModeStep1.Line3", _("find it on Troubleshooting section of the page,"));
	_label3 = new StaticTextWidget(_container, "ConnectionWizard_ManualModeStep1.Line4", _("and go to the next step here."));
}

void CloudConnectionWizard::hideStepManualMode1() {
	hideContainer();
	hideBackButton();
	hideNextButton();
	removeWidgetChecked(_label0);
	removeWidgetChecked(_button0);
	removeWidgetChecked(_label1);
	removeWidgetChecked(_label2);
	removeWidgetChecked(_label3);
}

void CloudConnectionWizard::showStepManualMode2() {
	_headlineLabel->setLabel(_("Manual Mode: Step 2"));
	showContainer("ConnectionWizard_ManualModeStep2");
	showBackButton();
	showNextButton();

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_ManualModeStep2.Line1", _("Copy JSON code from browser here and press Next:"));
	_codeBox = new EditTextWidget(_container, "ConnectionWizard_ManualModeStep2.CodeBox", Common::U32String(), Common::U32String(), 0, 0, ThemeEngine::kFontStyleConsole);
	_button0 = new ButtonWidget(_container, "ConnectionWizard_ManualModeStep2.PasteButton", _("Paste"), _("Paste code from clipboard"), kCloudConnectionWizardPasteCodeCmd);
	_label1 = new StaticTextWidget(_container, "ConnectionWizard_ManualModeStep2.Line2", Common::U32String());
}

void CloudConnectionWizard::hideStepManualMode2() {
	hideContainer();
	_closeButton->setEnabled(true);
	hideBackButton();
	hideNextButton();
	removeWidgetChecked(_label0);
	removeWidgetChecked(_codeBox);
	removeWidgetChecked(_button0);
	removeWidgetChecked(_label1);
}

void CloudConnectionWizard::showStepManualModeFailure() {
	_headlineLabel->setLabel(_("Manual Mode: Something went wrong"));
	showContainer("ConnectionWizard_Failure");
	showBackButton();

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_Failure.Line1", _("Cloud storage was not connected."));
	_label1 = new StaticTextWidget(_container, "ConnectionWizard_Failure.Line2", _("Make sure JSON code was copied correctly and retry."));
	_label2 = new StaticTextWidget(_container, "ConnectionWizard_Failure.Line3", _("It it doesn't work, try from the beginning."));
	_label3 = new StaticTextWidget(_container, "ConnectionWizard_Failure.Line4", _("Error message: ") + _errorMessage, Common::U32String(), ThemeEngine::kFontStyleNormal);
	_label3->setEnabled(false);
}

void CloudConnectionWizard::hideStepManualModeFailure() {
	hideContainer();
	hideBackButton();
	removeWidgetChecked(_label0);
	removeWidgetChecked(_label1);
	removeWidgetChecked(_label2);
	removeWidgetChecked(_label3);
}

void CloudConnectionWizard::showStepManualModeSuccess() {
	_headlineLabel->setLabel(_("Manual Mode: Success"));
	showContainer("ConnectionWizard_Success");
	_closeButton->setVisible(false);

	_label0 = new StaticTextWidget(_container, "ConnectionWizard_Success.Line1", _("Your cloud storage has been connected!"));
	_button0 = new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.FinishButton", _("Finish"), Common::U32String(), kCloseCmd);
}

void CloudConnectionWizard::hideStepManualModeSuccess() {
	hideContainer();
	_closeButton->setVisible(true);
	removeWidgetChecked(_label0);
	removeWidgetChecked(_button0);
}

// utils

void CloudConnectionWizard::showContainer(const Common::String &dialogName) {
	_container = new ScrollContainerWidget(this, "GlobalOptions_Cloud_ConnectionWizard.Container", dialogName, kCloudConnectionWizardReflowCmd);
	_container->setTarget(this);
	_container->setBackgroundType(ThemeEngine::kWidgetBackgroundNo);
}

void CloudConnectionWizard::hideContainer() {
	removeWidgetChecked(_container);
}

void CloudConnectionWizard::showBackButton() {
	_prevStepButton = new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.PrevButton", _("Back"), Common::U32String(), kCloudConnectionWizardBackButtonCmd);
}

void CloudConnectionWizard::hideBackButton() {
	removeWidgetChecked(_prevStepButton);
}

void CloudConnectionWizard::showNextButton() {
	_nextStepButton = new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.NextButton", _("Next"), Common::U32String(), kCloudConnectionWizardNextButtonCmd);
}

void CloudConnectionWizard::hideNextButton() {
	removeWidgetChecked(_nextStepButton);
}

void CloudConnectionWizard::removeWidgetChecked(ScrollContainerWidget *&widget) {
	if (widget) {
		removeWidget(widget);
		widget = nullptr;
	}
}

void CloudConnectionWizard::removeWidgetChecked(ButtonWidget *&widget) {
	if (widget) {
		removeWidget(widget);
		widget = nullptr;
	}
}

void CloudConnectionWizard::removeWidgetChecked(StaticTextWidget *&widget) {
	if (widget) {
		removeWidget(widget);
		widget = nullptr;
	}
}

void CloudConnectionWizard::removeWidgetChecked(EditTextWidget *&widget) {
	if (widget) {
		removeWidget(widget);
		widget = nullptr;
	}
}

// logic

void CloudConnectionWizard::storageConnectionCallback(Networking::ErrorResponse response) {
	if (response.failed || response.interrupted) {
		return;
	}

	_switchToSuccess = true;
}

void CloudConnectionWizard::manualModeConnect() {
	if (_connecting)
		return;

	if (_label1)
		_label1->setLabel(Common::U32String());

	// get the code entered
	Common::String code;
	if (_codeBox)
		code = _codeBox->getEditString().encode();
	if (code.size() == 0)
		return;

	// warn about other Storage working
	if (CloudMan.isWorking()) {
		bool cancel = true;

		MessageDialog alert(_("Another Storage is working now. Do you want to interrupt it?"), _("Yes"), _("No"));
		if (alert.runModal() == GUI::kMessageOK) {
			if (CloudMan.isDownloading())
				CloudMan.cancelDownload();
			if (CloudMan.isSyncing())
				CloudMan.cancelSync();

			// I believe it still would return `true` here, but just in case
			if (CloudMan.isWorking()) {
				MessageDialog alert2(_("Wait until current Storage finishes up and try again."));
				alert2.runModal();
			} else {
				cancel = false;
			}
		}

		if (cancel) {
			return;
		}
	}

	// parse JSON and display message if failed
	Common::MemoryWriteStreamDynamic jsonStream(DisposeAfterUse::YES);
	jsonStream.write(code.c_str(), code.size());
	char *contents = Common::JSON::untaintContents(jsonStream);
	Common::JSONValue *json = Common::JSON::parse(contents);

	// pass JSON to the manager
	_connecting = true;
	Networking::ErrorCallback callback = new Common::Callback<CloudConnectionWizard, Networking::ErrorResponse>(this, &CloudConnectionWizard::manualModeStorageConnectionCallback);
	Networking::JsonResponse jsonResponse(nullptr, json);
	if (!CloudMan.connectStorage(jsonResponse, callback)) { // no "storage" in JSON (or invalid one)
		_connecting = false;
		delete json;
		delete callback;
		if (_label1)
			// I18N: JSON is name of the format, this message is displayed if user entered something incorrect to the text field
			_label1->setLabel(_("JSON code contents is malformed."));
		return;
	}

	// disable UI	
	if (_codeBox)
		_codeBox->setEnabled(false);
	if (_button0)
		_button0->setEnabled(false);
	if (_closeButton)
		_closeButton->setEnabled(false);
	if (_prevStepButton)
		_prevStepButton->setEnabled(false);
	if (_nextStepButton)
		_nextStepButton->setEnabled(false);
}

void CloudConnectionWizard::manualModeStorageConnectionCallback(Networking::ErrorResponse response) {
	if (response.failed || response.interrupted) {
		if (response.failed) {
			const char *knownErrorMessages[] = {
				_s("OK"),
				_s("Incorrect JSON.") // see "cloud/basestorage.cpp"
			};
			(void)knownErrorMessages;

			_errorMessage = _(response.response.c_str());
		} else {
			// I18N: error message displayed on 'Manual Mode: Failure' step of 'Cloud Connection Wizard', describing that storage connection process was interrupted
			_errorMessage = _("Interrupted.");
		}

		_switchToFailure = true;
		return;
	}

	_switchToSuccess = true;
}

// public

void CloudConnectionWizard::open() {
	Dialog::open();
#ifdef USE_SDL_NET
	LocalServer.setStorageConnectionCallback(_callback);
#endif // USE_SDL_NET
}

void CloudConnectionWizard::close() {
#ifdef USE_SDL_NET
	LocalServer.setStorageConnectionCallback(nullptr);
#endif // USE_SDL_NET
	Dialog::close();
}

void CloudConnectionWizard::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {	
	switch (cmd) {
	case kCloudConnectionWizardQuickModeButtonCmd:
		showStep(Step::QUICK_MODE_STEP_1);
		break;

	case kCloudConnectionWizardManualModeButtonCmd:
		showStep(Step::MANUAL_MODE_STEP_1);
		break;

	case kCloudConnectionWizardRunServerButtonCmd:
#ifdef USE_SDL_NET
		if (LocalServer.isRunning()) {
			LocalServer.stopOnIdle();
			refreshStepQuickMode1(true);
		} else {
			LocalServer.start();
			refreshStepQuickMode1();
		}
#endif // USE_SDL_NET
		break;

	case kCloudConnectionWizardOpenUrlStorageCmd:
		if (!g_system->openUrl("https://cloud.scummvm.org/")) {
			MessageDialog alert(_("Failed to open URL!\nPlease navigate to this page manually."));
			alert.runModal();
		}
		break;

	case kCloudConnectionWizardPasteCodeCmd:
		if (g_system->hasTextInClipboard()) {
			Common::U32String message = g_system->getTextFromClipboard();
			if (!message.empty()) {
				_codeBox->setEditString(message);
			}
		}
		break;

	case kCloudConnectionWizardNextButtonCmd:
		switch (_currentStep) {
		case Step::QUICK_MODE_STEP_1:
			showStep(Step::QUICK_MODE_STEP_2);
			break;

		case Step::MANUAL_MODE_STEP_1:
			showStep(Step::MANUAL_MODE_STEP_2);
			break;

		case Step::MANUAL_MODE_STEP_2:
			manualModeConnect();
			break;

		default:
			break;
		}
		break;

	case kCloudConnectionWizardBackButtonCmd:
		switch (_currentStep) {
		case Step::QUICK_MODE_STEP_1:
		case Step::MANUAL_MODE_STEP_1:
			showStep(Step::MODE_SELECT);
			break;

		case Step::QUICK_MODE_STEP_2:
			showStep(Step::QUICK_MODE_STEP_1);
			break;

		case Step::MANUAL_MODE_STEP_2:
			showStep(Step::MANUAL_MODE_STEP_1);
			break;

		case Step::MANUAL_MODE_FAILURE:
			showStep(Step::MANUAL_MODE_STEP_2);
			break;

		default:
			break;
		}
		break;

	default:	
		Dialog::handleCommand(sender, cmd, data);
	}
}

void CloudConnectionWizard::handleTickle() {
	if (_connecting && _currentStep == Step::MANUAL_MODE_STEP_2) {
		bool switched = false;

		if (_switchToFailure) {
			showStep(Step::MANUAL_MODE_FAILURE);
			switched = true;
		} else if (_switchToSuccess) {
			showStep(Step::MANUAL_MODE_SUCCESS);
			switched = true;
		}

		if (switched) {
			_switchToFailure = false;
			_switchToSuccess = false;
			_connecting = false;
		}
	}

	if (_switchToSuccess) {
		showStep(Step::QUICK_MODE_SUCCESS);
		_switchToSuccess = false;
	}

	Dialog::handleTickle();
}

} // End of namespace GUI
