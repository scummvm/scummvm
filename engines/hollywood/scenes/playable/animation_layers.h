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
	kSceneAnimationActorReplacement
};

struct SceneAnimationLayerSpec {
	SceneAnimationStratum stratum;
	uint chunkIndex;
	uint16 descriptorCount;
	const byte *frameMap;
	uint frameMapSize;
	bool visible;
};

/**
 * Stores resource layers with an explicit position in the scene composite.
 *
 * Specification order assigns layer IDs and draw order within a stratum.
 * Visibility and frame changes remain independent.
 */
class SceneAnimationLayers {
public:
	void clear() {
		_layers.clear();
	}

	template<uint size>
	void configure(const SceneAnimationLayerSpec (&specs)[size]) {
		clear();
		for (uint i = 0; i < size; ++i)
			configureLayer(i, specs[i]);
	}

	void configureLayer(uint id, const SceneAnimationLayerSpec &spec) {
		if (id >= _layers.size())
			_layers.resize(id + 1);

		LayerState &state = _layers[id];
		state.configured = true;
		state.stratum = spec.stratum;
		state.layer.configure(spec.chunkIndex, spec.descriptorCount,
			spec.frameMap, spec.frameMapSize);
		state.layer.visible = spec.visible && availableFrameCount(state.layer) != 0;
	}

	void configureLayerResource(uint id, uint chunkIndex, uint16 descriptorCount,
			const byte *frameMap, uint frameMapSize, bool visible = true) {
		if (!hasLayer(id))
			return;

		LayerState &state = _layers[id];
		state.layer.configure(chunkIndex, descriptorCount, frameMap, frameMapSize);
		state.layer.visible = visible && availableFrameCount(state.layer) != 0;
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

	byte layerFrame(uint id) const {
		return hasLayer(id) ? _layers[id].layer.frameIndex : 0;
	}

private:
	struct LayerState {
		LayerState() :
				configured(false),
				stratum(kSceneAnimationBehindActors),
				layer() {
		}

		bool configured;
		SceneAnimationStratum stratum;
		ResourceSpriteLayer layer;
	};

	static uint availableFrameCount(const ResourceSpriteLayer &layer) {
		return layer.frameMap != nullptr ? layer.frameMapSize : layer.descriptorCount;
	}

	Common::Array<LayerState> _layers;
};

/**
 * Stores an ordered set of transient resource-sprite layers.
 *
 * It owns layer state and order only; the scene chooses the draw stratum by
 * calling drawTransientLayers() at the appropriate point in its composite.
 */
class TransientLayerCompositor {
public:
	enum {
		kInvalidLayer = 0xffffffff
	};

	TransientLayerCompositor() :
			_layers() {
	}

	void clear() {
		_layers.clear();
	}

	bool visible() const {
		for (uint i = 0; i < _layers.size(); ++i) {
			if (_layers[i].visible)
				return true;
		}
		return false;
	}

	uint layerCount() const {
		return _layers.size();
	}

	bool hasLayer(uint index) const {
		return index < _layers.size();
	}

	const ResourceSpriteLayer &layer(uint index) const {
		return _layers[index];
	}

	ResourceSpriteLayer &layer(uint index) {
		return _layers[index];
	}

	byte layerFrame(uint index) const {
		if (index >= _layers.size())
			return 0;
		return _layers[index].frameIndex;
	}

	bool layerVisible(uint index) const {
		return index < _layers.size() && _layers[index].visible;
	}

	uint addLayer(uint chunkIndex, uint16 descriptorCount, const byte *frameMap,
			uint frameMapSize, bool visible = true) {
		const uint index = _layers.size();
		configureLayer(index, chunkIndex, descriptorCount, frameMap, frameMapSize, visible);
		return index;
	}

	void configureLayer(uint index, uint chunkIndex, uint16 descriptorCount, const byte *frameMap,
			uint frameMapSize, bool visible = true) {
		if (index >= _layers.size())
			_layers.resize(index + 1);

		ResourceSpriteLayer &target = _layers[index];
		target.configure(chunkIndex, descriptorCount, frameMap, frameMapSize);
		target.visible = visible && availableFrameCount(target) != 0;
	}

	void setLayerVisible(uint index, bool visible) {
		if (index < _layers.size())
			_layers[index].visible = visible && availableFrameCount(_layers[index]) != 0;
	}

	void setLayerFrame(uint index, byte frameIndex) {
		if (index < _layers.size() && _layers[index].visible)
			_layers[index].setFrame(frameIndex);
	}

	void setLayerFramePreservingVisibility(uint index, byte frameIndex) {
		if (index < _layers.size())
			_layers[index].setFrame(frameIndex);
	}

	void setLayerFrameClamped(uint index, uint frameIndex) {
		if (index >= _layers.size() || !_layers[index].visible)
			return;

		const uint count = availableFrameCount(_layers[index]);
		if (count == 0)
			return;

		if (frameIndex >= count)
			frameIndex = count - 1;
		if (frameIndex > 0xff)
			frameIndex = 0xff;
		_layers[index].setFrame((byte)frameIndex);
	}

	uint frameCount() const {
		uint result = 0;
		for (uint i = 0; i < _layers.size(); ++i) {
			const uint count = layerFrameCount(_layers[i]);
			if (count > result)
				result = count;
		}
		return result;
	}

private:
	uint layerFrameCount(const ResourceSpriteLayer &target) const {
		if (!target.visible)
			return 0;
		return availableFrameCount(target);
	}

	uint availableFrameCount(const ResourceSpriteLayer &target) const {
		if (target.frameMap != nullptr)
			return target.frameMapSize;
		return target.descriptorCount;
	}

	Common::Array<ResourceSpriteLayer> _layers;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H
