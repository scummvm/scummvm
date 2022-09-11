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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/events.h"
#include "common/localization.h"
#include "common/translation.h"
#include "common/ustr.h"

#include "graphics/scaler.h"

#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/ThemeEval.h"

#include "scumm/dialogs.h"
#include "scumm/sound.h"
#include "scumm/scumm.h"
#include "scumm/imuse/imuse.h"
#include "scumm/verbs.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

using Graphics::kTextAlignCenter;
using Graphics::kTextAlignLeft;
using GUI::WIDGET_ENABLED;

namespace Scumm {

static const ResString string_map_table_v8[] = {
	{0, "/BT_100/Please insert disk %d. Press ENTER"},
	{0, "/BT__003/Unable to Find %s, (%s %d) Press Button."},
	{0, "/BT__004/Error reading disk %c, (%c%d) Press Button."},
	{0, "/BT__002/Game Paused.  Press SPACE to Continue."},
	{0, "/BT__005/Are you sure you want to restart?  (Y/N)"}, //BOOT.004
	{0, "/BT__006/Are you sure you want to quit?  (Y/N)"},    //BOOT.005
	{0, "/BT__008/Save"},
	{0, "/BT__009/Load"},
	{0, "/BT__010/Play"},
	{0, "/BT__011/Cancel"},
	{0, "/BT__012/Quit"},
	{0, "/BT__013/OK"},
	{0, ""},
	{0, "/BT__014/You must enter a name"},
	{0, "/BT__015/The game was NOT saved (disk full?)"},
	{0, "/BT__016/The game was NOT loaded"},
	{0, "/BT__017/Saving '%s'"},
	{0, "/BT__018/Loading '%s'"},
	{0, "/BT__019/Name your SAVE game"},
	{0, "/BT__020/Select a game to LOAD"},
	{0, "/BT__028/Do you want to replace this saved game? (Y/N)Y"},
	{0, "/SYST200/Are you sure you want to quit?"},
	{0, "/SYST201/Yes"},
	{0, "/SYST202/No"},
	{0, "/SYST203/iMuse buffer count changed to %d"},
	{0, "/BT_104/Voice and Text"},
	{0, "/BT_105/Text Display Only"},
	{0, "/BT_103/Voice Only"},
	{0, "/SYST300/y"},
	{0, "/BOOT.005/Are you sure you want to quit?  (Y-N)Y"}, // Demo strings
	{0, "/NEW.23/Text Speed  Slow  ==========  Fast"},
	{0, "/NEW.24/Music Volume  Low  =========  High"},
	{0, "/NEW.25/Voice Volume  Low  =========  High"},
	{0, "/NEW.26/Sfx Volume  Low  =========  High"},
	{0, "Heap %dK"} // Non-translatable string
};

static const ResString string_map_table_v7[] = {
	{96, "game name and version"}, //that's how it's supposed to be
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"},
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{0, ""},
	{78, "/BOOT.014/You must enter a name"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"},
	// Original GUI strings
	{68, "/BOOT.007/c:\\dig"},
	{69, "/BOOT.021/The Dig"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"},
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"},
	{78, "/BOOT.014/You must enter a name"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{90, "/BOOT.022/Music"},
	{91, "/BOOT.023/Voice"},
	{92, "/BOOT.024/Sfx"},
	{93, "/BOOT.025/disabled"},
	{94, "/BOOT.026/Text speed"},
	{95, "/BOOT.027/Display Text"},
	{96, "game name and version"},
	{137, "/BOOT.028/Spooled Music"},
	{138, "/BOOT.029/Do you want to replace this saved game?  (Y/N)"},
	{141, "/NEW.020/Voice Only"},
	{142, "/NEW.021/Voice and Text"},
	{143, "/NEW.022/Text Display Only"},
	{145, "/NEW.023/Text Speed   Slow  ==========  Fast"},
	{147, "/NEW.024/Music Volume    Low  =========  High"},
	{149, "/NEW.025/Voice Volume    Low  =========  High"},
	{151, "/NEW.026/SFX Volume    Low  =========  High"},
	{144, "Heap %dK"},
	{0, "iMuse buffer count changed to %d"} // Not in the original
};

static const ResString string_map_table_v6[] = {
	{90, "Insert Disk %c and Press Button to Continue."},
	{91, "Unable to Find %s, (%c%d) Press Button."},
	{92, "Error reading disk %c, (%c%d) Press Button."},
	{93, "Game Paused.  Press SPACE to Continue."},
	{94, "Are you sure you want to restart?  (Y/N)Y"},
	{95, "Are you sure you want to quit?  (Y/N)Y"},
	{96, "Save"},
	{97, "Load"},
	{98, "Play"},
	{99, "Cancel"},
	{100, "Quit"},
	{101, "OK"},
	{102, "Insert save/load game disk"},
	{103, "You must enter a name"},
	{104, "The game was NOT saved (disk full?)"},
	{105, "The game was NOT loaded"},
	{106, "Saving '%s'"},
	{107, "Loading '%s'"},
	{108, "Name your SAVE game"},
	{109, "Select a game to LOAD"},
	{117, "How may I serve you?"},
	{80, "Text Speed"},
	{81, "Display Text"},
	{113, "Music"},
	{114, "Voice"},
	{115, "Sfx"},
	{116, "disabled"},
	{0, "Voice Only"},
	{0, "Voice and Text"},
	{0, "Text Display Only"},
	{0, "Text Speed   Slow  ==========  Fast"},
	{0, "Music Volume    Low  =========  High"},
	{0, "Voice Volume    Low  =========  High"},
	{0, "SFX Volume    Low  =========  High"},
	{0, "Heap %dK"}
};

#pragma mark -

#ifndef DISABLE_HELP

class HelpDialog : public ScummDialog {
public:
	HelpDialog(const GameSettings &game);
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	void reflowLayout() override;

protected:
	typedef Common::String String;

	GUI::ButtonWidget *_nextButton;
	GUI::ButtonWidget *_prevButton;

	GUI::StaticTextWidget *_title;
	GUI::StaticTextWidget *_key[HELP_NUM_LINES];
	GUI::StaticTextWidget *_dsc[HELP_NUM_LINES];

	int _page;
	int _numPages;
	int _numLines;

	const GameSettings _game;

	void displayKeyBindings();
};

#endif

#pragma mark -

ScummDialog::ScummDialog(int x, int y, int w, int h) : GUI::Dialog(x, y, w, h) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
}

ScummDialog::ScummDialog(String name) : GUI::Dialog(name) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
}

#pragma mark -

#ifndef DISABLE_HELP

ScummMenuDialog::ScummMenuDialog(ScummEngine *scumm)
	: MainMenuDialog(scumm) {
	_helpDialog = new HelpDialog(scumm->_game);
	_helpButton->setEnabled(true);
}

ScummMenuDialog::~ScummMenuDialog() {
	delete _helpDialog;
}

void ScummMenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kHelpCmd:
		_helpDialog->runModal();
		break;
	default:
		MainMenuDialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV'
};

HelpDialog::HelpDialog(const GameSettings &game)
	: ScummDialog("ScummHelp"), _game(game) {
	_title = new GUI::StaticTextWidget(this, "ScummHelp.Title", Common::U32String());

	_page = 1;
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundDefault;

	_numPages = ScummHelp::numPages(_game.id);

	// I18N: Previous page button
	_prevButton = new GUI::ButtonWidget(this, "ScummHelp.Prev", _("~P~revious"), Common::U32String(), kPrevCmd);
	// I18N: Next page button
	_nextButton = new GUI::ButtonWidget(this, "ScummHelp.Next", _("~N~ext"), Common::U32String(), kNextCmd);
	new GUI::ButtonWidget(this, "ScummHelp.Close", _("~C~lose"), Common::U32String(), GUI::kCloseCmd);
	_prevButton->clearFlags(WIDGET_ENABLED);

	GUI::ContainerWidget *placeHolder = new GUI::ContainerWidget(this, "ScummHelp.HelpText");
	placeHolder->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);

