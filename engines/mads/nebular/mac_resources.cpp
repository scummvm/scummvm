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

#include "common/debug.h"
#include "common/endian.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"
#include "image/iff.h"
#include "mads/core/color.h"
#include "mads/core/inter.h"
#include "mads/core/pack.h"
#include "mads/nebular/mac_resources.h"

namespace MADS {
namespace RexNebular {

static const char *const kContainerNames[] = {
	nullptr,
	"Rex Nebular",
	"Rex Global Data",
	"Rex Sound Data",
	"Rex Section I Data",
	"Rex Section II Data",
	"Rex Section III Data",
	"Rex Section IV Data",
	"Rex Section V Data",
	"Rex Section VI Data",
	"Rex Section VII Data",
	"Rex Section VIII Data",
	"Rex Section IX Data"
};

static uint32 makeRoomType(char prefix, int room) {
	return MKTAG(prefix, '0' + room / 100, '0' + (room / 10) % 10, '0' + room % 10);
}

static bool parseDecimal(const Common::String &text, int &value) {
	if (text.empty())
		return false;

	value = 0;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] < '0' || text[i] > '9')
			return false;
		value = value * 10 + text[i] - '0';
	}
	return true;
}

static uint16 suffixToID(const Common::String &suffix) {
	if (suffix.size() == 1)
		return (byte)suffix[0];
	if (suffix.size() == 2)
		return ((byte)suffix[0] << 8) | (byte)suffix[1];
	return 0;
}

// Synthetic platform resources still pass through the standard MADS loader,
// so expose them as ordinary one-record, uncompressed packet files.
static byte *createUncompressedPack(uint32 payloadSize, byte priority) {
	byte *data = (byte *)calloc(PackList::SIZE + payloadSize, 1);
	if (!data)
		return nullptr;

	memcpy(data, PACK_ID_STRING, PACK_ID_LENGTH);
	WRITE_LE_UINT16(data + PACK_ID_LENGTH, 1);
	data[PACK_HEADER] = PACK_NONE;
	data[PACK_HEADER + 1] = priority;
	WRITE_LE_UINT32(data + PACK_HEADER + 2, payloadSize);
	WRITE_LE_UINT32(data + PACK_HEADER + 6, payloadSize);
	return data;
}

struct GlobalSeriesFamily {
	const char *prefix;
	uint32 type;
};

struct RoomSeriesAlias {
	const char *filename;
	int room;
	char type;
	uint16 id;
};

static const GlobalSeriesFamily kGlobalSeriesFamilies[] = {
	{ "RXMRD_",  MKTAG('R', 'M', 'R', 'D') },
	{ "RXSW_",   MKTAG('R', 'S', 'W', '_') },
	{ "RXCD_",   MKTAG('R', 'X', 'C', 'D') },
	{ "RXCL_",   MKTAG('R', 'X', 'C', 'L') },
	{ "RXKIC_",  MKTAG('R', 'X', 'K', 'I') },
	{ "RXMBD_",  MKTAG('R', 'X', 'M', 'B') },
	{ "RXMRC_",  MKTAG('R', 'M', 'R', 'C') },
	{ "RXMSU_",  MKTAG('R', 'X', 'M', 'S') },
	{ "RXMTP_",  MKTAG('R', 'X', 'M', 'T') },
	{ "RXM_",    MKTAG('R', 'X', 'M', '_') },
	{ "RXRC_",   MKTAG('R', 'X', 'R', 'C') },
	{ "RXRD_",   MKTAG('R', 'X', 'R', 'D') },
	{ "RXSWRC_", MKTAG('R', 'S', 'W', 'R') },
	{ "RXBOT_",  MKTAG('R', 'X', 'B', 'O') },
	{ "RXTH_",   MKTAG('R', 'X', 'T', 'H') },
	{ "RXMRO_",  MKTAG('R', 'M', 'R', 'O') },
	{ "RXTY_",   MKTAG('R', 'X', 'T', 'Y') },
	{ "ROXBD_",  MKTAG('R', 'O', 'X', 'B') },
	{ "ROXCD_",  MKTAG('R', 'O', 'C', 'D') },
	{ "ROXCL_",  MKTAG('R', 'O', 'C', 'L') },
	{ "ROXHAN",  MKTAG('R', 'O', 'X', 'H') },
	{ "ROXRC_",  MKTAG('R', 'O', 'R', 'C') },
	{ "ROXRD_",  MKTAG('R', 'O', 'R', 'D') },
	{ "ROXTP_",  MKTAG('R', 'O', 'X', 'T') },
	{ "ROX_",    MKTAG('R', 'O', 'X', '_') },
	{ "GRD1_",   MKTAG('G', 'R', 'D', '1') },
	{ "GRD2_",   MKTAG('G', 'R', 'D', '2') },
	{ "GRDRC2_", MKTAG('G', 'R', 'D', 'R') }
};

