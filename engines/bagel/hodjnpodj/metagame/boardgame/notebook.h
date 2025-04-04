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

#ifndef HODJNPODJ_VIEWS_NOTEBOOK_H
#define HODJNPODJ_VIEWS_NOTEBOOK_H

#include "bagel/hodjnpodj/views/dialog.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class Notebook : public Dialog {
private:
	OkButton _okButton;
	Common::Rect _scrollTopRect, _scrollBottomRect;
	Common::Point _moreTop, _moreBottom;
	Common::Rect _personRect, _placeRect;
	CNote *pNoteList = nullptr;	// Pointer to notebook note list
	CNote *pKeyNote = nullptr;	// Single note to be shown
	const char *lpsPersonSoundSpec = nullptr;
	const char *lpsPlaceSoundSpec = nullptr;
	GfxSurface _person, _place, _clue;

	bool hasPriorNote() const;
	bool hasNextNote() const;
	void priorNote();
	void nextNote();
	void firstNote();
	void lastNote();
	void updateContent();

public:
	Notebook();
	virtual ~Notebook() {
	}

	static void show(CNote *note);

	void draw() override;
	bool msgOpen(const OpenMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;

	static void show(CNote *pNotes, CNote *pNote);
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