	_numLines = HELP_NUM_LINES;

	// Dummy entries
	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i] = new GUI::StaticTextWidget(this, 0, 0, 10, 10, Common::U32String(), Graphics::kTextAlignRight);
		_dsc[i] = new GUI::StaticTextWidget(this, 0, 0, 10, 10, Common::U32String(), Graphics::kTextAlignLeft);
	}

}

void HelpDialog::reflowLayout() {
	ScummDialog::reflowLayout();

	int lineHeight = g_gui.getFontHeight();
	int16 x, y;
	int16 w, h;

	assert(lineHeight);

	g_gui.xmlEval()->getWidgetData("ScummHelp.HelpText", x, y, w, h);

	// Make sure than we don't have more lines than what we can fit
	// on the space that the layout reserves for text
	_numLines = MIN(HELP_NUM_LINES, (int)(h / lineHeight));

	int keyW = w * 20 / 100;
	int dscX = x + keyW + 32;
	int dscW = w * 80 / 100;

	int xoff = (_w >> 1) - (w >> 1);

	for (int i = 0; i < _numLines; i++) {
		_key[i]->resize(xoff + x, y + lineHeight * i, keyW, lineHeight, false);
		_dsc[i]->resize(xoff + dscX, y + lineHeight * i, dscW, lineHeight, false);
	}

	displayKeyBindings();
}

void HelpDialog::displayKeyBindings() {
	U32String titleStr, *keyStr, *dscStr;

	ScummHelp::updateStrings(_game.id, _game.version, _game.platform, _page, titleStr, keyStr, dscStr);

	_title->setLabel(titleStr);
	for (int i = 0; i < _numLines; i++) {
		_key[i]->setLabel(keyStr[i]);
		_dsc[i]->setLabel(dscStr[i]);
	}

	delete[] keyStr;
	delete[] dscStr;
}

void HelpDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kNextCmd:
		_page++;
		if (_page >= _numPages) {
			_nextButton->clearFlags(WIDGET_ENABLED);
		}
		if (_page >= 2) {
			_prevButton->setFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		g_gui.scheduleTopDialogRedraw();
		break;
	case kPrevCmd:
		_page--;
		if (_page <= _numPages) {
			_nextButton->setFlags(WIDGET_ENABLED);
		}
		if (_page <= 1) {
			_prevButton->clearFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

#endif

#pragma mark -

static bool isCJKLanguage(Common::Language lang) {
	switch (lang) {
	case Common::KO_KOR:
	case Common::JA_JPN:
	case Common::ZH_TWN:
	case Common::ZH_CHN:
		return true;
	default:
		return false;
	}
}

InfoDialog::InfoDialog(ScummEngine *scumm, int res)
: ScummDialog(0, 0, 0, 0), _vm(scumm), _style(GUI::ThemeEngine::kFontStyleBold) { // dummy x and w

	_message = queryResString(res);

	// Trim the hardcoded strings for the GUI Dialog. The extra spaces which some strings have might
	// be needed for proper alignment within the original display, but not here...
	if (scumm->_game.version < 3)
		_message.trim();

	Common::Language lang = isCJKLanguage(_vm->_language) ? _vm->_language : Common::UNK_LANG;

	// Width and height are dummy
	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleBold, lang);

	// Store this for the calls to getStringWidth() and getStringHeight() in reflowLayout().
	if (lang != Common::UNK_LANG)
		_style = GUI::ThemeEngine::kFontStyleLangExtra;
}

InfoDialog::InfoDialog(ScummEngine *scumm, const U32String &message)
: ScummDialog(0, 0, 0, 0), _vm(scumm), _style(GUI::ThemeEngine::kFontStyleBold) { // dummy x and w

	_message = message;

	// Width and height are dummy
	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, kTextAlignCenter);
}

