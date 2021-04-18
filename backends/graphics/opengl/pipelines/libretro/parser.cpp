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

#include "backends/graphics/opengl/opengl-sys.h"

#if !USE_FORCED_GLES
#include "backends/graphics/opengl/pipelines/libretro/parser.h"

#include "common/fs.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/algorithm.h"
#include "common/tokenizer.h"
#include "common/ptr.h"
#include "common/util.h"

#include "common/textconsole.h"

#include <limits.h>

namespace OpenGL {
namespace LibRetro {

class PresetParser {
public:
	ShaderPreset *parseStream(Common::SeekableReadStream &stream);

	const Common::String &getErrorDesc() const { return _errorDesc; }
private:
	bool parsePreset(Common::SeekableReadStream &stream);
	bool lookUpValue(const Common::String &key, Common::String *value);
	bool lookUpValue(const Common::String &key, bool *value);
	bool lookUpValue(const Common::String &key, uint *value);
	bool lookUpValue(const Common::String &key, float *value);
	bool lookUpValue(const Common::String &key, FilteringMode *value, const FilteringMode defaultValue);
	bool lookUpValue(const Common::String &key, ScaleType *value, const ScaleType defaultValue);
	bool lookUpValueScale(const Common::String &key, float *floatValue, uint *uintValue, const ScaleType scaleType);

	template<typename T, typename DefaultT>
	bool lookUpValue(const Common::String &key, T *value, const DefaultT &defaultValue) {
		if (_entries.contains(key)) {
			return lookUpValue(key, value);
		} else {
			*value = defaultValue;
			return true;
		}
	}


	bool parseTextures();
	bool parseTexture(const Common::String &id);

	bool parsePasses();
	bool parsePass(const uint id, const bool isLast);
	bool parsePassScaleType(const uint id, const bool isLast, ShaderPass *pass);
	bool parsePassScale(const uint id, ShaderPass *pass);

	typedef Common::HashMap<Common::String, Common::String> StringMap;
	StringMap _entries;

	Common::String _errorDesc;

	Common::ScopedPtr<ShaderPreset> _shader;
};

ShaderPreset *PresetParser::parseStream(Common::SeekableReadStream &stream) {
	_errorDesc.clear();
	_entries.clear();

	if (!parsePreset(stream)) {
		return nullptr;
	}

	_shader.reset(new ShaderPreset);
	if (!parseTextures()) {
		return nullptr;
	}

	if (!parsePasses()) {
		return nullptr;
	}

	return _shader.release();
}

bool PresetParser::parsePreset(Common::SeekableReadStream &stream) {
	while (!stream.eos()) {
		Common::String line = stream.readLine();
		if (stream.err()) {
			_errorDesc = "Read error";
			return false;
		}

		if (line.empty()) {
			continue;
		}

		// Split line into key, value pair.
		// TODO: Files can contain comments starting with '#', we need to
		// handle this.
		Common::String::const_iterator equalIter = Common::find(line.begin(), line.end(), '=');
		if (equalIter == line.end()) {
			_errorDesc = "Could not find '=' in line '" + line + '\'';
			return false;
		}

		Common::String key(line.begin(), equalIter);
		Common::String value(equalIter + 1);

		key.trim();
		value.trim();

		// Check whether the value is put in quotation marks. This might be
		// useful when a path contains a whitespace. But Libretro's is not
		// mentioning any exact format, but one example for 'textures'
		// indicates quotes are supported in this place.
		if (!value.empty() && value[0] == '"') {
			if (value.size() < 2 || value.lastChar() != '"') {
				_errorDesc = "Unmatched '\"' for value in line '" + line + '\'';
			}

			value.deleteLastChar();
			value.deleteChar(0);
		}

		_entries[key] = value;
	}

	return true;
}

bool PresetParser::lookUpValue(const Common::String &key, Common::String *value) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		*value = iter->_value;
		return true;
	} else {
		_errorDesc = "Missing key '" + key + "'";
		return false;
	}
}

bool PresetParser::lookUpValue(const Common::String &key, bool *value) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		if (Common::parseBool(iter->_value, *value)) {
			return true;
		} else {
			_errorDesc = "Invalid boolean value for key '" + key + "': '" + iter->_value + '\'';
			return false;
		}
	} else {
		_errorDesc = "Missing key '" + key + "'";
		return false;
	}
}

