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

#ifndef RIPPER_WAC_VOICE_LOCK_RENDERER_H
#define RIPPER_WAC_VOICE_LOCK_RENDERER_H

#include "common/array.h"
#include "common/rect.h"

namespace Ripper {

class WacDatabaseSession;
struct BitmapAssetFrame;
struct WacVoiceLockEditorState;
struct WacVoiceLockPcm;

class WacVoiceLockRenderer {
public:
	WacVoiceLockRenderer(WacDatabaseSession *database,
		const Common::Array<BitmapAssetFrame> &buttonAssets,
		const Common::Rect *buttonBounds,
		const Common::Rect &sourcePanel, const Common::Rect &editorPanel,
		const Common::Rect &sourceWaveform, const Common::Rect &editorWaveform,
		bool editorAvailable);

	bool drawPresentation(const WacVoiceLockPcm &sourcePcm,
		WacVoiceLockEditorState &state) const;
	bool restoreSourceSelectionDragBacking(
		WacVoiceLockEditorState &state) const;

private:
	WacDatabaseSession *_database;
	const Common::Array<BitmapAssetFrame> &_buttonAssets;
	Common::Rect _buttonBounds[3];
	Common::Rect _sourcePanel;
	Common::Rect _editorPanel;
	Common::Rect _sourceWaveform;
	Common::Rect _editorWaveform;
	bool _editorAvailable;
};

} // End of namespace Ripper

#endif // RIPPER_WAC_VOICE_LOCK_RENDERER_H
