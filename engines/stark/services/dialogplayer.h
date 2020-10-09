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

namespace Resources {
class Speech;
}

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

	enum OptionType {
		kOptionTypeAsk = 0
	};

	struct Option {
		uint32 _type;
		Common::String _caption;
		Resources::Dialog::Topic *_topic;
		int32 _replyIndex;
	};

	/** Enter a dialog */
	void run(Resources::Dialog *dialog);

	/** Play a one-shot sentence */
	void playSingle(Resources::Speech *speech);

	/** Check if a dialog is running */
	bool isRunning() const;

	/** Update the currently running dialog */
	void update();

	/** Select a dialog option */
	void selectOption(uint32 index);

	/** Can a speech be played? */
	bool isSpeechReady() const;
	bool isSpeechReady(Resources::Speech *speech) const;

	/** Return the speech to be played */
	Resources::Speech *acquireReadySpeech();

	/** Does the player need to choose between options? */
	bool areOptionsAvailable() const;

	/** List the currently available dialog options */
	Common::Array<DialogPlayer::Option> listOptions() const;

	/** Resume the dialog after it was interrupted to run a script */
	void resume(Resources::Dialog *dialog);

	/** Clear the currently running dialog */
	void reset();

protected:
	/** Build a list of available dialog options */
	void buildOptions();

	/** Removes the last only option from the options list */
	void removeLastOnlyOption();

	/** Initiate the next action after the end of a reply */
	void onReplyEnd();

	void saveToInterruptionSlot();
	void restoreFromInterruptionSlot();

	Resources::Dialog *_currentDialog;
	Resources::Dialog::Reply *_currentReply;

	Resources::Dialog *_interruptedDialog;
	Resources::Dialog::Reply *_interruptedReply;

	Resources::Speech *_singleSpeech;

	bool _speechReady;
	bool _optionsAvailable;
	Common::Array<Option> _options;
};

} // End of namespace Stark

#endif // STARK_SERVICES_DIALOG_PLAYER_H
