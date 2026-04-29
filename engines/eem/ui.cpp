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
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/music.h"
#include "eem/site.h"

// EEM — UI screens (NOTE.C, GALLERY.C, ACCUSE.C, MAP.C, CHOOSE.C combined).
// Each function is a self-contained modal `EEMEngine::doX()` reachable from
// the site loop. They share the same wait-for-input idiom and PIC 0x3f /
// 0x41 / 0x42 / 0x43 backdrops.

namespace EEM {

// Five fixed gallery slot positions verified at `29be:0x116`. Used by
// both `_DrawGallery @ 158f:0046` (notebook gallery) and the accuse
// portrait grid; the layout is identical so we share the table.
struct GallerySlot { int x; int y; };
const GallerySlot kGallerySlots[5] = {
	{  83,  14 }, // 0
	{ 155,  14 }, // 1
	{ 227,  14 }, // 2
	{ 119,  90 }, // 3
	{ 191,  90 }  // 4
};

// `_GetKDTextBalloon @ 1df2:0105` digit-balloon table @ `29be:1064`:
//   '0'→0x15  '1'→0x16  '2'→0x17  '3'→0x18  '4'→0x19
//   '5'→0x1a  '6'→0x20  '7'→0x21  '8'→0x22  '9'→0x1e
const uint16 kDigitBalloons[10] = {
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x20, 0x21, 0x22, 0x1e
};

// Return the next non-empty slot in `slotRects` starting from `from`,
// stepping by `dir` (+1 or -1) with wraparound. Used by the accuse
// gallery's keyboard-cycle (TAB / arrow keys) — mirrors the way
// `_PutMouseInRect` skips eliminated suspects in the original.
int nextLiveSlot(const Common::Array<Common::Rect> &slotRects,
				 int from, int dir) {
	const int n = (int)slotRects.size();
	if (n <= 0)
		return 0;
	for (int step = 1; step <= n; step++) {
		int idx = (from + dir * step) % n;
		if (idx < 0)
			idx += n;
		if (!slotRects[idx].isEmpty())
			return idx;
	}
	return from;
}

// Snapshot of `doCaseSelection`'s captured locals, used by
// `drawCaseSelectionFrame` (which replaces the original lambda). Lives
// on the stack inside `doCaseSelection`; never escapes.
struct CaseSelectionView {
	EEMEngine *vm;
	const Picture *caseBg;
	bool haveCaseBg;
	const Animation *kdAnim;
	bool haveKdAnim;
	uint16 kdAnimId;     ///< 0x15 / 0x16 — looked up in kAnimScripts
	int kdAnimX;
	int kdAnimY;
	const char *separator;
	const char *const *pickLabel;
	const bool *pickEnabled;
	uint pick;
};

// Mystery list shown in the "Choose A Mystery" sub-screen. Mirrors
// `_DoChooseMystery @ 1a35:02b7`: opens BOOK%u.NME (CRLF-separated
// ASCII strings, last entry is whitespace = sentinel), reads up to 25
// lines × 40 bytes each, hands the array to `_CaseSelection`. We
// preserve the trailing whitespace line as the original sentinel
// since `_DoChoose @ 1c33:0514` walks until `*piVar3 == 0 && piVar3[1]
// == 0` — but for our renderer we just keep the names array.
Common::StringArray loadBookNames(uint book) {
	Common::StringArray names;
	const Common::String fname = Common::String::format("BOOK%u.NME", book);
	Common::File f;
	if (!f.open(Common::Path(fname))) {
		warning("loadBookNames: %s missing", fname.c_str());
		return names;
	}
	while (!f.eos()) {
		Common::String line = f.readLine();
		if (f.eos() && line.empty())
			break;
		// `_fgets` in the original reads CRLF terminators with the line;
		// `Common::File::readLine` strips them, so `line` here is the
		// text only. Trim trailing whitespace so the sentinel "        "
		// last entry doesn't render as a blank scrollable row.
		while (!line.empty() &&
			   (line.lastChar() == ' ' || line.lastChar() == '\t' ||
				line.lastChar() == '\r'))
			line.deleteLastChar();
		if (line.empty())
			continue;
		names.push_back(line);
	}
	return names;
}

// Per-mystery sub-chooser ("Choose A Mystery") view.
//
// `names` are the entries from BOOK%d.NME (in display order — index 0
// = first case in the tier, mystery number = `tierLo + index`).
// `solvedFlags` is a parallel bool array indicating which entries are
// already solved (greyed and unselectable in `_DoChoose`).
// `topRow` is the scroll position; up to 12 entries are visible.
// `selRow` is the highlighted row (0-based within the names array).
struct CaseSubmenuView {
	EEMEngine *vm;
	const Picture *caseBg;
	bool haveCaseBg;
	const Common::StringArray *names;
	const Common::Array<bool> *solvedFlags;
	uint topRow;
	uint selRow;
	uint book;            ///< 1..3 — for the "Book N" / "Challenge Book" title
};

// Mirrors `_DoChoose`'s `DrawList @ 1c33:040d`. 12 visible rows × 10 px
// at (61, 35); colour palette: 0x13 = highlighted (selected), 0x1B =
// greyed (already solved), 0x5C = default. We approximate with the
// closest indices of site palette 0 — 0xF (white) / 0x7 (medium grey)
// / 0x8 (dark grey) — since we don't decode the original CLUT byte
// ramp.
void drawCaseSubmenu(const CaseSubmenuView &v) {
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (v.haveCaseBg)
		scratch.simpleBlitFrom(v.caseBg->surface);
	if (!v.vm->getFont().isLoaded() || !v.names)
		return;

	// Top centred title. `_CaseSelection @ 1c33:0aa3` formats "Book %d"
	// for tiers 1/2 and "Challenge Book" (sprintf with no arg) for
	// tier 3. `_Show_String(0xc, (0xba - width)/2 + 0x3c, …, 0x10)`
	// places it horizontally centred over the panel.
	const Common::String title = (v.book == 3)
		? Common::String("Challenge Book")
		: Common::String::format("Book %u", v.book);
	const int titleW = v.vm->getFont().getStringWidth(title);
	const int titleX = (0xba - titleW) / 2 + 0x3c;
	v.vm->getFont().drawString(&scratch, title, titleX, 12, 320, 0xF);

	const int kListX  = 61;
	const int kListW  = 238 - kListX;
	const int kListY0 = 35;
	const int kLineH  = 10;
	const int kVisible = 12;
	const uint count = (uint)v.names->size();

	for (int r = 0; r < kVisible; r++) {
		const uint idx = v.topRow + (uint)r;
		if (idx >= count)
			break;
		const Common::String &name = (*v.names)[idx];
		byte color = 0xF;  // default
		if (idx == v.selRow) {
			color = 0xF;   // highlighted
		} else if (v.solvedFlags && idx < v.solvedFlags->size() &&
				   (*v.solvedFlags)[idx]) {
			color = 0x8;   // greyed (already solved)
		} else {
			color = 0x7;   // normal
		}
		v.vm->getFont().drawString(&scratch, name,
			kListX, kListY0 + r * kLineH, kListW, color);
	}

	// Selection arrow at the left edge of the highlighted row — the
	// original highlights via colour change but adding an arrow makes
	// the keyboard-driven path obvious.
	if (v.selRow >= v.topRow && v.selRow < v.topRow + (uint)kVisible) {
		const int r = (int)(v.selRow - v.topRow);
		v.vm->getFont().drawString(&scratch, ">",
			kListX - 6, kListY0 + r * kLineH, 6, 0xF);
	}

	// Scrollbar thumb. `DrawThumb @ 1c33:????` renders a thumb at
	// (240, 45..146) proportional to scroll position. We draw a
	// 1-px outlined block to indicate the same range.
	if (count > (uint)kVisible) {
		const int trackY0 = 45;
		const int trackH  = 146 - 45;
		const int thumbH  = MAX<int>(8, (trackH * kVisible) / (int)count);
		const int travel  = trackH - thumbH;
		const int pos = (int)v.topRow * travel /
						MAX<int>(1, (int)count - kVisible);
		const Common::Rect thumb(240, trackY0 + pos,
								  250, trackY0 + pos + thumbH);
		scratch.fillRect(thumb, 0x8);
		scratch.frameRect(thumb, 0xF);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

void drawCaseSelectionFrame(const CaseSelectionView &v) {
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (v.haveCaseBg)
		scratch.simpleBlitFrom(v.caseBg->surface);

	// KD greeter frame — masked-blit current animation cell at
	// (0x112, 0x50). 100 ms tick matches `_CheckFrameRate`. The
	// original `_CaseSelection @ 1c33:0a87` calls `_NewAnimation(...,
	// CONCAT22(0x15, ...), ..., seqnum=0x15, ...)` so the script
	// key is 0x15 regardless of partner — even Jenny's CELLS (loaded
	// via animID 0x16 = ANI.DBD slot) get driven by Jake's 0x15
	// blink script. Both 0x15 and 0x16 are aliases of 0x00 in our
	// table so the result is identical, but routing through 0x15
	// matches the binary.
	if (v.haveKdAnim) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = partnerFrameAtTick(0x15,
												  (uint)v.kdAnim->size(), now);
		// Anchor-aware blit. Same rendering path used everywhere
		// the partner is registered through `_NewAnimation` in the
		// original.
		blitAnimFrameAnchored(scratch.surfacePtr(),
							  (*v.kdAnim)[frameIdx],
							  v.kdAnimX, v.kdAnimY);
	}
	if (v.vm->getFont().isLoaded()) {
		// `DrawList` @ 1c33:040d coordinates: `_TextBox + 3` for x
		// and `DAT_29be_0d02` for y. `_TextBox` @ 29be:0d00 holds
		// {x=58, y=35, x2=238, y2=158}. Matches the blue panel.
		const int kListX  = 58 + 3;
		const int kListW  = 238 - kListX;
		const int kListY0 = 35;
		const int kLineH  = 10;

		// Top centred "Book %d" / "Challenge Book" title — sprintf
		// format strings at 29be:0deb / 29be:0dfa shown via
		// `_Show_String(0xc, (0xba - width)/2 + 0x3c, …)` in the
		// original. We don't track challenge tier yet so always
		// show "Book 1".
		const Common::String book = "Book 1";
		const int titleW = v.vm->getFont().getStringWidth(book);
		const int titleX = (0xba - titleW) / 2 + 0x3c;
		v.vm->getFont().drawString(&scratch, book, titleX, 12, 320, 0xF);

		// Render 11 list rows: separator + menu item pairs.
		//   row 0  separator
		//   row 1  Choose A Mystery
		//   row 2  separator
		//   row 3  Practice Mystery
		//   ...
		//   row 9  See ScrapBook 3
		//   row 10 separator
		for (int r = 0; r < 11; r++) {
			const int y = kListY0 + r * kLineH;
			if ((r & 1) == 0) {
				v.vm->getFont().drawString(&scratch, v.separator,
										   kListX, y, kListW, 0x7);
				continue;
			}
			const uint mp = (uint)(r >> 1);
			const bool isSel  = (mp == v.pick);
			const byte color  = isSel             ? 0xF :
								v.pickEnabled[mp] ? 0x7 : 0x8;
			v.vm->getFont().drawString(&scratch, v.pickLabel[mp],
									   kListX, y, kListW, color);
		}
	}
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

void EEMEngine::doProfilePicker() {
	// Mirrors `screen8_handler @ 1c33:1012`. The original walks
	// `*.PLR` files in `C:\EEMCDSAV\` (max 25), reads the first 12
	// bytes of each (the player-name field of `_PlayerRecord`), and
	// hands the list to `_DoChoose`. If no profiles exist (loop hits
	// `local_20 == 0` at 1c33:1170), it falls straight into
	// `_NewPlayer`. Selecting an entry calls `_LoadPlayerRecord` and
	// returns; selecting the "exit" sentinel goes back to title.

	// Palette reset. `screen8_handler` runs `_FadeOut(); _GetPalette(0);
	// _GetBackground(0x104);` before the picker, so the BG always
	// renders against SITEPALS index 0 regardless of which intro
	// palette was active last. Without this, skipping out of an intro
	// anim (THEME / ANIM01..20 / TITLE) leaves the previous video's
	// palette in place and the picker draws with the wrong colours.
	setSitePalette(0);

	const SaveStateList saves = listProfiles();
	if (saves.empty()) {
		doNewPlayer();
		return;
	}

	if (!_font.isLoaded()) {
		// No font means we can't render the picker — fall through.
		doNewPlayer();
		return;
	}

	// Build the visible list: existing profile names + "[New Player]".
	struct Entry {
		Common::String label;
		int slot;       ///< -1 means "create new"
	};
	Common::Array<Entry> entries;
	for (const SaveStateDescriptor &s : saves) {
		Entry e;
		e.label = s.getDescription();
		e.slot  = s.getSaveSlot();
		entries.push_back(e);
	}
	Entry newEntry;
	newEntry.label = "[New Player]";
	newEntry.slot  = -1;
	entries.push_back(newEntry);

	int sel = 0;
	bool done = false;

	// Picker geometry: `DrawList @ 1c33:040d` is called from
	// `screen8_handler @ 1c33:1012` with `(_TextBox + 3, DAT_29be_0d02)`.
	// `_TextBox @ 29be:0d00` holds {x1=58, y1=35, x2=238, y2=158} so
	// the list origin is (61, 35), 10 px per row, max 12 visible
	// rows. The "Pick a player" caption is part of the BG (PIC 0x104)
	// — `screen8_handler` never draws it as text — so an extra
	// `drawString` would overlay on top of the baked-in heading.
	const int kListX = 61;
	const int kListY = 35;
	const int kLineH = 10;
	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		Picture bg;
		if (_picsArchive.getPicture(0x104, bg))
			scratch.simpleBlitFrom(bg.surface);
		for (uint i = 0; i < entries.size(); i++) {
			const byte color = ((int)i == sel) ? 0xF : 0x8;
			_font.drawString(&scratch, entries[i].label,
							 kListX, kListY + (int)i * kLineH, 220, color);
		}
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};
	draw();

	while (!done && !shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		bool committed = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_playerName = "Detective";
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_UP:
					sel = (sel + (int)entries.size() - 1) % (int)entries.size();
					dirty = true;
					break;
				case Common::KEYCODE_DOWN:
					sel = (sel + 1) % (int)entries.size();
					dirty = true;
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					committed = true;
					break;
				case Common::KEYCODE_ESCAPE:
					_playerName = "Detective";
					return;
				default:
					break;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const int hit = (ev.mouse.y - kListY) / kLineH;
				if (hit >= 0 && hit < (int)entries.size()) {
					sel = hit;
					committed = true;
				}
			}
			if (committed)
				break;
		}
		if (committed) {
			done = true;
			break;
		}
		if (dirty)
			draw();
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	const Entry &e = entries[sel];
	if (e.slot < 0) {
		doNewPlayer();
	} else {
		// Mirrors `_LoadPlayerRecord` at 1c33:1281 — slot found,
		// load it. The save body re-fills `_playerName`, partner,
		// chain stage, mysteriesSolved.
		if (!loadProfile(e.label)) {
			warning("doProfilePicker: failed to load profile '%s' at slot %d",
					e.label.c_str(), e.slot);
			doNewPlayer();
		}
	}
}

void EEMEngine::doNewPlayer() {
	// Mirrors `_NewPlayer` @ 1c33:0dda. The original draws background
	// 0x104 + character peek pic 0x107, then shows "Please type your
	// name" and accepts up to 12 characters until Enter. We render a
	// minimal version: black screen + prompt.
	if (!_font.isLoaded()) {
		_playerName = "Detective";
		return;
	}

	Common::String name;
	const int maxChars = 12;

	// Mirror the original: load PIC 0x104 as the name-entry backdrop.
	// The original also slides in PIC 0x107 (a peeking character).
	Picture bg;
	const bool haveBG = _picsArchive.getPicture(0x104, bg);

	// Match the original `_NewPlayer`: `_Show_String(rw=0x28, cl=0x50)`
	// for the prompt, then `_ShowChar(0x50, x, …)` for typed input.
	// (rw=row=y, cl=col=x.) Prompt at (y=40, x=80), input at (y=80, x=80).
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (haveBG)
		scratch.simpleBlitFrom(bg.surface);
	_font.drawString(&scratch, "Please type your name:", 80, 40, 240, 0xF);
	_font.drawString(&scratch, name + "_", 80, 80, 240, 0xF);
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_RETURN) {
				if (name.empty())
					name = "Detective";
				// Mirrors `_NewPlayer @ 1c33:0dda` tail (1c33:0fa0+):
				// after the name is committed, try `_LoadPlayerRecord`
				// — if it returns 0 (no existing .PLR), zero out the
				// per-profile state and call `_SavePlayerRecord` to
				// create a fresh profile file. Same flow here, mapped
				// onto ScummVM save slots via name → description.
				if (!loadProfile(name)) {
					_playerName = name;
					memset(_mysteriesSolved, 0, sizeof(_mysteriesSolved));
					_mystery.clear();
					_partner = 0;
					// `_NewPlayer @ 1c33:0fa3` writes
					// `DAT_2d5d_3f99 = 1` — fresh profiles always
					// start at the Junior tier.
					_chainStage = 1;
					saveProfile(name);
				}
				return;
			}
			if (k == Common::KEYCODE_ESCAPE) {
				_playerName = "Detective";
				return;
			}
			if (k == Common::KEYCODE_BACKSPACE) {
				if (!name.empty()) {
					name.deleteLastChar();
					dirty = true;
				}
				continue;
			}
			if (ev.kbd.ascii >= ' ' && ev.kbd.ascii < 127 &&
				(int)name.size() < maxChars) {
				name += (char)ev.kbd.ascii;
				dirty = true;
			}
		}
		if (dirty) {
			// Re-render with the updated `name`. Same body as the
			// initial render above — only `name + "_"` changes.
			scratch.clear();
			if (haveBG)
				scratch.simpleBlitFrom(bg.surface);
			_font.drawString(&scratch, "Please type your name:",
							 80, 40, 240, 0xF);
			_font.drawString(&scratch, name + "_", 80, 80, 240, 0xF);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, 320, 200);
			g_system->updateScreen();
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

