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

#ifndef NGI_MODAL_H
#define NGI_MODAL_H

#include "video/avi_decoder.h"

namespace NGI {

class PictureObject;
class Picture;
class Sound;

struct FileInfo {
	char filename[260];
	bool empty;
	char date[16];
	int fx1;
	int fx2;
	int fy1;
	int fy2;
};

class BaseModalObject {
 public:

	BaseModalObject *_parentObj;
	ObjType _objtype;

 public:
	BaseModalObject() : _parentObj(0) { _objtype = kObjTypeDefault; }
	virtual ~BaseModalObject() {}

	void deleteObject();

	virtual bool pollEvent() = 0;
	virtual bool handleMessage(ExCommand *message) = 0;
	virtual bool init(int counterdiff) = 0;
	virtual void update() = 0;

	virtual void saveload() = 0;
};

class ModalIntro : public BaseModalObject {
	int _field_8;
	int _introFlags;
	int _countDown;
	int _stillRunning;
	int _sfxVolume;

 public:
	ModalIntro();
	~ModalIntro() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}

	void finish();
};

class ModalIntroDemo : public BaseModalObject {
	int _field_8;
	int _introFlags;
	int _countDown;
	int _stillRunning;
	int _sfxVolume;

 public:
	ModalIntroDemo();
	~ModalIntroDemo() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}

	void finish();
};

class ModalVideoPlayer : public BaseModalObject {
public:

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override { return true; }
	bool init(int counterdiff) override { return false; }
	void update() override {}
	void saveload() override {}

	void play(const char *fname);

private:
	Video::AVIDecoder _decoder;
};

class ModalMap : public BaseModalObject {
	Scene *_mapScene;
	PictureObject *_pic;
	PictureObject *_picI03;
	PictureObject *_highlightedPic;
	bool _isRunning;
	Common::Rect _rect1;
	int _x;
	int _y;
	int _flag;
	int _mouseX;
	int _mouseY;
	int _dragX;
	int _dragY;
	int _hotSpotDelay;
	Common::Rect _rect2;

 public:
	ModalMap();
	~ModalMap() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	virtual bool init2(int counterdiff);
	void update() override;
	void saveload() override {}

	void initMap();

private:
	PictureObject *getScenePicture(int sceneId);
	PictureObject *getSceneHPicture(PictureObject *obj);
	bool checkScenePass(PreloadItem *item);
	bool isSceneEnabled(int sceneId);

	int findMapSceneId(int picId);
	void clickButton(PictureObject *pic);
};

class ModalFinal : public BaseModalObject {
	int _flags;
	int _counter;
	int _sfxVolume;

 public:
	ModalFinal();
	~ModalFinal() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}

	void unloadScenes();
};

class ModalCredits : public BaseModalObject {
	Scene *_sceneTitles;
	PictureObject *_creditsPic;
	bool _fadeIn;
	bool _fadeOut;
	int _countdown;
	int _sfxVolume;
	int _currX;
	int _currY;
	int _maxY;

 public:
	ModalCredits();
	~ModalCredits() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}
};

struct MenuArea {
	int picIdL;
	PictureObject *picObjD;
	PictureObject *picObjL;
};

class ModalMainMenu : public BaseModalObject {
public:
	Scene *_scene;
	int _hoverAreaId;
	Common::Array<MenuArea> _areas;
	int _menuSliderIdx;
	int _musicSliderIdx;
	MenuArea *_lastArea;
	int _sliderOffset;
	int _mfield_34;
	Common::Rect _screct;
	int _bgX;
	int _bgY;
	int _debugKeyCount;

public:
	ModalMainMenu();
	~ModalMainMenu() override {}

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}

private:
	bool isSaveAllowed();
	void enableDebugMenuButton();
	void setSliderPos();
	void enableDebugMenu(char c);
	int checkHover(Common::Point &point);
	void updateVolume();
	void updateSoundVolume(Sound &snd);
	void updateSliderPos();
	bool isOverArea(PictureObject *obj, Common::Point *point);
};

class ModalHelp : public BaseModalObject {
public:
	Scene *_mainMenuScene;
	Picture *_bg;
	bool _isRunning;
	Common::Rect _rect;
	int _hx;
	int _hy;

public:
	ModalHelp();
	~ModalHelp() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}

	void launch();
};

class ModalQuery : public BaseModalObject {
public:
	ModalQuery();
	~ModalQuery() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override {}

	bool create(Scene *sc, Scene *bgScene, int picId);
	int getQueryResult() { return _queryResult; }


private:
	Scene *_bgScene;
	PictureObject *_bg;
	PictureObject *_okBtn;
	PictureObject *_cancelBtn;
	int _queryResult;

};

class ModalSaveGame : public BaseModalObject {
public:
	ModalSaveGame();
	~ModalSaveGame() override;

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	void update() override;
	void saveload() override;

	void processMouse(int x, int y);

	void setScene(Scene *sc);
	void setup(Scene *sc, int mode);
	void processKey(int key);

	char *getSaveName();
	bool getFileInfo(int slot, FileInfo *fileinfo);

	Common::Rect _rect;
	int _oldBgX;
	int _oldBgY;
	PictureObject *_bgr;
	PictureObject *_okD;
	PictureObject *_okL;
	PictureObject *_cancelD;
	PictureObject *_cancelL;
	PictureObject *_emptyD;
	PictureObject *_emptyL;
	PictureObject *_fullD;
	PictureObject *_fullL;
	Scene *_menuScene;
	int _mode;
	ModalQuery *_queryDlg;
	Common::Array <FileInfo> _files;
	Common::Array <PictureObject *> _arrayL;
	Common::Array <PictureObject *> _arrayD;
	int _queryRes;
};

class ModalDemo : public BaseModalObject {
	PictureObject *_bg;
	PictureObject *_button;
	PictureObject *_text;
	int _clickedQuit;
	int _countdown;
	Scene *_scene;

 public:
	ModalDemo();
	~ModalDemo() override;

	bool launch();

	bool pollEvent() override { return true; }
	bool handleMessage(ExCommand *message) override;
	bool init(int counterdiff) override;
	bool init2(int counterdiff);
	void update() override;
	void saveload() override {}
};


} // End of namespace NGI

#endif /* NGI_MODAL_H */
