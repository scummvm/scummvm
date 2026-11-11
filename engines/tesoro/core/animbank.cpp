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

#include "tesoro/core/animbank.h"

#include "common/endian.h"
#include "common/textconsole.h"

#include "tesoro/core/streamutil.h"

namespace Tesoro {

static const uint32 kFileHeaderSize = 0x18;
static const uint32 kFileStepSize = 0x18;

enum {
	kStepDwords = 2, // step contains raw 32-bit dwords
	kStepPoints = 4  // step contains points (x | y << 16) and a scale
};

bool AnimBank::load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode) {
	*this = AnimBank();

	// The data starts four bytes after the offset given by the archive.
	if (!dat.seek(entryOffset + 4, SEEK_SET))
		return false;

	byte head[kFileHeaderSize];
	if (!readExactly(dat, head, sizeof(head)))
		return false;
	const uint32 stepCount = READ_LE_UINT16(head);
	const uint32 imageCount = READ_LE_UINT16(head + 4);

	_mode = mode;
	_headerFlag = head[0x14] == 1;
	for (uint i = 0; i < 3; ++i)
		_header[i] = READ_LE_UINT32(head + 8 + 4 * i);

	_steps.resize(stepCount);
	for (uint32 i = 0; i < stepCount; ++i) {
		byte rec[kFileStepSize];
		if (!readExactly(dat, rec, sizeof(rec))) {
			*this = AnimBank();
			return false;
		}

		AnimStep &seq = _steps[i];
		seq.imageX = (int16)READ_LE_UINT16(rec);
		seq.imageY = (int16)READ_LE_UINT16(rec + 2);
		seq.x = (int16)READ_LE_UINT16(rec + 4);
		seq.y = (int16)READ_LE_UINT16(rec + 6);
		seq.duration = READ_LE_UINT16(rec + 8);
		seq.imageIndex = (int16)READ_LE_UINT16(rec + 0xa);
		seq.hitX = (int16)READ_LE_UINT16(rec + 0xc);
		seq.hitY = (int16)READ_LE_UINT16(rec + 0xe);
		seq.hitW = (int16)READ_LE_UINT16(rec + 0x10);
		seq.hitH = (int16)READ_LE_UINT16(rec + 0x12);
		seq.type = READ_LE_UINT16(rec + 0x14);
		seq.firstDescent = 0;

		const uint32 extraSize = READ_LE_UINT16(rec + 0x16);
		Common::Array<byte> extra(extraSize);
		if (!readExactly(dat, extraSize ? &extra[0] : nullptr, extraSize)) {
			*this = AnimBank();
			return false;
		}

		if (seq.type == kStepDwords || seq.type == kStepPoints) {
			const bool points = seq.type == kStepPoints;
			const uint32 count = extraSize >= 2 ? READ_LE_UINT16(&extra[0]) : 0;
			const uint32 skip = points ? 8 : 6;
			const uint32 stride = points ? 6 : 4;
			if (count && skip + count * stride > extraSize)
				warning("AnimBank: step %u claims more points than its data holds", i);

			// Points the data does not hold read as zero.
			extra.resize(MAX<uint32>(extraSize, skip + count * stride), 0);
			for (uint32 k = 0; k < count; ++k) {
				const byte *e = &extra[skip + k * stride];
				seq.entries.push_back(READ_LE_UINT32(e));
				if (points)
					seq.scales.push_back(READ_LE_UINT16(e + 4));
			}

			// Entries are expected to be in ascending x order; remember where that stops.
			uint32 d = 0;
			while (d + 1 < count && (int16)(seq.entries[d + 1] & 0xffff) >= (int16)(seq.entries[d] & 0xffff))
				++d;
			seq.firstDescent = d;
		} else if (seq.type > kStepPoints || seq.type == 3) {
			warning("AnimBank: step %u has unsupported type %u", i, seq.type);
		}
	}

	_images.resize(imageCount);
	_imagePositions.resize(imageCount);
	for (uint32 k = 0; k < imageCount; ++k) {
		_imagePositions[k] = (uint32)dat.pos();
		bool ok;
		if (mode == kLoadDeferred) {
			const uint32 size = dat.readUint32LE();
			ok = !dat.eos() && size <= (uint32)(dat.size() - dat.pos()) && dat.skip(size);
		} else {
			ok = _images[k].load(dat);
		}
		if (!ok) {
			*this = AnimBank();
			return false;
		}
	}

	// Positions are relative to the first step.
	if (!_steps.empty()) {
		const int16 x0 = _steps[0].x, y0 = _steps[0].y;
		for (uint32 i = 1; i < stepCount; ++i) {
			_steps[i].x = (int16)(uint16)(_steps[i].x - x0);
			_steps[i].y = (int16)(uint16)(_steps[i].y - y0);
		}
		_steps[0].x = 0;
		_steps[0].y = 0;
	}
	return true;
}

