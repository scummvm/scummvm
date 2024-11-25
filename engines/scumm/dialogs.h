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

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "common/ustr.h"
#include "common/keyboard.h"
#include "gui/dialog.h"
#include "engines/dialogs.h"

namespace GUI {
class CommandSender;
class StaticTextWidget;
}


namespace Scumm {

struct ResString {
	int num;
	char string[80];
};

class ScummEngine;

class ScummDialog : public GUI::Dialog {
public:
	ScummDialog(int x, int y, int w, int h);
	ScummDialog(Common::String name);

protected:
	typedef Common::String String;
	typedef Common::U32String U32String;
};

#ifndef DISABLE_HELP
class ScummMenuDialog : public MainMenuDialog {
public:
	ScummMenuDialog(ScummEngine *scumm);
	~ScummMenuDialog() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

protected:
	GUI::Dialog		*_helpDialog;
};
#endif

/**
 * A dialog which displays an arbitrary message to the user and returns
 * the users reply as its result value. More specifically, it returns
 * the ASCII code of the key used to close the dialog (0 if a mouse
 * click closed the dialog).
 */
class InfoDialog : public ScummDialog {
protected:
	ScummEngine		*_vm;
	U32String _message;
	GUI::StaticTextWidget *_text;
	GUI::ThemeEngine::FontStyle _style;

public:
	// arbitrary message
	InfoDialog(ScummEngine *scumm, const U32String &message);
	// from resources
	InfoDialog(ScummEngine *scumm, int res);

	void setInfoText(const U32String &message);

	void handleMouseDown(int x, int y, int button, int clickCount) override {
		setResult(0);
		close();
	}
	void handleKeyDown(Common::KeyState state) override {
		setResult(state.ascii);
		close();
	}

	void reflowLayout() override;
	const char *getPlainEngineString(int stringno, bool forceHardcodedString = false);

protected:
	// Query a string from the resources
	const U32String queryResString(int stringno);
	// Query hard coded string (copied over from the executable)
	const ResString &getStaticResString(Common::Language lang, int stringno);
};

/**
 * The pause dialog, visible whenever the user activates pause mode. Goes
 * away uon any key or mouse button press.
 */
class PauseDialog : public InfoDialog {
public:
	PauseDialog(ScummEngine *scumm, int res);
	void handleKeyDown(Common::KeyState state) override;
};

/**
 * A simple yes/no dialog, used to ask the user whether to really
 * quit/restart ScummVM.
 */
class ConfirmDialog : public InfoDialog {
public:
	ConfirmDialog(ScummEngine *scumm, int res);
	void handleKeyDown(Common::KeyState state) override;

protected:
	char _yesKey, _noKey;
};

/**
 * A dialog used to display the music volume / text speed.
 * Automatically closes after a brief time passed.
 */
class ValueDisplayDialog : public GUI::Dialog {
public:
	ValueDisplayDialog(const Common::U32String &label, int minVal, int maxVal, int val, uint16 incKey, uint16 decKey);

	void open() override;
	void drawDialog(GUI::DrawLayer layerToDraw) override;
	void handleTickle() override;
	void handleMouseDown(int x, int y, int button, int clickCount) override {
		close();
	}
	void handleKeyDown(Common::KeyState state) override;

	void reflowLayout() override;

protected:
	enum {
		kDisplayDelay = 1500
	};
	Common::U32String _label;
	const int _min = 0, _max = 0;
	const uint16 _incKey = 0, _decKey = 0;
	int _percentBarWidth = 0;
	int _value = 0;
	uint32 _timer = 0;
};

/**
 * A dialog used to display and cycle subtitle settings.
 * Automatically closes after a brief time has passed.
 */
class SubtitleSettingsDialog : public InfoDialog {
public:
	SubtitleSettingsDialog(ScummEngine *scumm, int value);

	void open() override;
	void handleTickle() override;
	void handleMouseDown(int x, int y, int button, int clickCount) override {
		close();
	}
	void handleKeyDown(Common::KeyState state) override;
protected:
	int _value;
	uint32 _timer;

	void cycleValue();
};

//The Indy IQ dialog
class Indy3IQPointsDialog : public InfoDialog {
public:
	Indy3IQPointsDialog(ScummEngine *scumm, char* text);
	void handleKeyDown(Common::KeyState state) override;
};

class DebugInputDialog : public InfoDialog {
public:
	DebugInputDialog(ScummEngine *scumm, char* text);
	void handleKeyDown(Common::KeyState state) override;
	bool done;
	Common::String buffer;
	Common::String mainText;
};

/**
 * Difficulty selection dialog for Loom FM-Towns.
 */
class LoomTownsDifficultyDialog : public GUI::Dialog {
public:
	LoomTownsDifficultyDialog();

	int getSelectedDifficulty() const { return _difficulty; }
protected:
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	enum {
		kStandardCmd = 'STDD',
		kPracticeCmd = 'PRAD',
		kExpertCmd = 'EXPD'
	};

	int _difficulty;
};

/**
 * Common options widget stuff.
 */
class ScummOptionsContainerWidget : public GUI::OptionsContainerWidget {
public:
	ScummOptionsContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &dialogLayout, const Common::String &domain) :
		OptionsContainerWidget(boss, name, dialogLayout, domain) {
	}

