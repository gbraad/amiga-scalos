// WindowProperties.h 
// 11 Aug 2004 20:42:01

#ifndef WINDOWPROPERTIES_H 
#define	WINDOWPROPERTIES_H 

#define	debugLock_d1(LockName) ;
#define	debugLock_d2(LockName) \
	{\
	char xxName[200];\
	strcpy(xxName, "");\
	NameFromLock((LockName), xxName, sizeof(xxName));\
	kprintf(__FILE__ "/" __FUNC__ "/%ld: " #LockName "=%08lx <%s>\n", __LINE__, LockName, xxName);\
	}

#if defined(__SASC)
int snprintf(char *, size_t, const char *, /*args*/ ...);
int vsnprintf(char *, size_t, const char *, va_list ap);
#endif /* __SASC */

#endif	/* WINDOWPROPERTIES_H  */
