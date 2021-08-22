#include "hypno/hypno.h"
#include "hypno/grammar.h"

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
}


} // End of namespace Hypno