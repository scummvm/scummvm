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

const char *RemixXCMD::xlibName = "Remix";
const char *RemixXCMD::fileNames[] = {
	"Remix",
	nullptr
};

static BuiltinProto builtins[] = {
	{ "Remix", RemixXCMD::m_Remix, 1, 1, 300, CBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

struct RemixState {
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

static RemixState remixState;

// Turntable minigame in The Seven Colors.
// Uses a resource file containing samples + this XCMD to play them.

void RemixXCMD::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
	if (!remixState.arch) {
		remixState.arch = new MacArchive();
		if (!remixState.arch->openFile(path)) {
			warning("RemixXCMD::open: unable to open %s", path.toString().c_str());
			delete remixState.arch;
			remixState.arch = nullptr;
		}
		for (int i = 1; i <= 12; i++) {
			Common::SeekableReadStreamEndian *snd = remixState.arch->getResource(MKTAG('s', 'n', 'd', ' '), 10000 + i);
			if (!snd) {
				warning("RemixXCMD::open: couldn't find sample %d", i);

				continue;
			}
			SNDDecoder *decoder = new SNDDecoder();
			decoder->loadStream(*snd);
			remixState.samples.push_back(decoder);
		}
	}
}

void RemixXCMD::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
	doResetSound();
}

void RemixXCMD::endGame(bool success) {
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

void RemixXCMD::doSetSound() {
	if (!remixState.arch) {
		warning("RemixXCMD::DoSetSound: No archive found");
		return;
	}
	// load from sequ resource 10000
	Common::SeekableReadStream *sequ = remixState.arch->getResource(MKTAG('s', 'e', 'q', 'u'), 10000);
	if (!sequ) {
		warning("RemixCMD::DoSetSound: sequ resource 10000 not found");
		return;
	}
	remixState.sequence.clear();
	for (int i = 0; i < 5 && !sequ->eos(); i++) {
		int count = sequ->readByte();
		Common::Array<int> selection;
		debugC(5, kDebugXObj, "RemixXCMD::DoSetSound(): bank %d: ", i+1);
		while (count > 0 && !sequ->eos()) {
			byte id = sequ->readByte();
			selection.push_back(id);
			debugC(5, kDebugXObj, "%d, ", id);
			count -= 1;
		}
		debugC(5, kDebugXObj, "\n");
		remixState.sequence.push_back(selection);
	}
}

void RemixXCMD::doResetSound() {
	for (auto &it : remixState.samples)
		delete it;
	remixState.samples.clear();
	remixState.sequence.clear();
	if (remixState.arch) {
		delete remixState.arch;
		remixState.arch = nullptr;
	}
}

void RemixXCMD::doKeySound(int bank) {
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	if (!bank) {
		// hit crossfader
		remixState.faderLeft = !remixState.faderLeft;
		if (remixState.newRecord) {
			// start playing the new deck
			if (remixState.faderLeft) {
				sound->stopSound(1);
				remixState.deckASeqID = remixState.deckA;
				remixState.deckASubseqID = 0;
			} else {
				sound->stopSound(2);
				remixState.deckBSeqID = remixState.deckB;
				remixState.deckBSubseqID = 0;
			}
			remixState.newRecord = false;
		}
		// lower the volume on the old deck
		sound->setChannelVolume(remixState.faderLeft ? 1 : 2, 192);
		sound->setChannelVolume(remixState.faderLeft ? 2 : 1, 0);
	} else {
		// clicked a record
		if (remixState.faderLeft) {
			remixState.deckB = bank;
		} else {
			remixState.deckA = bank;
		}
		remixState.newRecord = true;
		// ReturnSwitchFlag
		Datum switchFlagRef("SwitchFlag");
		switchFlagRef.type = GLOBALREF;
		g_lingo->varAssign(switchFlagRef, Datum(1));
	}
}

void RemixXCMD::doStartSound() {
	remixState.deckA = 1;
	remixState.deckB = 0;
	remixState.deckASeqID = 1;
	remixState.deckASubseqID = 0;
	remixState.deckBSeqID = 0;
	remixState.deckBSubseqID = 0;
	remixState.faderLeft = true;
	remixState.totalLength = 0;
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	sound->stopSound(1);
	sound->stopSound(2);
}

void RemixXCMD::interruptCheck() {
	if (!remixState.arch)
		return;

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	// this function gets called on every frame.
	// both decks have music playing simultaneously, but with only one audible
	// at the same time.
	// if the record ends while you're crossfaded to it, you lose the minigame.

	Datum totalLengthRef("TotalLength");
	totalLengthRef.type = GLOBALREF;

	if (!sound->isChannelActive(1) && remixState.deckASeqID) {
		if (remixState.faderLeft && remixState.deckASubseqID >= (int)remixState.sequence[remixState.deckASeqID-1].size()) {
			// record ran out, trigger failure state
			endGame(false);
			return;
		}
		int currentSample = remixState.sequence[remixState.deckASeqID-1][remixState.deckASubseqID];

		debugC(5, kDebugXObj, "RemixXCMD::InterruptCheck(): deck A, bank: %d, subseq: %d, id: %d", remixState.deckASeqID, remixState.deckASubseqID, currentSample);
		sound->setPuppetSound(SoundID(kSoundExternal, -1, -1), 1);
		Audio::AudioStream *sample = remixState.samples[currentSample-1]->getAudioStream(0, true, DisposeAfterUse::YES);
		sound->playStream(*sample, 1);
		remixState.deckASubseqID += 1;

		if (remixState.faderLeft) {
			remixState.totalLength += 2;
			g_lingo->varAssign(totalLengthRef, remixState.totalLength);
		}
	}

	if (!sound->isChannelActive(2) && remixState.deckBSeqID) {
		if (!remixState.faderLeft && remixState.deckBSubseqID >= (int)remixState.sequence[remixState.deckBSeqID-1].size()) {
			// record ran out, trigger failure state
			endGame(true);
			return;
		}
		int currentSample = remixState.sequence[remixState.deckBSeqID-1][remixState.deckBSubseqID];

		debugC(5, kDebugXObj, "RemixXCMD::InterruptCheck(): deck B, bank: %d, subseq: %d, id: %d", remixState.deckBSeqID, remixState.deckBSubseqID, currentSample);
		sound->setPuppetSound(SoundID(kSoundExternal, -1, -1), 2);
		Audio::AudioStream *sample = remixState.samples[currentSample-1]->getAudioStream(0, true, DisposeAfterUse::YES);
		sound->playStream(*sample, 2);
		remixState.deckBSubseqID += 1;

		if (!remixState.faderLeft) {
			remixState.totalLength += 2;
			g_lingo->varAssign(totalLengthRef, remixState.totalLength);
		}
	}

	// if we've been mixing for long enough, success
	if (remixState.totalLength > 45) {
		endGame(true);
	}
}

void RemixXCMD::m_Remix(int nargs) {
	g_lingo->printSTUBWithArglist("RemixXCMD::m_Remix", nargs);
	Datum result;
	if (nargs != 1) {
		result = Datum("Wrong number of params");
		g_lingo->dropStack(nargs);
		g_lingo->push(result);
		return;
	}

	int arg = g_lingo->pop().asInt();
	if (arg == 0) {
		doSetSound();
	} else if (arg == -1) {
		doResetSound();
	} else if ((arg >= 1) && (arg <= 5)) {
		doKeySound(arg);
	} else if (arg == 6) {
		doKeySound(0);
	} else if (arg == 98) {
		doStartSound();
	} else if (arg == 99) {
		interruptCheck();
	} else {
		result = Datum("Parameter must be 0-15 or 0");
	}

	g_lingo->push(result);
	return;
}

}
