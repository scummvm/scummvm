/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_game_scene.h"
#include "qd_game_dispatcher.h"
#include "qd_screen_text_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdScreenTextDispatcher::qdScreenTextDispatcher()
{
	text_sets_.reserve(16);
}

qdScreenTextDispatcher::~qdScreenTextDispatcher()
{
}


void qdScreenTextDispatcher::redraw() const
{
#ifdef _QUEST_EDITOR
	std::vector<qdScreenTextSet>::const_iterator is;
	FOR_EACH(text_sets_,is)
		is -> redraw();
#endif
}

static bool operator == (const qdScreenTextSet& set,int id)
{
	return set.ID() == id;
}

qdScreenText* qdScreenTextDispatcher::add_text(int set_ID,const qdScreenText& txt)
{
	std::vector<qdScreenTextSet>::iterator it = std::find(text_sets_.begin(),text_sets_.end(),set_ID);

	if(it != text_sets_.end())
		return it -> add_text(txt);

	return NULL;
}

void qdScreenTextDispatcher::clear_texts()
{
	std::vector<qdScreenTextSet>::iterator it;
	FOR_EACH(text_sets_,it)
		it -> clear_texts();
}

void qdScreenTextDispatcher::clear_texts(qdNamedObject* p)
{
	std::vector<qdScreenTextSet>::iterator it;
	FOR_EACH(text_sets_,it)
		it -> clear_texts(p);
}

const qdScreenTextSet* qdScreenTextDispatcher::get_text_set(int id) const
{
	std::vector<qdScreenTextSet>::const_iterator it = std::find(text_sets_.begin(),text_sets_.end(),id);

	if(it != text_sets_.end())
		return &*it;

	return NULL;
}

qdScreenTextSet* qdScreenTextDispatcher::get_text_set(int id)
{
	std::vector<qdScreenTextSet>::iterator it = 
		std::find(text_sets_.begin(),text_sets_.end(),id);

	if(it != text_sets_.end())
		return &*it;

	return NULL;
}

void qdScreenTextDispatcher::pre_redraw() const
{
#ifdef _QUEST_EDITOR
	std::for_each(text_sets_.begin(),text_sets_.end(),std::mem_fun_ref(qdScreenTextSet::pre_redraw));
#endif
}

void qdScreenTextDispatcher::post_redraw()
{
#ifdef _QUEST_EDITOR
	std::for_each(text_sets_.begin(),text_sets_.end(),std::mem_fun_ref(qdScreenTextSet::post_redraw));
#endif
}

bool qdScreenTextDispatcher::save_script(XStream& fh,int indent) const
{
	std::vector<qdScreenTextSet>::const_iterator it;
	FOR_EACH(text_sets_,it)
		it -> save_script(fh,indent);

	return true;
}


