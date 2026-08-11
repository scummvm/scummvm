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

#ifndef MTROPOLIS_PLUGIN_MTI_H
#define MTROPOLIS_PLUGIN_MTI_H

#include "mtropolis/modifier_factory.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/plugin/mti_data.h"
#include "mtropolis/runtime.h"

namespace Common {

class RandomSource;

} // End of namespace Common

namespace MTropolis {

namespace MTI {

class MTIPlugIn;

class ShanghaiModifier : public Modifier {
public:
	ShanghaiModifier();
	~ShanghaiModifier();

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool load(const PlugInModifierLoaderContext &context, const Data::MTI::ShanghaiModifier &data);

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	const char *debugGetTypeName() const override { return "Shanghai Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	static const uint kNumTiles = 28;
	static const uint kNumFaces = 26;

	typedef uint32 BoardState_t;

	static const uint kBoardSizeX = 13;
	static const uint kBoardSizeY = 7;
	static const uint kBoardSizeZ = 3;

	struct TileCoordinate {
		uint x;
		uint y;
		uint z;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void resetTiles(Common::RandomSource &rng, uint (&tileFaces)[kNumTiles]) const;
	static uint selectAndRemoveOne(Common::RandomSource &rng, uint *valuesList, uint &listSize);
	bool boardStateHasValidMove(BoardState_t boardState) const;
	bool tileIsExposed(BoardState_t boardState, uint tile) const;
	bool tileExistsAtCoordinate(BoardState_t boardState, uint x, uint y, uint z) const;

	static BoardState_t boardStateBit(uint bit);
	static BoardState_t emptyBoardState();

	Event _resetTileSetWhen;
	VarReference _tileSetRef;

	static TileCoordinate _tileCoordinates[kNumTiles];
	int8 _tileAtCoordinate[kBoardSizeX][kBoardSizeY][kBoardSizeZ];
};

class MPEGVideoPlayer;

class IMPEGVideoCompletionNotifier : IInterfaceBase {
public:
	virtual void onVideoCompleted() = 0;
};

class SampleModifier : public Modifier, public IMPEGVideoCompletionNotifier, public IKeyboardEventReceiver {
public:
	SampleModifier();
	~SampleModifier();

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool load(const PlugInModifierLoaderContext &context, const Data::MTI::SampleModifier &data);

	void onVideoCompleted() override;
	void onKeyboardEvent(Runtime *runtime, const KeyboardInputEvent &keyEvt) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sample Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void stopPlaying();

	Event _executeWhen;
	int32 _videoNumber;

	Common::SharedPtr<MPEGVideoPlayer> _vidPlayer;
	Common::SharedPtr<KeyboardEventSignaller> _keySignaller;
	Runtime *_runtime;
	bool _isPlaying;
};


class MTIPlugIn : public MTropolis::PlugIn {
public:
	MTIPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<ShanghaiModifier, Data::MTI::ShanghaiModifier> _shanghaiModifierFactory;
	PlugInModifierFactory<SampleModifier, Data::MTI::SampleModifier> _sampleModifierFactory;
};

} // End of namespace MTI

} // End of namespace MTropolis

#endif
