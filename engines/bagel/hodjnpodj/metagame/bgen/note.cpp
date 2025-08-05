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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

IMPLEMENT_DYNCREATE(CNote, CObject)

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



// disk path specifications for note sounds
const char *CNote::m_pNoteSoundPath[NOTE_ICON_COUNT] = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	".\\sound\\mk6.wav",
	".\\sound\\ar2.wav",
	".\\sound\\nv3.wav",
	".\\sound\\ap2.wav",
	".\\sound\\lf2.wav",
	nullptr,
	".\\sound\\fm2.wav",
	".\\sound\\ac2.wav",
	".\\sound\\wc2.wav",
	".\\sound\\pm2.wav",
	".\\sound\\gf3b.wav",
	".\\sound\\wt2.wav",
	".\\sound\\mm2.wav",
	".\\sound\\br2.wav",
	".\\sound\\pi3.wav",
	".\\sound\\wr2.wav",
	".\\sound\\hm2.wav",
	".\\sound\\rsc1.wav",
	".\\sound\\inn.wav",
	".\\sound\\tfield.wav",
	".\\sound\\bhouse.wav",
	".\\sound\\acottage.wav",
	".\\sound\\temple.wav",
	nullptr,
	".\\sound\\farmhous.wav",
	".\\sound\\mansion.wav",
	".\\sound\\wcottage.wav",
	".\\sound\\postoff.wav",
	".\\sound\\theatre.wav"
};

// disk path specifications for note bitmaps
const char *CNote::m_pNoteBitmapPath[NOTE_ICON_COUNT] = {
	".\\art\\bat.bmp",
	".\\art\\buterfly.bmp",
	".\\art\\turtle.bmp",
	".\\art\\snail.bmp",
	".\\art\\owl.bmp",
	".\\art\\jelyfish.bmp",
	".\\art\\fish.bmp",
	".\\art\\ant.bmp",
	".\\art\\crab.bmp",
	".\\art\\archer.bmp",
	".\\art\\innkeep.bmp",
	".\\art\\artist.bmp",
	".\\art\\priest.bmp",
	".\\art\\bandit.bmp",
	".\\art\\farmer.bmp",
	".\\art\\aristoc.bmp",
	".\\art\\woodcut.bmp",
	".\\art\\postmast.bmp",
	".\\art\\leopold.bmp",
	".\\art\\witch.bmp",
	".\\art\\mermaid.bmp",
	".\\art\\bear.bmp",
	".\\art\\pirate.bmp",
	".\\art\\warlock.bmp",
	".\\art\\hermit.bmp",
	".\\art\\mishmosh.bmp",
	".\\art\\inn.bmp",
	".\\art\\tfield.bmp",
	".\\art\\bhouse.bmp",
	".\\art\\acottage.bmp",
	".\\art\\temple.bmp",
	".\\art\\hideout.bmp",
	".\\art\\farmhous.bmp",
	".\\art\\mansion.bmp",
	".\\art\\wcottage.bmp",
	".\\art\\postoff.bmp",
	".\\art\\theatre.bmp"
};
#endif

/*************************************************************************
 *
 * CNote()
 *
 * Parameters:
 *  int nID         note identifier
 *
 * Return Value:    none
 *
 * Description:     Constructor for note class.  Initialize all fields
 *                  to reflect a single instance of the note.
 *
 ************************************************************************/

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
	else if (nRepeat > NOTE_REPEAT_MAX)
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


/*************************************************************************
 *
 * ~CNote()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for note class.  Note note objects
 *                  are automatically purged from the equipment list.
 *
 ************************************************************************/

CNote::~CNote() {
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

/////////////////////////////////////////////////////////////////////////////
