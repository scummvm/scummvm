#ifndef HARVESTER_NPC_DIALOGUE_RUNTIME_H
#define HARVESTER_NPC_DIALOGUE_RUNTIME_H

#include <functional>

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

class DialogueRuntime {
public:
	struct KeywordMenuSelectionState {
		KeywordMenuSelectionState()
			: fromTypedInput(false), fromEscape(false), fromGenericBye(false) {
		}

		bool fromTypedInput;
		bool fromEscape;
		bool fromGenericBye;
	};

	typedef std::function<Common::Error(int, const Common::String &, int)> PlayDialogueLineWithVariantFn;
	typedef std::function<Common::Error(int, const Common::String &)> PlayDialogueLineFn;
	typedef std::function<Common::Error(const DialogueLineEntry *, uint)> PlayDialogueEntrySequenceFn;
	typedef std::function<Common::Error(const Common::String &)> PlayDialogueFstFn;
	typedef std::function<Common::Error(const Common::String &, int, Common::String &,
			KeywordMenuSelectionState &)> RunKeywordMenuFn;
	typedef std::function<Common::Error(int, int &)> RunResponseMenuFn;
	typedef std::function<Common::Error(const Common::String &, int &)> RunResponseMenuTextFn;
	typedef std::function<Common::Error()> RunGameOverScreenFn;
	typedef std::function<void(Common::String &, int &, int, const char *)> AssignTopicBufferFn;
	typedef std::function<bool(const Common::String &, int)> MatchesResponseLineFn;
	typedef std::function<bool(const Common::String &, const int *, uint)> MatchesAnyResponseLineFn;
	typedef std::function<void(const StartupInteractionResult &)> QueueDialogueInteractionIfNeededFn;
	typedef std::function<void(StartupInteractionResult &)> ApplyImmediateDialogueInteractionEffectsFn;
	typedef std::function<int(int)> GetRandomNumberFn;
	typedef std::function<void(const Common::String &, int)> SetActiveSpeakerPortraitFn;

	DialogueRuntime(HarvesterEngine &engine, Script &startupScript, Text &startupText,
			Flow &startupFlow, const Common::String &currentRoomName,
			const Common::String &genericByeTopic,
			const PlayDialogueLineWithVariantFn &playDialogueLineWithVariant,
			const PlayDialogueLineFn &playDialogueLine,
			const PlayDialogueEntrySequenceFn &playDialogueEntrySequence,
			const PlayDialogueFstFn &playDialogueFst,
			const RunKeywordMenuFn &runKeywordMenu,
			const RunResponseMenuFn &runResponseMenu,
			const RunResponseMenuTextFn &runResponseMenuText,
			const RunGameOverScreenFn &runGameOverScreen,
			const AssignTopicBufferFn &assignTopicBuffer,
			const MatchesResponseLineFn &matchesResponseLine,
			const MatchesAnyResponseLineFn &matchesAnyResponseLine,
			const QueueDialogueInteractionIfNeededFn &queueDialogueInteractionIfNeeded,
			const ApplyImmediateDialogueInteractionEffectsFn &applyImmediateDialogueInteractionEffects,
			const GetRandomNumberFn &getRandomNumber,
			const SetActiveSpeakerPortraitFn &setActiveSpeakerPortrait)
		: _engine(engine), _startupScript(startupScript), _startupText(startupText),
		  _startupFlow(startupFlow), _currentRoomName(currentRoomName),
		  _genericByeTopic(genericByeTopic),
		  _playDialogueLineWithVariant(playDialogueLineWithVariant),
		  _playDialogueLine(playDialogueLine), _playDialogueEntrySequence(playDialogueEntrySequence),
		  _playDialogueFst(playDialogueFst), _runKeywordMenu(runKeywordMenu),
		  _runResponseMenu(runResponseMenu), _runResponseMenuText(runResponseMenuText),
		  _runGameOverScreen(runGameOverScreen),
		  _assignTopicBuffer(assignTopicBuffer),
		  _matchesResponseLine(matchesResponseLine),
		  _matchesAnyResponseLine(matchesAnyResponseLine),
		  _queueDialogueInteractionIfNeeded(queueDialogueInteractionIfNeeded),
		  _applyImmediateDialogueInteractionEffects(applyImmediateDialogueInteractionEffects),
		  _getRandomNumber(getRandomNumber),
		  _setActiveSpeakerPortrait(setActiveSpeakerPortrait) {
	}

