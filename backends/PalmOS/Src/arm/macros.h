#ifndef __MACROS_H__
#define __MACROS_H__ 

#ifndef DISABLE_ARM
	// macros for ARM calls
	#define ARM_START(TYPE) \
			if (OPTIONS_TST(kOptDeviceARM)) { \
				TYPE dataARM;

	#define ARM_CHECK_EXEC(test)	if (test) {
	#define ARM_CHECK_END()			}

	#define ARM_ADDP(member)		dataARM.member = &member;
	#define ARM_ADDM(member)		dataARM.member = member;
	#define ARM_ADDV(member, var)	dataARM.member = var;
	#define ARM_INIT(id)			PnoType pno = { id, &dataARM };
	
	#define ARM_GETM(member)		member = dataARM.member;
	#define ARM_GETV(member, var)	var = dataARM.member;

	#define PNO_DATA()		&pno
	#define ARM_CONTINUE()	} else
	#define ARM_END()		return; \
						}

	#define ARM_CALL(rsc, data)				ARM(rsc).alignedHeader->userDataP = (UInt32)data; \
											PceNativeCall(ARM(rsc).pnoDesc.entry, ARM(rsc).alignedHeader);

	#define ARM_CALL_RETURN(rsc, data, var)		ARM(rsc).alignedHeader->userDataP = (UInt32)data; \
												var = PceNativeCall(ARM(rsc).pnoDesc.entry, ARM(rsc).alignedHeader);

#else
	// no ARM = empty definition
	#define ARM_START(TYPE)

	#define ARM_CHECK_EXEC(test)
	#define ARM_CHECK_END()

	#define ARM_ADDP(member)
	#define ARM_ADDM(member)
	#define ARM_ADDV(member, var)
	#define ARM_INIT(id)

	#define ARM_DATA()
	#define ARM_CONTINUE()
	#define ARM_END()

	#define ARM_CALL(data)
	#define ARM_CALL_RETURN(data, var)

#endif

	// Data access
	#define  _GETPTR(ptr, base, member, type)	(type)ReadUnaligned32( (byte *)ptr + OffsetOf(base, member) )
	#define  _SETPTR(base, member, type, var)	type var = _GETPTR(userData68KP, base, member, type);

	#define  _GET32	 _GETPTR
	#define  _SET32	 _SETPTR

	#define	 _GET16(ptr, base, member, type)	(type)ByteSwap16( ((UInt16 *)((byte *)ptr + OffsetOf(base, member)))[0] )
	#define  _SET16(base, member, type, var)	type var = _GET16(userData68KP, base, member, type);

	#define  _GET8(ptr, base, member, type)		(type)((byte *)ptr + OffsetOf(base, member))[0]
	#define  _SET8(base, member, type, var)		type var = _GET8(userData68KP, base, member, type);

	#define  _MEMBER(base, member)		((byte *)userData68KP + OffsetOf(base, member))

	// convenient macros to ease data access
	#ifdef MAIN_TYPE 
	#	define SETPTRV(type, member, var)	_SETPTR(MAIN_TYPE, member, type, var)
	#	define SETPTR(type, member)			_SETPTR(MAIN_TYPE, member, type, member)
	#	define SET32(type, member)			_SET32 (MAIN_TYPE, member, type, member)
	#	define SET16(type, member)			_SET16 (MAIN_TYPE, member, type, member)
	#	define SET8(type, member)			_SET8  (MAIN_TYPE, member, type, member)

	#	define RETVAL(member)				WriteUnaligned32(_MEMBER(MAIN_TYPE, member), member)
	#endif

#endif
