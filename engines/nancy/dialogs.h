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

#include "engines/dialogs.h"

#ifndef NANCY_DIALOGS_H
#define NANCY_DIALOGS_H

namespace Nancy {

class NancyOptionsWidget : public GUI::OptionsContainerWidget {
public:
	NancyOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~NancyOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	bool isInGame() const;

	GUI::CheckboxWidget *_playerSpeechCheckbox;
	GUI::CheckboxWidget *_characterSpeechCheckbox;
	GUI::CheckboxWidget *_originalMenusCheckbox;
	GUI::CheckboxWidget *_secondChanceCheckbox;
};

class CheatDialog : public GUI::Dialog {
public:
	CheatDialog();

protected:
	enum Commands {
		kInputSceneNr = 'isnr',
		kInputFrameNr = 'ifnr',
		kInputScroll = 'iscr',
		kInputHintsEasy = 'ihea',
		kInputHintsMedium = 'ihme',
		kInputHintsHard = 'ihha',
		kInputPlayerTime = 'plti',
		kInputDifficulty = 'diff',
		kInputTimer = 'time'
	};

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	static void sanitizeInput(GUI::EditTextWidget *textWidget, int maxValue = -1);

	GUI::CheckboxWidget *_restartScene;
	GUI::EditTextWidget *_scene;
	GUI::EditTextWidget *_frame;
	GUI::EditTextWidget *_offset;

	GUI::EditTextWidget *_hintsRemainingEasy;
	GUI::EditTextWidget *_hintsRemainingMedium;
	GUI::EditTextWidget *_hintsRemainingHard;

	GUI::EditTextWidget *_playerTimeDays;
	GUI::EditTextWidget *_playerTimeHours;
	GUI::EditTextWidget *_playerTimeMinutes;
	GUI::EditTextWidget *_difficulty;

	GUI::CheckboxWidget *_timerOn;
	GUI::EditTextWidget *_timerHours;
	GUI::EditTextWidget *_timerMinutes;
	GUI::EditTextWidget *_timerSeconds;

	Common::Array<GUI::CheckboxWidget *> _inventory;
};

class EventFlagDialog : public GUI::Dialog {
public:
	EventFlagDialog();

protected:
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
};

} // End of namespace Nancy

#endif // NANCY_DIALOGS_H
