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

#ifndef MOHAWK_ZOOMBINI_RESOURCE_H
#define MOHAWK_ZOOMBINI_RESOURCE_H

#include "common/hashmap.h"
#include "common/scummsys.h"

namespace Common {

class SeekableReadStream;

} // End of namespace Common

namespace Mohawk {

constexpr const char *ZMB_MHK_ROOT_EUR_GB = "ENGLISH/";
constexpr const char *ZMB_MHK_ROOT_EUR_FR = "FRENCH/";
constexpr const char *ZMB_MHK_ROOT_EUR_GE = "GERMAN/";
constexpr const char *ZMB_MHK_ROOT_EUR_IT = "ITALIAN/";
constexpr const char *ZMB_MHK_ROOT_GENERIC = "DATA/";

constexpr const char *ZMB_VIDEO_CDTOONS = "LOGO025.MOV";    // Broderbund v1.x releases only
constexpr const char *ZMB_VIDEO_BINK = "LOGO025.BIK";       // The Learning Company v2.0 release only
constexpr const char *ZMB_VIDEO_BINK_DEMO = "LOGODEMO.BIK"; // The Learning Company v2.0 demo only

constexpr const char *ZMB_MHK_ZOOMBINI = "ZOOMBINI.MHK";
// v1.x hybrid Mac/PC discs ship two separately authored MIDI profiles.
// Both archives provide the same tMID resource IDs:
// MIDIMPC.MHK - Windows/MPC profile (SMF type 0, merged, with inline GM/GS setup), used by the Windows release.
// MIDIMAC.MHK - Macintosh profile (mostly SMF type 1 multi-track,
// authored for QuickTime / Roland SC-55 with no inline setup),
// used by the Macintosh release.
// ScummVM can optionally play the Mac-authored soundtrack through @ref ZoombiniPage::openMidiArchive().
constexpr const char *ZMB_MHK_MIDIMPC = "MIDIMPC.MHK"; // Broderbund v1.x releases only
constexpr const char *ZMB_MHK_MIDIMAC = "MIDIMAC.MHK"; // Broderbund v1.x releases only
constexpr const char *ZMB_MHK_TEXTSTR = "TEXTSTR.MHK"; // Broderbund v1.0 Europe releases only
constexpr const char *ZMB_MHK_MUSIC = "MUSIC.MHK";     // The Learning Company v2.0 Picker background music only
constexpr const char *ZMB_MHK_HELP = "HELP.MHK";       // The Learning Company v2.0 release only
constexpr const char *ZMB_MHK_XFER = "XFER.MHK";
constexpr const char *ZMB_MHK_MAP = "MAP.MHK";
constexpr const char *ZMB_MHK_RODMAP = "RODMAP.MHK";
constexpr const char *ZMB_MHK_PICKER = "PICKER.MHK";
constexpr const char *ZMB_MHK_TOWN = "TOWN.MHK";
constexpr const char *ZMB_MHK_BASECAMP = "BASECAMP.MHK";
constexpr const char *ZMB_MHK_BCTWO = "BCTWO.MHK";
constexpr const char *ZMB_MHK_BRIDGE = "BRIDGE.MHK";
constexpr const char *ZMB_MHK_TUNNELS = "TUNNELS.MHK";
constexpr const char *ZMB_MHK_PIZZA = "PIZZA.MHK";
constexpr const char *ZMB_MHK_FERRY = "FERRY.MHK";
constexpr const char *ZMB_MHK_LILLY = "LILLY.MHK";
constexpr const char *ZMB_MHK_SLIDES = "SLIDES.MHK";
constexpr const char *ZMB_MHK_FLEENS = "FLEENS.MHK";
constexpr const char *ZMB_MHK_HOTEL = "HOTEL.MHK";
constexpr const char *ZMB_MHK_NET = "NET.MHK";
constexpr const char *ZMB_MHK_NET_DEMO = "NETDEMO.MHK";
constexpr const char *ZMB_MHK_CAVES = "CAVES.MHK";
constexpr const char *ZMB_MHK_SMOKE = "SMOKE.MHK";
constexpr const char *ZMB_MHK_MAZE2 = "MAZE2.MHK";

struct ZmbResource {
	/**
	 * In Zoombinis, Common archive and page archive cannot share a unified resource id namespace
	 * due to a few resource id conflicts, unlike other Mohawk games.
	 * Ex) XFER.MHK tBMP 3000 (Background) vs ZOOMBINI.MHK tBMP 3000 (Shapes)
	 * So let's separate each namespace.
	 */
	enum ArchiveKind : uint16 {
		/** Resource from the active archives in @ref MohawkEngine::_mhks. */
		kPage = 0,
		/** Resources from ZOOMBINI.MHK, etc. */
		kSystem = 1,
	};