bool PresetParser::lookUpValue(const Common::String &key, uint *value) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		char *endptr;
		const long uintVal = strtol(iter->_value.c_str(), &endptr, 0);
		if (*endptr != '\0' || uintVal >= UINT_MAX || uintVal < 0) {
			_errorDesc = "Invalid unsigned integer value for key '" + key + "': '" + iter->_value + '\'';
			return false;
		} else {
			*value = uintVal;
			return true;
		}
	} else {
		_errorDesc = "Missing key '" + key + "'";
		return false;
	}
}

bool PresetParser::lookUpValue(const Common::String &key, float *value) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		char *endptr;
		const double doubleVal = strtod(iter->_value.c_str(), &endptr);
		if (*endptr != '\0') {
			_errorDesc = "Invalid float value for key '" + key + "': '" + iter->_value + '\'';
			return false;
		} else {
			*value = doubleVal;
			return true;
		}
	} else {
		_errorDesc = "Missing key '" + key + "'";
		return false;
	}
}

bool PresetParser::lookUpValue(const Common::String &key, FilteringMode *value, const FilteringMode defaultValue) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		bool isLinear;
		if (Common::parseBool(iter->_value, isLinear)) {
			*value = isLinear ? kFilteringModeLinear : kFilteringModeNearest;
			return true;
		} else {
			_errorDesc = "Invalid filtering mode for key '" + key + "': '" + iter->_value + '\'';
			return false;
		}
	} else {
		*value = defaultValue;
		return true;
	}
}

bool PresetParser::lookUpValue(const Common::String &key, ScaleType *value, const ScaleType defaultValue) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		if (iter->_value == "source") {
			*value = kScaleTypeSource;
			return true;
		} else if (iter->_value == "viewport") {
			*value = kScaleTypeViewport;
			return true;
		} else if (iter->_value == "absolute") {
			*value = kScaleTypeAbsolute;
			return true;
		} else {
			_errorDesc = "Invalid scale type for key '" + key + "': '" + iter->_value + '\'';
			return false;
		}
	} else {
		*value = defaultValue;
		return true;
	}
}

bool PresetParser::lookUpValueScale(const Common::String &key, float *floatValue, uint *uintValue, const ScaleType scaleType) {
	if (!_entries.contains(key)) {
		switch (scaleType) {
		case kScaleTypeSource:
		case kScaleTypeViewport:
			*floatValue = 1.0f;
			return true;

		case kScaleTypeAbsolute:
			_errorDesc = "No value specified for scale '" + key + '\'';
			return false;

		case kScaleTypeFull:
			return true;

		default:
			_errorDesc = "Internal Error: Invalid scale type";
			return false;
		}
	}

	switch (scaleType) {
	case kScaleTypeSource:
	case kScaleTypeViewport:
		return lookUpValue(key, floatValue, 1.0f);

	case kScaleTypeAbsolute:
		return lookUpValue(key, uintValue);

	case kScaleTypeFull:
		return true;

	default:
		_errorDesc = "Internal Error: Invalid scale type";
		return false;
	}
}

bool PresetParser::parseTextures() {
	Common::String textures;
	if (!lookUpValue("textures", &textures)) {
		return true;
	}

	// Parse all texture information from preset.
	Common::StringTokenizer tokenizer(textures, ";");
	while (!tokenizer.empty()) {
		if (!parseTexture(tokenizer.nextToken())) {
			return false;
		}
	}

	return true;
}

bool PresetParser::parseTexture(const Common::String &id) {
	Common::String fileName;
	if (!lookUpValue(id, &fileName)) {
		_errorDesc = "No file name specified for texture '" + id + '\'';
		return false;
	}

	FilteringMode filteringMode;
	if (!lookUpValue(id + "_linear", &filteringMode, kFilteringModeLinear)) {
		return false;
	}

	_shader->textures.push_back(ShaderTexture(id, fileName, filteringMode));
	return true;
}

bool PresetParser::parsePasses() {
	uint numShaders;
	if (!lookUpValue("shaders", &numShaders)) {
		return false;
	}

	for (uint shaderPass = 0; shaderPass < numShaders; ++shaderPass) {
		if (!parsePass(shaderPass, shaderPass == numShaders - 1)) {
			return false;
		}
	}

	return true;
}

