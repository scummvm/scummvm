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

#ifndef RIPPER_OPTIONS_PANEL_H
#define RIPPER_OPTIONS_PANEL_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/controls.h"
#include "ripper/display.h"
#include "ripper/resources.h"
#include "ripper/settings.h"

namespace Video {
class SmackerDecoder;
}

namespace Ripper {

class ResourceManager;
class RipperEngine;

class OptionsPanelManager {
public:
	explicit OptionsPanelManager(RipperEngine *engine);

	bool initialize(ResourceManager &resources, bool demoVariant = false);
	bool run();

private:
	struct State {
		uint16 actionKeys[RipperSettings::kActionKeyCount];
		uint videoMode;
		uint combatLevel;
		uint puzzleLevel;
		bool bufferedVideo;
	};

	struct DemoState {
		bool toggles[RipperSettings::kDemoToggleCount];
		int sliders[RipperSettings::kDemoSliderCount];
		uint16 actionKeys[RipperSettings::kActionKeyCount - 1];
	};

	bool initializeRetail(ResourceManager &resources);
	bool initializeDemo(ResourceManager &resources);
	bool loadFrameSet(ResourceManager &resources, const char *prefix, uint count,
		Common::Array<BitmapAssetFrame> &frames);
	bool captureDisplay();
	void restoreDisplay();
	void applyPalette();
	void drawBackground();
	void drawState(const State &state);
	void drawBitmap(const BitmapAssetFrame &frame, int x, int y) const;
	void drawKeyLabel(uint16 command);
	void clearKeyLabel();
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const;
	uint measureText(const Common::String &text) const;
	Common::String formatKeyLabel(uint16 command) const;
	int findControl(const Common::Point &point) const;
	bool serviceBackground(Video::SmackerDecoder &decoder);
	void playControlSound(uint controlIndex);
	void commitState(const State &state);
	bool runRetail();
	bool runDemo();
	void drawDemoState(const DemoState &state, int captureSlot = -1);
	void commitDemoState(const DemoState &state);
	void resetDemoState(DemoState &state) const;
	int demoSliderValueFromPoint(uint slider, int x) const;

	RipperEngine *_engine;
	BitmapAssetFrame _background;
	BitmapFontAsset _font;
	Common::Array<BitmapAssetFrame> _toggleFrames;
	Common::Array<BitmapAssetFrame> _videoModeFrames;
	Common::Array<BitmapAssetFrame> _combatFrames;
	Common::Array<BitmapAssetFrame> _puzzleFrames;
	Common::Array<BitmapAssetFrame> _accentFrames;
	Common::Array<BitmapAssetFrame> _demoFrames;
	Common::Array<Common::String> _demoButtonLabels;
	Common::Array<Common::String> _demoCaptureLabels;
	Common::String _demoCapturePrompt;
	UiControlRegistry _controls;
	IndexedDisplaySnapshot _savedDisplay;
	Audio::SoundHandle _soundHandle;
	bool _demoVariant;
	bool _initialized;
};

} // End of namespace Ripper

#endif // RIPPER_OPTIONS_PANEL_H