int EEMEngine::doShowEnding(uint num, bool firstPage) {
	// Mirrors `_DisplayEnding @ 1df2:0548` + `_DisplayEndingPage @
	// 1df2:044c`. File format (verified by reading E0.BIN's bytes):
	//   u16 pageCount
	//   for each page:
	//     u16 picNum
	//     u16 x1, y1, x2, y2  (story rect — passed to WordWrap)
	//     char text[]        (null-terminated, ParseString opcodes)
	//
	// The original swaps the font: `_FreeFont(); _LoadFont("tiny.fnt")`
	// at 1df2:055f-1df2:0563, calls `_GetPalette(0)` (site palette 0),
	// then for each page `_GetBackground(picNum)` +
	// `_WordWrap2(x1, y1, x2-x1, text, fontColor=0, dropColor=-1)`. The
	// fontColor=0 draws in palette index 0 (the newspaper's body-text
	// colour), with no drop shadow. Verified at the call site asm
	// 1df2:04cf-1df2:04f4 (Ghidra mis-paired the two trailing args).
	//
	// Page navigation mirrors the original key/click handlers
	// (1df2:0689 / 1df2:06a0): LEFT decrements pageIdx, RIGHT (or
	// SPACE / Enter / click) increments it. Hitting the boundary
	// (LEFT on page 0, RIGHT on last page) sets `[BP-0x18]` to -1 / 1
	// respectively and exits — that return value is what
	// `_ShowScrapbook` uses to walk forward / backward through
	// solved mysteries (see 1f78:0664-1f78:069c). ESC and clicks
	// outside both PrevPage / NextPage rects exit with `[BP-0x18]=0`.
	//
	// `firstPage=false` opens the ending at the LAST page (used by
	// `doShowScrapbook` after a "previous mystery" navigation —
	// matches `local_8 = 0` written before the back-step at
	// 1f78:067e).
	const Common::String fname = Common::String::format("E%u.BIN", num);
	Common::File f;
	if (!f.open(Common::Path(fname))) {
		warning("doShowEnding: %s missing", fname.c_str());
		return 0;
	}
	const uint32 size = f.size();
	if (size < 2) {
		warning("doShowEnding: %s too small (%u bytes)",
				fname.c_str(), size);
		return 0;
	}
	Common::Array<byte> buf(size);
	if (f.read(buf.data(), size) != size) {
		warning("doShowEnding: %s short read", fname.c_str());
		return 0;
	}

	const uint16 pageCount = READ_LE_UINT16(buf.data());
	if (pageCount == 0)
		return 0;

	// Mirrors 1df2:0558-1df2:056a — `_FreeFont(); _LoadFont(tiny.fnt)`.
	// The newspaper layout uses TINY.FNT (smaller glyphs) so the body
	// copy fits in the columns. `_LoadFont(font.fnt)` is restored at
	// 1df2:0625 after the page loop.
	EEMFont tinyFont;
	const bool haveTinyFont = tinyFont.load(Common::Path("TINY.FNT"));
	if (!haveTinyFont)
		warning("doShowEnding: TINY.FNT failed to load — falling back");

	// Mirrors 1df2:055f `_GetPalette(0)` — site palette 0 is the
	// shared "newspaper" CLUT for ending pages. The newspaper body
	// text in particular is palette index 0 (= newspaper black) so we
	// MUST switch palettes before rendering.
	setSitePalette(0);

	// Walk page records. Each page header is 10 bytes; text is
	// null-terminated and follows the header.
	uint pageOffsets[8];   // ENDING_RANGE_MAX from `_DisplayEnding`
	const uint kMaxPages = MIN<uint>(pageCount,
									 (uint)(sizeof(pageOffsets) / sizeof(uint)));
	uint cursor = 2;
	for (uint p = 0; p < kMaxPages; p++) {
		pageOffsets[p] = cursor;
		if (cursor + 10 >= size)
			break;
		// Skip the 10-byte header and find the null terminator.
		cursor += 10;
		while (cursor < size && buf[cursor] != 0)
			cursor++;
		cursor++;  // past the null
	}

	uint pageIdx = firstPage ? 0 : (kMaxPages - 1);
	int direction = 0;     // -1 / 0 / +1, see header doc.
	bool exitLoop = false;
	bool dirty = true;
	while (!shouldQuit() && !exitLoop) {
		if (dirty) {
			const uint off = pageOffsets[pageIdx];
			if (off + 10 >= size)
				break;
			const uint16 picNum = READ_LE_UINT16(buf.data() + off);
			const uint16 x1     = READ_LE_UINT16(buf.data() + off + 2);
			const uint16 y1     = READ_LE_UINT16(buf.data() + off + 4);
			const uint16 x2     = READ_LE_UINT16(buf.data() + off + 6);
			(void)READ_LE_UINT16(buf.data() + off + 8);  // y2 (unused — WordWrap2 takes width only)

			Picture bg;
			Graphics::ManagedSurface scratch(320, 200,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.clear();
			if (_picsArchive.getPicture(picNum, bg))
				scratch.simpleBlitFrom(bg.surface);

			// Story text. The bytes are a null-terminated string with
			// `_ParseString` placeholders (0x80 = player name, 0x82
			// = partner first name, etc.).
			const char *raw = (const char *)buf.data() + off + 10;
			const Common::String text = parseString(raw, _playerName, _partner);

			// Use TINY.FNT (`_LoadFont(@29be:10a5)` at 1df2:055f-0563)
			// and color 0 (`_WordWrap2(...,0,-1)` per asm at 1df2:04cf,
			// not 0xF as Ghidra's decompile output suggests). Falls
			// back to the main font if TINY.FNT failed to load.
			const EEMFont &renderFont = haveTinyFont ? tinyFont : _font;
			if (renderFont.isLoaded() && x2 > x1) {
				const int textW = MIN<int>((int)x2 - (int)x1, 320 - (int)x1);
				renderFont.drawWordWrapped(&scratch, (int)x1, (int)y1,
										   textW, text, 0);
			}

			// Page indicator at top-right ("page 1/3"). Stays in the
			// main font + color 0xF so it doesn't blend into the
			// newspaper masthead.
			if (_font.isLoaded() && kMaxPages > 1) {
				const Common::String hdr = Common::String::format(
					"%u/%u", (unsigned)pageIdx + 1, (unsigned)kMaxPages);
				_font.drawString(&scratch, hdr, 280, 4, 32, 0xF);
			}

			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, 320, 200);
			g_system->updateScreen();
			dirty = false;
		}

		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				return 0;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					// ESC is the ONLY way out of the newspaper view.
					// Departs from the original `_DisplayEnding`, which
					// also exits on boundary arrow keys / clicks
					// (1df2:0689 / 1df2:06a0); the boundary-exit path is
					// what fed `[BP-0x18]` to `_ShowScrapbook` for
					// per-mystery scrapbook navigation. We don't expose
					// that — clicking ESC closes the scrapbook entirely.
					direction = 0;
					exitLoop = true;
					break;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_PAGEUP:
					// Clamp at page 0 — never exit on LEFT.
					if (pageIdx > 0) {
						pageIdx--;
						dirty = true;
					}
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
				case Common::KEYCODE_SPACE:
				case Common::KEYCODE_TAB:
					// Clamp at last page — never exit on RIGHT either.
					if (pageIdx + 1 < kMaxPages) {
						pageIdx++;
						dirty = true;
					}
					break;
				default:
					break;
				}
				if (exitLoop)
					break;
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Mouse clicks shift pages too — never exit on click.
				// Use the original PrevPage/NextPage rect split for
				// click direction (29be:1078 / 29be:1080); clicks
				// outside both rects fall through to next-page so the
				// player still gets some feedback.
				const Common::Rect kPrevPageRect(0, 0, 27, 200);
				if (kPrevPageRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (pageIdx > 0) {
						pageIdx--;
						dirty = true;
					}
				} else {
					if (pageIdx + 1 < kMaxPages) {
						pageIdx++;
						dirty = true;
					}
				}
				if (exitLoop)
					break;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	return direction;
}

void EEMEngine::doShowScrapbook(uint stage) {
	// Mirrors `_ShowScrapbook(stage, 0) @ 1f78:0642`. Walk the
	// stage's mystery range and call `_DisplayEnding` on each solved
	// mystery. The original splits the 55 cases into three tiers:
	//   stage 1 (Junior) → mysteries  1..0x18 (24 cases)
	//   stage 2 (Senior) → mysteries 0x19..0x30 (24 cases)
	//   stage 3 (Master) → mysteries 0x31..0x36 (6 cases)
	// Each tier's range is `lo = (stage-1)*0x18 + 1`, `hi = lo + 0x17`
	// (verified at 1f78:064b: `iVar1 = (param_1 - 1) * 0x18; uVar2 =
	// iVar1 + 1`). The current-stage filter at 1f78:065e
	// (`if (DAT_2d5d_3f99 == param_1)`) skips unsolved mysteries
	// inside the player's CURRENT tier — completed tiers show every
	// mystery regardless. We mirror that exactly.
	if (stage < 1 || stage > 3)
		return;
	const uint lo = (stage - 1) * 0x18 + 1;
	const uint hi = lo + 0x17;
	const bool currentTier = (stage == _chainStage);

	// `doShowEnding` only reports `direction = 0` now (ESC), so we
	// can't use the original 1f78:067e/0698 forward-backward walk.
	// Instead iterate every solved mystery in the tier linearly:
	// each ESC closes the current newspaper and moves us to the
	// next solved entry. Departs from `_ShowScrapbook @ 1f78:0642`
	// (which let the player browse via the boundary keys), but
	// matches the user-requested "ESC is the only exit" rule.
	for (uint m = lo; m <= hi && !shouldQuit(); m++) {
		if (m >= sizeof(_mysteriesSolved))
			break;
		// Current-tier filter (1f78:0664). Completed tiers show all
		// 24 entries; the active tier hides unsolved ones because
		// the player hasn't earned that scrapbook page yet.
		if (currentTier && _mysteriesSolved[m] == 0)
			continue;
		(void)doShowEnding(m, /*firstPage=*/true);
	}
}

void EEMEngine::doSetup() {
	// Mirrors `_DoSetup @ 1f78:044e`. The setup screen is BG `PIC 0x40`
	// (loaded once on entry) with every label baked in — "Setup",
	// "Partner", "Sound", "Music", the "Jake"/"Jenny"/"On"/"Off"
	// option strings, etc. — all rendered in palette key `0xFE`. The
	// original then runs `_SetupSettings @ 1f78:000d` which uses
	// `_SwapColors @ 172b:1d2a` to recolour those `0xFE` pixels per
	// label rect: `0x15` for the active state, `0` for the inactive
	// one. So nothing is drawn as text; the visible state of each
	// toggle is purely a per-rect colour swap on top of `PIC 0x40`.
	//
	// Click hit-tests go through `_SetupButtons @ 29be:1218` — 13×
	// 8-byte rects. Each click runs `HandleSetupButton @ 1f78:0158`,
	// which dispatches via the 12-entry jumptable at `1f78:0436`.
	// Verified handler map (decompiled at each jumptable target):
	//   [0]  ( 20, 44, 39, 61)   Partner toggle (1f78:017a)
	//   [1]  ( 20, 87, 39,104)   Voice toggle   (1f78:0196 → DAT_2d5d_3f97)
	//   [2]  ( 20,127, 39,144)   back to profile (NextScreen=8)
	//   [3]  (281, 43,299, 60)   ScrapBook 1   (_ShowScrapbook(0,1))
	//   [4]  (281, 62,299, 79)   ScrapBook 2   gated chainStage>=2
	//   [5]  (281, 81,299, 98)   ScrapBook 3   gated chainStage>=3
	//   [6]  (281,108,299,125)   Save game     (_SaveGame @ 2404:0c87)
	//   [7]  (281,127,299,144)   New Case      (NextScreen=0xa)
	//   [8]  ( 53,153,108,183)   Done          (SI=1, exit)
	//   [9]  (145,163,174,187)   Help          (_InterfaceHelp(1))
	//   [10] (212,153,266,184)   Quit          (_AreYouSure → NextScreen=0xffff)
	//   [11] ( 81, 25,238, 37)   Credits       (PIC 0x208 fullscreen)
	//   [12] ( 11,  1,  3,  3)   debug placeholder
	// Highlight rects (`Kid1 @ 29be:1320` / `Kid2 @ 29be:1328` /
	// `SoundOn @ 29be:1330` / `SoundOff @ 29be:1338`) drive
	// `_SwapColors`; they're not click targets in the original.
	if (!_font.isLoaded()) {
		_nextScreen = (ScreenId)_lastScreen;
		return;
	}

	// Original button rects (`_SetupButtons` indices wired here).
	const Common::Rect kPartnerBtn   ( 20,  44,  39,  61); // [0]
	const Common::Rect kVoiceBtn     ( 20,  87,  39, 104); // [1]
	const Common::Rect kProfileBtn   ( 20, 127,  39, 144); // [2]
	const Common::Rect kScrap1Btn    (281,  43, 299,  60); // [3]
	const Common::Rect kScrap2Btn    (281,  62, 299,  79); // [4]
	const Common::Rect kScrap3Btn    (281,  81, 299,  98); // [5]
	const Common::Rect kSaveBtn      (281, 108, 299, 125); // [6]
	const Common::Rect kNewCaseBtn   (281, 127, 299, 144); // [7]
	const Common::Rect kDoneBtn      ( 53, 153, 108, 183); // [8]
	const Common::Rect kHelpBtn      (145, 163, 174, 187); // [9]
	const Common::Rect kQuitBtn      (212, 153, 266, 184); // [10]
	const Common::Rect kCreditsBtn   ( 81,  25, 238,  37); // [11]
	// Highlight / fallback-click rects.
	const Common::Rect kKid1Rect     ( 99,  44, 148,  52);
	const Common::Rect kKid2Rect     ( 99,  54, 148,  62);
	const Common::Rect kSoundOnRect  (106,  86, 125,  94);
	const Common::Rect kSoundOffRect (106,  96, 125, 104);

	// Pixel-level color-key swap. Mirrors `_SwapColors @ 172b:1d2a`:
	// for each pixel in `r` whose value equals `from`, replace with
	// `to`. `0xFE` is the BG's text-key color; `0x15` is the active
	// (bright) palette index, `0x00` the inactive one — both verified
	// in `_SetupSettings @ 1f78:000d`.
	auto swapColors = [](Graphics::ManagedSurface &dst,
						 const Common::Rect &r, byte from, byte to) {
		const int x1 = MAX<int>(0, r.left);
		const int y1 = MAX<int>(0, r.top);
		const int x2 = MIN<int>(320, r.right);
		const int y2 = MIN<int>(200, r.bottom);
		for (int y = y1; y < y2; y++) {
			byte *row = (byte *)dst.getBasePtr(0, y);
			for (int x = x1; x < x2; x++) {
				if (row[x] == from)
					row[x] = to;
			}
		}
	};

	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		Picture bg;
		if (_picsArchive.getPicture(0x40, bg))
			scratch.simpleBlitFrom(bg.surface);

		const byte kKey    = 0xFE;
		const byte kBright = 0x15;
		const byte kDim    = 0x00;
		swapColors(scratch, kKid1Rect, kKey,
				   _partner == 0 ? kBright : kDim);
		swapColors(scratch, kKid2Rect, kKey,
				   _partner == 1 ? kBright : kDim);
		swapColors(scratch, kSoundOnRect,  kKey,
				   _voiceOn ? kBright : kDim);
		swapColors(scratch, kSoundOffRect, kKey,
				   _voiceOn ? kDim : kBright);

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};
	draw();

	// Modal "Are you sure?" yes/no prompt. Mirrors `_AreYouSure @
	// 1a35:0a5c` — the original draws a centred message, listens for
	// Y/Enter (confirm) or N/ESC (cancel). We render a minimal
	// overlay with Y / N keys (and click on left/right halves) so
	// the Quit button gives the player a chance to back out.
	auto areYouSure = [&]() -> bool {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			for (int row = 0; row < 200; row++)
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)cur->getBasePtr(0, row), 320);
			g_system->unlockScreen();
		}
		const Common::Rect kBox(80, 80, 240, 120);
		scratch.fillRect(kBox, 0x00);
		_font.drawString(&scratch, "Are you sure?", 100, 88, 200, 0xF);
		_font.drawString(&scratch, "Y = yes   N = no", 100, 102, 200, 0xF);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
		while (!shouldQuit()) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
					return true;
				if (ev.type == Common::EVENT_KEYDOWN) {
					const Common::KeyCode k = ev.kbd.keycode;
					if (k == Common::KEYCODE_y ||
						k == Common::KEYCODE_RETURN)
						return true;
					if (k == Common::KEYCODE_n ||
						k == Common::KEYCODE_ESCAPE)
						return false;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					return ev.mouse.x < 160;
				}
			}
			g_system->delayMillis(15);
		}
		return false;
	};

	// Render `picId` and block until click/key. Returns the pressed
	// keycode (KEYCODE_ESCAPE for an explicit bail, KEYCODE_INVALID
	// for a click or any other key). When `transparent` is true,
	// preserve the current screen behind and overlay `picId` with
	// its transparent colour key — mirrors `_InterfaceHelp @
	// 1560:0205` calling `_Rect_Move_Mask` with the pic's
	// `miscflags >> 8` as the transp byte. When false, do a raw
	// fullscreen blit — mirrors the credits handler at 1f78:0281
	// using `_vga_fbuffvid`.
	auto showFullscreenPic = [&](uint16 picId,
								  bool transparent) -> Common::KeyCode {
		Picture pic;
		if (!_picsArchive.getPicture(picId, pic)) {
			warning("doSetup: PIC %u missing", (uint)picId);
			return Common::KEYCODE_INVALID;
		}
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		if (transparent) {
			// Preserve the current screen so the help PIC's
			// transparent pixels show the setup BG underneath.
			Graphics::Surface *cur = g_system->lockScreen();
			if (cur) {
				for (int row = 0; row < 200; row++)
					memcpy((byte *)scratch.getBasePtr(0, row),
						   (const byte *)cur->getBasePtr(0, row), 320);
				g_system->unlockScreen();
			}
			const byte transp = (byte)(pic.flags >> 8);
			// Explicit destPos — the no-destPos overload of
			// `transBlitFrom` (managed_surface.cpp:738) stretches
			// src to dst dimensions, scaling the PIC to 320x200.
			// The original `_Rect_Move_Mask` blits at native size.
			scratch.transBlitFrom(pic.surface, Common::Point(0, 0),
								  (uint32)transp);
		} else {
			scratch.clear();
			scratch.simpleBlitFrom(pic.surface);
		}
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
		while (!shouldQuit()) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
					return Common::KEYCODE_ESCAPE;
				if (ev.type == Common::EVENT_KEYDOWN)
					return ev.kbd.keycode;
				if (ev.type == Common::EVENT_LBUTTONDOWN)
					return Common::KEYCODE_INVALID;
			}
			g_system->delayMillis(15);
		}
		return Common::KEYCODE_ESCAPE;
	};

	auto leaveSetup = [&]() {
		// `_DoSetup`'s entry writes `_NextScreen = _LastScreen`. We
		// honor any handler that has already overridden `_nextScreen`
		// (Credits / Save don't, but New Case / Quit do). Otherwise
		// fall back to `_lastScreen`.
		if (_nextScreen == kScreenSetup) {
			_nextScreen = (ScreenId)_lastScreen;
			if (_nextScreen == kScreenSetup ||
				_nextScreen == kScreenInvalid)
				_nextScreen = kScreenMap;
		}
		saveProfile(_playerName);
	};

	_nextScreen = kScreenSetup;  // sentinel — leaveSetup picks the real target
	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_nextScreen = kScreenInvalid;
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE ||
					ev.kbd.keycode == Common::KEYCODE_RETURN) {
					leaveSetup();
					return;
				}
			}
			if (ev.type != Common::EVENT_LBUTTONDOWN)
				continue;
			const int mx = ev.mouse.x;
			const int my = ev.mouse.y;

			// Partner toggle (button [0]) — original has no symmetric
			// right-side button (the [3] rect is ScrapBook 1, not a
			// partner arrow). Direct clicks on the Jake/Jenny labels
			// are accepted as a more intuitive fallback.
			if (kPartnerBtn.contains(mx, my)) {
				_partner = _partner == 0 ? 1 : 0;
				dirty = true;
				continue;
			}
			if (kKid1Rect.contains(mx, my)) {
				if (_partner != 0) { _partner = 0; dirty = true; }
				continue;
			}
			if (kKid2Rect.contains(mx, my)) {
				if (_partner != 1) { _partner = 1; dirty = true; }
				continue;
			}

			// Voice toggle (button [1]).
			if (kVoiceBtn.contains(mx, my)) {
				_voiceOn = !_voiceOn;
				if (_audio)
					_audio->setVoiceEnabled(_voiceOn);
				dirty = true;
				continue;
			}
			if (kSoundOnRect.contains(mx, my)) {
				if (!_voiceOn) {
					_voiceOn = true;
					if (_audio)
						_audio->setVoiceEnabled(_voiceOn);
					dirty = true;
				}
				continue;
			}
			if (kSoundOffRect.contains(mx, my)) {
				if (_voiceOn) {
					_voiceOn = false;
					if (_audio)
						_audio->setVoiceEnabled(_voiceOn);
					dirty = true;
				}
				continue;
			}

			// New Case (button [7]). Original handler at 1f78:01ad
			// sets `_NextScreen = 0xa` (CHOOSE_MYSTERY) and exits the
			// dispatch loop with SI=1.
			if (kNewCaseBtn.contains(mx, my)) {
				saveProfile(_playerName);
				_nextScreen = kScreenChooseMystery;
				return;
			}

			// Save (button [6]). Original calls `_SaveGame @
			// 2404:0c87` and stays in the setup loop. Our save is
			// profile-scoped (one slot per player name) — same effect.
			if (kSaveBtn.contains(mx, my)) {
				saveProfile(_playerName);
				continue;
			}

			// Done (button [8]). Original handler is just `MOV SI,1;
			// JMP exit` — `_NextScreen` stays at whatever entry set it
			// to (= `_LastScreen`).
			if (kDoneBtn.contains(mx, my)) {
				leaveSetup();
				return;
			}

			// Quit (button [10]). Original: `_AreYouSure(0)` →
			// confirmed → `_NextScreen = 0xffff` (sentinel quit).
			if (kQuitBtn.contains(mx, my)) {
				if (areYouSure()) {
					_nextScreen = kScreenInvalid;
					return;
				}
				dirty = true;  // restore the BG after the prompt
				continue;
			}

			// Help (button [9]). Original `_InterfaceHelp(1) @
			// 1560:0205` walks the help-pic table at `29be:00c8`:
			// each `num` slot is 5 bytes — count + two u16 PIC IDs.
			// For num=1: count=2, pics = {0x0192, 0x01B1}. The
			// original blits each pic with `_Rect_Move_Mask` — a
			// MASKED blit whose transparent colour is the pic's
			// `miscflags >> 8`, so the setup BG shows through. It
			// also hides the cursor (`MOV [0x3a00], 0` + `_RemoveMouse`
			// at the top of `_InterfaceHelp`, 1560:0216-021c). ESC
			// at any point breaks out (1560:02b3 sets uVar5 = count).
			if (kHelpBtn.contains(mx, my)) {
				static const uint16 kHelp1Pics[] = { 0x0192, 0x01B1 };
				CursorMan.showMouse(false);
				for (uint i = 0; i < ARRAYSIZE(kHelp1Pics); i++) {
					// Re-render the setup BG before each help PIC so
					// each one overlays a clean canvas. Without this,
					// `showFullscreenPic`'s `lockScreen` snapshot would
					// pick up the previous PIC and the two help cards
					// would composite together. Mirrors the original's
					// `_vga_fvidvid(0)` call at the tail of every
					// `_InterfaceHelp` iteration (1560:02e5), which
					// restores the back-buffer BG between cards.
					draw();
					const Common::KeyCode k =
						showFullscreenPic(kHelp1Pics[i], /*transparent=*/true);
					if (k == Common::KEYCODE_ESCAPE)
						break;
				}
				CursorMan.showMouse(true);
				dirty = true;
				continue;
			}

			// Credits (button [11]). Original handler at 1f78:025a
			// loads PIC 0x208, hides the cursor (`MOV [0x3a00], 0`
			// at 1f78:0269 + `_RemoveMouse @ 1000:542f` at 1f78:026F),
			// blits it fullscreen via `_vga_fbuffvid` (raw copy, no
			// mask), then waits for any input.
			if (kCreditsBtn.contains(mx, my)) {
				CursorMan.showMouse(false);
				showFullscreenPic(0x208, /*transparent=*/false);
				CursorMan.showMouse(true);
				// PIC 0x208 has its own palette baked into the BG
				// dump via `_GetPicture`; the original restores via
				// `_GetPalette` on return. Reset to setup palette
				// (SITEPALS index 0) so the setup BG renders right.
				setSitePalette(0);
				dirty = true;
				continue;
			}

			// Profile (button [2]). Goes back to the profile picker
			// in the original (`_NextScreen = 8`). Treat the same way
			// as Done for now — switching profiles mid-game isn't
			// wired in our port and would discard mystery state.
			if (kProfileBtn.contains(mx, my)) {
				leaveSetup();
				return;
			}

			// ScrapBook 1 / 2 / 3 (buttons [3] / [4] / [5]). Original
			// handlers at 1f78:021F (`_ShowScrapbook(0, 1)`) /
			// 1f78:022E (gated chain >= 2 / `_ShowScrapbook(0, 2)`) /
			// 1f78:0244 (gated chain >= 3 / `_ShowScrapbook(0, 3)`).
			// Convert the original's `(0, stage)` invocation into our
			// `doShowScrapbook(stage)` (we collapse the param_1=0
			// "no-current-mystery" indirection — relevant only for
			// the post-win callsite).
			auto runScrapbook = [&](uint stage) {
				CursorMan.showMouse(false);
				doShowScrapbook(stage);
				CursorMan.showMouse(true);
				setSitePalette(0);
			};
			if (kScrap1Btn.contains(mx, my)) {
				runScrapbook(1);
				dirty = true;
				continue;
			}
			if (kScrap2Btn.contains(mx, my) && _chainStage >= 2) {
				runScrapbook(2);
				dirty = true;
				continue;
			}
			if (kScrap3Btn.contains(mx, my) && _chainStage >= 3) {
				runScrapbook(3);
				dirty = true;
				continue;
			}
		}
		if (dirty)
			draw();
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

