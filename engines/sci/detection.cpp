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

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/translation.h"

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "sci/detection.h"
#include "sci/graphics/helpers_detection_enums.h"

namespace Sci {

// Titles of the games
static const PlainGameDescriptor s_sciGameTitles[] = {
	{"sci",             "Sierra SCI Game"},
	{"sci-fanmade",     "Fanmade SCI Game"},
	// === SCI0 games =========================================================
	{"astrochicken",    "Astro Chicken"},
	{"christmas1988",   "Christmas Card 1988"},
	{"iceman",          "Codename: Iceman"},
	{"camelot",         "Conquests of Camelot: King Arthur, Quest for the Grail"},
	{"funseeker",       "Fun Seeker's Guide"},
	{"hoyle1",          "Hoyle Official Book of Games: Volume 1"},
	{"hoyle2",          "Hoyle Official Book of Games: Volume 2"},
	{"kq4sci",          "King's Quest IV: The Perils of Rosella"},	// Note: There was also an AGI version of this
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI11 VGA remake of this (further down)
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	// === SCI01 games ========================================================
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"kq1sci",          "King's Quest I: Quest for the Crown"},	// Note: There was also an AGI version of this
	// === SCI1 games =========================================================
	{"castlebrain",     "Castle of Dr. Brain"},
	{"christmas1990",   "Christmas Card 1990: The Seasoned Professional"},
	{"cnick-lsl",       "Crazy Nick's Software Picks: Leisure Suit Larry's Casino"},
	{"cnick-kq",        "Crazy Nick's Software Picks: King Graham's Board Game Challenge"},
	{"cnick-laurabow",  "Crazy Nick's Software Picks: Parlor Games with Laura Bow"},
	{"cnick-longbow",   "Crazy Nick's Software Picks: Robin Hood's Game of Skill and Chance"},
	{"cnick-sq",        "Crazy Nick's Software Picks: Roger Wilco's Spaced Out Game Pack"},
	{"ecoquest",        "EcoQuest: The Search for Cetus"},	// floppy is SCI1, CD SCI1.1
	{"fairytales",      "Mixed-up Fairy Tales"},
	{"hoyle3",          "Hoyle Official Book of Games: Volume 3"},
	{"jones",           "Jones in the Fast Lane"},
	{"kq5",             "King's Quest V: Absence Makes the Heart Go Yonder"},
	{"longbow",         "Conquests of the Longbow: The Adventures of Robin Hood"},
	{"lsl1sci",         "Leisure Suit Larry in the Land of the Lounge Lizards"},	// Note: There was also an AGI version of this
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"mothergoose256",  "Mixed-Up Mother Goose"},
	{"msastrochicken",  "Ms. Astro Chicken"},
	{"pq1sci",          "Police Quest: In Pursuit of the Death Angel"},	// Note: There was also an AGI version of this
	{"pq3",             "Police Quest III: The Kindred"},
	{"sq1sci",          "Space Quest I: The Sarien Encounter"},	// Note: There was also an AGI version of this
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},	// floppy is SCI1, CD SCI1.1
	// === SCI1.1 games =======================================================
	{"christmas1992",   "Christmas Card 1992"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"inndemo",         "ImagiNation Network (INN) Demo"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"qfg1vga",         "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI0 version of this (further up)
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"pepper",          "Pepper's Adventure in Time"},
	{"slater",          "Slater & Charlie Go Camping"},
	{"gk1demo",         "Gabriel Knight: Sins of the Fathers"},
	{"qfg4demo",        "Quest for Glory IV: Shadows of Darkness"},
	{"pq4demo",         "Police Quest IV: Open Season"},
	// === SCI1.1+ games ======================================================
	{"catdate",         "The Dating Pool"},
	// === SCI2 games =========================================================
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},
	{"pq4",             "Police Quest IV: Open Season"}, // floppy is SCI2, CD SCI2.1
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},	// floppy is SCI2, CD SCI2.1
	// === SCI2.1 games ========================================================
	{"hoyle5",          "Hoyle Classic Games"},
	{"hoyle5bridge",    "Hoyle Bridge"},
	{"hoyle5children",  "Hoyle Children's Collection"},
	{"hoyle5solitaire", "Hoyle Solitaire"},
	{"chest",           "Inside the Chest"},	// aka Behind the Developer's Shield
	{"gk2",             "The Beast Within: A Gabriel Knight Mystery"},
	{"kq7",             "King's Quest VII: The Princeless Bride"},
	{"kquestions",      "King's Questions"},
	{"lsl6hires",       "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"mothergoosehires","Mixed-Up Mother Goose Deluxe"},
	{"phantasmagoria",  "Phantasmagoria"},
	{"pqswat",          "Police Quest: SWAT"},
	{"shivers",         "Shivers"},
	{"sq6",             "Space Quest 6: The Spinal Frontier"},
	{"torin",           "Torin's Passage"},
	// === SCI3 games =========================================================
	{"lsl7",            "Leisure Suit Larry 7: Love for Sail!"},
	{"lighthouse",      "Lighthouse: The Dark Being"},
	{"phantasmagoria2", "Phantasmagoria 2: A Puzzle of Flesh"},
	//{"shivers2",        "Shivers II: Harvest of Souls"},	// Not SCI
	{"rama",            "RAMA"},
	{0, 0}
};

} // End of namespace Sci

