#ifndef __QD_GAME_END_H__
#define __QD_GAME_END_H__

#include "xml_fwd.h"
#include "qd_conditional_object.h"

//! Концовка игры.
class qdGameEnd : public qdConditionalObject
{
public:
	qdGameEnd();
	qdGameEnd(const qdGameEnd& end);

	qdGameEnd& operator = (const qdGameEnd& end);

	~qdGameEnd();

	const char* interface_screen() const { return interface_screen_.c_str(); }
	bool has_interface_screen() const { return !interface_screen_.empty(); }
	void set_interface_screen(const char* name){ if(name) interface_screen_ = name;	else interface_screen_.clear(); }

	qdConditionalObject::trigger_start_mode trigger_start();

	int named_object_type() const { return QD_NAMED_OBJECT_GAME_END; };

	bool load_script(const xml::tag* p);
	bool save_script(class XStream& fh,int indent = 0) const;
	
private:

	//! Имя интерфейсного экрана, на который выходить из игры.
	/**
	Если строка пустая - будет выход в main menu.
	*/
	std::string interface_screen_;
};

#endif // __QD_GAME_END_H__

