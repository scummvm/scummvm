#include "hypno/hypno.h"
#include "hypno/grammar.h"

namespace Hypno {

SpiderEngine::SpiderEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void SpiderEngine::loadAssets() {
	LibData files; 
	loadLib("sixdemo/c_misc/missions.lib", files);
	uint32 i = 0;
	uint32 j = 0;

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

	// start level
	Level start;
	start.trans.level = "sixdemo/mis/demo.mis";
	start.trans.intros.push_back(MVideo("sixdemo/demo/dcine1.smk", Common::Point(0, 0), false, true, false));
	start.trans.intros.push_back(MVideo("sixdemo/demo/dcine2.smk", Common::Point(0, 0), false, true, false));
	_levels["<start>"] = start;

	// quit level
	Hotspot q;
	q.type = MakeMenu;
	Action *a = new Quit();
	q.actions.push_back(a);
	Level quit;
	Hotspots hs;
	hs.push_back(q);
	quit.scene.hots = hs;  
	_levels["<quit>"] = quit;

	// Read assets from mis files
	parseScene("sixdemo", "mis/demo.mis");
	_levels["sixdemo/mis/demo.mis"].scene.hots[1].setting = "sixdemo/c_misc/missions.lib/c1.mi_";
	_levels["sixdemo/mis/demo.mis"].scene.hots[2].setting = "sixdemo/mis/alley.mis";

	_levels["sixdemo/mis/demo.mis"].scene.hots[4].setting = "sixdemo/mis/shoctalk.mis";
	_levels["sixdemo/mis/demo.mis"].scene.hots[5].setting = "sixdemo/mis/order.mis";
	_levels["sixdemo/mis/demo.mis"].scene.sound = "sixdemo/demo/sound.lib/menu_mus.raw";

	parseScene("sixdemo", "mis/order.mis");
	_levels["sixdemo/mis/order.mis"].scene.hots[1].setting = "sixdemo/mis/quit.mis";
	parseScene("sixdemo", "mis/alley.mis");
	_levels["sixdemo/mis/alley.mis"].scene.intro = "demo/aleyc01s.smk";
	_levels["sixdemo/mis/alley.mis"].scene.sound = "sixdemo/demo/sound.lib/alleymus.raw";

	parseScene("sixdemo", "mis/shoctalk.mis");
}


} // End of namespace Hypno