void EEMEngine::doCaseSelection() {
	// Mirrors `_CaseSelection` @ 1c33:0a87. The original draws PIC 0x41
	// (chooser background) plus a centred "Book %d" / "Challenge Book"
	// header at (y=12) and then calls `_DoChoose(list)` to render the
	// menu via `DrawList` @ 1c33:040d at (_TextBox+3, DAT_29be_0d02) =
	// (61, 35), 12 rows × 10 px line height. The menu list itself is
	// the static array at 29be:0d6a (verified via `push 0x0d6a` at
	// 1c33:1ab4). Strings are at 29be:0ef4 onwards. Layout:
	//   list[0]  = "----------------------------------"
	//   list[1]  = "         Choose A Mystery"
	//   list[2..10] = alternating menu items + separators
	// Five selectable items: Choose A Mystery / Practice Mystery /
	// See ScrapBook 1/2/3.
	const uint kMaxMystery = 54;

	enum MenuPick {
		kPickChoose = 0,
		kPickPractice,
		kPickScrap1,
		kPickScrap2,
		kPickScrap3,
		kNumPicks
	};
	const char *kPickLabel[kNumPicks] = {
		"         Choose A Mystery",
		"         Practice Mystery",
		"         See ScrapBook 1",
		"         See ScrapBook 2",
		"         See ScrapBook 3"
	};
	// Menu entry gating per `_ActionScreen @ 1c33:195b` — the asm at
	// 1c33:19d1-1a70 sets greys[] based on chain stage AND per-tier
	// solve count:
	//   stage 1 → grey ScrapBook 2/3; grey ScrapBook 1 if no tier-1 solves
	//   stage 2 → grey Practice + ScrapBook 3; grey ScrapBook 2 if no tier-2 solves
	//   stage 3 → grey Practice; grey ScrapBook 3 if no tier-3 solves
	//   stage 4 → grey Choose + Practice (post-completion read-only state)
	// In other words: each tier's ScrapBook unlocks as soon as you've
	// solved your first case in that tier. Practice Mystery is only
	// available at stage 1. Choose A Mystery is greyed once every case
	// in every tier is solved (stage 4).
	bool anySolved1 = false;
	for (uint i = 1; i <= 0x18 && i < sizeof(_mysteriesSolved); i++)
		if (_mysteriesSolved[i]) { anySolved1 = true; break; }
	bool anySolved2 = false;
	for (uint i = 0x19; i <= 0x30 && i < sizeof(_mysteriesSolved); i++)
		if (_mysteriesSolved[i]) { anySolved2 = true; break; }
	bool anySolved3 = false;
	for (uint i = 0x31; i <= 0x36 && i < sizeof(_mysteriesSolved); i++)
		if (_mysteriesSolved[i]) { anySolved3 = true; break; }

	const bool chooseOn   = _chainStage < 4;
	const bool practiceOn = _chainStage <= 1;
	const bool scrap1On =
		_chainStage >= 2 || (_chainStage == 1 && anySolved1);
	const bool scrap2On =
		_chainStage >= 3 || (_chainStage == 2 && anySolved2);
	const bool scrap3On =
		_chainStage >= 4 || (_chainStage == 3 && anySolved3);
	const bool kPickEnabled[kNumPicks] = {
		chooseOn, practiceOn, scrap1On, scrap2On, scrap3On
	};
	// Seed selection on the first enabled entry — at stage 4 the
	// `Choose A Mystery` default is greyed, so we land on ScrapBook 1.
	uint pick = 0;
	for (uint i = 0; i < kNumPicks; i++) {
		if (kPickEnabled[i]) { pick = i; break; }
	}

	const char *kSeparator = "----------------------------------";

	// Click rectangles from the original `_DoChoose` @ 1c33:0514 — each
	// `_InRect(_MouseX, _MouseY, addr, 0x29be)` reads one 4×u16 rect at
	// the listed offset in segment 29be ({x1, y1, x2, y2}). We use
	// `Common::Rect` (left/top/right/bottom) which also gives us
	// `contains(x, y)` for hit testing.
	const Common::Rect kOkRect      ( 12,  63,  41,  87); // 29be:0cd8 confirm
	const Common::Rect kHelpRect    ( 12, 100,  41, 124); // 29be:0ce0 help
	const Common::Rect kExitRect    ( 12, 137,  41, 161); // 29be:0ce8 cancel
	const Common::Rect kUpArrowRect (240,  31, 250,  43); // 29be:0cf0 scroll up
	const Common::Rect kDnArrowRect (240, 148, 250, 159); // 29be:0cf8 scroll dn
	const Common::Rect kListRect    ( 58,  35, 238, 158); // 29be:0d00 list panel

	// The original `_NewPlayer` set `_MouseCursor = 1` on exit; the
	// chain of screens after it expects the cursor to stay visible.
	// Reassert here in case anything between hid it.
	CursorMan.showMouse(true);

	// Reassert site palette 0 (the case-selection / chooser CLUT). In
	// the normal flow `doProfilePicker` (or the post-screen reset paths
	// at lines 1402 / 1147 / 1121) leaves us on palette 0 already, but
	// the launcher-resume path jumps straight here from `_AllBlack`
	// (palette = all-zero) — without this the BG renders into a black
	// CLUT and the player sees an empty screen.
	setSitePalette(0);

	// Mirrors `_CaseSelection`: load PIC 0x41 as the chooser backdrop.
	Picture caseBg;
	const bool haveCaseBg = _picsArchive.getPicture(0x41, caseBg);

	// KD greeter sprite. `_CaseSelection @ 1c33:0a87` (1c33:0b7e-0ba1)
	// loads anim 0x15 (Jake-paired) or 0x16 (Jenny-paired) and registers
	// `_NewAnimation(0x112, 0x50, ..., seqnum=0x15, prior=1)` — partner-
	// dependent because the host KD changes who's "with him" on the
	// briefing intro frame. Runs continuously through the menu loop via
	// `_UpdateAnimations`. We approximate with millis-based frame cycling.
	const uint kKdAniId = (_partner == 0) ? 0x15 : 0x16;
	Animation kdAnim;
	const bool haveKdAnim = _aniArchive.loadAnimation(kKdAniId, kdAnim)
							 && !kdAnim.empty();
	const int kKdAnimX = 0x112;
	const int kKdAnimY = 0x50;

	CaseSelectionView v;
	v.vm = this;
	v.caseBg = &caseBg;
	v.haveCaseBg = haveCaseBg;
	v.kdAnim = &kdAnim;
	v.haveKdAnim = haveKdAnim;
	v.kdAnimId = (uint16)kKdAniId;
	v.kdAnimX = kKdAnimX;
	v.kdAnimY = kKdAnimY;
	v.separator = kSeparator;
	v.pickLabel = kPickLabel;
	v.pickEnabled = kPickEnabled;
	v.pick = pick;

	drawCaseSelectionFrame(v);
	uint32 lastTick = g_system->getMillis();

	bool exitChosen = false;
	while (!shouldQuit()) {
		Common::Event ev;
		bool confirmed = false;
		// Redraw every 100 ms so the KD greeter cycles. Mirrors the
		// `_CheckFrameRate` cadence in `_CaseSelection`'s main loop.
		const uint32 now = g_system->getMillis();
		if (haveKdAnim && now - lastTick >= 100) {
			lastTick = now;
			v.pick = pick;
			drawCaseSelectionFrame(v);
		}
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// OK / EXIT / HELP buttons (rectangles from `_DoChoose`).
				if (kOkRect.contains(ev.mouse.x, ev.mouse.y)) {
					// Greyed entries can't be confirmed (mirrors
					// `_DoChoose @ 1c33:0635` — clicks on a `_Greys[i]
					// != 0` row are ignored before `select` is set).
					if (kPickEnabled[pick])
						confirmed = true;
					break;
				}
				if (kExitRect.contains(ev.mouse.x, ev.mouse.y)) {
					exitChosen = true;
					confirmed = true;
					break;
				}
				if (kHelpRect.contains(ev.mouse.x, ev.mouse.y)) {
					// HELP placeholder — original calls `_DisplayHint`;
					// our help screen is wired to `H` later in the flow.
					continue;
				}
				// List panel: click on a non-separator row selects the
				// menu entry under the cursor.
				if (kListRect.contains(ev.mouse.x, ev.mouse.y)) {
					const int kLineH = 10;
					const int row = (ev.mouse.y - kListRect.top) / kLineH;
					if ((row & 1) == 1) {
						const uint mp = (uint)(row >> 1);
						if (mp < kNumPicks && kPickEnabled[mp]) {
							pick = mp;
							v.pick = pick;
							drawCaseSelectionFrame(v);
							continue;
						}
					}
				}
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				exitChosen = true;
				confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_RETURN ||
				k == Common::KEYCODE_KP_ENTER) {
				if (kPickEnabled[pick])
					confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_UP || k == Common::KEYCODE_LEFT) {
				// Cycle backwards through enabled picks (mirrors the
				// `_DoChoose` arrow handlers @ 1c33:0514). Loop is
				// bounded by `kNumPicks` so a row of all-disabled picks
				// can't spin forever.
				for (int i = 0; i < (int)kNumPicks; i++) {
					pick = (pick == 0) ? (uint)(kNumPicks - 1) : pick - 1;
					if (kPickEnabled[pick])
						break;
				}
				v.pick = pick;
				drawCaseSelectionFrame(v);
				continue;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_RIGHT ||
				k == Common::KEYCODE_TAB) {
				for (int i = 0; i < (int)kNumPicks; i++) {
					pick = (pick + 1) % kNumPicks;
					if (kPickEnabled[pick])
						break;
				}
				v.pick = pick;
				drawCaseSelectionFrame(v);
				continue;
			}
		}
		if (confirmed) {
			v.pick = pick;
			drawCaseSelectionFrame(v);
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (shouldQuit())
		return;

	if (exitChosen) {
		_mystery.clear();
		_nextScreen = kScreenInvalid;
		return;
	}

	// "Practice Mystery" is the tutorial → mystery 0.
	if (pick == kPickPractice) {
		if (!_mystery.load(0, &_rng)) {
			warning("doCaseSelection: failed to load practice mystery");
			_mystery.clear();
		} else if (_audio && !isFloppy()) {
			_audio->initMysterySounds(0);
		}
		return;
	}

	if (pick == kPickScrap1 || pick == kPickScrap2 || pick == kPickScrap3) {
		// `_ActionScreen` handlers at 1c33:1B13 / 1B26 / 1B40 each
		// call `_ShowScrapbook(0, stage)` for the matching tier
		// (verified at the action-handler jumptable bytes
		// `01 03 05 07 09 ff` paired with handlers at 1c33:1be1).
		// The picker here is meant to LEAVE the mystery state untouched
		// — viewing the scrapbook never starts a new case.
		const uint stage = (pick == kPickScrap1) ? 1
						 : (pick == kPickScrap2) ? 2 : 3;
		CursorMan.showMouse(false);
		doShowScrapbook(stage);
		CursorMan.showMouse(true);
		setSitePalette(0);
		_mystery.clear();
		return;
	}

	// "Choose A Mystery" sub-screen: pick a specific case from the
	// chain-stage's roster. Mirrors `_DoChooseMystery @ 1a35:02b7` +
	// `_CaseSelection @ 1c33:0a87`:
	//   stage 1 (Junior, BOOK1.NME) → mysteries  1..24
	//   stage 2 (Senior, BOOK2.NME) → mysteries 25..48
	//   stage 3 (Master, BOOK3.NME) → mysteries 49..54
	// `_DoChooseMystery` opens BOOK<stage>.NME and reads up to 25
	// CRLF-terminated lines into a 25-entry FAR-pointer array passed
	// to `_CaseSelection`. The grey mask `_Greys = &mysteriesSolved +
	// stageLo` (1c33:0b22) makes already-solved entries unselectable.
	uint stageLo = 1, stageHi = 0x18;
	uint book = 1;
	switch (_chainStage) {
	case 2: stageLo = 0x19; stageHi = 0x30; book = 2; break;
	case 3: stageLo = 0x31; stageHi = 0x36; book = 3; break;
	default: break;  // stage 1 (or fallback)
	}
	if (stageHi > kMaxMystery)
		stageHi = kMaxMystery;

	const Common::StringArray names = loadBookNames(book);
	if (names.empty()) {
		warning("doCaseSelection: BOOK%u.NME failed to load — bailing",
				book);
		_mystery.clear();
		return;
	}
	const uint listLen = MIN<uint>((uint)names.size(), stageHi - stageLo + 1);

	// Per-row solved flags. `_DoChoose @ 1c33:0521` skips solved entries
	// when seeding the initial selection (`while *_Greys[select] != 0`)
	// and again per-click via the same mask check.
	Common::Array<bool> solvedFlags;
	solvedFlags.resize(listLen);
	for (uint i = 0; i < listLen; i++) {
		const uint mn = stageLo + i;
		solvedFlags[i] =
			mn < sizeof(_mysteriesSolved) && _mysteriesSolved[mn] != 0;
	}

	// Seed the selection at the first unsolved entry — same as
	// `_DoChoose`'s `while (*Greys[select] != 0) select++;` loop at
	// 1c33:0524.
	uint selRow = 0;
	while (selRow < listLen && solvedFlags[selRow])
		selRow++;
	if (selRow >= listLen)
		selRow = 0;  // every case solved — let player re-pick
	uint topRow = 0;
	const uint kVisible = 12;
	if (selRow >= kVisible) {
		topRow = selRow - kVisible / 2;
		if (topRow + kVisible > listLen)
			topRow = listLen > kVisible ? listLen - kVisible : 0;
	}

	auto clampTopRow = [&](uint &t) {
		if (listLen <= kVisible) {
			t = 0;
			return;
		}
		const uint maxTop = listLen - kVisible;
		if (t > maxTop)
			t = maxTop;
	};

	CaseSubmenuView sv;
	sv.vm = this;
	sv.caseBg = &caseBg;
	sv.haveCaseBg = haveCaseBg;
	sv.names = &names;
	sv.solvedFlags = &solvedFlags;
	sv.topRow = topRow;
	sv.selRow = selRow;
	sv.book = book;

	drawCaseSubmenu(sv);
	bool confirmed = false;
	while (!confirmed && !shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				if (kOkRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (selRow < listLen && !solvedFlags[selRow])
						confirmed = true;
					break;
				}
				if (kExitRect.contains(ev.mouse.x, ev.mouse.y)) {
					_mystery.clear();
					return;
				}
				if (kUpArrowRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (topRow > 0) { topRow--; dirty = true; }
					continue;
				}
				if (kDnArrowRect.contains(ev.mouse.x, ev.mouse.y)) {
					topRow++;
					clampTopRow(topRow);
					dirty = true;
					continue;
				}
				if (kListRect.contains(ev.mouse.x, ev.mouse.y)) {
					// Pick the row under the cursor — mirrors
					// 1c33:0635 `i = (MouseY - DAT_29be_0d02) / 10;`.
					const int kLineH = 10;
					const int row = (ev.mouse.y - 35) / kLineH;
					if (row < 0 || row >= (int)kVisible)
						continue;
					const uint idx = topRow + (uint)row;
					if (idx >= listLen)
						continue;
					if (solvedFlags[idx])
						continue;  // greyed entries ignore clicks
					selRow = idx;
					dirty = true;
					continue;
				}
				continue;
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				_mystery.clear();
				return;
			}
			if (k == Common::KEYCODE_RETURN ||
				k == Common::KEYCODE_KP_ENTER) {
				if (selRow < listLen && !solvedFlags[selRow])
					confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_TAB) {
				if (selRow + 1 < listLen) {
					selRow++;
					if (selRow >= topRow + kVisible) {
						topRow = selRow - kVisible + 1;
						clampTopRow(topRow);
					}
					dirty = true;
				}
				continue;
			}
			if (k == Common::KEYCODE_UP) {
				if (selRow > 0) {
					selRow--;
					if (selRow < topRow)
						topRow = selRow;
					dirty = true;
				}
				continue;
			}
			if (k == Common::KEYCODE_PAGEDOWN) {
				selRow = MIN<uint>(selRow + kVisible, listLen - 1);
				if (selRow >= topRow + kVisible) {
					topRow = selRow - kVisible + 1;
					clampTopRow(topRow);
				}
				dirty = true;
				continue;
			}
			if (k == Common::KEYCODE_PAGEUP) {
				selRow = (selRow >= kVisible) ? selRow - kVisible : 0;
				if (selRow < topRow)
					topRow = selRow;
				dirty = true;
				continue;
			}
			if (k == Common::KEYCODE_HOME) {
				selRow = 0;
				topRow = 0;
				dirty = true;
				continue;
			}
			if (k == Common::KEYCODE_END) {
				selRow = listLen - 1;
				topRow = listLen > kVisible ? listLen - kVisible : 0;
				dirty = true;
				continue;
			}
		}
		if (dirty) {
			sv.topRow = topRow;
			sv.selRow = selRow;
			drawCaseSubmenu(sv);
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	const uint mn = stageLo + selRow;
	if (!_mystery.load(mn, &_rng)) {
		warning("doCaseSelection: failed to load mystery %u", mn);
		_mystery.clear();
		return;
	}
	if (_audio && !isFloppy())
		_audio->initMysterySounds(mn);
	debugC(1, kDebugMystery, "Mystery %u loaded; %u sites, %u suspects",
		   mn, _mystery.numSites(), _mystery.numSuspects());
}

