#include "hypno/hypno.h"
#include "hypno/grammar.h"

namespace Hypno {

SpiderEngine::SpiderEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void SpiderEngine::loadAssets() {
	LibData files; 
	loadLib("c_misc/missions.lib", files);
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
	parseArcadeShooting(arclevel, arc);
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);

	loadLib("c_misc/fonts.lib", _fontFiles);
	loadLib("c_misc/sound.lib", _soundFiles);
	loadLib("demo/sound.lib", _soundFiles);

	// quit level
	Hotspot q;
	q.type = MakeMenu;
	Action *a = new Quit();
	q.actions.push_back(a);
	Level level;
	Hotspots quit;
	quit.push_back(q);
	level.scene.hots = quit;  
	_levels["mis/quit.mis"] = level;

	// Read assets from mis files
	parseScene("mis/demo.mis");
	_levels["mis/demo.mis"].intros.push_back(MVideo("demo/dcine1.smk", Common::Point(0, 0), false, true, false));
	_levels["mis/demo.mis"].intros.push_back(MVideo("demo/dcine2.smk", Common::Point(0, 0), false, true, false));
	_levels["mis/demo.mis"].scene.hots[1].setting = "c_misc/missions.lib/c1.mi_";
	_levels["mis/demo.mis"].scene.hots[2].setting = "mis/alley.mis";

	_levels["mis/demo.mis"].scene.hots[4].setting = "mis/shoctalk.mis";
	_levels["mis/demo.mis"].scene.hots[5].setting = "mis/order.mis";
	_levels["mis/demo.mis"].scene.sound = "demo/sound.lib/menu_mus.raw";

	parseScene("mis/order.mis");
	_levels["mis/order.mis"].scene.hots[1].setting = "mis/quit.mis";
	parseScene("mis/alley.mis");
	_levels["mis/alley.mis"].intros.push_back(MVideo("demo/aleyc01s.smk", Common::Point(0, 0), false, true, false));
	_levels["mis/alley.mis"].scene.sound = "demo/sound.lib/alleymus.raw";

	parseScene("mis/shoctalk.mis");
	_nextSetting = "mis/demo.mis";
}


} // End of namespace Hypno