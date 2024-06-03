#include "qdengine/xlibs/util/serialization/Serialization.h"
#include "qdengine/xlibs/util/serialization/Handle.h"


template<class Type, class BaseType>
bool ShareHandle<Type, BaseType>::serialize(Archive &ar, const char *name, const char *nameAlt) {
	if (ar.isInput() && ptr_) {
		ptr_->decrRef();
		xassert("БАГ: Возможно удаление с созданием висячей ссылки (после Ignore упадет непредсказуемо)!" && ptr_->numRef() == 0);
	}

	bool log;
	if (!ar.inPlace()) {
		Type *ptr = get();
		log = ar.serializePolymorphic(ptr, name, nameAlt);
		ptr_ = ptr;
	} else
		log = ar.serializePolymorphic(reinterpret_cast<Type *&>(ptr_), name, nameAlt);

	if (ar.isInput() && ptr_)
		ptr_->addRef();
	return log;
}
