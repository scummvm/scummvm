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

#ifndef NANCY_SCRIPT_V5_CLIPTRANSFORMER_H
#define NANCY_SCRIPT_V5_CLIPTRANSFORMER_H

#include "engines/nancy/script/transformer.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

// Nancy26's Transformer. Every record has blend stages and a transform of its
// own by now, so what is left to it is to be the node the records attached to
// it hang under. They take on whether it is enabled, visible and paused, and
// how see-through it is, and get cut off at its "clip" rect.
class ClipTransformerRecord : public TransformerRecord {
public:
	ClipTransformerRecord(SceneContext &context, const char *typeName = "ND.TransformerAR") :
		TransformerRecord(context, typeName) {}

	// Where what is attached gets cut off, in the coordinates of the
	// Transformer; null for nowhere
	const ScriptRect *getClip() const { return _hasClip ? &_clip : nullptr; }

	void onStagesChanged() override;
	void onLocalTransformChanged() override;

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override {}
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	bool _hasClip = false;
	ScriptRect _clip;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_CLIPTRANSFORMER_H
