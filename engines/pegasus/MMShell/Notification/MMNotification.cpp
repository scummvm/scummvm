/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "pegasus/MMShell/MMConstants.h"
#include "pegasus/MMShell/Notification/MMNotification.h"
#include "pegasus/MMShell/Notification/MMNotificationManager.h"
#include "pegasus/MMShell/Notification/MMNotificationReceiver.h"

namespace Pegasus {

typedef tReceiverList::iterator tReceiverIterator;

MMNotification::MMNotification(const tNotificationID id, MMNotificationManager *owner) : MMIDObject(id) {
	fOwner = owner;
	fCurrentFlags = kNoNotificationFlags;
	if (fOwner)
		fOwner->AddNotification(this);
}

MMNotification::~MMNotification() {
	for (tReceiverIterator it = fReceivers.begin(); it != fReceivers.end(); it++)
		it->fReceiver->NewNotification(NULL);

	if (fOwner)
		fOwner->RemoveNotification(this);
}

//	Selectively set or clear notificiation bits.
//	Wherever mask is 0, leave existing bits untouched.
//	Wherever mask is 1, set bit equivalent to flags.
void MMNotification::NotifyMe(MMNotificationReceiver* receiver, tNotificationFlags flags, tNotificationFlags mask) {
	for (tReceiverIterator it = fReceivers.begin(); it != fReceivers.end(); it++) {
		if (it->fReceiver == receiver) {
			it->fMask = (it->fMask & ~mask) | (flags & mask);
			receiver->NewNotification(this);
			return;
		}
	}

	tReceiverEntry newEntry;
	newEntry.fReceiver = receiver;
	newEntry.fMask = flags;
	fReceivers.push_back(newEntry);

	receiver->NewNotification(this);
}

void MMNotification::CancelNotification(MMNotificationReceiver *receiver) {
	for (tReceiverIterator it = fReceivers.begin(); it != fReceivers.end(); it++)
		if (it->fReceiver == receiver)
			fReceivers.erase(it);
}

void MMNotification::SetNotificationFlags(tNotificationFlags flags, tNotificationFlags mask) {
	fCurrentFlags = (fCurrentFlags & ~mask) | flags;
}

void MMNotification::CheckReceivers() {	
	tNotificationFlags currentFlags = fCurrentFlags;
	fCurrentFlags = kNoNotificationFlags;

	for (tReceiverIterator it = fReceivers.begin(); it != fReceivers.end(); it++)
		if (it->fMask & currentFlags)
			it->fReceiver->ReceiveNotification(this, currentFlags);
}

//	Receiver entries are equal if their receivers are equal.

int operator==(const tReceiverEntry &entry1, const tReceiverEntry &entry2) {
	return	entry1.fReceiver == entry2.fReceiver;
}

int operator!=(const tReceiverEntry &entry1, const tReceiverEntry &entry2) {
	return	entry1.fReceiver != entry2.fReceiver;
}

} // End of namespace Pegasus
