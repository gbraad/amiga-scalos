//
// pmversion.c
//
// PopupMenu Library - Version information
//
// Copyright (C)1996-2001 Henrik Isaksson <henrik@boing.nu>
// All Rights Reserved.
//

#ifdef _M68020
#ifdef _M68030
#ifdef _M68040
#ifdef _M68060
#define __CPU__ "060"
#else
#define __CPU__ "040"
#endif
#else
#define __CPU__ "030"
#endif
#else
#define __CPU__ "020"
#endif
#else
#define __CPU__ "000"
#endif

char _LibID[] = "$VER: popupmenu.library-" __CPU__ " 10.10.181 (22.11.2006) �1996-2006 Henrik Isaksson";