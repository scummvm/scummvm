#include "hypno/hypno.h"
#include "hypno/grammar.h"

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
	start.trans.level = files[0].name;
	start.trans.intros.push_back("movie/nw_logo.smk");
	start.trans.intros.push_back("movie/hypnotix.smk");
	start.trans.intros.push_back("movie/wetlogo.smk");
	_levels["<start>"] = start;

	for (k = 0; k < files.size(); k++) {
		arc.clear();
		list.clear();
		arclevel = files[k].name;

		debug("Parsing %s",arclevel.c_str());
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
		_levels[arclevel].arcade.levelIfLose = "<gameover>";
		if (k < files.size()-1)
			_levels[arclevel].arcade.levelIfWin = files[k+1].name;
	}


	debug("%s", arc.c_str());
	debug("------------------------------------");

	Level over;
	over.trans.level = "<quit>";
	over.trans.intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;


	loadLib("wetlands/c_misc/fonts.lib", _fontFiles);
	loadLib("wetlands/c_misc/sound.lib", _soundFiles);
}

} // End of namespace Hypno
