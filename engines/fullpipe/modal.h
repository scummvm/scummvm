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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef FULLPIPE_MODAL_H
#define FULLPIPE_MODAL_H

namespace Fullpipe {

class PictureObject;

class BaseModalObject {
 public:

	BaseModalObject *_parentObj;

 public:
 	BaseModalObject() : _parentObj(0) {}
	virtual ~BaseModalObject() {}


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
	virtual ~ModalIntro();

	virtual bool pollEvent() { return true; }
	virtual bool handleMessage(ExCommand *message);
	virtual bool init(int counterdiff);
	virtual void update();
	virtual void saveload() {}

	void finish();
};

class ModalVideoPlayer : public BaseModalObject {
public:

	virtual bool pollEvent() { return true; }
	virtual bool handleMessage(ExCommand *message) { return true; }
	virtual bool init(int counterdiff) { return false; }
	virtual void update() {}
	virtual void saveload() {}

	void play(const char *fname);
};

class ModalMap : public BaseModalObject {
	Scene *_mapScene;
	PictureObject *_pic;
	bool _isRunning;
	Common::Rect _rect1;
	int _x;
	int _y;
	int _flag;
	int _mouseX;
	int _mouseY;
	int _field_38;
	int _field_3C;
	int _field_40;
	Common::Rect _rect2;

 public:
	ModalMap();
	virtual ~ModalMap();

	virtual bool pollEvent() { return true; }
	virtual bool handleMessage(ExCommand *message);
	virtual bool init(int counterdiff);
	virtual void update();
	virtual void saveload() {}

	void initMap();
	PictureObject *getScenePicture();
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_MODAL_H */
