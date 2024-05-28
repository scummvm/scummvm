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

#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"
#include "common/savefile.h"
#include "engines/advancedDetector.h"
#include "graphics/scaler.h"
#include "gui/gui-manager.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/popup.h"
#include "gui/ThemeEval.h"
#include "image/png.h"
#include "twp/twp.h"
#include "twp/achievements_tables.h"
#include "twp/detection.h"
#include "twp/metaengine.h"
#include "twp/detection.h"
#include "twp/savegame.h"
#include "twp/time.h"
#include "twp/actions.h"
#include "twp/debugtools.h"
#include "twp/dialogs.h"
#include "twp/twp.h"

#define MAX_SAVES 99

const char *TwpMetaEngine::getName() const {
	return "twp";
}

Common::Error TwpMetaEngine::createInstance(OSystem *syst, Engine **engine, const Twp::TwpGameDescription *desc) const {
	*engine = new Twp::TwpEngine(syst,(desc));
	return Common::kNoError;
}

bool TwpMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		   (f == kSupportsLoadingDuringStartup);
}

int TwpMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void TwpMetaEngine::registerDefaultSettings(const Common::String &) const {
	ConfMan.registerDefault("toiletPaperOver", false);
	ConfMan.registerDefault("annoyingInJokes", false);
	ConfMan.registerDefault("invertVerbHighlight", true);
	ConfMan.registerDefault("retroFonts", false);
	ConfMan.registerDefault("retroVerbs", false);
	ConfMan.registerDefault("hudSentence", false);
	ConfMan.registerDefault("ransomeUnbeeped", false);
	ConfMan.registerDefault("language", "en");
}

Common::AchievementsPlatform TwpMetaEngine::getAchievementsPlatform(const Common::String &target) const {
	return Common::STEAM_ACHIEVEMENTS;
}

const Common::AchievementDescriptionList *TwpMetaEngine::getAchievementDescriptionList() const {
	return Twp::achievementDescriptionList;
}

static Common::String getDesc(const Twp::SaveGame &savegame) {
	Common::String desc = Twp::formatTime(savegame.time, "%b %d at %H:%M");
	if (savegame.easyMode)
		desc += " (casual)";
	return desc;
}

SaveStateDescriptor TwpMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc = MetaEngine::querySaveMetaInfos(target, slot);
	if (desc.isValid())
		return desc;

	Common::String name(getSavegameFile(slot, target));
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(name);
	if (f) {
		Common::InSaveFile *thumbnailFile = g_system->getSavefileManager()->openForLoading(name + ".png");

		// Create the return descriptor
		desc = SaveStateDescriptor(this, slot, "?");

		if (thumbnailFile) {
			Image::PNGDecoder png;
			if (png.loadStream(*thumbnailFile)) {
				Graphics::ManagedSurface *thumbnail = new Graphics::ManagedSurface();
				thumbnail->copyFrom(*png.getSurface());
				Graphics::Surface *thumbnailSmall = new Graphics::Surface();
				createThumbnail(thumbnailSmall, thumbnail);
				desc.setThumbnail(thumbnailSmall);
			}
		}
		Twp::SaveGame savegame;
		Twp::SaveGameManager::getSaveGame(f, savegame);
		Common::String savegameDesc(getDesc(savegame));
		Twp::DateTime dt = Twp::toDateTime(savegame.time);
		desc.setDescription(savegameDesc);
		desc.setPlayTime(savegame.gameTime * 1000);
		desc.setSaveDate(dt.year, dt.month, dt.day);
		desc.setSaveTime(dt.hour, dt.min);

		return desc;
	}

	return SaveStateDescriptor();
}

GUI::OptionsContainerWidget *TwpMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	GUI::OptionsContainerWidget *widget = new Twp::TwpOptionsContainerWidget(boss, name, target);
	return widget;
}

void TwpMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	Twp::g_twp->capture(thumb, 160, 120);
}

