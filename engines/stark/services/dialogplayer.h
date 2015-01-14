/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_SERVICES_DIALOG_PLAYER_H
#define STARK_SERVICES_DIALOG_PLAYER_H

#include "common/array.h"

#include "engines/stark/resources/dialog.h"

namespace Stark {

/**
 * Dialog player
 *
 * Handles the state of the currently running dialog, and implements the
 * associated logic.
 */
class DialogPlayer {
public:
	DialogPlayer();
	virtual ~DialogPlayer();

	/** Enter a dialog */
	void run(Dialog *dialog);

	/** Check if a dialog is running */
	bool isRunning();

	/** Update the currently running dialog */
	void update();

	/** Select a dialog option */
	void selectOption(uint32 index);

protected:
	enum OptionType {
		kOptionTypeAsk = 0
	};

	struct Option {
		uint32 _type;
		Common::String _caption;
		Dialog::Topic *_topic;
		int32 _replyIndex;
	};

	/** Build a list of available dialog options */
	void buildOptions();

	/** Initiate the next action after the end of a reply */
	void onReplyEnd();

	/** Clear the currently running dialog */
	void reset();

	Dialog *_currentDialog;
	Dialog::Reply *_currentReply;

	bool _speechReady;
	Common::Array<Option> _options;
};

} // End of namespace Stark

#endif // STARK_SERVICES_DIALOG_PLAYER_H
