/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_grid_zone.h"
#include "qd_grid_zone_state.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const char* const qdGridZoneState::ZONE_STATE_ON_NAME = "Вкл";
const char* const qdGridZoneState::ZONE_STATE_OFF_NAME = "Выкл";

qdGridZoneState::qdGridZoneState(bool st) : state_(st)
{
	if(st)
		set_name(ZONE_STATE_ON_NAME);
	else
		set_name(ZONE_STATE_OFF_NAME);
}

qdGridZoneState::qdGridZoneState(const qdGridZoneState& st) : qdConditionalObject(st),
	state_(st.state_)
{
}

qdGridZoneState::~qdGridZoneState()
{
}

qdGridZoneState& qdGridZoneState::operator = (const qdGridZoneState& st)
{
	if(this == &st) return *this;

	state_ = st.state_;

	return *this;
}

bool qdGridZoneState::load_script(const xml::tag* p)
{
	return load_conditions_script(p);
}

bool qdGridZoneState::save_script(class XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<grid_zone_state";

	if(state_) fh < " state=\"1\"";
	else fh < " state=\"0\"";

	if(has_conditions()){
		fh < ">\r\n";

		save_conditions_script(fh,indent);

		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "</grid_zone_state>\r\n";
	}
	else
		fh < "/>\r\n";

	return true;
}

qdConditionalObject::trigger_start_mode qdGridZoneState::trigger_start()
{
	if(!owner()) return qdConditionalObject::TRIGGER_START_FAILED;

	static_cast<qdGridZone*>(owner()) -> set_state(state());
	
	return qdConditionalObject::TRIGGER_START_ACTIVATE;
}