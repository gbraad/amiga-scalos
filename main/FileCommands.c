// FileCommands.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <graphics/gels.h>
#include <graphics/rastport.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <utility/hooks.h>
#include <intuition/gadgetclass.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <cybergraphx/cybergraphics.h>
#include <datatypes/pictureclass.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/exall.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/iconobject.h>
#include <proto/utility.h>
#include <proto/gadtools.h>
#include <proto/datatypes.h>
#include <proto/wb.h>
#include "debug.h"
#include <proto/scalos.h>

#include <clib/alib_protos.h>

#include <defs.h>
#include <datatypes/iconobject.h>
#include <scalos/scalos.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#include "scalos_structures.h"
#include "locale.h"
#include "functions.h"
#include "Variables.h"
#include "int64.h"
#include "FileTransClass.h"
#include "GaugeGadgetClass.h"

//----------------------------------------------------------------------------

enum CopyErrReqResult { COPYERR_Retry = 1, COPYERR_Skip, COPYERR_Abort = 0, COPYERR_nop = ~0 };

struct GlobalCopyArgs
	{
	enum ExistsReqResult gca_Replace;
	};

typedef LONG (*ENTRYFUNC)(Class *cl, Object *o, 
	struct GlobalCopyArgs *gca, BPTR SrcDirLock, BPTR DestDirLock, 
	const struct FileInfoBlock *fib, CONST_STRPTR DestName);

//----------------------------------------------------------------------------

// local functions

static LONG MoveCommand_Move(Class *cl, Object *o, 
	BPTR SrcDirLock, BPTR DestDirLock, CONST_STRPTR FileName);
static LONG MoveCommand_CopyAndDelete(Class *cl, Object *o, 
	BPTR SrcDirLock, BPTR DestDirLock, CONST_STRPTR FileName);
static LONG CopyFile(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName);
static LONG CopyLink(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName);
static LONG CopyDir(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName,
	ENTRYFUNC EntryFunc);
static LONG CopyVolume(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName);
static LONG CopyEntry(Class *cl, Object *o,
	struct GlobalCopyArgs *gca, BPTR SrcDirLock, BPTR DestDirLock, 
	const struct FileInfoBlock *fib, CONST_STRPTR DestName);
static BOOL ExistsObject(BPTR DirLock, CONST_STRPTR Name);
static LONG ReportError(Class *cl, Object *o, LONG *Result, ULONG BodyTextID, ULONG GadgetTextID);
static LONG DeleteEntry(Class *cl, Object *o, BPTR parentLock, CONST_STRPTR Name);
static LONG ScaDeleteFile(Class *cl, Object *o, BPTR ParentLock, CONST_STRPTR Name);
static LONG ScaDeleteDir(Class *cl, Object *o, BPTR ParentLock, CONST_STRPTR Name);
static LONG CountEntry(Class *cl, Object *o, BPTR srcDirLock, struct FileInfoBlock *fib, BOOL recursive);
static LONG FileTrans_CountDir(Class *cl, Object *o, BPTR parentLock, CONST_STRPTR Name);
static LONG CopyAndDeleteEntry(Class *cl, Object *o, 
	struct GlobalCopyArgs *gca, BPTR SrcDirLock, BPTR DestDirLock, 
	const struct FileInfoBlock *fib, CONST_STRPTR DestName);
static LONG RememberError(Class *cl, Object *o, CONST_STRPTR FileName);
static void ClearError(Class *cl, Object *o);
static LONG RememberExNextError(Class *cl, Object *o, CONST_STRPTR FileName);
static LONG HandleWriteProtectError(Class *cl, Object *o, BPTR DirLock,
	CONST_STRPTR FileName, enum WriteProtectedReqType Type, LONG ErroCode);

//----------------------------------------------------------------------------

// local Data

//----------------------------------------------------------------------------

LONG CreateLinkCommand(Class *cl, Object *o, BPTR SrcDirLock, BPTR DestDirLock, CONST_STRPTR FileName)
{
	LONG Result = RETURN_OK;
	LONG reqResult = COPYERR_nop;
	STRPTR Buffer = NULL;
	BPTR oldDir;
	BPTR fLock = BNULL;
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);

	d1(KPrintF("%s/%s/%ld: ftci=%08lx\n", __FILE__, __FUNC__, __LINE__, ftci));

	if (!ExistsObject(SrcDirLock, FileName))
		{
		// silently return if source object is non-existing
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
		return RETURN_OK;
		}

	if (SCCV_ReplaceMode_Abort == ftci->ftci_ReplaceMode)
		{
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
		return RETURN_WARN;
		}
	
	if (ExistsObject(DestDirLock, FileName))
		{
		LONG existsResult = EXISTREQ_Replace;

		switch (ftci->ftci_ReplaceMode)
			{
		case SCCV_ReplaceMode_Ask:
			existsResult = DoMethod(o, SCCM_FileTrans_OverwriteRequest,
				OVERWRITEREQ_Move,
				SrcDirLock, FileName,
				DestDirLock, FileName,
				ftci->ftci_Window, 
				MSGID_EXISTSNAME_MOVE, MSGID_EXISTS_GNAME_NEW);
			break;

		case SCCV_ReplaceMode_Never:
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
			return RETURN_OK;
			break;

		case SCCV_ReplaceMode_Always:
			existsResult = EXISTREQ_ReplaceAll;
			break;
			}

		d1(kprintf("%s/%s/%ld: existsResult=%ld\n", __FILE__, __FUNC__, __LINE__, existsResult));

		switch (existsResult)
			{
		case EXISTREQ_ReplaceAll:
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Always;
			/* no break here !! */

		case EXISTREQ_Replace:
			do	{
				Result = DeleteEntry(cl, o, DestDirLock, FileName);

				if (Result != RETURN_OK && Result != RESULT_UserAborted)
					reqResult = ReportError(cl, o, &Result, MSGID_DELETEERRORNAME, MSGID_COPYERRORGNAME);
				} while (RETURN_OK == Result && COPYERR_Retry == reqResult);

			if (COPYERR_Abort == reqResult)
				{
				d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
				return RETURN_WARN;
				}
			break;

		case EXISTREQ_SkipAll:
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Never;
			return RETURN_OK;
			break;

		case EXISTREQ_Skip:
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
			return RETURN_OK;
			break;

		case EXISTREQ_Abort:
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Abort;
			
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
			return RETURN_WARN;
			break;
			}
		}

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_EnterNewDir,
		SrcDirLock, DestDirLock, FileName);

	oldDir = CurrentDir(DestDirLock);

	do	{
		BOOL createSoftLink = CurrentPrefs.pref_CreateSoftLinks;
		reqResult = COPYERR_nop;

		switch (ScaSameLock(SrcDirLock, DestDirLock))
			{
		case LOCK_SAME:
		case LOCK_SAME_VOLUME:
			break;
		case LOCK_DIFFERENT:
			// ALWAYS create soft links to different volumes!
			createSoftLink = TRUE;
			break;
			}

		do	{
			if (createSoftLink)
				{
				Buffer = AllocPathBuffer();
				d1(kprintf("%s/%s/%ld: Buffer=%08lx\n", __FILE__, __FUNC__, __LINE__, Buffer));
				if (NULL == Buffer)
					{
					Result = ERROR_NO_FREE_STORE;
					break;
					}

				if (!NameFromLock(SrcDirLock, Buffer, Max_PathLen))
					{
					Result = RememberError(cl, o, FileName);
					break;
					}
				if (!AddPart(Buffer, (STRPTR) FileName, Max_PathLen))
					{
					Result = RememberError(cl, o, FileName);
					break;
					}

				d1(kprintf("%s/%s/%ld: FileName=<%s> Buffer=<%s>\n", __FILE__, __FUNC__, __LINE__, FileName, Buffer));

				if (!MakeLink((STRPTR) FileName, (LONG) Buffer, LINK_SOFT))
					{
					Result = RememberError(cl, o, FileName);
					break;
					}
				}
			else
				{
				CurrentDir(SrcDirLock);

				fLock = Lock((STRPTR) FileName, ACCESS_READ);
				if ((BPTR)NULL == fLock)
					{
					Result = RememberError(cl, o, FileName);
					break;
					}

				CurrentDir(DestDirLock);

				if (!MakeLink((STRPTR) FileName, (LONG) fLock, 0))
					{
					Result = RememberError(cl, o, FileName);
					break;
					}
				}
			} while (0);

		if (Result != RETURN_OK && Result != RESULT_UserAborted)
			reqResult = ReportError(cl, o, &Result, MSGID_CREATE_LINK_ERRORNAME, MSGID_COPYERRORGNAME);

		} while (RETURN_OK == Result && COPYERR_Retry == reqResult);

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_Final,
		SrcDirLock, DestDirLock, FileName);

	if (RESULT_UserAborted == Result)
		{
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
		Result = RETURN_WARN;
		}

	CurrentDir(oldDir);

	if (fLock)
		UnLock(fLock);
	if (Buffer)
		FreePathBuffer(Buffer);
		
	d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}

