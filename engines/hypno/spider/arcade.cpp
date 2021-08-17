#include "hypno/hypno.h"
#include "hypno/grammar.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

void HypnoEngine::shootSpiderweb(Common::Point target) {
	uint32 c = _pixelFormat.RGBToColor(255, 255, 255);
	_compositeSurface->drawLine(80, 155, target.x, target.y+1, c);
	_compositeSurface->drawLine(80, 155, target.x, target.y  , c);
	_compositeSurface->drawLine(80, 155, target.x, target.y-1, c);
	//g_system->delayMillis(2);
}

void HypnoEngine::drawPlayer(Common::String player, uint32 idx) {
	Graphics::Surface *image = decodeFrame(player, idx);
	drawImage(*image, 60, 129, true);
	image->free();
	delete image;
}

void HypnoEngine::runArcade(ArcadeShooting arc) {

	Common::Event event;
	Common::Point mousePos;
	Common::List<uint32> shootsToRemove;
	ShootSequence shootSequence = arc.shootSequence;
	_shoots.clear();

	MVideo background = MVideo(arc.background, Common::Point(0, 0), false, false, false);
	uint32 playerIdx = 0;

	CursorMan.showMouse(false);
	changeCursor("mouse/cursor1.smk", 0);
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
				shootSpiderweb(mousePos);
				clickedShoot(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				playerIdx = mousePos.x / 50;
				break;

			default:
				break;
			}
		}

		if (background.decoder->endOfVideo()) {
			skipVideo(background);
			_nextSetting = "mis/demo.mis";
			return;
		}

		if (shootSequence.size() > 0) {
			ShootInfo si = shootSequence.front();
			if (si.timestamp <= background.decoder->getCurFrame()) {
				shootSequence.pop_front();
				for (Shoots::iterator it = arc.shoots.begin(); it != arc.shoots.end(); ++it) {
					if (it->name == si.name) {
						Shoot s = *it;
						s.video = new MVideo(it->animation, it->position , true, false, false);
						playVideo(*s.video);
						_shoots.push_back(s);
					}
				}
			}
		}

		uint32 i = 0;
		shootsToRemove.clear();

		for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
			if (it->video->decoder) {
				int frame = it->video->decoder->getCurFrame(); 
				if (frame > 0 && frame >= it->explosionFrame-3 && !it->destroyed) {
					skipVideo(*it->video);
				} else if (it->video->decoder->endOfVideo()){
					skipVideo(*it->video);
					shootsToRemove.push_back(i);	
				} else if (it->video->decoder->needsUpdate()) {
					updateScreen(*it->video);
				}
				
			}
			i++;
		}
		if (shootsToRemove.size() > 0) {
			for(Common::List<uint32>::iterator it = shootsToRemove.begin(); it != shootsToRemove.end(); ++it) {
				//debug("removing %d from %d size", *it, _shoots.size()); 
				_shoots.remove_at(*it);
			}
		}

		if (_music.empty()) {
			_music = "c_misc/sound.lib/" + arc.sounds.front();
			playSound(_music, 0);
		}

		drawPlayer(arc.player, playerIdx);

		drawScreen();
		g_system->delayMillis(10);
	}
}

bool HypnoEngine::clickedShoot(Common::Point mousePos) {
	bool found = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		if (it->destroyed || !it->video->decoder)
			continue;
		x = mousePos.x - it->position.x;
		y = mousePos.y - it->position.y;
		w = it->video->decoder->getWidth();
		h = it->video->decoder->getHeight(); 

		if (it->video->decoder && x >= 0 && y >= 0 && x < w && y < h) {
			uint32 c = it->video->currentFrame->getPixel(x, y);
			//debug("inside %x", c); 
			if (c > 0) {
				it->destroyed = true;
				it->video->position = Common::Point(mousePos.x - w/2, mousePos.y - h/2);
				it->video->decoder->forceSeekToFrame(it->explosionFrame+2);
			}
		}
	}
	return found;
}

} // End of namespace Hypno