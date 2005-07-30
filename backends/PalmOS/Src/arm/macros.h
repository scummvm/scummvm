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

	#define PNO_DATA()			&pno
	#define ARM_CONTINUE()		} else
	#define ARM_END()			return; \
							}
	#define ARM_END_RET(type)		return (type)dataARM.armRetVal; \
								}

	#define ARM_CALL(rsc, data)				ARM(rsc).alignedHeader->userDataP = (UInt32)data; \
											PceNativeCall(ARM(rsc).pnoDesc.entry, ARM(rsc).alignedHeader);

	#define ARM_CALL_RET(rsc, data)			ARM(rsc).alignedHeader->userDataP = (UInt32)data; \
											dataARM.armRetVal = PceNativeCall(ARM(rsc).pnoDesc.entry, ARM(rsc).alignedHeader);

	#define ARM_CALL_VALUE(rsc, data, var)			ARM(rsc).alignedHeader->userDataP = (UInt32)data; \
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
	#define ARM_END_RET(type)

	#define ARM_CALL(rsc, data)
	#define ARM_CALL_RET(rsc, data, var)
	#define ARM_CALL_VALUE(rsc, data, var)

#endif
	// New data access
	#define	GET_DATA(TYPE)		TYPE *dataARM	= (TYPE *)userData68KP;
	#define	GET_PTR(TYPE, var)	TYPE *var		= (TYPE *)ReadUnaligned32(&dataARM->var);
	#define GET_32(TYPE, var)	TYPE var		= ReadUnaligned32(&dataARM->var);
	#define	GET_16(TYPE, var)	TYPE var		= ByteSwap16(dataARM->var);
	#define	GET_8(TYPE, var)	TYPE var		= dataARM->var;

	#define	GET_PTRV(TYPE, member, var)	\
								TYPE *var		= (TYPE *)ReadUnaligned32(&dataARM->member);

	#define GET_XPTR(var, dst, src, TYPE) \
								dst.var = (TYPE *)ReadUnaligned32(&src->var);

	#define GET_X32(var, dst, src) \
								dst.var = ReadUnaligned32(&src->var);

	#define GET_X16(var, dst, src) \
								dst.var = ByteSwap16(src->var);

	#define GET_X8(var, dst, src) \
								dst.var = src->var;




	#define  _MEMBER(base, member)		((byte *)userData68KP + OffsetOf(base, member))
	#	define RETVAL(member)				WriteUnaligned32(_MEMBER(MAIN_TYPE, member), member)
#endif
