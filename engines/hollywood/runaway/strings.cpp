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

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/util.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const char *const kTextKeys[] = {
	"interface.look", "interface.take", "interface.use",
	"interface.putAway", "interface.with", "interface.inventory",
	"interface.resume", "interface.open", "interface.close",
	"interface.move", "interface.lookThrough", "interface.usePhone",
	"interface.takeFromShelf", "interface.unplugCooler", "interface.shirt",
	"interface.wetCloth", "interface.dirtyCloth", "interface.options",
	"interface.load", "interface.save", "interface.delete",
	"interface.play", "interface.quit", "interface.music",
	"interface.antialias", "interface.scrolling", "interface.pointerSound",
	"interface.subtitles", "interface.on", "interface.off",
	"interface.fast", "interface.slow", "interface.volume",
	"interface.brightness", "interface.credits", "interface.saveSettings",
	"interface.defaultSettings", "interface.confirmQuit", "interface.yes",
	"interface.no", "interface.quitKey", "interface.leaveHatch",
	"interface.enterSaveName", "interface.noSaves", "interface.newSave",
	"interface.gameSaved", "interface.settingsSaved", "interface.defaultsRestored",
	"interface.saveFailed", "interface.loadFailed", "interface.deleteFailed",
	"interface.noFreeSlots", "interface.deleteKey", "interface.creditsFailed",
	"interface.playBeginning", "interface.playChapter", "interface.dryMarkerPen",
	"interface.syringeWithAlcohol", "interface.fakeMedicalChart", "interface.headWithWig",
	"interface.glassWithWater", "interface.boxOfSyringes", "interface.medicalChart",
	"interface.fakeChart", "interface.museumDemoCondom", "interface.frozenBattery",
	"interface.mayanPryingTool", "interface.groundCoffeeBowl", "interface.emptyBowl",
	"interface.ladleWithBattery", "interface.anthropologyBooks", "interface.documents",
	"interface.alteredLaser", "interface.varnishBriefcase", "interface.willy",
	"interface.mayanBowlsAndJars", "interface.exhibitBucket", "interface.teapot",
	"interface.exhibitBowl", "interface.thermalChamber", "interface.key",
	"interface.nitrogenTank", "interface.electronMicroscope", "interface.makeshiftBullet",
	"interface.pileOfMakeshiftBullets", "interface.mendedStrap", "interface.bottleWithWater",
	"interface.bottleWithGasoline", "interface.filledMeasuringTube", "interface.polishedStone",
	"interface.stoneOfLight", "interface.makeshiftMortar", "interface.steamOutletLever",
	"interface.pressureWheel", "interface.makeshiftMiningTool", "interface.hotScalpel",
	"interface.openPods"
};
static_assert(ARRAYSIZE(kTextKeys) == RunawayStrings::kTextCount, "Runaway text keys must match TextId");

} // End of anonymous namespace

bool RunawayStrings::load() {
	_text.clear();
	Common::ScopedPtr<Common::Archive> archive(Common::makeZipArchive("hollywood.dat"));
	if (!archive || !loadCatalog(*archive, "en")) {
		return false;
	}
	const char *language = nullptr;
	switch (_language) {
	case Common::DE_DEU:
		language = "de";
		break;
	case Common::FR_FRA:
		language = "fr";
		break;
	case Common::ES_ESP:
		language = "es";
		break;
	default:
		break;
	}
	if (language && !loadCatalog(*archive, language)) {
		return false;
	}
	for (uint i = 0; i < ARRAYSIZE(kTextKeys); ++i) {
		if (!_text.contains(kTextKeys[i])) {
			return false;
		}
	}
	return true;
}

bool RunawayStrings::loadCatalog(const Common::Archive &archive, const char *language) {
	const Common::Path path(Common::String::format("runaway/%s.dat", language));
	Common::ScopedPtr<Common::SeekableReadStream> stream(archive.createReadStreamForMember(path));
	if (!stream || stream->readUint32BE() != MKTAG('R', 'W', 'T', 'X') || stream->readUint16LE() != 2) {
		return false;
	}
	const uint16 count = stream->readUint16LE();
	if (!count) {
		return false;
	}
	for (uint i = 0; i < count; ++i) {
		const uint16 keySize = stream->readUint16LE();
		const uint16 textSize = stream->readUint16LE();
		if (!keySize || keySize + textSize > stream->size() - stream->pos()) {
			return false;
		}
		const Common::String key = stream->readString(0, keySize);
		const Common::String text = stream->readString(0, textSize);
		if (stream->err() || key.size() != keySize || text.size() != textSize) {
			return false;
		}
		_text[key] = Common::U32String(text);
	}
	return !stream->err() && stream->pos() == stream->size();
}

Common::U32String RunawayStrings::lookup(const Common::String &key) const {
	Common::HashMap<Common::String, Common::U32String>::const_iterator entry = _text.find(key);
	return entry != _text.end() ? entry->_value : Common::U32String();
}

Common::U32String RunawayStrings::text(TextId id) const {
	assert(id < kTextCount);
	return lookup(kTextKeys[id]);
}

Common::U32String RunawayStrings::itemName(byte chapter, byte item) const {
	return lookup(Common::String::format("item.%u.%u", chapter, item));
}

Common::U32String RunawayStrings::hotspotName(uint16 scene, byte hotspot) const {
	return lookup(Common::String::format("hotspot.%u.%u", scene, hotspot));
}

Common::U32String RunawayStrings::sceneActionCaption(uint16 scene, byte hotspot, byte item, bool look, const char *variant) const {
	const Common::String key = item ? Common::String::format("action.%u.item.%u.%u", scene, item, hotspot) :
		Common::String::format("action.%u.%s.%u", scene, look ? "look" : "use", hotspot);
	if (variant) {
		const Common::U32String caption = lookup(key + "." + variant);
		if (!caption.empty()) {
			return caption;
		}
	}
	Common::U32String caption = lookup(key);
	if (caption.empty() && item) {
		caption = lookup(Common::String::format("action.%u.item.0.%u", scene, hotspot));
	}
	return caption;
}

Common::U32String RunawayStrings::combinationCaption(byte chapter, byte item, byte target) const {
	Common::U32String caption = lookup(Common::String::format("combination.%u.%u.%u", chapter, item, target));
	if (caption.empty()) {
		caption = lookup(Common::String::format("combination.%u.%u.0", chapter, item));
	}
	if (caption.empty()) {
		caption = lookup(Common::String::format("combination.%u.0.%u", chapter, target));
	}
	return caption;
}

Common::U32String RunawayStrings::formatActionCaption(const Common::U32String &pattern, const Common::U32String &item, const Common::U32String &target) {
	Common::U32String caption;
	for (uint i = 0; i < pattern.size(); ++i) {
		if (pattern[i] == '#') {
			caption += item;
		} else if (pattern[i] == '@') {
			caption += target;
		} else {
			caption += pattern[i];
		}
	}
	return caption;
}

Common::U32String RunawayStrings::shackHotspotName(byte hotspot) const {
	return hotspotName(5030, hotspot);
}

} // End of namespace Hollywood
