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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_SCRIPT_H
#define RIPPER_SCRIPT_H

#include "common/array.h"
#include "common/str.h"

namespace Ripper {

class AssetLibrary;
class ResourceManager;
class RipperEngine;

struct ScriptArgument {
	byte type;
	uint32 value;
	Common::Array<byte> data;
};

struct ScriptCommand {
	uint32 offset;
	byte opcode;
	uint16 selector;
	Common::Array<ScriptArgument> arguments;
};

struct ScriptFrame {
	byte presentationType;
	uint32 mediaNameOffset;
	uint32 labelOffset;
	uint32 textOffset;
	byte interactionCount;
	uint32 enterCallbackOffset;
	uint32 idleCallbackOffset;
	uint32 exitCallbackOffset;
	uint16 firstInteractionIndex;
	int16 x;
	int16 y;
	uint16 initialSelection;
};

struct ScriptInteraction {
	Common::String label;
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	byte initialSelection;
	uint32 conditionOffset;
	uint32 callbackOffset;
	byte flags;
};

class CompiledScript {
public:
	CompiledScript();

	bool load(AssetLibrary &library, const Common::String &memberName);
	bool decodeCallback(uint32 offset, bool decodeText, Common::Array<ScriptCommand> &commands) const;

	const Common::String &getMemberName() const { return _memberName; }
	const Common::String &getSourceName() const { return _sourceName; }
	uint16 getVersion() const { return _version; }
	const Common::Array<ScriptFrame> &getFrames() const { return _frames; }
	const Common::Array<ScriptInteraction> &getInteractions() const { return _interactions; }
	Common::String getString(uint32 offset) const;

private:
	bool parseHeader();
	bool validateCallbacks() const;
	bool canRead(uint32 offset, uint32 size) const;
	uint16 readUint16(uint32 offset) const;
	int16 readSint16(uint32 offset) const;
	uint32 readUint32(uint32 offset) const;

	Common::String _memberName;
	Common::String _sourceName;
	Common::Array<byte> _data;
	Common::Array<ScriptFrame> _frames;
	Common::Array<ScriptInteraction> _interactions;
	uint16 _version;
	uint32 _argumentLayoutOffset;
	byte _argumentLayoutStride;
	uint16 _argumentLayoutCount;
};

class ScriptManager {
public:
	explicit ScriptManager(RipperEngine *engine);

	bool initialize(ResourceManager &resources);
	bool runStartupPath();
	bool serviceScene();

	CompiledScript &startup() { return _startup; }
	CompiledScript &ba0() { return _ba0; }

private:
	bool executeCallback(CompiledScript &script, uint32 callbackOffset, int &result,
		uint *nextFrame = nullptr);
	void beginBa0InteractionWait(const Common::String &frameLabel, uint interactionCount);
	bool advanceBa0ToFrame(uint nextFrame);
	bool executeConcurrentFrame();
	bool findFrameByLabel(const CompiledScript &script, const Common::String &label, uint &frameIndex) const;
	bool isMilestoneFlagSet(uint32 flag) const;
	bool isScenePlayed(const Common::String &scene) const;
	void markScenePlayed(const Common::String &scene);
	static Common::String argumentString(const ScriptArgument &argument);

	RipperEngine *_engine;
	CompiledScript _startup;
	CompiledScript _ba0;
	CompiledScript _concurrent;
	Common::String _concurrentEntryLabel;
	Common::Array<bool> _milestoneFlags;
	Common::Array<Common::String> _playedScenes;
	uint _activeBa0Frame;
	int _hoveredBa0Interaction;
	bool _awaitingBa0Interaction;
	bool _briefingArmed;
	uint _briefingSelector;
};

} // End of namespace Ripper

#endif // RIPPER_SCRIPT_H
