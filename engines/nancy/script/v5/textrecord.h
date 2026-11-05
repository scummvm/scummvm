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

#ifndef NANCY_SCRIPT_V5_TEXTRECORD_H
#define NANCY_SCRIPT_V5_TEXTRECORD_H

#include "engines/nancy/script/records.h"

#include "engines/nancy/script/v5/bitmaptext.h"

namespace Nancy {
namespace Script {

// Nancy26's Text record, which writes with the fonts made of pictures. The
// text can be drawn in a color of its own, or with a part of its letters, and
// scripts read where the letters ended up from "bounds".
class BitmapTextRecord : public Record {
public:
	BitmapTextRecord(SceneContext &context) : Record(context, "ND.TextAR") {}

	RecordType getType() const override { return kRecordText; }

	void updateGraphics() override;
	void registerGraphics() override;

protected:
	void readTable(int tableIndex) override;
	void onZChanged() override;
	bool hasTransform() const override { return true; }
	void onTransformChanged() override { applyDest(); }
	bool prepareReset() override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	void setText(const Common::String &text);
	void applyDest();
	void draw();

	BitmapText _layout;
	RecordGraphic _graphic;
	bool _isDirty = true;

	ScriptRect _dest;
	ScriptRect _initialDest;	// what the record goes back to when it starts over
	Common::Point _inkOffset;	// where the picture of the letters is from the area

	bool _usesAutotext = true;
	Common::String _textKey;	// what the text was set to
	Common::String _text;		// what it says

	uint32 _modColor = 0xFFFFFFFF;
	int _overrideFontColor = -1;
	int _renderStart = 0;
	int _renderLength = -1;		// -1 for all of it
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_TEXTRECORD_H
