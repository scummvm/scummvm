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

#include "twp/prefs.h"

namespace Twp {

Preferences::Preferences() {
	_node = new Common::JSONValue(Common::JSONObject());
}

Common::String Preferences::prefs(const Common::String name, const Common::String &def) const {
	const Common::JSONObject &jObj = _node->asObject();
	return jObj.contains(name) ? jObj[name]->asString() : def;
}

float Preferences::prefs(const Common::String &name, float def) const {
	const Common::JSONObject &jObj = _node->asObject();
	return jObj.contains(name) ? jObj[name]->asNumber() : def;
}

bool Preferences::prefs(const Common::String &name, bool def) const {
	const Common::JSONObject &jObj = _node->asObject();
	return jObj.contains(name) ? jObj[name]->asIntegerNumber() != 0 : def;
}

int Preferences::prefs(const Common::String &name, int def) const {
	const Common::JSONObject &jObj = _node->asObject();
	return jObj.contains(name) ? jObj[name]->asIntegerNumber() : def;
}

void Preferences::setPrefs(const Common::String &name, const Common::String &value) {
	Common::JSONObject jObj = _node->asObject();
	jObj[name] = new Common::JSONValue(value);
	delete _node;
	_node = new Common::JSONValue(jObj);
	savePrefs();
}

void Preferences::setPrefs(const Common::String &name, float value) {
	Common::JSONObject jObj = _node->asObject();
	jObj[name] = new Common::JSONValue(value);
	delete _node;
	_node = new Common::JSONValue(jObj);
	savePrefs();
}

void Preferences::setPrefs(const Common::String &name, int value) {
	Common::JSONObject jObj = _node->asObject();
	jObj[name] = new Common::JSONValue((long long int)value);
	delete _node;
	_node = new Common::JSONValue(jObj);
	savePrefs();
}

void Preferences::setPrefs(const Common::String &name, bool value) {
	Common::JSONObject jObj = _node->asObject();
	jObj[name] = new Common::JSONValue((long long int)(value ? 1 : 0));
	delete _node;
	_node = new Common::JSONValue(jObj);
	savePrefs();
}

bool Preferences::hasPrefs(const Common::String& name) {
  return _node->asObject().contains(name);
}

Common::JSONValue* Preferences::prefsAsJson(const Common::String& name) {
  return _node->asObject()[name];
}

void Preferences::savePrefs() {
	// TODO: savePrefs()
}

Common::String Preferences::getKey(const Common::String& path){
  size_t i = path.findLastOf(".");
  Common::String name = path.substr(0, i);
  Common::String ext = path.substr(i+1);
  if (name.hasSuffix("_en")) {
    // TODO: const Common::String& lang = prefs(Lang);
	Common::String lang = "en";
    return Common::String::format("%s_%s%s", name.substr(0, name.size()-4).c_str(), lang.c_str(), ext.c_str());
  }
  return path;
}

} // namespace Twp
