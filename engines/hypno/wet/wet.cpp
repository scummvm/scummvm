#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

WetEngine::WetEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void WetEngine::loadAssets() {
	LibData files;
	loadLib("wetlands/c_misc/missions.lib", files);
	uint32 i = 0;
	uint32 j = 0;
	uint32 k = 0;

	Common::String arc;
	Common::String list;
	Common::String arclevel;

	Level start;
	Hotspot h;
	Hotspots hs;

	h.type = MakeMenu;
	Ambient *a = new Ambient();
	a->path = "movie/selector.smk";
	a->fullscreen = true;
	a->flag = "/LOOP";
	a->origin = Common::Point(0, 0);
	h.actions.push_back(a);

	hs.push_back(h);

	h.type = MakeHotspot;
	h.rect = Common::Rect(0, 424, 233, 462);
	h.actions.clear();
	ChangeLevel *cl = new ChangeLevel();
	cl->level = "<intro>";
	h.actions.push_back(cl);

	hs.push_back(h);

	h.rect = Common::Rect(504, 424, 637, 480);
	Quit *q = new Quit();
	h.actions.clear();
	h.actions.push_back(q);

	hs.push_back(h);

	start.scene.hots = hs;
	_levels["<start>"] = start;

	Level intro;
	intro.trans.level = files[0].name;
	intro.trans.intros.push_back("movie/nw_logo.smk");
	intro.trans.intros.push_back("movie/hypnotix.smk");
	intro.trans.intros.push_back("movie/wetlogo.smk");
	_levels["<intro>"] = intro;

	for (k = 0; k < files.size(); k++) {
		arc.clear();
		list.clear();
		arclevel = files[k].name;

		debug("Parsing %s", arclevel.c_str());
		for (i = 0; i < files[k].data.size(); i++) {
			arc += files[k].data[i];
			if (files[k].data[i] == 'X') {
				i++;
				for (j = i; j < files[k].data.size(); j++) {
					if (files[k].data[j] == 'Y')
						break;
					list += files[k].data[j];
				}
				break; // No need to keep parsing
			}
		}

		parseArcadeShooting("wetlands", arclevel, arc);
		_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
		_levels[arclevel].arcade.prefix = "wetlands";
		//_levels[arclevel].arcade.levelIfLose = "<gameover>";
		//Common::replace(_levels[arclevel].arcade.music, "sound\\", "");
		if (k < files.size() - 1) {
			_levels[arclevel].arcade.levelIfWin = files[k + 1].name;
			// failing a level in the demo takes you to the next one anyway
			_levels[arclevel].arcade.levelIfLose = files[k + 1].name; 
		}
			
	}

	// After finish the second level, it's always game over
	_levels[files[k-1].name].arcade.levelIfWin = "<gameover>";
	_levels[files[k-1].name].arcade.levelIfLose = "<gameover>"; 

	Level over;
	over.trans.level = "<quit>";
	over.trans.intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("wetlands/c_misc/fonts.lib", _fontFiles);
	loadLib("wetlands/c_misc/sound.lib", _soundFiles);
}

} // End of namespace Hypno