#include "sci/detection_tables.h"

namespace Sci {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_EGA_UNDITHER,
		{
			_s("Skip EGA dithering pass (full color backgrounds)"),
			_s("Skip dithering pass in EGA games, graphics are shown with full colors"),
			"disable_dithering",
			false
		}
	},

	{
		GAMEOPTION_HIGH_RESOLUTION_GRAPHICS,
		{
			_s("Enable high resolution graphics"),
			_s("Enable high resolution graphics/content"),
			"enable_high_resolution_graphics",
			true
		}
	},

	{
		GAMEOPTION_ENABLE_BLACK_LINED_VIDEO,
		{
			_s("Enable black-lined video"),
			_s("Draw black lines over videos to increase their apparent sharpness"),
			"enable_black_lined_video",
			false
		}
	},

#ifdef USE_RGB_COLOR
	{
		GAMEOPTION_HQ_VIDEO,
		{
			_s("Use high-quality video scaling"),
			_s("Use linear interpolation when upscaling videos, where possible"),
			"enable_hq_video",
			true
		}
	},
#endif

	{
		GAMEOPTION_LARRYSCALE,
		{
			_s("Use high-quality \"LarryScale\" cel scaling"),
			_s("Use special cartoon scaler for drawing character sprites"),
			"enable_larryscale",
			true
		}
	},

	{
		GAMEOPTION_PREFER_DIGITAL_SFX,
		{
			_s("Prefer digital sound effects"),
			_s("Prefer digital sound effects instead of synthesized ones"),
			"prefer_digitalsfx",
			true
		}
	},

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},
	// Jones in the Fast Lane - CD audio tracks or resource.snd
	{
		GAMEOPTION_JONES_CDAUDIO,
		{
			_s("Use CD audio"),
			_s("Use CD audio instead of in-game audio, if available"),
			"use_cdaudio",
			true
		}
	},

	// KQ6 Windows - windows cursors
	{
		GAMEOPTION_KQ6_WINDOWS_CURSORS,
		{
			_s("Use Windows cursors"),
			_s("Use the Windows cursors (smaller and monochrome) instead of the DOS ones"),
			"windows_cursors",
			false
		}
	},

	// SQ4 CD - silver cursors
	{
		GAMEOPTION_SQ4_SILVER_CURSORS,
		{
			_s("Use silver cursors"),
			_s("Use the alternate set of silver cursors instead of the normal golden ones"),
			"silver_cursors",
			false
		}
	},

	// Phantasmagoria 2 - content censoring option
	{
		GAMEOPTION_ENABLE_CENSORING,
		{
			_s("Enable content censoring"),
			_s("Enable the game's built-in optional content censoring"),
			"enable_censoring",
			false
		}
	},

	// KQ7 - Upscale videos to double their size (The in-game "Full screen" video setting)
	{
		GAMEOPTION_UPSCALE_VIDEOS,
		{
			_s("Upscale videos"),
			_s("Upscale videos to double their size"),
			"enable_video_upscale",
			true
		}
	},
	
	// SCI16 games: use RGB renderer instead of indexed
	{
		GAMEOPTION_RGB_RENDERING,
		{
			_s("Use RGB rendering"),
			_s("Use RGB rendering to improve screen transitions"),
			"rgb_rendering",
			false
		}
	},
	
	// SCI16 games: use custom per-resource palettes to improve visuals
	{
		GAMEOPTION_PALETTE_MODS,
		{
			_s("Use per-resource modified palettes"),
			_s("Use custom per-resource palettes to improve visuals"),
			"palette_mods",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

struct PopUpOptionsItem {
	const char *label;
	int configValue;
};

#define POPUP_OPTIONS_ITEMS_TERMINATOR { nullptr, 0 }

struct PopUpOptionsMap {
	const char *guioFlag;
	const char *label;
	const char *tooltip;
	const char *configOption;
	int defaultState;
	PopUpOptionsItem items[10];
};

#define POPUP_OPTIONS_TERMINATOR { nullptr, nullptr, nullptr, nullptr, 0, { POPUP_OPTIONS_ITEMS_TERMINATOR } }

static const PopUpOptionsMap popUpOptionsList[] = {
	{
		GAMEOPTION_MIDI_MODE,
		_s("MIDI mode:"),
		_s("When using external MIDI devices (e.g. through USB-MIDI), select your device here"),
		"midi_mode",
		kMidiModeStandard,
		{
			{
				_s("Standard (GM / MT-32)"),
				kMidiModeStandard
			},
			{
				_s("Roland D-110 / D-10 / D-20"),
				kMidiModeD110
			},
			{
				_s("Yamaha FB-01"),
				kMidiModeFB01
			},
			POPUP_OPTIONS_ITEMS_TERMINATOR
		}
	},
	POPUP_OPTIONS_TERMINATOR
};

static const char *directoryGlobs[] = {
	"avi",
	"english",
	"french",
	"german",
	"italian",
	"msg",
	"spanish",
	"patches",
	0
};

class OptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	Common::String _guiOptions;
	Common::HashMap<Common::String, GUI::CheckboxWidget *> _checkboxes;
	Common::HashMap<Common::String, GUI::PopUpWidget *> _popUps;
};

OptionsWidget::OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "SciOptionsDialog", false, domain) {
	_guiOptions = ConfMan.get("guioptions", domain);

	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			_checkboxes[entry->option.configOption] = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + "." + entry->option.configOption, _(entry->option.label), _(entry->option.tooltip));

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions)) {
			GUI::StaticTextWidget *textWidget = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + "." + entry->configOption + "_desc", _(entry->label), _(entry->tooltip));
			textWidget->setAlign(Graphics::kTextAlignRight);

			_popUps[entry->configOption] = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + "." + entry->configOption);

			for (uint i = 0; entry->items[i].label; ++i)
				_popUps[entry->configOption]->appendEntry(_(entry->items[i].label), entry->items[i].configValue);
		}
}

void OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(16, 16, 16, 16);

	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		layouts.addWidget(entry->option.configOption, "Checkbox");

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry) {
		layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal).addPadding(0, 0, 0, 0);
		layouts.addWidget(Common::String(entry->configOption) + "_desc", "OptionsLabel");
		layouts.addWidget(entry->configOption, "PopUp").closeLayout();
	}

	layouts.closeLayout().closeDialog();
}

void OptionsWidget::load() {
	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			_checkboxes[entry->option.configOption]->setState(ConfMan.getBool(entry->option.configOption, _domain));

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			_popUps[entry->configOption]->setSelectedTag(ConfMan.getInt(entry->configOption, _domain));

	// If the deprecated native_fb01 option is set, use it to set midi_mode
	if (ConfMan.hasKey("native_fb01", _domain) && ConfMan.getBool("native_fb01", _domain))
		_popUps["midi_mode"]->setSelectedTag(kMidiModeFB01);
}

bool OptionsWidget::save() {
	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			ConfMan.setBool(entry->option.configOption, _checkboxes[entry->option.configOption]->getState(), _domain);

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			ConfMan.setInt(entry->configOption, _popUps[entry->configOption]->getSelectedTag(), _domain);

	// Remove deprecated option
	ConfMan.removeKey("native_fb01", _domain);

	return true;
}

class SciMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SciMetaEngineDetection() : AdvancedMetaEngineDetection(Sci::SciGameDescriptions, sizeof(ADGameDescription), s_sciGameTitles, optionsList) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
		_matchFullPaths = true;
	}

	const char *getEngineId() const override {
		return "sci";
	}

	const char *getName() const override {
		return "SCI ["
#ifdef ENABLE_SCI32
			"all games"
#else
			"SCI0, SCI01, SCI10, SCI11"
#endif
			"]";
	}

	const char *getOriginalCopyright() const override {
		return "Sierra's Creative Interpreter (C) Sierra Online";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildEngineOptionsWidgetStatic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

void SciMetaEngineDetection::registerDefaultSettings(const Common::String &target) const {
	AdvancedMetaEngineDetection::registerDefaultSettings(target);

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		ConfMan.registerDefault(entry->configOption, entry->defaultState);
}

GUI::OptionsContainerWidget *SciMetaEngineDetection::buildEngineOptionsWidgetStatic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new OptionsWidget(boss, name, target);
}

ADDetectedGame SciMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	/**
	 * Fallback detection for Sci heavily depends on engine resources, so it's not possible
	 * to use them without the engine present in a clean way.
	 */

	if (ConfMan.hasKey("always_run_fallback_detection_extern")) {
		if (ConfMan.getBool("always_run_fallback_detection_extern") == false) {
			warning("SCI: Fallback detection is disabled.");
			return ADDetectedGame();
		}
	}

	const Plugin *metaEnginePlugin = EngineMan.findPlugin(getEngineId());

	if (metaEnginePlugin) {
		const Plugin *enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);
		if (enginePlugin) {
			return enginePlugin->get<AdvancedMetaEngine>().fallbackDetectExtern(_md5Bytes, allFiles, fslist);
		} else {
			static bool warn = true;
			if (warn) {
				warning("Engine plugin for SCI not present. Fallback detection is disabled.");
				warn = false;
			}
		}
	}

	return ADDetectedGame();
}

} // End of namespace Sci

REGISTER_PLUGIN_STATIC(SCI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sci::SciMetaEngineDetection);
