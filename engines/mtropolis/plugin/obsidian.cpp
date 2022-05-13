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
	return true;
}

Common::SharedPtr<Modifier> MovementModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MovementModifier(*this));
}

bool RectShiftModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::RectShiftModifier &data) {
	return true;
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
		thread->error("Not-yet-implemented TextWork attribute 'numword'");
		return false;
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
		thread->error("Not-yet-implemented TextWork attrib 'firstword'");
		return kMiniscriptInstructionOutcomeFailed;
	} else if (attrib == "lastword") {
		thread->error("Not-yet-implemented TextWork attrib 'lastword'");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

Common::SharedPtr<Modifier> TextWorkModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TextWorkModifier(*this));
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

WordMixerModifier::WordMixerModifier() {
}

bool WordMixerModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::WordMixerModifier &data) {
	return true;
}

Common::SharedPtr<Modifier> WordMixerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new WordMixerModifier(*this));
}

ObsidianPlugIn::ObsidianPlugIn(const Common::SharedPtr<WordGameData> &wgData)
	: _movementModifierFactory(this), _rectShiftModifierFactory(this), _textWorkModifierFactory(this),
	  _dictionaryModifierFactory(this), _wordMixerModifierFactory(this), _wgData(wgData) {
}

void ObsidianPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("Movement", &_movementModifierFactory);
	registrar->registerPlugInModifier("rectshift", &_rectShiftModifierFactory);
	registrar->registerPlugInModifier("TextWork", &_textWorkModifierFactory);
	registrar->registerPlugInModifier("Dictionary", &_dictionaryModifierFactory);
	registrar->registerPlugInModifier("WordMixer", &_wordMixerModifierFactory);
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