	HarvesterEngine &engine() const { return _engine; }
	Script &startupScript() const { return _startupScript; }
	Text &startupText() const { return _startupText; }
	Flow &startupFlow() const { return _startupFlow; }
	const Common::String &currentRoomName() const { return _currentRoomName; }
	const Common::String &genericByeTopic() const { return _genericByeTopic; }

	Common::Error playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
			int headVariant) const {
		return _playDialogueLineWithVariant(wavId, speakerId, headVariant);
	}
	Common::Error playDialogueLine(int wavId, const Common::String &speakerId) const {
		return _playDialogueLine(wavId, speakerId);
	}
	Common::Error playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) const {
		return _playDialogueEntrySequence(lines, count);
	}
	Common::Error playDialogueFst(const Common::String &path) const { return _playDialogueFst(path); }
	Common::Error runKeywordMenu(const Common::String &topicBuffer, int topicBufferLineIndex,
			Common::String &selectedTopic) const {
		_lastKeywordSelection = KeywordMenuSelectionState();
		return _runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic, _lastKeywordSelection);
	}
	Common::Error runResponseMenu(int responseLineIndex, int &responseIndex) const {
		return _runResponseMenu(responseLineIndex, responseIndex);
	}
	Common::Error runResponseMenuText(const Common::String &responseLine, int &responseIndex) const {
		return _runResponseMenuText(responseLine, responseIndex);
	}
	Common::Error runGameOverScreen() const { return _runGameOverScreen(); }
	void assignTopicBuffer(Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex, const char *label) const {
		_assignTopicBuffer(topicBuffer, topicBufferLineIndex, responseLineIndex, label);
	}
	bool matchesResponseLine(const Common::String &selectedTopic, int responseLineIndex) const {
		return _matchesResponseLine(selectedTopic, responseLineIndex);
	}
	bool matchesAnyResponseLine(const Common::String &selectedTopic,
			const int *responseLineIndices, uint responseLineCount) const {
		return _matchesAnyResponseLine(selectedTopic, responseLineIndices, responseLineCount);
	}
	bool lastKeywordSelectionWasTypedInput() const { return _lastKeywordSelection.fromTypedInput; }
	bool lastKeywordSelectionWasGenericBye() const { return _lastKeywordSelection.fromGenericBye; }
	void queueDialogueInteractionIfNeeded(const StartupInteractionResult &interaction) const {
		_queueDialogueInteractionIfNeeded(interaction);
	}
	void applyImmediateDialogueInteractionEffects(StartupInteractionResult &interaction) const {
		_applyImmediateDialogueInteractionEffects(interaction);
	}
	int getRandomNumber(int maxValue) const { return _getRandomNumber(maxValue); }
	void setActiveSpeakerPortrait(const Common::String &speakerId, int headVariant) const {
		_setActiveSpeakerPortrait(speakerId, headVariant);
	}

private:
	HarvesterEngine &_engine;
	Script &_startupScript;
	Text &_startupText;
	Flow &_startupFlow;
	const Common::String &_currentRoomName;
	const Common::String &_genericByeTopic;
	PlayDialogueLineWithVariantFn _playDialogueLineWithVariant;
	PlayDialogueLineFn _playDialogueLine;
	PlayDialogueEntrySequenceFn _playDialogueEntrySequence;
	PlayDialogueFstFn _playDialogueFst;
	RunKeywordMenuFn _runKeywordMenu;
	RunResponseMenuFn _runResponseMenu;
	RunResponseMenuTextFn _runResponseMenuText;
	RunGameOverScreenFn _runGameOverScreen;
	AssignTopicBufferFn _assignTopicBuffer;
	MatchesResponseLineFn _matchesResponseLine;
	MatchesAnyResponseLineFn _matchesAnyResponseLine;
	mutable KeywordMenuSelectionState _lastKeywordSelection;
	QueueDialogueInteractionIfNeededFn _queueDialogueInteractionIfNeeded;
	ApplyImmediateDialogueInteractionEffectsFn _applyImmediateDialogueInteractionEffects;
	GetRandomNumberFn _getRandomNumber;
	SetActiveSpeakerPortraitFn _setActiveSpeakerPortrait;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_DIALOGUE_RUNTIME_H