Common::Array<Common::Keymap *> TwpMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *engineKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, target, "Thimbleweed Park keymap");

	struct {
		const char *name;
		const char *desc;
		Twp::TwpAction action;
		char inputs[32];
		Common::EventType event;
		Common::KeyCode key;
	} actions[] = {
		{"ACTION", _s("Action"), Twp::kDefaultAction, "MOUSE_LEFT|JOY_A|RETURN", Common::EVENT_LBUTTONDOWN, Common::KEYCODE_INVALID},
		{"SELECTHIGHLIGHTEDVERB", _s("Select highlighted verb"), Twp::kSelectHighlightedVerb, "MOUSE_RIGHT|JOY_X", Common::EVENT_RBUTTONDOWN, Common::KEYCODE_INVALID},
		{Common::kStandardActionMoveLeft, _s("Left"), Twp::kMoveLeft, "LEFT|JOY_LEFT_STICK_X-", Common::EVENT_INVALID, Common::KEYCODE_LEFT},
		{Common::kStandardActionMoveRight, _s("Right"), Twp::kMoveRight, "RIGHT|JOY_LEFT_STICK_X+", Common::EVENT_INVALID, Common::KEYCODE_RIGHT},
		{Common::kStandardActionMoveUp, _s("Up"), Twp::kMoveUp, "UP|JOY_LEFT_STICK_Y-", Common::EVENT_INVALID, Common::KEYCODE_UP},
		{Common::kStandardActionMoveDown, _s("Down"), Twp::kMoveDown, "DOWN|JOY_LEFT_STICK_Y+", Common::EVENT_INVALID, Common::KEYCODE_DOWN},
		{"GOTONPREVOBJECT", _s("Go to previous object"), Twp::kGotoPreviousObject, "JOY_LEFT_SHOULDER", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"GOTONEXTOBJECT", _s("Go to next object"), Twp::kGotoNextObject, "JOY_RIGHT_SHOULDER", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELVERBINVENOTRYLEFT", _s("Select verb/item Left"), Twp::kSelectVerbInventoryLeft, "JOY_LEFT", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELVERBINVENOTRYRIGHT", _s("Select verb/item Right"), Twp::kSelectVerbInventoryRight, "JOY_RIGHT", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELVERBINVENOTRYUP", _s("Select verb/item Up"), Twp::kSelectVerbInventoryUp, "JOY_UP", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELVERBINVENOTRYDOWN", _s("Select verb/item Down"), Twp::kSelectVerbInventoryDown, "JOY_DOWN", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SKIPCUTSCENE", _s("Skip cutscene"), Twp::kSkipCutscene, "ESCAPE|JOY_Y", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTACTOR1", _s("Select Actor 1"), Twp::kSelectActor1, "1", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTACTOR2", _s("Select Actor 2"), Twp::kSelectActor2, "2", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTACTOR3", _s("Select Actor 3"), Twp::kSelectActor3, "3", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTACTOR4", _s("Select Actor 4"), Twp::kSelectActor4, "4", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTACTOR5", _s("Select Actor 5"), Twp::kSelectActor5, "5", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTCHOICE1", _s("Select Choice 1"), Twp::kSelectChoice1, "1", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTCHOICE2", _s("Select Choice 2"), Twp::kSelectChoice2, "2", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTCHOICE3", _s("Select Choice 3"), Twp::kSelectChoice3, "3", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTCHOICE4", _s("Select Choice 4"), Twp::kSelectChoice4, "4", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTCHOICE5", _s("Select Choice 5"), Twp::kSelectChoice5, "5", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTCHOICE6", _s("Select Choice 6"), Twp::kSelectChoice6, "6", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTNEXTACTOR", _s("Select Next Actor"), Twp::kSelectNextActor, "0|JOY_RIGHT_TRIGGER", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SELECTPREVACTOR", _s("Select Previous Actor"), Twp::kSelectPreviousActor, "9|JOY_LEFT_TRIGGER", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SKIPTEXT", _s("Skip Text"), Twp::kSkipText, "MOUSE_MIDDLE|PERIOD|JOY_B", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"SHOWHOTSPOTS", _s("Show hotspots"), Twp::kShowHotspots, "TAB", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"OPEN", _s("Open"), Twp::kOpen, "q", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"CLOSE", _s("Close"), Twp::kClose, "a", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"GIVE", _s("Give"), Twp::kGive, "z", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"PICKUP", _s("Pick up"), Twp::kPickUp, "w", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"LOOKAT", _s("Look at"), Twp::kLookAt, "s", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"TALKTO", _s("Talk to"), Twp::kTalkTo, "x", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"PUSH", _s("Push"), Twp::kPush, "e", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"PULL", _s("Pull"), Twp::kPull, "d", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{"USE", _s("Use"), Twp::kUse, "c", Common::EVENT_INVALID, Common::KEYCODE_INVALID},
		{0, 0, Twp::kSkipCutscene, {}, Common::EVENT_INVALID, Common::KEYCODE_INVALID}};

	Common::Action *act;
	for (int i = 0; actions[i].name; i++) {
		act = new Common::Action(actions[i].name, _(actions[i].desc));
		act->setCustomEngineActionEvent(actions[i].action);
		char *strToken = strtok(actions[i].inputs, "|");
		while (strToken) {
			act->addDefaultInputMapping(strToken);
			strToken = strtok(nullptr, "|");
		}
		if (actions[i].event != Common::EVENT_INVALID) {
			act->setEvent(actions[i].event);
		}
		if (actions[i].key != Common::KEYCODE_INVALID) {
			act->setKeyEvent(actions[i].key);
		}
		engineKeyMap->addAction(act);
	}

	return Common::Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(TWP)
REGISTER_PLUGIN_DYNAMIC(TWP, PLUGIN_TYPE_ENGINE, TwpMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TWP, PLUGIN_TYPE_ENGINE, TwpMetaEngine);
#endif
