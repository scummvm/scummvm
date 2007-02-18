/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "parallaction/parallaction.h"
#include "parallaction/disk.h"
#include "parallaction/inventory.h"
#include "parallaction/graphics.h"
#include "parallaction/zone.h"

#include "common/savefile.h"

#include "gui/widget.h"
#include "gui/ListWidget.h"
#include "gui/message.h"

/* Nippon Safes savefiles are called 'game.0' to 'game.9'. The game conventiently allows users to
 * give meanigful name to savegames, and it uses an extra file 'savegame' to keep track of these
 * names.
 *
 * This re-implementation disposes of the extra file to make moving of savefiles easier. Debugging
 * will benefit from this, too. The savegame name is written as the first line of the savefile
 * itself, thus breaking compatibility with the original version. Who cares anyway?
 */


namespace Parallaction {


extern char _gameNames[][20];

void Parallaction::doLoadGame(uint16 slot) {

	_introSarcData3 = 200;
	_introSarcData2 = 1;

	char filename[PATH_LEN];
	sprintf(filename, "game.%i", slot);

	Common::InSaveFile *f = _saveFileMan->openForLoading(filename);
	if (!f) return;

	char s[20];

	f->readLine(s, 19);

	f->readLine(_vm->_characterName, 15);
	f->readLine(_location, 15);

	strcat(_location, ".");

	f->readLine(s, 15);
	_firstPosition._x = atoi(s);

	f->readLine(s, 15);
	_firstPosition._y = atoi(s);

	f->readLine(s, 15);
	_score = atoi(s);

	f->readLine(s, 15);
	_commandFlags = atoi(s);

	f->readLine(s, 15);

	_engineFlags |= kEngineQuit;
	freeZones(_zones._next);
	freeNodeList(_zones._next);
	_zones._next = NULL;
	_engineFlags &= ~kEngineQuit;

	_numLocations = atoi(s);

	uint16 _si;
	for (_si = 0; _si < _numLocations; _si++) {
		f->readLine(s, 20);
		s[strlen(s)] = '\0';

		strcpy(_locationNames[_si], s);

		f->readLine(s, 15);
		_localFlags[_si] = atoi(s);
	}
	_locationNames[_si][0] = '\0';

	for (_si = 0; _si < 30; _si++) {
		f->readLine(s, 15);
		_inventory[_si]._id = atoi(s);

		f->readLine(s, 15);
		_inventory[_si]._index = atoi(s);
	}

	delete f;

	_engineFlags &= ~kEngineMiniDonna;
	if (!scumm_stricmp(_vm->_characterName, "donnatras")) {
		_engineFlags |= kEngineMiniDonna;
		strcpy(_vm->_characterName, "donna");
	}
	if (!scumm_stricmp(_vm->_characterName, "minidonnatras")) {
		_engineFlags |= kEngineMiniDonna;
		strcpy(_vm->_characterName, "minidonna");
	}

	if (_vm->_characterName[0] == 'm') {
		strcpy(filename, _vm->_characterName+4);
	} else {
		strcpy(filename, _vm->_characterName);
	}
	strcat(filename, ".tab");
	freeTable(_objectsNames);
	initTable(filename, _objectsNames);

	refreshInventory(_vm->_characterName);

	parseLocation("common");
	closeArchive();

	strcat(_location, _vm->_characterName);
	_engineFlags |= kEngineChangeLocation;

#if 0
	printf("game loaded: character %s, location %s\n", _vm->_characterName, _location);
#endif

	return;
}


void Parallaction::doSaveGame(uint16 slot, const char* name) {

	char path[PATH_LEN];
	sprintf(path, "game.%i", slot);

	Common::OutSaveFile *f = _saveFileMan->openForSaving(path);
	if (f == 0) {
		char buf[32];
		sprintf(buf, "Can't save game in slot %i\n\n(%s)", slot, path);
		GUI::MessageDialog dialog(buf);
		dialog.runModal();
		return;
	}

	char s[30];

	if (!name || name[0] == '\0') {
		sprintf(s, "default_%i", slot);
	} else {
		strncpy(s, name, 29);
	}

	f->writeString(s);
	f->writeString("\n");

	if (_engineFlags & kEngineMiniDonna) {
		sprintf(s, "%stras\n", _vm->_characterName);
	} else {
		sprintf(s, "%s\n", _vm->_characterName);
	}
	f->writeString(s);

	sprintf(s, "%s\n", _saveData1);
	f->writeString(s);
	sprintf(s, "%d\n", _yourself._zone.pos._position._x);
	f->writeString(s);
	sprintf(s, "%d\n", _yourself._zone.pos._position._y);
	f->writeString(s);
	sprintf(s, "%d\n", _score);
	f->writeString(s);
	sprintf(s, "%uld\n", _commandFlags);
	f->writeString(s);

	sprintf(s, "%d\n", _numLocations);
	f->writeString(s);
	for (uint16 _si = 0; _si < _numLocations; _si++) {
		sprintf(s, "%s\n%uld\n", _locationNames[_si], _localFlags[_si]);
		f->writeString(s);
	}

	for (uint16 _si = 0; _si < 30; _si++) {
		sprintf(s, "%uld\n%d\n", _inventory[_si]._id, _inventory[_si]._index);
		f->writeString(s);
	}

	delete f;

	refreshInventory(_vm->_characterName);

	return;


}

// FIXME: only to be implemented for text mode selection
void textModeFunc1( char *) {

}




#if 0
int16 selectSaveFile(uint16 arg_0) {

	REGISTERS pregs;
	pregs._ax = (void*)(uint32)dos_videomode;
	int86(0x10, &pregs);

//	window(0 ,0, 23, 79);
	puts(" FILE NAME ");

	if (arg_0 != 0) {
		printf("(Use \x18 \x19 \x10 to edit then press enter)\n\n");
	} else {
		puts("\n");
	}

	uint16 _si;
	for (_si = 0; _si < 11; _si++) {
		printf(" %s \n", _gameNames[_si]);
	}

	int16 _di = editSaveFileName(3, 13, 0, 12, 1, arg_0);

	pregs._ax = (void*)0x93;
	int86(0x10, &pregs);

	set_vga_mode_x(0);
	byte palette[PALETTE_SIZE];
	_vm->_graphics->getBlackPalette(palette);
	_vm->_graphics->setPalette(palette);


	_vm->_graphics->copyScreen(Graphics::kBit2, Graphics::kBitBack);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

	if (arg_0 != 0) {
		_vm->_graphics->setPalette(_palette);
	}

	_vm->_graphics->swapBuffers();

	return _si;
}

int16 editSaveFileName(uint16 arg_0, uint16 arg_2, uint16 arg_4, uint16 arg_6, uint16 arg_8, uint16 arg_A) {
	uint16 _di = arg_0;
	uint16 _si = _di;
	uint16 v4 = _di;
	uint16 v1 = 0;

	do {

		set_cursor(_si, arg_4);

		while (v1 == 0) {
			int16 v6 = get_kb();
			byte _al = v6 & 0xFF;
			if (_al == 0) {

				_al = (v6 & 0xFF00) >> 8;
				if (_al == 'H') {
					if (_si<=_di)
						_si = arg_2;
					else
						_si = _si - arg_8;

					v1 = 1;
				} else
				if (_al == 'M') {

				} else
				if (_al == 'P') {
					if (_si>=arg_2)
						_si = _di;
					else
						_si = _si + arg_8;

					v1 = 1;
				}

				if (_si != 0xD && arg_A != 0) {

					set_cursor(_si, arg_4+1);
					v1 = 1;
					textModeFunc1( _gameNames[(_si - _di)/arg_8] );

					return (_si - _di)/arg_8 + 1;

				}

			} else
			if (_al == 0xD) return (_si - _di)/arg_8 + 1;
		}

		v1 = 0;
		set_cursor(v4, arg_4);
		v4 = _si;

	} while (true);

	return 0;

}
#endif

#if 0
void loadGame() {

	uint16 v6 = 0;
	uint16 v4 = 0;
	uint16 v2 = 0;
	uint16 _bx;

	FILE* stream = fopen("savegame", "r");
	if (stream) {
		for (uint16 _si = 0; _si < 10; _si++) {
			fgets(_gameNames[_si], 18, stream);
			_bx = strlen(_gameNames[_si]) - 1;
			_gameNames[_si][_bx] = '\0';
		}
		fclose(stream);
	}

	int16 _di = selectSaveFile( 0 );
	if (_di > 10) return;

	_introSarcData3 = 200;
	_introSarcData2 = 1;
	_moveSarcExaZones[0] = 0;
	_moveSarcZones[0] = 0;

	char filename[PATH_LEN];
	sprintf(filename, "game.%d", _di);
	stream = fopen(filename, "r");
	if (!stream) return;

	fgets(_vm->_characterName, 15, stream);
	_vm->_characterName[strlen(_vm->_characterName)] = '\0';

	fgets(_location, 15, stream);
	_location[strlen(_location)] = '.';

	char s[20];
	fgets(s, 15, stream);
	_firstPosition._x = atoi(s);

	fgets(s, 15, stream);
	_firstPosition._y = atoi(s);

	fgets(s, 15, stream);
	_score = atoi(s);

	fgets(s, 15, stream);
	_commandFlags = atoi(s);

	fgets(s, 15, stream);

	_engineFlags |= kEngineQuit;
	freeZones(_zones._next);
	freeNodeList(_zones._next);
	_zones._next = NULL;
	_engineFlags &= ~kEngineQuit;

	_numLocations = atoi(s);

	uint16 _si;
	for (_si = 0; _si < _numLocations; _si++) {
		fgets(s, 20, stream);
		s[strlen(s)] = '\0';

		strcpy(_locationNames[_si], s);

		fgets(s, 15, stream);
		_localFlags[_si] = atoi(s);
	}
	_locationNames[_si] = '\0';

	for (_si = 0; _si < 30; _si++) {
		fgets(s, 15, stream);
		_inventory[_si]._id = atoi(s);

		fgets(s, 15, stream);
		_inventory[_si]._index = atoi(s);
	}

	fclose(stream);

	_engineFlags &= ~kEngineMiniDonna;
	if (!scumm_stricmp(_vm->_characterName, "donnatras")) {
		_engineFlags |= kEngineMiniDonna;
		strcpy(_vm->_characterName, "donna");
	}
	if (!scumm_stricmp(_vm->_characterName, "minidonnatras")) {
		_engineFlags |= kEngineMiniDonna;
		strcpy(_vm->_characterName, "minidonna");
	}

	if (_vm->_characterName[0] == 'm') {
		strcpy(filename, _vm->_characterName+4);
	} else {
		strcpy(filename, _vm->_characterName);
	}
	strcat(filename, ".tab");
	freeTable(_objectsNames);
	initTable(filename, _objectsNames);

	refreshInventory();

	parseLocation("common");
	closeArchive();

	strcat(_location, _vm->_characterName);
	_engineFlags |= kEngineChangeLocation;

	return;
}


int16 get_kb() {

	REGISTERS pregs;
	pregs._ax = 0;
	int86(0x16, &pregs);

	return (int16)(int32)pregs._ax;

}


#endif

#if 0
void saveGame() {

//	strcpy(v30, asc_1C91A);
	uint16 v8 = 0, v6 = 0;
	int16 v4 = -1;
	int16 v2 = 0;

	if (!scumm_stricmp(_location, "caveau")) return;

	FILE* stream = fopen("savegame", "r");
	if (stream) {
		for (uint16 _si = 0; _si < 10; _si++) {
			fgets(_gameNames[_si], 18, stream);
			uint16 _bx = strlen(_gameNames[_si]) - 1;
			_gameNames[_si][_bx] = '\0';
		}
		fclose(stream);
	}

	int16 _di = selectSaveFile( 1 );
	if (_di > 10) return;

	stream = fopen("savegame", "w");
	if (stream) {
		for (uint16 _si = 0; _si < 10; _si++) {
			fputs(_gameNames[_si], stream);
			fputs("\n", stream);
		}
		fclose(stream);
	}

	char path[PATH_LEN];
	sprintf(path, "game.%d", _di);
	stream = fopen(path, "w");
	if (!stream) return;

	if (_engineFlags & kEngineMiniDonna) {
		fprintf(stream, "%stras\n", _vm->_characterName);
	} else {
		fprintf(stream, "%s\n", _vm->_characterName);
	}

	fprintf(stream, "%s\n", _saveData1);
	fprintf(stream, "%d\n", _yourself._zone.pos._position._x);
	fprintf(stream, "%d\n", _yourself._zone.pos._position._y);
	fprintf(stream, "%d\n", _score);
	fprintf(stream, "%ld\n", _commandFlags);

	fprintf(stream, "%d\n", _numLocations);
	for (uint16 _si = 0; _si < _numLocations; _si++) {
		fprintf(stream, "%s\n%ld\n", _locationNames[_si], _localFlags[_si]);
	}

	for (uint16 _si = 0; _si < 30; _si++) {
		fprintf(stream, "%ld\n%d\n", _inventory[_si]._id, _inventory[_si]._index);
	}

	fclose(stream);

	refreshInventory();

	return;
}
#endif



enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT',
	kChooseCmd = 'CHOS'
};