// These names cannot be derived from the resource catalog alone: the Mac
// resource IDs retain shortened or renamed DOS identifiers used by the
// shared scripts.
static const RoomSeriesAlias kRoomSeriesAliases[] = {
	{ "RM101S.AA",   101, 'A', 'A' },
	{ "RM313A_2.SS", 313, 'S', MKTAG16('A', '2') },
	{ "RM313A_3.SS", 313, 'S', MKTAG16('A', '3') },
	{ "RM313A_6.SS", 313, 'S', MKTAG16('A', '6') },
	{ "RM313A_8.SS", 313, 'S', MKTAG16('A', '8') },
	{ "RM313A_9.SS", 313, 'S', MKTAG16('A', '9') },
	{ "RM604R1.AA",  604, 'A', MKTAG16('A', '1') },
	{ "RM971K0A.SS", 971, 'S', MKTAG16('K', '0') },
	{ "RM971K3A.SS", 971, 'S', MKTAG16('K', '3') },
	{ "RM971K4A.SS", 971, 'S', MKTAG16('K', '4') },
	{ "RM979.SS",    979, 'S', MKTAG16('.', 'S') },
	{ "RM980.SS",    980, 'S', MKTAG16('.', 'S') }
};

MacResourceProvider::MacResourceProvider() {
	for (uint i = kApplicationContainer; i <= kSection9Container; ++i)
		_containers[i] = new Common::MacResManager();
}

MacResourceProvider::~MacResourceProvider() {
	for (uint i = kApplicationContainer; i <= kSection9Container; ++i)
		delete _containers[i];
	delete _fontManager;
}

bool MacResourceProvider::load() {
	for (uint i = kApplicationContainer; i <= kSection9Container; ++i) {
		if (!_containers[i]->open(Common::Path(kContainerNames[i]))) {
			warning("Could not open Macintosh resource container '%s'", kContainerNames[i]);
			return false;
		}
	}

	delete _fontManager;
	_fontManager = new Graphics::MacFontManager(
		Graphics::MacGUIConstants::kWMModeForceMacFonts, Common::EN_ANY);
	return true;
}

Common::MacResManager *MacResourceProvider::getContainer(Container container) const {
	if (container < kApplicationContainer || container > kSection9Container)
		return nullptr;
	return _containers[container];
}

Common::SeekableReadStream *MacResourceProvider::openResource(
		Container container, uint32 type, uint16 id) const {
	Common::MacResManager *resourceContainer = getContainer(container);
	return resourceContainer ? resourceContainer->getResource(type, id) : nullptr;
}

const Graphics::Font *MacResourceProvider::getDialogFont() {
	if (!_fontManager)
		return nullptr;

	// The native popup code selects regular Geneva at 9 points. This is
	// distinct from the bold 10-point font adapted for the shared MADS scene
	// renderer by createFontResource().
	return _fontManager->getFont(Graphics::MacFont(
		Graphics::kMacFontGeneva, 9, Graphics::kMacFontRegular));
}

const Graphics::Font *MacResourceProvider::getInterfaceFont() {
	if (!_fontManager)
		return nullptr;

	// CODE 7 selects plain Geneva at 10 points for the native interface.
	// Use it directly instead of enlarging text from the compatibility surface.
	return _fontManager->getFont(Graphics::MacFont(
		Graphics::kMacFontGeneva, 10, Graphics::kMacFontRegular));
}

