#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"

namespace Hypno {

BoyzEngine::BoyzEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void BoyzEngine::loadAssets() {
	LibData files;
	loadLib("boyz/preload/missions.lib", files);
}

}