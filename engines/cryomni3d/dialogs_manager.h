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

#ifndef CRYOMNI3D_DIALOGS_MANAGER_H
#define CRYOMNI3D_DIALOGS_MANAGER_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/str-array.h"

namespace CryOmni3D {

class DialogsManager {
public:
	struct SubtitlesSettings {
		SubtitlesSettings() { }
		SubtitlesSettings(int16 textLeft, int16 textTop, int16 textRight, int16 textBottom,
		                  int16 drawLeft, int16 drawTop, int16 drawRight, int16 drawBottom) :
			textRect(textLeft, textTop, textRight, textBottom), drawRect(drawLeft, drawTop, drawRight,
			        drawBottom) { }
		Common::Rect textRect;
		Common::Rect drawRect;
	};

	DialogsManager() : _gtoBuffer(nullptr), _gtoEnd(nullptr),
		_ignoreNoEndOfConversation(false) { }
	virtual ~DialogsManager();

	void init(uint arraySize, const Common::String &endOfConversationText) { _dialogsVariables.resize(arraySize); _endOfConversationText = endOfConversationText; }
	void loadGTO(const Common::String &gtoFile);

	void setupVariable(uint id, const Common::String &variable) { _dialogsVariables[id] = DialogVariable(variable, 'N'); }
	void reinitVariables();
	uint size() const { return _dialogsVariables.size(); }
	byte &operator[](uint idx) { return _dialogsVariables[idx].value; }
	const byte &operator[](uint idx) const { return _dialogsVariables[idx].value; }
	byte &operator[](const Common::String &name) { return find(name).value; }
	const byte &operator[](const Common::String &name) const { return find(name).value; }

	void registerSubtitlesSettings(const Common::String &videoName, const SubtitlesSettings &settings);

	void setIgnoreNoEndOfConversation(bool ignore) { _ignoreNoEndOfConversation = ignore; }
	bool play(const Common::String &sequence, bool &slowStop);

protected:
	virtual void executeShow(const Common::String &show) = 0;
	virtual void playDialog(const Common::String &video, const Common::String &sound,
	                        const Common::String &text, const SubtitlesSettings &settings) = 0;
	virtual void displayMessage(const Common::String &text) = 0;
	virtual uint askPlayerQuestions(const Common::String &video,
	                                const Common::StringArray &questions) = 0;

private:
	struct Goto {
		Goto() : label(), text(nullptr) {
		}
		Goto(const Common::String &label_, const char *text_) : label(label_), text(text_) {
		}

		Common::String label;
		const char *text;
	};

	struct DialogVariable {
		DialogVariable() : name(), value(0) {
		}
		DialogVariable(const Common::String &name_, byte value_) : name(name_), value(value_) {
		}

		Common::String name;
		byte value;
	};

	const DialogVariable &find(const Common::String &name) const;
	DialogVariable &find(const Common::String &name);
	Common::Array<DialogVariable> _dialogsVariables;

	void populateLabels();
	const char *findLabel(const char *label, const char **realLabel = nullptr) const;
	Common::String getLabelSound(const char *label) const;

	const char *findSequence(const char *sequence) const;
	Common::String findVideo(const char *data) const;
	Common::String getText(const char *text) const;

	Common::Array<Goto> executeAfterPlayAndBuildGotoList(const char *actions);
	void buildGotoGoto(const char *gotoLine, Common::Array<Goto> &gotos);
	bool buildGotoIf(const char *ifLine, Common::Array<Goto> &gotos);
	void executeLet(const char *letLine);
	void executeShow(const char *showLine);

	const char *executePlayerQuestion(const char *text, bool dryRun, const char **realLabel = nullptr);
	const char *parseIf(const char *ifLine);

	const char *nextLine(const char *currentPtr) const;
	const char *nextChar(const char *currentPtr) const;
	const char *previousMatch(const char *currentPtr, const char *str) const;

	char *_gtoBuffer;
	const char *_gtoEnd;
	Common::Array<const char *> _labels;

	Common::String _endOfConversationText;
	bool _ignoreNoEndOfConversation;

	Common::HashMap<Common::String, SubtitlesSettings> _subtitlesSettings;
};

} // End of namespace CryOmni3D

#endif
