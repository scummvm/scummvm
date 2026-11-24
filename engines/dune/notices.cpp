/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dune/dune.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

namespace Dune {

void DuneEngine::selectTroopNoticePose(bool randomize) {
	const uint caller = READ_LE_UINT16(_state.begin() + 0x47c4);
	const bool fixed = caller >= 13 && caller <= 15 &&
		!(caller == 15 && storyStage() == 200);
	// 9123/994F choose a fixed variant for merchants/Fremen, but one of
	// four pose sequences for named callers. This is the secondary DOS RNG,
	// not the engine's general random source. Loading never rolls it again.
	if (fixed) {
		if (caller == 13) {
			const uint16 place = READ_LE_UINT16(_state.begin() + 0x114e);
			_speakerAnimation = place < _state.size() ? _state[place] >> 1 : 0;
		} else {
			_speakerAnimation = fremenAnimation(caller, _state[0x476c] & 7);
		}
		// 996C skips 32 zero-terminated frames before the first draw.
		_speakerFrame = 32;
	} else {
		_speakerAnimation = randomize ? (troopRandom(true) & 6) / 2 : 0;
		if (caller == 12 && (_state[0x10a7] & 0x10))
			_speakerAnimation += 5; // 91A0's alternate Harkonnen table.
		_speakerFrame = 0;
	}
}

void DuneEngine::startNoticeCloud() {
	_noticeCloudSource.resize(kScreenWidth * kScreenHeight);
	memcpy(_noticeCloudSource.begin(), _framebuffer1, _noticeCloudSource.size());
	_noticeCloudVisible = _noticeCloudSource;
	_noticeCloudRemainder = _noticeCloudPhase = 0;
	_system->getPaletteManager()->grabPalette(_noticeCloudPalette, 128, 64);
}

void DuneEngine::stopNoticeCloud() {
	_noticeCloudSource.clear();
	_noticeCloudVisible.clear();
	_noticeCloudRemainder = _noticeCloudPhase = 0;
}

void DuneEngine::applyNoticeCloudPalette() {
	byte colors[64 * 3];
	for (uint i = 0; i < 64; i++)
		memcpy(colors + i * 3, _noticeCloudPalette + ((i + _noticeCloudPhase) % 64) * 3, 3);
	_system->getPaletteManager()->setPalette(colors, 128, 64);
}

void DuneEngine::composeNoticeCloud() {
	if (_noticeCloudSource.empty())
		return;
	// The fresh VIS/portrait render reinstalls the uncycled cloud palette.
	_system->getPaletteManager()->grabPalette(_noticeCloudPalette, 128, 64);
	// C4F0 copies portrait dirty rectangles to both pages. Until the original
	// portrait decoder supplies those rectangles, use the changed bounds of
	// the native compositor, preserving the wave outside that rectangle.
	// The modal panel stays as composed on entry, even when load's panel
	// reset has installed the ordinary room compass underneath the report.
	int left = kScreenWidth, top = 148, right = 0, bottom = 0;
	for (int y = 0; y < 148; y++) {
		for (int x = 0; x < kScreenWidth; x++) {
			const uint at = y * kScreenWidth + x;
			if (_framebuffer1[at] != _noticeCloudSource[at]) {
				left = MIN(left, x);
				right = MAX(right, x + 1);
				top = MIN(top, y);
				bottom = MAX(bottom, y + 1);
			}
		}
	}
	for (int y = top; y < bottom; y++)
		memcpy(_noticeCloudVisible.begin() + y * kScreenWidth + left,
			_framebuffer1 + y * kScreenWidth + left, right - left);
	memcpy(_noticeCloudSource.begin(), _framebuffer1, kScreenWidth * 148);
	memcpy(_framebuffer1, _noticeCloudVisible.begin(), _noticeCloudVisible.size());
	applyNoticeCloudPalette();
}

bool DuneEngine::pollNoticeCloud(uint16 elapsedTicks) {
	if (_noticeCloudSource.empty())
		return false;
	// DA25 registers 2CC7 every six 5-ms ticks. D9D2 retains the modulo
	// remainder but invokes it only once per poll, even after a long stall.
	const uint16 ticks = (uint16)(elapsedTicks + _noticeCloudRemainder);
	_noticeCloudRemainder = ticks % 6;
	if (ticks < 6)
		return false;
	Graphics::distortNotice(_noticeCloudSource.begin(), _noticeCloudVisible.begin(),
		_noticeCloudRow, _noticeCloudPrevious);
	_noticeCloudPhase = (_noticeCloudPhase + 1) % 64;
	applyNoticeCloudPalette();
	memcpy(_framebuffer1, _noticeCloudVisible.begin(), _noticeCloudVisible.size());
	return true;
}

/** 1B0D/2B2A: this decision belongs to every poll, not the hourly round. */
uint DuneEngine::troopNoticeDispatchAction(uint16 idleTicks) const {
	if (!canOpenTroopNotice() || isPaused() || _noticeWarningActive || _noticeTransitionActive ||
			_state[0x2788] || _mixer->isSoundHandleActive(_speechHandle) ||
			!_state[0x1190] || idleTicks < 50)
		return kNoticeDeferred;
	const uint caller = _state[0x1192];
	const uint16 place = READ_LE_UINT16(_state.begin() + 0x1193);
	// 2AD8: even a named caller reporting another place can speak locally.
	// Only group fifteen additionally requires that its troop be here.
	if (caller < 16 && (READ_LE_UINT16(_state.begin() + 0x12) & (1 << caller)) &&
			(caller != 15 || place == READ_LE_UINT16(_state.begin() + 0x114e)))
		return kNoticeLocal;
	if (_state[0x2b] || idleTicks >= 450)
		return kNoticeRemote;
	if (idleTicks < 150)
		return kNoticeWarning2;
	if (idleTicks >= 250 && idleTicks < 350)
		return kNoticeWarning4;
	return kNoticeDeferred;
}

void DuneEngine::resetTroopNoticeIdle() {
	_noticeIdleTime = _system->getMillis() / 5 * 5;
	if (_state.size() > 0xdc5b)
		WRITE_LE_UINT16(_state.begin() + 0xdc5a, (uint16)(_noticeIdleTime / 5));
}

/** The separate 2BA1 gate, after 1B0D's speech/menu eligibility checks. */
bool DuneEngine::firstDesertVisionReady(uint16 idleTicks) const {
	return idleTicks >= 1000 && storyStage() == 20 &&
		canOpenTroopNotice(true) && !isPaused() && !_noticeWarningActive &&
		!_noticeTransitionActive && !_state[0x2788] &&
		!_mixer->isSoundHandleActive(_speechHandle) &&
		READ_LE_UINT16(_state.begin() + 0x10) == 0 && _state[8] == 0xff;
}

bool DuneEngine::updateTroopNoticeDispatch() {
	// 1B0D/2B2A precedes DB4C's input sample and D893's held-button reset.
	// In particular, WAIT arms the first vision even if its button stays down.
	const uint16 idleTicks = (uint16)((_system->getMillis() - _noticeIdleTime) / 5);
	// 2BA1 does not use the ordinary queued-report gate. At story stage 20,
	// Paul alone outside has his first vision after 1,000 idle ticks (5 s).
	// The earlier implementation reached it only through WAIT FOR EVENING.
	if (firstDesertVisionReady(idleTicks)) {
		// 2BB8 advances DC5A by 950 ticks before 1071/B17A. Retain the
		// elapsed remainder so the new palace event follows the ordinary
		// warning/dispatch gates instead of opening immediately at 1000.
		_noticeIdleTime += 950 * 5;
		WRITE_LE_UINT16(_state.begin() + 0xdc5a,
				READ_LE_UINT16(_state.begin() + 0xdc5a) + 950);
		return showFirstDesertVision();
	}
	switch (troopNoticeDispatchAction(idleTicks)) {
	case kNoticeLocal:
		return showLocalTroopNotice();
	case kNoticeRemote:
		if (_state[0x1192] >= 16 || (_state[0x1192] == 14 &&
				READ_LE_UINT16(_state.begin() + 0x1193) == 0)) {
			consumeTroopNotice(); // 2C09..2C14 reject these callers.
			return false;
		}
		return showPendingTroopNotice();
	case kNoticeWarning2:
		showNoticeWarning(kNoticeWarning2);
		break;
	case kNoticeWarning4:
		showNoticeWarning(kNoticeWarning4);
		break;
	default:
		break;
	}
	return false;
}

/** 2BC4..2BCC: the first vision owns the poll until its report closes. */
void DuneEngine::waitForFirstDesertVision() {
	do {
		// The interrupt keeps running, but this is not an ordinary 1B0D
		// troop-hour/input poll. In particular, held WAIT cannot reset DC5A
		// between 1071 and the report's idle warnings and reading window.
		tick(false);
		if (_menuKind == kMenuTroopNotice)
			pollTroopNotice();
		else {
			updateTroopNoticeDispatch();
			if (_menuKind != kMenuTroopNotice && _state[0xea] != 0xff)
				_system->delayMillis(5);
		}
	} while (!shouldQuit() && _state[0xea] != 0xff);
}

/** 2B90 and DNVGA 2F87/2F9F: four 36-tick scene pixelation passes. */
void DuneEngine::showNoticeWarning(uint effect) {
	if (effect != kNoticeWarning2 && effect != kNoticeWarning4)
		return;
	Common::Array<byte> source(kScreenWidth * kScreenHeight);
	memcpy(source.begin(), _framebuffer1, source.size());
	const bool cursorShown = CursorMan.showMouse(false);
	_noticeWarningActive = true;
	const bool fast = ConfMan.getBool("dune_nodelay");
	for (uint pass = 0; pass < 4 && !shouldQuit(); pass++) {
		const uint block = effect == kNoticeWarning4 && (pass == 1 || pass == 2) ? 4 : 2;
		const uint32 start = _system->getMillis() / 5;
		uint phase = 0;
		while (!shouldQuit()) {
			Graphics::pixelateNotice(source.begin(), _framebuffer1, block, phase++);
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kSceneHeight);
			_system->updateScreen();
			// Original sample cadence depends on rendering speed. Native
			// samples advance at 200 Hz; each pass retains its 36-tick wait.
			if (fast ? phase > 36 : _system->getMillis() / 5 - start >= 36)
				break;
			if (!fast)
				_system->delayMillis(5);
		}
	}
	memcpy(_framebuffer1, source.begin(), source.size());
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kSceneHeight);
	CursorMan.showMouse(cursorShown);
	_system->updateScreen();
	_noticeWarningActive = false;
	if (fast) {
		_noticeIdleTime -= 720;
		// 2B90's effect services the timer. Leave hourly work to 1B0D.
		advanceClockElapsed(720, false, false);
	}
}

