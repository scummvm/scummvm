#include "hypno/hypno.h"
#include "hypno/grammar.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

void HypnoEngine::shootSpiderweb(Common::Point target) {
	uint32 c = _pixelFormat.RGBToColor(255, 255, 255);
	_compositeSurface->drawLine(0, 300, target.x, target.y+1, c);
	_compositeSurface->drawLine(0, 300, target.x, target.y  , c);
	_compositeSurface->drawLine(0, 300, target.x, target.y-1, c);

	drawScreen();
	g_system->delayMillis(2);
}

void HypnoEngine::runArcade(ArcadeShooting arc) {

	Common::Event event;
	Common::Point mousePos;
	Common::List<uint32> videosToRemove;

	MVideo background = MVideo(arc.background, Common::Point(0, 0), false, false, false);	
	Graphics::Surface *sp = decodeFrame(arc.player, 2);

	CursorMan.showMouse(false);
	changeCursor("mouse/cursor1.smk", 0);
	playVideo(background);

	while (!shouldQuit()) {
		
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
				break;

			default:
				break;
			}
		}

		// Movies
		for (Videos::iterator it = _nextParallelVideoToPlay.begin(); it != _nextParallelVideoToPlay.end(); ++it) {
			playVideo(*it);
			_videosPlaying.push_back(*it);
		}

		if (_nextParallelVideoToPlay.size() > 0)
			_nextParallelVideoToPlay.clear();

		if (background.videoDecoder->endOfVideo()) {
			skipVideo(background);
			_nextSetting = "mis/demo.mis";
			return;
		}


		if (background.videoDecoder->needsUpdate())
			updateScreen(background);

		if (_shootInfos.size() > 0) {
			ShootInfo si = _shootInfos.front();
			if (si.timestamp <= background.videoDecoder->getCurFrame()) {
				_shootInfos.pop_front();
				for (Shoots::iterator it = arc.shoots.begin(); it != arc.shoots.end(); ++it) {
					if (it->name == si.name) {
						_nextParallelVideoToPlay.push_back(MVideo(it->animation, it->position , true, false, false));
						_nextParallelVideoToPlay[0].finishBeforeEnd = 24;
					}
				}
			}
		}

		//drawImage(*sp, 60, 129, true);
		uint32 i = 0;
		videosToRemove.clear();

		for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
			if (it->videoDecoder) {
				if (it->videoDecoder-> getCurFrame() > 0 && it->videoDecoder-> getCurFrame() >= it->videoDecoder->getFrameCount() - it->finishBeforeEnd) {
				delete it->videoDecoder;
				it->videoDecoder = nullptr;
				videosToRemove.push_back(i);

				} else if (it->videoDecoder->needsUpdate()) {
					updateScreen(*it);
				}
			}
			i++;
		}

		if (videosToRemove.size() > 0) {
			for(Common::List<uint32>::iterator it = videosToRemove.begin(); it != videosToRemove.end(); ++it) {
				debug("removing %d from %d size", *it, _videosPlaying.size()); 
				_videosPlaying.remove_at(*it);
			}
		}

		if (_music.empty()) {
			_music = "c_misc/sound.lib/" + arc.sounds.front();
			playSound(_music, 0);
		}

		drawScreen();
		g_system->delayMillis(1);
	}
}

bool HypnoEngine::clickedShoot(Common::Point mousePos) {
	bool found = false;
	int x;
	int y;
	int i = 0;
	//it++;
	for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
		x = mousePos.x - it->position.x;
		y = mousePos.y - it->position.y;
		if (it->videoDecoder && x >= 0 && y >= 0 && x < it->videoDecoder->getWidth() && y < it->videoDecoder->getHeight()) {
			uint32 c = it->currentFrame->getPixel(x, y);
			debug("inside %x", c); 
			if (c > 0) {
				//it->videoDecoder->seekToFrame(it->finishBeforeEnd+1); 
				delete it->videoDecoder;
				it->videoDecoder = nullptr;
				//videosToRemove.push_back(i);
				//it->videoDecoder->start();
			}
		}
		i++;
	}
	return found;
}

} // End of namespace Hypno