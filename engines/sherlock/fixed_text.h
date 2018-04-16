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

#ifndef SHERLOCK_FIXED_TEXT_H
#define SHERLOCK_FIXED_TEXT_H

#include "common/scummsys.h"
#include "common/language.h"

namespace Sherlock {

#define FIXED(MSG) _vm->_fixedText->getText(kFixedText_##MSG)

enum FixedTextActionId {
	kFixedTextAction_Invalid = -1,
	kFixedTextAction_Open = 0,
	kFixedTextAction_Close,
	kFixedTextAction_Move,
	kFixedTextAction_Pick,
	kFixedTextAction_Use
};

enum FixedJournalTextId {
	// Holmes asked/said...
	kFixedJournalText_HolmesAskedMe = 0,
	kFixedJournalText_HolmesAskedTheInspector,
	kFixedJournalText_HolmesAskedPerson,
	kFixedJournalText_HolmesSaidToMe,
	kFixedJournalText_HolmesSaidToTheInspector,
	kFixedJournalText_HolmesSaidToPerson,
	// I asked/said
	kFixedJournalText_IReplied,
	kFixedJournalText_TheReplyWas,
	// Holmes/I/The Inspector/Person asked/said (without "Then" prefix)
	kFixedJournalText_HolmesAsked,
	kFixedJournalText_HolmesSaid,
	kFixedJournalText_IAsked,
	kFixedJournalText_ISaid,
	kFixedJournalText_TheInspectorAsked,
	kFixedJournalText_TheInspectorSaid,
	kFixedJournalText_PersonAsked,
	kFixedJournalText_PersonSaid,
	// Then Holmes/I/The Inspector/Person asked/said
	kFixedJournalText_ThenHolmesAsked,
	kFixedJournalText_ThenHolmesSaid,
	kFixedJournalText_ThenIAsked,
	kFixedJournalText_ThenISaid,
	kFixedJournalText_ThenTheInspectorAsked,
	kFixedJournalText_ThenTheInspectorSaid,
	kFixedJournalText_ThenPersonAsked,
	kFixedJournalText_ThenPersonSaid
};

class SherlockEngine;

class FixedText {
protected:
	SherlockEngine *_vm;

	FixedText(SherlockEngine *vm);
public:
	static FixedText *init(SherlockEngine *vm);
	virtual ~FixedText() {}

	/**
	 * Gets text
	 */
	virtual const char *getText(int fixedTextId) = 0;

	/**
	 * Get action message
	 */
	virtual const Common::String getActionMessage(FixedTextActionId actionId, int messageIndex) = 0;

	/**
	 * Gets journal text
	 */
	const char *getJournalText(int fixedJournalTextId);

	/**
	 * Gets object "Picked Up" text
	 */
	const char *getObjectPickedUpText();

private:
	const char *const *_fixedJournalTextArray;
	const char *_fixedObjectPickedUpText;
};

} // End of namespace Sherlock

#endif
