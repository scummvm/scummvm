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

#include "access/noctropolis/noctropolis_resources.h"
#include "access/noctropolis/noctropolis_game.h"
#include "access/noctropolis/noctropolis_font.h"
#include "access/font.h"

namespace Access {

namespace Noctropolis {

static const char *NOCT_FILES[] = {
	"DARK/MAP.AP", "DARK/SCENE01.AP", "DARK/SCENE02.AP", "DARK/SCENE03.AP",
	"DARK/SCENE04.AP", "R05.AP", "R06.AP", "R07.AP",
	"R08.AP", "R09.AP", "R10.AP", "DARK/SCENE11.AP",
	"DARK/SCENE12.AP", "DARK/SCENE13.AP", "DARK/SCENE14.AP", "DARK/SCENE15.AP",
	"DARK/SCENE16.AP", "DARK/SCENE17.AP", "DARK/SCENE18.AP", "R19.AP",
	"DARK/SCENE20.AP", "R21.AP", "DARK/SCENE22.AP", "R23.AP",
	"DARK/SCENE24.AP", "R25.AP", "DARK/SCENE26.AP", "DARK/SCENE27.AP",
	"DARK/SCENE28.AP", "DARK/SCENE29.AP", "DARK/SCENE30.AP", "DARK/SCENE31.AP",
	"DARK/SCENE32.AP", "DARK/SCENE33.AP", "DARK/SCENE34.AP", "DARK/SCENE35.AP",
	"R36.AP", "DARK/SCENE37.AP", "DARK/SCENE38.AP", "DARK/SCENE39.AP",
	"R40.AP", "DARK/SCENE41.AP", "DARK/SCENE42.AP", "DARK/SCENE43.AP",
	"DARK/SCENE44.AP", "DARK/SCENE45.AP", "DARK/SCENE46.AP", "R47.AP",
	"DARK/SCENE48.AP", "DARK/SCENE49.AP", "DARK/SCENE50.AP", "DARK/SCENE51.AP",
	"DARK/SCENE52.AP", "DARK/SCENE53.AP", "DARK/SCENE54.AP", "DARK/SCENE55.AP",
	"R56.AP", "DARK/SCENE57.AP", "DARK/SCENE58.AP", "DARK/SCENE59.AP",
	"DARK/SCENE60.AP", "DARK/SCENE61.AP", "DARK/SCENE62.AP", "DARK/SCENE63.AP",
	"DARK/SCENE64.AP", "DARK/SCENE65.AP", "R66.AP", "DARK/SCENE67.AP",
	"DARK/SCENE68.AP", "DARK/SCENE69.AP", "DARK/SCENE70.AP", "DARK/SCENE71.AP",
	"DARK/SCENE72.AP", "DARK/SCENE73.AP", "DARK/SCENE74.AP", "R75.AP",
	"R76.AP", "R77.AP", "R78.AP", "R79.AP",
	"R80.AP", "DARK/SCENE81.AP", "DARK/SCENE82.AP", "R83.AP",
	"R84.AP", "DARK/SCENE85.AP", "DARK/SCENE86.AP", "DARK/SCENE87.AP",
	"R88.AP", "DARK/SCENE89.AP", "DARK/SCENE90.AP", "DARK/SCENE91.AP",
	"DARK/SCENE92.AP", "DARK/SCENE93.AP", "DARK/SCENE94.AP", "DARK/SCENE95.AP",
	"DARK/SCENE96.AP", "DARK/SCENE97.AP", "DARK/MUSIC.AP", "DARK/SOUND.AP",
	nullptr,
};

///////////////////////////////////////////////////////////////


NoctropolisResources::NoctropolisResources(AccessEngine *_vm) : Resources(_vm), _fontChaleteu(nullptr), _fontSystemeu(nullptr),
_fontSml3x5(nullptr), _fontNaples12(nullptr), _fontGothiceu(nullptr), _fontChaletse(nullptr),_fontComicseu(nullptr)
{
}

NoctropolisResources::~NoctropolisResources() {
	delete _fontChaleteu;
	delete _fontSystemeu;
	delete _fontSml3x5;
	delete _fontNaples12;
	delete _fontGothiceu;
	delete _fontChaletse;
	delete _fontComicseu;
}


const char *NoctropolisResources::getEgoName() const {
	return _vm->_flags[1] == 0 ? "PETER" : "DARKSHEER";
}

void NoctropolisResources::load(Common::SeekableReadStream &s) {
	// Note: *don't* call the base class here. Noctropolis doesn't have data in access.dat.

	// TODO: For non-EN variants we want to use something other than DARK/ as the path.
	for (int i = 0; i < ARRAYSIZE(NOCT_FILES); i++) {
		Common::Path filename = Common::Path(NOCT_FILES[i]).getLastComponent();
		FILENAMES.push_back(filename);
	}

	_fontChaleteu = new NoctropolisFont(0x66, 11, 1, 0xe2, CHALETEU_OFFSETS, CHALETEU_DATA);
	_fontSystemeu = new NoctropolisFont(0x48, 8, 0x20, 0xe2, SYSTEMEU_OFFSETS, SYSTEMEU_DATA);
	_fontSml3x5 = new NoctropolisFont(0x1e, 6, 0x20, 0x61, SML3X5_OFFSETS, SML3X5_DATA);
	_fontNaples12 = new NoctropolisFont(0x66, 15, 0x20, 0x7f, NAPLES12_OFFSETS, NAPLES12_DATA);
	_fontGothiceu = new NoctropolisFont(0x74, 16, 0x20, 0xec, GOTHICEU_OFFSETS, GOTHICEU_DATA);
	_fontChaletse = new NoctropolisFont(0x66, 8, 1, 0xec, CHALETSE_OFFSETS, CHALETSE_DATA);
	_fontComicseu = new NoctropolisFont(0x58, 8, 1, 0xec, COMICSEU_OFFSETS, COMICSEU_DATA);

	// Set up the fonts
	_vm->_fonts.addFont(_fontChaleteu);
	_vm->_fonts.addFont(_fontSystemeu);
	_vm->_fonts.addFont(_fontSml3x5);
	_vm->_fonts.addFont(_fontNaples12);
	_vm->_fonts.addFont(_fontGothiceu);
	_vm->_fonts.addFont(_fontChaletse);
	_vm->_fonts.addFont(_fontComicseu);
}


} // end namespace Noctropolis

} // end namespace Access
