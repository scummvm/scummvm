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

#ifndef XEEN_DIALOGS_SPELLS_H
#define XEEN_DIALOGS_SPELLS_H

#include "common/array.h"
#include "xeen/dialogs.h"
#include "xeen/party.h"

namespace Xeen {

struct SpellEntry {
	Common::String _name;
	int _spellIndex;
	int _spellId;
	int _color;

	SpellEntry(const Common::String &name, int spellIndex, int spellId) :
		_name(name), _spellIndex(spellIndex), _spellId(spellId), _color(9) {}
};

class SpellsDialog : public ButtonContainer {
private:
	XeenEngine *_vm;
	SpriteResource _iconSprites;
	SpriteResource _scrollSprites;
	Common::Array<SpellEntry> _spells;

	SpellsDialog(XeenEngine *vm) : ButtonContainer(), _vm(vm) {}

	Character *execute(Character *c, int v2);

	void loadButtons();

	const char *setSpellText(Character *c, int v2);
public:
	static Character *show(XeenEngine *vm, Character *c, int isCasting);
};

class CastSpell : public ButtonContainer {
private:
	XeenEngine *_vm;
	SpriteResource _iconSprites;

	CastSpell(XeenEngine *vm) : ButtonContainer(), _vm(vm) {}

	int execute(Character *&c, int mode);

	void loadButtons();
public:
	static int show(XeenEngine *vm, int mode);
	static int show(XeenEngine *vm, Character *&c, int mode);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_SPELLS_H */
