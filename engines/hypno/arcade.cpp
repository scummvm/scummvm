#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

void HypnoEngine::drawPlayer(Common::String player, MVideo &background) { error("not implemented"); }
void HypnoEngine::drawHealth() { error("not implemented"); }
void HypnoEngine::drawShoot(Common::Point target) { error("not implemented"); }


void HypnoEngine::runArcade(ArcadeShooting arc) {

	_font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	Common::Event event;
	Common::Point mousePos;
	Common::List<uint32> shootsToRemove;
	ShootSequence shootSequence = arc.shootSequence;
	_shootSound = arc.shootSound;
	_health = arc.health;
	_maxHealth = _health;
	_defaultCursor = "arcade";
	_shoots.clear();

	MVideo background = MVideo(arc.background, Common::Point(0, 0), false, false, false);

	changeCursor("arcade");
	playVideo(background);

	while (!shouldQuit()) {

		if (background.decoder->needsUpdate())
			updateScreen(background);

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				drawShoot(mousePos);
				clickedShoot(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				drawCursorArcade(mousePos);
				break;

			default:
				break;
			}
		}

		if (_health <= 0) {
			skipVideo(background);
			_nextLevel = arc.levelIfLose;
			return;
		}

		if (background.decoder->endOfVideo()) {
			skipVideo(background);
			_nextLevel = arc.levelIfWin;
			return;
		}

		if (shootSequence.size() > 0) {
			ShootInfo si = shootSequence.front();
			if (si.timestamp <= background.decoder->getCurFrame()) {
				shootSequence.pop_front();
				for (Shoots::iterator it = arc.shoots.begin(); it != arc.shoots.end(); ++it) {
					if (it->name == si.name && it->animation != "NONE") {
						Shoot s = *it;
						s.video = new MVideo(it->animation, it->position, true, false, false);
						playVideo(*s.video);
						_shoots.push_back(s);
						playSound(_soundPath + s.startSound, 1);
					}
				}
			}
		}

		uint32 i = 0;
		shootsToRemove.clear();

		for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
			if (it->video->decoder) {
				int frame = it->video->decoder->getCurFrame();
				if (frame > 0 && frame >= it->explosionFrame - 3 && !it->destroyed) {
					_health = _health - it->damage;
					skipVideo(*it->video);
				} else if (it->video->decoder->endOfVideo()) {
					skipVideo(*it->video);
					shootsToRemove.push_back(i);
				} else if (it->video->decoder->needsUpdate()) {
					updateScreen(*it->video);
				}
			}
			i++;
		}
		if (shootsToRemove.size() > 0) {
			for (Common::List<uint32>::iterator it = shootsToRemove.begin(); it != shootsToRemove.end(); ++it) {
				//debug("removing %d from %d size", *it, _shoots.size());
				_shoots.remove_at(*it);
			}
		}

		if (_music.empty()) {
			_music = _soundPath + arc.music;
			playSound(_music, 0);
		}

		drawPlayer(arc.player, background);
		drawHealth();

		drawScreen();
		g_system->delayMillis(10);
	}
}

int HypnoEngine::detectTarget(Common::Point mousePos) {
	int i = -1;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		i++;
		if (it->destroyed || !it->video->decoder)
			continue;
		x = mousePos.x - it->position.x;
		y = mousePos.y - it->position.y;
		w = it->video->decoder->getWidth();
		h = it->video->decoder->getHeight();
		if (it->video->decoder && x >= 0 && y >= 0 && x < w && y < h) {
			if (it->video->currentFrame->getPixel(x, y) > 0)
				return i;
		}
	}
	return -1;
}

void HypnoEngine::drawCursorArcade(Common::Point mousePos) {
	int i = detectTarget(mousePos);
	if (i > 0)
		changeCursor("target");
	else
		changeCursor("arcade");

}

bool HypnoEngine::clickedShoot(Common::Point mousePos) {
	int i = detectTarget(mousePos);
	int w = 0;
	int h = 0;
	if (i > 0) {
		playSound(_soundPath + _shoots[i].endSound, 1);
		w = _shoots[i].video->decoder->getWidth();
		h = _shoots[i].video->decoder->getHeight();
		_shoots[i].destroyed = true;
		_shoots[i].video->position = Common::Point(mousePos.x - w / 2, mousePos.y - h / 2);
		_shoots[i].video->decoder->forceSeekToFrame(_shoots[i].explosionFrame + 2);
	}
	return (i > 0);
}

} // End of namespace Hypno