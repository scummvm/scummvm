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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H
#define HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H

#include "common/array.h"
#include "common/types.h"

namespace Hollywood {

// Tracks one resource-backed sprite layer. A layer points at one chunk and one
// descriptor table; frameMap optionally converts a scene's logical animation
// frame into the descriptor index used by the resource.
struct ResourceSpriteLayer {
	ResourceSpriteLayer() :
		visible(false),
		chunkIndex(0),
		descriptorCount(0),
		frameMap(nullptr),
		frameMapSize(0),
		frameIndex(0),
		previousDescriptorIndex(0),
		hasPreviousDescriptor(false) {
	}

	void configure(uint newChunkIndex, uint16 newDescriptorCount, const byte *newFrameMap, uint newFrameMapSize) {
		chunkIndex = newChunkIndex;
		descriptorCount = newDescriptorCount;
		frameMap = newFrameMap;
		frameMapSize = newFrameMapSize;
		reset(0);
	}

	void reset(byte newFrameIndex) {
		frameIndex = newFrameIndex;
		previousDescriptorIndex = descriptorIndex();
		hasPreviousDescriptor = false;
	}

	void setFrame(byte newFrameIndex) {
		const uint16 oldDescriptorIndex = descriptorIndex();
		frameIndex = newFrameIndex;
		const uint16 newDescriptorIndex = descriptorIndex();
		if (oldDescriptorIndex != newDescriptorIndex) {
			previousDescriptorIndex = oldDescriptorIndex;
			hasPreviousDescriptor = true;
		}
	}

	uint16 descriptorIndex() const {
		if (frameMap != nullptr && frameIndex < frameMapSize)
			return frameMap[frameIndex];
		return frameIndex;
	}

	bool visible;
	uint chunkIndex;
	uint16 descriptorCount;
	const byte *frameMap;
	uint frameMapSize;
	byte frameIndex;
	uint16 previousDescriptorIndex;
	bool hasPreviousDescriptor;
};

enum SceneAnimationStratum {
	kSceneAnimationBehindActors,
	kSceneAnimationInFrontOfActors,
	kSceneAnimationActorReplacement,
	kSceneAnimationScenePlaced
};

struct SceneLayerSpec {
	SceneAnimationStratum stratum;
	uint chunkIndex;
	uint16 descriptorCount;
	const byte *frameMap;
	uint frameMapSize;
	bool visible;
	byte initialFrame;
};

/**
 * Stores ordered resource-layer state for a scene composite.
 *
 * Specification order assigns layer IDs and draw order within a stratum. Most
 * layers have a fixed actor-relative stratum; scene-placed layers are drawn by
 * custom composites at the point required by the current scene mode.
 */
class SceneLayerStack {
public:
	enum {
		kInvalidLayer = 0xffffffff
	};

	void clear() {
		_layers.clear();
	}

	template<uint size>
	void configure(const SceneLayerSpec (&specs)[size]) {
		// Reuse storage so same-sized reconfiguration keeps registered layers stable.
		if (_layers.size() != size)
			_layers.resize(size);
		for (uint i = 0; i < size; ++i)
			configureLayer(i, specs[i]);
	}

	void configureLayer(uint id, const SceneLayerSpec &spec) {
		configureLayer(id, spec.stratum, spec.chunkIndex, spec.descriptorCount,
			spec.frameMap, spec.frameMapSize, spec.visible, spec.initialFrame);
	}

	void configureLayer(uint id, SceneAnimationStratum stratum, uint chunkIndex,
			uint16 descriptorCount, const byte *frameMap, uint frameMapSize,
			bool visible = true, byte initialFrame = 0) {
		if (id >= _layers.size())
			_layers.resize(id + 1);

		LayerState &state = _layers[id];
		state.configured = true;
		state.stratum = stratum;
		state.defaultVisible = visible;
		state.initialFrame = initialFrame;
		state.layer.configure(chunkIndex, descriptorCount, frameMap, frameMapSize);
		state.layer.reset(initialFrame);
		state.layer.visible = visible && availableFrameCount(state.layer) != 0;
	}

