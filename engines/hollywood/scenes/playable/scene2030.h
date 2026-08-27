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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2030_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2030_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2030 : public PlayableScene {
public:
	Scene2030(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetMerchantLayers();
	void advanceMerchantLayers(uint32 delta);
	void advanceLeftMerchantTick();
	void advanceRightMerchantTick();
	void updateRandomMerchantCallouts(uint32 delta);
	bool startMerchantCalloutSpeech(bool rightMerchant);
	void advanceMerchantCalloutSpeech(uint32 delta);
	void stopMerchantCalloutSpeech();
	void openMerchantForInteraction(bool rightMerchant);
	void closeMerchantAfterInteraction(bool rightMerchant);
	void waitForMerchantState(bool rightMerchant, byte targetState);
	void beginSecondarySpeechLineAndOpenMerchant(uint16 rowIndex, byte frameIndex, bool rightMerchant);
	void waitForStartedSecondarySpeech(uint32 startMillis, uint32 durationMillis);
	void runMerchantPrimarySpeechLine(uint16 rowIndex, byte frameIndex, bool rightMerchant);
	bool merchantSpeechGroupIsRight(byte animationGroup) const;
	bool merchantSpeechGroupIsLeft(byte animationGroup) const;
	void runEntryFromChapterStart();
	void runEntryFromPyramid();
	void runEntryFromSphinx();
	void runEntryPathWithFade(int startX, int startY, byte startFacing, int targetX, int targetY);
	void runSphinxExitTransition();
	void runTransitionClip(uint chunkIndex, bool includeActiveActor, bool fadeIn);
	void drawTransitionClipFrame(uint chunkIndex, byte frameIndex, Graphics::Surface &transitionBackground);
	void drawClipFrameDeltaToSurface(uint chunkIndex, uint tableEntryCount, byte frameIndex, Graphics::Surface &destination);
	void runMerchantShopDialogue();
	void initializeMerchantShopDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setMerchantShopDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const;
	uint16 merchantShopProductPrice(byte productRowId) const;
	void runLeftMerchantPurchase(byte productRowId);
	void subtractEgyptianMoney(uint16 amount);
	void runRightMerchantTalkSequence();
	void runRightMerchantRejectedTradeSequence();
	void runRightStallTradeOverlay();
	void runRightMerchantBuyItemOverlay();
	void runSynchronizedMerchantOverlay(uint chunkIndex, uint descriptorCount,
		const byte *overlayFrameMap, const byte *merchantFrameMap, uint frameCount,
		bool rightMerchant);
	void runRightMerchantSaleSequence(byte soldItemId, uint16 moneyAmount, byte merchantFrameIndex, byte secondaryFrameIndex);
	void addEgyptianMoney(uint16 amount);

	TimedAnimationChannel _leftMerchantChannel;
	TimedAnimationChannel _rightMerchantChannel;
	ResourceSpriteLayer _leftMerchantLayer;
	ResourceSpriteLayer _rightMerchantLayer;
	byte _leftMerchantState;
	byte _rightMerchantState;
	byte _merchantCalloutSide;
	bool _merchantInteractionActive;
	bool _leftMerchantSequenceLocked;
	bool _rightMerchantSequenceLocked;
	bool _merchantCalloutSpeechActive;
	uint32 _merchantCalloutTimerAccumulator;
	uint32 _merchantCalloutSpeechElapsed;
	uint32 _merchantCalloutSpeechDuration;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2030_H
