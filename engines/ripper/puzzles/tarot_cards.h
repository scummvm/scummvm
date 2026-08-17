/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this program.
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

#ifndef RIPPER_PUZZLES_TAROT_CARDS_H
#define RIPPER_PUZZLES_TAROT_CARDS_H

#include "audio/mixer.h"

#include "ripper/display.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/puzzles/tarot_cards_model.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class TarotCardsPuzzle : public Puzzle {
public:
	explicit TarotCardsPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;

private:
	bool loadConfiguration();
	bool loadAssets();
	bool loadBitmap(const Common::String &name, BitmapAssetFrame &frame);
	bool loadPcx(const Common::String &name, BitmapAssetFrame &frame);
	void applyPalette() const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void drawScaledFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y, uint scaleNumerator,
		uint scaleDenominator) const;
	void render(const Common::Point &mousePosition) const;
	void renderDeparture(uint slot, uint step) const;
	bool animateRemainingCards();
	int findSlot(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool playSceneCue(uint cue);
	bool playCardCue(uint card);
	void stopAudio();
	Common::String stateString() const;

	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _smallCards[TarotCardsModel::kCardCount];
	BitmapAssetFrame _largeCards[TarotCardsModel::kCardCount];
	Audio::SoundHandle _sceneCueHandles[2];
	Audio::SoundHandle _cardCueHandles[TarotCardsModel::kCardCount];
	Audio::SoundHandle _musicHandle;
	IndexedDisplaySnapshot _incomingDisplay;
	TarotCardsModel _model;
	bool _departed[TarotCardsModel::kSlotCount];
	int _hoveredSlot;
	uint _timeLimitSeconds;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_TAROT_CARDS_H