/** C108/2F53: blur the old page, switch palette, resolve the new page. */
void DuneEngine::showNoticeTransition(const byte *oldPage, const byte *oldPalette) {
	Common::Array<byte> target(kScreenWidth * kScreenHeight);
	memcpy(target.begin(), _framebuffer1, target.size());
	byte palette[3 * 256];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);
	_system->getPaletteManager()->setPalette(oldPalette, 0, 256);
	const bool cursorShown = CursorMan.showMouse(false);
	_noticeTransitionActive = true;
	const bool fast = ConfMan.getBool("dune_nodelay");
	static const uint blocks[] = { 2, 4, 8, 8, 4, 2 };
	// The driver changes only the scene. The old command rows remain visible
	// until C4CD copies the full target page after the sixth pass.
	memcpy(_framebuffer1, oldPage, target.size());
	for (uint pass = 0; pass < ARRAYSIZE(blocks) && !shouldQuit(); pass++) {
		if (pass == 3)
			_system->getPaletteManager()->setPalette(palette, 0, 256);
		const byte *source = pass < 3 ? oldPage : target.begin();
		const uint32 start = _system->getMillis() / 5;
		uint phase = 0;
		while (!shouldQuit()) {
			Graphics::pixelateNotice(source, _framebuffer1, blocks[pass], phase++);
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
			_system->updateScreen();
			if (fast ? phase > 36 : _system->getMillis() / 5 - start >= 36)
				break;
			if (!fast)
				_system->delayMillis(5);
		}
	}
	memcpy(_framebuffer1, target.begin(), target.size());
	_system->getPaletteManager()->setPalette(palette, 0, 256);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	CursorMan.showMouse(cursorShown);
	_system->updateScreen();
	// Credit the nested effect's time. The next ordinary 1B0D poll can
	// handle the pending hour.
	if (fast)
		advanceClockElapsed(1080, false, false);
	_noticeTransitionActive = false;
}

