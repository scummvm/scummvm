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

#ifndef FULLPIPE_SCENE_H
#define FULLPIPE_SCENE_H

namespace Fullpipe {

class SceneTag : public CObject {
 public:
	int _field_4;
	char *_tag;
	int _scene;
	int16 _sceneId;
	int16 _field_12;

 public:
	SceneTag();
	~SceneTag();

	virtual bool load(MfcArchive &file);
};

class SceneTagList : public Common::List<SceneTag>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class Scene {
	Background bg;
	CPtrList staticANIObjectList1;
	CPtrList staticANIObjectList2;
	CPtrList messageQueueList;
	CPtrList faObjectList;
	Shadows *shadows;
	int soundList;
	int16 sceneId;
	int stringObj;
	int field_BC;
	int libHandle;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SCENE_H */
