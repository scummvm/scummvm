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

#include "mm/mm1/views_enh/game_view.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define TICKS_PER_FRAME 4

namespace Animations {

ViewAnimation::ViewAnimation(const char *prefix, uint count, uint frameCount) :
		_sound(*g_engine->_sound), _frameCount(frameCount) {
	_backgrounds.resize(count);

	for (uint i = 0; i < _backgrounds.size(); ++i) {
		Common::String name = Common::String::format(
			"%s%d.twn", prefix, i + 1);
		_backgrounds[i].load(name);
	}
}

void ViewAnimation::tick() {
	_frameIndex = (_frameIndex + 1) % _frameCount;
}

void ViewAnimation::draw(Graphics::ManagedSurface &s) {
	_backgrounds[_frameIndex / 8].draw(&s, _frameIndex % 8,
		Common::Point(0, 0));
}

void ViewAnimation::leave() {
	_sound.stopSound();
	_sound.stopSong();
}

/*------------------------------------------------------------------------*/

class Training : public ViewAnimation {
public:
	Training() : ViewAnimation("trng", 2, 16) {}
	~Training() override {}

	void enter() override {
		_sound.playVoice("hello1.voc");
		_sound.playSong("grounds.m");
	}
};

class Market : public ViewAnimation {
public:
	Market() : ViewAnimation("gild", 4, 32) {}
	~Market() override {}

	void enter() override {
		_sound.playVoice("guild10.voc");
		_sound.playSong("guild.m");
	}
};

class Temple : public ViewAnimation {
public:
	Temple() : ViewAnimation("tmpl", 4, 26) {}
	~Temple() override {}

	void enter() override {
		_sound.playVoice("maywe2.voc");
		_sound.playSong("temple.m");
	}
};

class Blacksmith : public ViewAnimation {
public:
	Blacksmith() : ViewAnimation("blck", 2, 13) {}
	~Blacksmith() override {}

	void enter() override {
		_sound.playVoice("whaddayo.voc");
		_sound.playSong("smith.m");
	}
};

class Tavern : public ViewAnimation {
public:
	Tavern() : ViewAnimation("tvrn", 2, 16) {}
	~Tavern() override {}

	void enter() override {
		_sound.playVoice("hello.voc");
		_sound.playSong("tavern.m");
	}

	void leave() override {
		ViewAnimation::leave();
		_sound.playVoice("goodbye.voc");
	}
};

} // namespace Animations

/*------------------------------------------------------------------------*/

bool GameView::msgGame(const GameMessage &msg) {
	if (msg._name == "LOCATION") {
		showLocation(msg._value);

	} else if (msg._name == "LOCATION_DRAW") {
		UIElement *view = g_events->findView("Game");
		view->draw();

	} else {
		return Views::GameView::msgGame(msg);
	}

	return true;
}

void GameView::showLocation(int locationId) {
	if (locationId == -1) {
		_anim->leave();
		delete _anim;
		_anim = nullptr;

	} else {
		assert(!_anim);
		switch (locationId) {
		case LOC_TRAINING:
			_anim = new Animations::Training();
			break;
		case LOC_MARKET:
			_anim = new Animations::Market();
			break;
		case LOC_TEMPLE:
			_anim = new Animations::Temple();
			break;
		case LOC_BLACKSMITH:
			_anim = new Animations::Blacksmith();
			break;
		case LOC_TAVERN:
			_anim = new Animations::Tavern();
			break;
		default:
			error("Unknown location type");
			break;
		}

		_anim->enter();
	}
}

void GameView::draw() {
	if (_anim == nullptr) {
		Views::GameView::draw();
	} else {
		Graphics::ManagedSurface s = getSurface();
		_anim->draw(s);
	}
}

bool GameView::tick() {
	if (_anim != nullptr) {
		if (++_timerCtr >= TICKS_PER_FRAME) {
			_timerCtr = 0;
			_anim->tick();
		}

		redraw();
	}

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