/** 2A51: local conversation acknowledges this caller, not only the head. */
void DuneEngine::consumeCallerNotices(uint caller, uint16 place) {
	const uint count = MIN<uint>(_state[0x1190], 10);
	uint kept = 0;
	for (uint i = 0; i < count; i++) {
		const byte *entry = _state.begin() + 0x1191 + 4 * i;
		if (entry[1] == caller && (caller != 15 || READ_LE_UINT16(entry + 2) == place))
			continue;
		memmove(_state.begin() + 0x1191 + 4 * kept++, entry, 4);
	}
	_state[0x1190] = kept;
	// 2A9E clears only the first vacated entry; the trailing scratch and
	// 118F latch differ from 2A34's removal of the oldest remote report.
	if (kept != count)
		memset(_state.begin() + 0x1191 + 4 * kept, 0, 4);
}

/** 2AFA/3542: a present caller opens a room conversation, not a cloud. */
bool DuneEngine::showLocalTroopNotice() {
	return showLocalTroopNoticeAt(0, true);
}

bool DuneEngine::showLocalTroopNoticeAt(uint index, bool clearRequest) {
	if (index >= MIN<uint>(_state[0x1190], 10))
		return false;
	const uint packet = 0x1191 + 4 * index;
	const uint caller = _state[packet + 1];
	if (caller >= 16)
		return false;
	const uint16 contextPlace = READ_LE_UINT16(_state.begin() + 0x11ce);
	const uint16 place = READ_LE_UINT16(_state.begin() + packet + 2);
	const byte oldKind = _state[0xea];
	if (place)
		expandPlaceContext(place); // 2B00/2B09: save context, expand payload.
	_state[0xea] = _state[packet];
	const int selected = troopNoticeRecord();
	if (selected < 0) {
		_state[0xea] = oldKind;
		expandPlaceContext(contextPlace);
		return false;
	}
	Dialogue::Record &record = _dialogue.entry(132)[selected];
	const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
	// 96D8 prepares the selected paragraph before978E/9908, unlike a
	// manually selected person's initial portrait. Do not spend its idle
	// RNG or retain the previous paragraph's word count while binding.
	selectLocalSpeaker(caller, false);
	WRITE_LE_UINT16(_state.begin() + 0x47c4, caller);
	_speakerReport = 0;
	_nextMouthTime = _system->getMillis() + 1000 / 12;
	_troopOrdersFromMap = false;
	_state[0x19] = 0xff;
	// 9945/9F9E reaches 9FD8 for a local queued report too. Lower the
	// panel face through the same 1803 guard as ordinary dialogue, before
	// painting its paragraph. Preparing the shared record directly skipped
	// that step and left Paul's face raised during Duncan's entry reminder.
	if (!_state[0x46eb])
		lowerPanelFace(true, true); // 2B00/96D8 also bypasses93AA's capture.
	// 2B00/96D8 uses the room's prepared names (or 94F3's epidemic
	// override), not a temporary change to Paul's physical location.
	prepareDialogueLine(record, 16);
	prepareLocalSpeakerIdle();
	// 2B17 calls9945 directly even though positiveEA skipped9936. Register
	// the prepared pose without selecting it again or resetting animation.
	_speakerIdleActive = _speakerIdleFrameCount != 0;
	_state[0xea] = 0xff; // 2B1A: local event voices can use the indoor take.
	rememberBookRecord(record, caller);
	record.flags |= 0x80;
	playSpeech(record.line, caller, true);
	expandPlaceContext(contextPlace); // 2B24/2B25: restore, not always Paul.
	consumeCallerNotices(caller, here);
	if (clearRequest)
		_state[0x23] = 0; // 2B68, unlike 3520's queued-report fallback.
	_state[0xe7] = 0;
	_state[0x47a7]++;
	// 2B68/3542 reaches 93DF after the event and its context restoration.
	// It starts at the person's first news part, without selecting a reply
	// yet. Keep the event's caption/recording for WHAT; TALK selects fresh
	// news using the restored context and first-pass flag, not a cursor
	// guessed before the event was spoken.
	beginPersonNews(caller);
	showTalkMenu();
	if (_inDesert)
		drawDesertDialogue();
	else
		drawScene();
	return true;
}