#define passKey(x) Common::String::format(x "%u", id)
bool PresetParser::parsePass(const uint id, const bool isLast) {
	ShaderPass pass;

	if (!lookUpValue(passKey("shader"), &pass.fileName)) {
		_errorDesc = Common::String::format("No file name specified for pass '%u'", id);
		return false;
	}

	if (!lookUpValue(passKey("filter_linear"), &pass.filteringMode, kFilteringModeUnspecified)) {
		return false;
	}

	if (!lookUpValue(passKey("mipmap_input"), &pass.mipmapInput, false)) {
		return false;
	}

	if (!lookUpValue(passKey("float_framebuffer"), &pass.floatFBO, false)) {
		return false;
	}

	if (!lookUpValue(passKey("srgb_framebuffer"), &pass.srgbFBO, false)) {
		return false;
	}

	if (!lookUpValue(passKey("frame_count_mod"), &pass.frameCountMod, 0)) {
		return false;
	}

	if (!parsePassScaleType(id, isLast, &pass)) {
		return false;
	}

	if (!parsePassScale(id, &pass)) {
		return false;
	}

	_shader->passes.push_back(pass);
	return true;
}

bool PresetParser::parsePassScaleType(const uint id, const bool isLast, ShaderPass *pass) {
	// Parse scale type for the pass.
	//
	// This is a little more complicated because it is possible to specify the
	// scale type per axis. However, a generic scale type overrides the axis
	// scale types.
	//
	// Additionally, the default value for the passes vary. The last pass
	// defaults to use full size, all other default to source scaling.

	const ScaleType defaultScaleType = isLast ? kScaleTypeFull : kScaleTypeSource;

	if (!lookUpValue(passKey("scale_type_x"), &pass->scaleTypeX, defaultScaleType)) {
		return false;
	}

	if (!lookUpValue(passKey("scale_type_y"), &pass->scaleTypeY, defaultScaleType)) {
		return false;
	}

	if (!lookUpValue(passKey("scale_type"), &pass->scaleTypeX, defaultScaleType)) {
		return false;
	} else {
		pass->scaleTypeY = pass->scaleTypeX;
	}

	return true;
}

bool PresetParser::parsePassScale(const uint id, ShaderPass *pass) {
	// Parse actual scale value for the pass.
	//
	// Like for the scale type, 'scale' overrides 'scale_x'/'scale_y'.
	// However, in case the scale types for x/y are different the usage of
	// 'scale' leads to undefined behavior. In our case we simply reject
	// the shader preset.

	if (!lookUpValueScale(passKey("scale_x"), &pass->scaleXFloat, &pass->scaleXUint, pass->scaleTypeX)) {
		return false;
	}

	if (!lookUpValueScale(passKey("scale_y"), &pass->scaleYFloat, &pass->scaleYUint, pass->scaleTypeY)) {
		return false;
	}

	if (!_entries.contains(passKey("scale"))) {
		return true;
	}

	if (pass->scaleTypeX != pass->scaleTypeY) {
		_errorDesc = Common::String::format("Pass %u: Scale types for x/y differ but 'scale%u' defined", id, id);
		return false;
	}

	pass->scaleXFloat = 0;
	pass->scaleXUint = 0;
	if (!lookUpValueScale(passKey("scale"), &pass->scaleXFloat, &pass->scaleXUint, pass->scaleTypeX)) {
		return false;
	}

	pass->scaleYFloat = pass->scaleXFloat;
	pass->scaleYUint = pass->scaleXUint;
	return true;
}
#undef passKey

ShaderPreset *parsePreset(const Common::String &fileName) {
	Common::FSNode fileNode(fileName);
	if (!fileNode.exists() || !fileNode.isReadable() || fileNode.isDirectory()) {
		warning("LibRetro Preset Parsing: No such readable file '%s'", fileName.c_str());
		return nullptr;
	}

	Common::FSNode basePath(".");
	if (!basePath.exists() || !basePath.isReadable() || !basePath.isDirectory()) {
		warning("LibRetro Preset Parsing: Base path '%s' to file '%s' invalid", basePath.getPath().c_str(), fileName.c_str());
		return nullptr;
	}

	Common::SeekableReadStream *stream = fileNode.createReadStream();
	if (!stream) {
		return nullptr;
	}

	PresetParser parser;
	ShaderPreset *shader = parser.parseStream(*stream);
	delete stream;
	stream = nullptr;

	if (!shader) {
		warning("LibRetro Preset Parsing: Error while parsing file '%s': %s", fileName.c_str(), parser.getErrorDesc().c_str());
		return nullptr;
	}

	shader->basePath = basePath.getPath();
	return shader;
}

} // End of namespace LibRetro
} // End of namespace OpenGL
#endif // !USE_FORCED_GLES