	uint addLayer(SceneAnimationStratum stratum, uint chunkIndex, uint16 descriptorCount,
			const byte *frameMap, uint frameMapSize, bool visible = true,
			byte initialFrame = 0) {
		const uint id = _layers.size();
		configureLayer(id, stratum, chunkIndex, descriptorCount, frameMap,
			frameMapSize, visible, initialFrame);
		return id;
	}

	void configureLayerResource(uint id, uint chunkIndex, uint16 descriptorCount,
			const byte *frameMap, uint frameMapSize, bool visible = true) {
		if (!hasLayer(id))
			return;

		LayerState &state = _layers[id];
		state.layer.configure(chunkIndex, descriptorCount, frameMap, frameMapSize);
		state.defaultVisible = visible;
		state.initialFrame = 0;
		state.layer.visible = visible && availableFrameCount(state.layer) != 0;
	}

	void reset() {
		for (uint i = 0; i < _layers.size(); ++i) {
			LayerState &state = _layers[i];
			if (!state.configured)
				continue;
			state.layer.reset(state.initialFrame);
			state.layer.visible = state.defaultVisible && availableFrameCount(state.layer) != 0;
		}
	}

	uint layerCount() const {
		return _layers.size();
	}

	bool hasLayer(uint id) const {
		return id < _layers.size() && _layers[id].configured;
	}

	bool isInStratum(uint id, SceneAnimationStratum stratum) const {
		return hasLayer(id) && _layers[id].stratum == stratum;
	}

	bool hasVisibleLayers() const {
		for (uint i = 0; i < _layers.size(); ++i) {
			if (_layers[i].configured && _layers[i].layer.visible)
				return true;
		}
		return false;
	}

	bool hasVisibleLayers(SceneAnimationStratum stratum) const {
		for (uint i = 0; i < _layers.size(); ++i) {
			if (_layers[i].configured && _layers[i].stratum == stratum &&
					_layers[i].layer.visible) {
				return true;
			}
		}
		return false;
	}

	ResourceSpriteLayer &layer(uint id) {
		return _layers[id].layer;
	}

	const ResourceSpriteLayer &layer(uint id) const {
		return _layers[id].layer;
	}

	void setLayerVisible(uint id, bool visible) {
		if (hasLayer(id))
			_layers[id].layer.visible = visible && availableFrameCount(_layers[id].layer) != 0;
	}

	void setLayerFrame(uint id, byte frameIndex) {
		if (hasLayer(id))
			_layers[id].layer.setFrame(frameIndex);
	}

	void setVisibleLayerFrame(uint id, byte frameIndex) {
		if (hasLayer(id) && _layers[id].layer.visible)
			_layers[id].layer.setFrame(frameIndex);
	}

	void setVisibleLayerFrameClamped(uint id, uint frameIndex) {
		if (!hasLayer(id) || !_layers[id].layer.visible)
			return;

		const uint count = availableFrameCount(_layers[id].layer);
		if (count == 0)
			return;

		if (frameIndex >= count)
			frameIndex = count - 1;
		if (frameIndex > 0xff)
			frameIndex = 0xff;
		_layers[id].layer.setFrame((byte)frameIndex);
	}

	byte layerFrame(uint id) const {
		return hasLayer(id) ? _layers[id].layer.frameIndex : 0;
	}

	bool layerVisible(uint id) const {
		return hasLayer(id) && _layers[id].layer.visible;
	}

	uint maximumVisibleFrameCount() const {
		uint result = 0;
		for (uint i = 0; i < _layers.size(); ++i) {
			if (!_layers[i].configured || !_layers[i].layer.visible)
				continue;
			const uint count = availableFrameCount(_layers[i].layer);
			if (count > result)
				result = count;
		}
		return result;
	}

private:
	struct LayerState {
		LayerState() :
				configured(false),
				stratum(kSceneAnimationBehindActors),
				defaultVisible(false),
				initialFrame(0),
				layer() {
		}

		bool configured;
		SceneAnimationStratum stratum;
		bool defaultVisible;
		byte initialFrame;
		ResourceSpriteLayer layer;
	};

	static uint availableFrameCount(const ResourceSpriteLayer &layer) {
		return layer.frameMap != nullptr ? layer.frameMapSize : layer.descriptorCount;
	}

	Common::Array<LayerState> _layers;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H
