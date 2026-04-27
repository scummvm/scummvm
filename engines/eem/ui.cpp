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

#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/music.h"

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
	int kdAnimX;
	int kdAnimY;
	const char *separator;
	const char *const *pickLabel;
	const bool *pickEnabled;
	uint pick;
};

// Per-mystery sub-chooser ("Choose A Mystery") view.
struct CaseSubmenuView {
	EEMEngine *vm;
	const Picture *caseBg;
	bool haveCaseBg;
	const byte *mysteriesSolved;
	uint mysteriesSolvedSize;
	uint sel;
	uint maxMystery;
};

void drawCaseSubmenu(const CaseSubmenuView &v) {
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (v.haveCaseBg) {
		const int w = MIN<int>(v.caseBg->surface.w, 320);
		const int h = MIN<int>(v.caseBg->surface.h, 200);
		for (int row = 0; row < h; row++)
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)v.caseBg->surface.getBasePtr(0, row), w);
	}
	if (v.vm->getFont().isLoaded()) {
		const int kListX  = 61;
		const int kListW  = 238 - kListX;
		const int kListY0 = 35;
		const int kLineH  = 10;
		const int kVisible = 12;
		int top = (int)v.sel - kVisible / 2;
		if (top < 0)
			top = 0;
		if (top + kVisible > (int)v.maxMystery + 1)
			top = (int)v.maxMystery + 1 - kVisible;
		for (int r = 0; r < kVisible; r++) {
			const int idx = top + r;
			if (idx > (int)v.maxMystery)
				break;
			char marker = ' ';
			if ((uint)idx < v.mysteriesSolvedSize) {
				if (v.mysteriesSolved[idx] == 2)
					marker = '*';
				else if (v.mysteriesSolved[idx] == 1)
					marker = '+';
			}
			const char arrow = ((uint)idx == v.sel) ? '>' : ' ';
			v.vm->getFont().drawString(&scratch,
				Common::String::format("%c %c Mystery %d", arrow, marker, idx),
				kListX, kListY0 + r * kLineH, kListW, 0xF);
		}
	}
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

