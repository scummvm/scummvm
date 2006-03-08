/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "gui/theme.h"
#include "gui/eval.h"

namespace GUI {

const char *Theme::_defaultConfigINI = "\n\
# Define our classic greenish theme here\n\
[320xY]\n\
def_widgetSize=kNormalWidgetSize\n\
def_buttonWidth=kButtonWidth\n\
def_buttonHeight=kButtonHeight\n\
def_sliderWidth=kSliderWidth\n\
def_sliderHeight=kSliderHeight\n\
def_kLineHeight=12\n\
def_kFontHeight=10\n\
def_globOptionsW=(w - 2 * 10)\n\
def_globOptionsH=(h - 1 * 40)\n\
def_tabPopupsLabelW=100\n\
def_midiControlsSpacing=1\n\
use=XxY\n\
\n\
[XxY]\n\
def_widgetSize=kBigWidgetSize\n\
def_buttonWidth=kBigButtonWidth\n\
def_buttonHeight=kBigButtonHeight\n\
def_sliderWidth=kBigSliderWidth\n\
def_sliderHeight=kBigSliderHeight\n\
def_kLineHeight=16\n\
def_kFontHeight=14\n\
def_globOptionsW=(w - 2 * 10)\n\
def_globOptionsH=(h - 2 * 40)\n\
def_tabPopupsLabelW=150\n\
def_midiControlsSpacing=2\n\
chooser_headline=10 6 (w - 2 * 16) (kLineHeight)\n\
chooser_list=10 (6 + kLineHeight + 2) (w - 2 * 16) (h - self.y - buttonHeight - 12)\n\
\n\
## launcher\n\
hBorder=10\n\
launcher_version=hBorder 8 (w - 2 * hBorder) kLineHeight\n\
top=(h - 8 - buttonHeight)\n\
numButtons=4\n\
space=8\n\
butWidth=((w - 2 * hBorder - space * (numButtons - 1)) / numButtons)\n\
launcher_quit_button=hBorder top butWidth buttonHeight\n\
launcher_about_button=(prev.x2 + space) top butWidth buttonHeight\n\
launcher_options_button=(prev.x2 + space) top butWidth buttonHeight\n\
launcher_start_button=(prev.x2 + space) top butWidth buttonHeight\n\
top=(top - buttonHeight * 2)\n\
numButtons=3\n\
space=10\n\
butWidth=((w - 2 * hBorder - space * (numButtons - 1)) / numButtons)\n\
launcher_addGame_button=hBorder top butWidth buttonHeight\n\
launcher_editGame_button=(prev.x2 + space) top butWidth buttonHeight\n\
launcher_removeGame_button=(prev.x2 + space) top butWidth buttonHeight\n\
launcher_list=hBorder (kLineHeight + 16) (w - 2 * hBorder) (top - kLineHeight - 20)\n\
\n\
# global options\n\
globaloptions=10 40 globOptionsW globOptionsH\n\
set_parent=globaloptions\n\
vBorder=5\n\
globaloptions_tabwidget=0, vBorder parent.w (parent.h - buttonHeight -8 - 2 * vBorder)\n\
\n\
# graphics tab\n\
opYoffset=vBorder\n\
opWidth=globOptionsW\n\
useWithPrefix=graphicsControls globaloptions_\n\
\n\
# audio tab\n\
opYoffset=vBorder\n\
opWidth=globOptionsW\n\
useWithPrefix=audioControls globaloptions_\n\
useWithPrefix=volumeControls globaloptions_\n\
\n\
# MIDI tab\n\
opYoffset=vBorder\n\
opWidth=globOptionsW\n\
useWithPrefix=midiControls globaloptions_\n\
\n\
# paths tab\n\
yoffset=vBorder\n\
globaloptions_savebutton=5 yoffset (buttonWidth + 5) buttonHeight\n\
globaloptions_savepath=(prev.x2 + 20) (vBorder + 3) (parent.w - (5 + buttonWidth + 20) - 10) kLineHeight\n\
yoffset=(yoffset + buttonHeight + 4)\n\
globaloptions_extrabutton=5 yoffset (buttonWidth + 5) buttonHeight\n\
globaloptions_extrapath=(prev.x2 + 20) (vBorder + 3) (parent.w - (5 + buttonWidth + 20) - 10) kLineHeight\n\
yoffset=(yoffset + buttonHeight + 4)\n\
globaloptions_keysbutton=5 yoffset buttonWidth buttonHeight\n\
\n\
globaloptions_ok=(parent.w - (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n\
globaloptions_cancel=(parent.w - 2 * (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n\
\n\
# game options\n\
opYoffset=(vBorder + buttonHeight)\n\
opWidth=globOptionsW\n\
useWithPrefix=graphicsControls gameoptions_\n\
\n\
opYoffset=(vBorder + buttonHeight)\n\
useWithPrefix=audioControls gameoptions_\n\
\n\
opYoffset=(vBorder + buttonHeight)\n\
useWithPrefix=volumeControls gameoptions_\n\
\n\
opYoffset=(vBorder + buttonHeight)\n\
useWithPrefix=midiControls gameoptions_\n\
\n\
### SCUMM game options\n\
opYoffset=8\n\
useWithPrefix=volumeControls scummoptions_\n\
\n\
[graphicsControls]\n\
gcx=10\n\
gcw=(opWidth - 2 * 10)\n\
grModePopup=(gcx - 5) opYoffset (gcw + 5) kLineHeight\n\
opYoffset=(opYoffset + kLineHeight + 4)\n\
grRenderPopup=(gcx - 5) opYoffset (gcw + 5) kLineHeight\n\
opYoffset=(opYoffset + kLineHeight + 4)\n\
grFullscreenCheckbox=gcx opYoffset (kFontHeight + 10 + 96) buttonHeight\n\
opYoffset=(opYoffset + buttonHeight)\n\
grAspectCheckbox=gcx opYoffset (kFontHeight + 10 + 136) buttonHeight\n\
opYoffset=(opYoffset + buttonHeight)\n\
\n\
[audioControls]\n\
aux=10\n\
auw=(opWidth - 2 * 10)\n\
auMidiPopup=(aux - 5) opYoffset (auw + 5) kLineHeight\n\
opYoffset=(opYoffset + kLineHeight + 4)\n\
auSubtitlesCheckbox=aux opYoffset (kFontHeight + 10 + 102) buttonHeight\n\
opYoffset=(opYoffset + buttonHeight + 18)\n\
\n\
[volumeControls]\n\
vctextw=95\n\
vcxoff=(vctextw + 15)\n\
vcMusicText=10 (opYoffset + 2) vctextw kLineHeight\n\
vcMusicSlider=vcxoff opYoffset sliderWidth sliderHeight\n\
vcMusicLabel=(vcxoff + prev.w + 10) (opYoffset + 2) 24 kLineHeight\n\
opYoffset=(opYoffset + sliderHeight + 4)\n\
vcSfxText=10 (opYoffset + 2) vctextw kLineHeight\n\
vcSfxSlider=vcxoff opYoffset sliderWidth sliderHeight\n\
vcSfxLabel=(vcxoff + prev.w + 10) (opYoffset + 2) 24 kLineHeight\n\
opYoffset=(opYoffset + sliderHeight + 4)\n\
vcSpeechText=10 (opYoffset + 2) vctextw kLineHeight\n\
vcSpeechSlider=vcxoff opYoffset sliderWidth sliderHeight\n\
vcSpeechLabel=(vcxoff + prev.w + 10) (opYoffset + 2) 24 kLineHeight\n\
opYoffset=(opYoffset + sliderHeight + 4)\n\
\n\
[midiControls]\n\
mcx=10\n\
mcFontButton=mcx opYoffset buttonWidth buttonHeight\n\
mcFontPath=(prev.x2 + 20) (opYoffset + 3) (opWidth - (buttonWidth + 20) - 10) kLineHeight\n\
opYoffset=(opYoffset + buttonHeight + 2 * midiControlsSpacing)\n\
mcMixedCheckbox=mcx opYoffset (kFontHeight + 10 + 135) buttonHeight\n\
opYoffset=(opYoffset + buttonHeight + midiControlsSpacing)\n\
mcMt32Checkbox=mcx opYoffset (kFontHeight + 10 + 256) buttonHeight\n\
opYoffset=(opYoffset + buttonHeight + midiControlsSpacing)\n\
mcGSCheckbox=mcx opYoffset (kFontHeight + 10 + 142) buttonHeight\n\
opYoffset=(opYoffset + buttonHeight + midiControlsSpacing)\n\
";

using Common::String;

void Theme::processSingleLine(const String &section, const String prefix, const String name, const String str) {
	int level = 0;
	int start = 0;
	uint i;
	int value;
	const char *postfixes[] = {"x", "y", "w", "h"};
	int npostfix = 0;

	// Make self.BLAH work
	for (i = 0; i < ARRAYSIZE(postfixes); i++) {
		String from, to;

		from = String("self.") + postfixes[i];
		to = prefix + name + "." + postfixes[i];

		_evaluator->setAlias(from, to);
		_evaluator->setVariable(to, EVAL_UNDEF_VAR);
	}

	for (i = 0; i < str.size(); i++) {
		if (isspace(str[i]) && level == 0) {
			value = _evaluator->eval(String(&(str.c_str()[start]), i - start), section, name, start);
			_evaluator->setVariable(prefix + name + "." + postfixes[npostfix++], value);
			start = i + 1;
		}
		if (str[i] == '(')
			level++;

		if (str[i] == ')') {
			if (level == 0) {
				error("Extra ')' in section: [%s] expression: \"%s\" start is at: %d",
					  section.c_str(), name.c_str(), start);
			}
			level--;
		}
	}

	if (level > 0)
		error("Missing ')' in section: [%s] expression: \"%s\" start is at: %d",
			  section.c_str(), name.c_str(), start);

	value = _evaluator->eval(String(&(str.c_str()[start]), i - start), section, name, start);

	// process VAR=VALUE construct
	if (npostfix == 0)
		_evaluator->setVariable(name, value);
	else
		_evaluator->setVariable(prefix + name + "." + postfixes[npostfix], value);

	// If we have all 4 parameters, set .x2 and .y2
	if (npostfix == 3) {
		_evaluator->setVariable(prefix + name + ".x2", 
			_evaluator->getVar(prefix + name + ".x") + _evaluator->getVar(prefix + name + ".w"));
		_evaluator->setVariable(prefix + name + ".y2", 
			_evaluator->getVar(prefix +name + ".y") + _evaluator->getVar(prefix + name + ".h"));
	}

	if (npostfix != 0)
		setSpecialAlias("prev", prefix + name);
}


void Theme::processResSection(Common::ConfigFile &config, String name, bool skipDefs, const String prefix) {
	debug(3, "Reading section: [%s]", name.c_str());

	const Common::ConfigFile::SectionKeyList &keys = config.getKeys(name);

	Common::ConfigFile::SectionKeyList::const_iterator iterk;
	for (iterk = keys.begin(); iterk != keys.end(); ++iterk) {
		if (iterk->key == "set_parent") {
			setSpecialAlias("parent", prefix + iterk->value);
			continue;
		}
		if (iterk->key.hasPrefix("set_")) {
			_evaluator->setAlias(name, iterk->key, prefix + iterk->value);
			continue;
		}
		if (iterk->key.hasPrefix("def_")) {
			if (!skipDefs)
				_evaluator->setVariable(name, prefix + iterk->key, iterk->value);
			continue;
		}
		if (iterk->key == "use") {
			if (iterk->value == name)
				error("Theme section [%s]: cannot use itself", name.c_str());
			if (!config.hasSection(name))
				error("Undefined use of section [%s]", name.c_str());
			processResSection(config, iterk->value, true);
			continue;
		}
		if (iterk->key == "useWithPrefix") {
			const char *temp = iterk->value.c_str();
            const char *pos = strrchr(temp, ' ');
			String n, pref;

			if (pos == NULL)
				error("2 arguments required for useWithPrefix keyword");

			n = String(temp, strchr(temp, ' ') - temp);
			pref = String(pos + 1);

			if (n == name)
				error("Theme section [%s]: cannot use itself", n.c_str());
			if (!config.hasSection(n))
				error("Undefined use of section [%s]", n.c_str());
			processResSection(config, n, true, pref);
			continue;
		}
		processSingleLine(name, prefix, iterk->key, iterk->value);
	}
}

void Theme::setSpecialAlias(const String alias, const String &name) {
	const char *postfixes[] = {"x", "y", "w", "h", "x2", "y2"};
	int i;

	for (i = 0; i < ARRAYSIZE(postfixes); i++) {
		String from, to;

		from = alias + "." + postfixes[i];
		to = name + "." + postfixes[i];

		_evaluator->setAlias(from, to);
	}
}

bool Theme::isThemeLoadingRequired() {
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (_loadedThemeX == x && _loadedThemeY == y)
		return false;

	_loadedThemeX = x;
	_loadedThemeY = y;

	return true;
}

void Theme::loadTheme(Common::ConfigFile &config, bool reset) {
	char name[80];
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (reset)
		_evaluator->reset();

	strcpy(name, "XxY");
	if (config.hasSection(name))
		processResSection(config, name);

	sprintf(name, "%dxY", x);
	if (config.hasSection(name))
		processResSection(config, name);

	sprintf(name, "Xx%d", y);
	if (config.hasSection(name))
		processResSection(config, name);

	sprintf(name, "%dx%d", x, y);
	if (config.hasSection(name))
		processResSection(config, name);
}

} // End of namespace GUI