MacResourceProvider::ResourceID MacResourceProvider::mapResource(const Common::String &filename) {
	Common::String name(filename);
	name.toUppercase();
	if (name.hasPrefix("*"))
		name.deleteChar(0);

	ResourceID result;
	if (name == "VOCAB.DAT")
		return { kGlobalContainer, MKTAG('V', 'o', 'c', 'a'), 1000 };
	if (name == "OBJECTS.DAT")
		return { kGlobalContainer, MKTAG('O', 'b', 'j', 'e'), 1000 };
	if (name == "QUOTES.DAT")
		return { kGlobalContainer, MKTAG('Q', 'u', 'o', 't'), 1000 };
	if (name == "HOGANUS.DAT")
		return { kGlobalContainer, MKTAG('H', 'o', 'g', 'a'), 1000 };
	if (name == "ERRORS.DB")
		return { kGlobalContainer, MKTAG('D', 'a', 'b', 'a'), 0x4552 };
	if (name == "MODULES.DB")
		return { kGlobalContainer, MKTAG('D', 'a', 'b', 'a'), 0x4d4f };
	if (name == "REXHAND.SS")
		return { kGlobalContainer, MKTAG('R', 'e', 'x', 'h'), 1000 };
	if (name == "METHAND.SS")
		return { kGlobalContainer, MKTAG('M', 'e', 't', 'h'), 1000 };
	if (name == "BTSPIN.SS")
		return { kGlobalContainer, MKTAG('B', 't', 's', 'p'), 1000 };

	if (name.hasPrefix("ENDING") && name.hasSuffix(".TXR") && name.size() == 11 &&
			name[6] >= '0' && name[6] <= '9') {
		return { kGlobalContainer, MKTAG('E', 'n', 'd', 'i'), (uint16)name[6] };
	}
	if (name.hasPrefix("WARN") && name.hasSuffix(".DAT") && name.size() == 9 &&
			name[4] >= '0' && name[4] <= '9') {
		return { kGlobalContainer, MKTAG('W', 'a', 'r', 'n'), (uint16)name[4] };
	}

	if (name.hasPrefix("OB") && name.hasSuffix(".SS")) {
		Common::String number = name.substr(2, 3);
		int object = 0;
		if (parseDecimal(number, object) && object <= 999) {
			const bool inventory = name.size() == 9 && name[5] == 'I';
			if ((inventory && name == Common::String::format("OB%03dI.SS", object)) ||
					(!inventory && name == Common::String::format("OB%03d.SS", object))) {
				return { kGlobalContainer,
					inventory ? MKTAG('O', 'b', 's', 'i') : MKTAG('O', 'b', 's', 's'),
					(uint16)object };
			}
		}
	}

	if (name.size() == 5 && name[0] == 'I' && name.hasSuffix(".AA") &&
			name[1] >= '0' && name[1] <= '9') {
		return { kGlobalContainer, MKTAG('A', 'i', ' ', ' '),
			(uint16)(1000 + name[1]) };
	}

	if (name.hasSuffix(".SS")) {
		const Common::String base = name.substr(0, name.size() - 3);
		for (uint i = 0; i < ARRAYSIZE(kGlobalSeriesFamilies); ++i) {
			const Common::String prefix(kGlobalSeriesFamilies[i].prefix);
			if (!base.hasPrefix(prefix))
				continue;
			const Common::String suffix = base.substr(prefix.size());
			if (suffix.size() >= 1 && suffix.size() <= 2) {
				return { kGlobalContainer, kGlobalSeriesFamilies[i].type,
					suffixToID(suffix) };
			}
		}
	}

	for (uint i = 0; i < ARRAYSIZE(kRoomSeriesAliases); ++i) {
		const RoomSeriesAlias &alias = kRoomSeriesAliases[i];
		if (name == alias.filename) {
			return { (Container)(kSection1Container + alias.room / 100 - 1),
				makeRoomType(alias.type, alias.room), alias.id };
		}
	}

	if (name.hasPrefix("SC") && name.hasSuffix(".SS")) {
		int section = 0;
		const Common::String suffix = name.substr(5, name.size() - 8);
		if (parseDecimal(name.substr(2, 3), section) && section >= 1 && section <= 9 &&
				suffix.size() >= 1 && suffix.size() <= 2) {
			return { (Container)(kSection1Container + section - 1),
				makeRoomType('S', section), suffixToID(suffix) };
		}
	}

	if (!name.hasPrefix("RM") || name.size() < 8)
		return result;

	int room = 0;
	if (!parseDecimal(name.substr(2, 3), room) || room < 100 || room > 999)
		return result;

	const int section = room / 100;
	if (section < 1 || section > 9)
		return result;
	result.container = (Container)(kSection1Container + section - 1);

	const int extensionAt = name.findLastOf('.');
	if (extensionAt < 5)
		return ResourceID();
	const Common::String suffix = name.substr(5, extensionAt - 5);
	const Common::String extension = name.substr(extensionAt);

	if (suffix.empty() && extension == ".ART") {
		result.type = MKTAG('R', 'a', 'r', 't');
		result.id = room;
	} else if (suffix.empty() && extension == ".DAT") {
		result.type = MKTAG('R', 'd', 'a', 't');
		result.id = room;
	} else if (suffix.empty() && extension == ".HH") {
		result.type = MKTAG('R', 'h', 'o', 't');
		result.id = room;
	} else if (!suffix.empty() && suffix.size() <= 2 && extension == ".SS") {
		result.type = makeRoomType('S', room);
		result.id = suffixToID(suffix);
	} else if (!suffix.empty() && suffix.size() <= 2 && extension == ".AA") {
		result.type = makeRoomType('A', room);
		result.id = suffixToID(suffix);
	} else {
		return ResourceID();
	}

	return result;
}