void InfoDialog::setInfoText(const U32String &message) {
	_message = message;
	_text->setLabel(_message);
	//reflowLayout(); // FIXME: Should we call this here? Depends on the usage patterns, I guess...
}

void InfoDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int width = g_gui.getStringWidth(_message, _style) + 16;
	int height = g_gui.getFontHeight(_style) + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	_text->setSize(_w, _h);
}

const char *InfoDialog::getPlainEngineString(int stringno) {
	const char *result;

	if (stringno == 0)
		return nullptr;

	if (_vm->_game.version == 8) {
		return string_map_table_v8[stringno - 1].string;
	} else if (_vm->_game.version == 7) {
		result = (const char *)_vm->getStringAddressVar(string_map_table_v7[stringno - 1].num);

		if (!result) {
			result = string_map_table_v7[stringno - 1].string;
		}
	} else if (_vm->_game.version == 6) {
		result = (const char *)_vm->getStringAddressVar(string_map_table_v6[stringno - 1].num);

		if (!result) {
			result = string_map_table_v6[stringno - 1].string;
		}
	} else if (_vm->_game.version >= 3) {
		result = (const char *)_vm->getStringAddress(getStaticResString(_vm->_language, stringno - 1).num);

		if (!result) {
			result = getStaticResString(_vm->_language, stringno - 1).string;
		}
	} else {
		result = getStaticResString(_vm->_language, stringno - 1).string;
	}

	return result;
}

void decodeV2String(Common::Language lang, Common::String &str) {
	struct mapping { char o, n; };
	// Just the bare minimum needed for the strings in getStaticResString()...
	static const mapping mapFR[] = { { '[', '\x82' }, { '<', '\x85' }, { '~', '\x96' }, { '\0', '\0' } };
	static const mapping mapDE[] = { { '[', '\x81' }, { '\\','\x84' }, { '\0', '\0' } };
	static const mapping mapIT[] = { { '\0', '\0' } };
	static const mapping mapES[] = { { '\0', '\0' } };
	static const mapping mapRU[] = { { '\0', '\0' } };
	static const mapping mapSE[] = { { '\0', '\0' } };

	const mapping *map = 0;
	switch (lang) {
	case Common::FR_FRA:
		map = mapFR;
		break;
	case Common::DE_DEU:
		map = mapDE;
		break;
	case Common::IT_ITA:
		map = mapIT;
		break;
	case Common::ES_ESP:
		map = mapES;
		break;
	case Common::RU_RUS:
		map = mapRU;
		break;
	case Common::SE_SWE:
		map = mapSE;
		break;
	default:
		break;
	}

	if (map) {
		while (map->o) {
			for (uint16 i = 0; i < str.size(); ++i) {
				if (str[i] == map->o)
					str.setChar(map->n, i);
			}
			++map;
		}
	}
}

const Common::U32String InfoDialog::queryResString(int stringno) {
	byte buf[256];
	byte reverseBuf[256];
	const byte *result;

	if (stringno == 0)
		return String();

	if (_vm->_game.heversion >= 80)
		return _(string_map_table_v6[stringno - 1].string);
	else if (_vm->_game.version == 8)
		result = (const byte *)string_map_table_v8[stringno - 1].string;
	else if (_vm->_game.version == 7)
		result = _vm->getStringAddressVar(string_map_table_v7[stringno - 1].num);
	else if (_vm->_game.version == 6)
		result = _vm->getStringAddressVar(string_map_table_v6[stringno - 1].num);
	else if (_vm->_game.version >= 3)
		result = _vm->getStringAddress(getStaticResString(_vm->_language, stringno - 1).num);
	else
		result = (const byte *)getStaticResString(_vm->_language, stringno - 1).string;

	if (result && *result == '/') {
		_vm->translateText(result, buf, sizeof(buf));
		result = buf;
	}

	if (!result || *result == '\0') // Gracelessly degrade to english :)
		result = (const byte *)getStaticResString(_vm->_language, stringno - 1).string;

	if (_vm->reverseIfNeeded(result, reverseBuf, sizeof(reverseBuf)))
		result = reverseBuf;
	// Convert to a proper string (take care of FF codes)
	byte chr;
	String tmp;
	while ((chr = *result++)) {
		if (chr == 0xFF) {
			result += 3;
		} else if (chr != '@') {
			tmp += chr;
		}
	}

	// The localized v1/v2 games use custom encodings, different for each language.
	// We just remap the characters that we need here..
	if (_vm->_game.version < 3)
		decodeV2String(_vm->_language, tmp);

	return U32String(tmp, _vm->getDialogCodePage());
}

