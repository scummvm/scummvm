#ifndef __QD_RESOURCE_FILE_H__
#define __QD_RESOURCE_FILE_H__


namespace QDEngine {

bool qd_open_resource(const char *file_name, class XStream &fh, bool readonly = true, bool err_message = true);

} // namespace QDEngine

#endif /* __QD_RESOURCE_FILE_H__ */