Common::SeekableReadStream *MacResourceProvider::open(const char *filename) {
	Common::String name(filename);
	name.toUppercase();
	if (name.hasPrefix("*"))
		name.deleteChar(0);
	if (name == "FONTMAIN.FF" || name == "FONTINTR.FF" ||
			name == "FONTCONV.FF" || name == "FONTMISC.FF" ||
			name == "FONTTELE.FF") {
		return createFontResource();
	}
	if (name.size() >= 6 && name[0] == 'I' &&
			name[1] >= '0' && name[1] <= '9' &&
			(name.substr(2) == ".INT" || name.substr(2) == "A.INT")) {
		return createInterfaceResource(name[1] - '0');
	}

	const ResourceID resource = mapResource(filename);
	if (!resource.isValid()) {
		debug(2, "Macintosh Rex resource is not mapped: '%s'", filename);
		return nullptr;
	}

	Common::MacResManager *container = getContainer(resource.container);
	Common::SeekableReadStream *stream =
		container ? container->getResource(resource.type, resource.id) : nullptr;
	if (!stream) {
		debug(2, "Macintosh Rex resource is missing: '%s' (container %d, type %08x, ID %u)",
			filename, resource.container, resource.type, resource.id);
	}
	return stream;
}

Common::SeekableReadStream *MacResourceProvider::createFontResource() {
	if (!_fontManager)
		return nullptr;

	// The native executable consistently selects bold Geneva (font ID 3) at
	// 10 points for in-game text. MADS' DOS renderer expects its own packed
	// font format, so rasterize the native system font into that format.
	const Graphics::Font *font = _fontManager->getFont(Graphics::MacFont(
		Graphics::kMacFontGeneva, 10, Graphics::kMacFontBold));
	if (!font)
		return nullptr;

	enum {
		kGlyphCount = 128,
		kHeaderSize = 2 + kGlyphCount + kGlyphCount * 2
	};

	const int height = MIN<int>(font->getFontHeight(), 200);
	byte widths[kGlyphCount];
	uint16 offsets[kGlyphCount];
	Common::Array<byte> pixels;
	byte maxWidth = 0;

	for (int index = 0; index < kGlyphCount; ++index) {
		const uint32 character = index + 1;
		const int width = CLIP<int>(font->getCharWidth(character), 0, 255);
		widths[index] = width;
		maxWidth = MAX<byte>(maxWidth, width);
		offsets[index] = kHeaderSize + pixels.size();
		if (!width)
			continue;

		Graphics::Surface glyph;
		glyph.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		memset(glyph.getPixels(), 0, glyph.pitch * glyph.h);
		font->drawChar(&glyph, character, 0, 0, 1);

		const int rowBytes = (width + 3) / 4;
		for (int y = 0; y < height; ++y) {
			for (int byteIndex = 0; byteIndex < rowBytes; ++byteIndex) {
				byte packed = 0;
				for (int pixel = 0; pixel < 4; ++pixel) {
					const int x = byteIndex * 4 + pixel;
					if (x < width && *(const byte *)glyph.getBasePtr(x, y))
						packed |= 1 << (6 - pixel * 2);
				}
				pixels.push_back(packed);
			}
		}
		glyph.free();
	}

	const uint32 fontSize = kHeaderSize + pixels.size();
	const uint32 size = PackList::SIZE + fontSize;
	byte *data = createUncompressedPack(fontSize, PACK_PRIORITY_FONTS);
	if (!data)
		return nullptr;

	byte *fontData = data + PackList::SIZE;
	fontData[0] = height;
	fontData[1] = maxWidth;
	memcpy(fontData + 2, widths, sizeof(widths));
	for (int index = 0; index < kGlyphCount; ++index)
		WRITE_LE_UINT16(fontData + 2 + kGlyphCount + index * 2, offsets[index]);
	memcpy(fontData + kHeaderSize, pixels.data(), pixels.size());

	return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *MacResourceProvider::createInterfaceResource(int interfaceID) {
	// The Macintosh port replaces each DOS I#.INT bitmap with an InBx ILBM.
	// Adapt its 512x88 native interface to the shared 320x44 MADS renderer;
	// the images use only the first eight palette indices.
	Common::SeekableReadStream *source = _containers[kGlobalContainer]->getResource(
		MKTAG('I', 'n', 'B', 'x'), 1000 + interfaceID);
	if (!source)
		return nullptr;

	::Image::IFFDecoder decoder;
	const bool loaded = decoder.loadStream(*source);
	delete source;
	const Graphics::Surface *surface = decoder.getSurface();
	if (!loaded || !surface || surface->format.bytesPerPixel != 1 ||
			surface->w <= 0 || surface->h <= 0 || decoder.getPalette().size() < 16) {
		warning("Invalid Macintosh Rex interface resource I%d", interfaceID);
		return nullptr;
	}

	// Keep the unscaled native panel for the Macintosh presentation path.
	// The synthetic pack below exists only so the shared interface state
	// machine can continue operating in its original 320x44 coordinate space.
	_nativeInterface.copyFrom(*surface);

	const uint32 payloadSize = sizeof(Color) * 16 + video_x * inter_size_y;
	const uint32 size = PackList::SIZE + payloadSize;
	byte *data = createUncompressedPack(payloadSize, PACK_PRIORITY_INTERFACES);
	if (!data)
		return nullptr;

	Color *colors = (Color *)(data + PackList::SIZE);
	const Graphics::Palette &palette = decoder.getPalette();
	for (uint i = 0; i < 16; ++i) {
		byte r, g, b;
		palette.get(i, r, g, b);
		_nativeInterfacePalette[i * 3 + 0] = r;
		_nativeInterfacePalette[i * 3 + 1] = g;
		_nativeInterfacePalette[i * 3 + 2] = b;
		colors[i].r = PALETTE_8BIT_TO_6BIT(r);
		colors[i].g = PALETTE_8BIT_TO_6BIT(g);
		colors[i].b = PALETTE_8BIT_TO_6BIT(b);
	}

	// InBx pixels use only colors 0 through 7. The native interface code
	// selects 13, 14, and 15 for its two selection states and normal text,
	// respectively; supply those QuickDraw foreground colors independently
	// of the bitmap's otherwise-black unused CMAP entries.
	colors[8].r = colors[8].g = colors[8].b = 0;
	colors[13] = colors[1];
	colors[14] = colors[2];
	colors[15].r = colors[15].g = colors[15].b = 63;

	byte *target = data + PackList::SIZE + sizeof(Color) * 16;
	for (int y = 0; y < inter_size_y; ++y) {
		const int sourceY = ((2 * y + 1) * surface->h) / (2 * inter_size_y);
		const byte *sourceRow = (const byte *)surface->getBasePtr(0, sourceY);
		for (int x = 0; x < video_x; ++x) {
			const int sourceX = ((2 * x + 1) * surface->w) / (2 * video_x);
			const byte color = sourceRow[sourceX];
			if (color >= 16) {
				warning("Macintosh Rex interface I%d uses unsupported color %u", interfaceID, color);
				free(data);
				return nullptr;
			}
			target[y * video_x + x] = color;
		}
	}

	// Retain the exact baseline supplied to the shared interface renderer.
	// The presentation layer compares its live 320x44 buffer against this
	// image, then composites only the changed pixels over the native panel.
	_logicalInterface.create(video_x, inter_size_y,
		Graphics::PixelFormat::createFormatCLUT8());
	for (int y = 0; y < inter_size_y; ++y)
		memcpy(_logicalInterface.getBasePtr(0, y), target + y * video_x, video_x);

	debug(2, "Adapted Macintosh Rex interface I%d from %dx%d to %dx%d",
		interfaceID, surface->w, surface->h, video_x, inter_size_y);
	return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

bool MacResourceProvider::installCursorResource(uint16 id) {
	Common::SeekableReadStream *stream =
		_containers[kApplicationContainer]->getResource(MKTAG('C', 'U', 'R', 'S'), id);
	if (!stream || stream->size() != 68) {
		delete stream;
		return false;
	}

	byte data[68];
	stream->read(data, sizeof(data));
	delete stream;

	Graphics::Surface surface;
	surface.create(16, 16, Graphics::PixelFormat::createFormatCLUT8());
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			const int bit = y * 16 + x;
			const byte mask = data[32 + bit / 8] & (0x80 >> (bit & 7));
			const byte image = data[bit / 8] & (0x80 >> (bit & 7));
			*(byte *)surface.getBasePtr(x, y) = !mask ? 2 : (image ? 0 : 1);
		}
	}

	static const byte kCursorPalette[] = { 0, 0, 0, 255, 255, 255 };
	CursorMan.replaceCursor(surface, READ_BE_UINT16(data + 66),
		READ_BE_UINT16(data + 64), 2);
	CursorMan.replaceCursorPalette(kCursorPalette, 0, 2);
	surface.free();
	return true;
}

