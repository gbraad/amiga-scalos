/*
 * defs.h
 *
 * $Date: 2007-04-26 12:02:25 +0200 (Do, 26 Apr 2007) $
 * $Revision: 2429 $
 *
 * Some global compiler- and platform-dependent definitions
 *
 */

#ifndef  DEFS_H
#define  DEFS_H

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif


#ifdef __VBCC__
	#define SEGLISTPTR APTR
#else /* !VBCC */
	#include <dos/dos.h>
	#define SEGLISTPTR BPTR
#endif /* VBCC */


#define STR(x)  STR2(x)
#define STR2(x) #x


/* compiler specific stuff */
#ifdef __VBCC__

	#define REG(x, arg) __reg(#x) arg
	#define ASM
	#define SAVEDS(x)   __saveds x
	#define	ALIGNED		__aligned

	#define	LIB_REG(x, arg)	REG(x, arg)
	#define	LIB_ASM		ASM
	#define	LIB_INTERRUPT	INTERRUPT
	#define	LIB_SAVEDS(x)	SAVEDS(x)

	#define	COMPILER_STRING	" (VBCC)";
	#define	T_UTILITYBASE	struct UtilityBase *
	#define	T_INPUTBASE	struct Device *
	#define	WBENCHMSG	WBenchMsg
	#define	T_LOCALEBASE	struct LocaleBase *
	#define T_REXXSYSBASE   struct RxsLib *
	#define T_TIMERBASE	struct Device *
	#define T_CONSOLEDEVICE	struct Device *
	#define T_INPUTDEVICE	struct Device *

#elif defined(__STORM__)

	#define REG(x, arg) register __##x arg
	#define ASM
	#define SAVEDS(x)   x __saveds
	#define	INTERRUPT
	#define	INLINE
	#define	ALIGNED		__aligned

	#define	LIB_REG(x, arg)	REG(x, arg)
	#define	LIB_ASM		ASM
	#define	LIB_INTERRUPT	INTERRUPT
	#define	LIB_SAVEDS(x)	SAVEDS(x)

	#define	T_UTILITYBASE	struct Library *
	#define	T_INPUTBASE	struct Device *
	#define	WBENCHMSG	WBenchMsg
	#define	T_LOCALEBASE	struct LocaleBase *
	#define T_REXXSYSBASE   struct RxsLib *
	#define T_TIMERBASE	struct Device *
	#define T_CONSOLEDEVICE	struct Device *
	#define T_INPUTDEVICE	struct Device *

	#define	COMPILER_STRING	" (VBCC)";

#elif defined(__GNUC__)

	#define ASM
//	#define SAVEDS(x)   x __saveds
#ifdef __amigaos4__
	#include <amiga_compiler.h>
	#undef SAVEDS
#else
	#define REG(x, arg) arg __asm(#x)
	#define	INLINE		__inline
#endif
	#define SAVEDS(x)   	x
	#define	INTERRUPT

#if (__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ > 3)
	#define __FUNC__ 	__func__
#else
	#define __FUNC__    __FUNCTION__
#endif

#ifndef __amigaos4__
	#define	WBENCHMSG	_WBenchMsg
#else
	#define WBENCHMSG	WBenchMsg
#endif

#if defined(__MORPHOS__) || defined(__amigaos4__)
	#define	LIB_REG(x, arg)	arg
	#define	LIB_ASM
	#define	LIB_INTERRUPT
	#define	LIB_SAVEDS(x)	x

	#define	ALIGNED

	#define	T_LOCALEBASE	struct Library *
#ifndef __USE_BASETYPE__
	#define	T_UTILITYBASE	struct Library *
#else
	#define T_UTILITYBASE	struct UtilityBase *
#endif
	#define	T_INPUTBASE	struct Library *
	#define T_REXXSYSBASE   struct Library *
	#define T_TIMERBASE	struct Library *
	#define T_CONSOLEDEVICE	struct Library *
	#define T_INPUTDEVICE	struct Library *

#ifdef __amigaos4__
	#define GCC_PLATFORM	"AmigaOS4/PPC"
#else
	#define GCC_PLATFORM	"MorphOS/PPC"
#endif

#else /* __MORPHOS__ || __amigaos4__ */
	#define	LIB_REG(x, arg)	REG(x, arg)
	#define	LIB_ASM		ASM
	#define	LIB_INTERRUPT	INTERRUPT
	#define	LIB_SAVEDS(x)	SAVEDS(x)

	#define	ALIGNED		__aligned

	#define	T_LOCALEBASE	struct LocaleBase *
	#define	T_UTILITYBASE	struct UtilityBase *
	#define	T_INPUTBASE	struct Device *
	#define T_REXXSYSBASE   struct RxsLib *
	#define T_TIMERBASE	struct Device *
	#define T_CONSOLEDEVICE	struct Device *
	#define T_INPUTDEVICE	struct Device *

	#define	GCC_PLATFORM	"68K"