//----------------------------------------------------------------------------


LONG MoveCommand(Class *cl, Object *o, BPTR SrcDirLock, BPTR DestDirLock, CONST_STRPTR FileName)
{
	LONG Result = RETURN_OK;
	LONG reqResult = COPYERR_nop;
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);

	d1(kprintf("%s/%s/%ld: ftci=%08lx\n", __FILE__, __FUNC__, __LINE__, ftci));

	if (!ExistsObject(SrcDirLock, FileName))
		{
		// silently return if source object is non-existing
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
		return RETURN_OK;
		}

	if (SCCV_ReplaceMode_Abort == ftci->ftci_ReplaceMode)
		{
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
		return RETURN_WARN;
		}
	
	if (ExistsObject(DestDirLock, FileName))
		{
		LONG existsResult = EXISTREQ_Replace;

		switch (ftci->ftci_ReplaceMode)
			{
		case SCCV_ReplaceMode_Ask:
			existsResult = DoMethod(o, SCCM_FileTrans_OverwriteRequest,
				OVERWRITEREQ_Move,
				SrcDirLock, FileName,
				DestDirLock, FileName,
				ftci->ftci_Window, 
				MSGID_EXISTSNAME_MOVE, MSGID_EXISTS_GNAME_NEW);
			break;

		case SCCV_ReplaceMode_Never:
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
			return RETURN_OK;
			break;

		case SCCV_ReplaceMode_Always:
			existsResult = EXISTREQ_ReplaceAll;
			break;
			}

		d1(kprintf("%s/%s/%ld: existsResult=%ld\n", __FILE__, __FUNC__, __LINE__, existsResult));

		switch (existsResult)
			{
		case EXISTREQ_ReplaceAll:
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Always;
			/* no break here !! */

		case EXISTREQ_Replace:
			do	{
				Result = DeleteEntry(cl, o, DestDirLock, FileName);

				if (Result != RETURN_OK && Result != RESULT_UserAborted)
					reqResult = ReportError(cl, o, &Result, MSGID_DELETEERRORNAME, MSGID_COPYERRORGNAME);
				} while (RETURN_OK == Result && COPYERR_Retry == reqResult);

			if (COPYERR_Abort == reqResult)
				{
				d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
				return RETURN_WARN;
				}
			break;

		case EXISTREQ_SkipAll:
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Never;
			return RETURN_OK;
			break;

		case EXISTREQ_Skip:
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
			return RETURN_OK;
			break;

		case EXISTREQ_Abort:
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Abort;
			
			d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
			return RETURN_WARN;
			break;
			}
		}

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_EnterNewDir,
		SrcDirLock, DestDirLock, FileName);

	do	{
		reqResult = COPYERR_nop;

		switch (ScaSameLock(SrcDirLock, DestDirLock))
			{
		case LOCK_SAME:
			break;
		case LOCK_SAME_VOLUME:
			Result = MoveCommand_Move(cl, o, SrcDirLock, DestDirLock, FileName);
			break;
		case LOCK_DIFFERENT:
			Result = MoveCommand_CopyAndDelete(cl, o, SrcDirLock, DestDirLock, FileName);
			break;
			}

		if (Result != RETURN_OK && Result != RESULT_UserAborted)
			reqResult = ReportError(cl, o, &Result, MSGID_MOVEERRORNAME, MSGID_COPYERRORGNAME);

		} while (RETURN_OK == Result && COPYERR_Retry == reqResult);

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_Final,
		SrcDirLock, DestDirLock, FileName);

	if (RESULT_UserAborted == Result)
		{
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_WARN));
		return RETURN_WARN;
		}

	d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


static LONG MoveCommand_Move(Class *cl, Object *o, 
	BPTR SrcDirLock, BPTR DestDirLock, CONST_STRPTR FileName)
{
	LONG Result = RETURN_OK;
	char *OldFileName, *NewFileName = NULL;

	do	{
		OldFileName = AllocPathBuffer();
		if (NULL == OldFileName)
			{
			SetIoErr(ERROR_NO_FREE_STORE);
			Result = RememberError(cl, o, FileName);
			break;
			}

		NewFileName = AllocPathBuffer();
		if (NULL == NewFileName)
			{
			SetIoErr(ERROR_NO_FREE_STORE);
			Result = RememberError(cl, o, FileName);
			break;
			}

		if (!NameFromLock(SrcDirLock, OldFileName, Max_PathLen - 1))
			{
			Result = RememberError(cl, o, FileName);
			break;
			}
		if (!NameFromLock(DestDirLock, NewFileName, Max_PathLen - 1))
			{
			Result = RememberError(cl, o, FileName);
			break;
			}

		if (!AddPart(OldFileName, (STRPTR) FileName, Max_PathLen - 1))
			{
			Result = RememberError(cl, o, FileName);
			break;
			}
		if (!AddPart(NewFileName, (STRPTR) FileName, Max_PathLen - 1))
			{
			Result = RememberError(cl, o, FileName);
			break;
			}

		if (!Rename(OldFileName, NewFileName))
			{
			Result = RememberError(cl, o, FileName);
			if (ERROR_WRITE_PROTECTED == Result
				|| ERROR_DELETE_PROTECTED == Result)
				{
				enum WriteProtectedReqResult overwrite;

				overwrite = HandleWriteProtectError(cl, o, SrcDirLock, OldFileName, WRITEPROTREQ_Move, Result);
				Result = RETURN_OK;
				switch (overwrite)
					{
				case WRITEPROTREQ_ReplaceAll:
				case WRITEPROTREQ_Replace:
					SetProtection(OldFileName, 0);
					if (!Rename(OldFileName, NewFileName))
						Result = RememberError(cl, o, FileName);
					break;
				default:
					break;
					}
				if (WRITEPROTREQ_Abort == overwrite)
					Result = RESULT_UserAborted;
				}
			}
		if (RETURN_OK != Result)
			break;
		} while (0);

	if (OldFileName)
		FreePathBuffer(OldFileName);
	if (NewFileName)
		FreePathBuffer(NewFileName);

	return Result;
}


