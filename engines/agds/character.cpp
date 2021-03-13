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

#include "agds/character.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/resourceManager.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include <math.h>

namespace AGDS {

void Character::load(Common::SeekableReadStream *stream) {
	debug("loading character...");
	stream->readUint32LE(); //unk
	uint16 magic = stream->readUint16LE();
	switch (magic) {
	case 0xdead:
		_movementDirections = 16;
		break;
	case 0x8888:
		_movementDirections = 8;
		break;
	default:
		error("invalid magic %04x", magic);
	}

	_animations.clear();
	while (stream->pos() < stream->size()) {
		uint size = stream->readUint32LE();
		uint index = stream->readUint16LE();
		debug("header size %u, index: %u", size, index);

		uint16 frames = stream->readUint16LE();
		uint16 format = stream->readUint16LE();
		Common::String filename = readString(stream);

		AnimationDescription animation;
		animation.filename = filename;
		debug("%u: animation %s, frames: %d, format: %d", _animations.size(), animation.filename.c_str(), frames, format);
		while (frames--) {
			int x = stream->readSint16LE();
			int y = stream->readSint16LE();
			int w = stream->readUint32LE();
			int h = stream->readUint32LE();
			AnimationDescription::Frame frame = {x, y, w, h};
			animation.frames.push_back(frame);
			debug("frame %d, %d, %dx%d", x, y, w, h);
			uint unk1 = stream->readUint32LE();
			uint unk2 = stream->readUint32LE();
			uint unk3 = stream->readUint32LE();
			uint unk4 = stream->readUint32LE(); //GRP file offset?
			uint unk5 = stream->readUint32LE();
			uint unk6 = stream->readByte();
			uint unk7 = stream->readUint32LE();
			uint unk8 = stream->readUint32LE();
			stream->readUint32LE(); //CDCDCDCD
			uint unk9 = stream->readUint32LE();
			uint unk10 = stream->readUint32LE();
			stream->readUint32LE(); //CDCDCDCD
			uint unk11 = stream->readByte();
			stream->readUint32LE(); //CDCDCDCD
			debug("unknown: %u %u %u 0x%08x - %u %u %u %u - %u %u %u",
			      unk1, unk2, unk3, unk4,
			      unk5, unk6, unk7, unk8,
			      unk9, unk10, unk11);
		}
		_animations[index] = animation;
	}

	delete stream;
}

void Character::loadState(Common::ReadStream* stream) {
	int x = stream->readUint16LE();
	int y = stream->readUint16LE();
	int dir = stream->readUint16LE();
	debug("character at %d, %d, dir: %d", x, y, dir);
	position(Common::Point(x, y));
	direction(dir);
	int n = 2;
	while(n--) {
		int v = stream->readUint16LE();
		debug("savegame character leftover: %d", v);
	}
}

void Character::saveState(Common::WriteStream* stream) const {
	stream->writeUint16LE(_pos.x);
	stream->writeUint16LE(_pos.y);
	stream->writeUint16LE(_direction);
	stream->writeUint16LE(1);
	stream->writeUint16LE(1);
}


void Character::direction(int dir) {
	debug("setDirection %d", dir);
	_direction = dir;

	if (dir < 0)
		return;

	auto desc = animationDescription(dir);
	_animation = desc? _engine->loadAnimation(desc->filename): nullptr;
	_animationPos = Common::Point();
	if (!_animation) {
		debug("no animation?");
		_phase = -1;
		_frames = 0;
		return;
	}
	_animation->rewind();
}

void Character::moveTo(const Common::String & processName, Common::Point dst, int dir) {
	debug("character move %d,%d %d", dst.x, dst.y, dir);
	_processName = processName;
	_pos = dst;
	direction(dir);
}

void Character::animate(const Common::String & processName, Common::Point pos, int direction, int speed) {
	debug("animate character: %d,%d %d %d", pos.x, pos.y, direction, speed);
	_processName = processName;
	if (direction == -1)
		return;
	auto jokes = _engine->jokes();
	auto animationDescription = jokes->animationDescription(direction);
	_animation = animationDescription? _engine->loadAnimation(animationDescription->filename): nullptr;
	if (!_animation) {
		warning("no jokes animation %d", direction);
		_phase = -1;
		_frames = 0;
		return;
	}
	_animation->speed(speed);
	_animation->rewind();
	_phase = 0;
	_frames = _animation->frames();
	_animationPos = pos;
	debug("character animation frames: %d, enabled: %d, visible: %d", _frames, _enabled, _visible);
}

void Character::stop() {
	debug("character %s: stop", _object->getName().c_str());
	_phase = -1;
	_frames = 0;
}


void Character::paint(Graphics::Surface &backbuffer) {
	if (!_enabled || !_visible || !_animation)
		return;

	Common::Point pos = _pos + _animationPos;

	auto screen = _engine->getCurrentScreen();
	auto scale = screen? screen->getZScale(_pos.y): 1;
	_animation->scale(scale);

	debug("character %d/%d", _phase, _frames);
	if (_phase >= 0 && _phase < _frames) {
		_animation->tick();
		_phase = _animation->phase();
		if (_phase >= _frames) {
			_phase = -1;
			_frames = 0;
		}
	}
	_engine->reactivate(_processName, true);

	pos.y -= _animation->height();
	pos.x -= _animation->width() / 2;
	_animation->paint(backbuffer, pos);
}

int Character::z() const {
	int y = _pos.y + _animationPos.y;
	//fixme: add temp var : _movePos?
	//debug("char z = %d", y);
	return g_system->getHeight() - y;
}


int Character::getDirectionForMovement(Common::Point delta) {
	auto angle = atan2(delta.y, delta.x);
	if (angle < 0)
		angle += M_PI * 2;

	if (_movementDirections == 16) {
		if (angle < 6.1850053125 && angle > 0.0981746875) {
			if (angle > 0.5235983333333333) {
				if (angle > 0.9490219791666666) {
					if (angle > 1.374445625) {
						if (angle > 1.767144375) {
							if (angle > 2.192568020833333) {
								if (angle > 2.617991666666666) {
									if (angle > 3.0434153125) {
										if (angle > 3.2397646875) {
											if (angle > 3.665188333333333) {
												if (angle > 4.090611979166667) {
													if (angle > 4.516035625) {
														if (angle > 4.908734375) {
															if (angle > 5.334158020833333) {
																if (angle > 5.759581666666667)
																	return 3;
																else
																	return 2;
															} else {
																return 1;
															}
														} else {
															return 0;
														}
													} else {
														return 15;
													}
												} else {
													return 14;
												}
											} else {
												return 13;
											}
										} else {
											return 12;
										}
									} else {
										return 11;
									}
								} else {
									return 10;
								}
							} else {
								return 9;
							}
						} else {
							return 8;
						}
					} else {
						return 7;
					}
				} else {
					return 6;
				}
			} else {
				return 5;
			}
		} else {
			return 4;
		}
	} else if (angle < 5.89048125 && angle > 0.39269875) {
		if (angle > 1.17809625) {
			if (angle > 1.96349375) {
				if (angle > 2.74889125) {
					if (angle > 3.53428875) {
						if (angle > 4.31968625) {
							if (angle > 5.105083749999999)
								return 2;
							else
								return 0;
						} else {
							return 14;
						}
					} else {
						return 12;
					}
				} else {
					return 10;
				}
			} else {
				return 8;
			}
		} else {
			return 6;
		}
	} else {
		return 4;
	}
}

} // namespace AGDS
