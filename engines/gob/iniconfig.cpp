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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "common/memstream.h"

#include "gob/iniconfig.h"
#include "gob/save/saveload.h"

namespace Gob {

INIConfig::INIConfig(GobEngine *vm) : _vm(vm) {
}

INIConfig::~INIConfig() {
	for (ConfigMap::iterator c = _configs.begin(); c != _configs.end(); ++c)
		delete c->_value.config;
}

bool INIConfig::getValue(Common::String &result, const Common::String &file,
		const Common::String &section, const Common::String &key,
		const Common::String &def) {

	Config config;
	if (!getConfig(file, config)) {
		if (!openConfig(file, config)) {
			result = def;
			return false;
		}
	}

	if (!config.config->getKey(key, section, result)) {
		result = def;
		return false;
	}

	return true;
}

bool INIConfig::setValue(const Common::String &file, const Common::String &section,
		const Common::String &key, const Common::String &value) {

	Config config;
	if (!getConfig(file, config))
		if (!createConfig(file, config))
			return false;

	config.config->setKey(key, section, value);
	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file.c_str());
	if (mode == SaveLoad::kSaveModeSave) {
		// Sync changes to save file
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		config.config->saveToStream(stream);

		_vm->_saveLoad->saveFromRaw(file.c_str(), stream.getData(), stream.size(), 0);
	}

	return true;
}

bool INIConfig::getConfig(const Common::String &file, Config &config) {
	if (!_configs.contains(file))
		return false;

	config = _configs.getVal(file);
	return true;
}

bool INIConfig::readConfigFromDisk(const Common::String &file, Gob::INIConfig::Config &config) {
	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file.c_str());
	if (mode == SaveLoad::kSaveModeSave) {
		debugC(3, kDebugFileIO, "Loading INI from save file \"%s\"", file.c_str());
		// Read from save file
		int size = _vm->_saveLoad->getSize(file.c_str());
		if (size < 0) {
			debugC(3, kDebugFileIO, "Failed to get size of save file \"%s\"", file.c_str());
			return false;
		}

		byte *data = new byte[size];
		_vm->_saveLoad->loadToRaw(file.c_str(), data, size, 0);
		Common::MemoryReadStream stream(data, size, DisposeAfterUse::YES);
		if (!config.config->loadFromStream(stream)) {
			debugC(3, kDebugFileIO, "Failed to load INI from save file \"%s\"", file.c_str());
			return false;
		}
	} else {
		// GOB uses \ as a path separator but
		// it almost always manipulates base names
		debugC(3, kDebugFileIO, "Loading INI from plain file \"%s\"", file.c_str());

		if (!config.config->loadFromFile(Common::Path(file, '\\'))) {
			return false;
		}
	}

	return true;
}


bool INIConfig::openConfig(const Common::String &file, Config &config) {
	config.config  = new Common::INIFile();
	config.config->allowNonEnglishCharacters();
	config.created = false;

	if (!readConfigFromDisk(file, config)) {
		delete config.config;
		config.config = nullptr;
		return false;
	}


	_configs.setVal(file, config);

	return true;
}

bool INIConfig::createConfig(const Common::String &file, Config &config) {
	config.config  = new Common::INIFile();
	config.config->allowNonEnglishCharacters();
	config.created = true;

	readConfigFromDisk(file, config); // May return false in case we are dealing with a temporary file

	_configs.setVal(file, config);

	return true;
}

} // End of namespace Gob
