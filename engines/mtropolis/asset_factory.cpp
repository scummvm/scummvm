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

#include "mtropolis/asset_factory.h"
#include "mtropolis/assets.h"

namespace MTropolis {

AssetLoaderContext::AssetLoaderContext(size_t pStreamIndex) : streamIndex(pStreamIndex) {
}

template<typename TAsset, typename TAssetData>
class AssetFactory {
public:
	static Common::SharedPtr<Asset> createAsset(AssetLoaderContext &context, const Data::DataObject &dataObject);
	static SIAssetFactory *getInstance();

private:
	static SIAssetFactory _instance;
};

template<typename TAsset, typename TAssetData>
Common::SharedPtr<Asset> AssetFactory<TAsset, TAssetData>::createAsset(AssetLoaderContext &context, const Data::DataObject &dataObject) {
	Common::SharedPtr<TAsset> asset(new TAsset());

	if (!asset->load(context, static_cast<const TAssetData &>(dataObject)))
		asset.reset();

	return Common::SharedPtr<Asset>(asset);
}

template<typename TAsset, typename TAssetData>
SIAssetFactory *AssetFactory<TAsset, TAssetData>::getInstance() {
	return &_instance;
}

template<typename TAsset, typename TAssetData>
SIAssetFactory AssetFactory<TAsset, TAssetData>::_instance = {
	AssetFactory<TAsset, TAssetData>::createAsset
};

SIAssetFactory *getAssetFactoryForDataObjectType(const Data::DataObjectTypes::DataObjectType dataObjectType) {
	switch (dataObjectType) {
	case Data::DataObjectTypes::kColorTableAsset:
		return AssetFactory<ColorTableAsset, Data::ColorTableAsset>::getInstance();
	case Data::DataObjectTypes::kAudioAsset:
		return AssetFactory<AudioAsset, Data::AudioAsset>::getInstance();
	case Data::DataObjectTypes::kMovieAsset:
		return AssetFactory<MovieAsset, Data::MovieAsset>::getInstance();
	case Data::DataObjectTypes::kAVIMovieAsset:
		return AssetFactory<AVIMovieAsset, Data::AVIMovieAsset>::getInstance();
	case Data::DataObjectTypes::kImageAsset:
		return AssetFactory<ImageAsset, Data::ImageAsset>::getInstance();
	case Data::DataObjectTypes::kMToonAsset:
		return AssetFactory<MToonAsset, Data::MToonAsset>::getInstance();
	case Data::DataObjectTypes::kTextAsset:
		return AssetFactory<TextAsset, Data::TextAsset>::getInstance();

	default:
		return nullptr;
	}
}

} // End of namespace MTropolis
