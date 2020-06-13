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

#include "backends/networking/sdl_net/uploadfileclienthandler.h"
#include "backends/fs/fs-factory.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/networking/sdl_net/reader.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/translation.h"

namespace Networking {

UploadFileClientHandler::UploadFileClientHandler(Common::String parentDirectoryPath):
	_state(UFH_READING_CONTENT), _headersStream(nullptr), _contentStream(nullptr),
	_parentDirectoryPath(parentDirectoryPath), _uploadedFiles(0) {}

UploadFileClientHandler::~UploadFileClientHandler() {
	delete _headersStream;
	delete _contentStream;
}

void UploadFileClientHandler::handle(Client *client) {
	if (client == nullptr) {
		warning("UploadFileClientHandler::handle(): empty client pointer");
		return;
	}

	while (true) {
		switch (_state) {
		case UFH_READING_CONTENT:
			if (client->readContent(nullptr)) {
				_state = UFH_READING_BLOCK_HEADERS;
				continue;
			}
			break;

		case UFH_READING_BLOCK_HEADERS:
			if (_headersStream == nullptr)
				_headersStream = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);

			if (client->readBlockHeaders(_headersStream)) {
				handleBlockHeaders(client);
				continue;
			}

			// fail on suspicious headers
			if (_headersStream->size() > Reader::SUSPICIOUS_HEADERS_SIZE) {
				setErrorMessageHandler(*client, Common::convertFromU32String(_("Invalid request: headers are too long!")));
			}
			break;

		case UFH_READING_BLOCK_CONTENT:
			// _contentStream is created by handleBlockHeaders() if needed

			if (client->readBlockContent(_contentStream)) {
				handleBlockContent(client);
				continue;
			}
			break;

		case UFH_ERROR:
		case UFH_STOP:
			return;
		}

		break;
	}
}

namespace {
void readFromThatUntilDoubleQuote(const char *cstr, Common::String needle, Common::String &result) {
	const char *position = strstr(cstr, needle.c_str());

	if (position) {
		char c;
		for (const char *i = position + needle.size(); c = *i, c != 0; ++i) {
			if (c == '"')
				break;
			result += c;
		}
	}
}
}

void UploadFileClientHandler::handleBlockHeaders(Client *client) {
	_state = UFH_READING_BLOCK_CONTENT;

	// fail on suspicious headers
	if (_headersStream->size() > Reader::SUSPICIOUS_HEADERS_SIZE) {
		setErrorMessageHandler(*client, Common::convertFromU32String(_("Invalid request: headers are too long!")));
	}

	// search for "upload_file" field
	Common::String headers = Reader::readEverythingFromMemoryStream(_headersStream);
	Common::String fieldName = "";
	readFromThatUntilDoubleQuote(headers.c_str(), "name=\"", fieldName);
	if (!fieldName.hasPrefix("upload_file"))
		return;

	Common::String filename = "";
	readFromThatUntilDoubleQuote(headers.c_str(), "filename=\"", filename);

	// skip block if <filename> is empty
	if (filename.empty())
		return;

	if (HandlerUtils::hasForbiddenCombinations(filename))
		return;

	// check that <path>/<filename> doesn't exist
	Common::String path = _parentDirectoryPath;
	if (path.lastChar() != '/' && path.lastChar() != '\\')
		path += '/';
	AbstractFSNode *originalNode = g_system->getFilesystemFactory()->makeFileNodePath(path + filename);
	if (!HandlerUtils::permittedPath(originalNode->getPath())) {
		setErrorMessageHandler(*client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}
	if (originalNode->exists()) {
		setErrorMessageHandler(*client, Common::convertFromU32String(_("There is a file with that name in the parent directory!")));
		return;
	}

	// remove previous stream (if there is one)
	if (_contentStream) {
		delete _contentStream;
		_contentStream = nullptr;
	}

	// create file stream (and necessary subdirectories)
	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(originalNode->getPath(), true)) {
		delete f;
		setErrorMessageHandler(*client, Common::convertFromU32String(_("Failed to upload the file!")));
		return;
	}

	_contentStream = f;
}

void UploadFileClientHandler::handleBlockContent(Client *client) {
	_state = UFH_READING_BLOCK_HEADERS;

	// if previous block headers were file-related and created a stream
	if (_contentStream) {
		_contentStream->flush();
		++_uploadedFiles;

		delete _contentStream;
		_contentStream = nullptr;

		if (client->noMoreContent()) {
			// success - redirect back to directory listing
			setSuccessHandler(*client);
			return;
		}
	}

	// no more content avaiable
	if (client->noMoreContent()) {
		// if no file field was found - failure
		if (_uploadedFiles == 0) {
			setErrorMessageHandler(*client, Common::convertFromU32String(_("No file was passed!")));
		} else {
			setSuccessHandler(*client);
		}
	}
}

void UploadFileClientHandler::setErrorMessageHandler(Client &client, Common::String message) {
	HandlerUtils::setFilesManagerErrorMessageHandler(client, message);
	_state = UFH_ERROR;
}

void UploadFileClientHandler::setSuccessHandler(Client &client) {
	// success - redirect back to directory listing
	HandlerUtils::setMessageHandler(
		client,
		Common::String::format(
			"%s<br/><a href=\"files?path=%s\">%s</a>",
			Common::convertFromU32String(_("Uploaded successfully!")).c_str(),
			client.queryParameter("path").c_str(),
			Common::convertFromU32String(_("Back to parent directory")).c_str()
			),
		(client.queryParameter("ajax") == "true" ? "/filesAJAX?path=" : "/files?path=") +
		LocalWebserver::urlEncodeQueryParameterValue(client.queryParameter("path"))
	);
	_state = UFH_STOP;
}

} // End of namespace Networking
