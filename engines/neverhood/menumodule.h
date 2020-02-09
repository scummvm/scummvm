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

#ifndef NEVERHOOD_MENUMODULE_H
#define NEVERHOOD_MENUMODULE_H

#include "common/str.h"
#include "common/str-array.h"
#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

struct SavegameItem {
	int slotNum;
	Common::String description;
};

typedef Common::Array<SavegameItem> SavegameList;

class MenuModule : public Module {
public:
	MenuModule(NeverhoodEngine *vm, Module *parentModule, int which);
	~MenuModule() override;
	void setLoadgameInfo(uint index);
	void setLoadgameSlot(int slot);
	void setSavegameInfo(const Common::String &description, uint index, bool newSavegame);
	void setDeletegameInfo(uint index);
	void refreshSaveGameList();
protected:
	int _sceneNum;
	byte *_savedPaletteData;
	SavegameList *_savegameList;
	Common::String _savegameDescription;
	int _savegameSlot;
	void createScene(int sceneNum, int which);
	void updateScene();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createLoadGameMenu();
	void createSaveGameMenu();
	void createDeleteGameMenu();
	void handleLoadGameMenuAction(bool doLoad);
	void handleSaveGameMenuAction(bool doSave, bool doQuery);
	void handleDeleteGameMenuAction(bool doDelete);
	void loadSavegameList();
};

class MenuButton : public StaticSprite {
public:
	MenuButton(NeverhoodEngine *vm, Scene *parentScene, uint buttonIndex, uint32 fileHash, const NRect &collisionBounds);
protected:
	Scene *_parentScene;
	int _countdown;
	uint _buttonIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class MainMenu : public Scene {
public:
	MainMenu(NeverhoodEngine *vm, Module *parentModule);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class CreditsScene : public Scene {
public:
	CreditsScene(NeverhoodEngine *vm, Module *parentModule, bool canAbort);
	~CreditsScene() override;
protected:
	int _screenIndex;
	int _countdown;
	MusicResource *_musicResource;
	uint32 _ticksTime;
	uint32 _ticksDuration;
	bool _canAbort;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Widget;
class GameStateMenu;

class Widget : public StaticSprite {
public:
	Widget(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
		int baseObjectPriority, int baseSurfacePriority);
	virtual void onClick();
	virtual void setPosition(int16 x, int16 y);
	virtual void refreshPosition();
	virtual void initialize();
	virtual int16 getWidth();
	virtual int16 getHeight();
	virtual void enterWidget();
	virtual void exitWidget();
protected:
	GameStateMenu *_parentScene;
	int _baseObjectPriority;
	int _baseSurfacePriority;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class TextLabelWidget : public Widget {
public:
	TextLabelWidget(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
		int baseObjectPriority, int baseSurfacePriority,
		const byte *string, int stringLen, BaseSurface *drawSurface, int16 tx, int16 ty, FontSurface *fontSurface);
	void initialize() override;
	int16 getWidth() override;
	int16 getHeight() override;
	void drawString(int maxStringLength);
	void clear();
	void setString(const byte *string, int stringLen);
	FontSurface *getFontSurface() const { return _fontSurface; }
protected:
	BaseSurface *_drawSurface;
	int16 _tx, _ty;
	FontSurface *_fontSurface;
	const byte *_string;
	int _stringLen;
};

class TextEditWidget : public Widget {
public:
	TextEditWidget(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
		int maxStringLength, FontSurface *fontSurface, uint32 fileHash, const NRect &rect);
	~TextEditWidget() override;
	void onClick() override;
	void initialize() override;
	void enterWidget() override;
	void exitWidget() override;
	void setCursor(uint32 cursorFileHash, int16 cursorWidth, int16 cursorHeight);
	void drawCursor();
	void updateString();
	Common::String& getString();
	void setString(const Common::String &string);
	void handleAsciiKey(char ch);
	void handleKeyDown(Common::KeyCode keyCode);
	void refresh();
	void setReadOnly(bool value) { _readOnly = value; }
	bool isReadOnly() const { return _readOnly; }
	bool isModified() const { return _modified; }
protected:
	NRect _rect;
	uint32 _fileHash;
	int _maxVisibleChars;
	int _maxStringLength;
	int _cursorPos;
	int _cursorTicks;
	Common::String _entryString;
	FontSurface *_fontSurface;
	TextLabelWidget *_textLabelWidget;
	BaseSurface *_cursorSurface;
	uint32 _cursorFileHash;
	int16 _cursorWidth, _cursorHeight;
	bool _modified;
	bool _readOnly;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SavegameListBox : public Widget {
public:
	SavegameListBox(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
		SavegameList *savegameList, FontSurface *fontSurface, uint32 bgFileHash, const NRect &rect);
	void onClick() override;
	void initialize() override;
	void buildItems();
	void drawItems();
	void refresh();
	void scrollUp();
	void scrollDown();
	void pageUp();
	void pageDown();
	uint getCurrIndex() const { return _currIndex; }
protected:
	const NRect _rect;
	uint32 _bgFileHash;
	int _maxStringLength;
	Common::Array<TextLabelWidget*> _textLabelItems;
	int _firstVisibleItem;
	int _lastVisibleItem;
	SavegameList *_savegameList;
	FontSurface *_fontSurface;
	uint _currIndex;
	int _maxVisibleItemsCount;
};

class GameStateMenu : public Scene {
public:
	GameStateMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList,
		const uint32 *buttonFileHashes, const NRect *buttonCollisionBounds,
		uint32 backgroundFileHash, uint32 fontFileHash,
		uint32 mouseFileHash, const NRect *mouseRect,
		uint32 listBoxBackgroundFileHash, int16 listBoxX, int16 listBoxY, const NRect &listBoxRect,
		uint32 textEditBackgroundFileHash, uint32 textEditCursorFileHash, int16 textEditX, int16 textEditY, const NRect &textEditRect,
		uint32 textFileHash1, uint32 textFileHash2);
	~GameStateMenu() override;
	NPoint getMousePos();
	virtual void setCurrWidget(Widget *newWidget);
	virtual Widget *getCurrWidget() { return _currWidget; }
	virtual void refreshDescriptionEdit();
protected:
	Widget *_currWidget;
	SavegameList *_savegameList;
	FontSurface *_fontSurface;
	SavegameListBox *_listBox;
	TextEditWidget *_textEditWidget;
	Common::String _savegameDescription;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	virtual void performAction();
	int scummVMSaveLoadDialog(bool isSave, Common::String &saveDesc);
};

class SaveGameMenu : public GameStateMenu {
public:
	SaveGameMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList);
protected:
	void performAction() override;
};

class LoadGameMenu : public GameStateMenu {
public:
	LoadGameMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList);
protected:
	void performAction() override;
};

class DeleteGameMenu : public GameStateMenu {
public:
	DeleteGameMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList);
protected:
	void performAction() override;
};

class QueryOverwriteMenu : public Scene {
public:
	QueryOverwriteMenu(NeverhoodEngine *vm, Module *parentModule, const Common::String &description);
protected:
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MENUMODULE_H */