SaveLoadChooser::SaveLoadChooser(const String &title, const String &buttonLabel, Parallaction *engine)
	: Dialog("scummsaveload"), _list(0), _chooseButton(0), _gfxWidget(0), _vm(engine) {

//	_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;

	new GUI::StaticTextWidget(this, "scummsaveload_title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "scummsaveload_list");
	_list->setEditable(true);
	_list->setNumberingMode(GUI::kListNumberingOne);

	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
	_container->setHints(GUI::THEME_HINT_USE_SHADOW);

	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);

	_date = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No date saved", GUI::kTextAlignCenter);
	_time = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No time saved", GUI::kTextAlignCenter);
	_playtime = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No playtime saved", GUI::kTextAlignCenter);

	// Buttons
	new GUI::ButtonWidget(this, "scummsaveload_cancel", "Cancel", GUI::kCloseCmd, 0);
	_chooseButton = new GUI::ButtonWidget(this, "scummsaveload_choose", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);
}

SaveLoadChooser::~SaveLoadChooser() {
}

const Common::String &SaveLoadChooser::getResultString() const {
	return _list->getSelectedString();
}

void SaveLoadChooser::setList(const StringList& list) {
	_list->setList(list);
}

int SaveLoadChooser::runModal() {
	if (_gfxWidget)
		_gfxWidget->setGfx(0);
	int ret = GUI::Dialog::runModal();
	return ret;
}

