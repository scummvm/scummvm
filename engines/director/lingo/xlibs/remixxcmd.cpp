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

#include "audio/audiostream.h"
#include "common/system.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/remixxcmd.h"
#include "director/sound.h"
#include "director/types.h"
#include "director/window.h"

/**************************************************
 *
 * USED IN:
 * the7colors
 *
 **************************************************/

namespace Director {

const char *const RemixXCMD::xlibName = "Remix";
const XlibFileDesc RemixXCMD::fileNames[] = {
	{ "Remix",	nullptr },
	{ nullptr,	nullptr },
};

static const BuiltinProto builtins[] = {
	{ "Remix", RemixXCMD::m_Remix, 1, 1, 300, CBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

class RemixXCMDState : public Object<RemixXCMDState> {
public:
	RemixXCMDState(const Common::Path &path);
	~RemixXCMDState();

	void endGame(bool success);
	void doSetSound();
	void doKeySound(int bank);
	void doStartSound();
	void interruptCheck();

	MacArchive *arch = nullptr;
	Common::Array<Common::Array<int>> sequence;
	Common::Array<SNDDecoder *> samples;
	bool faderLeft = true;
	bool newRecord = false;
	int deckA = 0;
	int deckASeqID = 0;
	int deckASubseqID = 0;
	int deckB = 0;
	int deckBSeqID = 0;
	int deckBSubseqID = 0;
	int totalLength = 0;
};

RemixXCMDState::RemixXCMDState(const Common::Path &path) : Object<RemixXCMDState>("Remix") {
	arch = new MacArchive();
	if (!arch->openFile(path)) {
		warning("RemixXCMDState: unable to open %s", path.toString().c_str());
		delete arch;
		arch = nullptr;
	}
	for (int i = 1; i <= 12; i++) {
		if (!arch)
			continue;
		Common::SeekableReadStreamEndian *snd = arch->getResource(MKTAG('s', 'n', 'd', ' '), 10000 + i);
		if (!snd) {
			warning("RemixXCMDState: couldn't find sample %d", i);

			continue;
		}
		SNDDecoder *decoder = new SNDDecoder();
		decoder->loadStream(*snd);
		samples.push_back(decoder);
		delete snd;
	}
}


RemixXCMDState::~RemixXCMDState() {
	for (auto &it : samples)
		delete it;
	samples.clear();
	sequence.clear();
	if (arch) {
		delete arch;
	}
}

// Turntable minigame in The Seven Colors.
// Uses a resource file containing samples + this XCMD to play them.

void RemixXCMD::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);

	if (!g_lingo->_openXLibsState.contains("Remix")) {
		RemixXCMDState *remixState = new RemixXCMDState(path);
		g_lingo->_openXLibsState.setVal("Remix", remixState);
	}

}

void RemixXCMD::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
	if (g_lingo->_openXLibsState.contains("Remix")) {
		AbstractObject *remixState = g_lingo->_openXLibsState.getVal("Remix");
		delete remixState;
		g_lingo->_openXLibsState.erase("Remix");
	}
}

void RemixXCMDState::endGame(bool success) {
	Datum totalLengthRef("TotalLength");
	totalLengthRef.type = GLOBALREF;
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	g_lingo->varAssign(totalLengthRef, success ? 100 : 200);
	sound->stopSound(1);
	sound->stopSound(2);
	sound->setPuppetSound(SoundID(), 1);
	sound->setPuppetSound(SoundID(), 2);
	sound->setChannelVolume(1, 192);
	sound->setChannelVolume(2, 192);
}


void RemixXCMDState::doSetSound() {
	if (!arch) {
		warning("RemixXCMDState::DoSetSound: No archive found");
		return;
	}
	// load from sequ resource 10000
	Common::SeekableReadStream *sequ = arch->getResource(MKTAG('s', 'e', 'q', 'u'), 10000);
	if (!sequ) {
		warning("RemixCMDState::DoSetSound: sequ resource 10000 not found");
		return;
	}
	sequence.clear();
	for (int i = 0; i < 5 && !sequ->eos(); i++) {
		int count = sequ->readByte();
		Common::Array<int> selection;
		debugC(5, kDebugXObj, "RemixXCMDState::DoSetSound(): bank %d: ", i+1);
		while (count > 0 && !sequ->eos()) {
			byte id = sequ->readByte();
			selection.push_back(id);
			debugC(5, kDebugXObj, "%d, ", id);
			count -= 1;
		}
		debugC(5, kDebugXObj, "\n");
		sequence.push_back(selection);
	}
	delete sequ;
}


