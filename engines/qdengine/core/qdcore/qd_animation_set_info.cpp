
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_animation_set_info.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"
#include "qdengine/core/qdcore/qd_game_scene.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationSet *qdAnimationSetInfo::animation_set() const {
	if (name()) {
		if (qdGameScene * p = static_cast<qdGameScene * >(owner(QD_NAMED_OBJECT_SCENE))) {
			if (qdAnimationSet * set = p -> get_animation_set(name()))
				return set;
		}

		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher())
			return p -> get_animation_set(name());
	}

	return NULL;
}
} // namespace QDEngine