void AnimBank::clearStepMovement(uint i) {
	if (i >= _steps.size()) {
		warning("AnimBank: step %u does not exist", i);
		return;
	}
	_steps[i].x = 0;
	_steps[i].y = 0;
}

bool AnimPlayer::load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode) {
	Common::SharedPtr<AnimBank> bank(new AnimBank());
	if (!bank->load(dat, entryOffset, mode))
		return false;

	_bank = bank;
	_flags = bank->hasHeaderFlag() ? kHeaderFlag : 0;
	return true;
}

void AnimPlayer::setRange(uint16 first, int last, bool loop, bool sumDurations, int32 now) {
	const uint32 count = _bank ? _bank->stepCount() : 0;

	_flags = (_flags & ~6u) | (loop ? kLoop : 0) | kPlaying;
	_first = first;

	if (count == 0) {
		_last = _first = _current = 0;
		_totalDuration = 0;
		_flags = 0;
	} else {
		if (last == -1) {
			_last = first;
			while ((uint32)_last + 1 < count) {
				if (_bank->step(_last + 1).hasImage()) {
					++_last;
				} else {
					// A looping range includes the step that has no image.
					if (loop)
						++_last;
					break;
				}
			}
		} else {
			_last = (uint16)last;
		}

		_current = first;
		_totalDuration = 0;
		if (sumDurations && first <= _last) {
			for (uint32 i = first; i <= _last; ++i) {
				if (i >= count) {
					warning("AnimPlayer: range %u..%u leaves the bank's %u steps", first, _last, count);
					break;
				}
				_totalDuration += _bank->step(i).duration;
			}
		}

		// A range of one step is not played.
		if (first == _last)
			_flags &= ~kPlaying;
	}

	_loopX = 0;
	_loopY = 0;
	_startTime = now;
}

void AnimPlayer::advance(int32 now) {
	if (!_bank || _bank->stepCount() == 0)
		return;

	const AnimBank &bank = *_bank;
	const uint32 count = bank.stepCount();
	if (_current >= count || _first >= count || _last >= count) {
		warning("AnimPlayer: range %u..%u, current %u leave the bank's %u steps", _first, _last, _current, count);
		return;
	}

	if ((_flags & kPlaying) && !(_flags & kPaused)) {
		// Cycle lengths already passed are skipped.
		if (_totalDuration) {
			const uint32 elapsed = (uint32)now - (uint32)_startTime;
			now = (int32)((uint32)now - (elapsed / _totalDuration) * _totalDuration);
		}

		uint32 duration = bank.step(_current).duration;
		while ((int32)((uint32)now - (uint32)_startTime) > (int32)duration) {
			if ((_flags & kLoop) && (uint32)_current + 1 == _last) {
				// Back to the first step, keeping the movement the loop made.
				_startTime = (int32)((uint32)_startTime + duration);
				_current = _first;
				_loopX = (int16)(uint16)(_loopX + bank.step(_last).x - bank.step(_first).x);
				_loopY = (int16)(uint16)(_loopY + bank.step(_last).y - bank.step(_first).y);
			} else {
				if (_current == _last) {
					_flags &= ~kPlaying;
					break;
				}
				_startTime = (int32)((uint32)_startTime + duration);
				++_current;
			}
			duration = bank.step(_current).duration;
		}
	}

	const AnimStep &seq = bank.step(_current);
	if (_current != _last && (_flags & kPlaying)) {
		if (!(_flags & kHeaderFlag)) {
			_movementX = (int16)(uint16)(_loopX + seq.x);
			_movementY = (int16)(uint16)(_loopY + seq.y);
		} else if (seq.duration == 0) {
			warning("AnimPlayer: cannot interpolate a step of no duration");
		} else {
			// Moves from this step's movement towards the next one's.
			const AnimStep &next = bank.step(_current + 1);
			const int32 t = (int32)((uint32)now - (uint32)_startTime);
			const int32 qx = (int32)((uint32)(next.x - seq.x) * (uint32)t) / (int32)seq.duration;
			const int32 qy = (int32)((uint32)(next.y - seq.y) * (uint32)t) / (int32)seq.duration;
			_movementX = (int16)(uint16)(qx + _loopX + seq.x);
			_movementY = (int16)(uint16)(qy + _loopY + seq.y);
		}
	} else {
		_movementX = seq.x;
		_movementY = seq.y;
	}
}

} // End of namespace Tesoro