const ResString &InfoDialog::getStaticResString(Common::Language lang, int stringno) {
	// The string parts are only needed for v1/2. So we need to provide only the
	// language varieties that exist for these. I have added the langugage I found
	// in scumm-md5.h. I guess we could actually ditch the first 3 lines...
	static const ResString strMap1[][6] = {
		{	// English
			{1, "Insert Disk %c and Press Button to Continue."},	// Original DOS has: "Please Insert Disk n.  Press ENTER"
			{2, "Unable to Find %s, (%c%d) Press Button."},			// Original DOS has: "Unable to find file nn.lfl Press ENTER"
			{3, "Error reading disk %c, (%c%d) Press Button."},		// Original DOS has: "ERROR READING FILE.  HIT KEY TO RETRY'" and "ERROR READING %d type %d"
			{4, "Game paused, press SPACE to continue.  "},			// Original string from the English v1/v2 interpreters
			{5, "Are you sure you want to restart? (y/n)y"},		// Original string from the English v1/v2 interpreters
			{6, "Are you sure you want to quit? (y/n)y"}			// The original does not have a quit confirmation question. So this it just the restart string with a word replacement.
		},
		{	// French
			{1, "Ins[rez le disque n. Appuyez sur ENTER."},						// Original DOS
			{2, "Incapable de trouver la fiche nn.lfl.   Appuyez sur ENTER."},	// Original DOS (bad translation...)
			{3, "ERREUR. Appuyez sur une touche pour     essayer < nouveau."},	// Original DOS (with that many spaces, maybe to force it to go to a new line?)
			{4, "PAUSE-Appuyez sur SPACE pour continuer."},						// Original string
			{5, "Etes-vous s~r de vouloir recommencer?   (o/n)o"},				// Original string: note the lack of the ending letter after the last parenthesis. Multiple spaces make the "(o/n)" part go to the next line
			{6, "Etes-vous s~r de vouloir quitter ? (o/n)o"}					// (matching the previous sentence)
		},
		{	// German
			{1, "Diskette n einlegen.  Bet\\tige EINGABE."},		// Original DOS German v2
			{2, "Datei nn.lfl nicht gefunden. EINGABE."},			// Original DOS German v2
			{3, "ERROR. Bet\\tigen Sie eine Taste."},				// Original DOS German v2
			{4, "PAUSE - Zum Spielen Leertaste dr[cken. "},			// Original DOS German v2
			{5, "Wollen Sie neu starten? (j/n)j"},					// Original DOS German v2
			{6, "Wollen Sie wirklich beenden? (j/n)j"}				// (matching the previous sentence)
		},
		{	// Italian
			{1, "Inserisci il disco %c e premi un pulsante per continuare."},
			{2, "Impossibile trovare %s, (%c%d) Premere un pulsante."},
			{3, "Errore nella lettura del disco %c, (%c%d) Premere un pulsante."},
			{4, "Gioco in pausa. Premi SPAZIO per continuare."},
			{5, "Sei sicuro di voler ricominciare?  (S/N)S"},
			{6, "Sei sicuro di voler uscire?  (S/N)S"}
		},
		{	// Spanish
			{1, "Introduce el disco %c y pulsa un bot""\xa2""n para continuar."},
			{2, "No se ha podido encontrar %s, (%c%d). Pulsa un bot""\xa2""n."},
			{3, "Error al leer el disco %c, (%c%d). Pulsa un bot""\xa2""n."},
			{4, "Partida en pausa. Pulsa Espacio para continuar."},
			{5, """\xa8""Seguro que quieres reiniciar?  (S/N)S"},
			{6, """\xa8""Seguro que quieres salir?  (S/N)S"}
		},
		{	// Russian
			{1, "\x82\xe1\xe2\xa0\xa2\xec\xe2\xa5"" ""\xa4\xa8\xe1\xaa"" %c ""\xa8"" ""\xad\xa0\xa6\xac\xa8\xe2\xa5"" ""\xaa\xab\xa0\xa2\xa8\xe8\xe3"", ""\xe7\xe2\xae\xa1\xeb"" ""\xaf\xe0\xae\xa4\xae\xab\xa6\xa8\xe2\xec""."},
			{2, "\x8d\xa5"" ""\xe3\xa4\xa0\xab\xae\xe1\xec"" ""\xad\xa0\xa9\xe2\xa8"" %s, (%c%d) ""\x8d\xa0\xa6\xac\xa8\xe2\xa5"" ""\xaa\xab\xa0\xa2\xa8\xe8\xe3""."},
			{3, "\x8e\xe8\xa8\xa1\xaa\xa0"" ""\xe7\xe2\xa5\xad\xa8\xef"" ""\xa4\xa8\xe1\xaa\xa0"" %c, (%c%d) ""\x8d\xa0\xa6\xac\xa8\xe2\xa5"" ""\xaa\xab\xa0\xa2\xa8\xe8\xe3""."},
			{4, "\x88\xa3\xe0\xa0"" ""\xaf\xe0\xa8\xae\xe1\xe2\xa0\xad\xae\xa2\xab\xa5\xad\xa0"".  ""\x84\xab\xef"" ""\xaf\xe0\xae\xa4\xae\xab\xa6\xa5\xad\xa8\xef"" ""\xad\xa0\xa6\xac\xa8\xe2\xa5"" ""\xaf\xe0\xae\xa1\xa5\xab""."},
			{5, "\x82\xeb"" ""\xe3\xa2\xa5\xe0\xa5\xad\xeb"", ""\xe7\xe2\xae"" ""\xe5\xae\xe2\xa8\xe2\xa5"" ""\xad\xa0\xe7\xa0\xe2\xec"" ""\xe1\xad\xae\xa2\xa0""?  (Y/N)Y"},
			{6, "\x82\xeb"" ""\xe3\xa2\xa5\xe0\xa5\xad\xeb"", ""\xe7\xe2\xae"" ""\xe5\xae\xe2\xa8\xe2\xa5"" ""\xa2\xeb\xa9\xe2\xa8""?  (Y/N)Y"}
		},
		{	// Swedish (seems to concern MM NES only)
			{1, "Mata in skivan %c och tryck p""\x86"" knappen f""\x94""r att forts""\x84""tta."},
			{2, "Kunde inte hitta %s, (%c%d) tryck p""\x86"" knappen."},
			{3, "Fel vid inl""\x84""sning av skivan %c, (%c%d) tryck p""\x86"" knappen."},
			{4, "Spelet pausat. Tryck MELLANSLAG f""\x94""r att forts""\x84""tta."},
			{5, """\x8e""r du s""\x84""ker p""\x86"" att du vill starta om? (J/N)J"},
			{6, """\x8e""r du s""\x84""ker p""\x86"" att du vill avsluta? (J/N)J"}
		}
	};

	// Added in SCUMM4. Only the numbers are used, so there
	// is no need to provide language specific strings; there are
	// some exceptions for which there's only an hardcoded English
	// string.
	static const ResString strMap2[] = {
		{7, ("Save")},
		{8, ("Load")},
		{9, ("Play")},
		{10, ("Cancel")},
		{11, ("Quit")},
		{12, ("OK")},
		{13, ("Insert save/load game disk")},
		{14, ("You must enter a name")},
		{15, ("The game was NOT saved (disk full?)")},
		{16, ("The game was NOT loaded")},
		{17, ("Saving '%s'")},
		{18, ("Loading '%s'")},
		{19, ("Name your SAVE game")},
		{20, ("Select a game to LOAD")},
		{28, ("Game title)")},

		// Hardcoded in English for each localized version
		{0, "Voice Only"},
		{0, "Voice and Text"},
		{0, "Text Display Only"},
		{0, "Text Speed   Slow  ==========  Fast"},
		{0, "Roland Volume    Low  =========  High"},
		{0, "Heap %dK"}
	};

	if (stringno + 1 >= ARRAYSIZE(strMap1)) {
		stringno -= ARRAYSIZE(strMap1) - 1;
		assert(stringno + 1 < ARRAYSIZE(strMap2));
		return strMap2[stringno];
	}

	// I have added the langugages I found in scumm-md5.h for v1/2 games...
	int langIndex = 0;
	switch (lang) {
	case Common::FR_FRA:
		langIndex = 1;
		break;
	case Common::DE_DEU:
		langIndex = 2;
		break;
	case Common::IT_ITA:
		langIndex = 3;
		break;
	case Common::ES_ESP:
		langIndex = 4;
		break;
	case Common::RU_RUS:
		langIndex = 5;
		break;
	case Common::SE_SWE:
		langIndex = 6;
		break;
	default:
		// Just stick with English.
		break;
	}

	return strMap1[langIndex][stringno];
}

#pragma mark -

PauseDialog::PauseDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res) {
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == ' ')  // Close pause dialog if space key is pressed
		close();
	else
		ScummDialog::handleKeyDown(state);
}