#endif /* __MORPHOS__*/

	#if defined(__GNUC_PATCHLEVEL__)
		#define	COMPILER_STRING	" (GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__) " " GCC_PLATFORM ")"
	#else
		#define	COMPILER_STRING	" (GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) ".x " GCC_PLATFORM ")"
	#endif

	#if defined(_INLINE_GRAPHICS_H)
	// Remove definition made by "inline/graphics.h" becauses it causes errors (too many parameters...)
		#undef	BltBitMap
	#endif /* _INLINE_GRAPHICS_H */

#elif defined(__SASC)

	#define REG(x, arg) register __##x arg
	#define ASM		__asm
	#define SAVEDS(x)   	x __saveds
	#define	INTERRUPT	__interrupt
	#define	INLINE		__inline
	#define	ALIGNED		__aligned

	#define	LIB_REG(x, arg)	REG(x, arg)
	#define	LIB_ASM		ASM
	#define	LIB_INTERRUPT	INTERRUPT
	#define	LIB_SAVEDS(x)	SAVEDS(x)

	#define	T_UTILITYBASE	struct Library *
	#define	T_INPUTBASE	struct Device *
	#define	WBENCHMSG	WBenchMsg
	#define	T_LOCALEBASE	struct LocaleBase *
	#define T_REXXSYSBASE   struct RxsLib *
	#define T_TIMERBASE	struct Device *
	#define T_CONSOLEDEVICE	struct Device *
	#define T_INPUTDEVICE	struct Device *

	#define	COMPILER_STRING	" (SAS/C " STR(__VERSION__) "." STR(__REVISION__) ")"

#else

	/* other compilers would get stacked up in here */
	#error "You need VBCC, GCC SAS/C or StormC to compile this"

	#define	COMPILER_STRING	" (unknown)"
#endif

/* Cut down on the load by not including old intuition defines */
#define INTUI_V36_NAMES_ONLY


#ifdef __MORPHOS__
	#define	DISPATCHER_PROTO(Name) \
		static ULONG Name##_Dispatcher(void)

	#define DISPATCHER(Name) \
	static ULONG Name##_Dispatcher(void); \
	struct EmulLibEntry GATE ##Name##_Dispatcher = { TRAP_LIB, 0, (void (*)(void)) Name##_Dispatcher }; \
	static ULONG Name##_Dispatcher(void) { struct IClass *cl=(struct IClass*)REG_A0; Msg msg=(Msg)REG_A1; Object *obj=(Object*)REG_A2;
	#define DISPATCHER_REF(Name) &GATE##Name##_Dispatcher
	#define DISPATCHER_END }

#else
	#define	DISPATCHER_PROTO(Name) \
		static SAVEDS(ULONG) ASM Name##Dispatcher(REG(a0, struct IClass *cl), REG(a2, Object *obj), REG(a1, Msg msg))

	#define DISPATCHER(Name) static SAVEDS(ULONG) ASM Name##Dispatcher(REG(a0, struct IClass *cl), REG(a2, Object *obj), REG(a1, Msg msg))
	#define DISPATCHER_REF(Name) Name##Dispatcher
	#define DISPATCHER_END

#endif

#ifdef __amigaos4__
	#define SETHOOKFUNC(hook, func) \
		(hook).h_Entry = (HOOKFUNC)func
	#define HOOKFUNC_DEF(func) \
		(HOOKFUNC)func, NULL
#else
	#define SETHOOKFUNC(hook, func) \
		(hook).h_Entry = (HOOKFUNC)HookEntry; \
		(hook).h_SubEntry = (HOOKFUNC)func
	#define HOOKFUNC_DEF(func) \
		(HOOKFUNC)HookEntry, (HOOKFUNC)func
#endif

// ==============================================================
//  Library function stuff
// ==============================================================

