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

#ifndef MTROPOLIS_SAVELOAD_H
#define MTROPOLIS_SAVELOAD_H

#include "mtropolis/core.h"

namespace Common {

class ReadStream;
class WriteStream;

} // End of namespace Common

namespace MTropolis {

class Modifier;
class Runtime;
class RuntimeObject;

struct ISaveWriter : public IInterfaceBase {
	virtual bool writeSave(Common::WriteStream *stream) = 0;
};

struct ISaveReader : public IInterfaceBase {
	virtual bool readSave(Common::ReadStream *stream, uint32 saveFileVersion) = 0;
};

struct ISaveUIProvider : public IInterfaceBase {
	virtual bool promptSave(ISaveWriter *writer) = 0;
};

struct ILoadUIProvider : public IInterfaceBase {
	virtual bool promptLoad(ISaveReader *reader) = 0;
};

struct IAutoSaveProvider : public IInterfaceBase {
	virtual bool autoSave(ISaveWriter *writer) = 0;
};

class CompoundVarSaver : public ISaveWriter {
public:
	explicit CompoundVarSaver(RuntimeObject *object);

	bool writeSave(Common::WriteStream *stream) override;

private:
	RuntimeObject *_object;
};

class SaveLoadHooks {
public:
	virtual ~SaveLoadHooks();

	virtual void onLoad(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier);
	virtual void onSave(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier);
};

} // End of namespace MTropolis

#endif /* MTROPOLIS_SAVELOAD_H */
