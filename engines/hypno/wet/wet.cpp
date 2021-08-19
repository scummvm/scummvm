#include "hypno/hypno.h"
#include "hypno/grammar.h"

namespace Hypno {

WetEngine::WetEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void WetEngine::loadAssets() {
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
	_nextSetting = "c_misc/missions.lib/c31.mi_";
}

} // End of namespace Hypno