bool MacResourceProvider::setCursor(int id) {
	_cursorID = id;
	if (id <= 1) {
		const byte *data, *palette, *mask;
		int width, height, hotspotX, hotspotY, transparentColor;
		if (Graphics::MacWindowManager::getBuiltInCursorData(Graphics::kMacCursorArrow,
				data, palette, mask, width, height, hotspotX, hotspotY, transparentColor)) {
			CursorMan.replaceCursor(data, width, height, hotspotX, hotspotY,
				transparentColor, nullptr, mask);
			CursorMan.replaceCursorPalette(palette, 0, 2);
		} else {
			CursorMan.setDefaultArrowCursor();
		}
		return true;
	}
	if (id == 2) {
		_waitCursorFrame = 0;
		_nextCursorTime = g_system->getMillis() + 15;
		return installCursorResource(1000);
	}
	if (id >= 3 && id <= 6)
		return installCursorResource(3000 + id);
	return false;
}

void MacResourceProvider::updateCursor() {
	if (_cursorID != 2)
		return;

	const uint32 now = g_system->getMillis();
	if (now < _nextCursorTime)
		return;

	const uint32 frames = 1 + (now - _nextCursorTime) / 15;
	_waitCursorFrame = (_waitCursorFrame + frames) % 34;
	_nextCursorTime += frames * 15;
	installCursorResource(1000 + _waitCursorFrame);
}

