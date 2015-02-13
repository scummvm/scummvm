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

#ifndef STARK_RESOURCES_DIALOG_H
#define STARK_RESOURCES_DIALOG_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

class XRCReadStream;

namespace Resources {

class Speech;

/**
 * A dialog between two characters.
 *
 * Dialogs are made of a list of topics. Each topic has a list of
 * possible answers, one of which is played when the player selects the topic,
 * until all the possible answers have been played.
 *
 * Answers are made of a list of lines. All of the lines of an answer are played,
 * one after the other when an answer is played. Lines reference Speech resources.
 */
class Dialog : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kDialog;

	Dialog(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Dialog();

	/**
	 * A topic reply
	 */
	class Reply {
	public:
		Reply();

		/** Start playing the reply. Sets the current line to the first one */
		void start();

		/** Select the next line to be played */
		void goToNextLine();

		/** Obtain the Speech resource for the current line, or null if the reply has ended */
		Speech *getCurrentSpeech();

	private:
		// Static data
		Common::Array<ResourceReference> _lines;
		uint32 _conditionType;
		ResourceReference _conditionReference;
		ResourceReference _conditionScriptReference;
		uint32 _conditionReversed;
		uint32 _field_88;
		uint32 _minChapter;
		uint32 _maxChapter;
		uint32 _noCaption;
		int32 _nextDialogIndex;
		ResourceReference _nextScriptReference;

		// State
		int32 _nextSpeechIndex;

		friend class Dialog;
	};

	/**
	 * A dialog topic
	 */
	class Topic {
	public:
		Topic();

		/** Compute the next possible reply index after the currently selected reply */
		int32 getNextReplyIndex() const;

		/** Obtain the caption for the topic */
		Common::String getCaption() const;

		/** Select a reply from its index */
		Reply *startReply(uint32 index);

		/** Returns the selected reply, or null if no reply is selected */
		Reply *getCurrentReply();

	private:
		Common::Array<Reply> _replies;

		bool _removeOnceDepleted;
		int32 _currentReplyIndex;

		friend class Dialog;
	};

	// Resource API
	void readData(XRCReadStream *stream) override;

	/** List the currently available topics for this Dialog */
	Common::Array<Topic *> listAvailableTopics();

	/** Obtain the Dialog which should be played at the outcome of this one, if any */
	Dialog *getNextDialog(Dialog::Reply *reply);

protected:
	void printData() override;

	Common::Array<Topic> _topics;
	uint32 _character;
	uint32 _hasAskAbout;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_DIALOG_H