void drawCaseSelectionFrame(const CaseSelectionView &v) {
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	if (v.haveCaseBg) {
		const int w = MIN<int>(v.caseBg->surface.w, 320);
		const int h = MIN<int>(v.caseBg->surface.h, 200);
		for (int row = 0; row < h; row++) {
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)v.caseBg->surface.getBasePtr(0, row), w);
		}
	}

	// KD greeter frame — masked-blit current animation cell at
	// (0x112, 0x50). 100 ms tick matches the engine's `_CheckFrameRate`.
	if (v.haveKdAnim) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = (uint)((now / 100) % v.kdAnim->size());
		const Picture &fr = (*v.kdAnim)[frameIdx];
		const byte transp = (byte)(fr.flags >> 8);
		for (int row = 0; row < fr.surface.h; row++) {
			const int dstY = v.kdAnimY + row;
			if (dstY < 0 || dstY >= 200)
				continue;
			const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < fr.surface.w; col++) {
				const int dstX = v.kdAnimX + col;
				if (dstX < 0 || dstX >= 320)
					continue;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
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
	if (haveBG) {
		const int w = MIN<int>(bg.surface.w, 320);
		const int h = MIN<int>(bg.surface.h, 200);
		for (int row = 0; row < h; row++)
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)bg.surface.getBasePtr(0, row), w);
	}
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
				_playerName = name;
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
			if (haveBG) {
				const int w = MIN<int>(bg.surface.w, 320);
				const int h = MIN<int>(bg.surface.h, 200);
				for (int row = 0; row < h; row++)
					memcpy((byte *)scratch.getBasePtr(0, row),
						   (const byte *)bg.surface.getBasePtr(0, row), w);
			}
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
	// ScrapBooks aren't implemented yet — grey them so the player can't
	// stop on them, mirroring the original `_Greys` mask.
	const bool kPickEnabled[kNumPicks] = { true, true, false, false, false };
	uint pick = kPickChoose;

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
			if (k == Common::KEYCODE_RETURN) {
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
		}
		return;
	}

	if (pick != kPickChoose) {
		// ScrapBooks aren't implemented; bail back to the menu loop.
		_mystery.clear();
		return;
	}

	// "Choose A Mystery" sub-screen: pick a specific case from the
	// 55-mystery roster. The original opens a different list here;
	// we approximate with the tier-aware numeric chooser we used
	// before. Default to the first unsolved mystery.
	uint sel = 0;
	for (uint i = 0; i <= kMaxMystery; i++) {
		if (i < sizeof(_mysteriesSolved) && !_mysteriesSolved[i]) {
			sel = i;
			break;
		}
	}

	CaseSubmenuView sv;
	sv.vm = this;
	sv.caseBg = &caseBg;
	sv.haveCaseBg = haveCaseBg;
	sv.mysteriesSolved = _mysteriesSolved;
	sv.mysteriesSolvedSize = sizeof(_mysteriesSolved);
	sv.sel = sel;
	sv.maxMystery = kMaxMystery;

	drawCaseSubmenu(sv);
	bool confirmed = false;
	while (!confirmed && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Same `_DoChoose` rectangles as the top-level menu.
				if (kOkRect.contains(ev.mouse.x, ev.mouse.y)) {
					confirmed = true;
					break;
				}
				if (kExitRect.contains(ev.mouse.x, ev.mouse.y)) {
					_mystery.clear();
					return;
				}
				if (kUpArrowRect.contains(ev.mouse.x, ev.mouse.y)) {
					sel = (sel == 0) ? kMaxMystery : sel - 1;
					sv.sel = sel;
					drawCaseSubmenu(sv);
					continue;
				}
				if (kDnArrowRect.contains(ev.mouse.x, ev.mouse.y)) {
					sel = (sel >= kMaxMystery) ? 0 : sel + 1;
					sv.sel = sel;
					drawCaseSubmenu(sv);
					continue;
				}
				if (kListRect.contains(ev.mouse.x, ev.mouse.y)) {
					// Pick the row under the cursor.
					const int kLineH = 10;
					const int kVisible = 12;
					int top = (int)sel - kVisible / 2;
					if (top < 0)
						top = 0;
					if (top + kVisible > (int)kMaxMystery + 1)
						top = (int)kMaxMystery + 1 - kVisible;
					const int row = (ev.mouse.y - kListRect.top) / kLineH;
					const int idx = top + row;
					if (idx >= 0 && idx <= (int)kMaxMystery) {
						sel = (uint)idx;
						sv.sel = sel;
						drawCaseSubmenu(sv);
					}
					continue;
				}
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				_mystery.clear();
				return;
			}
			if (k == Common::KEYCODE_RETURN) {
				confirmed = true;
				break;
			}
			if (k >= Common::KEYCODE_0 && k <= Common::KEYCODE_9) {
				sel = (uint)(k - Common::KEYCODE_0);
				sv.sel = sel;
				drawCaseSubmenu(sv);
				continue;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_TAB) {
				sel = (sel >= kMaxMystery) ? 0 : sel + 1;
				sv.sel = sel;
				drawCaseSubmenu(sv);
				continue;
			}
			if (k == Common::KEYCODE_UP) {
				sel = (sel == 0) ? kMaxMystery : sel - 1;
				sv.sel = sel;
				drawCaseSubmenu(sv);
				continue;
			}
			if (k == Common::KEYCODE_PAGEDOWN) {
				sel = (sel + 10 > kMaxMystery) ? kMaxMystery : sel + 10;
				sv.sel = sel;
				drawCaseSubmenu(sv);
				continue;
			}
			if (k == Common::KEYCODE_PAGEUP) {
				sel = (sel < 10) ? 0 : sel - 10;
				sv.sel = sel;
				drawCaseSubmenu(sv);
				continue;
			}
			if (k == Common::KEYCODE_HOME) { sel = 0; sv.sel = sel; drawCaseSubmenu(sv); continue; }
			if (k == Common::KEYCODE_END)  { sel = kMaxMystery; sv.sel = sel; drawCaseSubmenu(sv); continue; }
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (!_mystery.load(sel, &_rng)) {
		warning("doCaseSelection: failed to load mystery %u", sel);
		_mystery.clear();
		return;
	}
	debugC(1, kDebugMystery, "Mystery %u loaded; %u sites, %u suspects",
		   sel, _mystery.numSites(), _mystery.numSuspects());
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
	const Common::Rect kNoteArea   ( 66,  79, 267, 174);  // [10] note area

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
				} else if (ev.kbd.keycode == Common::KEYCODE_h) {
					doHelp();
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
				if (kNoteArea.contains(ev.mouse.x, ev.mouse.y)) {
					// Toggle the selection on whichever clue's text
					// the click landed in. The original calls
					// `_InterfaceHelp` here; that's the help screen,
					// not selection — selection is in the Accuse
					// screen. We use the area for selection because
					// keyboard 1..9 toggling is awkward, and the
					// resulting `_NoteSelected` state is what
					// `_SolvedCheck` reads.
					for (uint i = 0; i < _notebookSlotRects.size(); i++) {
						if (_notebookSlotRects[i].contains(ev.mouse.x,
														   ev.mouse.y)) {
							const uint clueId = _notebookSlotClues[i];
							_mystery._noteSelected[clueId] ^= 1;
							dirty = true;
							break;
						}
					}
					continue;
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
	if (_picsArchive.getPicture(0x3f, frame)) {
		const int w = MIN<int>(frame.surface.w, 320);
		const int h = MIN<int>(frame.surface.h, 200);
		for (int row = 0; row < h; row++)
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)frame.surface.getBasePtr(0, row), w);
	}

	// Partner sprite at (5, 80). Anim 1 for Jake, 0xb (11) for Jenny.
	const uint partnerAnim = (_partner == 0) ? 1 : 0xb;
	Animation partnerAni;
	if (_aniArchive.loadAnimation(partnerAnim, partnerAni) && !partnerAni.empty()) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = (uint)((now / 100) % partnerAni.size());
		const Picture &fr = partnerAni[frameIdx];
		const byte transp = (byte)(fr.flags >> 8);
		for (int row = 0; row < fr.surface.h; row++) {
			const int dstY = 80 + row;
			if (dstY < 0 || dstY >= 200)
				continue;
			const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < fr.surface.w; col++) {
				const int dstX = 5 + col;
				if (dstX < 0 || dstX >= 320)
					continue;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
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
	{
		const int lineH = _font.getFontHeight() + 1;
		int y = kRectY;
		while (clueCursor < (int)found.size()) {
			const uint clueId = found[clueCursor];
			Common::String txt;
			if (ni && clueId < niCount) {
				const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
				txt = parseString(_mystery.textAt(textOff),
								  _playerName, _partner);
			}
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
		Common::String txt;
		if (ni && clueId < niCount) {
			const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
			txt = parseString(_mystery.textAt(textOff),
							  _playerName, _partner);
		}
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

	// Page indicator + selected-points counter directly on PIC.
	_font.drawString(&scratch, Common::String::format("p%d/%d",
							   page + 1, (int)pageStarts.size()),
					 270, 4, 320, 0x5C);
	_font.drawString(&scratch, Common::String::format("%d pts",
							   _mystery.selectedPoints()),
					 270, 14, 320, 0x5C);

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

	// Partner sprite frame @ (5, 0x50).
	if (havePartner) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = (uint)((now / 100) % partnerAni.size());
		const Picture &fr = partnerAni[frameIdx];
		const byte transp = (byte)(fr.flags >> 8);
		const int px = 5;
		const int py = 0x50;
		for (int row = 0; row < fr.surface.h; row++) {
			const int dstY = py + row;
			if (dstY < 0 || dstY >= 200)
				continue;
			const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < fr.surface.w; col++) {
				const int dstX = px + col;
				if (dstX < 0 || dstX >= 320)
					continue;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
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

	drawBigMapOverview();
	uint32 mapLastTick = g_system->getMillis();

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
				// settings screen). We use it as "back to menu":
				// abandon the current mystery and return to case
				// selection.
				if (kSetupBtnRect.contains(ev.mouse.x, ev.mouse.y)) {
					_mystery.clear();
					_nextScreen = kScreenInvalid;
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
			drawBigMapOverview();
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

	drawBigMapDetail(scrollX, scrollY, mapPixels, mapW, mapH);
	uint32 detailLastTick = g_system->getMillis();

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
					// Setup button on detail too — `_NextScreen = 6` in
					// the original. We treat it the same way: bail back
					// to case selection.
					_mystery.clear();
					_nextScreen = kScreenInvalid;
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
					for (uint i = 0; i < _mystery.numSites(); i++) {
						if (!_mystery._onSites[i] &&
							i != _mystery._siteNumber)
							continue;
						const byte *entry = _mystery.mapEntry(i);
						if (!entry)
							continue;
						const uint16 buttonId = READ_LE_UINT16(entry + 0x0);
						const uint16 mx       = READ_LE_UINT16(entry + 0x8);
						const uint16 my       = READ_LE_UINT16(entry + 0xa);
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
			drawBigMapDetail(scrollX, scrollY, mapPixels, mapW, mapH);
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void EEMEngine::drawBigMapOverview() {
	// Map-overview redraw — formerly the `drawOverview` lambda inside
	// `doBigMap`. PIC 0x42 frame + per-site marker (Done / Crime / Site
	// per `_DrawBigMapButtons @ 20fe:0877`) + the partner idle sprite.
	// `_DoBigMap @ 20fe:09e7` (`_NewAnimation` block at 20fe:0a44-0a99)
	// registers the partner: when `_LastScreen == 2` it plays an
	// entrance one-shot at (0x102, 0x50) and on END swaps to the idle
	// at (0xfd, 0x50). We don't track LastScreen finely enough so we
	// always render the IDLE pose at (0xfd, 0x50). Idle anim ID:
	// Jake = 0x14 (20), Jenny = 0x12 (18).
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x42, frame)) {
		const int w = MIN<int>(frame.surface.w, 320);
		const int h = MIN<int>(frame.surface.h, 200);
		for (int row = 0; row < h; row++)
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)frame.surface.getBasePtr(0, row), w);
	}

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
		const uint16 mx    = READ_LE_UINT16(entry + 0x4);
		const uint16 my    = READ_LE_UINT16(entry + 0x6);
		const uint16 crime = READ_LE_UINT16(entry + 0xc);
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

	// Partner idle sprite at (0xfd, 0x50). Jake = anim 0x14, Jenny = 0x12.
	const uint kMapAniId = (_partner == 0) ? 0x14 : 0x12;
	Animation mapAnim;
	if (_aniArchive.loadAnimation(kMapAniId, mapAnim) && !mapAnim.empty()) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = (uint)((now / 100) % mapAnim.size());
		const Picture &fr = mapAnim[frameIdx];
		const byte transp = (byte)(fr.flags >> 8);
		const int kMapAnimX = 0xfd;
		const int kMapAnimY = 0x50;
		for (int row = 0; row < fr.surface.h; row++) {
			const int dstY = kMapAnimY + row;
			if (dstY < 0 || dstY >= 200)
				continue;
			const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < fr.surface.w; col++) {
				const int dstX = kMapAnimX + col;
				if (dstX < 0 || dstX >= 320)
					continue;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
}

void EEMEngine::drawBigMapDetail(int scrollX, int scrollY,
								 const Common::Array<byte> &mapPixels,
								 uint16 mapW, uint16 mapH) {
	// Map-detail redraw — formerly the `drawDetail` lambda inside
	// `doBigMap`. PIC 0x43 frame + a 0xe9 × 0xab BIGMAP.PIC viewport at
	// (2, 2), stamped site buttons, and the partner sprite at (0x101,
	// 0x50) — `_DoMapScreen @ 20fe:120b` (`_NewAnimation` at
	// 20fe:12cd-12f0, anim 0x13 Jake / 0x11 Jenny, seqnum 0x13).
	const int kMapWinW = 0xe9;
	const int kMapWinH = 0xab;
	const int kMapWinX = 2;
	const int kMapWinY = 2;

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	Picture frame;
	if (_picsArchive.getPicture(0x43, frame)) {
		const int w = MIN<int>(frame.surface.w, 320);
		const int h = MIN<int>(frame.surface.h, 200);
		for (int row = 0; row < h; row++)
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)frame.surface.getBasePtr(0, row), w);
	}

	const int copyW = MIN<int>(mapW - scrollX, kMapWinW);
	const int copyH = MIN<int>(mapH - scrollY, kMapWinH);
	for (int row = 0; row < copyH; row++) {
		memcpy((byte *)scratch.getBasePtr(kMapWinX, kMapWinY + row),
			   mapPixels.data() + (scrollY + row) * mapW + scrollX,
			   copyW);
	}

	// Stamped site buttons. `_StampButtons @ 20fe:0d2f`:
	//   button = _GetButton(MapData[+0])
	//   destX  = MapData[+8]
	//   destY  = MapData[+0xa]
	for (uint i = 0; i < _mystery.numSites(); i++) {
		if (!_mystery._onSites[i] && i != _mystery._siteNumber)
			continue;
		const byte *entry = _mystery.mapEntry(i);
		if (!entry)
			continue;
		const uint16 buttonId = READ_LE_UINT16(entry + 0x0);
		const uint16 mx       = READ_LE_UINT16(entry + 0x8);
		const uint16 my       = READ_LE_UINT16(entry + 0xa);

		Picture button;
		if (!_buttonArchive.loadEntry(buttonId, button))
			continue;
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
	// frame and the BIGMAP.PIC viewport).
	const uint kDetailAniId = (_partner == 0) ? 0x13 : 0x11;
	Animation detailAnim;
	if (_aniArchive.loadAnimation(kDetailAniId, detailAnim) &&
		!detailAnim.empty()) {
		const uint32 now = g_system->getMillis();
		const uint frameIdx = (uint)((now / 100) % detailAnim.size());
		const Picture &fr = detailAnim[frameIdx];
		const byte transp = (byte)(fr.flags >> 8);
		const int kDetailAnimX = 0x101;
		const int kDetailAnimY = 0x50;
		for (int row = 0; row < fr.surface.h; row++) {
			const int dstY = kDetailAnimY + row;
			if (dstY < 0 || dstY >= 200)
				continue;
			const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < fr.surface.w; col++) {
				const int dstX = kDetailAnimX + col;
				if (dstX < 0 || dstX >= 320)
					continue;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
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

void EEMEngine::doAccuse() {
	if (!_mystery.isLoaded())
		return;

	// Mirrors `_DoAccuseGallery @ 1df2:0a31`:
	//   1. Show KD's hint balloon (KDTextIndex[+8] text).
	//   2. `_GetBackground(0x3f)` — same backdrop as PDA / gallery.
	//   3. `_DrawGallery()` — renders portraits at the standard 5 slots
	//      (positions verified at 29be:0x116, bottom-aligned baseline 0x48).
	//   4. Click loop dispatching on `_NoteButtons` (same table as PDA)
	//      with a separate `_HandleAccuseNoteButton` jump table.
	const uint8 num = _mystery.numSuspects();
	if (num == 0)
		return;

	const byte *gd = _mystery.galleryData();

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
				Picture balloon;
				const bool haveBalloon =
					_balloonArchive.size() > (bubNum & 0x7F) &&
					_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

				// 1df2:0a8b-1df2:0aa5: y = (h < 0x4e) ? (0x50-h)>>1 : 1
				const int balloonX = 0x21;
				int balloonY = 1;
				if (haveBalloon && balloon.surface.h < 0x4e)
					balloonY = (0x50 - balloon.surface.h) / 2;

				Graphics::ManagedSurface ms(320, 200,
					Graphics::PixelFormat::createFormatCLUT8());
				ms.clear();
				if (haveAccuseBg) {
					const int bw = MIN<int>(accuseBg.surface.w, 320);
					const int bh = MIN<int>(accuseBg.surface.h, 200);
					for (int row = 0; row < bh; row++) {
						memcpy((byte *)ms.getBasePtr(0, row),
							   (const byte *)accuseBg.surface.getBasePtr(0, row), bw);
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

	// Real chain evaluation: sum point values of clues the player marked
	// "selected" in the notebook. Mirrors `_SolvedCheck` @ 1df2:00ec.
	const int points = _mystery.selectedPoints();
	const bool guessedRight = _mystery.solvedCheck();
	debugC(1, kDebugScript, "doAccuse: picked=%d selectedPts=%d -> %s",
		   picked, points, guessedRight ? "correct" : "wrong");

	// If the player hasn't marked any evidence yet, give them a hint
	// rather than an instant fail. Mirrors the original "We're not ready
	// to solve this mystery yet..." string at 29be:10f0.
	if (points == 0 && _font.isLoaded()) {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		_font.drawWordWrapped(&scratch, 16, 80, 288,
			"We're not ready to solve this mystery yet. "
			"Let's keep investigating until we have some "
			"more solid evidence to make our case! "
			"(Press N in the site screen to mark clues.)",
			0xF);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
		waitForInput(15000);
		return;
	}

	// Pick the ending based on the chain. For a correct accusation the
	// original would call `_DisplayClue(_Mystery + AChain[0])`, play
	// SCRAPBK.ANI and save progress. We load the matching `E<n>.BIN`
	// ending text and render its pages with prev/next navigation.
	const int endingNum = guessedRight ? picked : 0;
	const Common::String fname = Common::String::format("E%d.BIN", endingNum);
	Common::File f;
	if (!f.open(Common::Path(fname))) {
		warning("doAccuse: %s missing", fname.c_str());
		return;
	}

	// E<n>.BIN format (verified against `_DisplayEndingPage` @ 1df2:044c):
	//   u16 numPages
	//   per page (10 bytes header + NUL-string):
	//     u16 picNum
	//     u16 x1, y1, x2, y2  (story rect)
	//     bytes[] NUL-terminated text
	const uint32 fileLen = f.size();
	Common::Array<byte> blob(fileLen);
	if (f.read(blob.data(), fileLen) != fileLen)
		return;
	const byte *e = blob.data();
	const uint16 pages = READ_LE_UINT16(e);

	uint pageIdx = 0;

	while (!shouldQuit()) {
		// Walk to pageIdx.
		uint pos = 2;
		uint cur = 0;
		while (cur < pageIdx && pos + 10 < fileLen) {
			const char *t = (const char *)(e + pos + 10);
			pos += 10 + strlen(t) + 1;
			cur++;
		}
		if (pos + 10 >= fileLen)
			break;

		const uint16 picNum = READ_LE_UINT16(e + pos + 0);
		const uint16 x1     = READ_LE_UINT16(e + pos + 2);
		const uint16 y1     = READ_LE_UINT16(e + pos + 4);
		const uint16 x2     = READ_LE_UINT16(e + pos + 6);
		const uint16 y2     = READ_LE_UINT16(e + pos + 8);
		const char *raw     = (const char *)(e + pos + 10);
		const Common::String txt = parseString(raw, _playerName, _partner);

		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();

		// Page background.
		if (picNum != 0) {
			Picture bg;
			if (_picsArchive.getPicture(picNum, bg)) {
				const int w = MIN<int>(bg.surface.w, 320);
				const int h = MIN<int>(bg.surface.h, 200);
				for (int row = 0; row < h; row++) {
					memcpy((byte *)scratch.getBasePtr(0, row),
						   (const byte *)bg.surface.getBasePtr(0, row), w);
				}
			}
		}

		if (_font.isLoaded()) {
			Common::String banner = "Not enough evidence";
			if (guessedRight)
				banner = _mystery._firstTry ? "CORRECT - FIRST TRY!" : "CORRECT!";
			_font.drawString(&scratch, banner, 8, 4, 320, 0xF);
			_font.drawString(&scratch, Common::String::format("Evidence: %d/100  Suspect: %d",
									   points, picked + 1), 8, 16, 320, 0xF);
			const int wrapW = MAX<int>(16, x2 - x1);
			const int wrapY = MAX<int>(28, (int)y1);
			(void)y2;
			_font.drawWordWrapped(&scratch, x1, wrapY, wrapW, txt, 0xF);
			_font.drawString(&scratch, Common::String::format("page %u/%u  (Left/Right or click)",
									   pageIdx + 1, pages), 8, 188, 320, 0xF);
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		// Page navigation.
		bool advance = false;
		bool back    = false;
		bool exit    = false;
		while (!advance && !back && !exit && !shouldQuit()) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					exit = true; break;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					advance = true; break;
				}
				if (ev.type == Common::EVENT_KEYDOWN) {
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
						exit = true;
					else if (ev.kbd.keycode == Common::KEYCODE_LEFT)
						back = true;
					else
						advance = true;
					break;
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(15);
		}
		if (exit)
			break;
		if (advance) {
			if (pageIdx + 1 >= pages)
				break;
			pageIdx++;
		} else if (back) {
			if (pageIdx > 0)
				pageIdx--;
		}
	}

	// Mirror `_DisplayCorrect`'s scrap-book animation + solved tracking +
	// auto-save (the original calls `_SavePlayerRecord` after a win).
	if (guessedRight) {
		const uint mn = _mystery.number();
		if (mn < sizeof(_mysteriesSolved)) {
			_mysteriesSolved[mn] = _mystery._firstTry ? 2 : 1;
		}

		// `_DisplayCorrect @ 1df2:073c` calls `_MIDIPlay(5)` (1df2:0789)
		// before `_DifferenceAnimation("scrapbk.ani")` to swap from the
		// travel music to the winner cue.
		if (_music)
			_music->playMus(5, /*loop=*/false);
		playAnm(Common::Path("SCRAPBK.ANI"), 120, true);

		// Auto-save into slot 0 (the engine's quicksave slot).
		const Common::String desc = Common::String::format(
			"%s — solved mystery %u", _playerName.c_str(), mn);
		Common::Error err = saveGameState(0, desc, true);
		if (err.getCode() != Common::kNoError)
			warning("auto-save after solve failed: %s",
					err.getDesc().c_str());
	} else {
		_mystery._firstTry = false;
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
