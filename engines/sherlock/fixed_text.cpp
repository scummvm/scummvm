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

#include "sherlock/sherlock.h"
#include "sherlock/fixed_text.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"

namespace Sherlock {

static const char *const fixedJournalTextEN[] = {
	// Holmes asked/said...
	"Holmes asked me, ",
	"Holmes asked the Inspector, ",
	"Holmes asked %s, ",
	"Holmes said to me, ",
	"Holmes said to the Inspector, ",
	"Holmes said to %s, ",
	// I asked/said...
	"I replied, ",
	"The reply was, ",
	// Holmes/I/The Inspector/Person asked/said (without "Then" prefix)
	"Holmes asked, ",
	"Holmes said, ",
	"I asked, ",
	"I said, ",
	"The Inspector asked, ",
	"The Inspector said, ",
	"%s asked, ",
	"%s said, ",
	// Then Holmes/I/The Inspector/Person asked/said
	"Then Holmes asked, ",
	"Then Holmes said, ",
	"Then I asked, ",
	"Then I said, ",
	"Then the Inspector asked, ",
	"Then the Inspector said, ",
	"Then %s asked, ",
	"Then %s said, "
};

static const char *const fixedJournalTextDE[] = {
	// Holmes asked/said...
	"Holmes fragte mich, ",
	"Holmes fragte Inspektor Lestrade, ",
	"Holmes fragte %s, ",
	"Holmes sagte mir, ",
	"Holmes sagte Inspektor Lestrade, ",
	"Holmes sagte %s, ",
	// I asked/said...
	"Ich antwortete, ",
	"Die Antwort lautete, ",
	// Holmes/I/The Inspector/Person asked/said (without "Then" prefix)
	"Holmes fragte, ", // original: "fragte Holmes"
	"Holmes sagte, ", // original: "sagte Holmes"
	"Ich fragte, ", // original: "fragte Ich"
	"Ich sagte, ", // original: "sagte Ich"
	"Der Inspektor fragte, ",
	"Der Inspektor sagte, ",
	"%s fragte, ",
	"%s sagte, ",
	// Then Holmes/I/The Inspector/Person asked/said
	"Dann fragte Holmes, ",
	"Dann sagte Holmes, ",
	"Dann fragte ich, ", // original: "Dann sagte Ich"
	"Dann sagte ich, ", // original: "Dann sagte Ich"
	"Dann fragte der Inspektor, ",
	"Dann sagte der Inspektor, ",
	"Dann fragte %s, ",
	"Dann sagte %s, "
};

// Only used for Sherlock Holmes 2, so special characters should use the SH2 charset
// small a w/ accent grave: 0x85 / octal 205
// small e w/ accent acute: 0x82 / octal 202
// small e w/ accent grave: 0x8A / octal 212
// small e w/ circonflexe:  0x88 / octal 210
// small cedilla:           0x87 / octal 207
static const char *const fixedJournalTextFR[] = {
	// Holmes asked/said...
	"Holmes me demanda, ", // original: "Holmes m'a demand\202, "
	"Holmes demanda \205 l'inspecteur, ", // original: "Holmes a demand\202 \205 l'inspecteur, "
	"Holmes demanda \205 %s, ", // original: "Holmes a demand\202 \205 %s, "
	"Holmes me dit, ", // original: "Holmes m'a dit, "
	"Holmes dit \205 l'inspecteur, ", // original: "Holmes a dit \205 l'inspecteur, "
	"Holmes dit \205 %s, ", // original: "Holmes a dit \205 %s, "
	// I asked/said...
	"Je r\202pondis, ", // original: "J'ai r\202pondu, ",
	"La r\202ponse fut, ",
	// Holmes/I/The Inspector/Person asked/said (without "Then" prefix)
	"Holmes demanda, ", // original: "Holmes a demand\202, "
	"Holmes dit, ",
	"Je demandai, ", // original: "J'ai demand\202, "
	"Je dis, ", // original: "J'ai dit, "
	"L'inspecteur demanda, ", // original: ""L'inspecteur a demand\202, "
	"L'inspecteur dit, ",
	"%s demanda, ", // original: "%s a demand\202, "
	"%s dit, ",
	// Then Holmes/I/The Inspector/Person asked/said
	"Alors Holmes demanda, ", // original: it seems "puis"/"then" was not used/removed. They instead added a space character, so sentences looked weird
	"Alors Holmes dit, ",
	"Alors je demandai, ",
	"Alors je dis, ",
	"Alors l'inspecteur demanda, ",
	"Alors l'inspecteur dit, ",
	"Alors %s demanda, ",
	"Alors %s dit, "
};

// Attention: Special characters are NOT the same in Sherlock Holmes 1+2
// That's why we need 2 tables for Spanish.

// Sherlock Holmes 2:
// small a w/ accent bottom to top : 0xA0 / octal 240
// small i w/ accent bottom to top : 0xA1 / octal 241
// small o w/ accent bottom to top : 0xA2 / octal 242
// small u w/ accent bottom to top : 0xA3 / octal 243
// small n w/ wavy line            : 0xA4 / octal 244
// inverted question mark          : 0xA8 / octal 250
static const char *const fixedJournalTextES2[] = {
	// Holmes asked/said...
	"Holmes me pidi\242, ",
	"Holmes pregunt\242 el inspector, ",
	"Holmes pregunt\242 %s, ",
	"Holmes me dijo, ",
	"Holmes dijo el inspector, ",
	"Holmes dijo %s, ",
	// I asked/said...
	"Yo respond\241, ",
	"La respuesta fue, ",
	// Holmes/I/The Inspector/Person asked/said (without "Then" prefix)
	"Holmes pregunt\242, ",
	"Holmes dijo, ",
	"Yo pregunt\242, ",
	"Yo dije, ",
	"El inspector pregunt\242, ",
	"El inspector dijo, ",
	"%s pregunt\242, ",
	"%s dijo, ",
	// Then Holmes/I/The Inspector/Person asked/said
	"Entonces Holmes pregunt\242, ",
	"Entonces Holmes dijo, ",
	"Entonces yo pregunt\242, ",
	"Entonces yo dije, ",
	"Entonces el Inspector pregunt\242, ",
	"Entonces el Inspector dijo, ",
	"Entonces %s pregunt\242, ",
	"Entonces %s dijo, "
};

FixedText::FixedText(SherlockEngine *vm)  {
	_vm = vm;

	// Figure out which fixed texts to use
	Common::Language curLanguage = _vm->getLanguage();

	switch (curLanguage) {
	case Common::EN_ANY:
		// Used by Sherlock Holmes 1+2
		_fixedJournalTextArray = fixedJournalTextEN;
		break;
	case Common::DE_DEU:
		// Used by Sherlock Holmes 1+2
		_fixedJournalTextArray = fixedJournalTextDE;
		break;
	case Common::FR_FRA:
		// Used by Sherlock Holmes 2
		_fixedJournalTextArray = fixedJournalTextFR;
		break;
	case Common::ES_ESP:
		// Used by Sherlock Holmes 1+2
		// Because of different extended charsets, we need to use 2 tables
		if (vm->getGameID() == GType_SerratedScalpel)
			_fixedJournalTextArray = fixedJournalTextES2; // TODO check translation and then create ES1
		else
			_fixedJournalTextArray = fixedJournalTextES2;
		break;
	default:
		// Default to English
		_fixedJournalTextArray = fixedJournalTextEN;
		break;
	}
}

FixedText *FixedText::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelFixedText(vm);
	else
		return new Tattoo::TattooFixedText(vm);
}

const char *FixedText::getJournalText(int fixedJournalTextId) {
	return _fixedJournalTextArray[fixedJournalTextId];
}

} // End of namespace Sherlock
