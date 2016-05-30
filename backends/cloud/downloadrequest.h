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

#ifndef BACKENDS_CLOUD_DOWNLOADREQUEST_H
#define BACKENDS_CLOUD_DOWNLOADREQUEST_H

#include "backends/networking/curl/request.h"
#include "backends/networking/curl/networkreadstream.h"
#include "backends/cloud/storage.h"
#include "common/file.h"

namespace Cloud {

class DownloadRequest: public Networking::Request {	
	Storage::BoolCallback _boolCallback;	
	Common::DumpFile *_localFile;
	Common::String _remoteFileName;
	Storage *_storage;
	Networking::NetworkReadStream *_remoteFileStream;
	Request *_workingRequest;
	bool _ignoreCallback;

	void start();
	void streamCallback(Networking::NetworkReadStreamResponse response);
	void streamErrorCallback(Networking::ErrorResponse error);
	void finishSuccess(bool success);
public:
	DownloadRequest(Storage *storage, Storage::BoolCallback callback, Networking::ErrorCallback ecb, Common::String remoteFile, Common::DumpFile *dumpFile);
	virtual ~DownloadRequest();

	virtual void handle();
	virtual void restart();
};

} // End of namespace Cloud

#endif
