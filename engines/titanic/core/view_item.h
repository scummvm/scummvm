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

#ifndef TITANIC_VIEW_ITEM_H
#define TITANIC_VIEW_ITEM_H

#include "titanic/core/link_item.h"
#include "titanic/core/named_item.h"
#include "titanic/core/resource_key.h"
#include "titanic/messages/mouse_messages.h"

namespace Titanic {

class CViewItem : public CNamedItem {
	DECLARE_MESSAGE_MAP;
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	bool MouseMoveMsg(CMouseMoveMsg *msg);
	bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	bool MovementMsg(CMovementMsg *msg);
private:
	CTreeItem *_buttonUpTargets[4];
private:
	/**
	 * Sets the angle of the view relative to the node it belongs to
	 */
	void setAngle(double angle);

	/**
	 * Called to handle mouse messagaes on the view
	 */
	bool handleMouseMsg(CMouseMsg *msg, bool flag);

	/**
	 * Handles mouse button up messages
	 */
	void handleButtonUpMsg(CMouseButtonUpMsg *msg);

	/**
	 * Returns the item in the view at a given point that will
	 * receive any mouse click
	 */
	CTreeItem *getItemAtPoint(const Point &pt);
protected:
	int _field24;
	CResourceKey _resourceKey;
	Point _viewPos;
public:
	int _viewNumber;
	double _angle;
public:
	CLASSDEF;
	CViewItem();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	/**
	 * Get the resource key for the view
	 */
	bool getResourceKey(CResourceKey *key);

	/**
	 * Called when leaving the view
	 */
	void leaveView(CViewItem *newView);

	/**
	 * Called on an old view just left, and about to enter a new view
	 */
	void preEnterView(CViewItem *newView);

	/**
	 * Called when a new view is being entered
	 */
	void enterView(CViewItem *newView);

	/**
	 * Finds a link which connects to another designated view
	 */
	CLinkItem *findLink(CViewItem *newView);

	/**
	 * Return the full Id of the current view in a
	 * room.node.view tuplet form
	 */
	CString getFullViewName() const;

	/**
	 * Return the Id of the current view in a
	 * room.node.view tuplet form
	 */
	CString getNodeViewName() const;

	/**
	 * Gets the relative position of the view within the owning room
	 */
	void getPosition(double &xp, double &yp, double &zp);
};

} // End of namespace Titanic

#endif /* TITANIC_NAMED_ITEM_H */