void EEMEngine::doNotebook() {
	// Mirrors `_DoNotebook @ 161e:0500` + `_DrawNotes @ 161e:01d0` +
	// `_HandleNoteButton @ 161e:03cb`.
	//
	// Layout (verified from Ghidra labels in 29be:013f / 29be:0147):
	//   _NotebookRect = (78, 12, 288, 152)   — note display rectangle.
	//   _NoteButtons (11 entries, 8 bytes each, at 29be:0147):
	//     [0]  (134, 174, 155, 190)  decorative — `_HandleNoteButton(0)`
	//                                returns immediately (i-1 unsigned > 9).
	//     [1]  (93,  174, 115, 190)  → `_InterfaceHelp(0)` (handler 0x3f9)
	//     [2]  (157, 174, 178, 190)  → handler 0x477   (page nav)
	//     [3]  (5,   80,  44, 110)   → `_KDHelp` (host hint, 0x403)
	//     [4]  (180, 174, 201, 190)  → solve / accuse  (0x436)
	//     [5]  (204, 174, 224, 190)  → `_NextScreen = 5` (gallery, 0x489)
	//     [6]  (226, 174, 247, 190)  → handler 0x4ab
	//     [7]  (7,   177,  57, 200)  → handler 0x480   (back to map)
	//     [8]  (35,  111,  56, 136)  → `_NextScreen = 3` (site)
	//     [9]  (0, 0, 0, 0)          → same exit as [8]
	//     [10] (66,  79, 267, 174)   → `_InterfaceHelp(0)` (note area)
	//   Background: PIC 0x3f.
	//   Partner anim: anim 1 (Jake) / 0xb (Jenny) at (5, 80).
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	// Button rects from `_NoteButtons @ 29be:0147` matched to handler
	// addresses via the jump table at `_HandleNoteButton + 0xec` (i.e.
	// 161e:04ec). Decoded handlers (i = rect_index, dispatch = handler[i-1]):
	//   rect 0 (134,155) → no handler (i-1 underflows; original treats
	//                      this as a decorative/no-op slot)
	//   rect 1 (93,115)  → 0x03f9 = `_InterfaceHelp(0)`           (HELP)
	//   rect 2 (157,178) → 0x0477 = `_NextScreen = 5`             (GALLERY)
	//   rect 3 (5,80)    → 0x0403 = `_KDHelp`                     (host hint)
	//   rect 4 (180,201) → 0x0436 = `_SolvedCheck` -> NextScreen=7 (SOLVE)
	//   rect 5 (204,224) → 0x0489 = `_EraseNotes` + `_DrawNotes`  (PAGE NEXT)
	//   rect 6 (226,247) → 0x04ab = decrement CurrentPage + redraw (PAGE PREV)
	//   rect 7 (7,177)   → 0x0480 = `_NextScreen = 2`             (MAP)
	//   rect 8 (35,111)  → 0x03ed = `_NextScreen = 3`             (SITE)
	//   rect 9 (0,0)     → 0x03ed = same as rect 8
	//   rect 10 (66,79)  → 0x03f9 = `_InterfaceHelp(0)`           (note-area help)
	const Common::Rect kBtnHelp1   ( 93, 174, 115, 190);  // [1] HELP
	const Common::Rect kBtnGallery (157, 174, 178, 190);  // [2] GALLERY
	const Common::Rect kBtnPartner (  5,  80,  44, 110);  // [3] KD HELP
	const Common::Rect kBtnAccuse  (180, 174, 201, 190);  // [4] SOLVE
	const Common::Rect kBtnPageNext(204, 174, 224, 190);  // [5] PAGE NEXT
	const Common::Rect kBtnPagePrev(226, 174, 247, 190);  // [6] PAGE PREV
	const Common::Rect kBtnMap     (  7, 177,  57, 200);  // [7] MAP
	const Common::Rect kBtnSite    ( 35, 111,  56, 136);  // [8] SITE
	const Common::Rect kBtnHelp2   (267, 174, 288, 190);  // [10] extra HELP
	// (`_NoteButtons @ 29be:0147` actually has rect [10] at
	// (267,174,288,190) — small button on the right of the bottom
	// bar that the original handler dispatch table at 161e:04ec
	// routes to `_InterfaceHelp(0)` again. Earlier this rect was
	// mis-noted as a "note area" of (66,79,267,174) — that
	// rectangle exists nowhere in the binary's button table.)

	CursorMan.showMouse(true);

	int page = 0;
	int hoveredNoteSlot = -1;
	(void)hoveredNoteSlot;

	drawNotebookFrame(page);

	uint32 lastDraw = g_system->getMillis();

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		bool exitFlag = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				exitFlag = true;
				break;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					exitFlag = true;
					break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_LEFT ||
					ev.kbd.keycode == Common::KEYCODE_PAGEUP) {
					if (page > 0)
						page--;
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT ||
						   ev.kbd.keycode == Common::KEYCODE_PAGEDOWN ||
						   ev.kbd.keycode == Common::KEYCODE_TAB) {
					page++;
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Test buttons in the order the original would —
				// button 0 / 9 are dead zones, so check the actionable
				// rects directly. Earlier rects "win" when overlapping
				// (matches `_FindButton`).
				if (kBtnSite.contains(ev.mouse.x, ev.mouse.y)) {
					exitFlag = true;
					break;  // back to site
				}
				if (kBtnMap.contains(ev.mouse.x, ev.mouse.y)) {
					doBigMap();
					exitFlag = true;
					break;
				}
				if (kBtnPartner.contains(ev.mouse.x, ev.mouse.y)) {
					doHelp();              // _KDHelp = host hint
					dirty = true;
					continue;
				}
				if (kBtnAccuse.contains(ev.mouse.x, ev.mouse.y)) {
					doAccuse();
					exitFlag = true;
					break;
				}
				if (kBtnGallery.contains(ev.mouse.x, ev.mouse.y)) {
					doGallery();
					dirty = true;
					continue;
				}
				if (kBtnHelp1.contains(ev.mouse.x, ev.mouse.y)) {
					// rect 1 → `_InterfaceHelp(0)`: walks `HelpData[0]` and
					// blits PICs 0x63 / 0x1ae fullscreen for click-through.
					doInterfaceHelp(0);
					dirty = true;
					continue;
				}
				if (kBtnPagePrev.contains(ev.mouse.x, ev.mouse.y)) {
					if (page > 0)
						page--;
					dirty = true;
					continue;
				}
				if (kBtnPageNext.contains(ev.mouse.x, ev.mouse.y)) {
					page++;
					dirty = true;
					continue;
				}
				if (kBtnHelp2.contains(ev.mouse.x, ev.mouse.y)) {
					// `_NoteButtons[10]` → handler 0x03f9 = same
					// `_InterfaceHelp(0)` as button [1].
					doInterfaceHelp(0);
					dirty = true;
					continue;
				}
				// Click on a clue's slot rect → toggle selection. The
				// original `_DoNotebook` doesn't do this — note
				// selection lives in the accuse screen there — but
				// keyboard 1..9 toggling is awkward, and the resulting
				// `_NoteSelected` state is what `_SolvedCheck` reads
				// either way. Slot rects are the per-clue rectangles
				// `drawNotebookFrame` publishes, so this just
				// reproduces the visible-text-bbox click without the
				// previous bogus outer-area gate.
				for (uint i = 0; i < _notebookSlotRects.size(); i++) {
					if (_notebookSlotRects[i].contains(ev.mouse.x,
													   ev.mouse.y)) {
						const uint clueId = _notebookSlotClues[i];
						_mystery._noteSelected[clueId] ^= 1;
						dirty = true;
						break;
					}
				}
			}
		}
		if (exitFlag)
			break;

		const uint32 now = g_system->getMillis();
		// Re-render every 100 ms so the partner sprite cycles frames.
		if (dirty || now - lastDraw >= 100) {
			drawNotebookFrame(page);
			lastDraw = now;
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

void EEMEngine::drawNotebookFrame(int &page) {
	// PDA notebook redraw — formerly the `draw` lambda inside `doNotebook`.
	// Mirrors `_DrawNotes @ 161e:01d0` for the per-page note layout, plus
	// the partner-sprite blit at (5, 80) (`_NewAnimation` from
	// `_DoNotebook @ 161e:0500`). Uses `_notebookSlotRects` /
	// `_notebookSlotClues` to publish the per-page slot layout to the
	// click handler in `doNotebook`.
	const Common::Rect kNotebookRect(78, 12, 288, 152);

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	// PIC 0x3f frame.
	Picture frame;
	if (_picsArchive.getPicture(0x3f, frame))
		scratch.simpleBlitFrom(frame.surface);

	// Partner sprite at (5, 80). Anim 1 for Jake, 0xb (11) for Jenny
	// for CELLS, but the original `_DoNotebook @ 161e:0500` always
	// uses script 0x01 (verified by `CONCAT22(1, ...)` in its
	// `_NewAnimation` call at 161e:054c). Both 0x01 and 0x0b have
	// the SAME script in `kAnimScripts` (alias), so both lookups
	// produce identical results — but routing through 0x01
	// matches the original verbatim.
	const uint partnerAnim = (_partner == 0) ? 1 : 0xb;
	Animation partnerAni;
	if (_aniArchive.loadAnimation(partnerAnim, partnerAni) && !partnerAni.empty()) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = partnerFrameAtTick(0x01,
												  (uint)partnerAni.size(), now);
		// Anchor-aware blit. The PDA partner (anim 0x01/0x0b) cells
		// have miscflags = rowoff = 0 in the audit, but routing
		// through `blitAnimFrameAnchored` is harmless and keeps the
		// rendering path consistent with the BigMap partner.
		blitAnimFrameAnchored(scratch.surfacePtr(),
							  partnerAni[frameIdx], 5, 80);
	}

	// Notes — `_DrawNotes` walks `_NoteIndex` for the current page,
	// rendering each found clue's text inside `_NotebookRect` with
	// word-wrap. Selected clues are highlighted (color 0x3c in the
	// original's case-briefing palette).
	// Build a list of found-clue indices, identical ordering to the
	// original's iteration through `_CluesFound[]`.
	Common::Array<uint> found;
	for (uint i = 0; i < Mystery::kCluesFoundCap; i++) {
		if (_mystery._cluesFound[i])
			found.push_back(i);
	}
	const byte *ni = _mystery.noteIndex();
	const uint16 niCount = _mystery.noteIndexCount();

	const int kRectX = kNotebookRect.left;
	const int kRectY = kNotebookRect.top;
	const int kRectW = kNotebookRect.width();
	const int kRectH = kNotebookRect.height();

	// Walk forward to the start clue of the current page.
	// Each page renders as many clues as fit in `kRectH`.
	int clueCursor = 0;
	Common::Array<int> pageStarts;
	pageStarts.push_back(0);
	// Floppy NoteIndex entries are 7 bytes (`u16 ?; u16 jakeOff; u16
	// jennyOff; u8 score`) with ABSOLUTE byte offsets into the mystery
	// blob, while CD entries are 4 bytes with offsets relative to the
	// TextBlock at header[+0xc]. Resolve the right text for the active
	// partner / variant once per render.
	const bool floppyNb = isFloppy();
	const byte *bufBase = _mystery.blobAt(0);
	const uint32 mysSz  = _mystery.dataSize();
	auto noteText = [&](uint clueId) -> Common::String {
		if (!ni || clueId >= niCount)
			return Common::String();
		if (floppyNb && bufBase) {
			const uint stride = 7;
			const uint16 textOff = (_partner == 0)
				? READ_LE_UINT16(ni + clueId * stride + 2)
				: READ_LE_UINT16(ni + clueId * stride + 4);
			if (textOff == 0 || textOff >= mysSz)
				return Common::String();
			const char *p = (const char *)(bufBase + textOff);
			uint32 len = 0;
			while (textOff + len < mysSz && p[len] != 0)
				len++;
			return parseString(Common::String(p, len),
							   _playerName, _partner);
		}
		const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
		return parseString(_mystery.textAt(textOff),
						   _playerName, _partner);
	};
	{
		const int lineH = _font.getFontHeight() + 1;
		int y = kRectY;
		while (clueCursor < (int)found.size()) {
			const uint clueId = found[clueCursor];
			Common::String txt = noteText(clueId);
			// Measure height by wrapping the text without drawing.
			Common::Array<Common::String> wrapped;
			_font.wordWrapText(txt, kRectW, wrapped);
			const int h = (int)wrapped.size() * lineH;
			if (y + h + 7 > kRectY + kRectH) {
				// Page break before this clue.
				y = kRectY;
				pageStarts.push_back(clueCursor);
			}
			y += h + 7;
			clueCursor++;
		}
		if (page >= (int)pageStarts.size())
			page = (int)pageStarts.size() - 1;
		if (page < 0)
			page = 0;
	}

	// Track per-slot rectangles so the click handler can map a
	// click in `kNoteArea` back to a clue index.
	Common::Array<Common::Rect> slotRects;
	Common::Array<uint> slotClues;

	const int startClue = (page < (int)pageStarts.size())
							? pageStarts[page] : 0;
	const int endClue   = (page + 1 < (int)pageStarts.size())
							? pageStarts[page + 1] : (int)found.size();

	int y = kRectY;
	for (int i = startClue; i < endClue; i++) {
		const uint clueId = found[i];
		Common::String txt = noteText(clueId);
		if (txt.empty())
			txt = Common::String::format("clue %u", clueId);
		// Per `_DrawNotes @ 161e:01d0`: text uses
		// `_NoteUnselectedColor` (0x5c=cyan) for unselected and 0x3c
		// (light yellow-white) for selected. Both contrast cleanly
		// with the PDA screen's natural blue, so we draw text
		// directly on PIC 0x3f without an extra fill rectangle —
		// matches the original design.
		Common::Array<Common::String> wrapped;
		_font.wordWrapText(txt, kRectW, wrapped);
		const int lineH = _font.getFontHeight() + 1;
		const int h = (int)wrapped.size() * lineH;
		const byte color = _mystery._noteSelected[clueId] ? 0x3C : 0x5C;
		for (uint li = 0; li < wrapped.size(); li++) {
			_font.drawString(&scratch, wrapped[li], kRectX,
							 y + (int)li * lineH, kRectW, color);
		}
		slotRects.push_back(Common::Rect(kRectX, y,
										  kRectX + kRectW, y + h));
		slotClues.push_back(clueId);
		y += h + 7;
	}

	// Page indicator only — the original `_DrawNotes @ 161e:01d0`
	// has no points display in the PDA notebook (the per-clue point
	// values are SPOILERS for the chain weighting that the engine
	// uses internally for `_GetSelectedPoints`). Showing the running
	// total tells the player exactly when they have enough evidence
	// to solve, which deflates the deduction step.
	_font.drawString(&scratch, Common::String::format("p%d/%d",
							   page + 1, (int)pageStarts.size()),
					 270, 4, 320, 0x5C);

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();

	// Publish slot info to `doNotebook`'s click handler.
	_notebookSlotRects = slotRects;
	_notebookSlotClues = slotClues;
}

void EEMEngine::doGallery() {
	// Mirrors `_DoGallery @ 158f:065b` and `_DrawGallery @ 158f:0046`.
	// Verified directly from the disassembly:
	//   * Background: PIC 0x3f (same as PDA).
	//   * Partner sprite at (5, 0x50): anim 2 (Jake) / 0x10 (Jenny).
	//     `_NewAnimation(5, 0x50, ...)`. NOTE: gallery uses anim 2/0x10,
	//     PDA uses 1/0xb — different sprites.
	//   * Five fixed slot positions at `29be:0x116` (4 bytes per slot,
	//     `{u16 x, u16 y}`):
	//         slot 0 = ( 83,  14)   slot 3 = (119,  90)
	//         slot 1 = (155,  14)   slot 4 = (191,  90)
	//         slot 2 = (227,  14)
	//   * For each logical suspect i in 0..NumSuspects-1:
	//         picId   = `*(u16 *)(_GalleryData + i * 0x46)` (entry +0).
	//         visible = `_InGallery[_NewOrder[i]] != 0`.
	//         drawX   = positions[_NewOrder[i]].x
	//         drawY   = positions[_NewOrder[i]].y + (0x48 - pic.height)
	//     So portraits are BOTTOM-aligned to baselines 0x48 + pos.y.
	//   * Click on portrait via `_SearchSuspects` → `MoreInfo(i)` shows
	//     the suspect detail page. ESC returns to PDA.
	//   * Frame-cycled @ 100ms via `_CheckFrameRate` + `_UpdateAnimations`
	//     + `_GizmoColorCycle`.
	if (!_mystery.isLoaded())
		return;

	const byte *gd = _mystery.galleryData();
	if (!gd) {
		warning("doGallery: no GalleryData in mystery %u", _mystery.number());
		return;
	}

	CursorMan.showMouse(true);

	// Pre-load PIC 0x3f for the MoreInfo backdrop blit further down.
	// (`drawGalleryFrame` reloads it on its own per-call too.)
	Picture galBg;
	const bool haveBg = _picsArchive.getPicture(0x3f, galBg);

	const uint8 num = _mystery.numSuspects();

	// Cache slot rects for click hit-testing.
	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect; // logical suspect index in [0, num)
	slotRects.resize(num);
	slotSuspect.resize(num);
	for (uint i = 0; i < num; i++) {
		slotSuspect[i] = -1;
	}

	drawGalleryFrame(gd, num, slotRects, slotSuspect);
	uint32 lastDraw = g_system->getMillis();

	while (!shouldQuit()) {
		Common::Event ev;
		bool exitFlag = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					exitFlag = true;
					break;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// PDA bottom-bar buttons mirror `_NoteButtons @ 29be:0147`.
				// `_DoGallery @ 158f:065b` shares the SAME button table
				// with `_DoNotebook` (both call `_FindButton` against the
				// 11-entry table at 0x147). `_HandleGalleryButton @
				// 158f:05c0` dispatches via a different jump table
				// (158f:0645). Verified gallery button mapping:
				//   rect 0 (134,155) → 0x05ef = `_NextScreen = 4` (NOTEBOOK)
				//   rect 1 (93,115)  → 0x0625 = `_InterfaceHelp` (HELP)
				//   rect 2 (157,178) → 0x0638 = generic exit (no-op)
				//   rect 3 (5,80)    → 0x061e = `_KDHelp` (host hint)
				//   rect 4 (180,201) → 0x05ff = `_SolvedCheck` -> SOLVE
				//   rect 5 (204,224) → 0x0638 = generic exit
				//   rect 6 (226,247) → 0x0638 = generic exit
				//   rect 7 (7,177)   → 0x05f7 = `_NextScreen = 2` (MAP)
				//   rect 8 (35,111)  → 0x05e4 = `_NextScreen = 3` (SITE)
				const Common::Rect kBtnSite    ( 35, 111,  56, 136); // [8] SITE
				const Common::Rect kBtnMap     (  7, 177,  57, 200); // [7] MAP
				const Common::Rect kBtnAccuse  (180, 174, 201, 190); // [4] SOLVE
				const Common::Rect kBtnNotebook(134, 174, 155, 190); // [0] NOTEBOOK (back to PDA notes)
				const Common::Rect kBtnHelp    ( 93, 174, 115, 190); // [1] HELP
				const Common::Rect kBtnPartner (  5,  80,  44, 110); // [3] KD HELP
				if (kBtnSite.contains(ev.mouse.x, ev.mouse.y)) {
					exitFlag = true; break;
				}
				if (kBtnMap.contains(ev.mouse.x, ev.mouse.y)) {
					doBigMap();
					exitFlag = true; break;
				}
				if (kBtnAccuse.contains(ev.mouse.x, ev.mouse.y)) {
					doAccuse();
					exitFlag = true; break;
				}
				if (kBtnNotebook.contains(ev.mouse.x, ev.mouse.y)) {
					// Already came from notebook; exiting returns to it.
					exitFlag = true; break;
				}
				if (kBtnHelp.contains(ev.mouse.x, ev.mouse.y)) {
					// Gallery rect 1 → `_InterfaceHelp(0)` per jmp table at
					// 158f:0625 (HandleGalleryButton). Same picture sequence
					// as the notebook HELP button.
					doInterfaceHelp(0);
					lastDraw = 0;
					continue;
				}
				if (kBtnPartner.contains(ev.mouse.x, ev.mouse.y)) {
					doHelp();
					lastDraw = 0;
					continue;
				}
				// `_SearchSuspects` walks the per-slot rects and returns
				// the suspect index. We mirror that with cached rects.
				bool clicked = false;
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] < 0)
						continue;
					if (slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						// `MoreInfo(i)` — show the suspect detail page.
						// Mirrors `MoreInfo @ 158f:0419`:
						//   _RefreshGalleryBackground();
						//   _GetPicture(*(u16*)(gd + i*0x46));
						//   _AddPicBackground(pic, 0x94, 0xf);
						//   _DrawGalleryNotes(gd + i*0x46);
						//   loop until ESC or button click.
						// Suspect data layout (verified against M1):
						//   +0..1: picId (used here AND for gallery slot)
						//   +8..9: number of clues for this suspect
						//   +0xa..??: array of u16 clue IDs (terminated
						//             by 0xFFFF if shorter than count).
						const uint suspectIdx = (uint)slotSuspect[i];
						const byte *suspect = gd + suspectIdx * 0x46;
						const uint16 detailPic =
							READ_LE_UINT16(suspect + 0);
						const uint16 clueCount =
							READ_LE_UINT16(suspect + 8);

						Graphics::ManagedSurface ms(320, 200,
							Graphics::PixelFormat::createFormatCLUT8());
						ms.clear();
						if (haveBg) {
							const int bw = MIN<int>(galBg.surface.w, 320);
							const int bh = MIN<int>(galBg.surface.h, 200);
							for (int row = 0; row < bh; row++) {
								memcpy((byte *)ms.getBasePtr(0, row),
									   (const byte *)galBg.surface.getBasePtr(0, row), bw);
							}
						}
						// Partner sprite at (5, 0x50). The original
						// `MoreInfo @ 158f:0419` calls
						// `_RefreshGalleryBackground` (clears the
						// portrait grid) but the partner anim slot
						// registered by `_DoGallery` keeps painting
						// at every `_UpdateAnimations` tick — the
						// suspect detail pic covers the right side
						// only (drawn at 0x94, 0xf), so the partner
						// stays visible on the left. Without this
						// blit the suspect-detail screen has no
						// partner.
						{
							const uint partnerAnim =
								(_partner == 0) ? 2 : 0x10;
							Animation partnerAni;
							if (_aniArchive.loadAnimation(partnerAnim,
														   partnerAni) &&
								!partnerAni.empty()) {
								const uint32 now = g_system->getMillis();
								const uint frameIdx =
									partnerFrameAtTick(0x02,
										(uint)partnerAni.size(), now);
								blitAnimFrameAnchored(ms.surfacePtr(),
									partnerAni[frameIdx], 5, 0x50);
							}
						}
						// Full suspect picture at (0x94, 0xf).
						Picture detail;
						if (_picsArchive.getPicture(detailPic, detail)) {
							const byte transp =
								(byte)(detail.flags >> 8);
							const int dx = 0x94, dy = 0x0f;
							const int dw = MIN<int>(detail.surface.w, 320 - dx);
							const int dh = MIN<int>(detail.surface.h, 200 - dy);
							for (int row = 0; row < dh; row++) {
								const byte *src =
									(const byte *)detail.surface.getBasePtr(0, row);
								byte *dst =
									(byte *)ms.getBasePtr(0, dy + row);
								for (int col = 0; col < dw; col++) {
									if (src[col] != transp)
										dst[dx + col] = src[col];
								}
							}
						}
						// Suspect's clue notes inside _GalleryNoteRect
						// = (78, 93, 288, 152), per 29be:0100. Cyan text
						// renders directly on the PDA's natural blue
						// screen — matches `_DrawGalleryNotes @ 158f:01f4`.
						const int rx = 78, ry = 93;
						const int rw = 288 - 78, rh = 152 - 93;

						const byte *ni = _mystery.noteIndex();
						const uint16 niCount = _mystery.noteIndexCount();
						int yPos = ry;
						const int lineH = _font.getFontHeight() + 1;
						bool drewAny = false;
						for (uint k = 0; k < clueCount && k < 30; k++) {
							const uint16 clueId =
								READ_LE_UINT16(suspect + 0xa + k * 2);
							if (clueId == 0xFFFF)
								break;
							if (clueId >= Mystery::kCluesFoundCap ||
								!_mystery._cluesFound[clueId])
								continue;
							if (!ni || clueId >= niCount)
								continue;
							const uint16 textOff =
								READ_LE_UINT16(ni + clueId * 4);
							Common::String txt =
								parseString(_mystery.textAt(textOff),
											_playerName, _partner);
							if (txt.empty())
								continue;
							const byte color =
								_mystery._noteSelected[clueId] ? 0x3C : 0x5C;
							const int hLine = _font.drawWordWrapped(
								&ms, rx, yPos, rw, txt, color);
							yPos += hLine + 7;
							drewAny = true;
							if (yPos + lineH > ry + rh)
								break;
						}
						if (!drewAny && _font.isLoaded()) {
							_font.drawString(&ms,
								"No clues yet for this suspect.",
								rx, ry, rw, 0x5C);
						}
						// Header / footer text.
						if (_font.isLoaded()) {
							_font.drawString(&ms, "SUSPECT FILE",
											  rx, ry - 11, rw, 0x3C);
							_font.drawString(&ms, "(click / ESC: back)",
											  rx, ry + rh + 2, rw, 0x3C);
						}
						g_system->copyRectToScreen(ms.getPixels(),
							ms.pitch, 0, 0, 320, 200);
						g_system->updateScreen();

						// Wait for click or ESC. Drain the queued
						// LBUTTONDOWN that triggered this MoreInfo first
						// so we don't immediately accept it as the
						// dismiss event.
						g_system->delayMillis(150);
						{
							Common::Event drain;
							while (g_system->getEventManager()->pollEvent(drain)) {
								if (drain.type == Common::EVENT_QUIT ||
									drain.type == Common::EVENT_RETURN_TO_LAUNCHER)
									return;
							}
						}
						bool back = false;
						while (!back && !shouldQuit()) {
							Common::Event e2;
							while (g_system->getEventManager()->pollEvent(e2)) {
								if (e2.type == Common::EVENT_LBUTTONDOWN ||
									(e2.type == Common::EVENT_KEYDOWN &&
									 (e2.kbd.keycode == Common::KEYCODE_ESCAPE ||
									  e2.kbd.keycode == Common::KEYCODE_RETURN))) {
									back = true;
									break;
								}
								if (e2.type == Common::EVENT_QUIT ||
									e2.type == Common::EVENT_RETURN_TO_LAUNCHER)
									return;
							}
							// Per-tick `updateScreen()` so the SDL cursor
							// follows the mouse — without it the cursor
							// freezes on entry to the MoreInfo screen
							// (we never repaint here, so the cursor never
							// gets drawn at its current position).
							g_system->updateScreen();
							g_system->delayMillis(20);
						}
						// Force gallery redraw immediately so the
						// player isn't left looking at the dismissed
						// MoreInfo screen until the next 100 ms tick.
						drawGalleryFrame(gd, num, slotRects, slotSuspect);
						lastDraw = g_system->getMillis();
						clicked = true;
						break;
					}
				}
				(void)clicked;
			}
		}
		if (exitFlag)
			break;

		const uint32 now = g_system->getMillis();
		if (now - lastDraw >= 100) {
			drawGalleryFrame(gd, num, slotRects, slotSuspect);
			lastDraw = now;
		}
		// `g_system->updateScreen()` is what tells the framework to
		// re-render the cursor at its current mouse position; without
		// it here, the cursor only refreshes when `drawGalleryFrame`
		// runs (every 100 ms) and visibly lags the mouse. Match
		// `doNotebook`'s per-tick `updateScreen()` cadence (line 1548).
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

