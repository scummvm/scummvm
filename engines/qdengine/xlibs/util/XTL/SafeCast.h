#ifndef __SAFE_CAST_H__
#define __SAFE_CAST_H__

#ifndef _FINAL_VERSION_

template <class DestinationType, class SourceType> 
inline DestinationType safe_cast(SourceType* source) { DestinationType p = dynamic_cast<DestinationType>(source); xassert((p || !source) && "Incorrect cast"); return p; }

template <class DestinationType, class SourceType> 
inline DestinationType safe_cast_ref(SourceType& source) { return dynamic_cast<DestinationType>(source); }

#else

#define safe_cast static_cast
#define safe_cast_ref static_cast

#endif

#endif //__SAFE_CAST_H__
