//
// Created by andrei on 3/17/18.
//

#include "walk_mgr.h"
#include "walk_location.h"
#include "engines/pink/objects/actors/lead_actor.h"
#include "engines/pink/archive.h"


void Pink::WalkMgr::deserialize(Pink::Archive &archive) {
    _leadActor = static_cast<LeadActor*>(archive.readObject());
    archive >> _locations;
}

Pink::WalkLocation *Pink::WalkMgr::findLocation(Common::String &name) {
    return *Common::find_if(_locations.begin(), _locations.end(), [&name] (WalkLocation *location) {
        return location->getName() == name;
    });
}
