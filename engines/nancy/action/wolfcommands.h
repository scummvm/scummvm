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

#ifndef NANCY_ACTION_WOLFCOMMANDS_H
#define NANCY_ACTION_WOLFCOMMANDS_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Action {

// Nancy16 commands for Isis the wolf (AR 198). The record fills a UI dialog with a
// button for each command; the player queues up a few, and "Go" has Nancy call them
// out. The queued commands then get left as a playlist under the wolf's name, for the
// wolf's movie or the wolf maze to act out, and an event flag gets set.
class WolfCommands : public ActionRecord, public UI::NDUI::EngineTarget {
public:
	WolfCommands() {}
	virtual ~WolfCommands();

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	void onEngineAction(const UI::NDUI::Action &action) override;

protected:
	Common::String getRecordTypeName() const override { return "WolfCommands"; }

	struct Command {
		Common::String buttonName;	// also the movie the wolf acts the command out with
		Common::String label;
		RandomSoundBlock clickSound;
		RandomSoundBlock callSound;	// Nancy calls the command out
	};

	const Command *findCommand(const Common::String &buttonName) const;
	void addCommand(const Common::String &buttonName);
	void clearCommands();
	void callNextCommand();

	// -- File data --
	Common::String _playlistName;
	Common::String _dialogName;
	Common::String _buttonTemplate;
	Common::String _displayName;		// shows the queued commands
	uint32 _maxCommands = 0;
	Common::String _separator;
	FlagDescription _doneFlag;
	Common::Array<Command> _commands;
	RandomSoundBlock _nameSound;		// Nancy calls the wolf by her name
	RandomSoundBlock _goSound;
	RandomSoundBlock _fullSound;

	// -- Runtime state --
	Common::Array<Common::String> _queue;
	bool _isRegistered = false;
	bool _isCalling = false;			// Nancy is calling out the queued commands
	int _callIndex = -1;				// -1 while calling the wolf by her name
	SoundDescription _callSound;
	bool _clearOnNextCommand = false;	// the commands of the last run are still shown
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_WOLFCOMMANDS_H
