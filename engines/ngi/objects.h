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

#ifndef NGI_OBJECTS_H
#define NGI_OBJECTS_H

#include "common/ptr.h"
#include "ngi/utils.h"

namespace NGI {

class MessageQueue;
class SceneTagList;

class GameProject : public CObject {
 public:
	int _field_4;
	Common::String _headerFilename;
	Common::ScopedPtr<SceneTagList> _sceneTagList;
	int _field_10;

 public:
	GameProject();
	bool load(MfcArchive &file) override;
};

struct PicAniInfo {
	uint32 type;
	int16 objectId;
	int16 field_6;
	int32 field_8;
	int16 sceneId;
	int16 field_E;
	int32 ox;
	int32 oy;
	int32 priority;
	int16 staticsId;
	int16 movementId;
	int16 dynamicPhaseIndex;
	int16 flags;
	int32 field_24;
	int32 someDynamicPhaseIndex;

	bool load(MfcArchive &file);
	void save(MfcArchive &file);
	void print();

	PicAniInfo() { memset(this, 0, sizeof(PicAniInfo)); }
};
typedef Common::Array<PicAniInfo> PicAniInfoList;

union VarValue {
	float floatValue;
	int32 intValue;
	char *stringValue;
};

class GameVar : public CObject {
 public:
	GameVar *_nextVarObj;
	GameVar *_prevVarObj;
	GameVar *_parentVarObj;
	GameVar *_subVars;
	GameVar *_field_14;
	Common::String _varName;
	VarValue _value;
	int _varType;

 public:
	GameVar();
	~GameVar() override;

	bool load(MfcArchive &file) override;
	void save(MfcArchive &file) override;
	GameVar *getSubVarByName(const Common::String &name);
	bool setSubVarAsInt(const Common::String &name, int value);
	int getSubVarAsInt(const Common::String &name);
	GameVar *addSubVarAsInt(const Common::String &name, int value);
	bool addSubVar(GameVar *subvar);
	int getSubVarsCount();
	GameVar *getSubVarByIndex(int idx);
};

} // End of namespace NGI

#endif /* NGI_OBJECTS_H */
