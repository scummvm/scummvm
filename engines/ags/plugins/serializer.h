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

#ifndef AGS_PLUGINS_SERIALIZER_H
#define AGS_PLUGINS_SERIALIZER_H

#include "ags/plugins/agsplugin.h"
#include "common/serializer.h"

namespace AGS3 {
namespace Plugins {

class Serializer {
private:
	IAGSEngine *_engine;
	long _file;
	bool _isLoading;
public:
	Serializer(IAGSEngine *engine, long file, bool isLoading) :
		_engine(engine), _file(file), _isLoading(isLoading) {}

	bool isLoading() const { return _isLoading; }
	bool isSaving() const { return !_isLoading; }

	void syncAsInt(int &value) {
		byte buf[4];
		if (_isLoading) {
			_engine->FRead(buf, sizeof(int32), _file);
			value = READ_LE_INT32(buf);
		} else {
			WRITE_LE_UINT32(buf, value);
			_engine->FWrite(buf, sizeof(int32), _file);
		}
	}

	void syncAsBool(bool &value) {
		if (_isLoading)
			_engine->FRead(&value, 1, _file);
		else
			_engine->FWrite(&value, 1, _file);
	}

	void syncAsFloat(float &value) {
		if (_isLoading)
			_engine->FRead(&value, sizeof(float), _file);
		else
			_engine->FWrite(&value, sizeof(float), _file);
	}
};

} // namespace Plugins
} // namespace AGS3

#endif
