#ifndef __QD_GRID_ZONE_STATE_H__
#define __QD_GRID_ZONE_STATE_H__

#include "xml_fwd.h"

#include "qd_conditional_object.h"

class XStream;

//! Состояние зоны на сетке сцены.
class qdGridZoneState : public qdConditionalObject
{
public:
	explicit qdGridZoneState(bool st);
	qdGridZoneState(const qdGridZoneState& st);
	~qdGridZoneState();

	qdGridZoneState& operator = (const qdGridZoneState& st);

	int named_object_type() const { return QD_NAMED_OBJECT_GRID_ZONE_STATE; }

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag* p);
	//! Запись данных в скрипт.
	bool save_script(class XStream& fh,int indent = 0) const;

	//! Включить или выключить зону.
	bool state() const { return state_; }

	qdConditionalObject::trigger_start_mode trigger_start();

	static const char* const ZONE_STATE_ON_NAME;
	static const char* const ZONE_STATE_OFF_NAME;

private:

	//! Включить или выключить зону.
	bool state_;
};

#endif /* __QD_GRID_ZONE_STATE_H__ */
