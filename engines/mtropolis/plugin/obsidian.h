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

#ifndef MTROPOLIS_PLUGIN_OBSIDIAN_H
#define MTROPOLIS_PLUGIN_OBSIDIAN_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/obsidian_data.h"

namespace MTropolis {

namespace Obsidian {

class ObsidianPlugIn;
class WordGameData;

class MovementModifier : public Modifier {
public:
	MovementModifier();
	~MovementModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::MovementModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Movement Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void triggerMove(Runtime *runtime);

	Common::Point _dest;
	bool _type;
	double _rate;
	int32 _frequency;

	Event _enableWhen;
	Event _disableWhen;

	Event _triggerEvent;

	Common::Point _moveStartPoint;
	uint64 _moveStartTime;

	Common::SharedPtr<ScheduledEvent> _moveEvent;
	Runtime *_runtime;
};

class RectShiftModifier : public Modifier, public IPostEffect {
public:
	RectShiftModifier();
	~RectShiftModifier();

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::RectShiftModifier &data);

	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	void renderPostEffect(Graphics::ManagedSurface &surface) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Rect Shift Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _enableWhen;
	Event _disableWhen;

	int32 _direction;

	Runtime *_runtime;
	bool _isActive;
};

class TextWorkModifier : public Modifier {
public:
	TextWorkModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::TextWorkModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "TextWork Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetFirstWord(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetLastWord(MiniscriptThread *thread, const DynamicValue &value);

	Common::String _string;
	Common::String _token;

	// These appear to be 1-based?
	int32 _firstChar;
	int32 _lastChar;
};

class DictionaryModifier : public Modifier {
public:
	DictionaryModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::DictionaryModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Dictionary Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	void resolveStringIndex();
	MiniscriptInstructionOutcome scriptSetString(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetIndex(MiniscriptThread *thread, const DynamicValue &value);

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Common::String _str;

	const ObsidianPlugIn *_plugIn;
	int32 _index;
	bool _isIndexResolved;
};

class WordMixerModifier : public Modifier {
public:
	WordMixerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::WordMixerModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "WordMixer Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetInput(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetSearch(MiniscriptThread *thread, const DynamicValue &value);

	Common::String _input;
	Common::String _output;
	int _matches;
	int _result;

	const ObsidianPlugIn *_plugIn;
};

class XorModModifier : public Modifier {
public:
	XorModModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::XorModModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Xor Mod Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _enableWhen;
	Event _disableWhen;

	int32 _shapeID;
};

class XorCheckModifier : public Modifier {
public:
	XorCheckModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::XorCheckModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Xor Check Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetCheckNow(MiniscriptThread *thread, const DynamicValue &value);

	static void recursiveFindXorElements(Structural *structural, Common::Array<VisualElement *> &elements);
	static bool sliceRectX(const Common::Rect &rect, int32 x, Common::Array<Common::Rect> &outSlices);
	static bool sliceRectY(const Common::Rect &rect, int32 y, Common::Array<Common::Rect> &outSlices);

	bool _allClear;
};

class ObsidianPlugIn : public MTropolis::PlugIn {
public:
	ObsidianPlugIn(const Common::SharedPtr<WordGameData> &wgData);

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

	const Common::SharedPtr<WordGameData> &getWordGameData() const;

private:
	PlugInModifierFactory<MovementModifier, Data::Obsidian::MovementModifier> _movementModifierFactory;
	PlugInModifierFactory<RectShiftModifier, Data::Obsidian::RectShiftModifier> _rectShiftModifierFactory;
	PlugInModifierFactory<TextWorkModifier, Data::Obsidian::TextWorkModifier> _textWorkModifierFactory;
	PlugInModifierFactory<WordMixerModifier, Data::Obsidian::WordMixerModifier> _wordMixerModifierFactory;
	PlugInModifierFactory<DictionaryModifier, Data::Obsidian::DictionaryModifier> _dictionaryModifierFactory;
	PlugInModifierFactory<XorModModifier, Data::Obsidian::XorModModifier> _xorModModifierFactory;
	PlugInModifierFactory<XorCheckModifier, Data::Obsidian::XorCheckModifier> _xorCheckModifierFactory;

	Common::SharedPtr<WordGameData> _wgData;
};

struct WordGameLoadBucket {
	uint32 startAddress;
	uint32 endAddress;
};

class WordGameData {
public:
	struct WordBucket {
		Common::Array<char> chars;
		Common::Array<uint16> wordIndexes;
		uint32 spacing;
	};

	struct SortedWord {
		const char *chars;
		uint length;
	};

	bool load(Common::SeekableReadStream *stream, const WordGameLoadBucket *buckets, uint numBuckets, uint alignment, bool backwards);

	const Common::Array<WordBucket> &getWordBuckets() const;
	const Common::Array<SortedWord> &getSortedWords() const;

private:
	Common::Array<WordBucket> _buckets;
	Common::Array<SortedWord> _sortedWords;
};

} // End of namespace Obsidian

} // End of namespace MTropolis

#endif
