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

#ifndef SHERLOCK_TATTOO_FIXED_TEXT_H
#define SHERLOCK_TATTOO_FIXED_TEXT_H

#include "sherlock/fixed_text.h"

namespace Sherlock {

namespace Tattoo {

enum FixedTextId {
	kFixedText_Inv1,
	kFixedText_Inv2,
	kFixedText_Inv3,
	kFixedText_Inv4,
	kFixedText_Inv5,
	kFixedText_Inv6,
	kFixedText_Inv7,
	kFixedText_Inv8,
	kFixedText_InvDesc1,
	kFixedText_InvDesc2,
	kFixedText_InvDesc3,
	kFixedText_InvDesc4,
	kFixedText_InvDesc5,
	kFixedText_InvDesc6,
	kFixedText_InvDesc7,
	kFixedText_InvDesc8,
	kFixedText_Open,
	kFixedText_Look,
	kFixedText_Talk,
	kFixedText_Use,
	kFixedText_Journal,
	kFixedText_Inventory,
	kFixedText_Options,
	kFixedText_Solve,
	kFixedText_With,
	kFixedText_NoEffect,
	kFixedText_NothingToSay,
	kFixedText_PickedUp,

	kFixedText_Page,
	kFixedText_CloseJournal,
	kFixedText_SearchJournal,
	kFixedText_SaveJournal,
	kFixedText_AbortSearch,
	kFixedText_SearchBackwards,
	kFixedText_SearchForwards,
	kFixedText_TextNotFound,

	kFixedText_Holmes,
	kFixedText_Jock,
	kFixedText_Bull,
	kFixedText_Round,
	kFixedText_TurnTotal,
	kFixedText_Dart,
	kFixedText_ToStart,
	kFixedText_HitAKey,
	kFixedText_PressAKey,
	kFixedText_Bullseye,
	kFixedText_GameOver,
	kFixedText_Busted,
	kFixedText_Wins,
	kFixedText_Scored,
	kFixedText_Points,
	kFixedText_Hit,
	kFixedText_Double,
	kFixedText_Triple,

	kFixedText_Apply,
	kFixedText_Water,
	kFixedText_Heat,
	kFixedText_LoadGame,
	kFixedText_SaveGame,
	kFixedText_Music,
	kFixedText_SoundEffects,
	kFixedText_Voices,
	kFixedText_TextWindows,
	kFixedText_TransparentMenus,
	kFixedText_ChangeFont,
	kFixedText_Off,
	kFixedText_On,
	kFixedText_Quit,
	kFixedText_AreYouSureYou,
	kFixedText_WishToQuit,
	kFixedText_Yes,
	kFixedText_No,
	kFixedText_EnterPassword,
	kFixedText_CorrectPassword
};

class TattooFixedText: public FixedText {
private:
	const char *const *_fixedText;
public:
	TattooFixedText(SherlockEngine *vm);
	virtual ~TattooFixedText() {}

	/**
	 * Gets text
	 */
	virtual const char *getText(int fixedTextId);

	/**
	 * Get action message
	 */
	virtual const Common::String getActionMessage(FixedTextActionId actionId, int messageIndex);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