#ifdef __MORPHOS__

	#define PATCHFUNC(x)		struct EmulLibEntry x##EmulEntry = \
		{ TRAP_LIB, 0, (APTR) x }
	#define STATIC_PATCHFUNC(x)	static PATCHFUNC(x);
	#define __EMULENTRY(x) 		x##EmulEntry
	#define PATCH_NEWFUNC(x)	(APTR) &__EMULENTRY(x)

	//=======================================================

	#define	M68KFUNC_PROTO(funcname, libbase, returntype)	\
		returntype funcname(void)


	#define	M68KFUNC_P1_PROTO(returntype, funcname, 	\
			register1, type1, name1) 		\
		returntype funcname(void)

	#define	M68KFUNC_P2_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype funcname(void)

	#define	M68KFUNC_P3_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype funcname(void)

	#define	M68KFUNC_P4_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		returntype funcname(void)

	#define	M68KFUNC_P5_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		returntype funcname(void)

	#define	M68KFUNC_P6_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		returntype funcname(void)


	#define	M68KFUNC_P7_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		returntype funcname(void)

	#define	M68KFUNC_P8_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		returntype funcname(void)

	#define	M68KFUNC_P9_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		returntype funcname(void)

	//===== prototypes for data items ======================

	#define	M68KFUNC_P1_DPROTO(returntype, funcname, 	 \
			register1, type1, name1) 		\
		returntype funcname(void)

	#define	M68KFUNC_P2_DPROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype funcname(void)

	#define	M68KFUNC_P3_DPROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype funcname(void)

	#define	M68KFUNC_P4_DPROTO(returntype, funcname,	 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		returntype funcname(void)

	#define	M68KFUNC_P5_DPROTO(returntype, funcname,	 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		returntype funcname(void)

	#define	M68KFUNC_P6_DPROTO(returntype, funcname,	 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		returntype funcname(void)


	#define	M68KFUNC_P7_DPROTO(returntype, funcname,	 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		returntype funcname(void)

	#define	M68KFUNC_P8_DPROTO(returntype, funcname,	 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		returntype funcname(void)

	#define	M68KFUNC_P9_DPROTO(returntype, funcname,	 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		returntype funcname(void)

	//======================================================

	#define	M68KFUNC(name, libbase, returntype) 	 \
		M68KFUNC_PROTO(name, libbase, returntype) 	 \
			{ 					\
			struct Library *libbase = (struct Library *) REG_A6;

	#define	M68KFUNC_P1(returntype, funcname, 		 \
			register1, type1, name1) 		\
		M68KFUNC_P1_PROTO(returntype, funcname, 	 \
			register1, type1, name1) 		\
			{					\
			type1 name1 = (type1) REG_##register1;

	#define	M68KFUNC_P2(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		M68KFUNC_P2_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;

	#define	M68KFUNC_P3(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		M68KFUNC_P3_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;

	#define	M68KFUNC_P4(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4) 		\
		M68KFUNC_P4_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3, 		\
			register4, type4, name4) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;	\
			type4 name4 = (type4) REG_##register4;

	#define	M68KFUNC_P5(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5) 		\
		M68KFUNC_P5_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3, 		\
			register4, type4, name4, 		\
			register5, type5, name5) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;	\
			type4 name4 = (type4) REG_##register4;	\
			type5 name5 = (type5) REG_##register5;

	#define	M68KFUNC_P6(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6) 		\
		M68KFUNC_P6_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3, 		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;	\
			type4 name4 = (type4) REG_##register4;	\
			type5 name5 = (type5) REG_##register5;	\
			type6 name6 = (type6) REG_##register6;

	#define	M68KFUNC_P7(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7) 		\
		M68KFUNC_P7_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3, 		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;	\
			type4 name4 = (type4) REG_##register4;	\
			type5 name5 = (type5) REG_##register5;	\
			type6 name6 = (type6) REG_##register6;	\
			type7 name7 = (type7) REG_##register7;

	#define	M68KFUNC_P8(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8) 		\
		M68KFUNC_P8_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3, 		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;	\
			type4 name4 = (type4) REG_##register4;	\
			type5 name5 = (type5) REG_##register5;	\
			type6 name6 = (type6) REG_##register6;	\
			type7 name7 = (type7) REG_##register7;	\
			type8 name8 = (type8) REG_##register8;

	#define	M68KFUNC_P9(returntype, funcname, 		 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8, 		\
			register9, type9, name9) 		\
		M68KFUNC_P9_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3, 		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8, 		\
			register9, type9, name9) 		\
			{					\
			type1 name1 = (type1) REG_##register1;	\
			type2 name2 = (type2) REG_##register2;	\
			type3 name3 = (type3) REG_##register3;	\
			type4 name4 = (type4) REG_##register4;	\
			type5 name5 = (type5) REG_##register5;	\
			type6 name6 = (type6) REG_##register6;	\
			type7 name7 = (type7) REG_##register7;	\
			type8 name8 = (type8) REG_##register8;  \
			type9 name9 = (type9) REG_##register9;

	//======================================================

	#define	M68KFUNC_END \
			}

	//======================================================

	#define	CALLM68KFUNC(funcname, libbase) 	     	\
		funcname()

	#define	CALLM68KFUNC_P1(funcname, register1, arg1)	 \
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P2(funcname,			 \
			register1, arg1,			\
			register2, arg2)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P3(funcname,			 \
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P4(funcname,			 \
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			REG_##register4 = (ULONG) arg4,         \
			(APTR) funcname));

	#define	CALLM68KFUNC_P5(funcname,			 \
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			REG_##register4 = (ULONG) arg4,		\
			REG_##register5 = (ULONG) arg5,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P6(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			REG_##register4 = (ULONG) arg4,		\
			REG_##register5 = (ULONG) arg5,		\
			REG_##register6 = (ULONG) arg6,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P7(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			REG_##register4 = (ULONG) arg4,		\
			REG_##register5 = (ULONG) arg5,		\
			REG_##register6 = (ULONG) arg6,		\
			REG_##register7 = (ULONG) arg7,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P8(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7,			\
			register8, arg8)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			REG_##register4 = (ULONG) arg4,		\
			REG_##register5 = (ULONG) arg5,		\
			REG_##register6 = (ULONG) arg6,		\
			REG_##register7 = (ULONG) arg7,		\
			REG_##register8 = (ULONG) arg8,		\
			(APTR) funcname))

	#define	CALLM68KFUNC_P9(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7,			\
			register8, arg8,			\
			register9, arg9)			\
		(*MyEmulHandle->EmulCallDirect68k)((		\
			REG_##register1 = (ULONG) arg1,		\
			REG_##register2 = (ULONG) arg2,		\
			REG_##register3 = (ULONG) arg3,		\
			REG_##register4 = (ULONG) arg4,		\
			REG_##register5 = (ULONG) arg5,		\
			REG_##register6 = (ULONG) arg6,		\
			REG_##register7 = (ULONG) arg7,		\
			REG_##register8 = (ULONG) arg8,		\
			REG_##register9 = (ULONG) arg9,		\
			(APTR) funcname))

	//======================================================

	#define LIBFUNC_PROTO M68KFUNC_PROTO
	#define LIBFUNC_P1_PROTO M68KFUNC_P1_PROTO
	#define LIBFUNC_P2_PROTO M68KFUNC_P2_PROTO
	#define LIBFUNC_P3_PROTO M68KFUNC_P3_PROTO
	#define LIBFUNC_P4_PROTO M68KFUNC_P4_PROTO
	#define LIBFUNC_P5_PROTO M68KFUNC_P5_PROTO
	#define LIBFUNC_P6_PROTO M68KFUNC_P6_PROTO
	#define LIBFUNC_P7_PROTO M68KFUNC_P7_PROTO
	#define LIBFUNC_P8_PROTO M68KFUNC_P8_PROTO
	#define LIBFUNC_P9_PROTO M68KFUNC_P9_PROTO

	#define LIBFUNC_P1_DPROTO M68KFUNC_P1_DPROTO
	#define LIBFUNC_P2_DPROTO M68KFUNC_P2_DPROTO
	#define LIBFUNC_P3_DPROTO M68KFUNC_P3_DPROTO
	#define LIBFUNC_P4_DPROTO M68KFUNC_P4_DPROTO
	#define LIBFUNC_P5_DPROTO M68KFUNC_P5_DPROTO
	#define LIBFUNC_P6_DPROTO M68KFUNC_P6_DPROTO
	#define LIBFUNC_P7_DPROTO M68KFUNC_P7_DPROTO
	#define LIBFUNC_P8_DPROTO M68KFUNC_P8_DPROTO
	#define LIBFUNC_P9_DPROTO M68KFUNC_P9_DPROTO

	#define LIBFUNC M68KFUNC
	#define LIBFUNC_P1 M68KFUNC_P1
	#define LIBFUNC_P2 M68KFUNC_P2
	#define LIBFUNC_P3 M68KFUNC_P3
	#define LIBFUNC_P4 M68KFUNC_P4
	#define LIBFUNC_P5 M68KFUNC_P5
	#define LIBFUNC_P6 M68KFUNC_P6
	#define LIBFUNC_P7 M68KFUNC_P7
	#define LIBFUNC_P8 M68KFUNC_P8
	#define LIBFUNC_P9 M68KFUNC_P9

	#define LIBFUNC_END M68KFUNC_END

	#define CALLLIBFUNC CALLM68KFUNC
	#define CALLLIBFUNC_P1 CALLM68KFUNC_P1
	#define CALLLIBFUNC_P2 CALLM68KFUNC_P2
	#define CALLLIBFUNC_P3 CALLM68KFUNC_P3
	#define CALLLIBFUNC_P4 CALLM68KFUNC_P4
	#define CALLLIBFUNC_P5 CALLM68KFUNC_P5
	#define CALLLIBFUNC_P6 CALLM68KFUNC_P6
	#define CALLLIBFUNC_P7 CALLM68KFUNC_P7
	#define CALLLIBFUNC_P8 CALLM68KFUNC_P8
	#define CALLLIBFUNC_P9 CALLM68KFUNC_P9

	//======================================================

