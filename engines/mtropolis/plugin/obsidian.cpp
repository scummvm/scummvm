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

#include "mtropolis/plugin/obsidian.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

namespace MTropolis {

namespace Obsidian {

bool MovementModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::MovementModifier &data) {
	// FIXME: Map these
	_rate = 0;
	_frequency = 0;
	_type = false;
	_dest = Point16::create(0, 0);

	return true;
}

MiniscriptInstructionOutcome MovementModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "type") {
		DynamicValueWriteBoolHelper::create(&_type, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "dest") {
		DynamicValueWritePointHelper::create(&_dest, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "rate") {
		DynamicValueWriteIntegerHelper<int32>::create(&_rate, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "frequency") {
		DynamicValueWriteIntegerHelper<int32>::create(&_frequency, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

Common::SharedPtr<Modifier> MovementModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MovementModifier(*this));
}

bool RectShiftModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::RectShiftModifier &data) {
	if (data.rate.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	_direction = 0;
	_rate = data.rate.value.asInt;

	return true;
}

MiniscriptInstructionOutcome RectShiftModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "rate") {
		DynamicValueWriteIntegerHelper<int32>::create(&_rate, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "direction") {
		DynamicValueWriteIntegerHelper<int32>::create(&_direction, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

Common::SharedPtr<Modifier> RectShiftModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new RectShiftModifier(*this));
}

TextWorkModifier::TextWorkModifier() : _firstChar(0), _lastChar(0) {
}

bool TextWorkModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::TextWorkModifier &data) {
	return true;
}

bool TextWorkModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "numchar") {
		result.setInt(_string.size());
		return true;
	} else if (attrib == "output") {
		int32 firstChar = _firstChar - 1;
		int32 len = _lastChar - _firstChar + 1;
		if (_firstChar < 0) {
			len += firstChar;
			firstChar = 0;
		}
		if (len <= 0 || static_cast<size_t>(firstChar) >= _string.size())
			result.setString("");
		else {
			const size_t availChars = _string.size() - firstChar;
			if (static_cast<size_t>(len) > availChars)
				len = availChars;
			result.setString(_string.substr(firstChar, len));
		}
		return true;
	} else if (attrib == "exists") {
		bool exists = (_string.find(_token) != Common::String::npos);
		result.setInt(exists ? 1 : 0);
		return true;
	} else if (attrib == "index") {
		size_t index = _string.find(_token);
		if (index == Common::String::npos)
			index = 0;
		else
			index++;

		result.setInt(index);
		return true;
	} else if (attrib == "numword") {
		int numWords = 0;
		bool lastWasWhitespace = true;
		for (size_t i = 0; i < _string.size(); i++) {
			char c = _string[i];
			bool isWhitespace = (c <= ' ');
			if (lastWasWhitespace && !isWhitespace)
				numWords++;
			lastWasWhitespace = isWhitespace;
		}

		result.setInt(numWords);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome TextWorkModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "string") {
		DynamicValueWriteStringHelper::create(&_string, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "firstchar") {
		DynamicValueWriteIntegerHelper<int32>::create(&_firstChar, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "lastchar") {
		DynamicValueWriteIntegerHelper<int32>::create(&_lastChar, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "token") {
		DynamicValueWriteStringHelper::create(&_token, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "firstword") {
		DynamicValueWriteFuncHelper<TextWorkModifier, &TextWorkModifier::scriptSetFirstWord>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "lastword") {
		DynamicValueWriteFuncHelper<TextWorkModifier, &TextWorkModifier::scriptSetLastWord>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

Common::SharedPtr<Modifier> TextWorkModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TextWorkModifier(*this));
}

MiniscriptInstructionOutcome TextWorkModifier::scriptSetFirstWord(MiniscriptThread *thread, const DynamicValue &value) {
	// This and lastword are only used in tandem with lastword, exact functionality is unclear since it's
	// also used in tandem with "output" which is normally used with firstchar+lastchar.
	// We attempt to emulate it by setting firstchar+lastchar to the correct values
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	int numWords = 0;
	bool lastWasWhitespace = true;
	for (size_t i = 0; i < _string.size(); i++) {
		char c = _string[i];
		bool isWhitespace = (c <= ' ');
		if (lastWasWhitespace && !isWhitespace) {
			numWords++;

			if (numWords == asInteger) {
				_firstChar = i + 1;
				return kMiniscriptInstructionOutcomeContinue;
			}
		}
		lastWasWhitespace = isWhitespace;
	}

	thread->error("Invalid index for 'firstword'");
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome TextWorkModifier::scriptSetLastWord(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	int numWordEnds = 0;
	bool lastWasWhitespace = true;
	for (size_t i = 0; i < _string.size(); i++) {
		char c = _string[i];
		bool isWhitespace = (c <= ' ');
		if (!lastWasWhitespace && isWhitespace) {
			numWordEnds++;

			if (numWordEnds == asInteger) {
				_firstChar = i - 1;
				return kMiniscriptInstructionOutcomeContinue;
			}
		}
		lastWasWhitespace = isWhitespace;

		if (numWordEnds == asInteger) {
			_lastChar = i;
			return kMiniscriptInstructionOutcomeContinue;
		}
	}

	if (!lastWasWhitespace) {
		numWordEnds++;
		if (numWordEnds == asInteger) {
			_lastChar = _string.size();
			return kMiniscriptInstructionOutcomeContinue;
		}
	}

	thread->error("Invalid index for 'firstword'");
	return kMiniscriptInstructionOutcomeFailed;
}

DictionaryModifier::DictionaryModifier() : _plugIn(nullptr) {
}

bool DictionaryModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::DictionaryModifier &data) {
	if (data.str.type != Data::PlugInTypeTaggedValue::kString)
		return false;

	_str = data.str.str;

	if (data.index.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	_index = data.index.value.asInt;
	_isIndexResolved = true;
	_plugIn = static_cast<ObsidianPlugIn *>(context.plugIn);

	return true;
}

bool DictionaryModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "index") {
		resolveStringIndex();
		result.setInt(_index);
		return true;
	}
	if (attrib == "string") {
		result.setString(_str);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome DictionaryModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "index") {
		DynamicValueWriteFuncHelper<DictionaryModifier, &DictionaryModifier::scriptSetIndex>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "string") {
		DynamicValueWriteFuncHelper<DictionaryModifier, &DictionaryModifier::scriptSetString>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

void DictionaryModifier::resolveStringIndex() {
	if (_isIndexResolved)
		return;

	_index = 0;
	_isIndexResolved = true;

	const Common::Array<WordGameData::WordBucket> &wordBuckets = _plugIn->getWordGameData()->getWordBuckets();

	size_t strLength = _str.size();
	if (strLength >= wordBuckets.size())
		return;

	const WordGameData::WordBucket &bucket = wordBuckets[strLength];

	size_t lowOffsetInclusive = 0;
	size_t highOffsetExclusive = bucket.wordIndexes.size();

	const char *strChars = _str.c_str();

	// Binary search
	while (lowOffsetInclusive != highOffsetExclusive) {
		const size_t midOffset = (lowOffsetInclusive + highOffsetExclusive) / 2;
		const char *chars = &bucket.chars[bucket.spacing * midOffset];

		bool isMidGreater = false;
		bool isMidLess = false;
		for (size_t i = 0; i < strLength; i++) {
			if (chars[i] > strChars[i]) {
				isMidGreater = true;
				break;
			} else if (chars[i] < strChars[i]) {
				isMidLess = true;
				break;
			}
		}

		if (isMidLess)
			lowOffsetInclusive = midOffset + 1;
		else if (isMidGreater)
			highOffsetExclusive = midOffset;
		else {
			_index = bucket.wordIndexes[midOffset] + 1;
			break;
		}
	}
}

MiniscriptInstructionOutcome DictionaryModifier::scriptSetString(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString) {
		thread->error("Tried to set dictionary string to something that wasn't a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (_str != value.getString()) {
		_str = value.getString();
		_isIndexResolved = false;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome DictionaryModifier::scriptSetIndex(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Tried to set dictionary index to something that wasn't a number");
		return kMiniscriptInstructionOutcomeFailed;
	}

	_index = asInteger;
	if (_index < 1)
		_str.clear();
	else {
		const size_t indexAdjusted = _index - 1;
		const Common::Array<WordGameData::SortedWord> &sortedWords = _plugIn->getWordGameData()->getSortedWords();
		if (indexAdjusted >= sortedWords.size())
			_str.clear();
		else
			_str = Common::String(sortedWords[indexAdjusted].chars, sortedWords[indexAdjusted].length);
	}

	_isIndexResolved = true;

	return kMiniscriptInstructionOutcomeContinue;
}

Common::SharedPtr<Modifier> DictionaryModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new DictionaryModifier(*this));
}

WordMixerModifier::WordMixerModifier() : _matches(0), _result(0) {
}

bool WordMixerModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::WordMixerModifier &data) {
	_plugIn = static_cast<const ObsidianPlugIn *>(context.plugIn);

	return true;
}

bool WordMixerModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "result") {
		result.setInt(_result);
		return true;
	}
	if (attrib == "matches") {
		result.setInt(_matches);
		return true;
	}
	if (attrib == "output") {
		result.setString(_output);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome WordMixerModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "input") {
		DynamicValueWriteFuncHelper<WordMixerModifier, &WordMixerModifier::scriptSetInput>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "search") {
		DynamicValueWriteFuncHelper<WordMixerModifier, &WordMixerModifier::scriptSetSearch>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome WordMixerModifier::scriptSetInput(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString) {
		thread->error("Invalid type for WordMixer input attribute");
		return kMiniscriptInstructionOutcomeFailed;
	}

	_input = value.getString();

	Common::Array<char> sourceChars;
	for (char c : _input) {
		if (c > ' ')
			sourceChars.push_back(invariantToLower(c));
	}

	Common::Array<bool> charIsUsed;
	charIsUsed.resize(sourceChars.size());

	const Common::Array<WordGameData::WordBucket> &wordBuckets = _plugIn->getWordGameData()->getWordBuckets();

	_output.clear();
	_matches = 0;

	size_t numWordBuckets = wordBuckets.size();
	for (size_t rbucket = 0; rbucket < numWordBuckets; rbucket++) {
		size_t wordLength = numWordBuckets - 1 - rbucket;

		const WordGameData::WordBucket &bucket = wordBuckets[wordLength];

		size_t numWords = bucket.wordIndexes.size();

		for (size_t wi = 0; wi < numWords; wi++) {
			const char *wordChars = &bucket.chars[bucket.spacing * wi];

			for (bool &b : charIsUsed)
				b = false;

			bool isMatch = true;
			for (size_t ci = 0; ci < wordLength; ci++) {
				const char wordChar = wordChars[ci];

				bool foundAvailableSource = false;
				for (size_t srci = 0; srci < sourceChars.size(); srci++) {
					if (sourceChars[srci] == wordChar && !charIsUsed[srci]) {
						foundAvailableSource = true;
						charIsUsed[srci] = true;
						break;
					}
				}

				if (!foundAvailableSource) {
					isMatch = false;
					break;
				}
			}

			if (isMatch) {
				if (_matches > 0)
					_output += ' ';
				_output += Common::String(wordChars, wordLength);
				_matches++;
			}
		}

		if (_matches > 0)
			break;
	}

	if (_matches == 0)
		_output = "xxx";

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome WordMixerModifier::scriptSetSearch(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean) {
		thread->error("Invalid type for WordMixer search attribute");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (!value.getBool())
		return kMiniscriptInstructionOutcomeContinue;

	size_t searchLength = _input.size();
	const Common::Array<WordGameData::WordBucket> &buckets = _plugIn->getWordGameData()->getWordBuckets();
	_result = 0;
	if (searchLength < buckets.size()) {
		const WordGameData::WordBucket &bucket = buckets[searchLength];

		bool found = false;
		for (size_t wi = 0; wi < bucket.wordIndexes.size(); wi++) {
			const char *wordChars = &bucket.chars[wi * bucket.spacing];

			bool isMatch = true;
			for (size_t ci = 0; ci < searchLength; ci++) {
				if (invariantToLower(_input[ci]) != wordChars[ci]) {
					isMatch = false;
					break;
				}
			}

			if (isMatch) {
				_result = 1;
				break;
			}
		}
	}

	return kMiniscriptInstructionOutcomeContinue;
}

Common::SharedPtr<Modifier> WordMixerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new WordMixerModifier(*this));
}

XorModModifier::XorModModifier() {
}

bool XorModModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::XorModModifier &data) {
	if (data.enableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_enableWhen.load(data.enableWhen.value.asEvent))
		return false;

	if (data.disableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_disableWhen.load(data.disableWhen.value.asEvent))
		return false;

	if (data.shapeID.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	_shapeID = data.shapeID.value.asInt;

	return true;
}

bool XorModModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState XorModModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	Structural *owner = findStructuralOwner();
	if (!owner)
		return kVThreadError;

	if (!owner->isElement())
		return kVThreadReturn;

	Element *element = static_cast<Element *>(owner);
	if (!element->isVisual())
		return kVThreadReturn;

	VisualElement *visual = static_cast<VisualElement *>(element);

	VisualElementRenderProperties renderProps = visual->getRenderProperties();
	renderProps.setInkMode(VisualElementRenderProperties::kInkModeXor);

	if (_shapeID == 0)
		renderProps.setShape(VisualElementRenderProperties::kShapeRect);
	else
		renderProps.setShape(static_cast<VisualElementRenderProperties::Shape>(VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri1 + _shapeID - 1));

	visual->setRenderProperties(renderProps);

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> XorModModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new XorModModifier(*this));
}