void EEMEngine::drawGalleryFrame(const byte *gd, uint8 numSuspects,
								  Common::Array<Common::Rect> &slotRects,
								  Common::Array<int> &slotSuspect) {
	// Gallery redraw — formerly the `drawFrame` lambda inside `doGallery`.
	// Mirrors `_DrawGallery @ 158f:0046`: PIC 0x3f frame + partner sprite
	// at (5, 0x50) + suspect portraits in their `_NewOrder` slots. Slot
	// positions live in `kGallerySlots` in this file's anon namespace.
	Picture galBg;
	const bool haveBg = _picsArchive.getPicture(0x3f, galBg);
	const uint partnerAnim = (_partner == 0) ? 2 : 0x10;
	Animation partnerAni;
	const bool havePartner = _aniArchive.loadAnimation(partnerAnim, partnerAni)
							  && !partnerAni.empty();

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	if (haveBg) {
		const int bw = MIN<int>(galBg.surface.w, 320);
		const int bh = MIN<int>(galBg.surface.h, 200);
		for (int row = 0; row < bh; row++) {
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)galBg.surface.getBasePtr(0, row), bw);
		}
	}

	// Partner sprite frame @ (5, 0x50). The original `_DoGallery @
	// 158f:065b` registers `_NewAnimation(..., CONCAT22(2, ...), ...)`
	// — script key 0x02 regardless of partner. Jake's 0x02 script
	// (26 frames, brief wave + long hold + second wave) is what
	// drives BOTH partners' cells. Earlier our port used 0x10 for
	// Jenny, which is a 9-frame short blip — so Jenny was missing
	// 17 frames of the wave-and-pause cadence that Jake has.
	if (havePartner) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = partnerFrameAtTick(0x02,
												  (uint)partnerAni.size(), now);
		// Anchor-aware blit, consistent with site-loop / BigMap
		// rendering paths. Anim 0x02 has rowoff = miscflags = 0
		// per the audit but the anchored blitter is still the
		// right semantic for an `_NewAnimation`-rendered sprite.
		blitAnimFrameAnchored(scratch.surfacePtr(),
							  partnerAni[frameIdx], 5, 0x50);
	}

	// Portraits — `_DrawGallery @ 158f:0046` walks suspects 0..N-1 and
	// only renders those flagged in `_InGallery[NewOrder[i]]`.
	for (uint i = 0; i < numSuspects && i < Mystery::kGalleryCap; i++) {
		slotRects[i] = Common::Rect();
		slotSuspect[i] = -1;

		const uint8 phys = _mystery._newOrder[i];
		if (phys >= 5)
			continue;
		const GallerySlot &s = kGallerySlots[phys];

		const bool discovered = _mystery._inGallery[phys] != 0;
		if (discovered) {
			const uint16 picId = READ_LE_UINT16(gd + i * 0x46);
			Picture portrait;
			if (picId == 0 ||
				!_picsArchive.getPicture(picId, portrait))
				continue;

			const int placeX = s.x;
			const int placeY = s.y + (0x48 - portrait.surface.h);
			const byte transp = (byte)(portrait.flags >> 8);
			const int w = MIN<int>(portrait.surface.w, 320 - placeX);
			const int h = MIN<int>(portrait.surface.h, 200 - placeY);
			if (w <= 0 || h <= 0)
				continue;
			for (int row = 0; row < h; row++) {
				const int dstY = placeY + row;
				if (dstY < 0)
					continue;
				const byte *src =
					(const byte *)portrait.surface.getBasePtr(0, row);
				byte *dst = (byte *)scratch.getBasePtr(0, dstY);
				for (int col = 0; col < w; col++) {
					const int dstX = placeX + col;
					if (src[col] != transp)
						dst[dstX] = src[col];
				}
			}
			slotRects[i] = Common::Rect(placeX, placeY,
										 placeX + w, placeY + h);
			slotSuspect[i] = (int)i;
		} else {
			// Undiscovered placeholder — small framed "?" box.
			const int phW = 0x40;
			const int phH = 0x48;
			const int phX = s.x;
			const int phY = s.y;
			if (phX + phW <= 320 && phY + phH <= 200) {
				scratch.fillRect(Common::Rect(phX, phY,
					phX + phW, phY + phH), 0x20);
				scratch.frameRect(Common::Rect(phX, phY,
					phX + phW, phY + phH), 0x5C);
				if (_font.isLoaded()) {
					_font.drawString(&scratch, "?",
						phX + phW / 2 - 3,
						phY + phH / 2 - 4, phW, 0x5C);
				}
			}
		}
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

void EEMEngine::doBigMap() {
	// Two-stage flow that mirrors the original screen-1 wrapper at
	// 20fe:120b and `_DoBigMap @ 20fe:09e7`:
	//
	//   STAGE 1 — Overview. PIC 0x42 + site icons drawn via the
	//   `_DrawBigMapButtons` algorithm at BigMap coords MapData[+4/+6].
	//   The original `_DoBigMap` returns sx/sy = (mouseX*2 - 0x74,
	//   mouseY*2 - 0x55) when the player clicks inside `BigMapWindow`,
	//   which is the scroll position into the SmallMap.
	//
	//   STAGE 2 — Detail zoom. PIC 0x43 frame + a 0xe9 × 0xab viewport
	//   into BIGMAP.PIC at (2, 2), drawn by `DrawMap @ 20fe:1058` with
	//   the (sx, sy) returned from stage 1. Site icons are stamped at
	//   SmallMap coords MapData[+8/+0xa] via `_StampButtons`. Click on
	//   a site icon → travel.
	//
	// MapData entry layout (14 bytes), verified directly from the
	// disassembly of `_DrawBigMapButtons @ 20fe:0877` (`PUSH ES:[BX+4]`
	// for X, `PUSH ES:[BX+6]` for Y, `CMP ES:[BX+0xc], 0` for crime)
	// and `_StampButtons @ 20fe:0d2f` (`MOV AX, ES:[BX+8]`,
	// `MOV AX, ES:[BX+0xa]`):
	//   +0..3   ??? (not yet decoded)
	//   +4..5   BigMap X
	//   +6..7   BigMap Y
	//   +8..9   SmallMap X
	//   +0xa..b SmallMap Y
	//   +0xc..d crime-flag

	if (!_mystery.isLoaded())
		return;

	CursorMan.showMouse(true);

	// `_GetPalette(0x24)` per `_DoBigMap @ 20fe:09e7`.
	setSitePalette(0x24);

	const Common::Rect kSetupRect(0xc7, 0x12, 0xc7 + 0x32, 0x12 + 0xa); // approx; original from globals
	(void)kSetupRect; // not yet wired into our overlay

	// ------------------------------------------------------------------
	// STAGE 1 — Overview: PIC 0x42 + clickable site icons.
	// ------------------------------------------------------------------

	// Anchor for the partner-sprite timeline. `_DoBigMap`'s
	// `_NewAnimation` call seeds the slot's frame index to 0xffff so
	// the first `_UpdateAnimations` tick starts at script[0]; we mirror
	// that by passing elapsed-since-open (zero on the first paint) into
	// `bigMapPartnerFrameAtTick`, which plays the unfold once and then
	// loops the wait sequence.
	const uint32 mapStartTick = g_system->getMillis();
	drawBigMapOverview(0);
	uint32 mapLastTick = mapStartTick;

	// Static rectangles read directly from the binary at the labelled
	// addresses (29be:0x1596 onwards). Format is {x1, y1, x2, y2}.
	const Common::Rect kBigMapWindow   (  0,   0, 247, 192); // 29be:1596
	const Common::Rect kSetupBtnRect   (252,   4, 315,  42); // 29be:15ce

	bool wantZoom = false;
	int zoomX = 0;
	int zoomY = 0;
	while (!shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN &&
				ev.kbd.keycode == Common::KEYCODE_ESCAPE)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// SetupButtonRect → `_NextScreen = 6` (the original's
				// settings screen, mirrors `_DoBigMap @ 20fe:0c33`
				// where it pushes `_PressButton` then writes
				// `_NextScreen = 6`). Now wired to the actual
				// `doSetup` handler instead of dropping the player
				// out to the launcher.
				if (kSetupBtnRect.contains(ev.mouse.x, ev.mouse.y)) {
					_nextScreen = kScreenSetup;
					return;
				}
				// Click in the BigMapWindow → zoom. Original formula:
				//   sx = mouseX*2 - 0x74; sy = mouseY*2 - 0x55
				if (kBigMapWindow.contains(ev.mouse.x, ev.mouse.y)) {
					int sx = ev.mouse.x * 2;
					int sy = ev.mouse.y * 2;
					sx = (sx < 0x75) ? 0 : sx - 0x74;
					sy = (sy < 0x56) ? 0 : sy - 0x55;
					zoomX = sx;
					zoomY = sy;
					wantZoom = true;
					break;
				}
			}
		}
		if (wantZoom)
			break;
		// Cycle the partner-sprite frame every 100 ms (matching the
		// original's `_CheckFrameRate` cadence inside `_DoBigMap`).
		const uint32 now = g_system->getMillis();
		if (now - mapLastTick >= 100) {
			mapLastTick = now;
			drawBigMapOverview(now - mapStartTick);
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	if (!wantZoom)
		return;

	// ------------------------------------------------------------------
	// STAGE 2 — Detail zoom: PIC 0x43 frame + scrollable BIGMAP.PIC
	// viewport at (2, 2), 0xe9 × 0xab. Click on a stamped icon → travel.
	// ------------------------------------------------------------------

	Common::File f;
	if (!f.open(Common::Path("BIGMAP.PIC"))) {
		warning("doBigMap: BIGMAP.PIC missing for detail view");
		return;
	}
	const uint16 mapH = f.readUint16LE();
	const uint16 mapW = f.readUint16LE();
	if (mapW == 0 || mapH == 0)
		return;
	Common::Array<byte> mapPixels((uint32)mapW * mapH);
	if (f.read(mapPixels.data(), mapPixels.size()) != mapPixels.size()) {
		warning("doBigMap: short read on BIGMAP.PIC for detail view");
		return;
	}

	const int kMapWinW = 0xe9; // 233
	const int kMapWinH = 0xab; // 171
	const int kMapWinX = 2;
	const int kMapWinY = 2;

	int scrollX = MAX<int>(0, MIN<int>(mapW - kMapWinW, zoomX));
	int scrollY = MAX<int>(0, MIN<int>(mapH - kMapWinH, zoomY));

	// Anchor the detail-screen partner timeline (mirrors `_DoMapScreen`'s
	// `_NewAnimation` seeding the slot's frame index to 0xffff). The
	// unfold (script 0x13) plays once, then `_SmallMapWaitSeq` loops.
	const uint32 detailStartTick = g_system->getMillis();
	drawBigMapDetail(scrollX, scrollY, mapPixels, mapW, mapH, 0);
	uint32 detailLastTick = detailStartTick;

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
					return;  // exit detail back to caller (site loop / engine)
				const int kStep = 16;
				if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
					scrollX = MAX<int>(0, scrollX - kStep);
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT) {
					scrollX = MIN<int>(MAX<int>(0, mapW - kMapWinW),
						scrollX + kStep);
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_UP) {
					scrollY = MAX<int>(0, scrollY - kStep);
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_DOWN) {
					scrollY = MIN<int>(MAX<int>(0, mapH - kMapWinH),
						scrollY + kStep);
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Scroll arrows + slider rects live in `SmallMapButtons`
				// at 29be:0x159e (six 8-byte rects in order: Y-up, Y-down,
				// X-left, X-right, right-panel, top-right) plus the
				// dedicated `XSliderRect @ 29be:15d6` and
				// `YSliderRect @ 29be:15de`. Format {x1,y1,x2,y2}.
				const Common::Rect kArrowYUp   (237,   2, 247,  11);
				const Common::Rect kArrowYDown (237, 163, 247, 172);
				const Common::Rect kArrowXLeft (  2, 175,  12, 185);
				const Common::Rect kArrowXRight(224, 175, 234, 185);
				const Common::Rect kXSlider    ( 15, 175, 221, 185);
				const Common::Rect kYSlider    (237,  14, 247, 160);
				const Common::Rect kSetupBtn   (252,   4, 315,  42);

				const int kArrowStep = 16;
				const int kSliderRange = mapW - kMapWinW;
				const int kSliderRangeY = mapH - kMapWinH;

				if (kSetupBtn.contains(ev.mouse.x, ev.mouse.y)) {
					// `_DoMapScreen @ 20fe:1560` writes `_NextScreen
					// = 6` (= kScreenSetup) and `INC [BP-8]` to bail
					// out of the detail loop — verified via the byte
					// search for `c7 06 16 79 06 00`, which finds the
					// imm at exactly this site and `_DoBigMap @
					// 20fe:0c33`. Same `SetupButtonRect @ 29be:15ce`
					// rect used by both the overview and the detail
					// (no per-screen rect duplication in the binary).
					// The detail/zoom state is lost on return because
					// the screen driver re-enters BigMap at stage 1 —
					// this matches the original behaviour.
					_nextScreen = kScreenSetup;
					return;
				}
				if (kArrowYUp.contains(ev.mouse.x, ev.mouse.y)) {
					scrollY = MAX<int>(0, scrollY - kArrowStep);
					dirty = true;
				} else if (kArrowYDown.contains(ev.mouse.x, ev.mouse.y)) {
					scrollY = MIN<int>(MAX<int>(0, kSliderRangeY),
						scrollY + kArrowStep);
					dirty = true;
				} else if (kArrowXLeft.contains(ev.mouse.x, ev.mouse.y)) {
					scrollX = MAX<int>(0, scrollX - kArrowStep);
					dirty = true;
				} else if (kArrowXRight.contains(ev.mouse.x, ev.mouse.y)) {
					scrollX = MIN<int>(MAX<int>(0, kSliderRange),
						scrollX + kArrowStep);
					dirty = true;
				} else if (kXSlider.contains(ev.mouse.x, ev.mouse.y)) {
					// Click on X slider track → jump scrollX so the
					// click position maps proportionally into the map.
					if (kSliderRange > 0) {
						const int t = ev.mouse.x - kXSlider.left;
						const int tw = kXSlider.width();
						scrollX = MAX<int>(0, MIN<int>(kSliderRange,
							t * kSliderRange / MAX<int>(1, tw)));
						dirty = true;
					}
				} else if (kYSlider.contains(ev.mouse.x, ev.mouse.y)) {
					if (kSliderRangeY > 0) {
						const int t = ev.mouse.y - kYSlider.top;
						const int th = kYSlider.height();
						scrollY = MAX<int>(0, MIN<int>(kSliderRangeY,
							t * kSliderRangeY / MAX<int>(1, th)));
						dirty = true;
					}
				} else if (ev.mouse.x >= kMapWinX &&
						   ev.mouse.x < kMapWinX + kMapWinW &&
						   ev.mouse.y >= kMapWinY &&
						   ev.mouse.y < kMapWinY + kMapWinH) {
					// Hit-test the per-site button at its actual bbox
					// (`_StampButtons` records the rect at SmallMap +8/+0xa
					// with the button PIC's width/height).
					const bool fmap = _mystery.isLoaded() && isFloppy();
					for (uint i = 0; i < _mystery.numSites(); i++) {
						if (!_mystery._onSites[i] &&
							i != _mystery._siteNumber)
							continue;
						const byte *entry = _mystery.mapEntry(i);
						if (!entry)
							continue;
						uint16 mx;
						uint16 my;
						uint16 buttonId;
						if (fmap) {
							// Floppy detail view: click rect on
							// BIGMAP.PIC at (+0, +2), labelled BUTTON.DBD
							// entry ID at entry+4 (per
							// `FUN_1fed_0c3e @ 1fed:0c3e`).
							mx = READ_LE_UINT16(entry + 0x0);
							my = READ_LE_UINT16(entry + 0x2);
							buttonId = (uint16)entry[0x4];
						} else {
							buttonId = READ_LE_UINT16(entry + 0x0);
							mx       = READ_LE_UINT16(entry + 0x8);
							my       = READ_LE_UINT16(entry + 0xa);
						}
						Picture button;
						int bw = 16;
						int bh = 16;
						if (_buttonArchive.loadEntry(buttonId, button)) {
							bw = button.surface.w;
							bh = button.surface.h;
						}
						const int sx = (int)mx - scrollX + kMapWinX;
						const int sy = (int)my - scrollY + kMapWinY;
						if (ev.mouse.x >= sx && ev.mouse.x < sx + bw &&
							ev.mouse.y >= sy && ev.mouse.y < sy + bh) {
							_mystery._lastSite = _mystery._siteNumber;
							_mystery._siteNumber = (uint16)i;
							return;
						}
					}
				}
			}
		}
		// Cycle the partner sprite at 100 ms ticks (same cadence as
		// `_DoMapScreen`'s `_CheckFrameRate` + `_UpdateAnimations` loop).
		const uint32 now = g_system->getMillis();
		if (now - detailLastTick >= 100) {
			detailLastTick = now;
			dirty = true;
		}
		if (dirty)
			drawBigMapDetail(scrollX, scrollY, mapPixels, mapW, mapH,
							 now - detailStartTick);
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void EEMEngine::drawBigMapOverview(uint32 elapsedMs) {
	// Map-overview redraw — formerly the `drawOverview` lambda inside
	// `doBigMap`. PIC 0x42 frame + per-site marker (Done / Crime / Site
	// per `_DrawBigMapButtons @ 20fe:0877`) + the partner idle sprite.
	// `_DoBigMap @ 20fe:09e7` (`_NewAnimation` block at 20fe:0a44-0a99)
	// registers the partner: when `_LastScreen == 2` it plays an
	// entrance one-shot at (0x102, 0x50) and on END swaps to the idle
	// at (0xfd, 0x50). We don't track LastScreen finely enough so we
	// always render the IDLE pose at (0xfd, 0x50). Idle anim ID:
	// Jake = 0x14 (20), Jenny = 0x12 (18).
	//
	// `elapsedMs` is the time since `doBigMap` opened — the partner-sprite
	// timeline anchor. `bigMapPartnerFrameAtTick` uses it to play the
	// unfold script (0..8) once, then loop `_BigMapWaitSeq` (the open-map
	// hold). Without that anchor the unfold would loop indefinitely.
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x42, frame))
		scratch.simpleBlitFrom(frame.surface);

	// Marker PICs from `_main @ 1a35:0f59`. Three globals are filled
	// once at boot via `_GetPicture` (1-based IDs):
	//   _DoneMarker  = PIC 0x20d  (already-searched site)
	//   _SiteMarker  = PIC 0xc5   (default available site)
	//   _CrimeMarker = PIC 0xc6   (crime-scene flag set)
	Picture done;
	Picture normal;
	Picture crimeM;
	const bool haveDone   = _picsArchive.getPicture(0x20d, done);
	const bool haveNormal = _picsArchive.getPicture(0xc5,  normal);
	const bool haveCrime  = _picsArchive.getPicture(0xc6,  crimeM);

	for (uint i = 0; i < _mystery.numSites(); i++) {
		if (!_mystery._onSites[i] && i != _mystery._siteNumber)
			continue;
		const byte *entry = _mystery.mapEntry(i);
		if (!entry)
			continue;
		// CD entries are 14 bytes: X@+4, Y@+6, crime@+12.
		// Floppy entries are 11 bytes: X@+6, Y@+8, recolor@+10.
		// Floppy layout verified at `FUN_1fed_07ed` (BigMap iteration):
		//   `*(int *)(pcVar2 + i*0xb + 7)` (= entry+6, X u16)
		//   `*(int *)(pcVar2 + i*0xb + 9)` (= entry+8, Y u16)
		//   `pcVar2[i*0xb + 0xb]` (= entry+10, recolor flag — non-zero
		//   selects the crime-marker PIC over the regular site marker).
		const bool floppy  = _mystery.isLoaded() && isFloppy();
		const uint16 mx    = floppy ? READ_LE_UINT16(entry + 0x6)
									: READ_LE_UINT16(entry + 0x4);
		const uint16 my    = floppy ? READ_LE_UINT16(entry + 0x8)
									: READ_LE_UINT16(entry + 0x6);
		const uint16 crime = floppy ? (uint16)entry[0xa]
									: READ_LE_UINT16(entry + 0xc);
		const bool   done_ = (i < Mystery::kVisitedSiteCap)
							  && _mystery._visitedSite[i];

		const Picture *m = nullptr;
		if (done_ && haveDone)
			m = &done;
		else if (crime != 0 && haveCrime)
			m = &crimeM;
		else if (haveNormal)
			m = &normal;

		if (m) {
			// Masked-blit the marker PIC.
			const byte transp = (byte)(m->flags >> 8);
			for (int row = 0; row < m->surface.h; row++) {
				const int dstY = (int)my + row;
				if (dstY < 0 || dstY >= 200)
					continue;
				const byte *src = (const byte *)m->surface.getBasePtr(0, row);
				byte *dst = (byte *)scratch.getBasePtr(0, dstY);
				for (int col = 0; col < m->surface.w; col++) {
					const int dstX = (int)mx + col;
					if (dstX < 0 || dstX >= 320)
						continue;
					if (src[col] != transp)
						dst[dstX] = src[col];
				}
			}
		} else {
			// Fallback if the markers couldn't be loaded.
			const Common::Rect mark(mx - 3, my - 3, mx + 4, my + 4);
			scratch.fillRect(mark, 0x0F);
		}
	}

	// Partner idle sprite at (0xfd, 0x50). Jake = anim 0x14, Jenny = 0x12
	// for the loaded CELLS, but the original `_DoBigMap @ 20fe:0a47`
	// always passes `CONCAT22(0x14, ...)` to `_NewAnimation` so the
	// SCRIPT key is 0x14 (`[0..8]` count-up) regardless of partner.
	// Without this, Jenny was running 0x12's count-DOWN script
	// `[8..0]` over her cells — visually backwards from the original.
	const uint kMapAniId = (_partner == 0) ? 0x14 : 0x12;
	Animation mapAnim;
	if (_aniArchive.loadAnimation(kMapAniId, mapAnim) && !mapAnim.empty()) {
		const uint frameIdx = bigMapPartnerFrameAtTick((uint)mapAnim.size(),
													   elapsedMs);
		// Anchor-aware: the BigMap walk-cycle has miscflags = -2 per
		// cell, so the partner shifts left as it cycles — without the
		// anchor adjustment the sprite "shakes in place" instead of
		// walking forward.
		blitAnimFrameAnchored(scratch.surfacePtr(), mapAnim[frameIdx],
							  0xfd, 0x50);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

void EEMEngine::drawBigMapDetail(int scrollX, int scrollY,
								 const Common::Array<byte> &mapPixels,
								 uint16 mapW, uint16 mapH,
								 uint32 elapsedMs) {
	// Map-detail redraw — formerly the `drawDetail` lambda inside
	// `doBigMap`. PIC 0x43 frame + a 0xe9 × 0xab BIGMAP.PIC viewport at
	// (2, 2), stamped site buttons, and the partner sprite at (0x101,
	// 0x50) — `_DoMapScreen @ 20fe:120b` (`_NewAnimation` at
	// 20fe:12cd-12f0, anim 0x13 Jake / 0x11 Jenny, seqnum 0x13).
	//
	// `elapsedMs` is the time since the detail screen was opened —
	// `bigMapDetailPartnerFrameAtTick` uses it to play the unfold once
	// and then loop `_SmallMapWaitSeq`.
	const int kMapWinW = 0xe9;
	const int kMapWinH = 0xab;
	const int kMapWinX = 2;
	const int kMapWinY = 2;

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x43, frame))
		scratch.simpleBlitFrom(frame.surface);

	const int copyW = MIN<int>(mapW - scrollX, kMapWinW);
	const int copyH = MIN<int>(mapH - scrollY, kMapWinH);
	for (int row = 0; row < copyH; row++) {
		memcpy((byte *)scratch.getBasePtr(kMapWinX, kMapWinY + row),
			   mapPixels.data() + (scrollY + row) * mapW + scrollX,
			   copyW);
	}

	// Stamped site buttons. `_StampButtons @ 20fe:0d2f` (CD):
	//   button = _GetButton(MapData[+0])
	//   destX  = MapData[+8]
	//   destY  = MapData[+0xa]
	// Floppy uses `FUN_1fed_0c3e @ 1fed:0c3e`: for each SITES row, the
	// byte at entry+4 is a BUTTON.DBD entry ID (loaded via
	// `FUN_16e2_1838 @ 16e2:1838`, which opens `button.dbd` — string
	// at `2608:0558`). The labelled button is stamped at
	// `(entry+0..1, entry+2..3)` on BIGMAP.PIC. These are the same
	// per-site labelled buttons the CD uses, just keyed off a
	// different field offset.
	const bool floppyMap = _mystery.isLoaded() && isFloppy();
	for (uint i = 0; i < _mystery.numSites(); i++) {
		if (!_mystery._onSites[i] && i != _mystery._siteNumber)
			continue;
		const byte *entry = _mystery.mapEntry(i);
		if (!entry)
			continue;
		uint16 mx;
		uint16 my;
		Picture button;
		if (floppyMap) {
			mx = READ_LE_UINT16(entry + 0x0);
			my = READ_LE_UINT16(entry + 0x2);
			const uint16 buttonId = (uint16)entry[0x4];
			if (!_buttonArchive.loadEntry(buttonId, button))
				continue;
		} else {
			const uint16 buttonId = READ_LE_UINT16(entry + 0x0);
			mx                    = READ_LE_UINT16(entry + 0x8);
			my                    = READ_LE_UINT16(entry + 0xa);
			if (!_buttonArchive.loadEntry(buttonId, button))
				continue;
		}
		const int sx = (int)mx - scrollX + kMapWinX;
		const int sy = (int)my - scrollY + kMapWinY;
		const byte transp = (byte)(button.flags >> 8);

		// Crop blit against the viewport.
		const int x0 = MAX<int>(sx, kMapWinX);
		const int y0 = MAX<int>(sy, kMapWinY);
		const int x1 = MIN<int>(sx + button.surface.w, kMapWinX + kMapWinW);
		const int y1 = MIN<int>(sy + button.surface.h, kMapWinY + kMapWinH);
		for (int row = y0; row < y1; row++) {
			const byte *src = (const byte *)button.surface.getBasePtr(0, row - sy);
			byte *dst = (byte *)scratch.getBasePtr(0, row);
			for (int col = x0; col < x1; col++) {
				const byte px = src[col - sx];
				if (px != transp)
					dst[col] = px;
			}
		}
	}

	// Partner sprite on the detail map (drawn last to sit over the
	// frame and the BIGMAP.PIC viewport). The original always passes
	// `CONCAT22(0x13, ...)` to `_NewAnimation` (i.e. script ID 0x13)
	// regardless of partner — verified at `_DoBigMap @ 20fe:0a47`.
	// So we look up script 0x13 for both partners while still
	// loading the partner-specific CELLS via `kDetailAniId`.
	const uint kDetailAniId = (_partner == 0) ? 0x13 : 0x11;
	Animation detailAnim;
	if (_aniArchive.loadAnimation(kDetailAniId, detailAnim) &&
		!detailAnim.empty()) {
		const uint frameIdx = bigMapDetailPartnerFrameAtTick(
				(uint)detailAnim.size(), elapsedMs);
		blitAnimFrameAnchored(scratch.surfacePtr(),
							  detailAnim[frameIdx], 0x101, 0x50);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

uint16 EEMEngine::getKDTextBalloon(byte firstChar) const {
	// Mirrors `_GetKDTextBalloon @ 1df2:0105`:
	//   if ((ctype[firstChar] & 2) == 0)  bub = *(u16*)29be:1068 = 0x17
	//   else                              bub = *(u16*)(29be:0fe6+0x1e+c*2)
	// `ctype` is Borland's `_ctype_` array at `29be:2be1`. Bit 1 (0x02) is
	// set only for digits '0'..'9' (verified by reading the table — '0'..'9'
	// each map to byte 0x02; everything else has bit 1 clear).
	// Lookup table at 29be:1064 (= 29be:0fe6 + 0x1e + '0'*2):
	//   '0'→0x15  '1'→0x16  '2'→0x17  '3'→0x18  '4'→0x19
	//   '5'→0x1a  '6'→0x20  '7'→0x21  '8'→0x22  '9'→0x1e
	// Note `*(u16*)29be:1068` (= entry for '2') is the same byte the
	// non-digit fallback returns — the original encodes the constant by
	// reusing the digit-2 slot.
	if (firstChar < '0' || firstChar > '9')
		return 0x17;
	// `kDigitBalloons` lives at file scope above.
	return kDigitBalloons[firstChar - '0'];
}

bool EEMEngine::doAccuseNotes() {
	// Mirrors the accuse-notes screen at the head of `_DoAccuse @
	// 1df2:0bdd`:
	//   * BG: PIC 0x1A7 (the red "accuse-mode" backdrop).
	//   * `_AccuseNoteRect @ 29be:1048` = (79, 27, 304, 159) holds
	//     the rendered clue list.
	//   * Counter at `(0xd1, 0xb)` = `(209, 11)` shows "N clue(s)"
	//     remaining (`_UpdateSelectionCount @ 1df2:08dd`,
	//     `_Show_String(0xb, 0xd1, ...)`).
	//   * Expected count = `6 - DAT_2d5d_3f99`:
	//       chainStage 1 → 5 clues, 2 → 4 clues, 3 → 3 clues.
	//   * `_NoteUnselectedColor = 1` (red) for unselected, `0x3c`
	//     for selected (1df2:0c2c sets it on entry).
	//   * Click on a clue toggles its selection
	//     (`_SearchNoteAreas` + `_SwapColors`).
	//   * Click `_NoteButtons[2]` (rect at `(157, 174, 178, 190)`,
	//     the original's "go to gallery" button) jumps to the
	//     evidence check; `_HandleAccuseNoteButton(2)` returns 2
	//     and the outer loop forces `uStack_8 = uStack_a` to
	//     trigger `_SolvedCheck`.
	//   * ESC sets `_NextScreen = 3` and exits.
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return false;
	const byte *ni = _mystery.noteIndex();
	const uint16 niCount = _mystery.noteIndexCount();
	if (!ni)
		return false;

	Picture accuseBg;
	const bool haveBg = _picsArchive.getPicture(0x1a7, accuseBg);

	// Required count for solving — `6 - chainStage`.
	const uint expected = (_chainStage >= 1 && _chainStage <= 3)
		? (uint)(6 - _chainStage)
		: 5;

	// Build the list of FOUND clue IDs (in clue-ID order; the
	// original's `_DrawNotes(NULL, 100, ...)` walks `_CluesFound[]`
	// the same way).
	Common::Array<uint> found;
	for (uint i = 0; i < niCount && i < Mystery::kCluesFoundCap; i++) {
		if (_mystery._cluesFound[i])
			found.push_back(i);
	}

	// `_AccuseNoteRect` (79, 27, 304, 159) — text wrap area.
	const int rectX = 79;
	const int rectY = 27;
	const int rectW = 304 - 79;
	const int rectH = 159 - 27;

	// `_NoteButtons` rects (verified at `29be:0147`). `_DoAccuse`
	// re-uses the same table as `_DoNotebook`, but only SOLVE /
	// PAGE NEXT / PAGE PREV do anything; others sit inert.
	// `_HandleAccuseNoteButton @ 1df2:0990` returns `DI` (initialised
	// to 0) and only sets `DI = 2` in the `i == 4` branch (asm:
	// `1df2:09b2: MOV DI, 0x2`). The outer loop's `iVar6 == 2` test
	// at `1df2:0db2` is checking the HANDLER'S RETURN VALUE, not the
	// button INDEX — earlier comment had this backwards. So the
	// SOLVE rect is `[4]` (180, 174, 201, 190), the same icon the
	// PDA uses to trigger the accuse flow in the first place.
	const Common::Rect kBtnSolve   (180, 174, 201, 190); // [4] SOLVE
	const Common::Rect kBtnPageNext(204, 174, 224, 190); // [5] PAGE NEXT
	const Common::Rect kBtnPagePrev(226, 174, 247, 190); // [6] PAGE PREV
	const Common::Rect kBtnPartner (  5,  80,  44, 110); // [3] KD HELP

	// Per-page slot rects + their clue IDs (for click hit-testing).
	Common::Array<Common::Rect> slotRects;
	Common::Array<uint> slotClues;

	int page = 0;
	int pageBreaks[16];
	int numPages = 1;
	pageBreaks[0] = 0;

	auto rebuildPagination = [&]() {
		numPages = 1;
		pageBreaks[0] = 0;
		const int lineH = _font.getFontHeight() + 1;
		int y = rectY;
		for (uint i = 0; i < found.size(); i++) {
			const uint clueId = found[i];
			Common::String txt;
			if (clueId < niCount) {
				const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
				txt = parseString(_mystery.textAt(textOff),
								   _playerName, _partner);
			}
			Common::Array<Common::String> wrapped;
			_font.wordWrapText(txt, rectW, wrapped);
			const int h = (int)wrapped.size() * lineH;
			if (y + h + 7 > rectY + rectH) {
				if (numPages < (int)ARRAYSIZE(pageBreaks)) {
					pageBreaks[numPages++] = (int)i;
					y = rectY;
				}
			}
			y += h + 7;
		}
		if (page >= numPages)
			page = numPages - 1;
		if (page < 0)
			page = 0;
	};

	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveBg)
			scratch.simpleBlitFrom(accuseBg.surface);

		// Partner sprite at (5, 0x50). The original `_DoAccuse @
		// 1df2:0c2c` does `_NewAnimation(5, 0x50, partnerCells,
		// script=2, prior=1)` — same anim cells as the gallery
		// (anim 2 for Jake / 0x10 for Jenny) with script 0x02. The
		// `_UpdateAnimations` loop in `_DoAccuse @ 1df2:0bfa` keeps
		// the slot painting through the entire selection screen;
		// without an explicit blit here the player sees a partner-
		// less accuse-mode screen.
		const uint partnerAnim = (_partner == 0) ? 2 : 0x10;
		Animation partnerAni;
		if (_aniArchive.loadAnimation(partnerAnim, partnerAni) &&
			!partnerAni.empty()) {
			const uint32 now = g_system->getMillis();
			const uint frameIdx = partnerFrameAtTick(0x02,
													  (uint)partnerAni.size(), now);
			blitAnimFrameAnchored(scratch.surfacePtr(),
								  partnerAni[frameIdx], 5, 0x50);
		}

		// Clue list inside `_AccuseNoteRect`. Selected = 0x3c (yellow),
		// unselected = 1 (red), per `_NoteUnselectedColor = 1` set at
		// 1df2:0c25 — the red colour is what gives the screen its
		// "accuse-mode" look together with PIC 0x1A7.
		slotRects.clear();
		slotClues.clear();
		const int lineH = _font.getFontHeight() + 1;
		const int startIdx = pageBreaks[page];
		const int endIdx   = (page + 1 < numPages)
			? pageBreaks[page + 1]
			: (int)found.size();
		int y = rectY;
		uint selectedCount = 0;
		for (uint i = 0; i < found.size(); i++) {
			if (_mystery._noteSelected[found[i]])
				selectedCount++;
		}
		for (int i = startIdx; i < endIdx; i++) {
			const uint clueId = found[i];
			Common::String txt;
			if (clueId < niCount) {
				const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
				txt = parseString(_mystery.textAt(textOff),
								   _playerName, _partner);
			}
			if (txt.empty())
				txt = Common::String::format("clue %u", clueId);
			Common::Array<Common::String> wrapped;
			_font.wordWrapText(txt, rectW, wrapped);
			const int h = (int)wrapped.size() * lineH;
			const byte color = _mystery._noteSelected[clueId] ? 0x3c : 0x01;
			for (uint li = 0; li < wrapped.size(); li++) {
				_font.drawString(&scratch, wrapped[li], rectX,
								 y + (int)li * lineH, rectW, color);
			}
			slotRects.push_back(Common::Rect(rectX, y,
											  rectX + rectW, y + h));
			slotClues.push_back(clueId);
			y += h + 7;
		}

		// Counter — `_UpdateSelectionCount(remaining)` at (0xd1, 0xb).
		const uint remaining = (selectedCount < expected)
			? expected - selectedCount
			: 0;
		const Common::String counter = Common::String::format("%u %s",
			remaining, remaining == 1 ? "clue" : "clues");
		_font.drawString(&scratch, counter, 209, 11, 100, 0x0F);

		if (numPages > 1) {
			_font.drawString(&scratch,
				Common::String::format("p%d/%d", page + 1, numPages),
				rectX, 11, 60, 0x0F);
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	rebuildPagination();
	draw();

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return false;
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
					return false;
				if (ev.kbd.keycode == Common::KEYCODE_LEFT &&
					page > 0) {
					page--;
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT &&
						   page + 1 < numPages) {
					page++;
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const int mx = ev.mouse.x;
				const int my = ev.mouse.y;
				// Page navigation — `_NoteButtons[5]` / `[6]`,
				// dispatched in `_HandleAccuseNoteButton @
				// 1df2:0990`. Only effective if there's another
				// page in that direction; mirrors the
				// `1 < _CurrentPage` guard at 1df2:09a8 and the
				// `_NextClue != -1` guard at 1df2:099e.
				if (kBtnPageNext.contains(mx, my)) {
					if (page + 1 < numPages) {
						page++;
						dirty = true;
					}
					continue;
				}
				if (kBtnPagePrev.contains(mx, my)) {
					if (page > 0) {
						page--;
						dirty = true;
					}
					continue;
				}
				// Partner click — `_NoteButtons[3]`. Original
				// `_HandleAccuseNoteButton` doesn't dispatch this
				// (no case for i == 3), but the rect is still in
				// the table; we wire it to the puzzle hint so the
				// player can ask the partner what to look for
				// without leaving accuse mode.
				if (kBtnPartner.contains(mx, my)) {
					doHelp();
					dirty = true;
					continue;
				}
				if (kBtnSolve.contains(mx, my)) {
					// Count selected.
					uint selected = 0;
					for (uint i = 0; i < found.size(); i++) {
						if (_mystery._noteSelected[found[i]])
							selected++;
					}
					if (selected == expected) {
						// Commit — let the caller do the
						// `_SolvedCheck` + suspect picker dance.
						return true;
					}
					// Wrong count — `_DoAccuse` only triggers the
					// check when `uStack_8 == uStack_a`; we just
					// stay in the loop so the player can keep
					// adjusting.
					continue;
				}
				// Toggle clue under cursor.
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotRects[i].contains(mx, my)) {
						const uint clueId = slotClues[i];
						_mystery._noteSelected[clueId] ^= 1;
						dirty = true;
						break;
					}
				}
			}
		}
		if (dirty)
			draw();
		// Per-tick redraw so the partner sprite cycles. Same
		// 100 ms cadence as `_CheckFrameRate` + `_UpdateAnimations`
		// in the original (1df2:0bfa).
		static uint32 sLastTick = 0;
		const uint32 now = g_system->getMillis();
		if (now - sLastTick >= 100) {
			sLastTick = now;
			draw();
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	return false;
}