#elif defined(__amigaos4__)

	//======================================================

	#define	PATCHFUNC(x)
	#define	STATIC_PATCHFUNC(x)
	#define	PATCH_NEWFUNC(x)	(APTR) (x)

	//=======================================================

	#define	M68KFUNC_PROTO(name, libbase, returntype) 	 \
		returntype name##(void)


	#define	M68KFUNC_P1_PROTO(returntype, funcname, \
			register1, type1, name1) \
		returntype funcname(type1 name1)

	#define	M68KFUNC_P2_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype funcname( \
			type1 name1, \
			type2 name2)

	#define	M68KFUNC_P3_PROTO(returntype, funcname, 	 \
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3)

	#define	M68KFUNC_P4_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3, \
			type4 name4)

	#define	M68KFUNC_P5_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3, \
			type4 name4, \
			type5 name5)

	#define	M68KFUNC_P6_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3, \
			type4 name4, \
			type5 name5, \
			type6 name6)


	#define	M68KFUNC_P7_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3, \
			type4 name4, \
			type5 name5, \
			type6 name6, \
			type7 name7)

	#define	M68KFUNC_P8_PROTO(returntype, funcname,		 \
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3, \
			type4 name4, \
			type5 name5, \
			type6 name6, \
			type7 name7, \
			type8 name8)

	#define	M68KFUNC_P9_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		returntype funcname( \
			type1 name1, \
			type2 name2, \
			type3 name3, \
			type4 name4, \
			type5 name5, \
			type6 name6, \
			type7 name7, \
			type8 name8, \
			type9 name9)

	//===== prototypes for data items ======================

	#define	M68KFUNC_P1_DPROTO  M68KFUNC_P1_PROTO
	#define	M68KFUNC_P2_DPROTO  M68KFUNC_P2_PROTO
	#define	M68KFUNC_P3_DPROTO  M68KFUNC_P3_PROTO
	#define	M68KFUNC_P4_DPROTO  M68KFUNC_P4_PROTO
	#define	M68KFUNC_P5_DPROTO  M68KFUNC_P5_PROTO
	#define	M68KFUNC_P6_DPROTO  M68KFUNC_P6_PROTO
	#define	M68KFUNC_P7_DPROTO  M68KFUNC_P7_PROTO
	#define	M68KFUNC_P8_DPROTO  M68KFUNC_P8_PROTO
	#define	M68KFUNC_P9_DPROTO  M68KFUNC_P9_PROTO

	//======================================================

	#define	M68KFUNC(name, libbase, returntype) 	 	\
		returntype name(void) {

	#define	M68KFUNC_P1(returntype, funcname, 		\
			register1, type1, name1) 		\
		returntype funcname( 				\
			type1 name1) {

	#define	M68KFUNC_P2(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2) {

	#define	M68KFUNC_P3(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3) {

	#define	M68KFUNC_P4(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4) {

	#define	M68KFUNC_P5(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5) {

	#define	M68KFUNC_P6(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6) {

	#define	M68KFUNC_P7(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7) {

	#define	M68KFUNC_P8(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			type8 name8) {

	#define	M68KFUNC_P9(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8, 		\
			register9, type9, name9) 		\
		returntype funcname( 				\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			type8 name8, 				\
			type9 name9) {

	//======================================================

	#define	M68KFUNC_END \
			}

	//======================================================

	#define	CALLM68KFUNC(name, libbase) 		 	\
		name##()

	#define	CALLM68KFUNC_P1(funcname, register1, arg1)	\
		funcname(arg1)

	#define	CALLM68KFUNC_P2(funcname,			\
			register1, arg1,			\
			register2, arg2)			\
		funcname(arg1, arg2)

	#define	CALLM68KFUNC_P3(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3)			\
		funcname(arg1, arg2, arg3)

	#define	CALLM68KFUNC_P4(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4)			\
		funcname(arg1, arg2, arg3, arg4)

	#define	CALLM68KFUNC_P5(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5)			\
		funcname(arg1, arg2, arg3, arg4, arg5)

	#define	CALLM68KFUNC_P6(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6)

	#define	CALLM68KFUNC_P7(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6, arg7)

	#define	CALLM68KFUNC_P8(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7,			\
			register8, arg8)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

	#define	CALLM68KFUNC_P9(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7,			\
			register8, arg8,			\
			register9, arg9)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

	//======================================================

	#define	LIBFUNC_PROTO(name, libbase, returntype) 	\
		returntype name(struct Interface *self)


	#define	LIBFUNC_P1_PROTO(returntype, funcname, 		\
			register1, type1, name1) 		\
		returntype funcname(struct Interface *self)

	#define	LIBFUNC_P2_PROTO(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1)

	#define	LIBFUNC_P3_PROTO(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2)

	#define	LIBFUNC_P4_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3)

	#define	LIBFUNC_P5_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4)

	#define	LIBFUNC_P6_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5)

	#define	LIBFUNC_P7_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6)

	#define	LIBFUNC_P8_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7)

	#define	LIBFUNC_P9_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			type8 name8)

	//===== prototypes for data items ======================

	#define	LIBFUNC_P1_DPROTO  LIBFUNC_P1_PROTO
	#define	LIBFUNC_P2_DPROTO  LIBFUNC_P2_PROTO
	#define	LIBFUNC_P3_DPROTO  LIBFUNC_P3_PROTO
	#define	LIBFUNC_P4_DPROTO  LIBFUNC_P4_PROTO
	#define	LIBFUNC_P5_DPROTO  LIBFUNC_P5_PROTO
	#define	LIBFUNC_P6_DPROTO  LIBFUNC_P6_PROTO
	#define	LIBFUNC_P7_DPROTO  LIBFUNC_P7_PROTO
	#define	LIBFUNC_P8_DPROTO  LIBFUNC_P8_PROTO
	#define	LIBFUNC_P9_DPROTO  LIBFUNC_P9_PROTO

	//======================================================

	#define	LIBFUNC(name, libbase, returntype) 		\
		returntype name(struct Interface *self) {   	\
			struct Library *libbase = (struct Library *)self->Data.LibBase;

	#define	LIBFUNC_P1(returntype, funcname, 		\
			register1, type1, name1) 		\
		returntype funcname( 				\
			struct Interface *self) { 		\
			type1 name1 = (type1)self->Data.LibBase;

	#define	LIBFUNC_P2(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1) { 				\
			type2 name2 = (type2)self->Data.LibBase;

	#define	LIBFUNC_P3(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2) { 				\
			type3 name3 = (type3)self->Data.LibBase;

	#define	LIBFUNC_P4(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3) { 				\
			type4 name4 = (type4)self->Data.LibBase;

	#define	LIBFUNC_P5(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4) { 				\
			type5 name5 = (type5)self->Data.LibBase;

	#define	LIBFUNC_P6(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5) { 				\
			type6 name6 = (type6)self->Data.LibBase;

	#define	LIBFUNC_P7(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6) { 				\
			type7 name7 = (type7)self->Data.LibBase;

	#define	LIBFUNC_P8(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7) { 				\
			type8 name8 = (type8)self->Data.LibBase;

	#define	LIBFUNC_P9(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8, 		\
			register9, type9, name9) 		\
		returntype funcname( 				\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			type8 name8) { 				\
			type9 name9 = (type9)self->Data.LibBase;

	//======================================================

	#define LIBFUNC_END 					\
		}

	//======================================================

	#define	LIBFUNC_P1VA_PROTO(returntype, funcname, 	\
			register1, type1, name1) 		\
		returntype VARARGS68K funcname(struct Interface *self, ...)

	#define	LIBFUNC_P2VA_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			...)

	#define	LIBFUNC_P3VA_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			...)

	#define	LIBFUNC_P4VA_PROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			...)

	#define	LIBFUNC_P5VA_PROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			...)

	#define	LIBFUNC_P6VA_PROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			...)

	#define	LIBFUNC_P7VA_PROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			...)

	#define	LIBFUNC_P8VA_PROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			...)

	#define	LIBFUNC_P9VA_PROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			type8 name8, 				\
			...)

	//======================================================

	#define	LIBFUNC_P1VA(returntype, funcname, 		\
			register1, type1, name1) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			...) { \
			type1 name1 = (type1)self->Data.LibBase;

	#define	LIBFUNC_P2VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			...) { 					\
			type2 name2 = (type2)self->Data.LibBase;

	#define	LIBFUNC_P3VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			...) { 					\
			type3 name3 = (type3)self->Data.LibBase;

	#define	LIBFUNC_P4VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			...) { 					\
			type4 name4 = (type4)self->Data.LibBase;

	#define	LIBFUNC_P5VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			...) { \
			type5 name5 = (type5)self->Data.LibBase;

	#define	LIBFUNC_P6VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			...) { 					\
			type6 name6 = (type6)self->Data.LibBase;

	#define	LIBFUNC_P7VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			...) { 					\
			type7 name7 = (type7)self->Data.LibBase;

	#define	LIBFUNC_P8VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			...) { 					\
			type8 name8 = (type8)self->Data.LibBase;

	#define	LIBFUNC_P9VA(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3,		\
			register4, type4, name4, 		\
			register5, type5, name5, 		\
			register6, type6, name6, 		\
			register7, type7, name7, 		\
			register8, type8, name8, 		\
			register9, type9, name9) 		\
		returntype VARARGS68K funcname( 		\
			struct Interface *self, 		\
			type1 name1, 				\
			type2 name2, 				\
			type3 name3, 				\
			type4 name4, 				\
			type5 name5, 				\
			type6 name6, 				\
			type7 name7, 				\
			type8 name8, 				\
			...) { 					\
			type9 name9 = (type9) self->Data.LibBase;

	//======================================================

	#define EXTLIBBASE(base) ((struct ExtendedLibrary *) ( (ULONG)base + ((struct Library *)base)->lib_PosSize) )
	#define IFACEBASE(base) ( EXTLIBBASE(base)->MainIFace )

	#define CALLLIBFUNC(name,base) \
		name(IFACEBASE(base))

	#define CALLLIBFUNC_P1(name, \
		register1, arg1) \
		name(IFACEBASE(arg1))

	#define CALLLIBFUNC_P2(name, \
		register1, arg1, \
		register2, arg2) \
		name(IFACEBASE(arg2), arg1)

	#define CALLLIBFUNC_P3(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3) \
		name(IFACEBASE(arg3), arg1, arg2)

	#define CALLLIBFUNC_P4(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3, \
		register4, arg4) \
		name(IFACEBASE(arg4), arg1, arg2, arg3)

	#define CALLLIBFUNC_P5(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3, \
		register4, arg4, \
		register5, arg5) \
		name(IFACEBASE(arg5), arg1, arg2, arg3, arg4)

	#define CALLLIBFUNC_P6(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3, \
		register4, arg4, \
		register5, arg5, \
		register6, arg6) \
		name(IFACEBASE(arg6), arg1, arg2, arg3, arg4, arg5)

	#define CALLLIBFUNC_P7(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3, \
		register4, arg4, \
		register5, arg5, \
		register6, arg6, \
		register7, arg7) \
		name(IFACEBASE(arg7), arg1, arg2, arg3, arg4, arg5, arg6)

	#define CALLLIBFUNC_P8(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3, \
		register4, arg4, \
		register5, arg5, \
		register6, arg6, \
		register7, arg7, \
		register8, arg8) \
		name(IFACEBASE(arg8), arg1, arg2, arg3, arg4, arg5, arg6, arg7)

	#define CALLLIBFUNC_P9(name, \
		register1, arg1, \
		register2, arg2, \
		register3, arg3, \
		register4, arg4, \
		register5, arg5, \
		register6, arg6, \
		register7, arg7, \
		register8, arg8, \
		register9, arg9) \
		name(IFACEBASE(arg9), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

	//======================================================

#else

	//======================================================

	#define	REGPARM_A0	REG(a0
	#define	REGPARM_A1	REG(a1
	#define	REGPARM_A2	REG(a2
	#define	REGPARM_A3	REG(a3
	#define	REGPARM_A4	REG(a4
	#define	REGPARM_A5	REG(a5
	#define	REGPARM_A6	REG(a6

	#define	REGPARM_D0	REG(d0
	#define	REGPARM_D1	REG(d1
	#define	REGPARM_D2	REG(d2
	#define	REGPARM_D3	REG(d3
	#define	REGPARM_D4	REG(d4
	#define	REGPARM_D5	REG(d5
	#define	REGPARM_D6	REG(d6
	#define	REGPARM_D7	REG(d7

	//======================================================

	#define	PATCHFUNC(x)
	#define	STATIC_PATCHFUNC(x)
	#define	PATCH_NEWFUNC(x)	(APTR) (x)

	//======================================================

	#define	M68KFUNC_PROTO(name, libbase, returntype) 	\
		SAVEDS(returntype##) ASM INTERRUPT name##(REG(a6, struct Library *libbase##))

	#define	M68KFUNC_P1_PROTO(returntype, funcname, 	\
			register1, type1, name1) 		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1))

	#define	M68KFUNC_P2_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1), 	\
			REGPARM_##register2, type2 name2))

	#define	M68KFUNC_P3_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1), 	\
			REGPARM_##register2, type2 name2), 	\
			REGPARM_##register3, type3 name3))

	#define	M68KFUNC_P4_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4))

	#define	M68KFUNC_P5_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5))
		
	#define	M68KFUNC_P6_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6))	\

	#define	M68KFUNC_P7_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6),	\
			REGPARM_##register7, type7 name7))

	#define	M68KFUNC_P8_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6),	\
			REGPARM_##register7, type7 name7),	\
			REGPARM_##register8, type8 name8))

	#define	M68KFUNC_P9_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		SAVEDS(returntype) ASM INTERRUPT funcname( 	\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6),	\
			REGPARM_##register7, type7 name7),	\
			REGPARM_##register8, type8 name8),	\
			REGPARM_##register9, type9 name9))

	//===== prototypes for data items (no SAVEDS, no INTERRUPT) ====

	#define	M68KFUNC_P1_DPROTO(returntype, funcname, 	\
			register1, type1, name1) 		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1))

	#define	M68KFUNC_P2_DPROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1), 	\
			REGPARM_##register2, type2 name2))

	#define	M68KFUNC_P3_DPROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1), 	\
			REGPARM_##register2, type2 name2), 	\
			REGPARM_##register3, type3 name3))

	#define	M68KFUNC_P4_DPROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4))

	#define	M68KFUNC_P5_DPROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5))
		
	#define	M68KFUNC_P6_DPROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6))	\

	#define	M68KFUNC_P7_DPROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6),	\
			REGPARM_##register7, type7 name7))

	#define	M68KFUNC_P8_DPROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6),	\
			REGPARM_##register7, type7 name7),	\
			REGPARM_##register8, type8 name8))

	#define	M68KFUNC_P9_DPROTO(returntype, funcname,	\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		returntype ASM funcname( 			\
			REGPARM_##register1, type1 name1),	\
			REGPARM_##register2, type2 name2),	\
			REGPARM_##register3, type3 name3),	\
			REGPARM_##register4, type4 name4),	\
			REGPARM_##register5, type5 name5),	\
			REGPARM_##register6, type6 name6),	\
			REGPARM_##register7, type7 name7),	\
			REGPARM_##register8, type8 name8),	\
			REGPARM_##register9, type9 name9))

	//=======================================================

	#define	M68KFUNC(name, libbase, returntype) 	 	\
		M68KFUNC_PROTO(name, libbase, returntype) 	\
			{

	#define	M68KFUNC_P1(returntype, funcname, 		\
			register1, type1, name1) 		\
		M68KFUNC_P1_PROTO(returntype, funcname, 	\
			register1, type1, name1) 		\
			{

	#define	M68KFUNC_P2(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
		M68KFUNC_P2_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2) 		\
			{

	#define	M68KFUNC_P3(returntype, funcname, 		\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
		M68KFUNC_P3_PROTO(returntype, funcname, 	\
			register1, type1, name1, 		\
			register2, type2, name2, 		\
			register3, type3, name3) 		\
			{

	#define	M68KFUNC_P4(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
		M68KFUNC_P4_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4)		\
			{

	#define	M68KFUNC_P5(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
		M68KFUNC_P5_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5)		\
			{

	#define	M68KFUNC_P6(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
		M68KFUNC_P6_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6)		\
			{


	#define	M68KFUNC_P7(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
		M68KFUNC_P7_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7)		\
			{

	#define	M68KFUNC_P8(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
		M68KFUNC_P8_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8)		\
			{

	#define	M68KFUNC_P9(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
		M68KFUNC_P9_PROTO(returntype, funcname,		\
			register1, type1, name1,		\
			register2, type2, name2,		\
			register3, type3, name3,		\
			register4, type4, name4,		\
			register5, type5, name5,		\
			register6, type6, name6,		\
			register7, type7, name7,		\
			register8, type8, name8,		\
			register9, type9, name9)		\
			{

	//=======================================================

	#define	M68KFUNC_END 					\
			}

	//=======================================================

	#define	CALLM68KFUNC(name, libbase) 			\
		name##(libbase)

	#define	CALLM68KFUNC_P1(funcname, register1, arg1)	\
		funcname(arg1);

	#define	CALLM68KFUNC_P2(funcname,			\
			register1, arg1,			\
			register2, arg2)			\
		funcname(arg1, arg2)

	#define	CALLM68KFUNC_P3(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3)			\
		funcname(arg1, arg2, arg3)

	#define	CALLM68KFUNC_P4(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4)			\
		funcname(arg1, arg2, arg3, arg4)

	#define	CALLM68KFUNC_P5(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5)			\
		funcname(arg1, arg2, arg3, arg4, arg5)

	#define	CALLM68KFUNC_P6(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6)

	#define	CALLM68KFUNC_P7(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6, arg7)

	#define	CALLM68KFUNC_P8(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7,			\
			register8, arg8)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

	#define	CALLM68KFUNC_P9(funcname,			\
			register1, arg1,			\
			register2, arg2,			\
			register3, arg3,			\
			register4, arg4,			\
			register5, arg5,			\
			register6, arg6,			\
			register7, arg7,			\
			register8, arg8,			\
			register9, arg9)			\
		funcname(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

	//======================================================

	#define LIBFUNC_PROTO M68KFUNC_PROTO
	#define LIBFUNC_P1_PROTO M68KFUNC_P1_PROTO
	#define LIBFUNC_P2_PROTO M68KFUNC_P2_PROTO
	#define LIBFUNC_P3_PROTO M68KFUNC_P3_PROTO
	#define LIBFUNC_P4_PROTO M68KFUNC_P4_PROTO
	#define LIBFUNC_P5_PROTO M68KFUNC_P5_PROTO
	#define LIBFUNC_P6_PROTO M68KFUNC_P6_PROTO
	#define LIBFUNC_P7_PROTO M68KFUNC_P7_PROTO
	#define LIBFUNC_P8_PROTO M68KFUNC_P8_PROTO
	#define LIBFUNC_P9_PROTO M68KFUNC_P9_PROTO

	#define LIBFUNC_P1_DPROTO M68KFUNC_P1_DPROTO
	#define LIBFUNC_P2_DPROTO M68KFUNC_P2_DPROTO
	#define LIBFUNC_P3_DPROTO M68KFUNC_P3_DPROTO
	#define LIBFUNC_P4_DPROTO M68KFUNC_P4_DPROTO
	#define LIBFUNC_P5_DPROTO M68KFUNC_P5_DPROTO
	#define LIBFUNC_P6_DPROTO M68KFUNC_P6_DPROTO
	#define LIBFUNC_P7_DPROTO M68KFUNC_P7_DPROTO
	#define LIBFUNC_P8_DPROTO M68KFUNC_P8_DPROTO
	#define LIBFUNC_P9_DPROTO M68KFUNC_P9_DPROTO

	#define LIBFUNC M68KFUNC
	#define LIBFUNC_P1 M68KFUNC_P1
	#define LIBFUNC_P2 M68KFUNC_P2
	#define LIBFUNC_P3 M68KFUNC_P3
	#define LIBFUNC_P4 M68KFUNC_P4
	#define LIBFUNC_P5 M68KFUNC_P5
	#define LIBFUNC_P6 M68KFUNC_P6
	#define LIBFUNC_P7 M68KFUNC_P7
	#define LIBFUNC_P8 M68KFUNC_P8
	#define LIBFUNC_P9 M68KFUNC_P9

	#define LIBFUNC_END M68KFUNC_END

	#define CALLLIBFUNC CALLM68KFUNC
	#define CALLLIBFUNC_P1 CALLM68KFUNC_P1
	#define CALLLIBFUNC_P2 CALLM68KFUNC_P2
	#define CALLLIBFUNC_P3 CALLM68KFUNC_P3
	#define CALLLIBFUNC_P4 CALLM68KFUNC_P4
	#define CALLLIBFUNC_P5 CALLM68KFUNC_P5
	#define CALLLIBFUNC_P6 CALLM68KFUNC_P6
	#define CALLLIBFUNC_P7 CALLM68KFUNC_P7
	#define CALLLIBFUNC_P8 CALLM68KFUNC_P8
	#define CALLLIBFUNC_P9 CALLM68KFUNC_P9

	//======================================================

#endif

// ==============================================================

#endif /* DEFS_H */

