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

#include "mtropolis/element_factory.h"
#include "mtropolis/elements.h"
#include "mtropolis/runtime.h"

namespace MTropolis {

ElementLoaderContext::ElementLoaderContext(Runtime *runtime, size_t streamIndex) : runtime(runtime), streamIndex(streamIndex) {
}

template<typename TElement, typename TElementData>
class ElementFactory : public IElementFactory {
public:
	Common::SharedPtr<Element> createElement(ElementLoaderContext &context, const Data::DataObject &dataObject) const override;
	static IElementFactory *getInstance();

private:
	static ElementFactory<TElement, TElementData> _instance;
};

template<typename TElement, typename TElementData>
Common::SharedPtr<Element> ElementFactory<TElement, TElementData>::createElement(ElementLoaderContext &context, const Data::DataObject &dataObject) const {
	Common::SharedPtr<TElement> element(new TElement());

	if (!element->load(context, static_cast<const TElementData &>(dataObject)))
		element.reset();
	else
		element->setSelfReference(element);

	return Common::SharedPtr<Element>(element);
}

template<typename TElement, typename TElementData>
IElementFactory *ElementFactory<TElement, TElementData>::getInstance() {
	return &_instance;
}

template<typename TElement, typename TElementData>
ElementFactory<TElement, TElementData> ElementFactory<TElement, TElementData>::_instance;

IElementFactory *getElementFactoryForDataObjectType(const Data::DataObjectTypes::DataObjectType dataObjectType) {
	switch (dataObjectType) {
	case Data::DataObjectTypes::kGraphicElement:
		return ElementFactory<GraphicElement, Data::GraphicElement>::getInstance();
	case Data::DataObjectTypes::kMovieElement:
		return ElementFactory<MovieElement, Data::MovieElement>::getInstance();
	case Data::DataObjectTypes::kImageElement:
		return ElementFactory<ImageElement, Data::ImageElement>::getInstance();
	case Data::DataObjectTypes::kMToonElement:
		return ElementFactory<MToonElement, Data::MToonElement>::getInstance();
	case Data::DataObjectTypes::kTextLabelElement:
		return ElementFactory<TextLabelElement, Data::TextLabelElement>::getInstance();
	case Data::DataObjectTypes::kSoundElement:
		return ElementFactory<SoundElement, Data::SoundElement>::getInstance();

	default:
		return nullptr;
	}
}

} // End of namespace MTropolis
