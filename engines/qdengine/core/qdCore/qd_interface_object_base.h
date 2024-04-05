
#ifndef __QD_INTERFACE_OBJECT_BASE_H__
#define __QD_INTERFACE_OBJECT_BASE_H__

#include "qd_named_object_base.h"

//! Базовый класс для всех интерфейсных объектов.
class qdInterfaceObjectBase : public qdNamedObjectBase
{
public:
	qdInterfaceObjectBase();
	qdInterfaceObjectBase(const qdInterfaceObjectBase& obj);
	~qdInterfaceObjectBase();

	//! Устанавливает владельца объекта.
	void set_owner(qdInterfaceObjectBase* p){ owner_ = p; }
	//! Возвращает указатель на владельца объекта.
	qdInterfaceObjectBase* owner() const { return owner_; }

protected:

	qdInterfaceObjectBase& operator = (const qdInterfaceObjectBase& obj);

private:

	//! Владелец объекта.
	mutable qdInterfaceObjectBase* owner_;
};

#endif /* __QD_INTERFACE_OBJECT_BASE_H__ */
