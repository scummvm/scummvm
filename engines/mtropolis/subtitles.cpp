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

#include "mtropolis/subtitles.h"

#include "common/archive.h"
#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "common/hash-str.h"

#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"

#include "mtropolis/render.h"
#include "mtropolis/runtime.h"

namespace MTropolis {

class SubtitleCSVLoader {
public:
	explicit SubtitleCSVLoader(Common::ReadStream *stream);

	bool readLine(Common::Array<Common::String> &outStrings);

private:
	bool readQuotedCel(Common::String &str);
	bool readUnquotedCel(Common::String &str);

	Common::Array<char> _contents;
	uint _readOffset;
	uint _line;
};

SubtitleCSVLoader::SubtitleCSVLoader(Common::ReadStream *stream) : _readOffset(0), _line(1) {
	char chunk[4096];
	while (!stream->eos() && !stream->err()) {
		uint32 countRead = stream->read(chunk, sizeof(chunk));
		if (countRead == 0)
			return;

		_contents.resize(_contents.size() + countRead);
		memcpy(&_contents[_contents.size() - countRead], chunk, countRead);
	}
}

bool SubtitleCSVLoader::readLine(Common::Array<Common::String> &outStrings) {
	uint numStrs = 0;
	while (_readOffset < _contents.size()) {
		if (outStrings.size() == numStrs)
			outStrings.push_back(Common::String());

		Common::String &celStr = outStrings[numStrs];

		char nextChar = _contents[_readOffset];
		bool celStatus = false;
		if (nextChar == '\"')
			celStatus = readQuotedCel(celStr);
		else
			celStatus = readUnquotedCel(celStr);

		if (!celStatus)
			return false;

		numStrs++;

		if (_readOffset == _contents.size())
			break;

		char dividerChar = _contents[_readOffset];
		if (dividerChar == ',')
			_readOffset++;
		else if (dividerChar == '\r' || dividerChar == '\n') {
			_readOffset++;
			if (dividerChar == '\r' && _readOffset < _contents.size() && _contents[_readOffset] == '\n')
				_readOffset++;
			break;
		} else {
			return false;
		}
	}

	outStrings.resize(numStrs);
	_line++;
	return true;
}

bool SubtitleCSVLoader::readQuotedCel(Common::String &str) {
	assert(_contents[_readOffset] == '\"');
	_readOffset++;

	str.clear();
	for (;;) {
		if (_readOffset == _contents.size())
			return false;

		char nextChar = _contents[_readOffset];
		if (nextChar == '\"') {
			_readOffset++;
			if (_readOffset < _contents.size()) {
				char subsequentChar = _contents[_readOffset];
				if (subsequentChar == '\"') {
					str += '\"';
					_readOffset++;
				} else
					break;
			}
		} else {
			str += nextChar;
			_readOffset++;
		}
	}

	return true;
}

bool SubtitleCSVLoader::readUnquotedCel(Common::String &str) {
	assert(_contents[_readOffset] != '\"');

	str.clear();
	for (;;) {
		if (_readOffset == _contents.size())
			return true;

		char nextChar = _contents[_readOffset];
		if (nextChar == ',' || nextChar == '\n' || nextChar == '\r')
			break;

		str += nextChar;
		_readOffset++;
	}

	return true;
}

SubtitleAssetMappingTable::SubtitleAssetMappingTable() {
}

Common::ErrorCode SubtitleAssetMappingTable::load(const Common::String &filePath) {
	Common::File f;
	if (!f.open(Common::Path(filePath)))
		return Common::kPathDoesNotExist;

	SubtitleCSVLoader loader(&f);

	Common::Array<Common::String> strs;
	if (!loader.readLine(strs))
		return Common::kReadingFailed;

	if (strs.size() != 3 || strs[0] != "subtitle_set_id" || strs[1] != "asset_id" || strs[2] != "asset_name")
		return Common::kReadingFailed;

	while (loader.readLine(strs)) {
		if (strs.size() == 0)
			break;

		if (strs.size() != 3)
			return Common::kReadingFailed;

		uint assetID = 0;
		if (sscanf(strs[1].c_str(), "%u", &assetID) == 1 && assetID != 0)
			_assetIDToSubtitleSet[assetID] = strs[0];

		if (strs[2].size() > 0)
			_assetNameToSubtitleSet[strs[2]] = strs[0];
	}

	return Common::kNoError;
}

const Common::String *SubtitleAssetMappingTable::findSubtitleSetForAssetID(uint32 assetID) const {
	Common::HashMap<uint32, Common::String>::const_iterator it = _assetIDToSubtitleSet.find(assetID);
	if (it == _assetIDToSubtitleSet.end())
		return nullptr;
	return &it->_value;
}

const Common::String *SubtitleAssetMappingTable::findSubtitleSetForAssetName(const Common::String &assetName) const {
	Common::HashMap<Common::String, Common::String>::const_iterator it = _assetNameToSubtitleSet.find(assetName);
	if (it == _assetNameToSubtitleSet.end())
		return nullptr;
	return &it->_value;
}


SubtitleModifierMappingTable::SubtitleModifierMappingTable() {
}

Common::ErrorCode SubtitleModifierMappingTable::load(const Common::String &filePath) {
	Common::File f;
	if (!f.open(Common::Path(filePath)))
		return Common::kPathDoesNotExist;

	SubtitleCSVLoader loader(&f);

	Common::Array<Common::String> strs;
	if (!loader.readLine(strs))
		return Common::kReadingFailed;

	if (strs.size() != 2 || strs[0] != "subtitle_set_id" || strs[1] != "modifier_guid")
		return Common::kReadingFailed;

	while (loader.readLine(strs)) {
		if (strs.size() == 0)
			break;

		if (strs.size() != 2)
			return Common::kReadingFailed;

		uint modifierGUID = 0;
		for (char c : strs[1]) {
			modifierGUID *= 0x10;
			if (c >= '0' && c <= '9') {
				modifierGUID += (c - '0');
			} else if (c >= 'a' && c <= 'f') {
				modifierGUID += (c - 'a' + 0xa);
			} else if (c >= 'A' && c <= 'F') {
				modifierGUID += (c - 'A' + 0xa);
			} else {
				return Common::kReadingFailed;
			}
		}

		_guidToSubtitleSet[modifierGUID] = strs[0];
	}

	return Common::kNoError;
}

const Common::String *SubtitleModifierMappingTable::findSubtitleSetForModifierGUID(uint32 guid) const {
	Common::HashMap<uint32, Common::String>::const_iterator it = _guidToSubtitleSet.find(guid);
	if (it == _guidToSubtitleSet.end())
		return nullptr;
	return &it->_value;
}

SubtitleSpeakerTable::SubtitleSpeakerTable() {
}

Common::ErrorCode SubtitleSpeakerTable::load(const Common::String &filePath) {
	Common::File f;
	if (!f.open(Common::Path(filePath)))
		return Common::kPathDoesNotExist;

	SubtitleCSVLoader loader(&f);

	Common::Array<Common::String> strs;
	if (!loader.readLine(strs))
		return Common::kReadingFailed;

	if (strs.size() != 2 || strs[0] != "speaker" || strs[1] != "text")
		return Common::kReadingFailed;

	_speakers.resize(1);

	while (loader.readLine(strs)) {
		if (strs.size() == 0)
			break;

		if (strs.size() != 2)
			return Common::kReadingFailed;

		_speakerToID[strs[0]] = _speakers.size();
		_speakers.push_back(strs[1]);
	}

	return Common::kNoError;
}

const Common::Array<Common::String> &SubtitleSpeakerTable::getSpeakers() const {
	return _speakers;
}

uint SubtitleSpeakerTable::getSpeakerID(const Common::String &speakerName) const {
	Common::HashMap<Common::String, uint>::const_iterator it = _speakerToID.find(speakerName);
	if (it == _speakerToID.end())
		return 0;
	return it->_value;
}


SubtitleLineTable::SubtitleLineTable() {
}

Common::ErrorCode SubtitleLineTable::load(const Common::String &filePath, const SubtitleSpeakerTable &speakerTable) {
	Common::File f;
	if (!f.open(Common::Path(filePath)))
		return Common::kPathDoesNotExist;

	SubtitleCSVLoader loader(&f);

	Common::Array<Common::String> strs;
	if (!loader.readLine(strs))
		return Common::kReadingFailed;

	if (strs.size() != 8 || strs[0] != "subtitle_set_id" || strs[1] != "text" || strs[2] != "time" || strs[3] != "duration" || strs[4] != "slot" || strs[5] != "speaker" || strs[6] != "class" || strs[7] != "position")
		return Common::kReadingFailed;

	uint currentLine = 0;
	while (loader.readLine(strs)) {
		if (strs.size() == 0)
			break;

		if (strs.size() != 8)
			return Common::kReadingFailed;

		double timestamp = 0;
		double duration = 0;
		double position = 0;
		uint slot = 0;
		LineClass lineClass = kLineClassDefault;
		if (sscanf(strs[2].c_str(), "%lf", &timestamp) != 1)
			timestamp = 0;

		if (sscanf(strs[3].c_str(), "%lf", &duration) != 1)
			duration = 0;

		if (sscanf(strs[4].c_str(), "%u", &slot) != 1)
			slot = 0;

		if (strs[6] == "gameplay")
			lineClass = kLineClassGameplay;

		if (sscanf(strs[7].c_str(), "%lf", &position) != 1)
			position = 0;

		LineData lineData;
		lineData.durationMSec = static_cast<uint32>(duration * 1000.0);
		lineData.timeOffsetMSec = static_cast<uint32>(timestamp * 1000.0);
		lineData.textUTF8 = strs[1];
		lineData.slot = slot;
		lineData.speakerID = speakerTable.getSpeakerID(strs[5]);
		lineData.lineClass = lineClass;
		lineData.position = position;

		LineRange &range = _lineRanges[strs[0]];
		if (range.numLines == 0)
			range.linesStart = currentLine;
		else if (range.linesStart + range.numLines != currentLine) {
			warning("Failed to load lines table, subtitle set '%s' was not contiguous", strs[0].c_str());
			return Common::kReadingFailed;
		}

		range.numLines++;

		_lines.push_back(lineData);

		currentLine++;
	}

	return Common::kNoError;
}

SubtitleLineTable::LineRange::LineRange() : linesStart(0), numLines(0) {
}


SubtitleLineTable::LineData::LineData() : timeOffsetMSec(0), slot(0), durationMSec(0), speakerID(0), lineClass(kLineClassDefault), position(0) {
}

const Common::Array<SubtitleLineTable::LineData> &SubtitleLineTable::getAllLines() const {
	return _lines;
}

const SubtitleLineTable::LineRange *SubtitleLineTable::getLinesForSubtitleSetID(const Common::String &subtitleSetID) const {
	Common::HashMap<Common::String, LineRange>::const_iterator it = _lineRanges.find(subtitleSetID);
	if (it == _lineRanges.end())
		return nullptr;
	return &it->_value;
}

SubtitleRenderer::DisplayItem::DisplayItem() : expireTime(0) {
}

SubtitleDisplayItem::SubtitleDisplayItem(const Common::String &text, const Common::String &speaker, uint slot, double position)
	: _slot(slot), _position(position) {
	_text = text.decode(Common::kUtf8);
	_speaker = speaker.decode(Common::kUtf8);
}

const Common::U32String &SubtitleDisplayItem::getText() const {
	return _text;
}

const Common::U32String &SubtitleDisplayItem::getSpeaker() const {
	return _speaker;
}

uint SubtitleDisplayItem::getSlot() const {
	return _slot;
}

double SubtitleDisplayItem::getPosition() const {
	return _position;
}

SubtitleRenderer::SubtitleRenderer(bool enableGameplaySubtitles)
	: _nonImmediateDisappearTime(3500), _isDirty(true), _lastTime(0), _fontHeight(0), _enableGameplaySubtitles(enableGameplaySubtitles) {
#ifdef USE_FREETYPE2
	Common::File fontFile;
	const char *fontPath = "LiberationSans-Bold.ttf";

	_font.reset(Graphics::loadTTFFontFromArchive(fontPath, 14, Graphics::kTTFSizeModeCell));

	if (_font) {
		_fontHeight = _font->getFontHeight();
	} else
		warning("Couldn't open '%s', subtitles will not work", fontPath);

	// TODO: Maybe support subtitles some other way if FreeType isn't enabled
#endif
}

void SubtitleRenderer::addDisplayItem(const Common::SharedPtr<SubtitleDisplayItem> &item, uint duration) {
	assert(item.get() != nullptr);

	_isDirty = true;

	for (DisplayItem &existingItem : _displayItems) {
		if (existingItem.item->getSlot() == item->getSlot()) {
			existingItem.item = item;
			existingItem.surface.reset();
			if (duration > 0)
				existingItem.expireTime = _lastTime + duration;
			else
				existingItem.expireTime = 0;
			return;
		}
	}

	DisplayItem newItem;
	newItem.expireTime = 0;
	newItem.item = item;

	if (duration > 0)
		newItem.expireTime = _lastTime + duration;

	_displayItems.push_back(newItem);
}

void SubtitleRenderer::removeDisplayItem(const SubtitleDisplayItem *item, bool immediately) {
	if (item == nullptr)
		return;

	for (uint i = 0; i < _displayItems.size(); i++) {
		DisplayItem &existingItem = _displayItems[i];
		if (existingItem.item.get() == item) {
			if (immediately) {
				_displayItems.remove_at(i);
				_isDirty = true;
			} else {
				if (existingItem.expireTime == 0)
					existingItem.expireTime = _lastTime + _nonImmediateDisappearTime;
			}

			return;
		}
	}
}

bool SubtitleRenderer::update(uint64 currentTime) {
	_lastTime = currentTime;

	for (uint ridx = _displayItems.size(); ridx > 0; ridx--) {
		uint i = ridx - 1;
		DisplayItem &item = _displayItems[i];
		if (item.expireTime != 0 && item.expireTime <= currentTime) {
			_displayItems.remove_at(i);
			_isDirty = true;
		}
	}

	if (_isDirty) {
		render();
		_isDirty = false;
		return true;
	}

	return false;
}

void SubtitleRenderer::composite(Window &window) const {
	for (const DisplayItem& displayItem : _displayItems) {
		if (displayItem.surface) {
			Graphics::ManagedSurface *windowSurf = window.getSurface().get();
			if (windowSurf) {
				int x = (windowSurf->w - displayItem.surface->w) / 2;
				int y = (windowSurf->h + 300) / 2 - displayItem.surface->h + static_cast<int>(_fontHeight * displayItem.item->getPosition());
				windowSurf->blitFrom(*displayItem.surface, Common::Point(x, y));
			}
		}
	}
}

bool SubtitleRenderer::isDirty() const {
	return _isDirty;
}

bool SubtitleRenderer::isGameplaySubtitlesEnabled() const {
	return _enableGameplaySubtitles;
}


void SubtitleRenderer::render() {
	if (!_font)
		return;

	for (uint ri = _displayItems.size(); ri > 0; ri--) {
		uint i = ri - 1;

		DisplayItem &item = _displayItems[i];
		if (!item.surface) {
			uint widestLine = 0;

			const SubtitleDisplayItem &dispItem = *item.item;
			Common::Array<Common::U32String> lines;

			splitLines(dispItem.getText(), lines);

			for (const Common::U32String &str : lines) {
				uint width = _font->getStringWidth(str);
				if (width > widestLine)
					widestLine = width;
			}

			uint itemLines = lines.size();

			const Common::U32String &speaker = item.item->getSpeaker();

			if (speaker.size() > 0) {
				itemLines++;
				uint speakerWidth = _font->getStringWidth(speaker);
				if (speakerWidth > widestLine)
					widestLine = speakerWidth;
			}

			if (itemLines == 0 || widestLine == 0) {
				// Nothing to render, remove the item
				_displayItems.remove_at(i);
				_isDirty = true;
				continue;
			}

			const int borderWidth = 1;
			const int verticalPadding = 5;
			const int horizontalPadding = 20;

			int surfaceWidth = static_cast<int>(widestLine) + borderWidth * 2 + horizontalPadding * 2;
			int surfaceHeight = static_cast<int>(itemLines) * _fontHeight + borderWidth * 2 + verticalPadding * 2;

			Graphics::PixelFormat fmt = Graphics::createPixelFormat<8888>();
			Common::SharedPtr<Graphics::ManagedSurface> surface(new Graphics::ManagedSurface(surfaceWidth, surfaceHeight, fmt));

			surface->fillRect(Common::Rect(0, 0, surfaceWidth, surfaceHeight), fmt.RGBToColor(0, 0, 0));

			for (int drawPass = 0; drawPass < 2; drawPass++) {
				int textStartLine = 0;
				if (speaker.size() > 0) {
					textStartLine++;

					int speakerWidth = _font->getStringWidth(speaker);

					int startX = (surfaceWidth - speakerWidth) / 2;

					uint32 drawColor = 0;
					if (drawPass == 0)
						drawColor = fmt.RGBToColor(255, 0, 0);
					else
						drawColor = fmt.RGBToColor(255, 255, 127);

					_font->drawString(surface.get(), speaker, startX, verticalPadding + borderWidth, speakerWidth, drawColor);
				}

				for (uint lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
					const Common::U32String &line = lines[lineIndex];

					int lineWidth = _font->getStringWidth(line);

					int startX = (surfaceWidth - lineWidth) / 2;

					uint32 drawColor = 0;
					if (drawPass == 0)
						drawColor = fmt.RGBToColor(255, 0, 0);
					else
						drawColor = fmt.RGBToColor(255, 255, 255);

					_font->drawString(surface.get(), line, startX, (textStartLine + static_cast<int>(lineIndex)) * _fontHeight + verticalPadding + borderWidth, lineWidth, drawColor);
				}

				if (drawPass == 0) {
					int w = surface->w;
					int h = surface->h;

					// Horizontal pass (max r -> g)
					for (int y = borderWidth; y < h - borderWidth; y++) {
						uint32 *rowPixels = static_cast<uint32 *>(surface->getBasePtr(0, y));

						for (int x = borderWidth; x < w - borderWidth; x++) {
							uint8 r, g, b, a;
							uint8 brightest = 0;

							for (int kxo = -borderWidth; kxo < borderWidth; kxo++) {
								fmt.colorToARGB(rowPixels[x + kxo], a, r, g, b);

								if (r > brightest)
									brightest = r;
							}

							fmt.colorToARGB(rowPixels[x], a, r, g, b);
							rowPixels[x] = fmt.ARGBToColor(a, r, brightest, b);
						}
					}

					// Vertical pass (max g -> b)
					int pitch = surface->pitch;
					for (int x = borderWidth; x < w - borderWidth; x++) {
						char *basePixelPtr = static_cast<char *>(surface->getBasePtr(x, 0));

						for (int y = borderWidth; y < h - borderWidth; y++) {
							uint8 r, g, b, a;
							uint8 brightest = 0;

							for (int kyo = -borderWidth; kyo < borderWidth; kyo++) {
								uint32 *offsetPxPtr = reinterpret_cast<uint32 *>(basePixelPtr + (y + kyo) * pitch);
								fmt.colorToARGB(*offsetPxPtr, a, r, g, b);

								if (g > brightest)
									brightest = g;
							}

							uint32 *pxPtr = reinterpret_cast<uint32 *>(basePixelPtr + y * pitch);
							fmt.colorToARGB(*pxPtr, a, r, g, b);
							*pxPtr = fmt.ARGBToColor(a, r, g, brightest);
						}
					}

					// Shadow backdrop pass
					for (int y = 0; y < h; y++) {
						uint32 *rowPixels = static_cast<uint32 *>(surface->getBasePtr(0, y));

						for (int x = 0; x < w; x++) {
							uint8 a, r, g, b;
							fmt.colorToARGB(rowPixels[x], a, r, g, b);

							uint8 minGrayAlpha = 224;
							uint8 grayAlpha = (((256 - minGrayAlpha) * b) >> 8) + minGrayAlpha;

							rowPixels[x] = fmt.ARGBToColor(grayAlpha, 0, 0, 0);
						}
					}
				}
			}

			// Done drawing
			item.surface = surface;
		}
	}
}

void SubtitleRenderer::splitLines(const Common::U32String &str, Common::Array<Common::U32String> &outLines) {
	uint32 splitScanStart = 0;
	while (splitScanStart < str.size()) {
		size_t splitLoc = str.find('\\', splitScanStart);
		if (splitLoc == Common::U32String::npos)
			break;

		outLines.push_back(str.substr(splitScanStart, splitLoc - splitScanStart));
		splitScanStart = static_cast<uint32>(splitLoc + 1);
	}

	outLines.push_back(str.substr(splitScanStart));
}


SubtitlePlayer::SubtitlePlayer(Runtime *runtime, const Common::String &subtitleSetID, const SubtitleTables &tables) : _runtime(runtime) {
	const SubtitleLineTable::LineRange *lineRange = tables.lines->getLinesForSubtitleSetID(subtitleSetID);
	if (lineRange) {
		_lineRange = *lineRange;
	} else {
		warning("Subtitle set '%s' was defined, but no lines were defined", subtitleSetID.c_str());
		return;
	}

	_speakers = tables.speakers;
	_lines = tables.lines;
}

SubtitlePlayer::~SubtitlePlayer() {
	stop();
}

void SubtitlePlayer::update(uint64 prevTime, uint64 newTime) {
	if (!_lineRange.numLines)
		return;

	const Common::Array<SubtitleLineTable::LineData> &allLines = _lines->getAllLines();

	for (uint i = 0; i < _lineRange.numLines; i++) {
		const SubtitleLineTable::LineData &line = allLines[_lineRange.linesStart + i];
		if (line.timeOffsetMSec >= prevTime && line.timeOffsetMSec < newTime)
			triggerSubtitleLine(line);
	}
}

void SubtitlePlayer::stop() {
	SubtitleRenderer *renderer = _runtime->getSubtitleRenderer().get();
	if (renderer) {
		for (const Common::SharedPtr<SubtitleDisplayItem> &item : _items)
			renderer->removeDisplayItem(item.get(), false);
	}
	_items.clear();
}

void SubtitlePlayer::triggerSubtitleLine(const SubtitleLineTable::LineData &line) {
	SubtitleRenderer *renderer = _runtime->getSubtitleRenderer().get();
	if (renderer) {
		if (line.lineClass == SubtitleLineTable::kLineClassGameplay && !renderer->isGameplaySubtitlesEnabled())
			return;

		Common::SharedPtr<SubtitleDisplayItem> dispItem(new SubtitleDisplayItem(line.textUTF8, _speakers->getSpeakers()[line.speakerID], line.slot, line.position));
		for (uint i = 0; i < _items.size(); i++) {
			if (_items[i]->getSlot() == line.slot) {
				renderer->removeDisplayItem(_items[i].get(), true);
				_items.remove_at(i);
				break;
			}
		}

		renderer->addDisplayItem(dispItem, line.durationMSec);
		_items.push_back(dispItem);
	}
}

} // End of namespace MTropolis
