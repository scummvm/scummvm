#include "agds/dialog.h"
#include "agds/agds.h"
#include "agds/systemVariable.h"
#include "agds/object.h"
#include "common/debug.h"

namespace AGDS {

void Dialog::run(const Common::String & dialogProcess) {
	debug("runDialog: %s", dialogProcess.c_str());
	_dialogProcessName = dialogProcess;
	_engine->runObject(dialogProcess);
}

void Dialog::parseDialogDefs(const Common::String &defs) {
	_dialogDefs.clear();
	Common::String name, value;
	bool readName = true;
	for (uint32 p = 0, size = defs.size(); p < size; ++p) {
		char ch = defs[p];
		if (ch == ' ') {
			continue;
		} else if (ch == '\n' || ch == '\r') {
			//debug("dialog definition: '%s' = '%s'", name.c_str(), value.c_str());
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

void Dialog::load(const Common::String &dialogScript, const Common::String &defs) {
	parseDialogDefs(defs);
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

	//this looks like an error in original code
	//original first value in this table is 0x0FFFFFEB4 (-332)
	//0x0FFFFFEB4 * 20 and then 16-to-32 conversion gives 0xffffe610
	// 0xe610 / 20 = 2944
	static const int delays[] = {
		2944, 631, 398, 251, 158,
		100, 63, 40, 25, 16, 10
	};
	return delays[speed] * delay / 41 + 1;
}


bool Dialog::tick() {
	if (_dialogProcessName.empty())
		return false;

	auto dialog_var = _engine->getSystemVariable("dialog_var");
	int dialog_var_value = dialog_var->getInteger();
	if (dialog_var_value != 0) {
		debug("dialog_var = %d, skipping tick", dialog_var_value);
		return false;
	}

	uint n = _dialogScript.size();
	if (_dialogScriptPos >= n)
		return false;

	Common::String &line = _dialogLine;
	line.clear();
	while (_dialogScriptPos < n && _dialogScript[_dialogScriptPos] != '\n' && _dialogScript[_dialogScriptPos] != '\r') {
		line += _dialogScript[_dialogScriptPos++];
	}
	++_dialogScriptPos;

	if (line.empty())
		return true;

	debug("dialog line: %s", line.c_str());
	if (line[0] == '@') {
		if (line[1] == '@') //comment
			return true;

		line.erase(0, 1);

		if (line.hasPrefix("sound")) {
			debug("sound: %s", line.c_str());
		} else {
			DialogDefsType::const_iterator it = _dialogDefs.find(line);
			if (it != _dialogDefs.end()) {
				int value = it->_value;
				debug("dialog value %d (0x%04x)", value, value);
				_engine->getSystemVariable("dialog_var")->setInteger(value);
				_engine->reactivate(_dialogProcessName);
			} else
				warning("invalid dialog directive: %s", line.c_str());
		}
	} else if (line[0] == ' ') {
		debug("text: %s", line.c_str() + 1);
		dialog_var->setInteger(-3);
	}
	if (_dialogScriptPos >= n && !_dialogProcessName.empty()) {
		Common::String process = _dialogProcessName;
		_dialogProcessName.clear();

		debug("end of dialog, running %s", process.c_str());
		_engine->getSystemVariable("dialog_var")->setInteger(-2);
		_engine->reactivate(_dialogProcessName);
		return false;
	}
	return true;
}


}