void SaveLoadChooser::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();
	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0) {
			if (!getResultString().empty()) {
				_list->endEditMode();
				setResult(selItem);
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		setResult(selItem);
		close();
		break;
	case GUI::kListSelectionChangedCmd: {
		if (_gfxWidget) {
			updateInfos();
		}

		_list->startEditMode();
		// Disable button if nothing is selected, or (in load mode) if an empty
		// list item is selected. We allow choosing an empty item in save mode
		// because we then just assign a default name.
		_chooseButton->setEnabled(selItem >= 0 && (!getResultString().empty()));
		_chooseButton->draw();
	} break;
	case GUI::kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooser::reflowLayout() {
	_container->setFlags(GUI::WIDGET_INVISIBLE);
	_gfxWidget->setFlags(GUI::WIDGET_INVISIBLE);
	_date->setFlags(GUI::WIDGET_INVISIBLE);
	_time->setFlags(GUI::WIDGET_INVISIBLE);
	_playtime->setFlags(GUI::WIDGET_INVISIBLE);

	Dialog::reflowLayout();
}

void SaveLoadChooser::updateInfos() {
/*	int selItem = _list->getSelected();
	Graphics::Surface *thumb;
	thumb = _vm->loadThumbnailFromSlot(selItem + 1);

	if (thumb) {
		_gfxWidget->setGfx(thumb);
		_gfxWidget->useAlpha(256);
		thumb->free();
	} else {
		_gfxWidget->setGfx(-1, -1, _fillR, _fillG, _fillB);
	}

	delete thumb;
	_gfxWidget->draw();

	InfoStuff infos;
	memset(&infos, 0, sizeof(InfoStuff));
	char buffer[32];
	if (_vm->loadInfosFromSlot(selItem + 1, &infos)) {
		snprintf(buffer, 32, "Date: %.2d.%.2d.%.4d",
			(infos.date >> 24) & 0xFF, (infos.date >> 16) & 0xFF,
			infos.date & 0xFFFF);
		_date->setLabel(buffer);
		_date->draw();

		snprintf(buffer, 32, "Time: %.2d:%.2d",
			(infos.time >> 8) & 0xFF, infos.time & 0xFF);
		_time->setLabel(buffer);
		_time->draw();

		int minutes = infos.playtime / 60;
		int hours = minutes / 60;
		minutes %= 60;

		snprintf(buffer, 32, "Playtime: %.2d:%.2d",
			hours & 0xFF, minutes & 0xFF);
		_playtime->setLabel(buffer);
		_playtime->draw();
	} else {
		snprintf(buffer, 32, "No date saved");
		_date->setLabel(buffer);
		_date->draw();

		snprintf(buffer, 32, "No time saved");
		_time->setLabel(buffer);
		_time->draw();

		snprintf(buffer, 32, "No playtime saved");
		_playtime->setLabel(buffer);
		_playtime->draw();
	}
*/
}

int Parallaction::buildSaveFileList(Common::StringList& l) {

	char name[16];
	char buf[30];

	int count = 0;

	for (int i = 0; i < 10; i++) {
		sprintf(name, "game.%i", i);

		buf[0] = '\0';
		Common::InSaveFile *f = _saveFileMan->openForLoading(name);

		if (f) {
			f->readLine(buf, 29);
			delete f;

			count++;
		}

		l.push_back(buf);
	}

	return count;
}


int Parallaction::selectSaveFile(uint16 arg_0, const char* caption, const char* button) {

	SaveLoadChooser* slc = new SaveLoadChooser(caption, button, _vm);

	Common::StringList l;

	/*int count = */ buildSaveFileList(l);
	slc->setList(l);

	int idx = slc->runModal();
	if (idx >= 0) {
		_saveFileName = slc->getResultString();
	}

	delete slc;

	return idx;
}



void Parallaction::loadGame() {

	int _di = selectSaveFile( 0, "Load file", "Load" );
	if (_di == -1) {
		return;
	}

	doLoadGame(_di);

	GUI::TimedMessageDialog dialog("Loading game...", 1500);
	dialog.runModal();

	changeCursor(kCursorArrow);

	return;
}


void Parallaction::saveGame() {

//	strcpy(v30, asc_1C91A);

	if (!scumm_stricmp(_location, "caveau"))
		return;

	int slot = selectSaveFile( 1, "Save file", "Save" );
	if (slot == -1) {
		return;
	}

	doSaveGame(slot, _saveFileName.c_str());

	GUI::TimedMessageDialog dialog("Saving game...", 1500);
	dialog.runModal();

	return;


}





} // namespace Parallaction
