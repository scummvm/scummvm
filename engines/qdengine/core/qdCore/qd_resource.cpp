/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#ifdef __QD_DEBUG_ENABLE__
#include <stdio.h>
#endif

#include "qd_resource.h"
#include "qd_named_object.h"
#include "qdscr_parser.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdResource::qdResource() : is_loaded_(false)
{
}

qdResource::qdResource(const qdResource& res) : is_loaded_(res.is_loaded_)
{
}

qdResource& qdResource::operator = (const qdResource& res)
{
	if(this == &res) return *this;

	is_loaded_ = res.is_loaded_;

	return *this;
}

qdResource::~qdResource()
{
}

qdResource::file_format_t qdResource::file_format(const char* file_name)
{
	char ext[_MAX_EXT];
	_splitpath(file_name,NULL,NULL,NULL,ext);

	if(!stricmp(ext,".qda")) return RES_ANIMATION;
	if(!stricmp(ext,".tga")) return RES_SPRITE;
	if(!stricmp(ext,".wav")) return RES_SOUND;

	return RES_UNKNOWN;
}

#ifdef __QD_DEBUG_ENABLE__
qdResourceInfo::qdResourceInfo(const qdResource* res,const qdNamedObject* owner) : resource_(res), data_size_(0), resource_owner_(owner)
{
	if(resource_)
		data_size_ = resource_ -> resource_data_size();
}

qdResourceInfo::~qdResourceInfo()
{
}

qdResource::file_format_t qdResourceInfo::file_format() const
{
	if(resource_)
		return qdResource::file_format(resource_ -> resource_file());

	return qdResource::RES_UNKNOWN;
}

bool qdResourceInfo::write(XStream& fh,int line_class_id) const
{
	if(!resource_) return false;

	fh < "<tr";

	if(line_class_id != -1){
		if(line_class_id) fh < " class=\"line1\"";
		else fh < " class=\"line0\"";
	}

	static XBuffer name;
	name.init();

	if(resource_owner_)
		name < resource_owner_ -> name();
	else
		name < "???";

	fh < "><td nowrap class=\"name\">" < qdscr_XML_string(name.c_str());
	fh < "</td><td nowrap>" < qdscr_XML_string(resource_ -> resource_file()) < "</td><td class=\"to_r\">";

	static char buf[1024];
	float sz = float(data_size_) / (1024.0f * 1024.0f);
	sprintf(buf,"%.2f",sz);
	fh < buf;

	fh < "</td></tr>\r\n";

	return true;
}
#endif
