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

#include "ags/shared/util/proxy_stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

ProxyStream::ProxyStream(Stream *stream, ObjectOwnershipPolicy stream_ownership_policy)
	: _stream(stream)
	, _streamOwnershipPolicy(stream_ownership_policy) {
}

ProxyStream::~ProxyStream() {
	ProxyStream::Close();
}

void ProxyStream::Close() {
	if (_streamOwnershipPolicy == kDisposeAfterUse) {
		delete _stream;
	}
	_stream = nullptr;
}

bool ProxyStream::Flush() {
	return _stream->Flush();
}

bool ProxyStream::IsValid() const {
	return _stream && _stream->IsValid();
}

bool ProxyStream::EOS() const {
	return _stream->EOS();
}

soff_t ProxyStream::GetLength() const {
	return _stream->GetLength();
}

soff_t ProxyStream::GetPosition() const {
	return _stream->GetPosition();
}

bool ProxyStream::CanRead() const {
	return _stream->CanRead();
}

bool ProxyStream::CanWrite() const {
	return _stream->CanWrite();
}

bool ProxyStream::CanSeek() const {
	return _stream->CanSeek();
}

size_t ProxyStream::Read(void *buffer, size_t size) {
	return _stream->Read(buffer, size);
}

int32_t ProxyStream::ReadByte() {
	return _stream->ReadByte();
}

int16_t ProxyStream::ReadInt16() {
	return _stream->ReadInt16();
}

int32_t ProxyStream::ReadInt32() {
	return _stream->ReadInt32();
}

int64_t ProxyStream::ReadInt64() {
	return _stream->ReadInt64();
}

size_t ProxyStream::ReadArray(void *buffer, size_t elem_size, size_t count) {
	return _stream->ReadArray(buffer, elem_size, count);
}

size_t ProxyStream::ReadArrayOfInt16(int16_t *buffer, size_t count) {
	return _stream->ReadArrayOfInt16(buffer, count);
}

size_t ProxyStream::ReadArrayOfInt32(int32_t *buffer, size_t count) {
	return _stream->ReadArrayOfInt32(buffer, count);
}

size_t ProxyStream::ReadArrayOfInt64(int64_t *buffer, size_t count) {
	return _stream->ReadArrayOfInt64(buffer, count);
}

size_t ProxyStream::Write(const void *buffer, size_t size) {
	return _stream->Write(buffer, size);
}

int32_t ProxyStream::WriteByte(uint8_t b) {
	return _stream->WriteByte(b);
}

size_t ProxyStream::WriteInt16(int16_t val) {
	return _stream->WriteInt16(val);
}

size_t ProxyStream::WriteInt32(int32_t val) {
	return _stream->WriteInt32(val);
}

size_t ProxyStream::WriteInt64(int64_t val) {
	return _stream->WriteInt64(val);
}

size_t ProxyStream::WriteArray(const void *buffer, size_t elem_size, size_t count) {
	return _stream->WriteArray(buffer, elem_size, count);
}

size_t ProxyStream::WriteArrayOfInt16(const int16_t *buffer, size_t count) {
	return _stream->WriteArrayOfInt16(buffer, count);
}

size_t ProxyStream::WriteArrayOfInt32(const int32_t *buffer, size_t count) {
	return _stream->WriteArrayOfInt32(buffer, count);
}

size_t ProxyStream::WriteArrayOfInt64(const int64_t *buffer, size_t count) {
	return _stream->WriteArrayOfInt64(buffer, count);
}

bool ProxyStream::Seek(soff_t offset, StreamSeek origin) {
	return _stream->Seek(offset, origin);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
