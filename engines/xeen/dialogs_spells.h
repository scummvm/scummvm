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
	SpriteResource _iconSprites;
	SpriteResource _scrollSprites;
	Common::Array<SpellEntry> _spells;

	SpellsDialog(XeenEngine *vm) : ButtonContainer(vm) {}

	Character *execute(ButtonContainer *priorDialog, Character *c, int isCasting);

	void loadButtons();

	const char *setSpellText(Character *c, int isCasting);
public:
	static Character *show(XeenEngine *vm, ButtonContainer *priorDialog,
		Character *c, int isCasting);
};

class CastSpell : public ButtonContainer {
private:
	SpriteResource _iconSprites;
	int _oldMode;
private:
	CastSpell(XeenEngine *vm);
	~CastSpell();

	int execute(Character *&c);

	void loadButtons();
public:
	static int show(XeenEngine *vm);
	static int show(XeenEngine *vm, Character *&c);
};

class SpellOnWho : public ButtonContainer {
private:
	SpellOnWho(XeenEngine *vm) : ButtonContainer(vm) {}

	int execute(int spellId);
public:
	static Character *show(XeenEngine *vm, int spellId);
};

class SelectElement : public ButtonContainer {
private:
	SpriteResource _iconSprites;

	SelectElement(XeenEngine *vm) : ButtonContainer(vm) {}

	int execute(int spellId);

	void loadButtons();
public:
	static int show(XeenEngine *vm, int spellId);
};

class NotWhileEngaged : public ButtonContainer {
private:
	NotWhileEngaged(XeenEngine *vm) : ButtonContainer(vm) {}

	void execute(int spellId);
public:
	static void show(XeenEngine *vm, int spellId);
};

class LloydsBeacon : public ButtonContainer {
private:
	SpriteResource _iconSprites;

	LloydsBeacon(XeenEngine *vm) : ButtonContainer(vm) {}

	bool execute();

	void loadButtons();
public:
	static bool show(XeenEngine *vm);
};

class Teleport : public ButtonContainer {
private:
	SpriteResource _iconSprites;

	Teleport(XeenEngine *vm) : ButtonContainer(vm) {}

	int execute();
public:
	static int show(XeenEngine *vm);
};

class TownPortal : public ButtonContainer {
private:
	TownPortal(XeenEngine *vm) : ButtonContainer(vm) {}

	int execute();
public:
	static int show(XeenEngine *vm);
};

class IdentifyMonster : public ButtonContainer {
private:
	IdentifyMonster(XeenEngine *vm) : ButtonContainer(vm) {}

	void execute();
public:
	static void show(XeenEngine *vm);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_SPELLS_H */