	/**
	 * Explicit address of a resource in one of the Zoombini archive namespaces.
	 *
	 * The archive kind is part of the address because page archives and the system
	 * archive can contain the same numeric resource ID with different content.
	 * @ref ZmbResource::kPage addresses the active page archive stack, while
	 * @ref ZmbResource::kSystem addresses ZOOMBINI.MHK and the supported system
	 * fallback archives.
	 *
	 * A default-constructed value has no resource ID. SCRB and SCRS sound IDs are
	 * converted to this explicit form while the script is parsed, so callers do
	 * not have to repeat the archive-selection rule at each playback site.
	 */
	static constexpr int16 RESOURCE_NONE = 0;

	/** Archive namespace containing the addressed resource. */
	ArchiveKind _archiveKind = kPage;
	/** Signed 16-bit numeric ID within @ref _archiveKind, or @ref RESOURCE_NONE. */
	int16 _id = RESOURCE_NONE;

	/** Construct an invalid resource address. */
	ZmbResource() = default;
	/** Construct an explicit archive-qualified resource address. */
	ZmbResource(ArchiveKind archiveKind, int16 id) : _archiveKind(archiveKind), _id(id) {}

	/** Return whether @p byteCount bytes remain at the current stream position. */
	static bool hasBytes(Common::SeekableReadStream *stream, int64 byteCount);
	/** Return whether the complete stream length is within the inclusive range. */
	static bool hasSize(Common::SeekableReadStream *stream, int64 minimumSize, int64 maximumSize);

	/** Parse a decimal resource ID without changing the archive kind. */
	static bool parseInt(const char *str, int32 &result);
	/** Parse an archive-qualified resource string into @p outRes. */
	static bool parse(const char *str, ZmbResource &outRes);

