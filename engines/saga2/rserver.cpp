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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/dlist.h"
#include "saga2/fta.h"
#include "saga2/ioerrors.h"


namespace Saga2 {

class ResourceRequest;

class ResourceServer : public DList {
	HR_FILE         *fHandle;               // resource file handle
	int32           lastSeekPos;            // where drive head is
	int32           actual;                 // bytes read so far

	ResourceRequest *currentRequest;

public:
	ResourceServer(HR_FILE *);               // constructor
	void service(void);
};

class ResourceRequest : public DNode {
public:
	RHANDLE         handle;                 // where to put the data
	uint32          offset,                 // offset in file of data
	                length;                 // desired length of data
	bool            done;                   // TRUE if load is finished
	void            *userData;              // for app use

	//  Function to notify when request is done.
	void (*notify)(ResourceRequest &);

	ResourceRequest(DList &dl);      // constructor
};

/* ===================================================================== *
   Globals
 * ===================================================================== */

ResourceServer      *resourceServer;        // resource server ptr
DList               resourceRequestPool;    // pool of messages

const int           numResRequests = 32;    // up to 32 messages allowed

/* ===================================================================== *
   Member Functions
 * ===================================================================== */
ResourceServer::ResourceServer(HR_FILE *fh) {
	currentRequest = NULL;
	fHandle = fh;                           // file handle
	lastSeekPos = 0;                        // drive position
}

const int           loadQuanta = 0x4000;    //  16K

void ResourceServer::service(void) {
	if (currentRequest == NULL) {
		currentRequest = (ResourceRequest *)remHead();

		if (currentRequest == NULL) return;

		// seek to position in file
		HR_SEEK(fHandle, currentRequest->offset, SEEK_SET);

		// calculate final seek position
		lastSeekPos = currentRequest->offset + currentRequest->length;
		actual = 0;                         // bytes read so far
	}

	if (currentRequest->length > 0           // while there's data to read
	        &&  *currentRequest->handle != NULL) {  // and block not flushed
		int32       loadSize = MIN<uint>(currentRequest->length, loadQuanta);
		uint8       *buffer = (UBytePtr) * currentRequest->handle + actual;

		//  Read 16K worth of data, or however much is left.
		if (HR_READ(buffer, loadSize, 1, fHandle) != 1)
			error("Error reading resource");

		buffer += loadSize;
		currentRequest->length -= loadSize;
		currentRequest->offset += loadSize;
		actual += loadSize;

#if DEBUG
		WriteStatusF(1, "Loaded: %8.8d", actual);
#endif
	} else {
		currentRequest->done = TRUE;
		resourceRequestPool.addTail(*currentRequest);
		currentRequest = NULL;

		//  Mark handle as ready for use.
		RHandleDoneLoading(currentRequest->handle);              // mark handle as loaded

		//  Notify callback that resource is done loading
		if (currentRequest->notify)
			currentRequest->notify(*currentRequest);
		currentRequest->notify = NULL;
	}
}

ResourceRequest::ResourceRequest(DList &dl) {
	notify = NULL;
	dl.addTail(*this);
}

void RequestResource(
    RHANDLE         handle,
    int32           offset,
    int32           length,
    //  Function to notify when request is done.
    void            *notify,
    void            *userData) {
	ResourceRequest *rr;

	//  Try to get a resource request. If none are available,
	//  then wait until one is avauilable.

	for (;;) {
		rr = (ResourceRequest *)
		     resourceRequestPool.remHead();

		if (rr != NULL) break;

		//  Service resources until request is free
		resourceServer->service();
	}

	RHandleStartLoading(handle);

	rr->done   = FALSE;
	rr->handle = handle;
	rr->offset = offset;
	rr->length = length;
	rr->notify = (void (*)(ResourceRequest &))notify;
	rr->userData = userData;

	resourceServer->addTail(*rr);
}

void initServers(void) {
	warning("STUB: initServers()");
#if 0
	int16           i;
	resourceServer = NEW_PRES ResourceServer(resFile->resFileHandle());
	if (resourceServer == NULL) {
		error("Unable to start up resource server!\n");
	}

	for (i = 0; i < numResRequests; i++) {
		NEW_PRES ResourceRequest(resourceRequestPool);
	}
#endif
}

void cleanupServers(void) {
	ResourceRequest *rr;
	if (resourceServer)
		delete resourceServer;
	resourceServer = NULL;
	while ((rr = (ResourceRequest *)resourceRequestPool.remHead()))
		delete rr;
}

void loadAsyncResources(void) {
	resourceServer->service();
}

void syncResources(void) {
	while (resourceServer->count() > 0)
		resourceServer->service();
}

void *lockResource(RHANDLE h) {
	if (h && *h) {
		while (RHandleLoading(h)) resourceServer->service();
		return RLockHandle(h);
	}
	return NULL;
}

void unlockResource(RHANDLE handle) {
	RUnlockHandle(handle);
}

} // end if namespace Saga2
