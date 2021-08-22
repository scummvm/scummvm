#include "hypno/hypno.h"
#include "hypno/grammar.h"

#include "common/events.h"

namespace Hypno {

SpiderEngine::SpiderEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void SpiderEngine::loadAssets() {
	LibData files; 
	loadLib("sixdemo/c_misc/missions.lib", files);
	uint32 i = 0;
	uint32 j = 0;

	// start level
	Level start;
	start.trans.level = "sixdemo/mis/demo.mis";
	start.trans.intros.push_back("sixdemo/demo/dcine1.smk");
	start.trans.intros.push_back("sixdemo/demo/dcine2.smk");
	_levels["<start>"] = start;

	Common::String arc;
	Common::String list;

	debug("Splitting file: %s",files[0].name.c_str());
	for (i = 0; i < files[0].data.size(); i++) {
		arc += files[0].data[i];
		if (files[0].data[i] == 'X') {
			i++;
			for (j = i; j < files[0].data.size(); j++) {
				if (files[0].data[j] == 'Y')
					break;
				list += files[0].data[j];
			}
			break; // No need to keep parsing, no more files are used in the demo
		}
	}

	Common::String arclevel = files[0].name; 
	parseArcadeShooting("sixdemo", arclevel, arc);
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);

	loadLib("sixdemo/c_misc/fonts.lib", _fontFiles);
	loadLib("sixdemo/c_misc/sound.lib", _soundFiles);
	loadLib("sixdemo/demo/sound.lib", _soundFiles);

	// Read assets from mis files
	parseScene("sixdemo", "mis/demo.mis");
	ChangeLevel *cl = new ChangeLevel();
	cl->level = "sixdemo/c_misc/missions.lib/c1.mi_";
	_levels["sixdemo/mis/demo.mis"].scene.hots[1].actions.push_back(cl);

	cl = new ChangeLevel();
	cl->level = "sixdemo/mis/alley.mis";
	_levels["sixdemo/mis/demo.mis"].scene.hots[2].actions.push_back(cl);

	cl = new ChangeLevel();
	cl->level = "sixdemo/puz_matr";
	_levels["sixdemo/mis/demo.mis"].scene.hots[3].actions.push_back(cl);	

	cl = new ChangeLevel();
	cl->level = "sixdemo/mis/shoctalk.mis";
	_levels["sixdemo/mis/demo.mis"].scene.hots[4].actions.push_back(cl);
	
	cl = new ChangeLevel();
	cl->level = "sixdemo/mis/order.mis";
	_levels["sixdemo/mis/demo.mis"].scene.hots[5].actions.push_back(cl);
	_levels["sixdemo/mis/demo.mis"].scene.sound = "sixdemo/demo/sound.lib/menu_mus.raw";

	parseScene("sixdemo", "mis/order.mis");
	cl = new ChangeLevel();
	cl->level = "<quit>";
	_levels["sixdemo/mis/order.mis"].scene.hots[1].actions.push_back(cl);

	parseScene("sixdemo", "mis/alley.mis");
	_levels["sixdemo/mis/alley.mis"].scene.intro = "demo/aleyc01s.smk";
	_levels["sixdemo/mis/alley.mis"].scene.sound = "sixdemo/demo/sound.lib/alleymus.raw";

	parseScene("sixdemo", "mis/shoctalk.mis");

	Level matrix;
	matrix.puzzle.name = "sixdemo/puz_matr";
	matrix.puzzle.intros.push_back("spiderman/demo/aleyc01s.smk");
	matrix.puzzle.levelIfWin = "sixdemo/mis/demo.mis";
	matrix.puzzle.levelIfLose = "sixdemo/mis/demo.mis";
	_levels["sixdemo/puz_matr"] = matrix; 

}

void SpiderEngine::runPuzzle(Puzzle puzzle) {
	if (puzzle.name == "sixdemo/puz_matr")
		runMatrix(puzzle);
	else 
		error("invalid puzzle");
}

void SpiderEngine::runMatrix(Puzzle puzzle) {
	Common::Point mousePos;
	Common::Event event;

	defaultCursor();
	bool data[10][10] = {};
	bool solution[10][10] = { 
		{0, 0, 0, 1, 1, 1, 1, 0, 0, 0},  
		{0, 0, 1, 1, 1, 1, 1, 1, 0, 0},  
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},  
		{0, 1, 1, 0, 0, 0, 0, 1, 1, 0},  
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},  
		{0, 0, 1, 1, 1, 1, 1, 1, 0, 0},  
		{0, 0, 1, 1, 0, 0, 1, 1, 0, 0},  
		{0, 0, 1, 1, 0, 0, 1, 1, 0, 0},  
		{0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, 
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	};
	Common::Rect matrix(175, 96, 461, 385);
	Common::Rect cell(0, 0, 27, 27);
	uint32 activeColor = _pixelFormat.RGBToColor(0, 130, 0);
	uint32 deactiveColor = _pixelFormat.RGBToColor(0, 0, 0);
	int x, y;
	bool found;

	loadImage("sixdemo/puz_matr/matrixbg.smk", 0, 0, false);
	MVideo v("sixdemo/puz_matr/matintro.smk", Common::Point(0, 0), false, false, false);
	playVideo(v);
	while (!shouldQuit()) {
		
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				playSound("sixdemo/demo/sound.lib/matrix.raw", 1);
				if (matrix.contains(mousePos)) {
					x = (mousePos.x - 175) / 29;
					y = (mousePos.y - 96) / 29;
					cell.moveTo(175 + 29*x + 1, 96 + 29*y + 1);
					_compositeSurface->fillRect(cell, data[x][y] ? deactiveColor : activeColor);
					data[x][y] = !data[x][y];
				}
				break;

			default:
				break;
			}
		}

		found = true;
		for (x = 0; x < 10; x++) {
			for (y = 0; y < 10; y++) {
				if (data[x][y] != solution[y][x]) {
					found = false;
					break;
				}
			}
			if (!found)
				break;
		}
				
		if (found) {
			playSound("sixdemo/demo/sound.lib/matrix_2.raw", 1);
			_nextLevel = puzzle.levelIfWin;
			return;
		} 

		if (v.decoder->needsUpdate()) {
			updateScreen(v);
		}


		drawScreen();
		g_system->delayMillis(10);
	}
}

} // End of namespace Hypno