ConfirmDialog::ConfirmDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res), _yesKey('y'), _noKey('n') {

	if (_message.empty())
		return;

	if (_message[_message.size() - 1] != ')') {
		_yesKey = _message[_message.size() - 1];
		_message.deleteLastChar();

		if (_yesKey >= 'A' && _yesKey <= 'Z')
			_yesKey += 'a' - 'A';

		_text->setLabel(_message);
		reflowLayout();
	}
}

void ConfirmDialog::handleKeyDown(Common::KeyState state) {
	Common::KeyCode keyYes, keyNo;

	Common::getLanguageYesNo(keyYes, keyNo);

	if (state.keycode == Common::KEYCODE_n || state.ascii == _noKey || state.ascii == keyNo) {
		setResult(0);
		close();
	} else if (state.keycode == Common::KEYCODE_y || state.ascii == _yesKey || state.ascii == keyYes) {
		setResult(1);
		close();
	} else
		ScummDialog::handleKeyDown(state);
}

#pragma mark -

ValueDisplayDialog::ValueDisplayDialog(const Common::U32String &label, int minVal, int maxVal,
		int val, uint16 incKey, uint16 decKey)
	: GUI::Dialog(0, 0, 0, 0),
	_label(label), _min(minVal), _max(maxVal),
	_value(val), _incKey(incKey), _decKey(decKey), _timer(0) {
	assert(_min <= _value && _value <= _max);
}

void ValueDisplayDialog::drawDialog(GUI::DrawLayer layerToDraw) {
	Dialog::drawDialog(layerToDraw);

	const int labelWidth = _w - 8 - _percentBarWidth;
	g_gui.theme()->drawText(Common::Rect(_x+4, _y+4, _x+labelWidth+4,
				_y+g_gui.theme()->getFontHeight()+4), _label);
	g_gui.theme()->drawSlider(Common::Rect(_x+4+labelWidth, _y+4, _x+_w-4, _y+_h-4),
				_percentBarWidth * (_value - _min) / (_max - _min));
}

void ValueDisplayDialog::handleTickle() {
	if (g_system->getMillis() > _timer) {
		close();
	}
}

void ValueDisplayDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_percentBarWidth = screenW * 100 / 640;

	int width = g_gui.getStringWidth(_label) + 16 + _percentBarWidth;
	int height = g_gui.getFontHeight() + 4 * 2;

	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;
	_w = width;
	_h = height;
}

void ValueDisplayDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == _incKey || state.ascii == _decKey) {
		if (state.ascii == _incKey && _value < _max)
			_value++;
		else if (state.ascii == _decKey && _value > _min)
			_value--;

		setResult(_value);
		_timer = g_system->getMillis() + kDisplayDelay;
		g_gui.scheduleTopDialogRedraw();
	} else {
		close();
	}
}

void ValueDisplayDialog::open() {
	GUI::Dialog::open();
	setResult(_value);
	_timer = g_system->getMillis() + kDisplayDelay;
}

SubtitleSettingsDialog::SubtitleSettingsDialog(ScummEngine *scumm, int value)
	: InfoDialog(scumm, U32String()), _value(value), _timer(0) {

}

void SubtitleSettingsDialog::handleTickle() {
	InfoDialog::handleTickle();
	if (g_system->getMillis() > _timer)
		close();
}

void SubtitleSettingsDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_t && state.hasFlags(Common::KBD_CTRL)) {
		cycleValue();

		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
	} else {
		close();
	}
}

void SubtitleSettingsDialog::open() {
	cycleValue();
	InfoDialog::open();
	setResult(_value);
}

void SubtitleSettingsDialog::cycleValue() {
	static const char *const subtitleDesc[] = {
		_s("Speech Only"),
		_s("Speech and Subtitles"),
		_s("Subtitles Only")
	};

	_value += 1;
	if (_value > 2)
		_value = 0;

	if (_value == 1 && g_system->getOverlayWidth() <= 320)
		setInfoText(_c("Speech & Subs", "lowres"));
	else
		setInfoText(_(subtitleDesc[_value]));

	_timer = g_system->getMillis() + 1500;
}

Indy3IQPointsDialog::Indy3IQPointsDialog(ScummEngine *scumm, char* text)
	: InfoDialog(scumm, Common::U32String(text)) {
}

void Indy3IQPointsDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == 'i')
		close();
	else
		ScummDialog::handleKeyDown(state);
}

DebugInputDialog::DebugInputDialog(ScummEngine *scumm, char* text)
	: InfoDialog(scumm, U32String(text)) {
	mainText = text;
	done = 0;
}

void DebugInputDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_BACKSPACE && buffer.size() > 0) {
		buffer.deleteLastChar();
		Common::String total = mainText + ' ' + buffer;
		setInfoText(total);
		g_gui.scheduleTopDialogRedraw();
		reflowLayout();
	} else if (state.keycode == Common::KEYCODE_RETURN) {
		done = 1;
		close();
		return;
	} else if ((state.ascii >= '0' && state.ascii <= '9') || (state.ascii >= 'A' && state.ascii <= 'Z') || (state.ascii >= 'a' && state.ascii <= 'z') || state.ascii == '.' || state.ascii == ' ') {
		buffer += state.ascii;
		Common::String total = mainText + ' ' + buffer;
		g_gui.scheduleTopDialogRedraw();
		reflowLayout();
		setInfoText(total);
	}
}

LoomTownsDifficultyDialog::LoomTownsDifficultyDialog()
	: Dialog("LoomTownsDifficultyDialog"), _difficulty(-1) {
	GUI::StaticTextWidget *text1 = new GUI::StaticTextWidget(this, "LoomTownsDifficultyDialog.Description1", _("Select a Proficiency Level."));
	text1->setAlign(Graphics::kTextAlignCenter);
	GUI::StaticTextWidget *text2 = new GUI::StaticTextWidget(this, "LoomTownsDifficultyDialog.Description2", _("Refer to your Loom(TM) manual for help."));
	text2->setAlign(Graphics::kTextAlignCenter);

	new GUI::ButtonWidget(this, "LoomTownsDifficultyDialog.Standard", _("Standard"), Common::U32String(), kStandardCmd);
	new GUI::ButtonWidget(this, "LoomTownsDifficultyDialog.Practice", _("Practice"), Common::U32String(), kPracticeCmd);
	new GUI::ButtonWidget(this, "LoomTownsDifficultyDialog.Expert", _("Expert"), Common::U32String(), kExpertCmd);
}

void LoomTownsDifficultyDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kStandardCmd:
		_difficulty = 1;
		close();
		break;

	case kPracticeCmd:
		_difficulty = 0;
		close();
		break;

	case kExpertCmd:
		_difficulty = 2;
		close();
		break;

	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

// Game options widgets

// Normally this would be added as a static game settings widget, but I see no
// way to get both the dynamic and the static one, so we have to duplicate it
// here.

GUI::CheckboxWidget *ScummOptionsContainerWidget::createEnhancementsCheckbox(GuiObject *boss, const Common::String &name) {
	return new GUI::CheckboxWidget(boss, name, _("Enable game-specific enhancements"), _("Allow ScummVM to make small enhancements to the game, usually based on other versions of the same game."));
}

void ScummOptionsContainerWidget::updateAdjustmentSlider(GUI::SliderWidget *slider, GUI::StaticTextWidget *value) {
	int adjustment = slider->getValue();
	const char *sign = "";

	if (adjustment < 0) {
		adjustment = -adjustment;
		sign = "-";
	} else if (adjustment > 0)
		sign = "+";

	value->setLabel(Common::String::format("%s%d.%02d", sign, adjustment / 100, adjustment % 100));

}

// EGA Loom Overture settings

