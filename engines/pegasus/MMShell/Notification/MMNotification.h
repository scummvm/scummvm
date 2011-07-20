/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_MMSHELL_NOTIFICATION_MMNOTIFICATION_H
#define PEGASUS_MMSHELL_NOTIFICATION_MMNOTIFICATION_H

#include "common/list.h"

#include "pegasus/MMShell/MMTypes.h"
#include "pegasus/MMShell/Utilities/MMIDObject.h"

namespace Pegasus {

class MMNotificationManager;
class MMNotificationReceiver;

struct tReceiverEntry {
	MMNotificationReceiver *fReceiver;
	tNotificationFlags fMask;
};

int operator==(const tReceiverEntry &entry1, const tReceiverEntry &entry1);
int operator!=(const tReceiverEntry &entry1, const tReceiverEntry &entry1);

typedef Common::List<tReceiverEntry> tReceiverList;

/*
	A notification can have 32 flags associated with it, which can be user-defined.
*/

class MMNotification : public MMIDObject {
friend class MMNotificationManager;

public:
	MMNotification(const tNotificationID id, MMNotificationManager *owner);
	virtual ~MMNotification();

	//	NotifyMe will have this receiver notified when any of the specified notification
	//	flags are set.
	//	If there is already a notification set for this receiver, NotifyMe does a bitwise
	//	OR with the receiver's current notification flags.

	//	Can selectively set or clear notification bits by using the flags and mask argument.

	void NotifyMe(MMNotificationReceiver*, tNotificationFlags flags, tNotificationFlags mask);
	void CancelNotification(MMNotificationReceiver *receiver);
	
	void SetNotificationFlags(tNotificationFlags flags,	tNotificationFlags mask);
	tNotificationFlags GetNotificationFlags() { return fCurrentFlags; }
	
	void ClearNotificationFlags() { SetNotificationFlags(0, ~(tNotificationFlags)0); }

protected:
	void CheckReceivers();
	
	MMNotificationManager *fOwner;
	tReceiverList fReceivers;
	tNotificationFlags fCurrentFlags;
};

} // End of namespace Pegasus

#endif
