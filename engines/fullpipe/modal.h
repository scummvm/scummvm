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

class BaseModalObject {
 public:

	BaseModalObject *_parentObj;

 public:
 	BaseModalObject() : _parentObj(0) {}
	virtual ~BaseModalObject() {}

	virtual bool handleMessage(ExCommand *message);
	virtual bool init(int counterdiff);
	virtual bool update();

	void saveload();
};

class ModalIntro : public BaseModalObject {
	int _field_8;
	int _introFlags;
	int _countDown;
	int _needRedraw;
	int _sfxVolume;

 public:
	ModalIntro();

	virtual bool handleMessage(ExCommand *message);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_MODAL_H */