LoomEgaGameOptionsWidget::LoomEgaGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		ScummOptionsContainerWidget(boss, name, "LoomEgaGameOptionsDialog", domain) {
	GUI::StaticTextWidget *text = new GUI::StaticTextWidget(widgetsBoss(), "LoomEgaGameOptionsDialog.OvertureTicksLabel", _("Overture Timing:"));

	text->setAlign(Graphics::TextAlign::kTextAlignEnd);

	_overtureTicksSlider = new GUI::SliderWidget(widgetsBoss(), "LoomEgaGameOptionsDialog.OvertureTicks", _("When using replacement music, this adjusts the time when the Overture changes to the scene with the Lucasfilm and Loom logotypes."), kOvertureTicksChanged);

	// In the Ozawa recording, the transition happens at about 1:56. At is
	// turns out, this is a fairly fast version of the tune. After checking
	// a number of different recordings, I've settled on an interval of
	// 1:40 - 2:50. This is larger than I had hoped, but I guess it's
	// really necessary.
	//
	// Hopefully that still means you can set the slider back to its default
	// value at most reasonable screen resolutions.

	_overtureTicksSlider->setMinValue(-160);
	_overtureTicksSlider->setMaxValue(540);

	_overtureTicksValue = new GUI::StaticTextWidget(widgetsBoss(), "LoomEgaGameOptionsDialog.OvertureTicksValue", Common::U32String());

	_overtureTicksValue->setFlags(GUI::WIDGET_CLEARBG);

	_enableEnhancementsCheckbox = createEnhancementsCheckbox(widgetsBoss(), "LoomEgaGameOptionsDialog.EnableEnhancements");
}

void LoomEgaGameOptionsWidget::load() {
	int loomOvertureTicks = 0;

	if (ConfMan.hasKey("loom_overture_ticks", _domain))
		loomOvertureTicks = ConfMan.getInt("loom_overture_ticks", _domain);

	_overtureTicksSlider->setValue(loomOvertureTicks);
	updateOvertureTicksValue();

	_enableEnhancementsCheckbox->setState(ConfMan.getBool("enable_enhancements", _domain));
}

bool LoomEgaGameOptionsWidget::save() {
	ConfMan.setInt("loom_overture_ticks", _overtureTicksSlider->getValue(), _domain);
	ConfMan.setBool("enable_enhancements", _enableEnhancementsCheckbox->getState(), _domain);
	return true;
}

void LoomEgaGameOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 12)
			.addPadding(0, 0, 0, 0)
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
				.addPadding(0, 0, 12, 0)
				.addWidget("OvertureTicksLabel", "OptionsLabel")
				.addWidget("OvertureTicks", "WideSlider")
				.addWidget("OvertureTicksValue", "ShortOptionsLabel")
			.closeLayout()
			.addWidget("EnableEnhancements", "Checkbox")
		.closeLayout()
	.closeDialog();
}

void LoomEgaGameOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kOvertureTicksChanged:
		updateOvertureTicksValue();
		break;
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
		break;
	}
}

void LoomEgaGameOptionsWidget::updateOvertureTicksValue() {
	int ticks = DEFAULT_LOOM_OVERTURE_TRANSITION + _overtureTicksSlider->getValue();

	_overtureTicksValue->setLabel(Common::String::format("%d:%02d.%d", ticks / 600, (ticks % 600) / 10, ticks % 10));
}

// VGA Loom Playback Adjustment settings

LoomVgaGameOptionsWidget::LoomVgaGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		ScummOptionsContainerWidget(boss, name, "LoomVgaGameOptionsDialog", domain) {
	GUI::StaticTextWidget *text = new GUI::StaticTextWidget(widgetsBoss(), "LoomVgaGameOptionsDialog.PlaybackAdjustmentLabel", _("Playback Adjust:"));

	text->setAlign(Graphics::TextAlign::kTextAlignEnd);

	_playbackAdjustmentSlider = new GUI::SliderWidget(widgetsBoss(), "LoomVgaGameOptionsDialog.PlaybackAdjustment", _("When playing sound from the CD audio track, adjust the start position of the sound by this much. Use this if you often hear bits of the wrong sound."), kPlaybackAdjustmentChanged);

	// The first sound in the track is played from frame 22. It's not
	// possible to move that more than about 0.3 seconds towards zero.
	// Everything else can be moved by as much as two seconds in each
	// direction.

	_playbackAdjustmentSlider->setMinValue(-200);
	_playbackAdjustmentSlider->setMaxValue(200);

	_playbackAdjustmentValue = new GUI::StaticTextWidget(widgetsBoss(), "LoomVgaGameOptionsDialog.PlaybackAdjustmentValue", Common::U32String());

	_playbackAdjustmentValue->setFlags(GUI::WIDGET_CLEARBG);

	_enableEnhancementsCheckbox = createEnhancementsCheckbox(widgetsBoss(), "LoomVgaGameOptionsDialog.EnableEnhancements");
}

void LoomVgaGameOptionsWidget::load() {
	int playbackAdjustment = 0;

	if (ConfMan.hasKey("loom_playback_adjustment", _domain))
		playbackAdjustment = ConfMan.getInt("loom_playback_adjustment", _domain);

	_playbackAdjustmentSlider->setValue(playbackAdjustment);
	updatePlaybackAdjustmentValue();

	_enableEnhancementsCheckbox->setState(ConfMan.getBool("enable_enhancements", _domain));
}

bool LoomVgaGameOptionsWidget::save() {
	ConfMan.setInt("loom_playback_adjustment", _playbackAdjustmentSlider->getValue(), _domain);
	ConfMan.setBool("enable_enhancements", _enableEnhancementsCheckbox->getState(), _domain);
	return true;
}

void LoomVgaGameOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 12)
			.addPadding(0, 0, 0, 0)
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
				.addPadding(0, 0, 12, 0)
				.addWidget("PlaybackAdjustmentLabel", "OptionsLabel")
				.addWidget("PlaybackAdjustment", "WideSlider")
				.addWidget("PlaybackAdjustmentValue", "ShortOptionsLabel")
			.closeLayout()
			.addWidget("EnableEnhancements", "Checkbox")
		.closeLayout()
	.closeDialog();
}

void LoomVgaGameOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kPlaybackAdjustmentChanged:
		updatePlaybackAdjustmentValue();
		break;
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
		break;
	}
}

void LoomVgaGameOptionsWidget::updatePlaybackAdjustmentValue() {
	updateAdjustmentSlider(_playbackAdjustmentSlider, _playbackAdjustmentValue);
}

// MI1 (CD) Playback Adjustment settings

MI1CdGameOptionsWidget::MI1CdGameOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		ScummOptionsContainerWidget(boss, name, "MI1CdGameOptionsDialog", domain) {
	Common::String extra = ConfMan.get("extra", domain);

	GUI::StaticTextWidget *text = new GUI::StaticTextWidget(widgetsBoss(), "MI1CdGameOptionsDialog.IntroAdjustmentLabel", _("Intro Adjust:"));

	text->setAlign(Graphics::TextAlign::kTextAlignEnd);

	_introAdjustmentSlider = new GUI::SliderWidget(widgetsBoss(), "MI1CdGameOptionsDialog.IntroAdjustment", _("When playing the intro track, play from this point in it. Use this if the music gets cut off prematurely, or if you are unhappy with the way the music syncs up with the intro."), kIntroAdjustmentChanged);

	_introAdjustmentSlider->setMinValue(0);
	_introAdjustmentSlider->setMaxValue(200);

	_introAdjustmentValue = new GUI::StaticTextWidget(widgetsBoss(), "MI1CdGameOptionsDialog.IntroAdjustmentValue", Common::U32String());

	_introAdjustmentValue->setFlags(GUI::WIDGET_CLEARBG);

	text = new GUI::StaticTextWidget(widgetsBoss(), "MI1CdGameOptionsDialog.OutlookAdjustmentLabel", _("Outlook Adjust:"));

	text->setAlign(Graphics::TextAlign::kTextAlignEnd);

	_outlookAdjustmentSlider = new GUI::SliderWidget(widgetsBoss(), "MI1CdGameOptionsDialog.OutlookAdjustment", _("The outlook music is part of the intro track. Adjust the position in the track at which it starts playing. Use this if the music is cut off, or if you hear part of the previous music."), kOutlookAdjustmentChanged);

	_outlookAdjustmentSlider->setMinValue(-200);
	_outlookAdjustmentSlider->setMaxValue(200);

	_outlookAdjustmentValue = new GUI::StaticTextWidget(widgetsBoss(), "MI1CdGameOptionsDialog.OutlookAdjustmentValue", Common::U32String());

	_outlookAdjustmentValue->setFlags(GUI::WIDGET_CLEARBG);

	_enableEnhancementsCheckbox = createEnhancementsCheckbox(widgetsBoss(), "MI1CdGameOptionsDialog.EnableEnhancements");
}

void MI1CdGameOptionsWidget::load() {
	int introAdjustment = 0;
	int outlookAdjustment = 0;

	if (ConfMan.hasKey("mi1_intro_adjustment", _domain))
		introAdjustment = ConfMan.getInt("mi1_intro_adjustment", _domain);
	_introAdjustmentSlider->setValue(introAdjustment);
	updateIntroAdjustmentValue();

	if (ConfMan.hasKey("mi1_outlook_adjustment", _domain))
		outlookAdjustment = ConfMan.getInt("mi1_outlook_adjustment", _domain);

	_outlookAdjustmentSlider->setValue(outlookAdjustment);
	updateOutlookAdjustmentValue();

	_enableEnhancementsCheckbox->setState(ConfMan.getBool("enable_enhancements", _domain));
}

bool MI1CdGameOptionsWidget::save() {
	ConfMan.setInt("mi1_intro_adjustment", _introAdjustmentSlider->getValue(), _domain);
	ConfMan.setInt("mi1_outlook_adjustment", _outlookAdjustmentSlider->getValue(), _domain);
	ConfMan.setBool("enable_enhancements", _enableEnhancementsCheckbox->getState(), _domain);
	return true;
}

void MI1CdGameOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 12)
			.addPadding(0, 0, 0, 0)
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
				.addPadding(0, 0, 12, 0)
				.addWidget("IntroAdjustmentLabel", "OptionsLabel")
				.addWidget("IntroAdjustment", "WideSlider")
				.addWidget("IntroAdjustmentValue", "ShortOptionsLabel")
			.closeLayout()
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
				.addPadding(0, 0, 0, 0)
				.addWidget("OutlookAdjustmentLabel", "OptionsLabel")
				.addWidget("OutlookAdjustment", "WideSlider")
				.addWidget("OutlookAdjustmentValue", "ShortOptionsLabel")
			.closeLayout()
			.addWidget("EnableEnhancements", "Checkbox")
		.closeLayout()
	.closeDialog();
}

void MI1CdGameOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kIntroAdjustmentChanged:
		updateIntroAdjustmentValue();
		break;
	case kOutlookAdjustmentChanged:
		updateOutlookAdjustmentValue();
		break;
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
		break;
	}
}

void MI1CdGameOptionsWidget::updateIntroAdjustmentValue() {
	updateAdjustmentSlider(_introAdjustmentSlider, _introAdjustmentValue);
}

void MI1CdGameOptionsWidget::updateOutlookAdjustmentValue() {
	updateAdjustmentSlider(_outlookAdjustmentSlider, _outlookAdjustmentValue);
}

} // End of namespace Scumm
