#ifndef HARVESTER_NPC_DIALOGUE_RUNTIME_H
#define HARVESTER_NPC_DIALOGUE_RUNTIME_H

#include "common/error.h"
#include "common/str.h"
#include "harvester/script.h"

namespace Harvester {

class HarvesterEngine;
class Flow;
class Text;

struct DialogueLineEntry {
	int wavId;
	const char *speakerId;
	int headVariant;
};

struct DialogueKeywordMenuSelectionState {
	DialogueKeywordMenuSelectionState()
		: fromTypedInput(false), fromEscape(false), fromGenericBye(false) {
	}

	bool fromTypedInput;
	bool fromEscape;
	bool fromGenericBye;
};

class DialogueRuntimeCallbacks {
public:
	virtual ~DialogueRuntimeCallbacks() {}

	virtual Common::Error playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
		int headVariant) = 0;
	virtual Common::Error playDialogueLine(int wavId, const Common::String &speakerId) = 0;
	virtual Common::Error playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) = 0;
	virtual Common::Error playDialogueFst(const Common::String &path) = 0;
	virtual Common::Error clearScreenToBlack() = 0;
	virtual Common::Error showCdChangePrompt(int discNumber) = 0;
	virtual Common::Error runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
		Common::String &selectedTopic, DialogueKeywordMenuSelectionState &selection) = 0;
	virtual Common::Error runResponseMenu(int responseLineIndex, int &responseIndex) = 0;
	virtual Common::Error runResponseMenuText(const Common::String &responseLine, int &responseIndex) = 0;
	virtual Common::Error runGameOverScreen() = 0;
	virtual void assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
		int responseLineIndex, const char *label) = 0;
	virtual bool matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) = 0;
	virtual bool matchesAnyResponseLine(const Common::String &selectedTopic,
		const int *responseLineIndices, uint responseLineCount) = 0;
	virtual void queueDialogueInteractionIfNeeded(const InteractionResult &interaction) = 0;
	virtual void applyImmediateDialogueInteractionEffects(InteractionResult &interaction) = 0;
	virtual int getRandomNumber(int maxValue) = 0;
	virtual void setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) = 0;
};

class DialogueRuntime {
public:
	typedef DialogueKeywordMenuSelectionState KeywordMenuSelectionState;

	DialogueRuntime(HarvesterEngine &engine, Script &script, Text &text,
			Flow &flow, const Common::String &currentRoomName,
			const Common::String &genericByeTopic,
			DialogueRuntimeCallbacks &callbacks)
		: _engine(engine), _script(script), _text(text),
		  _flow(flow), _currentRoomName(currentRoomName),
		  _genericByeTopic(genericByeTopic),
		  _callbacks(callbacks) {
	}

	HarvesterEngine &engine() const { return _engine; }
	Script &startupScript() const { return _script; }
	Text &startupText() const { return _text; }
	Flow &startupFlow() const { return _flow; }
	const Common::String &currentRoomName() const { return _currentRoomName; }
	const Common::String &genericByeTopic() const { return _genericByeTopic; }
	bool executeActionTag(const Common::String &tag, InteractionResult &result,
			bool allowTransitions = true) const {
		return _script.executeActionTag(tag, result, allowTransitions, _currentRoomName);
	}

	Common::Error playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
			int headVariant) const {
		return _callbacks.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	}
	Common::Error playDialogueLine(int wavId, const Common::String &speakerId) const {
		return _callbacks.playDialogueLine(wavId, speakerId);
	}
	Common::Error playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) const {
		return _callbacks.playDialogueEntrySequence(lines, count);
	}
	Common::Error playDialogueFst(const Common::String &path) const { return _callbacks.playDialogueFst(path); }
	Common::Error clearScreenToBlack() const { return _callbacks.clearScreenToBlack(); }
	Common::Error showCdChangePrompt(int discNumber) const { return _callbacks.showCdChangePrompt(discNumber); }
	Common::Error runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
			Common::String &selectedTopic) const {
		_lastKeywordSelection = KeywordMenuSelectionState();
		return _callbacks.runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic, _lastKeywordSelection);
	}
	Common::Error runResponseMenu(int responseLineIndex, int &responseIndex) const {
		return _callbacks.runResponseMenu(responseLineIndex, responseIndex);
	}
	Common::Error runResponseMenuText(const Common::String &responseLine, int &responseIndex) const {
		return _callbacks.runResponseMenuText(responseLine, responseIndex);
	}
	Common::Error runGameOverScreen() const { return _callbacks.runGameOverScreen(); }
	void assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex, const char *label) const {
		_callbacks.assignTopicBuffer(topicBuffer, topicBufferLineIndex, responseLineIndex, label);
	}
	bool matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) const {
		return _callbacks.matchesResponseLine(selectedTopic, responseLineIndex);
	}
	bool matchesAnyResponseLine(const Common::String &selectedTopic,
			const int *responseLineIndices, uint responseLineCount) const {
		return _callbacks.matchesAnyResponseLine(selectedTopic, responseLineIndices, responseLineCount);
	}
	bool lastKeywordSelectionWasTypedInput() const { return _lastKeywordSelection.fromTypedInput; }
	bool lastKeywordSelectionWasGenericBye() const { return _lastKeywordSelection.fromGenericBye; }
	void queueDialogueInteractionIfNeeded(const InteractionResult &interaction) const {
		_callbacks.queueDialogueInteractionIfNeeded(interaction);
	}
	void applyImmediateDialogueInteractionEffects(InteractionResult &interaction) const {
		_callbacks.applyImmediateDialogueInteractionEffects(interaction);
	}
	int getRandomNumber(int maxValue) const { return _callbacks.getRandomNumber(maxValue); }
	void setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) const {
		_callbacks.setActiveSpeakerPortrait(speakerId, headVariant);
	}

private:
	HarvesterEngine &_engine;
	Script &_script;
	Text &_text;
	Flow &_flow;
	const Common::String &_currentRoomName;
	const Common::String &_genericByeTopic;
	DialogueRuntimeCallbacks &_callbacks;
	mutable KeywordMenuSelectionState _lastKeywordSelection;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_DIALOGUE_RUNTIME_H