XorCheckModifier::XorCheckModifier() : _allClear(false) {
}

bool XorCheckModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::XorCheckModifier &data) {
	return true;
}

bool XorCheckModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "allclear") {
		result.setBool(_allClear);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome XorCheckModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "checknow") {
		DynamicValueWriteFuncHelper<XorCheckModifier, &XorCheckModifier::scriptSetCheckNow>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

Common::SharedPtr<Modifier> XorCheckModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new XorCheckModifier(*this));
}

MiniscriptInstructionOutcome XorCheckModifier::scriptSetCheckNow(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	if (!value.getBool())
		return kMiniscriptInstructionOutcomeContinue;

	Structural *scene = findStructuralOwner();
	while (!scene->getParent()->isSubsection())
		scene = scene->getParent();

	Common::Array<VisualElement *> xorElements;
	recursiveFindXorElements(scene, xorElements);

	Rect16 triRects[4];
	for (int i = 0; i < 4; i++)
		triRects[i] = Rect16::create(0, 0, 0, 0);

	Common::Array<Rect16> pendingRects;

	for (VisualElement *element : xorElements) {
		VisualElementRenderProperties::Shape shape = element->getRenderProperties().getShape();
		Rect16 rect = element->getRelativeRect();
		Point16 absOrigin = element->getCachedAbsoluteOrigin();
		Rect16 absRect = rect.translate(absOrigin.x - rect.left, absOrigin.y - rect.top);

		if (shape >= VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri1 && shape <= VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri4)
			triRects[shape - VisualElementRenderProperties::kShapeObsidianCanvasPuzzleTri1] = absRect;
		else
			pendingRects.push_back(absRect);
	}

	// The canvas puzzle has 4 triangles, right-angled in each corner, pairs 1-4 and 2-3 form rects.
	// It isn't possible to solve the puzzle unless both rects are formed.  So, we do this by forming the rects and
	// then eliminating overlaps.  If the rects can't be formed, the puzzle fails.
	if (triRects[0] == triRects[3])
		pendingRects.push_back(triRects[0]);
	else {
		_allClear = false;
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (triRects[1] == triRects[2])
		pendingRects.push_back(triRects[1]);
	else {
		_allClear = false;
		return kMiniscriptInstructionOutcomeContinue;
	}

	Common::Array<Rect16> maskedRects;
	while (pendingRects.size() > 0) {
		const Rect16 pendingRect = pendingRects.back();
		pendingRects.pop_back();

		bool hasIntersection = false;
		size_t intersectionIndex = 0;
		for (size_t j = 0; j < maskedRects.size(); j++) {
			if (maskedRects[j].intersect(pendingRect).isValid()) {
				hasIntersection = true;
				intersectionIndex = j;
			}
		}

		if (!hasIntersection) {
			maskedRects.push_back(pendingRect);
			continue;
		}

		if (pendingRect == maskedRects[intersectionIndex]) {
			// Total overlap
			maskedRects.remove_at(intersectionIndex);
			continue;
		}

		const Rect16 intersectingRect = maskedRects[intersectionIndex];

		// Try to subdivide the intersecting rect using one of the axes of the incoming rect.
		// If this succeeds, requeue the intersecting rect fragments and add the pending rect
		// to the workspace.  Since that amounts to replacement, just replace the rect.
		if (sliceRectX(intersectingRect, pendingRect.left, pendingRects)
			|| sliceRectX(intersectingRect, pendingRect.right, pendingRects)
			|| sliceRectY(intersectingRect, pendingRect.top, pendingRects)
			|| sliceRectY(intersectingRect, pendingRect.bottom, pendingRects)) {
			maskedRects[intersectionIndex] = pendingRect;
			continue;
		}

		// Try to subdivide the pending rect using one of the axes of the intersecting rect.
		// If this succeeds, the fragments will be requeued and no further action is needed.
		if (sliceRectX(pendingRect, intersectingRect.left, pendingRects)
			|| sliceRectX(pendingRect, intersectingRect.right, pendingRects)
			|| sliceRectY(pendingRect, intersectingRect.top, pendingRects)
			|| sliceRectY(pendingRect, intersectingRect.bottom, pendingRects)) {
			continue;
		}

		// This should never happen
		assert(false);
		return kMiniscriptInstructionOutcomeFailed;
	}

	_allClear = (maskedRects.size() == 0);

	return kMiniscriptInstructionOutcomeContinue;
}

void XorCheckModifier::recursiveFindXorElements(Structural *structural, Common::Array<VisualElement *> &elements) {
	for (const Common::SharedPtr<Structural> &child : structural->getChildren())
		recursiveFindXorElements(child.get(), elements);

	if (!structural->isElement())
		return;

	Element *element = static_cast<Element *>(structural);
	if (!element->isVisual())
		return;

	VisualElement *visual = static_cast<VisualElement *>(element);
	if (visual->getRenderProperties().getInkMode() == VisualElementRenderProperties::kInkModeXor)
		elements.push_back(visual);
}

bool XorCheckModifier::sliceRectX(const Rect16 &rect, int32 x, Common::Array<Rect16> &outSlices) {
	if (x > rect.left && x < rect.right) {
		Rect16 leftSlice = Rect16::create(rect.left, rect.top, x, rect.bottom);
		Rect16 rightSlice = Rect16::create(x, rect.top, rect.right, rect.bottom);
		outSlices.push_back(leftSlice);
		outSlices.push_back(rightSlice);
		return true;
	}

	return false;
}

bool XorCheckModifier::sliceRectY(const Rect16 &rect, int32 y, Common::Array<Rect16> &outSlices) {
	if (y > rect.top && y < rect.bottom) {
		Rect16 topSlice = Rect16::create(rect.left, rect.top, rect.right, y);
		Rect16 bottomSlice = Rect16::create(rect.left, y, rect.right, rect.bottom);
		outSlices.push_back(topSlice);
		outSlices.push_back(bottomSlice);
		return true;
	}

	return false;
}

ObsidianPlugIn::ObsidianPlugIn(const Common::SharedPtr<WordGameData> &wgData)
	: _movementModifierFactory(this), _rectShiftModifierFactory(this), _textWorkModifierFactory(this),
	  _dictionaryModifierFactory(this), _wordMixerModifierFactory(this), _xorModModifierFactory(this),
	  _xorCheckModifierFactory(this), _wgData(wgData) {
}

void ObsidianPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("Movement", &_movementModifierFactory);
	registrar->registerPlugInModifier("rectshift", &_rectShiftModifierFactory);
	registrar->registerPlugInModifier("TextWork", &_textWorkModifierFactory);
	registrar->registerPlugInModifier("Dictionary", &_dictionaryModifierFactory);
	registrar->registerPlugInModifier("WordMixer", &_wordMixerModifierFactory);
	registrar->registerPlugInModifier("xorMod", &_xorModModifierFactory);
	registrar->registerPlugInModifier("xorCheck", &_xorCheckModifierFactory);
}

