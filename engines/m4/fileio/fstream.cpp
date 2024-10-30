/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/fileio/fstream.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/vars.h"

namespace M4 {

#define STR_STRMREQ "stream request"

StreamFile::StreamFile(const Common::Path &filename) {
	if (!_file.open(filename))
		error("Could not open - %s", filename.toString().c_str());
}

int32 StreamFile::read(Handle bufferHandle, int32 n) {
	return _file.read(bufferHandle, n);
}

bool StreamFile::seek(uint32 n) {
	return _file.seek(n);
}

bool StreamFile::seek_ahead(int32 n) {
	return _file.skip(n);
}

uint32 StreamFile::get_pos() {
	return _file.pos();
}

bool f_stream_Init() {
	_G(firstStream) = nullptr;
	_G(lastStream) = nullptr;
	return true;
}

void f_stream_Shutdown(void) {
	strmRequest *myStream;

	// Loop through the list, closing all stream requests, which also deallocs the request
	myStream = _G(firstStream);
	while (myStream) {
		_G(firstStream) = _G(firstStream)->next;
		f_stream_Close(myStream);
		myStream = _G(firstStream);
	}
}

strmRequest *f_stream_Open(SysFile *srcFile, int32 fileOffset, int32 strmMinBuffSize, int32 strmBuffSize,
	int32 numBlocksToRead, int32 *blockSizeArray, int32 initialRead, bool wrapStream) {

	strmRequest *newStream;
	int32 bytesRead, i, bytesToRead;
	bool finished;
	int32 memAvail;

	// Parameter verification        
	if (!srcFile) {
		error_show(FL, 'FSF!');
	}

	if (strmMinBuffSize < 0) {
		error_show(FL, 'FSF1', "neg min buffsize: %d", strmMinBuffSize);
	}

	// Allocate a new stream request struct
	if ((newStream = (strmRequest *)mem_alloc(sizeof(strmRequest), STR_STRMREQ)) == nullptr) {
		error_show(FL, 'OOM!', "%d", sizeof(strmRequest));
		return nullptr;
	}

	// Try to get memory
	newStream->strmHandle = NewHandle(strmBuffSize, "stream buff");
	if (newStream->strmHandle) {
		goto got_mem;
	}

	// Maximize available memory
	MaxMem((Size *)&memAvail);	// param on PC is max mem avail in one block

	// try to get requested size
	if (memAvail >= strmBuffSize) {
		/*
				if ((newStream->strmBuff = (uint8*)mem_alloc(strmBuffSize, STR_STRMBUFF)) != nullptr) {
					goto got_mem;
				}
		*/
		// try to get memory
		newStream->strmHandle = NewHandle(strmBuffSize, "stream buff");
		if (newStream->strmHandle) {
			goto got_mem;
		}
	}

	// try to get what's left if it's enough
	// get a compromise between free and requested.
	// if we get it all, system gets unstable...
	if (memAvail > strmMinBuffSize) {
		int32 alloc_me = ((memAvail - strmMinBuffSize) / 2) + strmMinBuffSize;
		/*
				if ((newStream->strmBuff = (uint8*)mem_alloc(alloc_me, STR_STRMBUFF)) != nullptr) {
					strmBuffSize = alloc_me;
					goto got_mem;
				}
		*/
		// try to get memory
		newStream->strmHandle = NewHandle(alloc_me, "stream buff");
		if (newStream->strmHandle) {
			strmBuffSize = alloc_me;
			goto got_mem;
		}

	}

	// sorry, bud.
	error_show(FL, 'FSOM', "want: %d, have: %d", strmMinBuffSize, memAvail);

got_mem:
	//lock the buffer - to be locked until the stream is closed
	HLock(newStream->strmHandle);
	newStream->strmBuff = (uint8 *) * (newStream->strmHandle);

	// Initialize the stream request
	newStream->strmSize = strmBuffSize;
	newStream->strmHead = newStream->strmBuff;
	newStream->strmTail = newStream->strmBuff;
	newStream->endStrmBuff = newStream->strmBuff + strmBuffSize;
	newStream->strmWrap = newStream->endStrmBuff;
	newStream->strmLastRead = newStream->endStrmBuff;
	newStream->numBlocksToRead = numBlocksToRead;
	newStream->blockSizeArray = blockSizeArray;
	newStream->wrapStream = wrapStream;
	newStream->srcFile = srcFile;

	// If the streaming should begin part way into the file, seek to the beginning of where to start streaming
	if (fileOffset > 0) {
		// If (fseek(newStream->srcFile, fileOffset, SEEK_SET) != 0) {
		if (!newStream->srcFile->seek(fileOffset)) {
			delete newStream->srcFile;
			mem_free(newStream);
			return nullptr;
		}
	}

	// Check if we are to initially read the stream
	if (initialRead > 0) {
		// If the blockSizeArray exists, then initialRead is the number of blocks to read
		if (newStream->blockSizeArray) {
			// Calculate the total number of bytes to read in initially
			initialRead = (int32)imath_min(initialRead, numBlocksToRead);
			finished = false;
			bytesToRead = 0;
			i = 0;
			while ((i < initialRead) && (!finished)) {
				if ((bytesToRead + blockSizeArray[i]) <= strmBuffSize) {
					bytesToRead += blockSizeArray[i];
					i++;
				} else {
					finished = true;
				}
			}

			// Update the blockSizeArray, and numBlocksToRead entries.  We plan to read in "i" blocks so far.
			newStream->numBlocksToRead -= i;
			newStream->blockSizeArray += i;
		}

		// Else the initialRead refers to the number of bytes to initially read
		else {

			//bounds check the initialRead and set the nextReadSize field
			bytesToRead = (int32)imath_min(initialRead, strmBuffSize);
			newStream->nextReadSize = bytesToRead;
		}

		// Make sure we still have something to read
		if (bytesToRead > 0) {

			// Read in the initial bytes to read
			bytesRead = newStream->srcFile->read(newStream->strmHead, bytesToRead);

			//did we actually read that many?  If not, close the file
			if (bytesRead < bytesToRead) {
				delete newStream->srcFile;
				newStream->srcFile = nullptr;
			}

			// Update the strmHead pointer
			newStream->strmHead += bytesRead;
		}
	}

	//link the stream request into the list of requests
	newStream->prev = nullptr;
	newStream->next = _G(firstStream);
	if (_G(firstStream)) {
		_G(firstStream)->prev = newStream;
	} else {
		_G(lastStream) = newStream;
	}
	_G(firstStream) = newStream;

	// Return the stream request
	return newStream;
}


static bool UnwrapStream(strmRequest *myStream) {
	int32 bytesToMove = 0, bytesAvail;
	uint8 *tempBuff;

	// Using tempBuff as a flag to determine whether data needs to be temporarily stored
	tempBuff = nullptr;

	// Since strmTail is never allowed to be equal to strmWrap if it is > strmHead, there must be a
	// Non-zero amount of data at the end which we must move.
	//Therefore, we may have to temporarily store anything at the beginning of the buffer
	if (myStream->strmHead > myStream->strmBuff) {

		// Calculate how many bytes to store and copy to a temporary buffer
		bytesToMove = (byte *)myStream->strmHead - (byte *)myStream->strmBuff;

		if ((tempBuff = (uint8 *)mem_alloc(bytesToMove, "stream temp buff")) == nullptr)
			error_show(FL, 'OOM!', "UnwrapStream() failed - temp buff avail: %d", bytesToMove);

		memcpy(tempBuff, myStream->strmBuff, bytesToMove);
	}

	// Move the data at the end of the buffer to the beginning and reset the strmWrap pointer
	bytesAvail = (byte *)myStream->strmWrap - (byte *)myStream->strmTail;
	memmove(myStream->strmBuff, myStream->strmTail, bytesAvail);
	myStream->strmTail = myStream->strmBuff;
	myStream->strmHead = (uint8 *)((byte *)(myStream->strmTail) + bytesAvail);
	myStream->strmWrap = myStream->endStrmBuff;

	// Now check if we temporarily store data. if so, copy it back to the stream and turf the temp buffer
	if (tempBuff) {
		memcpy(myStream->strmHead, tempBuff, bytesToMove);
		myStream->strmHead += bytesToMove;
		mem_free(tempBuff);
	}
	return true;
}


void f_stream_DumpPreviouslyRead(strmRequest *myStream) {
	// This is used to allow the f_stream_Process() function to overwrite the stream buffer space where
	// the previously read data was stored.  ie.  If you call f_stream_Read(), and then make a copy,
	// you wouldn't care if the data in the stream buffer was overwritten, so call this procedure.
	if (myStream) {
		myStream->strmLastRead = myStream->strmTail;
		if (myStream->strmTail == myStream->strmHead) {
			myStream->strmTail = myStream->strmBuff;
			myStream->strmHead = myStream->strmBuff;
		}
	}
}


int32 f_stream_Read(strmRequest *myStream, uint8 **dest, int32 numBytes) {
	int32   bytesAvail, bytesNeeded, bytesRead;

	// Parameter verification
	if (!myStream)
		error_show(FL, 'FSIS', "f_stream_Read() failed - invalid stream request");

	if ((numBytes <= 0) || (numBytes >= myStream->strmSize))
		error_show(FL, 'FSR!', "%d stream size %d", numBytes, myStream->strmSize);

	// If the stream tail is > the stream head, and the number of bytes at the end of the buffer is < numBytes
	// we must unwrap the stream, moving the data at the end of the buffer to the beginning, and slide the beginning down
	if ((myStream->strmTail > myStream->strmHead) && (((byte *)myStream->strmWrap - (byte *)myStream->strmTail) < numBytes)) {
		UnwrapStream(myStream);
	}

	// Now either the strmHead is >= the strmTail, or there is enough data at the end of the buffer to fulfill numBytes      

	// Calculate the number of bytes available
	if (myStream->strmTail <= myStream->strmHead) {
		bytesAvail = (int32)(myStream->strmHead - myStream->strmTail);
	} else {
		// No extra data is available at the beginning of the stream buffer, since we "unwrapped" the stream
		bytesAvail = (int32)(myStream->strmWrap - myStream->strmTail);
	}

	// Now check and see if we have enough bytes available
	if (bytesAvail >= numBytes) {
		// Set the destination pointer
		*dest = (uint8 *)myStream->strmTail;
		myStream->strmLastRead = myStream->strmTail;

		// Update the strmTail pointer
		myStream->strmTail += numBytes;

		// If there is no data left at the end of the stream buffer, reset the strmTail and strmWrap pointers
		if (myStream->strmTail == myStream->strmWrap) {
			myStream->strmTail = myStream->strmBuff;
			myStream->strmWrap = myStream->endStrmBuff;
		}

		return numBytes;
	} else {
		// Else we will have to read more data from disc
		// If this has happened, since we "unwrapped" the stream buff, we can guarantee that strmTail < strmHead

		// Calculate how much more must be read in
		bytesNeeded = numBytes - bytesAvail;

		// Make sure we have enough room at the end of the buffer to accommodate
		if ((int32)(myStream->endStrmBuff - myStream->strmHead) < bytesNeeded) {
			// We need to memmove the contents of the stream to the beginning of the buff to allow
			// F_stream_read() to return a pointer to a contiguous block

			// Move the data to the beginning of the stream buffer, and reset the head and tail pointers
			memmove((void *)myStream->strmBuff, myStream->strmTail, bytesAvail);
			myStream->strmTail = myStream->strmBuff;
			myStream->strmHead = (uint8 *)((byte *)myStream->strmTail + bytesAvail);
		}

		// If the client is using a blockSizeArray, hopefully bytesNeeded will be equal to the next blockSize
		if (myStream->blockSizeArray && (*myStream->blockSizeArray == bytesNeeded) && (myStream->numBlocksToRead > 0)) {
			myStream->blockSizeArray++;
			myStream->numBlocksToRead--;
		} else {
			// Otherwise we just trashed the whole point of using a calculated blockSizeArray
			myStream->blockSizeArray = nullptr;
			myStream->numBlocksToRead = -1;
			myStream->nextReadSize = numBytes;
		}

		// Read in the bytesNeeded
		bytesRead = myStream->srcFile->read(myStream->strmHead, bytesNeeded);

		if (bytesRead < bytesNeeded) {
			// If we could not read that much in, close the srcFile
			delete myStream->srcFile;
			myStream->srcFile = nullptr;
		}

		// Set the destination pointer and update the stream pointers
		*dest = (uint8 *)myStream->strmTail;
		myStream->strmLastRead = myStream->strmTail;
		myStream->strmHead += bytesRead;
		myStream->strmTail = myStream->strmHead;

		// Return the number of bytes successfully available
		return (bytesRead + bytesAvail);
	}
}

void f_stream_Close(strmRequest *myStream) {
	// Parameter verification
	if (!myStream) {
		return;
	}

	// Close the stream and throw out the stream buffer
	if (myStream->srcFile) {
		delete myStream->srcFile;
	}

	// Kill the stream buffer
	HUnLock(myStream->strmHandle);
	DisposeHandle(myStream->strmHandle);
	myStream->strmBuff = nullptr;


	// Remove the stream request from the list of requests
	if (myStream->next) {
		myStream->next->prev = myStream->prev;
	} else {
		_G(lastStream) = myStream->prev;
	}
	if (myStream->prev) {
		myStream->prev->next = myStream->next;
	} else {
		_G(firstStream) = myStream->next;
	}

	// Final, turf the stream request
	mem_free(myStream);
}

void f_stream_Process(int32 numToProcess) {
	strmRequest *myStream;
	int32 buffEndBytesAvail = 0, buffStartBytesAvail = 0;
	int32 bytesRead, bytesAvail, nextReadSize;
	bool buffWrap, useBlockSizeArray;

	// No sense wasting time if there are no stream requests to process
	if (!_G(firstStream)) {
		return;
	}

	// Loop through until either the end of the list of requests, or we've serviced the "numToProcess"
	myStream = _G(firstStream);
	while (myStream && (numToProcess > 0)) {

		// Make sure we still have an open srcFile
		if (myStream->srcFile && (myStream->numBlocksToRead != 0)) {
			buffWrap = false;
			useBlockSizeArray = false;

			// Calculate the amount of empty space in the stream buff
			// If all the empty space in the stream buff is between the head and the lastRead...
			if (myStream->strmLastRead >= myStream->strmHead) {
				bytesAvail = (byte *)myStream->strmLastRead - (byte *)myStream->strmHead;

				// strmTail and strmHead can never equal unless the buffer is completely empty, therefore,
				// make sure the amout of bytes available won't cause strmHead to become equal to strmTail
				if ((bytesAvail > 0) && (myStream->strmLastRead == myStream->strmTail)) {
					bytesAvail--;
				}
			} else {
				// Else all the empty space is wrapped around the end of the buffer
				buffWrap = true;

				// Calculate how much space is available at the start and at the end of the buffer
				buffEndBytesAvail = (byte *)myStream->endStrmBuff - (byte *)myStream->strmHead;
				buffStartBytesAvail = (byte *)myStream->strmLastRead - (byte *)myStream->strmBuff;

				// As above, ensure strmHead won't become equal to strmTail
				if ((buffStartBytesAvail > 0) && (myStream->strmLastRead == myStream->strmTail)) {
					buffStartBytesAvail--;
				}

				// Calculate the total bytes available
				bytesAvail = buffEndBytesAvail + buffStartBytesAvail;
			}

			// Now find the number of bytes to read - either from the blockSizeArray...
			if (myStream->blockSizeArray) {
				useBlockSizeArray = true;
				nextReadSize = *myStream->blockSizeArray;
			} else {
				// ...or directly from the nextReadSize field of the stream request
				nextReadSize = myStream->nextReadSize;
			}

			// See if we can simply read the next chunk into the strmHead, without worrying about "wrapping" the buffer
			if ((buffWrap && (buffEndBytesAvail >= nextReadSize)) ||
					((!buffWrap) && (bytesAvail >= nextReadSize))) {
				// Read the bytes into the stream buffer 
				bytesRead = myStream->srcFile->read(myStream->strmHead, nextReadSize);

				// If we could not read that much in, close the srcFile
				if (bytesRead < nextReadSize) {
					delete myStream->srcFile;
					myStream->srcFile = nullptr;
				}

				// Update the stream head
				myStream->strmHead += bytesRead;

				// Update the blockSizeArray pointer if necessary
				if (useBlockSizeArray) {
					myStream->blockSizeArray++;
					myStream->numBlocksToRead--;
				}
			} else if (buffWrap) {
				// Else if the empty space is wrapped, we may still be able to store the next data chunk, otherwise no more room

				// See if we can wrap the next data chunk around 
				if (!myStream->wrapStream) {

					// No wrapping allowed, so do we have room for it at the beginning of the stream
					if (buffStartBytesAvail >= nextReadSize) {

						//we can read it in at the beginning, so set the strmWrap pointer
						myStream->strmWrap = myStream->strmHead;

						// Read the bytes into the stream buffer 
						bytesRead = myStream->srcFile->read(myStream->strmBuff, nextReadSize);

						// If we could not read that much in, close the srcFile
						if (bytesRead < nextReadSize) {
							delete myStream->srcFile;
							myStream->srcFile = nullptr;
						}

						// Update the stream head
						myStream->strmHead = (uint8 *)((byte *)myStream->strmBuff + bytesRead);

						// Update the blockSizeArray pointer if necessary
						if (useBlockSizeArray) {
							myStream->blockSizeArray++;
							myStream->numBlocksToRead--;
						}
					}
				} else if (bytesAvail >= nextReadSize) {
					// Else we might have to read in part at the end, and part at the beginning of the stream buffer

					// Read into the end of the stream buffer
					if (buffEndBytesAvail > 0) {

						// Read into the end of the buffer
						bytesRead = (int32)myStream->srcFile->read(myStream->strmHead, buffEndBytesAvail);

						// If we could not read that much in, close the srcFile and update the head pointer
						if (bytesRead < buffEndBytesAvail) {
							delete myStream->srcFile;
							myStream->srcFile = nullptr;
							myStream->strmHead += bytesRead;
						}
					}

					// Make sure we didn't close the srcFile in the last read
					if (myStream->srcFile) {

						// Read into the beginning of the buffer
						bytesRead = myStream->srcFile->read(myStream->strmBuff, nextReadSize - buffEndBytesAvail);

						// If we could not read that much in, close the srcFile
						if (bytesRead < (nextReadSize - buffEndBytesAvail)) {
							delete myStream->srcFile;
							myStream->srcFile = nullptr;
						}

						// Update the head pointer
						myStream->strmHead = (uint8 *)((byte *)myStream->strmBuff + bytesRead);

						// Update the blockSizeArray pointer if necessary
						if (useBlockSizeArray) {
							myStream->blockSizeArray++;
							myStream->numBlocksToRead--;
						}
					}
				}
			}
		}

		// If we were able, we serviced the above stream request. Get the next request and decriment the counter
		myStream = myStream->next;
		numToProcess--;
	}

	// See if we ran out of processes or if the counter ran out
	if (myStream) {
		// This implies the counter ran out.  Move the front of the list to myStream->prev to the end of the list
		myStream->prev->next = nullptr;
		_G(lastStream)->next = _G(firstStream);
		_G(firstStream)->prev = _G(lastStream);
		_G(lastStream) = myStream->prev;
		myStream->prev = nullptr;
		_G(firstStream) = myStream;
	}
}

} // namespace M4