void EEMEngine::doAccuse() {
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	// Mirrors `_DoAccuse @ 1df2:0bdd` + `_DoAccuseGallery @ 1df2:0a31`:
	//   1. ACCUSE-NOTES SCREEN (PIC 0x1A7, the red "accuse-mode" BG):
	//      `_DrawNotes(_AccuseNoteRect, NULL, 100, _NoteSelected)`
	//      lists every found clue inside the rect at `29be:1048` =
	//      `(79, 27, 304, 159)`. `_UpdateSelectionCount(remaining)`
	//      shows "N clue(s)" at `(209, 11)` (ASM: `_Show_String(0xb,
	//      0xd1, ...)` at 1df2:0907). `_NoteUnselectedColor = 1` is
	//      the dim red used for unselected entries; selected ones
	//      get `0x3c`. Click toggles via `_SearchNoteAreas` +
	//      `_SwapColors`. Expected count = `6 - DAT_2d5d_3f99`
	//      (= 6 - chainStage):
	//          stage 1 → 5 clues, stage 2 → 4, stage 3 → 3.
	//      When the count matches, `_SolvedCheck` decides:
	//          fail → KD "not enough evidence" balloon → return.
	//          pass → `_DoAccuseGallery()` (suspect picker).
	//   2. KD intro balloon (`KDTextIndex[+8]` + `_SayKDDigital(4)`).
	//   3. `_GetBackground(0x3f)` + `_DrawGallery()` — portraits at
	//      the 5 fixed slots (`29be:0x116`).
	//   4. Click loop on portraits → `_WITCH(picked)` → guilty/alibi.
	const uint8 num = _mystery.numSuspects();
	if (num == 0)
		return;

	const byte *gd = _mystery.galleryData();

	// ACCUSE-NOTES SCREEN — let the player commit which N clues they
	// believe solve the case. Mirrors the click-driven selection of
	// `_DoAccuse @ 1df2:0bdd`'s outer loop. ESC / cancel returns to
	// the site (matches `_DoAccuse @ 1df2:0c11` writing
	// `_NextScreen = 3` on ESC).
	if (!doAccuseNotes()) {
		_nextScreen = _lastScreen != kScreenInvalid
						? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	// Evidence gate. `_DoAccuse @ 1df2:0c75` runs `_SolvedCheck`
	// before opening the suspect picker; on failure it renders a
	// partner balloon over the CURRENT screen (the PDA in the
	// original) and returns. We render the same hint over the
	// caller's screen and bail back to `_lastScreen` without ever
	// touching the gallery BG.
	if (!_mystery.solvedCheck()) {
		const byte *kdIdx = _mystery.kdTextIndex();
		const int16 hintOff = kdIdx
			? (int16)READ_LE_UINT16(kdIdx + 6)
			: -1;
		Common::String hint;
		if (hintOff != -1)
			hint = parseString(_mystery.textAt((uint16)hintOff),
							   _playerName, _partner);
		if (hint.empty()) {
			// Fallback if `KDTextIndex[+6]` isn't set in this mystery.
			hint = (_mystery.selectedPoints() == 0)
				? "We're not ready to solve this mystery yet. "
				  "Let's keep investigating until we have some "
				  "more solid evidence."
				: "We don't have quite enough evidence yet. "
				  "Let's review our notes and find a few more "
				  "clues before we accuse anyone.";
		}

		// Compose balloon overlay on the current screen. Mirrors the
		// `_GetKDTextBalloon` + `_GetBalloon` + `_AddPicBackground`
		// + `_WordWrap` sequence at 1df2:0c8d-0cd1.
		Graphics::ManagedSurface ms(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		ms.clear();
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			for (int row = 0; row < 200; row++)
				memcpy((byte *)ms.getBasePtr(0, row),
					   (const byte *)cur->getBasePtr(0, row), 320);
			g_system->unlockScreen();
		}
		const byte firstChar =
			hint.empty() ? (byte)0 : (byte)hint[0];
		const uint16 bubNum = getKDTextBalloon(firstChar);
		// Strip the digit prefix used for balloon dispatch — it's
		// consumed by the original at `_DisplayAlibi @ 1df2:0163`
		// (`str = pbVar7 + 1`) and shouldn't appear in the rendered
		// text. `_GetKDTextBalloon` itself doesn't advance past it.
		if (firstChar >= '0' && firstChar <= '9')
			hint.deleteChar(0);
		Picture balloon;
		const bool haveBalloon =
			_balloonArchive.size() > (bubNum & 0x7F) &&
			_balloonArchive.loadEntry(bubNum & 0x7F, balloon);
		const int balloonX = 0x21;
		int balloonY = 1;
		if (haveBalloon && balloon.surface.h < 0x4e)
			balloonY = (0x50 - balloon.surface.h) / 2;
		if (haveBalloon) {
			const byte transp = (byte)(balloon.flags >> 8);
			ms.transBlitFrom(balloon.surface,
							 Common::Point(balloonX, balloonY),
							 (uint32)transp);
		}
		uint16 tx = 5, ty = 4, tw = 155;
		getBalloonInsets(bubNum, tx, ty, tw);
		if (_font.isLoaded()) {
			_font.drawWordWrapped(&ms, balloonX + tx,
								  balloonY + ty, tw, hint,
								  haveBalloon ? 0 : 0xF);
		}
		g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		// `_DoAccuse @ 1df2:0cd9` plays `_SayKDDigital(3)` —
		// partner-specific "not enough evidence" voice line.
		if (_audio && kdIdx)
			_audio->sayKDDigital(kdIdx, 3, _partner);

		waitForInput(20000);
		// `_DoAccuse @ 1df2:0ce5` writes `_NextScreen = _LastScreen`
		// so the player drops back where they came from.
		_nextScreen = _lastScreen != kScreenInvalid
						? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	// Verbatim from 29be:0x116 — same five suspect slot positions as
	// `_DrawGallery @ 158f:0046`.
	Picture accuseBg;
	const bool haveAccuseBg = _picsArchive.getPicture(0x3f, accuseBg);

	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect;
	slotRects.resize(num);
	slotSuspect.resize(num);
	for (uint i = 0; i < num; i++)
		slotSuspect[i] = -1;

	int highlighted = 0;

	// Step 1 — KD hint balloon. Mirrors `_DoAccuseGallery @ 1df2:0a31`
	// (1df2:0a4c-1df2:0afe):
	//   text  = TextBlock + KDTextIndex[+8]               (1df2:0a4c-0a57)
	//   bub   = _GetKDTextBalloon(text[0])                (1df2:0a6d)
	//   GetBalloon(bub)                                   (1df2:0a7c)
	//   y     = (h < 0x4e) ? (0x50 - h) >> 1 : 1          (1df2:0a8b-0aa5)
	//   AddPicBackground(pic, 0x21, y)                    (1df2:0aab)
	//   WordWrap(0x21+tbl[bub].x, y+tbl[bub].y, tbl[bub].w, text, color=0)
	//     tbl @ 29be:0875, 10-byte entries (1df2:0ad6-0af1)
	const byte *kdIdx = _mystery.kdTextIndex();
	if (kdIdx) {
		const int16 textOff = (int16)READ_LE_UINT16(kdIdx + 8);
		if (textOff != -1) {
			const char *raw = _mystery.textAt((uint16)textOff);
			Common::String hint =
				parseString(raw ? raw : "", _playerName, _partner);
			if (!hint.empty()) {
				// First-char dispatch via getKDTextBalloon (1df2:0105).
				// Note: we pass the *parsed* first char; the original
				// reads it BEFORE `_ParseString`, but the player-name /
				// partner-name substitutions never start with digits, so
				// the dispatch result is the same either way.
				const byte firstChar =
					hint.empty() ? (byte)0 : (byte)hint[0];
				const uint16 bubNum = getKDTextBalloon(firstChar);
				// Strip the digit prefix used for balloon dispatch.
				// `_DisplayAlibi @ 1df2:0163` does `str = pbVar7 + 1`
				// after using `*str` for `bindx`. Same pattern used by
				// `_DisplayHint`: digit picks the bubble shape AND is
				// then consumed from the rendered text. Without this
				// the intro balloon shows e.g. "1Ready to solve?".
				if (firstChar >= '0' && firstChar <= '9')
					hint.deleteChar(0);
				Picture balloon;
				const bool haveBalloon =
					_balloonArchive.size() > (bubNum & 0x7F) &&
					_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

				// 1df2:0a8b-1df2:0aa5: y = (h < 0x4e) ? (0x50-h)>>1 : 1
				const int balloonX = 0x21;
				int balloonY = 1;
				if (haveBalloon && balloon.surface.h < 0x4e)
					balloonY = (0x50 - balloon.surface.h) / 2;

				// Render the gallery FIRST so the balloon snapshot
				// includes the partner sprite. The original
				// `_DoAccuseGallery @ 1df2:0a31` does this implicitly:
				// `_NewAnimation` registered the partner slot at
				// (5, 0x50) before reaching this point, then
				// `_GetBackground(0x3f)` + `_DrawGallery` paint the
				// portraits, and `_UpdateAnimations` keeps the partner
				// visible underneath the balloon overlay. Without this,
				// the player sees an 8-second partner-less screen
				// while reading the hint.
				drawAccuseGallery(num, gd, /*highlighted=*/-1,
								  slotRects, slotSuspect);

				Graphics::ManagedSurface ms(320, 200,
					Graphics::PixelFormat::createFormatCLUT8());
				ms.clear();
				{
					Graphics::Surface *cur = g_system->lockScreen();
					if (cur) {
						for (int row = 0; row < 200; row++)
							memcpy((byte *)ms.getBasePtr(0, row),
								   (const byte *)cur->getBasePtr(0, row), 320);
						g_system->unlockScreen();
					} else if (haveAccuseBg) {
						// Fallback: lockScreen failed somehow; at least
						// fill from PIC 0x3f so we don't render against
						// stale memory.
						const int bw = MIN<int>(accuseBg.surface.w, 320);
						const int bh = MIN<int>(accuseBg.surface.h, 200);
						for (int row = 0; row < bh; row++) {
							memcpy((byte *)ms.getBasePtr(0, row),
								   (const byte *)accuseBg.surface.getBasePtr(0, row), bw);
						}
					}
				}
				// Masked balloon blit — `_Rect_Move_Mask` (1000:03fc)
				// skips pixels equal to `pic[0] >> 8`.
				if (haveBalloon) {
					const byte transp = (byte)(balloon.flags >> 8);
					const int bw = MIN<int>(balloon.surface.w, 320 - balloonX);
					const int bh = MIN<int>(balloon.surface.h, 200 - balloonY);
					for (int row = 0; row < bh; row++) {
						const byte *src = (const byte *)balloon.surface.getBasePtr(0, row);
						byte *dst = (byte *)ms.getBasePtr(balloonX, balloonY + row);
						for (int col = 0; col < bw; col++) {
							if (src[col] != transp)
								dst[col] = src[col];
						}
					}
				}
				// Inset table @ 29be:0875 — 1df2:0acb pushes color=0.
				uint16 tx = 5;
				uint16 ty = 4;
				uint16 tw = 155;
				getBalloonInsets(bubNum, tx, ty, tw);
				if (_font.isLoaded()) {
					_font.drawWordWrapped(&ms, balloonX + tx,
										  balloonY + ty, tw, hint,
										  haveBalloon ? 0 : 0xF);
				}
				g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
					0, 0, 320, 200);
				g_system->updateScreen();
				waitForInput(8000);
			}
		}
	}

	// Helper to find the next "alive" slot (one whose `_inGallery[phys]`
	// flag is still set so a portrait was actually drawn). Mirrors the
	// way the original wraps DI past empty slots.
	if (slotRects[highlighted].isEmpty())
		highlighted = nextLiveSlot(slotRects, highlighted, +1);

	drawAccuseGallery(num, gd, highlighted, slotRects, slotSuspect);

	// Wait-for-pick loop. Mirrors `_DoAccuseGallery` 1df2:0b26-1df2:0bc8:
	//   * `_CheckFrameRate` + `_UpdateAnimations` per tick (1df2:0b2a-0b33)
	//   * 5-entry input dispatch table @ 1df2:0bc9:
	//       0x09 (TAB)   → handler 0x0b94 (cycle highlight)
	//       0x0d (Enter) → handler 0x0b72 (pick = _SearchSuspects)
	//       0x4b (LEFT)  → handler 0x0b94
	//       0x4d (RIGHT) → handler 0x0b94
	//       0xFFFF (mb)  → handler 0x0b72
	//   * 0x0b94: `INC DI` + wraparound + `_PutMouseInRect(&Guys[DI])`,
	//     i.e. advance highlight and warp cursor (1df2:0b94-0bb1).
	//   * 0x0b72: `_SearchSuspects` (158f:0584) — mouse-rect hit-test;
	//     if non-0xFFFF, pick that suspect.
	// We don't warp the cursor (unfriendly under SDL); instead the
	// highlight is drawn as a 1px outline and Enter picks it.
	int picked = -1;
	uint32 lastTick = g_system->getMillis();
	bool dirty = false;
	while (picked < 0 && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					return;
				case Common::KEYCODE_TAB:
				case Common::KEYCODE_RIGHT:
					highlighted = nextLiveSlot(slotRects, highlighted, +1);
					dirty = true;
					break;
				case Common::KEYCODE_LEFT:
					// 1df2:0b94 increments DI for LEFT too — but a
					// keyboard-driven UX is friendlier with separate
					// directions, so we mirror Right=+1 / Left=-1.
					highlighted = nextLiveSlot(slotRects, highlighted, -1);
					dirty = true;
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					if (highlighted >= 0 &&
						highlighted < (int)slotRects.size() &&
						!slotRects[highlighted].isEmpty()) {
						picked = highlighted;
					}
					break;
				default: {
					const int k = (int)ev.kbd.keycode;
					if (k >= Common::KEYCODE_1 && k <= Common::KEYCODE_9) {
						const int idx = k - Common::KEYCODE_1;
						if (idx < num &&
							!slotRects[idx].isEmpty())
							picked = idx;
					}
					break;
				}
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] < 0)
						continue;
					if (slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						picked = (int)i;
						break;
					}
				}
			}
		}
		// 100 ms tick — the original calls `_UpdateAnimations` per
		// `_CheckFrameRate` (1df2:0b33). The accuse screen has no
		// animations registered, so the tick is just a redraw cadence.
		// We still re-render whenever the highlight moves (`dirty`).
		const uint32 now = g_system->getMillis();
		if (dirty || now - lastTick >= 100) {
			drawAccuseGallery(num, gd, highlighted, slotRects, slotSuspect);
			lastTick = now;
			dirty = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (picked < 0)
		return;

	// Real chain evaluation. Mirrors the original two-gate accusation:
	//   1. `_AccuseEntry @ 1df2:0ff8` checks `_GetFoundPoints() >= 100`
	//      — gates whether the suspect picker is even reachable. We
	//      `_SolvedCheck → selectedPoints > 99` is now gated at the
	//      TOP of `doAccuse` — by the time we reach this point we
	//      already know `solvedCheck()` was true (the picker wouldn't
	//      have opened otherwise).
	//   2. `_WITCH @ 1df2:089f` checks `GalleryData[picked*0x46+0x02] ==
	//      0xFFFF`. Innocent suspects store an alibi-text TextBlock
	//      offset there; the guilty one uses the sentinel.
	const int points          = _mystery.selectedPoints();
	const bool pickedGuilty   = _mystery.isGuilty((uint)picked);
	const bool guessedRight   = pickedGuilty;
	debugC(1, kDebugScript,
		   "doAccuse: picked=%d selectedPts=%d guilty=%s -> %s",
		   picked, points,
		   pickedGuilty ? "yes" : "no",
		   guessedRight ? "correct" : "wrong");

	// Wrong suspect: full alibi flow. Mirrors `_DisplayAlibi @
	// 1df2:0145`:
	//   1. Plays MIDI 6 (loser sting) and waits for it to finish while
	//      the gallery is still on screen (1df2:0184-1df2:0192).
	//   2. Draws PIC 0x3e + the suspect's speech balloon + their
	//      portrait at (0x82, py), where the balloon shape comes from
	//      `AlibiBubbles[bindx]` (table @ 29be:1050) and bindx is the
	//      digit-prefix on the alibi text (else 2). bindx<8 centres the
	//      balloon horizontally; bindx>=8 pins it at x=0x21.
	//   3. Plays the suspect's voice via `_SpoolSound(talk - 1)` where
	//      `talk = (Partner == 0) ? gd[+0x6] : gd[+0x0]` (1df2:0258),
	//      then waits for a click.
	//   4. Overlays the partner's reaction balloon (text @
	//      `KDTextIndex[+10]`) at (0x21, y) and plays
	//      `_SayKDDigital(5)`.
	//   5. Clears `_FirstTry` (1df2:0447) and returns to LastScreen.
	if (!guessedRight) {
		// Balloon-shape table @ 29be:1050 — 16 entries × u16.
		static const uint16 kAlibiBubbles[16] = {
			0x002B, 0x002C, 0x002D, 0x002E,
			0x00AB, 0x00AC, 0x00AD, 0x00AE,
			0x001D, 0x001E, 0x0015, 0x0016,
			0x0017, 0x0018, 0x0019, 0x001A,
		};

		const uint16 alibiOff = _mystery.alibiTextOffset((uint)picked);
		Common::String alibi;
		if (gd && alibiOff != 0xFFFF) {
			const char *raw = _mystery.textAt(alibiOff);
			if (raw)
				alibi = parseString(raw, _playerName, _partner);
		}
		// Digit-prefix dispatch — `_DisplayAlibi @ 1df2:0163` reads
		// `*str` for `bindx` and advances `str = pbVar7 + 1` so the
		// digit doesn't reach the renderer. Non-digit first chars fall
		// through to the default bindx=2 (1df2:015e).
		uint bindx = 2;
		const byte firstChar = alibi.empty() ? (byte)0 : (byte)alibi[0];
		if (firstChar >= '0' && firstChar <= '9') {
			bindx = (uint)(firstChar - '0');
			alibi.deleteChar(0);
		}
		if (bindx >= 16)
			bindx = 2;
		const uint16 bubNum = kAlibiBubbles[bindx];

		Picture alibiBg;
		const bool haveAlibiBg = _picsArchive.getPicture(0x3e, alibiBg);
		Picture suspect;
		const uint16 picId = gd
			? READ_LE_UINT16(gd + (uint)picked * 0x46)
			: 0;
		const bool haveSuspect = picId != 0 &&
			_picsArchive.getPicture(picId, suspect);
		Picture balloon;
		const bool haveBalloon =
			_balloonArchive.size() > (bubNum & 0x7F) &&
			_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

		// Position math from 1df2:01a4-1df2:0207. py is the suspect
		// portrait's Y; defaults to 0x5a, only overridden in the
		// bindx<8 branch when the balloon is too tall to fit.
		int balloonX = 0x21;
		int balloonY = 1;
		int py = 0x5a;
		if (bindx < 8) {
			const int bw = haveBalloon ? balloon.surface.w : 0;
			const int bh = haveBalloon ? balloon.surface.h : 0;
			balloonX = (320 - bw) / 2;
			if (bh < 0x5a) {
				balloonY = (0x5a - bh) / 2;
			} else {
				balloonY = 1;
				py = bh;
			}
		} else {
			const int bh = haveBalloon ? balloon.surface.h : 0;
			balloonX = 0x21;
			balloonY = (bh < 0x4f) ? (0x50 - bh) / 2 : 1;
		}

		// `base` = BG + suspect + partner sprite — the persistent layer
		// that survives across both balloon phases. The original engine
		// keeps PIC 0x3e in the master BG buffer (16000), `_AddPicBackground`
		// commits the suspect there, and the partner animation
		// registered by `_DoAccuse @ 1df2:0c30` is re-blitted by every
		// `_Repaint` via `_DrawActiveAnimations`. We don't have a
		// slot-based animation system, so we manually keep a "base"
		// surface and re-draw the partner frame for each phase.
		Graphics::ManagedSurface base(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		base.clear();
		if (haveAlibiBg)
			base.simpleBlitFrom(alibiBg.surface);
		if (haveSuspect) {
			const byte transp = (byte)(suspect.flags >> 8);
			base.transBlitFrom(suspect.surface,
							   Common::Point(0x82, py),
							   (uint32)transp);
		}
		// Partner sprite at (5, 0x50). Anim cells: 2 (Jake) / 0x10
		// (Jenny); script key 0x02 — same indices `_DoAccuse @
		// 1df2:0c30` uses for its `_NewAnimation` call. Partner is
		// drawn AFTER the suspect so it doesn't get clipped by the
		// portrait if their bounding boxes graze.
		const uint partnerAnim = (_partner == 0) ? 2 : 0x10;
		Animation partnerAni;
		const bool havePartner =
			_aniArchive.loadAnimation(partnerAnim, partnerAni) &&
			!partnerAni.empty();

		// Alibi-phase scratch = base + alibi balloon + alibi text +
		// partner sprite (animation slot drawn on top per
		// `_DrawActiveAnimations`).
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(base);
		if (haveBalloon) {
			const byte transp = (byte)(balloon.flags >> 8);
			scratch.transBlitFrom(balloon.surface,
								  Common::Point(balloonX, balloonY),
								  (uint32)transp);
		}
		// Balloon-text inset table @ 29be:0875 — same dispatch as KD
		// balloons. WordWrap color is 0 inside a balloon (1df2:0240).
		uint16 tx = 5, ty = 4, tw = 155;
		getBalloonInsets(bubNum, tx, ty, tw);
		if (_font.isLoaded() && !alibi.empty()) {
			_font.drawWordWrapped(&scratch, balloonX + tx,
								  balloonY + ty, tw, alibi,
								  haveBalloon ? 0 : 0xF);
		}
		if (havePartner) {
			const uint frameIdx = partnerFrameAtTick(0x02,
				(uint)partnerAni.size(), g_system->getMillis());
			blitAnimFrameAnchored(scratch.surfacePtr(),
								  partnerAni[frameIdx], 5, 0x50);
		}

		// Step 1 — alibi music. Original blocks until MIDI 6 ends with
		// the gallery still on screen. We poll `_music->isPlaying`;
		// click/ESC aborts early.
		if (_music) {
			_music->playMus(6, /*loop=*/false);
			const uint32 musStart = g_system->getMillis();
			bool aborted = false;
			while (_music->isPlaying() && !shouldQuit() && !aborted) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					if (ev.type == Common::EVENT_QUIT ||
						ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
						return;
					if (ev.type == Common::EVENT_KEYDOWN ||
						ev.type == Common::EVENT_LBUTTONDOWN) {
						aborted = true;
						break;
					}
				}
				// Hard cap so we never get stuck if MIDI never reports
				// finish (some sound configurations).
				if (g_system->getMillis() - musStart > 10000)
					break;
				g_system->updateScreen();
				g_system->delayMillis(20);
			}
			_music->stop();
		}

		// Step 2 — flip the alibi scene to screen + play suspect voice.
		// `talk = (Partner==0) ? gd[+0x6] : gd[+0x0]` (1df2:0252-0258);
		// indices are 1-based so subtract 1 before SpoolSound.
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
		if (_audio && gd) {
			const uint16 alibiVoice =
				READ_LE_UINT16(gd + (uint)picked * 0x46 + 0x00);
			const uint16 jakeVoice =
				READ_LE_UINT16(gd + (uint)picked * 0x46 + 0x06);
			const uint16 talk =
				(_partner == 0) ? jakeVoice : alibiVoice;
			if (talk != 0)
				_audio->spoolSound((uint)(talk - 1));
		}
		waitForInput(60000);

		// Step 3 — partner reaction balloon. Mirrors 1df2:026e-1df2:02b6.
		// Rebuild scratch from `base` (BG + suspect + partner sprite)
		// so the alibi balloon + text don't bleed through. The
		// original's `_Repaint` re-renders the master BG (which still
		// has the alibi balloon committed), but the engine ALSO calls
		// `_GetBackground(0x3e)` again before `_AddPicBackground` for
		// each new balloon — flushing the master back to a clean state.
		// We achieve the same end result by restoring `base` here.
		// `_SayKDDigital(5)` auto-cancels the still-playing alibi voice
		// (spoolSound calls stopSpool internally) so no explicit stop.
		const byte *reactIdx = _mystery.kdTextIndex();
		if (reactIdx) {
			const int16 reactOff = (int16)READ_LE_UINT16(reactIdx + 10);
			Common::String react;
			if (reactOff != -1) {
				const char *raw = _mystery.textAt((uint16)reactOff);
				if (raw)
					react = parseString(raw, _playerName, _partner);
			}
			if (!react.empty()) {
				const byte rChar = (byte)react[0];
				const uint16 rBub = getKDTextBalloon(rChar);
				if (rChar >= '0' && rChar <= '9')
					react.deleteChar(0);
				Picture rBalloon;
				const bool haveR =
					_balloonArchive.size() > (rBub & 0x7F) &&
					_balloonArchive.loadEntry(rBub & 0x7F, rBalloon);
				const int rX = 0x21;
				int rY = 1;
				if (haveR && rBalloon.surface.h < 0x4e)
					rY = (0x50 - rBalloon.surface.h) / 2;

				// Reset to a clean BG + suspect, then layer the new
				// balloon and the (refreshed) partner frame on top.
				scratch.simpleBlitFrom(base);
				if (haveR) {
					const byte transp = (byte)(rBalloon.flags >> 8);
					scratch.transBlitFrom(rBalloon.surface,
										   Common::Point(rX, rY),
										   (uint32)transp);
				}
				uint16 rtx = 5, rty = 4, rtw = 155;
				getBalloonInsets(rBub, rtx, rty, rtw);
				if (_font.isLoaded()) {
					_font.drawWordWrapped(&scratch, rX + rtx,
										  rY + rty, rtw, react,
										  haveR ? 0 : 0xF);
				}
				if (havePartner) {
					const uint frameIdx = partnerFrameAtTick(0x02,
						(uint)partnerAni.size(),
						g_system->getMillis());
					blitAnimFrameAnchored(scratch.surfacePtr(),
										  partnerAni[frameIdx],
										  5, 0x50);
				}
				g_system->copyRectToScreen(scratch.getPixels(),
					scratch.pitch, 0, 0, 320, 200);
				g_system->updateScreen();
				if (_audio)
					_audio->sayKDDigital(reactIdx, 5, _partner);
			}
		}
		waitForInput(60000);

		_mystery._firstTry = false;
		// `_DisplayAlibi @ 1df2:043f` writes `_NextScreen =
		// _LastScreen`. The original returns the player to the caller
		// (PDA / site / map) for another try.
		_nextScreen = _lastScreen != kScreenInvalid
						? (ScreenId)_lastScreen : kScreenSite;
		return;
	}

	// Right suspect — full win flow. Mirrors `_DisplayCorrect @
	// 1df2:073c`: mark mystery solved, advance chain stage if the
	// tier is complete, swap MIDI to the win cue, run SCRAPBK.ANI,
	// show the per-mystery ending, save the profile, return to the
	// action menu (`_NextScreen = 0xc` at 1df2:0895).
	{
		const uint mn = _mystery.number();
		if (mn < sizeof(_mysteriesSolved)) {
			_mysteriesSolved[mn] = _mystery._firstTry ? 2 : 1;
		}

		// Mirrors the chain-advancement loop at `_DisplayCorrect @
		// 1df2:0824-0850`. Skip mystery 0 (the practice case) per the
		// `if (_MysteryNumber != 0)` guard at 1df2:080d, then check
		// every mystery in the current tier:
		//   stage 1 → check  1..0x18 (24 mysteries, "A chain")
		//   stage 2 → check 0x19..0x30 (24 mysteries, "B chain")
		//   stage 3 → check 0x31..0x36 (6 mysteries, "C chain")
		// If every solve flag in that range is non-zero, bump the
		// stage. Original increments unconditionally (3→4 is harmless
		// since no further range covers it); we cap at 3 for clarity.
		if (mn != 0) {
			uint lo = 0, hi = 0;
			switch (_chainStage) {
			case 1: lo = 1;    hi = 0x18; break;
			case 2: lo = 0x19; hi = 0x30; break;
			case 3: lo = 0x31; hi = 0x36; break;
			default: break;
			}
			bool allSolved = (hi >= lo);
			for (uint i = lo; i <= hi && allSolved; i++) {
				if (i >= sizeof(_mysteriesSolved) || _mysteriesSolved[i] == 0)
					allSolved = false;
			}
			// `_DisplayCorrect @ 1df2:0852` increments unconditionally
			// when every case in the current tier is solved — including
			// past stage 3 (so a stage-4 endgame state exists in the
			// `.PLR` save format). `_ActionScreen @ 1c33:19d1` gates
			// the menu on `if (3 < _chainStage)` to grey
			// Choose-A-Mystery and Practice once everything's solved.
			if (allSolved && _chainStage < 4) {
				_chainStage++;
				debugC(1, kDebugMystery,
					   "chainStage advanced to %u after solving mystery %u",
					   _chainStage, mn);
			}
		}

		// `_DisplayCorrect @ 1df2:073c` order:
		//   1df2:0773  _AllBlack();
		//   1df2:0776  _BuildBackground(5, 0x42, 0x14);  // conclusion BG
		//   1df2:0780  _FadeIn();
		//   1df2:0789  _MIDIPlay(5);                      // win music
		//   1df2:07ac  _DisplayClue(MysteryIndex[+0x10]); // chain recap
		// `_BuildBackground @ 172b:13e2` loads PIC 0x3D (the standard
		// frame) and overlays SITES.DBD entry 5 at (0x42, 0x14), then
		// sets palette via `_GetPalette(sitenum + 1)` = palette 6.
		// Without this BG the chain-recap balloons render on top of
		// the accuse-gallery BG (PIC 0x3F + suspect portraits), which
		// is visually jarring — the conclusion is supposed to play
		// against the dedicated "office / desk" scene.
		Graphics::Surface *blk = g_system->lockScreen();
		if (blk) {
			memset(blk->getPixels(), 0, 320 * 200);
			g_system->unlockScreen();
		}
		setSitePalette(6); // sitenum + 1 per `_GetPalette` call
		Picture frame, scene;
		if (_picsArchive.loadEntry(0x3d, frame)) {
			g_system->copyRectToScreen(frame.surface.getPixels(),
									   frame.surface.pitch, 0, 0,
									   frame.surface.w, frame.surface.h);
		}
		if (5 < _sitesArchive.size() &&
			_sitesArchive.loadEntry(5, scene)) {
			const int sx = 0x42, sy = 0x14;
			const int sw = MIN<int>(scene.surface.w, 320 - sx);
			const int sh = MIN<int>(scene.surface.h, 200 - sy);
			if (sw > 0 && sh > 0)
				g_system->copyRectToScreen(scene.surface.getPixels(),
										   scene.surface.pitch, sx, sy,
										   sw, sh);
		}

		// Partner sprite at (5, 0x50). The original `_DoAccuse @
		// 1df2:0c30` registered the partner anim BEFORE entering the
		// gallery; that slot stays active across `_DisplayCorrect`'s
		// `_BuildBackground` (which calls `_Repaint` →
		// `_DrawActiveAnimations` and re-blits the partner over the
		// fresh BG). We don't have a slot system, so manually stamp
		// the resting frame here. `displayClue` snapshots the screen
		// on entry, so the partner ends up baked into its BG and is
		// preserved across every clue iteration.
		const uint partnerAnim = (_partner == 0) ? 2 : 0x10;
		Animation partnerAni;
		if (_aniArchive.loadAnimation(partnerAnim, partnerAni) &&
			!partnerAni.empty()) {
			Graphics::Surface *screen = g_system->lockScreen();
			if (screen) {
				const uint frameIdx = partnerFrameAtTick(0x02,
					(uint)partnerAni.size(),
					g_system->getMillis());
				blitAnimFrameAnchored(screen, partnerAni[frameIdx],
									  5, 0x50);
				g_system->unlockScreen();
			}
		}
		g_system->updateScreen();

		if (_music)
			_music->playMus(5, /*loop=*/false);

		// Chain-by-chain RECAP. Partner enumerates every required
		// clue ("Look at this — the suspect was here at 8pm", "... and
		// remember the broken vase from the kitchen", "... so it had
		// to be X!") and arrives at the conclusion. Without rendering
		// it the player goes straight from suspect-pick to the
		// scrapbook anim and misses the deduction entirely.
		const byte *solved = _mystery.solvedClueBlock();
		if (solved)
			displayClue(solved);

		// `_DifferenceAnimation("scrapbk.ani")` (1df2:0848) — the
		// physical scrapbook flip animation that introduces the
		// per-mystery ending pages.
		playAnm(Common::Path("SCRAPBK.ANI"), 120, true);

		// `_ShowOneScrap @ 1f78:0773` is `_DisplayEnding(num, 1)` —
		// the multi-page per-mystery ending narrative.
		doShowEnding(mn);

		// Mirrors `_SavePlayerRecord` at 1df2:0857 — once the
		// `_mysteriesSolved` table is updated, the original
		// immediately persists the player record so the win sticks
		// even if the player quits before reaching the menu.
		//
		// Order matters: `_mystery.clear()` BEFORE `saveProfile` so the
		// save records `hasMystery=false`. Otherwise the next load of
		// this profile sees the just-won mystery still loaded and the
		// screen driver routes to its map (forcing the player to
		// replay the win flow). Mirrors `_DisplayCorrect @ 1df2:0851`
		// (`_DeleteSavedGame` removes the in-progress save before
		// `_SavePlayerRecord` writes the post-win profile).
		_mystery.clear();
		const Common::Error err = saveProfile(_playerName);
		if (err.getCode() != Common::kNoError)
			warning("saveProfile after solve failed: %s",
					err.getDesc().c_str());

		// `_DisplayCorrect @ 1df2:0895` writes `_NextScreen = 0xc` —
		// the winner returns to the post-mystery `_ActionScreen`.
		_nextScreen = kScreenAction;
	}
}

