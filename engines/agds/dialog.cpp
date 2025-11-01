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

#include "agds/dialog.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "agds/systemVariable.h"
#include "common/debug.h"

namespace AGDS {

void Dialog::parseDialogDefs(const Common::String &defs) {
	_dialogDefs.clear();
	Common::String name, value;
	bool readName = true;
	for (uint32 p = 0, size = defs.size(); p < size; ++p) {
		char ch = defs[p];
		if (ch == ' ') {
			continue;
		} else if (ch == '\n' || ch == '\r' || p + 1 == size) {
			if (p + 1 == size)
				value += ch;
			// debug("dialog definition: '%s' = '%s'", name.c_str(), value.c_str());
			if (!name.empty() && !value.empty()) {
				_dialogDefs[name] = atoi(value.c_str());
			}
			readName = true;
			name.clear();
			value.clear();
			continue;
		} else if (ch == '=') {
			if (readName) {
				readName = false;
			} else {
				warning("equal sign in value, skipping");
			}
		} else {
			if (readName)
				name += ch;
			else
				value += ch;
		}
	}
}

void Dialog::load(const Common::String &processName, const Common::String &dialogScript, const Common::String &defs) {
	_currentDef.clear();
	_sounds.clear();
	parseDialogDefs(defs);
	_dialogProcessName = processName;
	_dialogScript = dialogScript;
	_dialogScriptPos = 0;
	_engine->getSystemVariable("dialog_var")->setInteger(-1);
}

int Dialog::textDelay(const Common::String &str) {
	int speed = _engine->getSystemVariable("text_speed")->getInteger();
	if (speed < 0)
		speed = 0;
	else if (speed > 100)
		speed = 100;
	speed /= 10;

	int delay = str.size();
	if (delay < 20)
		delay = 20;

	// this looks like an error in original code
	// original first value in this table is 0x0FFFFFEB4 (-332)
	// 0x0FFFFFEB4 * 20 and then 16-to-32 conversion gives 0xffffe610
	//  0xe610 / 20 = 2944
	static const int delays[] = {
		2944, 631, 398, 251, 158,
		100, 63, 40, 25, 16, 10};
	return delays[speed] * delay / 41 + 1;
}

bool Dialog::tick() {
	if (_dialogProcessName.empty())
		return false;

	auto dialog_var = _engine->getSystemVariable("dialog_var");
	int dialog_var_value = dialog_var->getInteger();
	if (dialog_var_value != 0) {
		return false;
	}

	uint n = _dialogScript.size();
	if (_dialogScriptPos >= n) {

		if (!_dialogProcessName.empty()) {
			debug("end of dialog, running %s", _dialogProcessName.c_str());
			dialog_var->setInteger(-2);
			_engine->reactivate(_dialogProcessName, "end of dialog");
		}

		return false;
	}

	Common::String &line = _dialogLine;
	line.clear();

	bool command = _dialogScript[_dialogScriptPos] == '@';
	while (_dialogScriptPos < n) {
		if (!command && _dialogScript[_dialogScriptPos] == '@')
			break;

		while (_dialogScriptPos < n && _dialogScript[_dialogScriptPos] != '\n' && _dialogScript[_dialogScriptPos] != '\r') {
			line += _dialogScript[_dialogScriptPos++];
		}
		if (!command)
			line += '\n';

		while (_dialogScriptPos < n && (_dialogScript[_dialogScriptPos] == '\n' || _dialogScript[_dialogScriptPos] == '\r'))
			++_dialogScriptPos;

		if (command)
			break;
	}

	if (line.empty())
		return true;

	debug("dialog line: %s", line.c_str());
	if (line[0] == '@') {
		processDirective(line);
	} else {
		debug("text: %s", line.c_str() + 1);
		dialog_var->setInteger(-3);
	}

	return true;
}

void Dialog::processSoundDirective(const Common::String &line) {
	debug("sound: %s", line.c_str());
	auto arg1 = line.find('(');
	if (arg1 == line.npos) {
		warning("invalid sound directive");
		return;
	}
	++arg1;

	auto comma1 = line.find(',', arg1);
	if (comma1 == line.npos) {
		warning("invalid sound directive, missing arg2");
		return;
	}
	++comma1;
	auto comma2 = line.find(',', comma1);
	if (comma2 == line.npos) {
		warning("invalid sound directive, missing arg3");
		return;
	}
	auto end = line.find(')', comma2 + 1);
	if (end == line.npos) {
		warning("invalid sound directive");
		return;
	}
	--end;
	Common::String name = line.substr(arg1, comma1 - arg1 - 1);
	while (line[comma1] == ' ')
		++comma1;
	Common::String sample = line.substr(comma1, comma2 - comma1);
	while (line[comma2] == ' ')
		++comma2;
	Common::String step = line.substr(comma2 + 1, end - comma2);
	debug("sound args = %s,%s,%s", name.c_str(), sample.c_str(), step.c_str());
	_sounds.push_back(Sound(name, sample, atoi(step.c_str())));
}

void Dialog::processDirective(Common::String line) {
	if (line.size() < 2 || line[1] == '@') {
		debug("comment, bailing out");
		return;
	}

	line.erase(0, 1);

	if (line.hasPrefix("sound")) {
		processSoundDirective(line);
	} else {
		DialogDefsType::const_iterator it = _dialogDefs.find(line);
		if (it != _dialogDefs.end()) {
			int value = it->_value;
			_currentDef = line;
			if (!_currentDef.hasPrefix("vybervarianty") && !_currentDef.hasPrefix("varianta")) {
				_currentSoundIndex = -1;

				for (uint s = 0; s < _sounds.size(); ++s) {
					auto &sound = _sounds[s];
					if (_currentDef.hasPrefixIgnoreCase(sound.Name)) {
						_currentSoundIndex = s;
						break;
					}
				}
			}
			debug("dialog value %s = %d (0x%04x), sample index: %d", line.c_str(), value, value, _currentSoundIndex);
			_engine->getSystemVariable("dialog_var")->setInteger(value);
		} else
			warning("invalid dialog directive: %s", line.c_str());
	}
}

Common::String Dialog::getNextDialogSound() {
	if (_currentDef.empty())
		return Common::String();

	debug("getNextDialogSound %s %d", _currentDef.c_str(), _currentSoundIndex);
	if (_currentSoundIndex < 0)
		return Common::String();

	auto &sound = _sounds[_currentSoundIndex];
	auto &sample = sound.Sample;
	auto currentSample = sample;

	int carry = sound.Step;
	for (auto pos = sample.size() - 1; pos > 0 && sample[pos] >= '0' && sample[pos] <= '9'; --pos) {
		int d = sample[pos] - '0';
		d += carry;
		carry = d / 10;
		sample.setChar('0' + (d % 10), pos);
		if (carry == 0)
			break;
	}
	if (carry != 0)
		warning("sample index overflow, %s", sample.c_str());

	debug("returning sample name %s", currentSample.c_str());
	return currentSample + ".ogg";
}

} // namespace AGDS
