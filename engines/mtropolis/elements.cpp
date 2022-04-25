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

#include "mtropolis/elements.h"

namespace MTropolis {

GraphicElement::GraphicElement() : _directToScreen(false), _cacheBitmap(false) {
}

GraphicElement::~GraphicElement() {
}

bool GraphicElement::load(ElementLoaderContext &context, const Data::GraphicElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_directToScreen = ((data.elementFlags & Data::ElementFlags::kNotDirectToScreen) == 0);
	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);

	return true;
}

MovieElement::MovieElement()
	: _directToScreen(false), _cacheBitmap(false), _paused(false)
	, _loop(false), _alternate(false), _playEveryFrame(false), _assetID(0) {
}

MovieElement::~MovieElement() {
}

bool MovieElement::load(ElementLoaderContext &context, const Data::MovieElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_directToScreen = ((data.elementFlags & Data::ElementFlags::kNotDirectToScreen) == 0);
	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_paused = ((data.elementFlags & Data::ElementFlags::kPaused) != 0);
	_loop = ((data.animationFlags & Data::AnimationFlags::kLoop) != 0);
	_alternate = ((data.animationFlags & Data::AnimationFlags::kAlternate) != 0);
	_playEveryFrame = ((data.animationFlags & Data::AnimationFlags::kPlayEveryFrame) != 0);
	_assetID = data.assetID;

	return true;
}

bool MovieElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "direct") {
		result.setBool(_directToScreen);
		return true;
	}

	if (attrib == "paused") {
		result.setBool(_paused);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

bool MovieElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "direct") {
		writeProxy = DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetDirect>::create(this);
		return true;
	}

	if (attrib == "paused") {
		writeProxy = DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetPaused>::create(this);
		return true;
	}
	
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

bool MovieElement::scriptSetDirect(const DynamicValue &dest) {
	if (dest.getType() == DynamicValueTypes::kBoolean) {
		_directToScreen = dest.getBool();
		return true;
	}
	return false;
}

bool MovieElement::scriptSetPaused(const DynamicValue& dest) {
	if (dest.getType() == DynamicValueTypes::kBoolean) {
		_paused = dest.getBool();
		return true;
	}
	return false;
}

} // End of namespace MTropolis
