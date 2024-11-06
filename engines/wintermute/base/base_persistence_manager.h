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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_PERSISTENCE_MANAGER_H
#define WINTERMUTE_BASE_PERSISTENCE_MANAGER_H


#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/savestate.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/rect.h"

#ifdef ENABLE_WME3D
namespace Math {

class Angle;
template<int rows, int cols> class Matrix;
typedef Matrix<4, 4> Matrix4;
typedef Matrix<3, 1> Vector3d;

} // namespace Math
#endif

namespace Wintermute {

class Vector2;
class BaseGame;
class BasePersistenceManager {
public:
	char *_savedDescription;
	Common::String _savePrefix;
	Common::String _savedName;
	bool saveFile(const Common::String &filename);
	uint32 getDWORD();
	void putDWORD(uint32 val);
	char *getString();
	Common::String getStringObj();
	void putString(const char *val);
	float getFloat();
	void putFloat(float val);
	double getDouble();
	void putDouble(double val);
	void cleanup();
	void getSaveStateDesc(int slot, SaveStateDescriptor &desc);
	bool deleteSaveSlot(int slot);
	uint32 getMaxUsedSlot();
	bool getSaveExists(int slot);
	bool initLoad(const Common::String &filename);
	bool initSave(const Common::String &desc);
	bool getBytes(byte *buffer, uint32 size);
	bool putBytes(byte *buffer, uint32 size);
	uint32 _offset;

	bool getIsSaving() { return _saving; }
	TimeDate getSavedTimestamp() { return _savedTimestamp; }

	uint32 _richBufferSize;
	byte *_richBuffer;

	bool transferPtr(const char *name, void *val);
	bool transferSint32(const char *name, int32 *val);
	bool transferUint32(const char *name, uint32 *val);
	bool transferFloat(const char *name, float *val);
	bool transferDouble(const char *name, double *val);
	bool transferBool(const char *name, bool *val);
	bool transferByte(const char *name, byte *val);
	bool transferRect32(const char *name, Rect32 *val);
	bool transferPoint32(const char *name, Point32 *val);
	bool transferConstChar(const char *name, const char **val);
	bool transferCharPtr(const char *name, char **val);
	bool transferString(const char *name, Common::String *val);
	bool transferVector2(const char *name, Vector2 *val);
#ifdef ENABLE_WME3D
	bool transferVector3d(const char *name, DXVector3 *val);
	bool transferVector4d(const char *name, DXVector4 *val);
	bool transferMatrix4(const char *name, DXMatrix *val);
	bool transferAngle(const char *name, float *val);
#endif
	BasePersistenceManager(const Common::String &savePrefix = "", bool deleteSingleton = false);
	virtual ~BasePersistenceManager();
	bool checkVersion(byte  verMajor, byte verMinor, byte verBuild);

	uint32 _thumbnailDataSize;
	byte *_thumbnailData;
	uint32 _scummVMThumbSize;
	byte *_scummVMThumbnailData;
	Common::String getFilenameForSlot(int slot) const;
private:
	bool _deleteSingleton;
	bool readHeader(const Common::String &filename);
	TimeDate getTimeDate();
	bool putTimeDate(const TimeDate &t);
	Common::WriteStream *_saveStream;
	Common::SeekableReadStream *_loadStream;
	TimeDate _savedTimestamp;
	uint32 _savedPlayTime;
	byte _savedVerMajor;
	byte _savedVerMinor;
	byte _savedVerBuild;
	byte _savedExtMajor;
	byte _savedExtMinor;
	bool _saving;
	// Separate from Base, as this class can do SOME operations without a _gameRef.
	BaseGame *_gameRef;
};

} // End of namespace Wintermute

#endif