const Common::SharedPtr<WordGameData>& ObsidianPlugIn::getWordGameData() const {
	return _wgData;
}

bool WordGameData::load(Common::SeekableReadStream *stream, const WordGameLoadBucket *buckets, uint numBuckets, uint alignment, bool backwards) {
	_buckets.resize(numBuckets);

	size_t totalWords = 0;
	for (size_t i = 0; i < numBuckets; i++) {
		const WordGameLoadBucket &inBucket = buckets[i];
		WordBucket &outBucket = _buckets[i];

		uint32 sizeBytes = inBucket.endAddress - inBucket.startAddress;
		uint wordLength = i;
		uint spacing = (wordLength + alignment) - (wordLength % alignment);

		outBucket.spacing = spacing;
		outBucket.chars.resize(sizeBytes);

		assert(sizeBytes % alignment == 0);

		if (sizeBytes > 0) {
			if (!stream->seek(inBucket.startAddress, SEEK_SET))
				return false;
			stream->read(&outBucket.chars[0], sizeBytes);
		}

		uint numWords = sizeBytes / spacing;
		outBucket.wordIndexes.resize(numWords);

		if (backwards) {
			for (size_t wordIndex = 0; wordIndex < numWords / 2; wordIndex++) {
				char *swapA = &outBucket.chars[wordIndex * spacing];
				char *swapB = &outBucket.chars[(numWords - 1 - wordIndex) * spacing];

				for (size_t chIndex = 0; chIndex < wordLength; chIndex++) {
					char temp = swapA[chIndex];
					swapA[chIndex] = swapB[chIndex];
					swapB[chIndex] = temp;
				}
			}
		}

		totalWords += numWords;
	}

	_sortedWords.resize(totalWords);

	Common::Array<size_t> currentWordIndexes;
	currentWordIndexes.resize(numBuckets);

	for (size_t i = 0; i < numBuckets; i++)
		currentWordIndexes[i] = 0;

	for (size_t wordIndex = 0; wordIndex < totalWords; wordIndex++) {
		size_t bestBucket = numBuckets;
		const char *bestChars = nullptr;
		for (size_t bucketIndex = 0; bucketIndex < numBuckets; bucketIndex++) {
			size_t wordOffset = currentWordIndexes[bucketIndex] * _buckets[bucketIndex].spacing;
			if (wordOffset < _buckets[bucketIndex].chars.size()) {
				const char *candidate = &_buckets[bucketIndex].chars[wordOffset];
				bool isWorse = true;
				if (bestChars == nullptr)
					isWorse = false;
				else  {
					// The best bucket will always be shorter if it's set, so this is only better if it precedes it alphabetically
					for (size_t i = 0; i < bestBucket; i++) {
						if (candidate[i] > bestChars[i]) {
							break;
						} else if (candidate[i] < bestChars[i]) {
							isWorse = false;
							break;
						}
					}
				}

				if (!isWorse) {
					bestBucket = bucketIndex;
					bestChars = candidate;
				}
			}
		}

		assert(bestChars != nullptr);

		const size_t bucketWordIndex = currentWordIndexes[bestBucket];
		_buckets[bestBucket].wordIndexes[bucketWordIndex] = wordIndex;
		currentWordIndexes[bestBucket]++;

		_sortedWords[wordIndex].chars = bestChars;
		_sortedWords[wordIndex].length = bestBucket;
	}

	return !stream->err();
}

const Common::Array<WordGameData::WordBucket> &WordGameData::getWordBuckets() const {
	return _buckets;
}

const Common::Array<WordGameData::SortedWord>& WordGameData::getSortedWords() const {
	return _sortedWords;
}

} // End of namespace ObsidianPlugIn

namespace PlugIns {

Common::SharedPtr<PlugIn> createObsidian(const Common::SharedPtr<Obsidian::WordGameData> &wgData) {
	return Common::SharedPtr<PlugIn>(new Obsidian::ObsidianPlugIn(wgData));
}

} // End of namespace PlugIns

} // End of namespace MTropolis
