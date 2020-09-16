#include "agds/dialog.h"
#include "agds/agds.h"
#include "agds/systemVariable.h"
#include "agds/object.h"
#include "common/debug.h"

namespace AGDS {

void Dialog::run(const Common::String & dialogProcess) {
	_dialogProcessName = dialogProcess;
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



bool Dialog::tick() {
	if (_dialogProcessName.empty())
		return false;

	int dialog_var = _engine->getSystemVariable("dialog_var")->getInteger();
	if (dialog_var > 0) {
		_engine->getSystemVariable("dialog_var")->setInteger(-3);
		return false;
	} else if (dialog_var < 0) {
		_engine->getSystemVariable("dialog_var")->setInteger(0);
		return true;
	}

	uint n = _dialogScript.size();
	if (_dialogScriptPos >= n)
		return false;

	Common::String line;
	while (_dialogScriptPos < n && _dialogScript[_dialogScriptPos] != '\n' && _dialogScript[_dialogScriptPos] != '\r') {
		line += _dialogScript[_dialogScriptPos++];
	}
	++_dialogScriptPos;

	if (line.empty())
		return true;

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
			} else
				warning("invalid dialog directive: %s", line.c_str());
		}
	} else if (line[0] == ' ') {
		debug("text: %s", line.c_str() + 1);
	}
	if (_dialogScriptPos >= n && !_dialogProcessName.empty()) {
		Common::String process = _dialogProcessName;
		_dialogProcessName.clear();

		debug("end of dialog, running %s", process.c_str());
		_engine->runObject(process);
		_engine->getSystemVariable("dialog_var")->setInteger(-2);
		return false;
	}
	return true;
}


}
