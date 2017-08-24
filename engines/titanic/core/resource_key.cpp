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

#include "titanic/core/resource_key.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"
#include "common/file.h"

namespace Titanic {

void CResourceKey::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine("Resource Key...", indent);
	file->writeQuotedLine(_key, indent);

	CSaveableObject::save(file, indent);
}

void CResourceKey::load(SimpleFile *file) {
	int val = file->readNumber();

	if (val == 0 || val == 1) {
		file->readBuffer();
		CString str = file->readString();
		setValue(str);
	}

	CSaveableObject::load(file);
}

void CResourceKey::setValue(const CString &name) {
	CString nameStr = name;
	nameStr.toLowercase();
	_key = nameStr;

	_value = nameStr;
	int idx = _value.lastIndexOf('\\');
	if (idx >= 0)
		_value = _value.mid(idx + 1);
}

CString CResourceKey::getFilename() const {
	CString name = _key;

	// Check for a resource being specified within an ST container
	int idx = name.indexOf('#');
	if (idx >= 0) {
		name = name.left(idx);
		name += ".st";
	}

	// The original did tests for the file in the different asset paths,
	// which aren't needed in ScummVM, so just return full name
	return name;
}

bool CResourceKey::scanForFile() const {
	return g_vm->_filesManager->scanForFile(_value);
}

FileType CResourceKey::fileTypeSuffix() const {
	return _value.fileTypeSuffix();
}

ImageType CResourceKey::imageTypeSuffix() const {
	return _value.imageTypeSuffix();
}

} // End of namespace Titanic
