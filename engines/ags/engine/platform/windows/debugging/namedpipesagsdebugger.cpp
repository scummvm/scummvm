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

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS

#include "platform/windows/debug/namedpipesagsdebugger.h"

#include <stdio.h> // sprintf

void NamedPipesAGSDebugger::SendAcknowledgement() {
	DWORD bytesWritten;
	WriteFile(_hPipeSending, "MSGACK", 6, &bytesWritten, NULL);
}


NamedPipesAGSDebugger::NamedPipesAGSDebugger(const char *instanceToken) {
	_hPipeSending = NULL;
	_hPipeReading = NULL;
	_instanceToken = instanceToken;
}

bool NamedPipesAGSDebugger::Initialize() {
	// can't use a single duplex pipe as it was deadlocking
	char pipeNameBuffer[MAX_PATH];
	sprintf(pipeNameBuffer, "\\\\.\\pipe\\AGSEditorDebuggerGameToEd%s", _instanceToken);
	_hPipeSending = CreateFile(pipeNameBuffer, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	sprintf(pipeNameBuffer, "\\\\.\\pipe\\AGSEditorDebuggerEdToGame%s", _instanceToken);
	_hPipeReading = CreateFile(pipeNameBuffer, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if ((_hPipeReading == INVALID_HANDLE_VALUE) ||
	        (_hPipeSending == INVALID_HANDLE_VALUE))
		return false;

	return true;
}

void NamedPipesAGSDebugger::Shutdown() {
	if (_hPipeReading != NULL) {
		CloseHandle(_hPipeReading);
		CloseHandle(_hPipeSending);
		_hPipeReading = NULL;
	}
}

bool NamedPipesAGSDebugger::SendMessageToEditor(const char *message) {
	DWORD bytesWritten;
	return (WriteFile(_hPipeSending, message, strlen(message), &bytesWritten, NULL) != 0);
}

bool NamedPipesAGSDebugger::IsMessageAvailable() {
	DWORD bytesAvailable = 0;
	PeekNamedPipe(_hPipeReading, NULL, 0, NULL, &bytesAvailable, NULL);

	return (bytesAvailable > 0);
}

char *NamedPipesAGSDebugger::GetNextMessage() {
	DWORD bytesAvailable = 0;
	PeekNamedPipe(_hPipeReading, NULL, 0, NULL, &bytesAvailable, NULL);

	if (bytesAvailable > 0) {
		char *buffer = (char *)malloc(bytesAvailable + 1);
		DWORD bytesRead = 0;
		ReadFile(_hPipeReading, buffer, bytesAvailable, &bytesRead, NULL);
		buffer[bytesRead] = 0;

		SendAcknowledgement();
		return buffer;
	} else {
		return NULL;
	}

}

#endif // AGS_PLATFORM_OS_WINDOWS
