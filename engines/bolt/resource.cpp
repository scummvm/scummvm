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

#include "bolt/bolt.h"

namespace Bolt {

bool BoltEngine::openBOLTLib(const char *fileName, int *outIdx, BOLTLib **outLib) {
	return false;
}

void BoltEngine::closeBOLTLib(BOLTLib **lib) {
}

bool BoltEngine::attemptFreeIndex(BOLTLib *lib, int16 groupId) {
	return false;
}

void BoltEngine::loadGroupDirectory() {
}

bool BoltEngine::getBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags) {
	return false;
}

void BoltEngine::freeBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags) {
}

void *BoltEngine::getBOLTMember(BOLTLib *lib, int16 resId) {
	return nullptr;
}

bool BoltEngine::freeBOLTMember(BOLTLib *lib, int16 resId) {
	return false;
}

void *BoltEngine::memberAddr(BOLTLib *lib, int16 resId) {
	return nullptr;
}

void *BoltEngine::memberAddrOffset(BOLTLib *lib, uint32 resIdAndOffset) {
	return nullptr;
}

uint32 BoltEngine::memberSize(BOLTLib *lib, int16 resId) {
	return uint32();
}

void *BoltEngine::groupAddr(BOLTLib *lib, int16 groupId) {
	return nullptr;
}

bool BoltEngine::allocResourceIndex() {
	return false;
}

void BoltEngine::freeResourceIndex() {
}

bool BoltEngine::initVRam(int16 poolSize) {
	return false;
}

void BoltEngine::freeVRam() {
}

bool BoltEngine::vLoad(void *dest, const char *name) {
	return false;
}

bool BoltEngine::vSave(void *src, uint16 srcSize, const char *name) {
	return false;
}

bool BoltEngine::vDelete(const char *name) {
	return false;
}

void BoltEngine::memMove(void *dest, void *src, uint16 count) {
}

void *BoltEngine::dataAddress(int16 recordOffset) {
	return nullptr;
}

uint16 BoltEngine::dataSize(int16 recordOffset) {
	return uint16();
}

bool BoltEngine::findRecord(const char *name, int16 *outOffset) {
	return false;
}

} // End of namespace Bolt
