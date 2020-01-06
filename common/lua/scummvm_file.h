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

#ifndef LUA_SCUMMVM_FILE_H
#define LUA_SCUMMVM_FILE_H

#include "common/str.h"
#include "common/file.h"

namespace Lua {

class LuaFileProxy {
public:
	static LuaFileProxy *create(const Common::String &filename, const Common::String &mode);
public:
	virtual ~LuaFileProxy() {}
	virtual bool eof() const = 0;
	virtual size_t read(void *ptr, size_t size, size_t count) = 0;
	virtual size_t write(const char *ptr, size_t count) = 0;
};

/**
 * The following class acts as a proxy interface to the I/O code, pretending that the ScummVM
 * settings are a properly formatted 'config.lua' file
 */
class LuaFileConfig : public LuaFileProxy {
	friend class LuaFileProxy;
private:
	Common::String _readData;
	uint _readPos;
	Common::String _settings;

	Common::String formatDouble(double value);
	void setupConfigFile();
	Common::String getLanguage();
	void setLanguage(const Common::String &lang);
	void writeSettings();
	void updateSetting(const Common::String &setting, const Common::String &value);

	LuaFileConfig(const Common::String &filename, const Common::String &mode);
public:
	virtual ~LuaFileConfig();

	virtual bool eof() const override { return _readPos >= _readData.size(); }
	virtual size_t read(void *ptr, size_t size, size_t count) override;
	virtual size_t write(const char *ptr, size_t count) override;
};

class LuaFileRead : public LuaFileProxy {
private:
	Common::File _file;
	int32 _size;
public:
	LuaFileRead(const Common::String &filename, const Common::String &mode);
public:
	virtual ~LuaFileRead() {}

	virtual bool eof() const override;
	virtual size_t read(void *ptr, size_t size, size_t count) override;
	virtual size_t write(const char *ptr, size_t count) override;
};

} // End of namespace Lua

#endif
