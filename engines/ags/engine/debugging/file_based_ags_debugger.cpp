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

#include "ags/engine/debugging/file_based_ags_debugger.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/text_stream_writer.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *SENT_MESSAGE_FILE_NAME = "dbgrecv.tmp";

bool FileBasedAGSDebugger::Initialize() {
	if (File::IsFile(SENT_MESSAGE_FILE_NAME)) {
		File::DeleteFile(SENT_MESSAGE_FILE_NAME);
	}
	return true;
}

void FileBasedAGSDebugger::Shutdown() {
}

bool FileBasedAGSDebugger::SendMessageToEditor(const char *message) {
	while (File::IsFile(SENT_MESSAGE_FILE_NAME)) {
		_G(platform)->YieldCPU();
	}

	Stream *out = Shared::File::CreateFile(SENT_MESSAGE_FILE_NAME);
	// CHECKME: originally the file was opened as "wb" for some reason,
	// which means the message should be written as a binary array;
	// or shouldn't it?
	out->Write(message, strlen(message));
	delete out;
	return true;
}

bool FileBasedAGSDebugger::IsMessageAvailable() {
	return (File::IsFile("dbgsend.tmp") != 0);
}

char *FileBasedAGSDebugger::GetNextMessage() {
	Stream *in = Shared::File::OpenFileRead("dbgsend.tmp");
	if (in == nullptr) {
		// check again, because the editor might have deleted the file in the meantime
		return nullptr;
	}
	soff_t fileSize = in->GetLength();
	char *msg = (char *)malloc(fileSize + 1);
	in->Read(msg, fileSize);
	delete in;
	File::DeleteFile("dbgsend.tmp");
	msg[fileSize] = 0;
	return msg;
}

} // namespace AGS3