static LONG MoveCommand_CopyAndDelete(Class *cl, Object *o,
	BPTR SrcDirLock, BPTR DestDirLock, CONST_STRPTR FileName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	STRPTR allocSrcFileName = NULL;
	BPTR oldDir;
	BPTR fLock = (BPTR)NULL;
	BPTR parentLock = (BPTR)NULL;
	struct FileInfoBlock *fib;
	LONG Result;
	BOOL SourceIsVolume = FALSE;
	struct GlobalCopyArgs gca;

	d1(kprintf("\n" "%s/%s/%ld: FileName=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, \
		FileName, FileName ? FileName : (CONST_STRPTR) ""));

	d1(kprintf("%s/%s/%ld: ftci=%08lx\n", __FILE__, __FUNC__, __LINE__, ftci));

	debugLock_d1(SrcDirLock);
	debugLock_d1(DestDirLock);

	if (SCCV_ReplaceMode_Abort == ftci->ftci_ReplaceMode)
		return RESULT_UserAborted;

	gca.gca_Replace = EXISTREQ_Skip;

	do	{
		oldDir = CurrentDir(SrcDirLock);

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, FileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (0 == strlen(FileName))
			{
			// Source is an absolute filename, i.e. "Volume:"
			SourceIsVolume = TRUE;
			}

		fLock = Lock((STRPTR) FileName, ACCESS_READ);
		if ((BPTR)NULL == fLock)
			{
			Result = RememberError(cl, o, FileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		d1(kprintf("%s/%s/%ld: FileName=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, \
			FileName, FileName ? FileName : (CONST_STRPTR) ""));

		debugLock_d1(fLock);

		if (!ScalosExamine64(fLock, fib))
			{
			Result = RememberError(cl, o, FileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (SourceIsVolume)
			{
			fib->fib_DirEntryType = ST_ROOT;
			if (strlen(fib->fib_FileName) < sizeof(fib->fib_FileName) - 1)
				SafeStrCat(fib->fib_FileName, ":", sizeof(fib->fib_FileName));
			}
		else
			{
			parentLock = ParentDir(fLock);
			if ((BPTR)NULL == parentLock)
				{
				Result = RememberError(cl, o, FileName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}
			}

		UnLock(fLock);
		fLock = (BPTR)NULL;

		DoMethod(o, SCCM_FileTrans_UpdateWindow,
			FTUPDATE_EnterNewDir,
			SrcDirLock, DestDirLock, fib->fib_FileName);

		Result = CopyAndDeleteEntry(cl, o, &gca, SrcDirLock, DestDirLock, fib, FileName);
		} while (0);

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_Final,
		SrcDirLock, DestDirLock, FileName);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	CurrentDir(oldDir);
	if (fLock)
		UnLock(fLock);
	if (parentLock)
		UnLock(parentLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);
	if (allocSrcFileName)
		FreePathBuffer(allocSrcFileName);

	if (RESULT_UserAborted == Result)
		Result = RESULT_UserAborted;

	return Result;
}


// SrcFileName may be "", then SourceDirLock is an absolute filename, i.e. "Volume:"
// returns	RETURN_OK or error code
LONG CopyCommand(Class *cl, Object *o, BPTR SrcDirLock, BPTR DestDirLock,
	CONST_STRPTR SrcFileName, CONST_STRPTR DestFileName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	STRPTR allocSrcFileName = NULL;
	BPTR oldDir;
	BPTR fLock = (BPTR)NULL;
	struct FileInfoBlock *fib;
	LONG Result;
	BOOL SourceIsVolume = FALSE;
	struct GlobalCopyArgs gca;

	d1(kprintf("\n" "%s/%s/%ld: SrcFileName=%08lx <%s>  DestFileName=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, \
		SrcFileName, SrcFileName ? SrcFileName : (CONST_STRPTR) "", \
		DestFileName, DestFileName ? DestFileName : (CONST_STRPTR) ""));

	d1(kprintf("%s/%s/%ld: ftci=%08lx\n", __FILE__, __FUNC__, __LINE__, ftci));

	debugLock_d1(SrcDirLock);
	debugLock_d1(DestDirLock);

	if (SCCV_ReplaceMode_Abort == ftci->ftci_ReplaceMode)
		return RETURN_WARN;

	if (!ExistsObject(SrcDirLock, SrcFileName))
		{
		// silently return if source object is non-existing
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
		return RETURN_OK;
		}

	gca.gca_Replace = EXISTREQ_Skip;

	do	{
		oldDir = CurrentDir(SrcDirLock);

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, SrcFileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (0 == strlen(SrcFileName))
			{
			// Source is an absolute filename, i.e. "Volume:"
			SourceIsVolume = TRUE;
			}

		if (IsSoftLink(SrcFileName))
			{
			stccpy(fib->fib_FileName, SrcFileName, sizeof(fib->fib_FileName));
			fib->fib_DirEntryType = ST_SOFTLINK;
			}
		else
			{
			fLock = Lock((STRPTR) SrcFileName, ACCESS_READ);
			if ((BPTR)NULL == fLock)
				{
				Result = RememberError(cl, o, SrcFileName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			d1(kprintf("%s/%s/%ld: SrcFileName=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, \
				SrcFileName, SrcFileName ? SrcFileName : (CONST_STRPTR) ""));

			debugLock_d1(fLock);

			if (!ScalosExamine64(fLock, fib))
				{
				Result = RememberError(cl, o, SrcFileName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}
			}

		if (SourceIsVolume)
			{
			fib->fib_DirEntryType = ST_ROOT;
			if (strlen(fib->fib_FileName) < sizeof(fib->fib_FileName) - 1)
				SafeStrCat(fib->fib_FileName, ":", sizeof(fib->fib_FileName));
			}

		DoMethod(o, SCCM_FileTrans_UpdateWindow,
			FTUPDATE_EnterNewDir,
			SrcDirLock, DestDirLock, fib->fib_FileName);

		Result = CopyEntry(cl, o, &gca, SrcDirLock, 
				DestDirLock, fib, DestFileName);
		} while (0);

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_Final,
		SrcDirLock, DestDirLock, DestFileName);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	CurrentDir(oldDir);
	if (fLock)
		UnLock(fLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);
	if (allocSrcFileName)
		FreePathBuffer(allocSrcFileName);

	if (RESULT_UserAborted == Result)
		Result = RETURN_WARN;

	return Result;
}


// SrcParentLock	: Lock to source parent directory
// DestParentLock	: Lock to destination parent directory
// Name			: Name of source dir (relative to SrcParentLock)

// returns 		: RETURN_OK or DOS error code
static LONG CopyFile(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result = RETURN_OK;
	UBYTE *Buffer = NULL;
	BPTR fdSrc, fdDest = (BPTR)NULL;
	BPTR oldDir;

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  DestName=<%s>\n", __FILE__, __FUNC__, __LINE__, SrcName, DestName));
	debugLock_d1(SrcParentLock);
	debugLock_d1(DestParentLock);

	do	{
		LONG Actual;

		oldDir = CurrentDir(SrcParentLock);

		fdSrc = Open((STRPTR) SrcName, MODE_OLDFILE);
		if ((BPTR)NULL == fdSrc)
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		Buffer = AllocVec(CurrentPrefs.pref_CopyBuffLen, MEMF_PUBLIC);
		if (NULL == Buffer)
			{
			Result = ERROR_NO_FREE_STORE;
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		CurrentDir(DestParentLock);

		fdDest = Open((STRPTR) DestName, MODE_NEWFILE);
		if ((BPTR)NULL == fdDest)
			{
			Result = RememberError(cl, o, DestName);
			d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (ERROR_WRITE_PROTECTED == Result
				|| ERROR_DELETE_PROTECTED == Result)
				{
				enum WriteProtectedReqResult overwrite;

				overwrite = HandleWriteProtectError(cl, o, DestParentLock, DestName, WRITEPROTREQ_Copy, Result);
				Result = RETURN_OK;
				switch (overwrite)
					{
				case WRITEPROTREQ_ReplaceAll:
				case WRITEPROTREQ_Replace:
					SetProtection(DestName, 0);
					fdDest = Open((STRPTR) DestName, MODE_NEWFILE);
					break;
				default:
					break;
					}
				if (WRITEPROTREQ_Abort == overwrite)
					{
					Result = RESULT_UserAborted;
					break;
					}
				}
			}

		if ((BPTR)NULL == fdDest)
			break;

		do	{
			Actual = Read(fdSrc, Buffer, CurrentPrefs.pref_CopyBuffLen);
			if (Actual < 0)
				{
				Result = RememberError(cl, o, SrcName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}
			if (Actual > 0)
				{
				LONG Written = Write(fdDest, Buffer, Actual);

				ftci->ftci_CurrentBytes = Add64(ftci->ftci_CurrentBytes, Make64(Written));

				if (Written != Actual)
					{
					Result = RememberError(cl, o, DestName);
					d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
					break;
					}
				}

			DoMethod(o, SCCM_FileTrans_UpdateWindow,
				FTUPDATE_SameFile,
				SrcParentLock, DestParentLock, SrcName);
			if (RETURN_OK == Result)
				Result = DoMethod(o, SCCM_FileTrans_CheckAbort);
			} while (Actual && RETURN_OK == Result);

		if (RETURN_OK != Result)
			{
			// Remove destination after error

			// no need to check fdDest -- file MUST have been opened successfully!
			Close(fdDest);
			fdDest = (BPTR)NULL;

			// don't check for errors here
			(void) ScaDeleteFile(cl, o, DestParentLock, DestName);
			}
		} while (0);

	if (Buffer)
		FreeVec(Buffer);
	if (fdSrc)
		Close(fdSrc);
	if (fdDest)
		Close(fdDest);
	CurrentDir(oldDir);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


// SrcParentLock	: Lock to source parent directory
// DestParentLock	: Lock to destination parent directory
// Name			: Name of source dir (relative to SrcParentLock)

// returns 		: RETURN_OK or DOS error code
static LONG CopyLink(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result = RETURN_OK;
	STRPTR Buffer;
	struct DevProc *devproc = NULL;
	BPTR oldDir;

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  DestName=<%s>\n", __FILE__, __FUNC__, __LINE__, SrcName, DestName));
	debugLock_d1(SrcParentLock);
	debugLock_d1(DestParentLock);

	do	{
		oldDir = CurrentDir(SrcParentLock);

		Buffer = AllocPathBuffer();
		d1(kprintf("%s/%s/%ld: Buffer=%08lx\n", __FILE__, __FUNC__, __LINE__, Buffer));
		if (NULL == Buffer)
			{
			Result = ERROR_NO_FREE_STORE;
			break;
			}

		if (!NameFromLock(SrcParentLock, Buffer, Max_PathLen))
			{
			Result = RememberError(cl, o, SrcName);
			break;
			}

		devproc = GetDeviceProc(Buffer, NULL);
		if (NULL == devproc)
			{
			Result = RememberError(cl, o, SrcName);
			break;
			}

		if (!ReadLink(devproc->dvp_Port, SrcParentLock, (STRPTR) SrcName,
				Buffer, Max_PathLen))
			{
			Result = RememberError(cl, o, SrcName);
			break;
			}

		d1(kprintf("%s/%s/%ld: Link Dest=<%s>\n", __FILE__, __FUNC__, __LINE__, Buffer));

		CurrentDir(DestParentLock);

		if (!MakeLink((STRPTR) DestName, (LONG) Buffer, LINK_SOFT))
			{
			Result = RememberError(cl, o, DestName);
			d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

			if (ERROR_OBJECT_EXISTS == Result)
				{
				// try to delete and re-MakeLink()
				d1(kprintf("%s/%s/%ld: Special error handling for ERROR_OBJECT_EXISTS\n", __FILE__, __FUNC__, __LINE__));

				Result = RETURN_OK;
				ClearError(cl, o);

				if (!DeleteFile((STRPTR) DestName))
					{
					Result = RememberError(cl, o, DestName);
					}

				d1(kprintf("%s/%s/%ld: after deleting link, Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

				if (RETURN_OK == Result)
					{
					if (!MakeLink((STRPTR) DestName, (LONG) Buffer, TRUE))
						{
						Result = RememberError(cl, o, DestName);
						}

					d1(KPrintF("%s/%s/%ld: after MakeLink(), Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
					}
				}

			if (RETURN_OK != Result)
				break;
			}
		} while (0);

	if (devproc)
		FreeDeviceProc(devproc);
	if (Buffer)
		FreePathBuffer(Buffer);
	CurrentDir(oldDir);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


// SrcParentLock	: Lock to source parent directory
// DestParentLock	: Lock to destination parent directory
// Name			: Name of source dir (relative to SrcParentLock)

// returns 		: RETURN_OK or DOS error code
static LONG CopyDir(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName,
	ENTRYFUNC EntryFunc)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;
	struct FileInfoBlock *fib = NULL;
	BPTR oldDir = CurrentDir(SrcParentLock);
	BPTR srcDirLock;			// lock to source directory
	BPTR DestDirLock = (BPTR)NULL;		// lock to (freshly-created) destination directory

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  DestName=<%s>\n", __FILE__, __FUNC__, __LINE__, SrcName, DestName));
	debugLock_d1(SrcParentLock);
	debugLock_d1(DestParentLock);

	do	{
		srcDirLock = Lock((STRPTR) SrcName, ACCESS_READ);
		if ((BPTR)NULL == srcDirLock)
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (!ScalosExamine64(srcDirLock, fib))
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		// create dest dir
		CurrentDir(DestParentLock);
		DestDirLock = CreateDir((STRPTR) DestName);
		if ((BPTR)NULL == DestDirLock)
			{
			Result = RememberError(cl, o, DestName);
			d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

			if (ERROR_OBJECT_EXISTS == Result)
				{
				DestDirLock = Lock((STRPTR) DestName, ACCESS_READ);
				ClearError(cl, o);
				Result = RememberError(cl, o, DestName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}

			if ((BPTR)NULL == DestDirLock)
				break;
			}
		else
			{
			// change ACCESS_WRITE lock to ACCESS_READ
			ChangeMode(CHANGE_LOCK, DestDirLock, ACCESS_READ);
			}

		DoMethod(o, SCCM_FileTrans_UpdateWindow,
			FTUPDATE_EnterNewDir,
			srcDirLock, DestDirLock, SrcName);

		do	{
			if (!ScalosExNext64(srcDirLock, fib))
				{
				Result = RememberExNextError(cl, o, SrcName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			Result = CopyEntry(cl, o, gca, srcDirLock, DestDirLock, fib, fib->fib_FileName);
			} while (RETURN_OK == Result);

		if (ERROR_NO_MORE_ENTRIES == Result)
			Result = RETURN_OK;
		} while (0);

	CurrentDir(oldDir);

	if (DestDirLock)
		UnLock(DestDirLock);
	if (srcDirLock)
		UnLock(srcDirLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


// SrcParentLock	: Lock to source parent directory
// DestParentLock	: Lock to destination parent directory
// Name			: Name of source dir (relative to SrcParentLock)

// returns 		: RETURN_OK or DOS error code
static LONG CopyVolume(Class *cl, Object *o, struct GlobalCopyArgs *gca,
	BPTR SrcParentLock, BPTR DestParentLock, CONST_STRPTR SrcName, CONST_STRPTR DestName)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;
	struct FileInfoBlock *fib = NULL;
	BPTR oldDir = CurrentDir(SrcParentLock);
	BPTR srcDirLock;			// lock to source directory
	BPTR DestDirLock = (BPTR)NULL;		// lock to (freshly-created) destination directory

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  DestName=<%s>\n", __FILE__, __FUNC__, __LINE__, SrcName, DestName));
	debugLock_d1(SrcParentLock);
	debugLock_d1(DestParentLock);

	do	{
		srcDirLock = DupLock(SrcParentLock);
		if ((BPTR)NULL == srcDirLock)
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (!ScalosExamine64(srcDirLock, fib))
			{
			Result = RememberError(cl, o, SrcName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		CurrentDir(DestParentLock);

		// create dest dir
		CurrentDir(DestParentLock);
		DestDirLock = CreateDir((STRPTR) DestName);
		if ((BPTR)NULL == DestDirLock)
			{
			Result = RememberError(cl, o, DestName);
			d1(KPrintF("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

			if (ERROR_OBJECT_EXISTS == Result)
				{
				DestDirLock = Lock((STRPTR) DestName, ACCESS_READ);
				ClearError(cl, o);
				Result = RememberError(cl, o, DestName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}

			if ((BPTR)NULL == DestDirLock)
				break;
			}
		else
			{
			// change ACCESS_WRITE lock to ACCESS_READ
			ChangeMode(CHANGE_LOCK, DestDirLock, ACCESS_READ);
			}

		DoMethod(o, SCCM_FileTrans_UpdateWindow,
			FTUPDATE_EnterNewDir,
			srcDirLock, DestDirLock, SrcName);

		do	{
			if (!ScalosExNext64(srcDirLock, fib))
				{
				Result = RememberExNextError(cl, o, SrcName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			Result = CopyEntry(cl, o, gca, srcDirLock, DestDirLock, fib, fib->fib_FileName);
			} while (RETURN_OK == Result);
		} while (0);

	CurrentDir(oldDir);

	if (DestDirLock)
		UnLock(DestDirLock);
	if (srcDirLock)
		UnLock(srcDirLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


static LONG CopyEntry(Class *cl, Object *o, 
	struct GlobalCopyArgs *gca, BPTR SrcDirLock, BPTR DestDirLock, 
	const struct FileInfoBlock *fib, CONST_STRPTR DestName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result = RETURN_OK;
	LONG reqResult;
	BOOL fExists;

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  DestName=<%s>  Type=%ld  current=%lu\n", \
		__FILE__, __FUNC__, __LINE__, fib->fib_FileName, DestName, fib->fib_DirEntryType, ftci->ftci_CurrentItems));

	if (ST_ROOT == fib->fib_DirEntryType)
		{
		STRPTR VolName = AllocCopyString(DestName);

		if (VolName)
			{
			StripTrailingColon(VolName);
			fExists = ExistsObject(DestDirLock, VolName);

			FreeCopyString(VolName);
			}
		else
			fExists = ExistsObject(DestDirLock, DestName);
		}
	else
		{
		fExists = ExistsObject(DestDirLock, DestName);
		}

	ftci->ftci_MostCurrentReplaceMode = ftci->ftci_ReplaceMode;

	if (fExists)
		{
		LONG existsResult = EXISTREQ_Replace;

		switch (ftci->ftci_ReplaceMode)
			{
		case SCCV_ReplaceMode_Ask:
			if (EXISTREQ_ReplaceAll == gca->gca_Replace)
				existsResult = EXISTREQ_Replace;
			else
				{
				existsResult = ftci->ftci_MostCurrentReplaceMode = DoMethod(o, SCCM_FileTrans_OverwriteRequest,
					OVERWRITEREQ_Copy,
					SrcDirLock, fib->fib_FileName,
					DestDirLock, DestName,
					ftci->ftci_Window, 
					MSGID_EXISTSNAME_COPY, MSGID_EXISTS_GNAME_NEW);
				}
			break;
		case SCCV_ReplaceMode_Never:
			return RETURN_OK;
			break;
		case SCCV_ReplaceMode_Always:
			existsResult = ftci->ftci_MostCurrentReplaceMode = EXISTREQ_Replace;
			break;
			}

		d1(kprintf("%s/%s/%ld: existsResult=%ld\n", __FILE__, __FUNC__, __LINE__, existsResult));

		switch (existsResult)
			{
		case EXISTREQ_Replace:
			break;

		case EXISTREQ_Skip:
			return RETURN_OK;
			break;

		case EXISTREQ_SkipAll:
			ftci->ftci_ReplaceMode = SCCV_ReplaceMode_Never;
			return RETURN_OK;
			break;

		case EXISTREQ_ReplaceAll:
			gca->gca_Replace = EXISTREQ_ReplaceAll;
			ftci->ftci_ReplaceMode = ftci->ftci_MostCurrentReplaceMode = SCCV_ReplaceMode_Always;
			break;

		case EXISTREQ_Abort:
			ftci->ftci_ReplaceMode = ftci->ftci_MostCurrentReplaceMode = SCCV_ReplaceMode_Abort;
			return RESULT_UserAborted;
			break;
			}
		}

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_Entry,
		SrcDirLock, DestDirLock, fib->fib_FileName);

	do	{
		d1(kprintf("%s/%s/%ld: Start of do loop\n", __FILE__, __FUNC__, __LINE__));
		reqResult = COPYERR_nop;

		switch (fib->fib_DirEntryType)
			{
		case ST_FILE:
		case ST_LINKFILE:
		case ST_PIPEFILE:
			d1(kprintf("%s/%s/%ld: Calling CopyFile for FILE, LINK or PIPE\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyFile(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
			break;

		case ST_ROOT:
			d1(kprintf("%s/%s/%ld: Calling CopyVolume for ROOT\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyVolume(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
			break;

		case ST_USERDIR:
		case ST_LINKDIR:
			d1(kprintf("%s/%s/%ld: Calling CopyDir for USERDIR or LINKDIR\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyDir(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName, CopyEntry);
			break;

		case ST_SOFTLINK:
			d1(kprintf("%s/%s/%ld: Calling CopyLink for softlink\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyLink(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
			break;

		/* Catch for types which are not any of the above values, but should still
		 * be handled (i.e. copying files across a Samba connection)
		 */
		default:
			if(fib->fib_DirEntryType < 0)
				{
				// generally speaking <0 is file
				d1(kprintf("%s/%s/%ld: DeFAULT: Calling CopyFile for FILE, LINK or PIPE\n", __FILE__, __FUNC__, __LINE__));
				Result = CopyFile(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
				}
			else
				{
				// and >=0 is a directory
				d1(kprintf("%s/%s/%ld: DEFAULT: Calling CopyDir for USERDIR or LINKDIR\n", __FILE__, __FUNC__, __LINE__));
				Result = CopyDir(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName, CopyEntry);
				}
			break;
			}

		if (RETURN_OK == Result)
			{
			BPTR oldDir = CurrentDir(DestDirLock);

			d1(kprintf("%s/%s/%ld: Result OK\n", __FILE__, __FUNC__, __LINE__));

			SetFileDate((STRPTR) DestName, (struct DateStamp *) &fib->fib_Date);
			SetComment((STRPTR) DestName, (STRPTR) fib->fib_Comment);
			SetProtection((STRPTR) DestName, fib->fib_Protection & ~FIBF_ARCHIVE);

			CurrentDir(oldDir);
			}

		if (RETURN_OK == Result)
			Result = DoMethod(o, SCCM_FileTrans_CheckAbort);

		if (Result != RETURN_OK && Result != RESULT_UserAborted)
			{
			d1(kprintf("%s/%s/%ld: Weird error!\n", __FILE__, __FUNC__, __LINE__));
			reqResult = ReportError(cl, o, &Result, MSGID_COPYERRORNAME, MSGID_COPYERRORGNAME);
			}

		} while (RETURN_OK == Result && COPYERR_Retry == reqResult);

	ftci->ftci_CurrentItems++;

	return Result;
}


// check if named object (file/directory) exists
static BOOL ExistsObject(BPTR DirLock, CONST_STRPTR Name)
{
	BOOL Exists = FALSE;
	BPTR oldDir = CurrentDir(DirLock);
	BPTR fLock;

	fLock = Lock((STRPTR) Name, ACCESS_READ);
	if (fLock)
		{
		Exists = TRUE;
		UnLock(fLock);
		}
	else
		{
		LONG Err = IoErr();

		switch (Err)
			{
		case ERROR_OBJECT_IN_USE:
			Exists = TRUE;
			break;
			}
		}

	CurrentDir(oldDir);

	return Exists;
}


static LONG ReportError(Class *cl, Object *o, LONG *Result, ULONG BodyTextID, ULONG GadgetTextID)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG reqResult;
	char FaultBuffer[100];

	Fault(ftci->ftci_LastErrorCode, (STRPTR) "", FaultBuffer, sizeof(FaultBuffer)-1);

	// reqResult : 1=Retry, 2=Skip, 3=Abort
	reqResult = UseRequestArgs(ftci->ftci_Window,
		BodyTextID, GadgetTextID, 
		2,
		ftci->ftci_LastErrorFileName,
		FaultBuffer
		);

	d1(kprintf("%s/%s/%ld: UseRequest returned=%ld\n", __FILE__, __FUNC__, __LINE__, reqResult));

	switch (reqResult)
		{
	case COPYERR_Skip:
		// Skip : drop current object but try to continue copying
		*Result = RETURN_OK;
		break;

	case COPYERR_Retry:
		*Result = RETURN_OK;
		break;

	case COPYERR_Abort:
		*Result = RESULT_UserAborted;
		break;
		}

	if (ftci->ftci_LastErrorFileName)
		{
		FreeCopyString(ftci->ftci_LastErrorFileName);
		ftci->ftci_LastErrorFileName = NULL;
		}

	return reqResult;
}


static LONG DeleteEntry(Class *cl, Object *o, BPTR parentLock, CONST_STRPTR Name)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;
	struct FileInfoBlock *fib;
	BPTR objLock = (BPTR)NULL;
	BPTR oldDir = CurrentDir(parentLock);

	d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, Name));
	debugLock_d1(parentLock);

	do	{
		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (IsSoftLink(Name))
			{
			stccpy(fib->fib_FileName, Name, sizeof(fib->fib_FileName));
			fib->fib_DirEntryType = ST_SOFTLINK;
			}
		else
			{
			objLock = Lock((STRPTR) Name, ACCESS_READ);
			if ((BPTR)NULL == objLock)
				{
				Result = RememberError(cl, o, Name);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			if (!ScalosExamine64(objLock, fib))
				{
				Result = RememberError(cl, o, Name);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			d1(kprintf("%s/%s/%ld: FileName=<%s>  DirEntryType=%ld\n", \
				__FILE__, __FUNC__, __LINE__, fib->fib_FileName, fib->fib_DirEntryType));

			// if we don't UnLock() now, object cannot be deleted !
			UnLock(objLock);
			objLock = (BPTR)NULL;
			}

		switch (fib->fib_DirEntryType)
			{
		case ST_FILE:
		case ST_LINKFILE:
		case ST_PIPEFILE:
		case ST_SOFTLINK:
			Result = ScaDeleteFile(cl, o, parentLock, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;

		case ST_ROOT:
		case ST_USERDIR:
		case ST_LINKDIR:
			Result = ScaDeleteDir(cl, o, parentLock, Name);	// delete directory contents
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (RETURN_OK == Result)
				Result = ScaDeleteFile(cl, o, parentLock, Name);	// delete directory itself
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;

		default:
			if (fib->fib_DirEntryType < 0)
				{
				// generally speaking <0 is file
				Result = ScaDeleteFile(cl, o, parentLock, Name);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}
			else
				{
				// and >=0 is a directory
				Result = ScaDeleteDir(cl, o, parentLock, Name);	// delete directory contents
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				if (RETURN_OK == Result)
					Result = ScaDeleteFile(cl, o, parentLock, Name);	// delete directory itself
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}
			break;
			}
		} while (0);

	CurrentDir(oldDir);

	if (objLock)
		UnLock(objLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


// ParentLock		: Lock to parent directory
// Name			: Name of file (relative to ParentLock)

// returns 		: RETURN_OK or DOS error code
static LONG ScaDeleteFile(Class *cl, Object *o, BPTR parentLock, CONST_STRPTR Name)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result = RETURN_OK;
	BPTR oldDir;

	d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, Name));
	debugLock_d1(parentLock);

	do	{
		oldDir = CurrentDir(parentLock);

		if (!DeleteFile((STRPTR) Name))
			{
			Result = RememberError(cl, o, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (ERROR_WRITE_PROTECTED == Result
				|| ERROR_DELETE_PROTECTED == Result)
				{
				enum WriteProtectedReqResult overwrite;

				overwrite = HandleWriteProtectError(cl, o, parentLock, Name, WRITEPROTREQ_Move, Result);
				Result = RETURN_OK;
				switch (overwrite)
					{
				case WRITEPROTREQ_ReplaceAll:
				case WRITEPROTREQ_Replace:
					SetProtection(Name, 0);
					if (!DeleteFile((STRPTR) Name))
						Result = RememberError(cl, o, Name);
					break;
				default:
					break;
					}
				if (WRITEPROTREQ_Abort == overwrite)
					Result = RESULT_UserAborted;
				}
			}
		if (RETURN_OK != Result)
			break;
		} while (0);

	CurrentDir(oldDir);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


// parentLock		: Lock to parent directory
// Name			: Name of dir (relative to parentLock)

// returns 		: RETURN_OK or DOS error code
static LONG ScaDeleteDir(Class *cl, Object *o, BPTR parentLock, CONST_STRPTR Name)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;
	struct FileInfoBlock *fib = NULL;
	BPTR oldDir = CurrentDir(parentLock);
	BPTR dirLock;			// lock to source directory

	d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, Name));
	debugLock_d1(parentLock);

	do	{
		dirLock = Lock((STRPTR) Name, ACCESS_READ);
		if ((BPTR)NULL == dirLock)
			{
			Result = RememberError(cl, o, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (!ScalosExamine64(dirLock, fib))
			{
			Result = RememberError(cl, o, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		do	{
			if (!ScalosExNext64(dirLock, fib))
				{
				Result = RememberExNextError(cl, o, Name);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			Result = DeleteEntry(cl, o, dirLock, fib->fib_FileName);
			} while (RETURN_OK == Result);
		} while (0);

	CurrentDir(oldDir);
	if (dirLock)
		UnLock(dirLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


LONG CountCommand(Class *cl, Object *o, BPTR SrcDirLock, CONST_STRPTR SrcFileName, BOOL Recursive)
{
	LONG Result = RETURN_OK;
	BOOL SourceIsVolume = FALSE;
	struct FileInfoBlock *fib;
	BPTR fLock = (BPTR)NULL;
	BPTR oldDir;

	d1(kprintf("%s/%s/%ld: Name=<%s>  totalItems=%lu\n", __FILE__, __FUNC__, __LINE__, SrcFileName, ftci->ftci_TotalItems));
	debugLock_d1(SrcDirLock);

	DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);

	do	{
		oldDir = CurrentDir(SrcDirLock);

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, SrcFileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		if (0 == strlen(SrcFileName))
			{
			// Source is an absolute filename, i.e. "Volume:"
			SourceIsVolume = TRUE;
			}

		if (IsSoftLink(SrcFileName))
			{
			stccpy(fib->fib_FileName, SrcFileName, sizeof(fib->fib_FileName));
			fib->fib_DirEntryType = ST_SOFTLINK;
			}
		else
			{
			fLock = Lock((STRPTR) SrcFileName, ACCESS_READ);
			if ((BPTR)NULL == fLock)
				{
				Result = RememberError(cl, o, SrcFileName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			d1(kprintf("%s/%s/%ld: SrcFileName=%08lx <%s>\n", __FILE__, __FUNC__, __LINE__, \
				SrcFileName, SrcFileName ? SrcFileName : (CONST_STRPTR) ""));

			debugLock_d1(fLock);

			if (!ScalosExamine64(fLock, fib))
				{
				Result = RememberError(cl, o, SrcFileName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}
			}

		if (SourceIsVolume)
			{
			fib->fib_DirEntryType = ST_ROOT;
			if (strlen(fib->fib_FileName) < sizeof(fib->fib_FileName) - 1)
				SafeStrCat(fib->fib_FileName, ":", sizeof(fib->fib_FileName));
			}

		if (RETURN_OK == Result)
			Result = CountEntry(cl, o, SrcDirLock, fib, Recursive);
		} while (0);

	if (RETURN_OK == Result)
		Result = DoMethod(o, SCCM_FileTrans_CheckAbort);

	CurrentDir(oldDir);

	if (fLock)
		UnLock(fLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


static LONG CountEntry(Class *cl, Object *o, BPTR srcDirLock, struct FileInfoBlock *fib, BOOL recursive)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result = RETURN_OK;

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  Type=%ld  current=%lu\n", \
		__FILE__, __FUNC__, __LINE__, fib->fib_FileName, fib->fib_DirEntryType, ftci->ftci_CurrentItems));

	DoMethod(o, SCCM_FileTrans_OpenWindowDelayed);

	do	{
		switch (fib->fib_DirEntryType)
			{
		case ST_FILE:
		case ST_LINKFILE:
		case ST_PIPEFILE:
			ftci->ftci_TotalFiles++;

			ftci->ftci_TotalBytes = Add64(ftci->ftci_TotalBytes, ScalosFibSize64(fib));

			d1(kprintf("%s/%s/%ld: Name=<%s>  Size=%ld  Total=%lu\n", \
				__FILE__, __FUNC__, __LINE__, Name, fib->fib_Size, ftci->ftci_TotalBytes.Low));
			break;

		case ST_SOFTLINK:
			ftci->ftci_TotalLinks++;
			break;

		case ST_LINKDIR:
		case ST_USERDIR:
			if (recursive)
				Result = FileTrans_CountDir(cl, o, srcDirLock, fib->fib_FileName);	// count directory contents
			ftci->ftci_TotalDirs++;
			break;

		default:
			if(fib->fib_DirEntryType < 0)
				{
				// generally speaking <0 is file
				ftci->ftci_TotalItems++;
				ftci->ftci_TotalFiles++;

				ftci->ftci_TotalBytes = Add64(ftci->ftci_TotalBytes, ScalosFibSize64(fib));

				d1(kprintf("%s/%s/%ld: Name=<%s>  Size=%ld  Total=%lu\n", \
					__FILE__, __FUNC__, __LINE__, Name, fib->fib_Size, ftci->ftci_TotalBytes.Low));
				}
			else
				{
				// and >=0 is a directory
				// count directory contents
				if (recursive)
					Result = FileTrans_CountDir(cl, o, srcDirLock, fib->fib_FileName);
				ftci->ftci_TotalDirs++;
				}
			break;
			}

		if (RETURN_OK == Result)
			Result = DoMethod(o, SCCM_FileTrans_CheckAbort);

		} while (0);

	ftci->ftci_TotalItems++;

	return Result;
}


// parentLock		: Lock to parent directory
// Name			: Name of dir (relative to parentLock)

// returns 		: RETURN_OK or DOS error code
static LONG FileTrans_CountDir(Class *cl, Object *o, BPTR parentLock, CONST_STRPTR Name)
{
//	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result;
	struct FileInfoBlock *fib = NULL;
	BPTR oldDir = CurrentDir(parentLock);
	BPTR dirLock;			// lock to source directory

	d1(kprintf("%s/%s/%ld: Name=<%s>\n", __FILE__, __FUNC__, __LINE__, Name));
	debugLock_d1(parentLock);

	if (DoMethod(o, SCCM_FileTrans_CountTimeout))
		return RETURN_WARN;

	do	{
		dirLock = Lock((STRPTR) Name, ACCESS_READ);
		if ((BPTR)NULL == dirLock)
			{
			Result = RememberError(cl, o, Name);
			break;
			}

		fib = AllocDosObject(DOS_FIB, NULL);
		if (NULL == fib)
			{
			Result = RememberError(cl, o, Name);
			break;
			}

		if (!ScalosExamine64(dirLock, fib))
			{
			Result = RememberError(cl, o, Name);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;
			}

		do	{
			if (!ScalosExNext64(dirLock, fib))
				{
				Result = RememberExNextError(cl, o, Name);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				break;
				}

			Result = CountEntry(cl, o, dirLock, fib, TRUE);
			} while (RETURN_OK == Result);
		} while (0);

	CurrentDir(oldDir);
	if (dirLock)
		UnLock(dirLock);
	if (fib)
		FreeDosObject(DOS_FIB, fib);

	d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

	return Result;
}


static LONG CopyAndDeleteEntry(Class *cl, Object *o, 
	struct GlobalCopyArgs *gca, BPTR SrcDirLock, BPTR DestDirLock, 
	const struct FileInfoBlock *fib, CONST_STRPTR DestName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG Result = RETURN_OK;
	LONG reqResult;
	BOOL fExists;

	d1(kprintf("%s/%s/%ld: SrcName=<%s>  DestName=<%s>  Type=%ld\n", \
		__FILE__, __FUNC__, __LINE__, fib->fib_FileName, DestName, fib->fib_DirEntryType));

	if (ST_ROOT == fib->fib_DirEntryType)
		{
		STRPTR VolName = AllocCopyString(DestName);

		if (VolName)
			{
			StripTrailingColon(VolName);
			fExists = ExistsObject(DestDirLock, VolName);

			FreeCopyString(VolName);
			}
		else
			fExists = ExistsObject(DestDirLock, DestName);
		}
	else
		{
		fExists = ExistsObject(DestDirLock, DestName);
		}

	if (fExists)
		{
		LONG existsResult = EXISTREQ_Replace;

		switch (ftci->ftci_ReplaceMode)
			{
		case SCCV_ReplaceMode_Ask:
			if (EXISTREQ_ReplaceAll == gca->gca_Replace)
				existsResult = EXISTREQ_Replace;
			else
				{
				existsResult = ftci->ftci_MostCurrentReplaceMode = DoMethod(o, SCCM_FileTrans_OverwriteRequest,
					OVERWRITEREQ_Copy,
					SrcDirLock, fib->fib_FileName,
					DestDirLock, DestName,
					ftci->ftci_Window, 
					MSGID_EXISTSNAME_COPY, MSGID_EXISTS_GNAME_NEW);
				}
			break;
		case SCCV_ReplaceMode_Never:
			return RETURN_OK;
			break;
		case SCCV_ReplaceMode_Always:
			existsResult = ftci->ftci_MostCurrentReplaceMode = EXISTREQ_Replace;
			break;
			}

		d1(kprintf("%s/%s/%ld: existsResult=%ld\n", __FILE__, __FUNC__, __LINE__, existsResult));

		switch (existsResult)
			{
		case EXISTREQ_Replace:
			break;

		case EXISTREQ_Skip:
			return RETURN_OK;
			break;

		case EXISTREQ_SkipAll:
			ftci->ftci_ReplaceMode = ftci->ftci_MostCurrentReplaceMode = SCCV_ReplaceMode_Never;
			return RETURN_OK;
			break;

		case EXISTREQ_ReplaceAll:
			gca->gca_Replace = EXISTREQ_ReplaceAll;
			ftci->ftci_ReplaceMode = ftci->ftci_MostCurrentReplaceMode = SCCV_ReplaceMode_Always;
			break;

		case EXISTREQ_Abort:
			ftci->ftci_ReplaceMode = ftci->ftci_MostCurrentReplaceMode = SCCV_ReplaceMode_Abort;
			return RESULT_UserAborted;
			break;
			}
		}

	DoMethod(o, SCCM_FileTrans_UpdateWindow,
		FTUPDATE_Entry,
		SrcDirLock, DestDirLock, fib->fib_FileName);

	do	{
		d1(kprintf("%s/%s/%ld: Start of do loop  fib_DirEntryType=%ld\n", __FILE__, __FUNC__, __LINE__, fib->fib_DirEntryType));
		reqResult = COPYERR_nop;

		switch (fib->fib_DirEntryType)
			{
		case ST_FILE:
		case ST_LINKFILE:
		case ST_PIPEFILE:
			d1(kprintf("%s/%s/%ld: Calling CopyFile for FILE, LINK or PIPE\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyFile(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
			if (RETURN_OK == Result)
				Result = ScaDeleteFile(cl, o, SrcDirLock, fib->fib_FileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;

		case ST_ROOT:
			d1(kprintf("%s/%s/%ld: Calling CopyVolume for ROOT\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyVolume(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;

		case ST_USERDIR:
		case ST_LINKDIR:
			d1(kprintf("%s/%s/%ld: Calling CopyDir for USERDIR or LINKDIR\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyDir(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName, CopyAndDeleteEntry);
			if (RETURN_OK == Result)
				Result = ScaDeleteDir(cl, o, SrcDirLock, fib->fib_FileName);	// delete directory contents
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (RETURN_OK == Result)
				Result = ScaDeleteFile(cl, o, SrcDirLock, fib->fib_FileName);	// delete directory itself
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;

		case ST_SOFTLINK:
			d1(kprintf("%s/%s/%ld: Calling CopyLink for softlink\n", __FILE__, __FUNC__, __LINE__));
			Result = CopyLink(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			if (RETURN_OK == Result)
				Result = ScaDeleteFile(cl, o, SrcDirLock, fib->fib_FileName);
			d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
			break;

		/* Catch for types which are not any of the above values, but should still
		 * be handled (i.e. copying files across a Samba connection)
		 */
		default:
			if(fib->fib_DirEntryType < 0)
				{
				// generally speaking <0 is file
				d1(kprintf("%s/%s/%ld: DEFAULT: Calling CopyFile for FILE, LINK or PIPE\n", __FILE__, __FUNC__, __LINE__));
				Result = CopyFile(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				if (RETURN_OK == Result)
					Result = ScaDeleteFile(cl, o, SrcDirLock, fib->fib_FileName);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}
			else
				{
				// and >=0 is a directory
				d1(kprintf("%s/%s/%ld: DEFAULT: Calling CopyDir for USERDIR or LINKDIR\n", __FILE__, __FUNC__, __LINE__));
				Result = CopyDir(cl, o, gca, SrcDirLock, DestDirLock, fib->fib_FileName, DestName, CopyAndDeleteEntry);
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				if (RETURN_OK == Result)
					Result = ScaDeleteDir(cl, o, SrcDirLock, fib->fib_FileName);	// delete directory contents
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				if (RETURN_OK == Result)
					Result = ScaDeleteFile(cl, o, SrcDirLock, fib->fib_FileName);	// delete directory itself
				d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));
				}
			break;
			}

		if (RETURN_OK == Result)
			{
			BPTR oldDir = CurrentDir(DestDirLock);

			d1(kprintf("%s/%s/%ld: Result OK\n", __FILE__, __FUNC__, __LINE__));

			SetFileDate((STRPTR) DestName, (struct DateStamp *) &fib->fib_Date);
			SetComment((STRPTR) DestName, (STRPTR) fib->fib_Comment);
			SetProtection((STRPTR) DestName, fib->fib_Protection & ~FIBF_ARCHIVE);

			CurrentDir(oldDir);
			}

		d1(kprintf("%s/%s/%ld: Result=%ld\n", __FILE__, __FUNC__, __LINE__, Result));

		if (RETURN_OK == Result)
			Result = DoMethod(o, SCCM_FileTrans_CheckAbort);

		if (Result != RETURN_OK && Result != RESULT_UserAborted)
			{
			d1(kprintf("%s/%s/%ld: Weird error!\n", __FILE__, __FUNC__, __LINE__));
			reqResult = ReportError(cl, o, &Result, MSGID_MOVEERRORNAME, MSGID_COPYERRORGNAME);
			}

		} while (RETURN_OK == Result && COPYERR_Retry == reqResult);

	ftci->ftci_CurrentItems++;

	return Result;
}


static LONG RememberError(Class *cl, Object *o, CONST_STRPTR FileName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG ErrorCode = IoErr();

	d1(kprintf("%s/%s/%ld: ErrorCode=%ld  FileName=<%s>\n", __FILE__, __FUNC__, __LINE__, ErrorCode, FileName));

	if (RETURN_OK != ErrorCode)
		{
		if (NULL == ftci->ftci_LastErrorFileName)
			{
			ftci->ftci_LastErrorCode = ErrorCode;
			ftci->ftci_LastErrorFileName = AllocCopyString(FileName);
			}
		}

	return ErrorCode;
}


static void ClearError(Class *cl, Object *o)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);

	ftci->ftci_LastErrorCode = RETURN_OK;

	if (ftci->ftci_LastErrorFileName)
		{
		FreeCopyString(ftci->ftci_LastErrorFileName);
		ftci->ftci_LastErrorFileName = NULL;
		}
}


// just like RememberError() except that 
// ERROR_NO_MORE_ENTRIES get remapped to RETURN_OK
static LONG RememberExNextError(Class *cl, Object *o, CONST_STRPTR FileName)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	LONG ErrorCode = IoErr();

	d1(kprintf("%s/%s/%ld: ErrorCode=%ld  FileName=<%s>\n", __FILE__, __FUNC__, __LINE__, ErrorCode, FileName));

	if (ERROR_NO_MORE_ENTRIES == ErrorCode)
		ErrorCode = RETURN_OK;

	if (RETURN_OK != ErrorCode)
		{
		if (NULL == ftci->ftci_LastErrorFileName)
			{
			ftci->ftci_LastErrorCode = ErrorCode;
			ftci->ftci_LastErrorFileName = AllocCopyString(FileName);
			}
		}

	return ErrorCode;
}


static LONG HandleWriteProtectError(Class *cl, Object *o, BPTR DirLock,
	CONST_STRPTR FileName, enum WriteProtectedReqType Type, LONG ErrorCode)
{
	struct FileTransClassInstance *ftci = INST_DATA(cl, o);
	enum WriteProtectedReqResult existsResult = WRITEPROTREQ_Skip;
	ULONG ReqMsgId;

	d1(kprintf("%s/%s/%ld: ErrorCode=%ld  FileName=<%s>\n", __FILE__, __FUNC__, __LINE__, ErrorCode, FileName));

	switch (ftci->ftci_OverwriteMode)
		{
	case SCCV_WriteProtectedMode_Ask:
		switch (Type)
			{
		case WRITEPROTREQ_Move:
			ReqMsgId = MSGID_WRITEPROTECTED_MOVE;
			break;
		case WRITEPROTREQ_CopyIcon:
			ReqMsgId = MSGID_WRITEPROTECTED_COPYICON;
			break;
		case WRITEPROTREQ_Copy:
		default:
			ReqMsgId = MSGID_WRITEPROTECTED_COPY;
			break;
			}

		existsResult = DoMethod(o, SCCM_FileTrans_WriteProtectedRequest,
			WRITEPROTREQ_Move,
			DirLock, FileName,
			ftci->ftci_Window,
			ErrorCode,
			ReqMsgId, MSGID_WRITEPROTECTED_GNAME_NEW);
		
		if (WRITEPROTREQ_ReplaceAll == existsResult
			|| WRITEPROTREQ_SkipAll == existsResult)
			ftci->ftci_OverwriteMode = existsResult;
                break;

	case SCCV_WriteProtectedMode_Never:
		d1(kprintf("%s/%s/%ld: Result=%08lx\n", __FILE__, __FUNC__, __LINE__, RETURN_OK));
		return RETURN_OK;
		break;

	case SCCV_WriteProtectedMode_Always:
		existsResult = WRITEPROTREQ_ReplaceAll;
		break;
		}

	switch (existsResult)
		{
	case WRITEPROTREQ_ReplaceAll:
	case WRITEPROTREQ_Replace:
	case WRITEPROTREQ_SkipAll:
	case WRITEPROTREQ_Skip:
		ClearError(cl, o);
		break;
	default:
		break;
		}

	return existsResult;
}