bool MacResourceProvider::exists(const char *filename) {
	Common::SeekableReadStream *stream = open(filename);
	const bool result = stream != nullptr;
	delete stream;
	return result;
}

bool MacResourceProvider::allowsFallback(const char *filename) const {
	// Star-prefixed names address the MADS virtual resource hierarchy. The
	// Macintosh release stores that hierarchy entirely in resource forks and
	// has no DOS HAG archives to search after a native lookup fails. Ordinary
	// paths remain eligible for the established filesystem fallback.
	return !filename || filename[0] != '*';
}

Common::SeekableReadStream *MacResourceProvider::openText(int32 id, uint16 &unpackedSize) {
	uint32 type;
	uint16 resourceID;
	if (id >= 10000) {
		type = makeRoomType('M', id / 100);
		resourceID = 1000 + id % 100;
	} else {
		type = MKTAG('M', '0', '0', '0');
		resourceID = 1000 + id;
	}

	Common::SeekableReadStream *stream = _containers[kGlobalContainer]->getResource(type, resourceID);
	if (!stream)
		return nullptr;

	const int32 storedID = stream->readSint32BE();
	stream->skip(4); // Offset in the original aggregate message file.
	unpackedSize = stream->readUint16BE();
	if (storedID != id || stream->err()) {
		warning("Invalid Macintosh message resource for ID %d", id);
		delete stream;
		return nullptr;
	}

	return stream;
}

Common::SeekableReadStream *MacResourceProvider::openSound(int section, int commandId) {
	if (section < 1 || section > 9 || commandId < 0 || commandId >= 1000)
		return nullptr;
	return _containers[kSoundContainer]->getResource(MKTAG('s', 'n', 'd', ' '),
		section * 1000 + commandId);
}

} // namespace RexNebular
} // namespace MADS
