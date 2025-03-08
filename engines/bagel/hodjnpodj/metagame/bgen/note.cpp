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

#include "bagel/hodjnpodj/metagame/bgen/note.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#ifndef FRAME_EXE
// descriptive text for each note
const char *CNote::m_pNoteText[NOTE_COUNT] = {
	//mankala
		"You've done well, young human!  I'll pass along some scuttlebutt"
		" that I've heard among my fellow bottom crawlers.  You'll find"
		" what you need at the following point in your book of clues...",

	// archeroids
		"You have defeated us, brave prince!  We were wrong to go on our"
		" cotton candy binge.  With our last breath, we give you a clue to"
		" help you on your quest.  Look to your book, and seek the following"
		" advice...",

	//no vacancy
		"Ah, fortune has smiled upon you, and so shall I.  Open your clue"
		" book, and heed the following advice...",

	// artparts
		"Thank you, thank you, my ingenious prince!  At last my mind is"
		" clear, and I can remember the clue I was to give you!  Look in"
		" your book, and seek this advice...",

	// life
		"You have achieved an impressive mastery of the many paths of Life,"
		" o prince!  The wisdom of the priests is yours.  Actually, you've"
		" had the knowledge with you all along.  It's in the ruby slippers..."
		"  I mean, in your clue book... just search for the following section...",

	// everything under the sun	
		"Fellow bandits!  This hard-working prince has done a fine inventory"
		" of our booty!  Shall we give him a bit of information for his trouble?!"
		"  [Shouts of \"Aye\" and \"Yeah\" and \"Righty-O\" and one \"Nah, string"
		" 'em up!]  The consensus seems to be yes, so look to this part of your book"
		" of clues...",

	// farmhouse
		"Ah, the prince who seeks Mish and Mosh.  I was told you would be here"
		" with many questions.  Well, here is what I know...",

	// mansion
		"My solicitor informed me that I was to expect you.  I believe I have some"
		" information that could be useful to you...",

	// wood cutter's cottage
		"Hello, there!  A little birdie told me to expect you!  Or, perhaps"
		" it was a very large bumble bee.  I don't know; I'm a woodcutter, not"
		" a biologist!  Anyway, here's everything that I know...",

	// post office
		"Pssst!  Hey you, prince!  Come here!  I know something that might be of"
		" help to you.  I was sorting the mail, and one of the envelopes fell open"
		" -- it was just an accident, you understand!  Opening mail is a royal offense!"
		"  In any case, the envelope contained the following message...",

	// theatre
		"Bravo!  A magnificent performance!  You've certainly earned the information"
		" I'm about to give to you, my young prince.  Just open your cluebook, and"
		" look for the following clue..."
};

// Disk path specifications for note sounds
const char *CNote::m_pNoteSoundPath[NOTE_ICON_COUNT] = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"meta/sound/mk6.wav",
	"meta/sound/ar2.wav",
	"meta/sound/nv3.wav",
	"meta/sound/ap2.wav",
	"meta/sound/lf2.wav",
	nullptr,
	"meta/sound/fm2.wav",
	"meta/sound/ac2.wav",
	"meta/sound/wc2.wav",
	"meta/sound/pm2.wav",
	"meta/sound/gf3b.wav",
	"meta/sound/wt2.wav",
	"meta/sound/mm2.wav",
	"meta/sound/br2.wav",
	"meta/sound/pi3.wav",
	"meta/sound/wr2.wav",
	"meta/sound/hm2.wav",
	"meta/sound/rsc1.wav",
	"meta/sound/inn.wav",
	"meta/sound/tfield.wav",
	"meta/sound/bhouse.wav",
	"meta/sound/acottage.wav",
	"meta/sound/temple.wav",
	nullptr,
	"meta/sound/farmhous.wav",
	"meta/sound/mansion.wav",
	"meta/sound/wcottage.wav",
	"meta/sound/postoff.wav",
	"meta/sound/theatre.wav"
};

// Disk path specifications for note bitmaps
const char *CNote::m_pNoteBitmapPath[NOTE_ICON_COUNT] = {
	"meta/art/bat.bmp",
	"meta/art/buterfly.bmp",
	"meta/art/turtle.bmp",
	"meta/art/snail.bmp",
	"meta/art/owl.bmp",
	"meta/art/jelyfish.bmp",
	"meta/art/fish.bmp",
	"meta/art/ant.bmp",
	"meta/art/crab.bmp",
	"meta/art/archer.bmp",
	"meta/art/innkeep.bmp",
	"meta/art/artist.bmp",
	"meta/art/priest.bmp",
	"meta/art/bandit.bmp",
	"meta/art/farmer.bmp",
	"meta/art/aristoc.bmp",
	"meta/art/woodcut.bmp",
	"meta/art/postmast.bmp",
	"meta/art/leopold.bmp",
	"meta/art/witch.bmp",
	"meta/art/mermaid.bmp",
	"meta/art/bear.bmp",
	"meta/art/pirate.bmp",
	"meta/art/warlock.bmp",
	"meta/art/hermit.bmp",
	"meta/art/mishmosh.bmp",
	"meta/art/inn.bmp",
	"meta/art/tfield.bmp",
	"meta/art/bhouse.bmp",
	"meta/art/acottage.bmp",
	"meta/art/temple.bmp",
	"meta/art/hideout.bmp",
	"meta/art/farmhous.bmp",
	"meta/art/mansion.bmp",
	"meta/art/wcottage.bmp",
	"meta/art/postoff.bmp",
	"meta/art/theatre.bmp"
};
#endif

CNote::CNote(int nID, int nClue, int nRepeat, int nPerson, int nPlace) {
	if ((nID < 0) ||
		(nID >= NOTE_COUNT))
		m_nID = -1;
	else
		m_nID = nID;

	if ((nClue < NOTE_ICON_BAT) ||
		(nClue > NOTE_ICON_CRAB))
		m_nClueID = -1;
	else
		m_nClueID = nClue;

	if (nRepeat < 1)
		m_nRepeatCount = 1;
	else
		if (nRepeat > NOTE_REPEAT_MAX)
			m_nRepeatCount = NOTE_REPEAT_MAX;
		else
			m_nRepeatCount = nRepeat;

	if ((nPerson < NOTE_ICON_ARCHER) ||
		(nPerson > NOTE_ICON_MISHMOSH))
		m_nPersonID = -1;
	else
		m_nPersonID = nPerson;

	if ((nPlace < NOTE_ICON_INN) ||
		(nPlace > NOTE_ICON_POSTOFFICE))
		m_nPlaceID = -1;
	else
		m_nPlaceID = nPlace;

	m_pNext = nullptr;
	m_pPrev = nullptr;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