void RemixXCMDState::doKeySound(int bank) {
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	if (!bank) {
		// hit crossfader
		faderLeft = !faderLeft;
		if (newRecord) {
			// start playing the new deck
			if (faderLeft) {
				sound->stopSound(1);
				deckASeqID = deckA;
				deckASubseqID = 0;
			} else {
				sound->stopSound(2);
				deckBSeqID = deckB;
				deckBSubseqID = 0;
			}
			newRecord = false;
		}
		// lower the volume on the old deck
		sound->setChannelVolume(faderLeft ? 1 : 2, 192);
		sound->setChannelVolume(faderLeft ? 2 : 1, 0);
	} else {
		// clicked a record
		if (faderLeft) {
			deckB = bank;
		} else {
			deckA = bank;
		}
		newRecord = true;
		// ReturnSwitchFlag
		Datum switchFlagRef("SwitchFlag");
		switchFlagRef.type = GLOBALREF;
		g_lingo->varAssign(switchFlagRef, Datum(1));
	}
}

void RemixXCMDState::doStartSound() {
	deckA = 1;
	deckB = 0;
	deckASeqID = 1;
	deckASubseqID = 0;
	deckBSeqID = 0;
	deckBSubseqID = 0;
	faderLeft = true;
	totalLength = 0;
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	sound->stopSound(1);
	sound->stopSound(2);
}

void RemixXCMDState::interruptCheck() {
	if (!arch)
		return;

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	// this function gets called on every frame.
	// both decks have music playing simultaneously, but with only one audible
	// at the same time.
	// if the record ends while you're crossfaded to it, you lose the minigame.

	Datum totalLengthRef("TotalLength");
	totalLengthRef.type = GLOBALREF;

	if (!sound->isChannelActive(1) && deckASeqID) {
		if (faderLeft && deckASubseqID >= (int)sequence[deckASeqID-1].size()) {
			// record ran out, trigger failure state
			endGame(false);
			return;
		}
		if (deckASubseqID < (int)sequence[deckASeqID-1].size()) {
			int currentSample = sequence[deckASeqID-1][deckASubseqID];

			debugC(5, kDebugXObj, "RemixXCMDState::InterruptCheck(): deck A, bank: %d, subseq: %d, id: %d", deckASeqID, deckASubseqID, currentSample);
			sound->setPuppetSound(SoundID(kSoundExternal, -1, -1), 1);
			Audio::AudioStream *sample = samples[currentSample-1]->getAudioStream(0, true, DisposeAfterUse::YES);
			sound->playStream(*sample, 1);
			deckASubseqID += 1;

			if (faderLeft) {
				totalLength += 2;
				g_lingo->varAssign(totalLengthRef, totalLength);
			}
		}
	}

	if (!sound->isChannelActive(2) && deckBSeqID) {
		if (!faderLeft && deckBSubseqID >= (int)sequence[deckBSeqID-1].size()) {
			// record ran out, trigger failure state
			endGame(false);
			return;
		}
		if (deckASubseqID < (int)sequence[deckBSeqID-1].size()) {
			int currentSample = sequence[deckBSeqID-1][deckBSubseqID];

			debugC(5, kDebugXObj, "RemixXCMDState::InterruptCheck(): deck B, bank: %d, subseq: %d, id: %d", deckBSeqID, deckBSubseqID, currentSample);
			sound->setPuppetSound(SoundID(kSoundExternal, -1, -1), 2);
			Audio::AudioStream *sample = samples[currentSample-1]->getAudioStream(0, true, DisposeAfterUse::YES);
			sound->playStream(*sample, 2);
			deckBSubseqID += 1;

			if (!faderLeft) {
				totalLength += 2;
				g_lingo->varAssign(totalLengthRef, totalLength);
			}
		}
	}

	// if we've been mixing for long enough, success
	if (totalLength > 45) {
		endGame(true);
	}
}

void RemixXCMD::m_Remix(int nargs) {
	g_lingo->printArgs("RemixXCMD::m_Remix", nargs);
	Datum result;
	if (nargs != 1) {
		result = Datum("Wrong number of params");
		g_lingo->dropStack(nargs);
		g_lingo->push(result);
		return;
	}

	if (!g_lingo->_openXLibsState.contains("Remix")) {
		warning("RemixXCMD::m_Remix: Missing state");
		return;
	}
	RemixXCMDState *state = (RemixXCMDState *)g_lingo->_openXLibsState.getVal("Remix");

	int arg = g_lingo->pop().asInt();
	if (arg == 0) {
		state->doSetSound();
	} else if (arg == -1) {
		// handled by destructor
	} else if ((arg >= 1) && (arg <= 5)) {
		state->doKeySound(arg);
	} else if (arg == 6) {
		state->doKeySound(0);
	} else if (arg == 98) {
		state->doStartSound();
	} else if (arg == 99) {
		state->interruptCheck();
	} else {
		result = Datum("Parameter must be 0-15 or 0");
	}

	g_lingo->push(result);
	return;
}

}
