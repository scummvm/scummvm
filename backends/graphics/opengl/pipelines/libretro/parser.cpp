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

#include "graphics/opengl/system_headers.h"

#if !USE_FORCED_GLES
#include "backends/graphics/opengl/pipelines/libretro/parser.h"

#include "common/file.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/algorithm.h"
#include "common/tokenizer.h"
#include "common/ptr.h"
#include "common/util.h"

#include "common/textconsole.h"

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
	bool lookUpValue(const Common::String &key, WrapMode *value, const WrapMode defaultValue);

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
	bool computeDefaultScale(const Common::String &key, float *floatValue, uint *uintValue, const ScaleType scaleType);

	bool parseParameters();

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

	if (!parseParameters()) {
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

		size_t sharpPos = line.findFirstOf('#');
		if (sharpPos != line.npos) {
			// Remove the end of line
			line.erase(sharpPos);
		}

		if (line.empty()) {
			continue;
		}

		bool empty = true;
		for (uint i = 0; i < line.size(); i++) {
			if (!Common::isSpace(line[i])) {
				empty = false;
				break;
			}
		}

		if (empty)
			continue;

		// Split line into key, value pair.
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
		const unsigned long uintVal = strtoul(iter->_value.c_str(), &endptr, 0);
		// Original libretro is quite laxist with int values and only checks errno
		// This means that as long as there is some number at start, parsing won't fail
		if (endptr == iter->_value.c_str() || uintVal >= UINT_MAX) {
			_errorDesc = "Invalid unsigned integer value for key '" + key + "': '" + iter->_value + '\'';
			return false;
		} else {
			if (*endptr != '\0') {
				warning("Possibly invalid unsigned integer value for key '%s': '%s'", key.c_str(), iter->_value.c_str());
			}
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

bool PresetParser::lookUpValue(const Common::String &key, WrapMode *value, const WrapMode defaultValue) {
	StringMap::const_iterator iter = _entries.find(key);
	if (iter != _entries.end()) {
		if (iter->_value == "clamp_to_border") {
			*value = kWrapModeBorder;
			return true;
		} else if (iter->_value == "clamp_to_edge") {
			*value = kWrapModeEdge;
			return true;
		} else if (iter->_value == "repeat") {
			*value = kWrapModeRepeat;
			return true;
		} else if (iter->_value == "mirrored_repeat") {
			*value = kWrapModeMirroredRepeat;
			return true;
		} else {
			_errorDesc = "Invalid wrap mode for key '" + key + "': '" + iter->_value + '\'';
			return false;
		}
	} else {
		*value = defaultValue;
		return true;
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

	WrapMode wrapMode;
	if (!lookUpValue(id + "_wrap_mode", &wrapMode, kWrapModeBorder)) {
		return false;
	}

	_shader->textures.push_back(ShaderTexture(id, fileName, filteringMode, wrapMode));
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

	if (!lookUpValue(passKey("alias"), &pass.alias)) {
		_errorDesc.clear();
		pass.alias.clear();
	}

	if (!lookUpValue(passKey("filter_linear"), &pass.filteringMode, kFilteringModeUnspecified)) {
		return false;
	}

	if (!lookUpValue(passKey("wrap_mode"), &pass.wrapMode, kWrapModeBorder)) {
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

	ScaleType scale_type;
	// Small trick here: lookUpValue never returns kScaleTypeFull
	if (!lookUpValue(passKey("scale_type"), &scale_type, kScaleTypeFull)) {
		return false;
	} else if (scale_type != kScaleTypeFull) {
		pass->scaleTypeY = pass->scaleTypeX = scale_type;
	}

	return true;
}

bool PresetParser::parsePassScale(const uint id, ShaderPass *pass) {
	// Parse actual scale value for the pass.
	//
	// Like for the scale type, 'scale' overrides 'scale_x'/'scale_y'.
	if (_entries.contains(passKey("scale"))) {
		if (!lookUpValueScale(passKey("scale"), &pass->scaleXFloat, &pass->scaleXUint, pass->scaleTypeX)) {
			return false;
		}

		if (!lookUpValueScale(passKey("scale"), &pass->scaleYFloat, &pass->scaleYUint, pass->scaleTypeY)) {
			return false;
		}

		return true;
	}

	if (_entries.contains(passKey("scale_x"))) {
		if (!lookUpValueScale(passKey("scale_x"), &pass->scaleXFloat, &pass->scaleXUint, pass->scaleTypeX)) {
			return false;
		}
	} else {
		if (!computeDefaultScale(passKey("scale_x"), &pass->scaleXFloat, &pass->scaleXUint, pass->scaleTypeX)) {
			return false;
		}
	}

	if (_entries.contains(passKey("scale_y"))) {
		if (!lookUpValueScale(passKey("scale_y"), &pass->scaleYFloat, &pass->scaleYUint, pass->scaleTypeY)) {
			return false;
		}
	} else {
		if (!computeDefaultScale(passKey("scale_y"), &pass->scaleYFloat, &pass->scaleYUint, pass->scaleTypeY)) {
			return false;
		}
	}

	return true;
}
#undef passKey

bool PresetParser::computeDefaultScale(const Common::String &key, float *floatValue, uint *uintValue, const ScaleType scaleType) {
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

bool PresetParser::parseParameters() {
	Common::String parameters;
	if (!lookUpValue("parameters", &parameters)) {
		return true;
	}

	// Parse all texture information from preset.
	Common::StringTokenizer tokenizer(parameters, ";");
	while (!tokenizer.empty()) {
		Common::String key = tokenizer.nextToken();
		if (_entries.contains(key)) {
			float value;
			if (!lookUpValue(key, &value)) {
				return false;
			}
			_shader->parameters[key] = value;
		}
	}

	return true;
}

ShaderPreset *parsePreset(const Common::Path &shaderPreset, Common::SearchSet &archSet) {
	Common::SeekableReadStream *stream;
	Common::Archive *container = nullptr;
	Common::Path basePath;

	// First try SearchMan, then fallback to filesystem
	if (archSet.hasFile(shaderPreset)) {
		Common::ArchiveMemberPtr member = archSet.getMember(shaderPreset, &container);
		stream = member->createReadStream();
		basePath = shaderPreset.getParent();
	} else {
		Common::FSNode fsnode(shaderPreset);
		if (!fsnode.exists() || !fsnode.isReadable() || fsnode.isDirectory()
				|| !(stream = fsnode.createReadStream())) {
			warning("LibRetro Preset Parsing: Invalid file path '%s'", shaderPreset.toString().c_str());
			return nullptr;
		}
#if defined(WIN32)
		static const char delimiter = '\\';
#else
		static const char delimiter = '/';
#endif
		basePath = Common::Path(fsnode.getParent().getPath(), delimiter);
	}

	PresetParser parser;
	ShaderPreset *shader = parser.parseStream(*stream);

	delete stream;

	if (!shader) {
		warning("LibRetro Preset Parsing: Error while parsing file '%s': %s", shaderPreset.toString().c_str(), parser.getErrorDesc().c_str());
		return nullptr;
	}

	shader->container = container;
	shader->basePath = basePath;
	return shader;
}

} // End of namespace LibRetro
} // End of namespace OpenGL
#endif // !USE_FORCED_GLES