/** 35AD/36EE/3520: on-foot room-entry replies precede ordinary news. */
bool DuneEngine::showLocalRoomRequests() {
	if (_state[0x11c9] || _onWorld || _onGlobe || _inCabin || _onMixer)
		return false; // 35E9's carried/discovery branch is separate.
	_state[0x1a] = _state[0x47a7] = 0;
	const byte inhibited = _state[0x47a6];
	_state[0x47a6] = 0;
	if (inhibited)
		return false;
	_state[0x1a] = 1;
	// 35CF/2566 ships before walking resident records. The walk, rather
	// than a forced Duncan line, then selects the first available response.
	if (_state[0x0b] == 8 &&
			(READ_LE_UINT16(_state.begin() + 0xc0) & READ_LE_UINT16(_state.begin() + 0x1158)) &&
			(READ_LE_UINT16(_state.begin() + 0x12) & 8))
		sendSpiceShipment();
	return walkLocalRoomRequests();
}

/** 36EE/3520, shared by arrival 35AD and predeparture 36D3. */
bool DuneEngine::walkLocalRoomRequests() {
	const uint16 where = READ_LE_UINT16(_state.begin() + 4);
	const uint16 site = READ_LE_UINT16(_state.begin() + 6);
	for (uint slot = 0; slot < 16; slot++) {
		const byte *resident = _state.begin() + 0xfd8 + 16 * slot;
		if (READ_LE_UINT16(resident) != where || READ_LE_UINT16(resident + 2) != site)
			continue; // 36FE/3703: both physical words, in record order.
		const uint caller = resident[14];
		if (caller >= 16)
			continue;
		WRITE_LE_UINT16(_state.begin() + 0x47c4, caller);
		preparePersonMusterContext(caller);
		WRITE_LE_UINT16(_state.begin() + 0x47a2, 0xfd8 + 16 * caller);
		// 96F8 expands only the un-recruited Fremen on this request path.
		if (caller == 14) {
			const uint troop = READ_LE_UINT16(_state.begin() + 0x4756);
			if (troop >= 0x8aa && troop < 0xfbb && (troop - 0x8aa) % 27 == 0)
				prepareTroopContactContext((troop - 0x8aa) / 27);
		}
		preparePersonDialogueContext(caller);
		const uint part = caller * 8 + kMakingOff;
		if (part < _dialogue.count()) {
			const Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line < records.size(); line++) {
				if (!dialogueRecordAvailable(records[line], 0x20) ||
						!conditionHolds(records[line].condition))
					continue;
				// 3520/96F1 selects the resident, then 9F8B prepares the
				// chosen paragraph before 9F97/9908 selects its portrait.
				// Manual 93AA contact prepares a portrait immediately; using
				// that path here spends RNG on the previous paragraph first.
				selectLocalSpeaker(caller, false);
				_dialoguePart = part;
				_dialogueLine = line;
				_dialogueMask = 0x20;
				WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
				_state[0x47a8] = 0;
				_localRoomRequestPending = true;
				showTalkMenu(false);
				sayChosenLine();
				return true;
			}
		}
		// 2AAF searches this caller's first matching packet. Do not let the
		// queue head or a later resident steal its place in the room walk.
		const uint count = MIN<uint>(_state[0x1190], 10);
		for (uint index = 0; index < count; index++) {
			const byte *packet = _state.begin() + 0x1191 + 4 * index;
			if (packet[1] == caller &&
					(caller != 15 || READ_LE_UINT16(packet + 2) == READ_LE_UINT16(_state.begin() + 0x114e))) {
				return showLocalTroopNoticeAt(index, false);
			}
		}
	}
	return false;
}

/** 3542/3551/3595, only after the automatic paragraph has completed. */
void DuneEngine::finishLocalRoomRequest() {
	if (!_localRoomRequestPending)
		return;
	_localRoomRequestPending = false;
	const uint caller = READ_LE_UINT16(_state.begin() + 0x47c4);
	consumeCallerNotices(caller, READ_LE_UINT16(_state.begin() + 0x114e));
	_state[0xe7] = 0;
	_state[0x47a7]++;
	if (!_state[0x4774] && _state[0x23] < 100) {
		beginPersonNews(caller);
		showTalkMenu();
	}
	if (_pendingRoomExit >= 0) {
		_state[0x23] = 0; // 36E8, outside 3542's ordinary news reset.
		if (_state[0x47a5] != 0xff)
			finishRoomDeparture();
		// finishDialogueLine reaches this cleanup only after the final
		// page's voice has started, so accepted movement can wait for it.
	}
}

} // End of namespace Dune
