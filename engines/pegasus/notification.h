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

#ifndef PEGASUS_NOTIFICATION_H
#define PEGASUS_NOTIFICATION_H

#include "common/list.h"

#include "pegasus/types.h"
#include "pegasus/util.h"

namespace Pegasus {

class NotificationManager;
class NotificationReceiver;

struct tReceiverEntry {
	NotificationReceiver *receiver;
	tNotificationFlags mask;
};

int operator==(const tReceiverEntry &entry1, const tReceiverEntry &entry2);
int operator!=(const tReceiverEntry &entry1, const tReceiverEntry &entry2);

typedef Common::List<tReceiverEntry> tReceiverList;

/*
	A notification can have 32 flags associated with it, which can be user-defined.
*/

class Notification : public IDObject {
friend class NotificationManager;

public:
	Notification(const tNotificationID id, NotificationManager *owner);
	virtual ~Notification();

	//	NotifyMe will have this receiver notified when any of the specified notification
	//	flags are set.
	//	If there is already a notification set for this receiver, NotifyMe does a bitwise
	//	OR with the receiver's current notification flags.

	//	Can selectively set or clear notification bits by using the flags and mask argument.

	void notifyMe(NotificationReceiver*, tNotificationFlags flags, tNotificationFlags mask);
	void cancelNotification(NotificationReceiver *receiver);
	
	void setNotificationFlags(tNotificationFlags flags,	tNotificationFlags mask);
	tNotificationFlags getNotificationFlags() { return _currentFlags; }
	
	void clearNotificationFlags() { setNotificationFlags(0, ~(tNotificationFlags)0); }

protected:
	void checkReceivers();
	
	NotificationManager *_owner;
	tReceiverList _receivers;
	tNotificationFlags _currentFlags;
};

class NotificationReceiver {
friend class Notification;

public:
	NotificationReceiver();
	virtual ~NotificationReceiver();
	
protected:
	//	ReceiveNotification is called automatically whenever a notification that this
	//	receiver depends on has its flags set
	
	virtual void receiveNotification(Notification *, const tNotificationFlags);
	virtual void newNotification(Notification *notification);

private:
	Notification *_notification;
};

typedef Common::List<Notification *> tNotificationList;

class NotificationManager : public NotificationReceiver {
friend class Notification;

public:
	NotificationManager();
	virtual ~NotificationManager();
	
	void checkNotifications();

protected:
	void addNotification(Notification *notification);
	void removeNotification(Notification *notification);
	void detachNotifications();
	
	tNotificationList _notifications;
};

} // End of namespace Pegasus

#endif
