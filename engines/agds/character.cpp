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

#include "agds/character.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/region.h"
#include "agds/resourceManager.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/managed_surface.h"

namespace AGDS {

Character::Character(AGDSEngine *engine, const Common::String &name) : _engine(engine), _object(), _animation(nullptr), _jokes(false),
																	   _name(name),
																	   _enabled(true), _visible(false), _stopped(false), _shown(false),
																	   _phase(-1), _frames(0), _direction(-1), _movementDirections(0) {
}

Character::~Character() {
}

void Character::load(Common::SeekableReadStream &stream) {
	debug("loading character...");
	stream.readUint32LE(); // unk
	uint16 magic = stream.readUint16LE();
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
	while (stream.pos() < stream.size()) {
		uint size = stream.readUint32LE();
		uint index = stream.readUint16LE();
		debug("header size %u, index: %u", size, index);

		uint16 frames = stream.readUint16LE();
		uint16 format = stream.readUint16LE();
		Common::String filename = readString(stream);

		AnimationDescription animation;
		animation.filename = filename;
		debug("%s:%u: animation %s, frames: %d, format: %d", _name.c_str(), _animations.size(), animation.filename.c_str(), frames, format);
		while (frames--) {
			int x = stream.readSint16LE();
			int y = stream.readSint16LE();
			uint w = stream.readUint32LE();
			uint h = stream.readUint32LE();
			AnimationDescription::Frame frame = {x, y, w, h};
			animation.frames.push_back(frame);
			debug("frame %d, %d, %dx%d", x, y, w, h);
			uint unk1 = stream.readUint32LE();
			uint unk2 = stream.readUint32LE();
			uint unk3 = stream.readUint32LE();
			uint unk4 = stream.readUint32LE(); // GRP file offset?
			uint unk5 = stream.readUint32LE();
			uint unk6 = stream.readByte();
			uint unk7 = stream.readUint32LE();
			uint unk8 = stream.readUint32LE();
			stream.readUint32LE(); // CDCDCDCD
			uint unk9 = stream.readUint32LE();
			uint unk10 = stream.readUint32LE();
			stream.readUint32LE(); // CDCDCDCD
			uint unk11 = stream.readByte();
			stream.readUint32LE(); // CDCDCDCD
			debug("unknown: %u %u %u 0x%08x - %u %u %u %u - %u %u %u",
				  unk1, unk2, unk3, unk4,
				  unk5, unk6, unk7, unk8,
				  unk9, unk10, unk11);
		}
		_animations[index] = animation;
	}
}

void Character::associate(const Common::String &name) {
	_object = _engine->loadObject(name);
	_engine->runObject(_object);
}

void Character::visible(bool visible) {
	if (visible) {
		_shown = true;
	}
	_visible = visible;
}

void Character::loadState(Common::ReadStream &stream) {
	int x = stream.readUint16LE();
	int y = stream.readUint16LE();
	int dir = stream.readSint16LE();
	debug("character at %d, %d, dir: %d", x, y, dir);
	position(Common::Point(x, y));
	direction(dir);
	_visible = stream.readUint16LE();
	_enabled = stream.readUint16LE();
}

void Character::saveState(Common::WriteStream &stream) const {
	stream.writeUint16LE(_pos.x);
	stream.writeUint16LE(_pos.y);
	stream.writeUint16LE(_direction);
	stream.writeUint16LE(_visible);
	stream.writeUint16LE(_enabled);
}

bool Character::direction(int dir) {
	debug("setDirection %d", dir);
	_direction = dir;

	if (dir < 0)
		return false;

	_animationPos = Common::Point();
	return animate(dir, 100, false);
}

void Character::notifyProcess(const Common::String &name) {
	debug("%s:notifyProcess %s", _name.c_str(), name.c_str());
	if (!_processName.empty())
		_engine->reactivate(name, "Character::notifyProcess", false);

	_processName = name;
}

bool Character::moveTo(const Common::String &processName, Common::Point dst, int dir) {
	if (!_visible)
		return false;

	debug("character move %d,%d %d", dst.x, dst.y, dir);
	notifyProcess(processName);
	_pos = dst;
	_shown = true;
	bool r = direction(dir);

	auto *screen = _engine->getCurrentScreen();
	if (screen) {
		auto objects = screen->find(dst);
		for (auto &object : objects) {
			auto region = object->getTrapRegion();
			if (region && region->pointIn(dst)) {
				debug("starting trap process");
				_engine->runProcess(object, object->getTrapHandler());
			}
		}
	}
	return r;
}

void Character::pointTo(const Common::String &processName, Common::Point dst) {
	debug("character point to stub %d,%d, process: %s", dst.x, dst.y, processName.c_str());
	notifyProcess(processName);
	if (!_processName.empty() && !_engine->activeCurtain()) {
		_engine->reactivate(_processName, "Character::pointTo");
		_processName.clear();
	}
	_shown = true;
}

bool Character::animate(int direction, int speed, bool jokes) {
	if (direction == -1 || !_enabled)
		return false;

	if (_stopped) {
		debug("character stopped, skipping");
		_stopped = false;
		return false;
	}

	auto character = jokes ? _engine->jokes() : this;
	auto description = character->animationDescription(direction);
	if (!description) {
		warning("no %s animation %d", jokes ? "jokes" : "character", direction);
		return false;
	}
	auto animation = _engine->loadAnimation(description->filename);
	if (!animation) {
		warning("no %s animation file %s", jokes ? "jokes" : "character", description->filename.c_str());
		return false;
	}
	_description = description;
	_shown = true;
	_animation = animation;
	_animation->speed(speed);
	_animation->rewind();
	_phase = 0;
	_frames = _animation->frames();
	_jokes = jokes;
	if (jokes)
		_jokesDirection = direction;
	else
		_direction = direction;
	debug("character animation frames: %d, enabled: %d, visible: %d", _frames, _enabled, _visible);
	return true;
}

bool Character::animate(Common::Point pos, int direction, int speed) {
	debug("animate character: %d,%d %d %d", pos.x, pos.y, direction, speed);
	auto ok = animate(direction, speed, true);
	if (!ok)
		return false;

	_animationPos = pos;

	return true;
}

void Character::stop() {
	_stopped = true;
}

void Character::leave(const Common::String &processName) {
	debug("character %s: leave, process: %s", _object->getName().c_str(), processName.c_str());
	notifyProcess(processName);
}

void Character::tick(bool reactivate) {
	if (!active())
		return;
	if (_animation) {
		auto screen = _engine->getCurrentScreen();
		auto scale = screen ? screen->getZScale(_pos.y) : 1;
		_animation->scale(scale);

		if (!_stopped && _phase >= 0 && _phase < _frames) {
			_animation->tick();
			_phase = _animation->phase();
			if (_phase >= _frames) {
				bool wasJokes = _jokes;
				_jokes = false;
				_phase = -1;
				_frames = 0;
				if (wasJokes)
					direction(_direction);
			}
			return;
		}
	}

	if (reactivate && !_processName.empty() && !_engine->activeCurtain()) {
		_engine->reactivate(_processName, "Character::tick");
		_processName.clear();
	}
}

bool Character::pointIn(Common::Point pos) const {
	if (!_animation)
		return false;

	Common::Rect rect(_animation->width(), _animation->height());
	rect.moveTo(animationPosition());
	return rect.contains(pos);
}

Common::Point Character::animationPosition() const {
	Common::Point pos = _pos + _animationPos;

	if (_animation) {
		pos.y -= _animation->visibleHeight();
		pos.x -= _animation->visibleCenter();

		if (_description) {
			auto &frames = _description->frames;
			if (_phase >= 0 && _phase < static_cast<int>(frames.size())) {
				auto &frame = frames[_phase];
				pos.x += frame.x * _animation->scale();
				pos.y += frame.y * _animation->scale();
			}
		}
	}
	return pos;
}

void Character::paint(Graphics::Surface &backbuffer, Common::Point pos) const {
	if (!_enabled || !visible() || !_animation)
		return;

	pos += animationPosition();

	int fogAlpha = 0;
	if (_fog) {
		auto z = this->z();
		if (z >= _fogMinZ && z < _fogMaxZ) {
			fogAlpha = 255 * (z - _fogMinZ) / (_fogMaxZ - _fogMinZ);
		} else if (z >= _fogMaxZ) {
			fogAlpha = 255;
		}
	}
	_animation->paint(backbuffer, pos, _fog.get(), fogAlpha);
}

int Character::z() const {
	int y = _pos.y + _animationPos.y;
	// fixme: add temp var : _movePos?
	// debug("char z = %d", y);
	return g_system->getHeight() - y;
}

void Character::reset() {
	_fog.reset();
	_shown = false;
	_animation.reset();
	_phase = -1;
	_frames = 0;
}

void Character::setFog(Graphics::ManagedSurface *surface, int minZ, int maxZ) {
	_fog.reset(surface);
	_fogMinZ = minZ;
	_fogMaxZ = maxZ;
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
