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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_IMAGCACH_H
#define SAGA2_IMAGCACH_H

namespace Saga2 {

void initImageCache(void);
void cleanupImageCache(void);

/* ===================================================================== *
   ImageNode class which defines a re-entrant image resource
 * ===================================================================== */

class CImageNode : public DNode {
private:
	uint32      contextID;  // ID of context
	uint32      resourceID;     // RES_ID of  image

	uint16  requested;  // the number of allocation requests made to node
	void    *image;     // the image

public:
	CImageNode(hResContext *con, uint32 resID);
	~CImageNode(void);

	void    *getImagePtr(void);
	bool    isSameImage(hResContext *con, uint32 resID);
	bool    isSameImage(void *imagePtr);
	uint16  getNumRequested(void) {
		return requested;
	}
	bool    releaseRequest(void);
};


/* ===================================================================== *
   ImageCache class which maintains a list of ImageNodes
 * ===================================================================== */

class CImageCache {
private:
	DList nodes;    // list of ImageNode

public:
	CImageCache(void) {
		assert(nodes.empty());
	}
	~CImageCache(void);

	void *requestImage(hResContext *con, uint32 resID);
	void releaseImage(void *);
};

} // end of namespace Saga2

#endif
