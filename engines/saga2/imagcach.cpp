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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"
#include "saga2/fta.h"
#include "saga2/dlist.h"
#include "saga2/imagcach.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

#if DEBUG
extern char *idname(long s);
#endif

CImageNode::CImageNode(hResContext *con, uint32 resID) {
	if (con) {
#if DEBUG
		char            descr[ 32 ];

		sprintf(descr, "CImage '%s'", idname(resID));

		image           = LoadResource(con, resID, descr);
#else
		image           = LoadResource(con, resID, "CImageNode Allocation");
#endif
		resourceID      = resID;
		contextID       = con->getResID();
		requested       = 0;    // zero request for this node at creation
	}
}

CImageNode::~CImageNode(void) {
	if (image) {
		RDisposePtr(image);
		image = NULL;
	}
}

// figures out if the requested image is the same as this one
bool CImageNode::isSameImage(hResContext *con, uint32 resID) {
	if (con) {
		if (con->getResID() == contextID &&
		        resourceID == resID) {
			return TRUE;    // match
		}
	}

	return FALSE;   // no match
}

bool CImageNode::isSameImage(void *imagePtr) {
	// if the image passed has the same address as the image in the node...
	if (imagePtr == image) {
		return TRUE;
	}

	return FALSE;
}

// return TRUE if this node needs to be deleted
bool CImageNode::releaseRequest(void) {
	// the number of requests on this resource goes down by one
	requested--;

	// if that was the last request, release this node
	if (requested <= 0) {
		return TRUE;
	}

	// not the last request, keep image
	return FALSE;
}

void *CImageNode::getImagePtr(void) {
	requested++;
	return image;
}

/* ===================================================================== *
   ImageCache member functions
 * ===================================================================== */

CImageCache::~CImageCache(void) {

	/* >>> See notes below
	    // return if list is empty
	if( nodes.empty() ) return;


	CImageNode *imageNode, *nextImageNode;


	    // for some reason, doing a nodes.last() ( or nodes.first() )
	    // here causes an access viloation.  This might be caused
	    // by list corruption occuring during runtime,
	    // I'm going to disable destructor for now to run some tests
	    // to determine cause.  This should not adversely affect normal runtime
	    // execution as all nodes should be released DURING runtime.
	for( imageNode = ( CImageNode * )nodes.last();
	     imageNode != NULL;
	     imageNode = nextImageNode )
	{

	        // keep a copy of the pointer
	    nextImageNode = ( CImageNode *)imageNode->prev();

	        // remove and delete it
	    imageNode->remove();
	    delete imageNode;

	}
	*/
}

void CImageCache::releaseImage(void *imagePtr) {
	if (!imagePtr)  return;

	CImageNode *imageNode;
	CImageNode *prevImageNode;


	for (imageNode = (CImageNode *)nodes.last();
	        imageNode != NULL;
	        imageNode = prevImageNode) {
		prevImageNode = (CImageNode *)imageNode->prev();

		if (imageNode->isSameImage(imagePtr)) {
			// if that was the last request for the imageNode, delete it
			if (imageNode->releaseRequest()) {
				// remove and delete it
				imageNode->remove();
				delete imageNode;
			}
		}
	}
}

void *CImageCache::requestImage(hResContext *con, uint32 resID) {
	CImageNode *imageNode;

	// look through all nodes to see if we have that image already
	for (imageNode = (CImageNode *)nodes.last();
	        imageNode;
	        imageNode = (CImageNode *)imageNode->prev()) {
		if (imageNode->isSameImage(con, resID)) {
			// return the image Ptr to the already allocated image resource
			return imageNode->getImagePtr();
		}
	}

	// if no previously allocated image node then make one and return the
	// ptr to the new image resource
	// creates node and loads in the resource
	imageNode = new CImageNode(con, resID);
	checkAlloc(imageNode);

	// add this node to the list
	nodes.addTail(*imageNode);

	// return the newly loaded image
	return imageNode->getImagePtr();
}

// global declarations
CImageCache ImageCache;

} // end of namespace Saga2
