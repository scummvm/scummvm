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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/logic.h"

#include "common/memstream.h"

namespace Nancy {

bool Logic::addNewActionRecord(Common::SeekableReadStream &inputData) {
    inputData.seek(0x30);
    byte ARType = inputData.readByte();
    ActionRecord *newRecord = createActionRecord(ARType);

    inputData.seek(0);
    char *descBuf = new char[0x30];
    inputData.read(descBuf, 0x30);
    newRecord->description = Common::String(descBuf);
    delete[] descBuf;

    newRecord->type = inputData.readByte(); // redundant
    newRecord->execType = inputData.readByte();

    uint16 localChunkSize = newRecord->readData(inputData);
    localChunkSize += 0x32;

    // If the localChunkSize is less than the total data, there must be dependencies at the end of the chunk
    uint16 depsDataSize = (uint16)inputData.size() - localChunkSize;
    if (depsDataSize > 0) {
        // Each dependency is 0x0C bytes long (in v1)
        newRecord->numDependencies = depsDataSize / 0xC;
        if (depsDataSize % 0xC) {
            error("Invalid dependency data size!");
        }

        newRecord->dependencies = new DependencyRecord[newRecord->numDependencies];

        // Initialize the dependencies data
        inputData.seek(/*0x32 + */localChunkSize);
        for (uint16 i = 0; i < newRecord->numDependencies; ++i) {
            newRecord->dependencies[i].type = (DependencyType)inputData.readByte();
            newRecord->dependencies[i].label = inputData.readByte();
            newRecord->dependencies[i].condition = inputData.readByte();
            newRecord->dependencies[i].orFlag = inputData.readByte();
            newRecord->dependencies[i].hours = inputData.readSint16LE();
            newRecord->dependencies[i].minutes = inputData.readSint16LE();
            newRecord->dependencies[i].seconds = inputData.readSint16LE();
            newRecord->dependencies[i].milliseconds = inputData.readSint16LE();
        }

        for (uint16 i = 0; i < newRecord->numDependencies; ++i) {
            DependencyRecord &current = newRecord->dependencies[i];
            if (current.type != 9 || current.hours != -1 || current.minutes != -1 || current.seconds != -1) {
                newRecord->timers[i] = ((current.hours * 60 + current.minutes) * 60 + current.seconds) * 1000 + current.milliseconds;
            }
        }

        for (uint16 i = 0; i < newRecord->numDependencies; ++i) {
            if (newRecord->dependencies[i].orFlag == 1) {
                // newRecord->0x78+i = true
            } else {
                // 0x78+i = false
            }
        }
    }

    _records.push_back(newRecord);
    return true;
}

} // End of namespace Nancy