	enum {
		kEnhancementGroup1Cmd = 'ENH1',
		kEnhancementGroup2Cmd = 'ENH2',
		kEnhancementGroup3Cmd = 'ENH3',
		kEnhancementGroup4Cmd = 'ENH4'
	};

	void load() override;
	bool save() override;

protected:
	void createEnhancementsWidget(GuiObject *boss, const Common::String &name);
	GUI::ThemeEval &addEnhancementsLayout(GUI::ThemeEval &layouts) const;
	GUI::CheckboxWidget *createOriginalGUICheckbox(GuiObject *boss, const Common::String &name);
	GUI::CheckboxWidget *createCopyProtectionCheckbox(GuiObject *boss, const Common::String &name);
	void updateAdjustmentSlider(GUI::SliderWidget *slider, GUI::StaticTextWidget *value);

	Common::Array<GUI::CheckboxWidget *> _enhancementsCheckboxes;

};

/**
 * Options widget for SCUMM games in general.
 */
class ScummGameOptionsWidget : public ScummOptionsContainerWidget {
public:
	ScummGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain, const ExtraGuiOptions &options);
	~ScummGameOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	enum {
		kSmoothScrollCmd = 'SMSC'
	};

	GUI::CheckboxWidget *_smoothScrollCheckbox;
	GUI::CheckboxWidget *_semiSmoothScrollCheckbox;

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	ExtraGuiOptions _options;
	Common::Array<GUI::CheckboxWidget *> _checkboxes;
};

/**
 * Options widget for EGA Loom.
 */
class LoomEgaGameOptionsWidget : public ScummOptionsContainerWidget {
public:
	LoomEgaGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~LoomEgaGameOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	enum {
		kOvertureTicksChanged = 'OTCH'
	};

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	GUI::CheckboxWidget *_enableOriginalGUICheckbox;
	GUI::CheckboxWidget *_enableCopyProtectionCheckbox;

	GUI::SliderWidget *_overtureTicksSlider;
	GUI::StaticTextWidget *_overtureTicksValue;

	void updateOvertureTicksValue();
};

/**
* Options widget for various Macintosh games.
*/
class MacGameOptionsWidget : public ScummOptionsContainerWidget {
public:
	MacGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain, int gameId, const Common::String &extra);
	~MacGameOptionsWidget() override {};

	void load() override;
	bool save() override;
private:
	enum {
		kQualitySliderUpdate = 'QUAL'
	};
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void updateQualitySlider();

	GUI::CheckboxWidget *_enableOriginalGUICheckbox;
	GUI::CheckboxWidget *_enableCopyProtectionCheckbox;
	GUI::SliderWidget *_sndQualitySlider;
	GUI::StaticTextWidget *_sndQualityValue;
	int _quality;
};

/**
 * Options widget for VGA Loom (DOS CD).
 */
class LoomVgaGameOptionsWidget : public ScummOptionsContainerWidget {
public:
	LoomVgaGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~LoomVgaGameOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	enum {
		kPlaybackAdjustmentChanged = 'PBAC'
	};

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	GUI::CheckboxWidget *_enableOriginalGUICheckbox;

	GUI::SliderWidget *_playbackAdjustmentSlider;
	GUI::StaticTextWidget *_playbackAdjustmentValue;

	void updatePlaybackAdjustmentValue();
};

/**
 * Options widget for CD Monkey Island 1.
 */
class MI1CdGameOptionsWidget : public ScummOptionsContainerWidget {
public:
	MI1CdGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~MI1CdGameOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	enum {
		kIntroAdjustmentChanged = 'IACH',
		kOutlookAdjustmentChanged = 'OACH'
	};

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	GUI::CheckboxWidget *_enableOriginalGUICheckbox;

	GUI::SliderWidget *_introAdjustmentSlider;
	GUI::StaticTextWidget *_introAdjustmentValue;
	GUI::SliderWidget *_outlookAdjustmentSlider;
	GUI::StaticTextWidget *_outlookAdjustmentValue;

	void updateIntroAdjustmentValue();
	void updateOutlookAdjustmentValue();
};

#ifdef USE_ENET
/**
 * Options widget for network supported HE games
 * (Football 1999/2002, Baseball 2001 and
 * Moonbase Commander).
 */
class HENetworkGameOptionsWidget : public ScummOptionsContainerWidget {
public:
	HENetworkGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain, const Common::String &&gameid);
	~HENetworkGameOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	enum {
		kEnableSessionCmd = 'ENBS',
		kResetServersCmd = 'CLRS',
	};

	Common::String _gameid;

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	GUI::CheckboxWidget *_audioOverride;

	GUI::CheckboxWidget *_enableSessionServer;

	GUI::EditTextWidget *_sessionServerAddr;
	GUI::ButtonWidget *_serverResetButton;

	GUI::CheckboxWidget *_enableLANBroadcast;

	GUI::CheckboxWidget *_generateRandomMaps;

	GUI::EditTextWidget *_lobbyServerAddr;

#ifdef USE_LIBCURL
	GUI::CheckboxWidget *_enableCompetitiveMods;
#endif

	GUI::StaticTextWidget *_networkVersion;
};
#endif

} // End of namespace Scumm

#endif
