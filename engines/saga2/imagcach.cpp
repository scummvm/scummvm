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

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/imagcach.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

CImageNode::CImageNode(hResContext *con, uint32 resID) {
	if (con) {
		image           = LoadResource(con, resID, "CImageNode Allocation");
		resourceID      = resID;
		contextID       = con->getResID();
		requested       = 0;    // zero request for this node at creation
	} else {
		image = nullptr;
		resourceID = 0;
		contextID = 0;
		requested = 0;
	}
}

CImageNode::~CImageNode() {
	if (image) {
		free(image);
		image = nullptr;
	}
}

// figures out if the requested image is the same as this one
bool CImageNode::isSameImage(hResContext *con, uint32 resID) {
	if (con) {
		if (con->getResID() == contextID &&
		        resourceID == resID) {
			return true;    // match
		}
	}

	return false;   // no match
}

bool CImageNode::isSameImage(void *imagePtr) {
	// if the image passed has the same address as the image in the node...
	if (imagePtr == image) {
		return true;
	}

	return false;
}

// return true if this node needs to be deleted
bool CImageNode::releaseRequest() {
	// the number of requests on this resource goes down by one
	requested--;

	// if that was the last request, release this node
	if (requested <= 0) {
		return true;
	}

	// not the last request, keep image
	return false;
}

void *CImageNode::getImagePtr() {
	requested++;
	return image;
}

/* ===================================================================== *
   ImageCache member functions
 * ===================================================================== */

CImageCache::~CImageCache() {

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
	Common::List<CImageNode *>::iterator nextIt;

	for (Common::List<CImageNode *>::iterator it = _nodes.begin(); it != _nodes.end(); it = nextIt) {
		nextIt = it;
		nextIt++;

		imageNode = *it;
		if (imageNode->isSameImage(imagePtr)) {
			// if that was the last request for the imageNode, delete it
			if (imageNode->releaseRequest()) {
				// remove and delete it
				_nodes.remove(imageNode);
				delete imageNode;
			}
		}
	}
}

void *CImageCache::requestImage(hResContext *con, uint32 resID) {
	CImageNode *imageNode;

	// look through all nodes to see if we have that image already
	for (Common::List<CImageNode *>::iterator it = _nodes.begin(); it != _nodes.end(); it++) {
		imageNode = *it;

		if (imageNode->isSameImage(con, resID)) {
			// return the image Ptr to the already allocated image resource
			return imageNode->getImagePtr();
		}
	}

	// if no previously allocated image node then make one and return the
	// ptr to the new image resource
	// creates node and loads in the resource
	imageNode = new CImageNode(con, resID);

	// add this node to the list
	_nodes.push_back(imageNode);

	// return the newly loaded image
	return imageNode->getImagePtr();
}

} // end of namespace Saga2