	/** Compare both archive kind and numeric ID. */
	bool operator==(const ZmbResource &other) const;
	/** Compare both archive kind and numeric ID for inequality. */
	bool operator!=(const ZmbResource &other) const;
	/** Return whether this address contains a resource ID. */
	bool hasId() const;
	/** Format the archive-qualified address for diagnostics. */
	Common::String toString() const;
};

enum class ZoombiniPageCategory : uint16 {
	kNone = 0,
	/**
	 * Normal rest page or puzzle page, usually has three buttons.
	 * Clears loaded page archives when closed.
	 * Ex) Shelter pages like PICKER, BASECAMP1, BASECAMP2, TOWN
	 * Ex) Puzzle pages like BRIDGE, TUNNELS, PIZZA, FERRY, LILLY, SLIDES, FLEENS, HOTEL, NET, CAVES, SMOKE, MAZE
	 */
	kInteractive = 1,
	/**
	 * Transition pages between puzzles, a simple click or keypress finishes the page.
	 * Clears loaded page archives when closed.
	 * Ex) XFER, LOGO, etc.
	 */
	kTransition = 2,
	/**
	 * Modal dialog pages.
	 * Does not clear loaded page archives when closed.
	 */
	kDialog = 3,
};

enum class ZoombiniPageType : uint16 {
	kNone = 0,
	/** Map */
	kRodMap = 1,
	/** Intersection */
	kXfer = 2,
	/** Zoombini Isle */
	kPicker = 3,
	/** Shelter Rock */
	kBasecamp1 = 4,
	/** Shade Tree */
	kBasecamp2 = 5,
	/** Zoombiniville */
	kTown = 6,
	// Route1: The Big, the Bad, and the Hungry
	/** Allergic Cliffs */
	kBridge = 7,
	/**
	 * Stone Cold Caves.
	 * Archive filename: TUNNELS.MHK.
	 */
	kTunnels = 8,
	/** Pizza Pass */
	kPizza = 9,
	// Route2: Who's Bayou
	/** Captain Cajun's Ferryboat */
	kFerry = 10,
	/** Titanic Tattooed Toads */
	kLilly = 11,
	/** Stone Rise */
	kSlides = 12,
	// Route3: The Deep, Dark Forest
	/** Fleens! */
	kFleens = 13,
	/** Hotel Dimensia */
	kHotel = 14,
	/** Mudball Wall */
	kNet = 15,
	// Route4: Mountains of Despair
	/**
	 * The Lion's Lair.
	 * Archive filename: CAVES.MHK.
	 */
	kCaves = 16,
	/** Mirror Machine */
	kSmoke = 17,
	/** Bubblewonder Abyss */
	kMaze = 18,
	/** Virtual page for a logo movie */
	kLogo = 19,
	/** Virtual page for a option dialog */
	kDialogOptions = 20,
	/** Virtual page for a save/load dialog */
	kDialogSaveLoad = 21,
	/** Virtual page for a message box dialog */
	kDialogMsgBox = 22,
	/** Virtual page for a credits dialog */
	kCreditScreen = 23,
	/** Virtual page for a help dialog */
	kDialogHelp = 24,
	/** Virtual page for a console debug screen */
	kDialogDebug = 26,
};

enum class ZoombiniDialogType : uint16 {
	kNone = 0,
	kOptions = 1,
	kLoad = 2,
	kSave = 3,
	kAsk = 4,
	kHelp = 5,
	kCredits = 6,
};

enum class ZoombiniFontUsage : uint32 {
	kFontDebugTitle = 0,
	kFontDebugText,
	kFontTitle,
	kFontText,
};

enum class ZmbEventHandleResult : uint32 {
	kPassthrough = 0,
	kConsumed,
};

enum class ZmbRenderResult : uint32 {
	/** Render was a success, postRender hook will be called. */
	kRendered = 0,
	/** Render was skipped, postRender hook will NOT be called. */
	kSkipped = 1,
};

enum class ZoombiniFrameResult : uint32 {
	kSuccess = 0,
	kRedraw,
};

enum class ZoombiniMsgBoxType : uint32 {
	kNone = 0,
	kAlertNoSavedGame,
	kAskCreateAndSaveNewGame,
	kAskCreateNewGame,
	kAskReplaceSave,
	/** ScummVM addition: Save delete feature */
	kAskRemoveSave,
	kAskSaveCurrentGame,
	kAskSaveBeforeQuit,
	kAlertCannotSaveInPractice,
	kAlertCannotSaveMoreGames,
	kAlertCannotLoadInPractice,
	kAlertCannotCreateNewInPractice,
	kAskReallyQuit,
	kAskSaveDirtyGame,
	kAskGoMapWillLost,
};

enum class ZoombiniDialogResult : uint32 {
	kNone = 0,
	kYes = 1,
	kNo = 2,
};

/** Direction for post-arrival turn-around animations. */
enum class ArrivalTurnDirection : int8 {
	kLeft = -1,
	kIdle = 0,
	kRight = 1,
};

} // End of namespace Mohawk

namespace Common {

/**
 * Hash function for Mohawk::ZoombiniPageType.
 * @tparam Mohawk::ZoombiniPageType
 * @return Hash value.
 */
template<>
struct Hash<Mohawk::ZoombiniPageType> {
	uint operator()(const Mohawk::ZoombiniPageType k) const {
		return static_cast<uint>(k);
	}
};

} // End of namespace Common

#endif // MOHAWK_ZOOMBINI_H
