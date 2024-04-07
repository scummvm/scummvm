#ifndef __QD_SCALE_INFO_H__
#define __QD_SCALE_INFO_H__

#include "xml_fwd.h"
#include "qd_named_object.h"

class qdScaleInfo : public qdNamedObject
{
public:
	qdScaleInfo() : scale_(1.0f) { }
	qdScaleInfo(const qdScaleInfo& sc);
	~qdScaleInfo(){ }

	int named_object_type() const { return QD_NAMED_OBJECT_SCALE_INFO; }

	float scale() const { return scale_; }
	void set_scale(float sc){ scale_ = sc; }

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

private:

	float scale_;
};

typedef std::list<qdScaleInfo*> qdScaleInfoList;

#endif /* __QD_SCALE_INFO_H__ */
