
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_sound_info.h"
#include "qd_game_scene.h"
#include "qd_game_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdSound* qdSoundInfo::sound() const
{
	if(name()){
		if(qdGameScene* p = static_cast<qdGameScene*>(owner(QD_NAMED_OBJECT_SCENE))){
			if(qdSound* snd = p -> get_sound(name()))
				return snd;
		}

		if(qdGameDispatcher* p = qdGameDispatcher::get_dispatcher())
			return p -> get_sound(name());
	}

	return NULL;
}
