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

#include "common/scummsys.h"

#include "zvision/scripting/controls/input_control.h"
#include "zvision/scripting/controls/save_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/string_manager.h"

#include "zvision/file/save_manager.h"
#include "zvision/graphics/render_manager.h"

#include "common/str.h"
#include "common/stream.h"

namespace ZVision {

SaveControl::SaveControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_SAVE),
	  _saveControl(false) {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("savebox", true)) {
			int saveId;
			int inputId;

			sscanf(values.c_str(), "%d %d", &saveId, &inputId);
			saveElement elmnt;
			elmnt.inputKey = inputId;
			elmnt.saveId = saveId;
			elmnt.exist = false;
			_inputs.push_back(elmnt);
		} else if (param.matchString("control_type", true)) {
			if (values.contains("save"))
				_saveControl = true;
			else
				_saveControl = false;
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}

	for (saveElmntList::iterator iter = _inputs.begin(); iter != _inputs.end(); ++iter) {
		Control *ctrl = _engine->getScriptManager()->getControl(iter->inputKey);
		if (ctrl && ctrl->getType() == Control::CONTROL_INPUT) {
			InputControl *inp = (InputControl *)ctrl;
			inp->setReadOnly(!_saveControl);
			Common::SeekableReadStream *save = _engine->getSaveManager()->getSlotFile(iter->saveId);
			if (save) {
				SaveGameHeader header;
				if (_engine->getSaveManager()->readSaveGameHeader(save, header)) {
					inp->setText(header.saveName);
					iter->exist = true;
				}
				delete save;
			}
		}
	}
}

bool SaveControl::process(uint32 deltaTimeInMillis) {
	for (saveElmntList::iterator iter = _inputs.begin(); iter != _inputs.end(); ++iter) {
		Control *ctrl = _engine->getScriptManager()->getControl(iter->inputKey);
		if (ctrl && ctrl->getType() == Control::CONTROL_INPUT) {
			InputControl *inp = (InputControl *)ctrl;
			if (inp->enterPress()) {
				if (_saveControl) {
					if (inp->getText().size() > 0) {
						bool toSave = true;
						if (iter->exist)
							if (!_engine->getRenderManager()->askQuestion(_engine->getStringManager()->getTextLine(StringManager::ZVISION_STR_SAVEEXIST)))
								toSave = false;

						if (toSave) {
							_engine->getSaveManager()->saveGame(iter->saveId, inp->getText(), true);
							_engine->getRenderManager()->delayedMessage(_engine->getStringManager()->getTextLine(StringManager::ZVISION_STR_SAVED), 2000);
							_engine->getScriptManager()->changeLocation(_engine->getScriptManager()->getLastMenuLocation());
						}
					} else {
						_engine->getRenderManager()->timedMessage(_engine->getStringManager()->getTextLine(StringManager::ZVISION_STR_SAVEEMPTY), 2000);
					}
				} else {
					_engine->getSaveManager()->loadGame(iter->saveId);
					return true;
				}
				break;
			}
		}
	}
	return false;
}

} // End of namespace ZVision
