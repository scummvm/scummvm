/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/qdcore/qd_scale_info.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdScaleInfo::qdScaleInfo(const qdScaleInfo &sc) : qdNamedObject(sc),
	scale_(sc.scale_) {
}

void qdScaleInfo::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_SCALE:
			xml::tag_buffer(*it) > scale_;
			break;
		}
	}
}

bool qdScaleInfo::save_script(XStream &fh, int indent) const {
	for (int i = 0; i < indent; i ++) fh < "\t";

	fh < "<object_scale name=";
	if (name()) fh < "\"" < qdscr_XML_string(name()) < "\"";
	else fh < "\" \"";

	fh < " scale=\"" <= scale() < "\"/>\r\n";

	return true;
}
} // namespace QDEngine
