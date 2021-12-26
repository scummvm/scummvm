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

#ifndef AGS_PLUGINS_SERIALIZER_H
#define AGS_PLUGINS_SERIALIZER_H

#include "ags/shared/api/stream_api.h"
#include "ags/plugins/ags_plugin.h"
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

	bool isLoading() const {
		return _isLoading;
	}
	bool isSaving() const {
		return !_isLoading;
	}

	template<typename T>
	void syncAsInt(T &value) {
		byte buf[4];
		if (_isLoading) {
			_engine->FRead(buf, 4, _file);
			value = READ_LE_INT32(buf);
		} else {
			WRITE_LE_UINT32(buf, value);
			_engine->FWrite(buf, 4, _file);
		}
	}

	void syncAsBool(bool &value) {
		if (_isLoading)
			_engine->FRead(&value, 1, _file);
		else
			_engine->FWrite(&value, 1, _file);
	}

	void syncAsInt8(int8 &value) {
		if (_isLoading)
			_engine->FRead(&value, 1, _file);
		else
			_engine->FWrite(&value, 1, _file);
	}

	void syncAsByte(byte &value) {
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

	void syncAsDouble(double &value) {
		if (_isLoading)
			_engine->FRead(&value, sizeof(double), _file);
		else
			_engine->FWrite(&value, sizeof(double), _file);
	}

	void unreadInt() {
		_engine->FSeek(-4, AGS::Shared::kSeekCurrent, _file);
	}
};

} // namespace Plugins
} // namespace AGS3

#endif