void EEMEngine::drawAccuseGallery(uint8 numSuspects, const byte *gd,
								   int highlighted,
								   Common::Array<Common::Rect> &slotRects,
								   Common::Array<int> &slotSuspect) {
	// Accuse-gallery redraw — formerly the `drawGallery` lambda inside
	// `doAccuse`. Mirrors `_DoAccuseGallery @ 1df2:0a31` portrait grid:
	// PIC 0x3f backdrop, suspect portraits at the 5 fixed slots
	// (`kGallerySlots` in this file's anon namespace), and a 1-px
	// outline (palette index 0xFE) around the highlighted slot.
	//
	// Partner sprite at (5, 0x50): the original `_DoAccuse @ 1df2:0bdd`
	// registers `_NewAnimation(5, 0x50, partnerCells, script=2, prior=1)`
	// (1df2:0c30) BEFORE calling `_DoAccuseGallery`, then `_DrawGallery`
	// calls `_DrawActiveAnimations` (158f:00a3) which re-renders the
	// slot every frame. Without an explicit blit here, our port's
	// accuse screen comes out partner-less. Anim CELLS are 2 (Jake) /
	// 0x10 (Jenny); SCRIPT key is 0x02 for both partners (matches the
	// `CONCAT22(2, ...)` arg verified at 1df2:0c2e).
	Picture accuseBg;
	const bool haveAccuseBg = _picsArchive.getPicture(0x3f, accuseBg);

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (haveAccuseBg) {
		const int bw = MIN<int>(accuseBg.surface.w, 320);
		const int bh = MIN<int>(accuseBg.surface.h, 200);
		for (int row = 0; row < bh; row++) {
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)accuseBg.surface.getBasePtr(0, row), bw);
		}
	}

	// Partner sprite, drawn BEFORE portraits so the suspect grid
	// covers it where they overlap (the gallery slots start at
	// y=14 / y=90, partner is at y=0x50=80 — no overlap, so order
	// is purely defensive).
	const uint partnerAnim = (_partner == 0) ? 2 : 0x10;
	Animation partnerAni;
	if (_aniArchive.loadAnimation(partnerAnim, partnerAni) &&
		!partnerAni.empty()) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = partnerFrameAtTick(0x02,
												  (uint)partnerAni.size(), now);
		blitAnimFrameAnchored(scratch.surfacePtr(),
							  partnerAni[frameIdx], 5, 0x50);
	}

	for (uint i = 0; i < numSuspects && i < Mystery::kGalleryCap; i++) {
		slotRects[i] = Common::Rect();
		slotSuspect[i] = -1;
		if (!gd)
			continue;
		const uint8 phys = _mystery._newOrder[i];
		if (phys >= 5)
			continue;
		// `_DrawGallery @ 158f:00b9` skips suspects whose
		// `_InGallery[phys]` flag is 0 — that's the original gate.
		if (_mystery._inGallery[phys] == 0)
			continue;
		const GallerySlot &s = kGallerySlots[phys];

		const uint16 picId = READ_LE_UINT16(gd + i * 0x46);
		if (picId == 0)
			continue;
		Picture portrait;
		if (!_picsArchive.getPicture(picId, portrait))
			continue;

		const int placeX = s.x;
		const int placeY = s.y + (0x48 - portrait.surface.h);
		const byte transp = (byte)(portrait.flags >> 8);
		const int w = MIN<int>(portrait.surface.w, 320 - placeX);
		const int h = MIN<int>(portrait.surface.h, 200 - placeY);
		if (w <= 0 || h <= 0)
			continue;
		for (int row = 0; row < h; row++) {
			const int dstY = placeY + row;
			if (dstY < 0)
				continue;
			const byte *src =
				(const byte *)portrait.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < w; col++) {
				const int dstX = placeX + col;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
		slotRects[i] = Common::Rect(placeX, placeY,
									 placeX + w, placeY + h);
		slotSuspect[i] = (int)i;
	}

	// Highlight indicator. The original moves the mouse cursor to the
	// centre of the highlighted suspect via `_PutMouseInRect` (1df2:0b8e);
	// we draw a 1-px outline in palette index 0xFE instead, which sits
	// inside the marching-ants cycle range 0xF9..0xFE and is visible
	// under any palette without warping the player's cursor.
	if (highlighted >= 0 && highlighted < (int)slotRects.size() &&
		!slotRects[highlighted].isEmpty()) {
		Common::Rect r = slotRects[highlighted];
		r.grow(1);
		scratch.frameRect(r, 0xFE);
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

} // End of namespace EEM
