/*
** $Date$
** $Revision$
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code that is specific to windows.
*/

#if SQLITE_OS_OTHER               /* This file is used for AmigaDOS only */

#include <ctype.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <scalos/scalos.h>
#ifdef __amigaos4__
#define Flush(f) FFlush(f)
#endif /* __amigaos4__ */
#include <proto/timer.h>

#undef GLOBAL

#include "os_other.h"
#include "sqliteInt.h"
#include "os.h"

//#define SQLITE_DEBUG

#include "sqlite3_base.h"
#include "LibSQLite3.h"

#if defined(SQLITE_DEBUG)
#undef  d2
#define d1(x)	          x
#endif /* SQLITE_DEBUG */

//---------------------------------------------------------------------

/*
** Include code that is common to all os_*.c files
*/
#include "os_common.h"

//---------------------------------------------------------------------

struct LockKey
	{
	LONG lk_LockCount;
	struct SignalSemaphore lk_Sema;
	};

struct LockInfo
	{
	struct Node li_Node;

	LONG li_UseCount;
	ULONG li_Hash;

	struct LockKey li_Shared;
	struct LockKey li_Pending;
	struct LockKey li_Reserved;
	};

//---------------------------------------------------------------------

/*
** The otherFile structure is a subclass of sqlite3_file specific to the win32
** portability layer.
** This is the definition for AmigaOS
*/
typedef struct otherFile otherFile;
struct otherFile
{
	sqlite3_io_methods const *pMethod;	  /* Must be first */
	BPTR  h;               		/* Handle for accessing the file */
	unsigned char locktype; 	/* Type of lock currently held on this file */
	BOOL DeleteOnClose;
	BOOL ReadOnly;

	LONG lastErrno;

	ULONG FileNameHash;

#ifndef NDEBUG
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
	unsigned char transCntrChng;   /* True if the transaction counter changed */
	unsigned char dbUpdate;        /* True if any part of database file changed */
	unsigned char inNormalWrite;   /* True if in a normal write operation */
#endif
#ifdef SQLITE_TEST
  /* In test mode, increase the size of this structure a bit so that
  ** it is larger than the struct CrashFile defined in test6.c.
  */
  char aPadding[32];
#endif
};
//---------------------------------------------------------------------

/*
** Do not include any of the File I/O interface procedures if the
** SQLITE_OMIT_DISKIO macro is defined (indicating that there database
** will be in-memory only)
*/
#ifndef SQLITE_OMIT_DISKIO

/*
** Macros used to determine whether or not to use threads.
*/
#if defined(THREADSAFE) && THREADSAFE
# define SQLITE_OTHER_THREADS 1
#endif

//---------------------------------------------------------------------

extern sqlite3_mutex_methods *sqlite3OtherMutex(void);

//---------------------------------------------------------------------

static void otherEnterMutex(void);
static void otherLeaveMutex(void);
static int otherCheckWriteable(const char *zFilename);
static int otherDelete(sqlite3_vfs *pVfs, const char *zFilename, int syncDir);
static int otherAccess(sqlite3_vfs *pVfs, const char *zFilename, int flags, int *pResOut);
static int otherOpen(sqlite3_vfs *pVfs, const char *zName, sqlite3_file *id, int flags, int *pOutFlags);
static int otherTempFileName(sqlite3_vfs *pVfs, int nBuf, char *zBuf);
static int otherClose(sqlite3_file *pId);
static int otherRead(sqlite3_file *id, void *pBuf, int amt, sqlite3_int64 offset);
static int otherWrite(sqlite3_file *id, const void *pBuf, int amt, sqlite3_int64 offset);
static int otherSeek(sqlite3_file *id, sqlite3_int64 offset);
static int otherSync(sqlite3_file *id, int dataOnly);
static int otherTruncate(sqlite3_file *id, sqlite3_int64 nByte);
static int otherFileSize(sqlite3_file *id, sqlite3_int64 *pSize);
static int otherDeviceCharacteristics(sqlite3_file *id);
#if defined(SQLITE_DEBUG)
static const char *locktypeName(int locktype);
#endif /* SQLITE_DEBUG */
static int otherLock(sqlite3_file *id, int locktype);
static int otherCheckReservedLock(sqlite3_file *id, int *pResOut);
static int otherUnlock(sqlite3_file *id, int locktype);

static int otherFullPathname(sqlite3_vfs *pVfs, const char *zRelative, int nFull, char *zFull);
static void *otherDlOpen(sqlite3_vfs *pVfs, const char *zFilename);
static void otherDlError(sqlite3_vfs *pVfs, int nBuf, char *zBufOut);
static void otherDlClose(sqlite3_vfs *pVfs, void *pHandle);
static int otherRandomness(sqlite3_vfs *pVfs, int nBuf, char *zBuf);
static int otherSleep(sqlite3_vfs *pVfs, int microsec);
static int otherCurrentTime(sqlite3_vfs *pVfs, double *prNow);

static __inline__ ULONG hashadd(ULONG h, UBYTE c);
static ULONG hash_nocase(CONST_STRPTR s);
static BOOL NewLockInfo(otherFile *id, const char *zFilename);
static void DisposeLockInfo(otherFile *id);
static struct LockInfo *FindLockInfo(ULONG Hash);
static struct LockInfo *FindLockInfoProtected(ULONG Hash);
static int otherFileControl(sqlite3_file *id, int op, void *pArg);
static int otherSectorSize(sqlite3_file *id);
static int allocateOtherFile(otherFile *pNew, const char *zFilename);
static void InitOtherKey(struct LockKey *lk);
static LONG AttemptLockOtherKey(struct LockKey *lk);
static LONG AttemptLockOtherKeyShared(struct LockKey *lk);
static void UnlockOtherKey(struct LockKey *lk);
static int otherGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf);

//---------------------------------------------------------------------

static struct List LockInfoList;
static struct SignalSemaphore LockInfoListSema;
static BOOL LockInfoInit = FALSE;

//---------------------------------------------------------------------

/*
** Helper functions to obtain and relinquish the global mutex.
*/
static void otherEnterMutex(void)
{
	sqlite3_mutex_enter(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER));
}

static void otherLeaveMutex(void)
{
	sqlite3_mutex_leave(sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER));
}

//---------------------------------------------------------------------

static int otherCheckWriteable(const char *zFilename)
{
	int isWriteable = 0;
	BPTR fLock;
	struct FileInfoBlock *fib = NULL;
	struct InfoData *id = NULL;

	d1(KPrintF(__FILE__ "/%s/%ld: zFilename=<%s>\n", __FUNC__, __LINE__, zFilename));

	do	{
		fLock = Lock(zFilename, ACCESS_READ);
		d1(KPrintF(__FILE__ "/%s/%ld: fLock=%08lx\n", __FUNC__, __LINE__, fLock));
		if (0 == fLock)
			break;

		id = sqlite3_malloc(sizeof(struct InfoData));
		d1(KPrintF(__FILE__ "/%s/%ld: id=%08lx\n", __FUNC__, __LINE__, id));
		if (NULL == id)
			break;

		fib = AllocDosObject(DOS_FIB, NULL);
		d1(KPrintF(__FILE__ "/%s/%ld: fib=%08lx\n", __FUNC__, __LINE__, fib));
		if (NULL == fib)
			break;

		if (!Examine(fLock, fib))
			break;

		d1(KPrintF(__FILE__ "/%s/%ld: Examine() succeeded, fib_Protection=%08lx\n", __FUNC__, __LINE__, fib->fib_Protection));

		if (fib->fib_Protection & FIBF_OTR_WRITE)
			break;		// read-only!

		// Object seems writebale, but might be on read-only disk
		if (!Info(fLock, id))
			break;

		d1(KPrintF(__FILE__ "/%s/%ld: Info() succeeded,  id_DiskState=%ld\n", __FUNC__, __LINE__, id->id_DiskState));

		if (ID_VALIDATED != id->id_DiskState)
			break;		// not validated or read-only

		isWriteable = 1;
		} while (0);

	if (fib)
		FreeDosObject(DOS_FIB, fib);
	if (id)
		sqlite3_free(id);
	if (fLock)
		UnLock(fLock);

	d1(KPrintF(__FILE__ "/%s/%ld: isWriteable=%ld\n", __FUNC__, __LINE__, isWriteable));

	return isWriteable;
}

//---------------------------------------------------------------------

/*
** Delete the named file
*/
static int otherDelete(sqlite3_vfs *pVfs, const char *zFilename, int syncDir)
{
	(void) syncDir;

	d1(KPrintF(__FILE__ "/%s/%ld: zFilename=<%s>  syncDir=%ld\n", __FUNC__, __LINE__, zFilename, syncDir));
	DeleteFile(zFilename);
	OSTRACE2("DELETE \"%s\"\n", zFilename);
        return SQLITE_OK;
}

/*
** Test the existance of or access permissions of file zPath. The
** test performed depends on the value of flags:
**
**     SQLITE_ACCESS_EXISTS: Return 1 if the file exists
**     SQLITE_ACCESS_READWRITE: Return 1 if the file is read and writable.
**     SQLITE_ACCESS_READONLY: Return 1 if the file is readable.
**
** Otherwise return 0.
*/
static int otherAccess(sqlite3_vfs *pVfs, const char *zFilename, int flags, int *pResOut)
{
	BPTR fLock = NULL;
	int exists = 0;

	d1(KPrintF(__FILE__ "/%s/%ld: zFilename=<%s>  flags=%08lx\n", __FUNC__, __LINE__, zFilename, flags));

	switch( flags )
		{
	case SQLITE_ACCESS_READ:
	case SQLITE_ACCESS_EXISTS:
		fLock = Lock(zFilename, ACCESS_READ);
		break;
	case SQLITE_ACCESS_READWRITE:
		exists = otherCheckWriteable(zFilename);
		break;
	default:
		assert(!"Invalid flags argument");
		break;
		}

	if (fLock)
		{
		exists = 1;
		UnLock(fLock);
		}
	else
		{
		d1(KPrintF(__FILE__ "/%s/%ld: Lock(9 failed, IoErr=%ld\n", __FUNC__, __LINE__, IoErr()));
		}

	d1(KPrintF(__FILE__ "/%s/%ld: exists=%ld\n", __FUNC__, __LINE__, exists));

	*pResOut = exists;

	return SQLITE_OK;
}

/*
** Open the file zPath.
**
** Previously, the SQLite OS layer used three functions in place of this
** one:
**
**     sqlite3OsOpenReadWrite();
**     sqlite3OsOpenReadOnly();
**     sqlite3OsOpenExclusive();
**
** These calls correspond to the following combinations of flags:
**
**     ReadWrite() ->     (READWRITE | CREATE)
**     ReadOnly()  ->     (READONLY)
**     OpenExclusive() -> (READWRITE | CREATE | EXCLUSIVE)
**
** The old OpenExclusive() accepted a boolean argument - "delFlag". If
** true, the file was configured to be automatically deleted when the
** file handle closed. To achieve the same effect using this new
** interface, add the DELETEONCLOSE flag to those specified above for
** OpenExclusive().
*/
static int otherOpen(sqlite3_vfs *pVfs, const char *zPath, sqlite3_file *id, int flags, int *pOutFlags)
{
	otherFile *pFile = (otherFile*) id;
	int isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
	int isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
	int isCreate     = (flags & SQLITE_OPEN_CREATE);
	int isReadonly   = (flags & SQLITE_OPEN_READONLY);
	int isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
	char zTmpname[SQLITE_TEMPNAME_SIZE];
	const char *zName = zPath;

	d1(KPrintF(__FILE__ "/%s/%ld: zPath=<%s>  flags=%08lx\n", __FUNC__, __LINE__, zPath, flags));

	/* If argument zPath is a NULL pointer, this function is required to open
	** a temporary file. Use this buffer to store the file name in.
	*/

	/* Check the following statements are true:
	**
	**   (a) Exactly one of the READWRITE and READONLY flags must be set, and
	**   (b) if CREATE is set, then READWRITE must also be set, and
	**   (c) if EXCLUSIVE is set, then CREATE must also be set.
	**   (d) if DELETEONCLOSE is set, then CREATE must also be set.
	*/
	assert((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
	assert(isCreate==0 || isReadWrite);
	assert(isExclusive==0 || isCreate);
	assert(isDelete==0 || isCreate);


	/* The main DB, main journal, and master journal are never automatically
	** deleted
	*/
	assert( eType!=SQLITE_OPEN_MAIN_DB || !isDelete );
	assert( eType!=SQLITE_OPEN_MAIN_JOURNAL || !isDelete );
	assert( eType!=SQLITE_OPEN_MASTER_JOURNAL || !isDelete );

	/* Assert that the upper layer has set one of the "file-type" flags. */
	assert( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
	     || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
	     || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_MASTER_JOURNAL
	     || eType==SQLITE_OPEN_TRANSIENT_DB
	);

	(void) isExclusive;
	(void) isCreate;

	memset(pFile, 0, sizeof(otherFile));

	if( !zName )
		{
		int rc;

		assert(isDelete && !isOpenDirectory);
		rc = otherTempFileName(pVfs, sizeof(zTmpname), zTmpname);
		if( rc != SQLITE_OK )
			{
			return rc;
			}
		zName = zTmpname;
		}

	if (isReadonly)
		pFile->h = Open(zName, MODE_OLDFILE);
	if (isReadWrite)
		pFile->h = Open(zName, MODE_READWRITE);

	d1(KPrintF(__FILE__ "/%s/%ld: h=%08lx\n", __FUNC__, __LINE__, pFile->h));

	pFile->ReadOnly = isReadonly;
	pFile->DeleteOnClose = isDelete;

	if(0 == pFile->h)
		{
		if (isReadWrite)
			{
			return otherOpen(NULL, zName, id,
				((flags|SQLITE_OPEN_READONLY)&~SQLITE_OPEN_READWRITE),
                                pOutFlags);
			}
		else
			{
			return SQLITE_CANTOPEN;
			}
		}

	if( pOutFlags )
		{
		if( flags & SQLITE_OPEN_READWRITE )
			{
			*pOutFlags = SQLITE_OPEN_READWRITE;
			}
		else
			{
			*pOutFlags = SQLITE_OPEN_READONLY;
			}
		d1(KPrintF(__FILE__ "/%s/%ld: *pOutFlags=%08lx\n", __FUNC__, __LINE__, *pOutFlags));
		}

	d1(KPrintF(__FILE__ "/%s/%ld: id=%08lx  h=%08lx\n", __FUNC__, __LINE__, id, pFile->h));

	return allocateOtherFile(pFile, zName);
}

/*
** Create a temporary file name in zBuf.
*/
static int otherTempFileName(sqlite3_vfs *pVfs, int nBuf, char *zBuf)
{
	static const char zChars[] =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789";
	int i, j;
	char zTempPath[SQLITE_TEMPNAME_SIZE];
	int exists = 0;

	if (sqlite3_temp_directory )
		{
		strncpy(zTempPath, sqlite3_temp_directory, SQLITE_TEMPNAME_SIZE-30);
		zTempPath[SQLITE_TEMPNAME_SIZE-30] = 0;

		for(i=strlen(zTempPath); i>0 && zTempPath[i-1]=='/'; i--)
			{
			}
		zTempPath[i] = 0;
		}
	else
		{
		memset(zTempPath, 0, sizeof(zTempPath));
		}

	do	{
		sqlite3_snprintf(nBuf, zBuf, "%s%s", STR(TEMP_FILE_PREFIX), zTempPath);

		j = strlen(zBuf);
		sqlite3_randomness(15, &zBuf[j]);
		for(i=0; i<15; i++, j++)
			{
			zBuf[j] = (char)zChars[ ((unsigned char)zBuf[j])%(sizeof(zChars)-1) ];
			}
		zBuf[j] = 0;
		sqlite3OsAccess(pVfs, zBuf, SQLITE_ACCESS_EXISTS, &exists);
		} while (exists);

	OSTRACE2("TEMP FILENAME: %s\n", zBuf);
	d1(KPrintF(__FILE__ "/%s/%ld: zBuf=<%s>\n", __FUNC__, __LINE__, zBuf));
	return SQLITE_OK;
}

/*
** Close a file.
*/
static int otherClose(sqlite3_file *pId)
{
	STRPTR Buffer = NULL;
	otherFile *pFile = NULL;

	do	{
		d1(KPrintF(__FILE__ "/%s/%ld: pId=%08lx\n", __FUNC__, __LINE__, pId));

		if (NULL == pId)
			break;

		pFile = (otherFile *) pId;
		if (NULL == pFile)
			break;

		d1(KPrintF(__FILE__ "/%s/%ld: \n", __FUNC__, __LINE__));
		OSTRACE2("CLOSE %ld\n", pFile->h);

		if (NULL == pFile->h)
			break;

		do	{
			// try to delete files that have been opened with "delFlag" set
			const LONG MaxLen = 1024;

			if (!pFile->DeleteOnClose)
				break;

			Buffer = sqlite3_malloc(MaxLen);
			if (NULL == Buffer)
				break;;

			if (!NameFromFH(pFile->h, Buffer, MaxLen))
				break;

			// close file before trying to delete it!
			if ( !Close(pFile->h) )
				pFile->lastErrno = IoErr();

			pFile->h = 0;

			DeleteFile(Buffer);
			} while (0);

		if (pFile->h)
			{
			if ( !Close(pFile->h) )
				pFile->lastErrno = IoErr();
			pFile->h = 0;
			}

		OpenCounter(-1);
		} while (0);

	if (Buffer)
		{
		d1(KPrintF(__FILE__ "/%s/%ld: Buffer=%08lx\n", __FUNC__, __LINE__, Buffer));
		sqlite3_free(Buffer);
		}
	if (pFile)
		DisposeLockInfo(pFile);

        return SQLITE_OK;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static int otherRead(sqlite3_file *id, void *pBuf, int amt, sqlite3_int64 offset)
{
	otherFile *pFile = (otherFile *) id;
	LONG got;
	int rc;

	d1(KPrintF(__FILE__ "/%s/%ld: amt=%ld\n", __FUNC__, __LINE__, amt));
	SimulateIOError(SQLITE_IOERR);
	OSTRACE3("READ %08lx lock=%s\n", pFile->h, locktypeName(pFile->locktype));

	rc = otherSeek(id, offset);
	if (SQLITE_OK != rc)
		{
		d1(KPrintF(__FILE__ "/%s/%ld: Read seek error=%ld\n", __FUNC__, __LINE__, rc));
		return rc;
		}

	got = FRead(pFile->h, pBuf, 1, amt);
	d1(KPrintF(__FILE__ "/%s/%ld: got=%ld\n", __FUNC__, __LINE__, got));
	if (got == amt )
		return SQLITE_OK;
	else if (got < 0)
		{
		pFile->lastErrno = IoErr();
		return SQLITE_IOERR;
		}
	else
		{
		d1(KPrintF(__FILE__ "/%s/%ld: SQLITE_IOERR_SHORT_READ\n", __FUNC__, __LINE__));
		pFile->lastErrno = IoErr();
		memset(&((char*)pBuf)[got], 0, amt-got);
		return SQLITE_IOERR_SHORT_READ;
		}
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static int otherWrite(sqlite3_file *id, const void *pBuf, int amt, sqlite3_int64 offset)
{
	otherFile *pFile = (otherFile *) id;
	LONG  wrote;
	int rc;

	d1(KPrintF(__FILE__ "/%s/%ld: amt=%ld\n", __FUNC__, __LINE__, amt));
	SimulateIOError(( wrote=(-1), amt=1 ));
	SimulateDiskfullError(( wrote=0, amt=1 ));
	OSTRACE3("WRITE %ld lock=%ld\n", pFile->h, pFile->locktype);
	assert( amt>0 );

#ifndef NDEBUG
	/* If we are doing a normal write to a database file (as opposed to
	** doing a hot-journal rollback or a write to some file other than a
	** normal database file) then record the fact that the database
	** has changed.  If the transaction counter is modified, record that
	** fact too.
	*/
	if( pFile->inNormalWrite )
		{
		pFile->dbUpdate = 1;  /* The database has been modified */
		if( offset<=24 && offset+amt>=27 )
			{
			char oldCntr[4];
			SimulateIOErrorBenign(1);
			seekAndRead(pFile, 24, oldCntr, 4);
			SimulateIOErrorBenign(0);
			if( memcmp(oldCntr, &((char*)pBuf)[24-offset], 4)!=0 )
				{
				pFile->transCntrChng = 1;  /* The transaction counter has changed */
				}
			}
		}
#endif

	rc = otherSeek(id, offset);
	if (SQLITE_OK != rc)
		{
		d1(KPrintF(__FILE__ "/%s/%ld: Write seek error=%ld\n", __FUNC__, __LINE__, rc));
		return rc;
		}

	wrote = FWrite(pFile->h, (APTR) pBuf, 1, amt);
	d1(KPrintF(__FILE__ "/%s/%ld: wrote=%ld\n", __FUNC__, __LINE__, wrote));
	if (wrote < 0)
		{
		pFile->lastErrno = IoErr();
		d1(KPrintF(__FILE__ "/%s/%ld: SQLITE_IOERR_WRITE\n", __FUNC__, __LINE__));
		return SQLITE_IOERR_WRITE;
		}
	else if (wrote != amt)
		{
		pFile->lastErrno = IoErr();
		d1(KPrintF(__FILE__ "/%s/%ld: SQLITE_FULL\n", __FUNC__, __LINE__));
		return SQLITE_FULL;
		}

	return SQLITE_OK;
}

/*
** Move the read/write pointer in a file.
*/
static int otherSeek(sqlite3_file *id, sqlite3_int64 offset)
{
	otherFile *pFile = (otherFile *) id;
//	LONG upperBits = offset>>32;
	LONG lowerBits = offset & 0xffffffff;
	LONG rc;
	LONG Pos, OldPos;

	d1(KPrintF(__FILE__ "/%s/%ld: offset=%ld\n", __FUNC__, __LINE__, lowerBits));
#ifdef SQLITE_TEST
	if (offset )
		SimulateDiskfullError
#endif
	rc = Seek(pFile->h, lowerBits, OFFSET_BEGINNING);
	OSTRACE3("SEEK %ld %lld\n", pFile->h, offset);
	if (rc >= 0)
		return SQLITE_OK;

	pFile->lastErrno = IoErr();

	OldPos = Seek(pFile->h, 0, OFFSET_END);
	Pos = Seek(pFile->h, 0, OFFSET_END);
	d1(KPrintF(__FILE__ "/%s/%ld: Pos=%ld\n", __FUNC__, __LINE__, Pos));
	if (Pos < lowerBits)
		{
		rc = SetFileSize(pFile->h, lowerBits, OFFSET_BEGINNING);
		if (rc < 0 )
			{
			pFile->lastErrno = IoErr();
			d1(KPrintF(__FILE__ "/%s/%ld: rc=%ld  IoErr()=%ld\n", __FUNC__, __LINE__, rc, IoErr()));
			return SQLITE_FULL;
			}

		rc = Seek(pFile->h, lowerBits, OFFSET_BEGINNING);
		if (rc < 0 )
			{
			pFile->lastErrno = IoErr();
			d1(KPrintF(__FILE__ "/%s/%ld: rc=%ld  IoErr()=%ld\n", __FUNC__, __LINE__, rc, IoErr()));
			return SQLITE_FULL;
			}
		return SQLITE_OK;
		}
	return SQLITE_FULL;
}

/*
** Make sure all writes to a particular file are committed to disk.
*/
static int otherSync(sqlite3_file *id, int dataOnly)
{
	otherFile *pFile = (otherFile *) id;

	d1(KPrintF(__FILE__ "/%s/%ld: \n", __FUNC__, __LINE__));
	OSTRACE3("SYNC %ld lock=%ld\n", pFile->h, pFile->locktype);
	if (Flush(pFile->h) )
		return SQLITE_OK;
	else
		{
		pFile->lastErrno = IoErr();
		return SQLITE_IOERR;
		}
}

/*
** Sync the directory zDirname. This is a no-op on operating systems other
** than UNIX.
*/
int sqlite3OtherSyncDirectory(const char *zDirname)
{
	SimulateIOError(SQLITE_IOERR);
	return SQLITE_OK;
}

/*
** Truncate an open file to a specified size
*/
static int otherTruncate(sqlite3_file *id, sqlite3_int64 nByte)
{
	otherFile *pFile = (otherFile *) id;
//	LONG upperBits = nByte>>32;
	LONG lowerBits = nByte & 0xffffffff;

	d1(KPrintF(__FILE__ "/%s/%ld: nByte=%ld\n", __FUNC__, __LINE__, lowerBits));
	OSTRACE3("TRUNCATE %ld %lld\n", pFile->h, nByte);
	SimulateIOError(SQLITE_IOERR);
	if (SetFileSize(pFile->h, lowerBits, OFFSET_BEGINNING) < 0)
		{
		pFile->lastErrno = IoErr();
		}
	return SQLITE_OK;
}

/*
** Determine the current size of a file in bytes
*/
static int otherFileSize(sqlite3_file *id, sqlite3_int64 *pSize)
{
	otherFile *pFile = (otherFile *) id;
	LONG OldPos;

	SimulateIOError(SQLITE_IOERR);

	OldPos = Seek(pFile->h, 0, OFFSET_END);
	*pSize = Seek(pFile->h, OldPos, OFFSET_BEGINNING);

	return SQLITE_OK;
}


#ifdef SQLITE_DEBUG
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string represetation of the supplied
** integer lock-type.
*/
static const char *locktypeName(int locktype)
{
	switch( locktype )
	{
	case NO_LOCK: 
		return "NONE";
	case SHARED_LOCK: 
		return "SHARED";
	case RESERVED_LOCK: 
		return "RESERVED";
	case PENDING_LOCK: 
		return "PENDING";
	case EXCLUSIVE_LOCK: 
		return "EXCLUSIVE";
	default:
		return "ERROR";
	}
}
#endif


/*
** Lock the file with the lock specified by parameter locktype - one
** of the following:
**
**     (1) SHARED_LOCK
**     (2) RESERVED_LOCK
**     (3) PENDING_LOCK
**     (4) EXCLUSIVE_LOCK
**
** Sometimes when requesting one lock state, additional lock states
** are inserted in between.  The locking might fail on one of the later
** transitions leaving the lock state different from what it started but
** still short of its goal.  The following chart shows the allowed
** transitions and the inserted intermediate states:
**
**    UNLOCKED -> SHARED
**    SHARED -> RESERVED
**    SHARED -> (PENDING) -> EXCLUSIVE
**    RESERVED -> (PENDING) -> EXCLUSIVE
**    PENDING -> EXCLUSIVE
**
** This routine will only increase a lock.  The sqlite3OsUnlock() routine
** erases all locks at once and returns us immediately to locking level 0.
** It is not possible to lower the locking level one step at a time.  You
** must go straight to locking level 0.
*/
static int otherLock(sqlite3_file *id, int locktype)
{
	/* The following describes the implementation of the various locks and
	** lock transitions in terms of the POSIX advisory shared and exclusive
	** lock primitives (called read-locks and write-locks below, to avoid
	** confusion with SQLite lock names). The algorithms are complicated
	** slightly in order to be compatible with windows systems simultaneously
	** accessing the same database file, in case that is ever required.
	**
	** To obtain a SHARED lock, a shared lock is obtained on the PendingSema
	** If this is successful, a shared lock on SharedSema is obtained
	** and the lock on the PendingSema released.
	**
	** A process may only obtain a RESERVED lock after it has a SHARED lock.
	** A RESERVED lock is implemented by grabbing an exclusive lock on the
	** ReservedSema. 
	**
	** A process may only obtain a PENDING lock after it has obtained a
	** SHARED lock. A PENDING lock is implemented by obtaining an exclusive 
	** lock on the PendingSema. This ensures that no new SHARED locks can be
	** obtained, but existing SHARED locks are allowed to persist. A process
	** does not have to obtain a RESERVED lock on the way to a PENDING lock.
	** This property is used by the algorithm for rolling back a journal file
	** after a crash.
	**
	** An EXCLUSIVE lock, obtained after a PENDING lock is held, is
	** implemented by obtaining an exclusive lock on the SharedSema.
	** Since all other locks require a shared lock on one of the SharedSema,
	** this ensures that no other locks are held on the database. 
	**
	*/
	int rc = SQLITE_OK;    	/* Return code from subroutines */
	BOOL res = TRUE;     	/* Result of a DOS lock call */
	otherFile *pFile = (otherFile *) id;
	struct LockInfo *pLock;

	assert( NULL != pFile );
	OSTRACE5("LOCK    %08lx %s was %s pid=%08lx\n", pFile->h,
		locktypeName(locktype), locktypeName(pFile->locktype),
		FindTask(NULL));

	/* If there is already a lock of this type or more restrictive on the
	** sqlite3_file, do nothing. Don't use the end_lock: exit path, as
	** otherEnterMutex() hasn't been called yet.
	*/
	if (pFile->locktype >= locktype )
		{
		OSTRACE3("LOCK    %08lx %s ok (already held)\n", pFile->h, locktypeName(locktype));
		return SQLITE_OK;
		}

	/* Make sure the locking sequence is correct
	*/
	assert( NO_LOCK != pFile->locktype || SHARED_LOCK == locktype );
	assert( locktype != PENDING_LOCK );
	assert( locktype != RESERVED_LOCK || SHARED_LOCK == pFile->locktype );

	pLock = FindLockInfoProtected(pFile->FileNameHash);
	d1(KPrintF(__FILE__ "/%s/%ld: pLock=%08lx\n", __FUNC__, __LINE__, pLock));
	if (NULL == pLock)
		return SQLITE_IOERR;

	/* This mutex is needed because pFile->pLock is shared across threads
	*/
	otherEnterMutex();

	/* A PENDING lock is needed before acquiring a SHARED lock and before
	** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
	** be released.
	*/
	if (SHARED_LOCK == locktype 
	    || (EXCLUSIVE_LOCK == locktype && pFile->locktype < PENDING_LOCK))
		{
		res = AttemptLockOtherKey(&pLock->li_Pending);
		d1(KPrintF(__FILE__ "/%s/%ld: Attempt PendingSema pLock=%08lx  res=%ld\n", __FUNC__, __LINE__, pLock, res));
		if ( !res )
			{
			rc = SQLITE_BUSY;
	    		goto end_lock;
	  		}
		}

	/* If control gets to this point, then actually go ahead and make
	** operating system calls for the specified lock.
	*/
	if (SHARED_LOCK == locktype)
		{
		assert( pLock->cnt==0 );
		assert( pLock->locktype==0 );

		/* Now get the shared lock on SharedSema */
		res = AttemptLockOtherKeyShared(&pLock->li_Shared);
		d1(KPrintF(__FILE__ "/%s/%ld: Attempt shared SharedSema pLock=%08lx  res=%ld\n", __FUNC__, __LINE__, pLock, res));

		/* Drop the temporary PENDING lock */
		d1(KPrintF(__FILE__ "/%s/%ld: Release PendingSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Pending));
		UnlockOtherKey(&pLock->li_Pending);
		if (!res )
			{
			rc = SQLITE_BUSY;
			}
		else
			{
			pFile->locktype = SHARED_LOCK;
			}
		}
	else
		{
		/* The request was for a RESERVED or EXCLUSIVE lock.  It is
		** assumed that there is a SHARED or greater lock on the file
		** already.
		*/
		assert( NO_LOCK != pFile->locktype );

		switch( locktype )
			{
		case RESERVED_LOCK:
			res = AttemptLockOtherKey(&pLock->li_Reserved);
			d1(KPrintF(__FILE__ "/%s/%ld: Attempt ReservedSema pLock=%08lx  res=%ld\n", __FUNC__, __LINE__, pLock, res));
			break;
		case EXCLUSIVE_LOCK:
			// Release the current shared lock on SharedSema
			// and try to get an exclusive lock
			UnlockOtherKey(&pLock->li_Shared);
			res = AttemptLockOtherKey(&pLock->li_Shared);
			d1(KPrintF(__FILE__ "/%s/%ld: Attempt SharedSema pLock=%08lx  res=%ld\n", __FUNC__, __LINE__, pLock, res));
			break;
		default:
			assert(0);
			}

		if (!res )
			{
			rc = SQLITE_BUSY;
			}
		}
	
#ifndef NDEBUG
  /* Set up the transaction-counter change checking flags when
  ** transitioning from a SHARED to a RESERVED lock.  The change
  ** from SHARED to RESERVED marks the beginning of a normal
  ** write operation (not a hot journal rollback).
  */
	if( SQLITE_OK == rc
		&& pFile->locktype <= SHARED_LOCK
		&& locktype == RESERVED_LOCK)
                {
		pFile->transCntrChng = 0;
		pFile->dbUpdate = 0;
		pFile->inNormalWrite = 1;
		}
#endif

	if (SQLITE_OK == rc)
		{
		pFile->locktype = locktype;
		}
	else if (EXCLUSIVE_LOCK == locktype)
		{
		pFile->locktype = PENDING_LOCK;
		}

end_lock:
	otherLeaveMutex();
	OSTRACE4("LOCK    %08lx %s %s\n", pFile->h, locktypeName(locktype),
	    rc == SQLITE_OK ? "ok" : "failed");

	return rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero.  If the file is unlocked or holds only SHARED locks, then
** return zero.
*/
static int otherCheckReservedLock(sqlite3_file *id, int *pResOut)
{
	int rc = 0;
	otherFile *pFile = (otherFile *) id;
	struct LockInfo *pLock;

	assert( NULL != pFile );
	pLock = FindLockInfoProtected(pFile->FileNameHash);
	d1(KPrintF(__FILE__ "/%s/%ld: pLock=%08lx\n", __FUNC__, __LINE__, pLock));
	if (NULL == pLock)
		return SQLITE_IOERR_CHECKRESERVEDLOCK;

	d1(KPrintF(__FILE__ "/%s/%ld: \n", __FUNC__, __LINE__));
	if (pFile->locktype >= RESERVED_LOCK )
		{
		rc = 1;
		OSTRACE3("TEST WR-LOCK %ld %ld (local)\n", pFile->h, rc);
		}
	else
		{
		rc = AttemptLockOtherKey(&pLock->li_Reserved);
		d1(KPrintF(__FILE__ "/%s/%ld: Attempt ReservedSema pLock=%08lx  rc=%ld\n", __FUNC__, __LINE__, pLock, rc));
		if (rc )
			{
			d1(KPrintF(__FILE__ "/%s/%ld: Release ReservedSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Reserved));
			UnlockOtherKey(&pLock->li_Reserved);
			}

		rc = !rc;
		OSTRACE3("TEST WR-LOCK %ld %ld (remote)\n", pFile->h, rc);
		}

	*pResOut = rc;

	return SQLITE_OK;
}

/*
** Lower the locking level on file descriptor id to locktype.  locktype
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
**
** It is not possible for this routine to fail if the second argument
** is NO_LOCK.  If the second argument is SHARED_LOCK then this routine
** might return SQLITE_IOERR;
*/
static int otherUnlock(sqlite3_file *id, int locktype)
{
	int rc = SQLITE_OK;
        struct LockInfo *pLock;
	otherFile *pFile = (otherFile *) id;

	assert( NULL != pFile );
	assert( locktype <= SHARED_LOCK );

	OSTRACE5("UNLOCK  %08lx %s was %s pid=%08lx\n", pFile->h,
		locktypeName(locktype),
		locktypeName(pFile->locktype), FindTask(NULL));

	if (pFile->locktype <= locktype )
		{
		return SQLITE_OK;
		}

	pLock = FindLockInfoProtected(pFile->FileNameHash);
	d1(KPrintF(__FILE__ "/%s/%ld: pLock=%08lx\n", __FUNC__, __LINE__, pLock));
	if (NULL == pLock)
		return SQLITE_IOERR;

	otherEnterMutex();

#ifndef NDEBUG
	/* When reducing a lock such that other processes can start
	** reading the database file again, make sure that the
	** transaction counter was updated if any part of the database
	** file changed.  If the transaction counter is not updated,
	** other connections to the same file might not realize that
	** the file has changed and hence might not know to flush their
	** cache.  The use of a stale cache can lead to database corruption.
	*/
	assert( pFile->inNormalWrite==0
	     || pFile->dbUpdate==0
	     || pFile->transCntrChng==1 );
	pFile->inNormalWrite = 0;
#endif

	if (pFile->locktype > SHARED_LOCK )
		{
		if (SHARED_LOCK == locktype )
			{
			d1(KPrintF(__FILE__ "/%s/%ld: Release SharedSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Shared));
			// Change exclusive lock on SharedSema to a shared one
			UnlockOtherKey(&pLock->li_Shared);

			d1(KPrintF(__FILE__ "/%s/%ld: Obtain SharedSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Shared));
			if (!AttemptLockOtherKeyShared(&pLock->li_Shared))
				rc = SQLITE_IOERR;  /* This should never happen */
			}
		d1(KPrintF(__FILE__ "/%s/%ld: Release PendingSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Pending));
		UnlockOtherKey(&pLock->li_Pending);
		d1(KPrintF(__FILE__ "/%s/%ld: Release ReservedSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Reserved));
		UnlockOtherKey(&pLock->li_Reserved);
		}
	if (locktype == NO_LOCK )
		{
		// Unlock shared lock on SharedSema
		d1(KPrintF(__FILE__ "/%s/%ld: Release SharedSema pLock=%08lx  Count=%ld\n", __FUNC__, __LINE__, pLock, pLock->li_Shared));
		UnlockOtherKey(&pLock->li_Shared);
		}

	otherLeaveMutex();
	pFile->locktype = locktype;

	d1(KPrintF(__FILE__ "/%s/%ld: rc=%ld  locktype=%s\n", __FUNC__, __LINE__, rc, locktypeName(pFile->locktype)));

	return rc;
}

/*
** Turn a relative pathname into a full pathname. The relative path
** is stored as a nul-terminated string in the buffer pointed to by
** zPath.
**
** zOut points to a buffer of at least sqlite3_vfs.mxPathname bytes
** (in this case, MAX_PATHNAME bytes). The full-path is written to
** this buffer before returning.
*/
static int otherFullPathname(sqlite3_vfs *pVfs, const char *zRelative, int nFull, char *zFull)
{
	BPTR pLock = 0;
	BOOL Success = FALSE;
	int Result = SQLITE_OK;

	d1(KPrintF(__FILE__ "/%s/%ld: START  zRelative=<%s>\n", __FUNC__, __LINE__, zRelative));

	do	{
		if (NULL == zFull)
			{
			Result = SQLITE_NOMEM;
			break;
			}

		pLock = Lock(zRelative, ACCESS_READ);
		if (0 == pLock)
			{
			sqlite3_snprintf(nFull, zFull, "%s", zRelative);
			//Result = SQLITE_CANTOPEN;
			break;
			}

		if (!NameFromLock(pLock, zFull, nFull))
			{
			Result = SQLITE_NOMEM;
			break;
			}

		Success = TRUE;
		} while (0);

	if (pLock)
		UnLock(pLock);
	if (!Success && zFull)
		{
		strcpy(zFull, zRelative);
		}

	d1(KPrintF(__FILE__ "/%s/%ld: END  Result=%ld\n", __FUNC__, __LINE__, Result));

	return Result;
}

#endif /* SQLITE_OMIT_DISKIO */


#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
static void *otherDlOpen(sqlite3_vfs *pVfs, const char *zFilename)
{
	return 0;
}


static void (*otherDlSym(sqlite3_vfs *NotUsed, void *p, const char*zSym))(void)
{
	return 0;
}

static void otherDlError(sqlite3_vfs *pVfs, int nBuf, char *zBufOut)
{
}

static void otherDlClose(sqlite3_vfs *pVfs, void *pHandle)
{
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Get information to seed the random number generator.  The seed
** is written into the buffer zBuf[256].  The calling function must
** supply a sufficiently large buffer.
*/
static int otherRandomness(sqlite3_vfs *pVfs, int nBuf, char *zBuf)
{
	/* We have to initialize zBuf to prevent valgrind from reporting
	** errors.  The reports issued by valgrind are incorrect - we would
	** prefer that the randomness be increased by making use of the
	** uninitialized space in zBuf - but valgrind errors tend to worry
	** some users.  Rather than argue, it seems easier just to initialize
	** the whole array and silence valgrind, even if that means less randomness
	** in the random seed.
	**
	** When testing, initializing zBuf[] to zero is all we do.  That means
	** that we always use the same random number sequence.* This makes the
	** tests repeatable.
	*/
	T_TIMEVAL tv;

	d1(KPrintF(__FILE__ "/%s/%ld: TimerBase=%08lx\n", __FUNC__, __LINE__, TimerBase));
	memset(zBuf, 0, 256);
	GetSysTime(&tv);
	d1(KPrintF(__FILE__ "/%s/%ld: \n", __FUNC__, __LINE__));
	sqlite3_snprintf(nBuf, zBuf, "%08lx-%08lx", tv.tv_secs, tv.tv_micro);
	d1(KPrintF(__FILE__ "/%s/%ld: zBuf=<%s>\n", __FUNC__, __LINE__, zBuf));
	return SQLITE_OK;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
static int otherSleep(sqlite3_vfs *pVfs, int microsec)
{
	(void) pVfs;

	Delay((microsec + 999) / (20 * 1000)); // 50ms per tick
	return ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, becomes the result
** returned from sqlite3OsCurrentTime().  This is used for testing.
*/
#ifdef SQLITE_TEST
int sqlite3_current_time = 0;
#endif

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static int otherCurrentTime(sqlite3_vfs *pVfs, double *prNow)
{
	double now;
	T_TIMEVAL tv;

	GetSysTime(&tv);

	now = ((double) tv.tv_secs) * 4294967296.0;
	*prNow = (now + tv.tv_micro)/864000000000.0 + 2305813.5;
#ifdef SQLITE_TEST
	if (sqlite3_current_time )
		{
		*prNow = sqlite3_current_time/86400.0 + 2440587.5;
		}
#endif
	return 0;
}


static __inline__ ULONG hashadd(ULONG h, UBYTE c)
{
	h += (h << 5); /* multiply by 33 */
	return (h ^ c);
}


// String hashing function (taken from Bernstein's cdb).
static ULONG hash_nocase(CONST_STRPTR s)
{
	ULONG h = 5381;
	UBYTE c;

	assert(s);

	while (c = *s++)
		{
		h = hashadd(h, toupper(c));
		}

	return (h);
}


static BOOL NewLockInfo(otherFile *id, const char *zFilename)
{
	struct LockInfo *pLock;

	d1(KPrintF(__FILE__ "/%s/%ld: START  id=%08lx  zFilename=<%s>\n", __FUNC__, __LINE__, id, zFilename));

	Forbid();
	if (!LockInfoInit)
		{
		NewList(&LockInfoList);
		InitSemaphore(&LockInfoListSema);
		LockInfoInit = TRUE;
		}
	Permit();

	id->FileNameHash = hash_nocase(zFilename);
	d1(KPrintF(__FILE__ "/%s/%ld: FileNameHash=%08lx\n", __FUNC__, __LINE__, id->FileNameHash));

	ObtainSemaphore(&LockInfoListSema);

	pLock = FindLockInfo(id->FileNameHash);
	d1(KPrintF(__FILE__ "/%s/%ld: pLock=%08lx\n", __FUNC__, __LINE__, pLock));

	if (pLock)
		{
		pLock->li_UseCount++;
		}
	else
		{
		pLock = sqlite3_malloc(sizeof(struct LockInfo));
		if (NULL == pLock)
			{
			ReleaseSemaphore(&LockInfoListSema);
			return FALSE;
			}

		pLock->li_Hash = id->FileNameHash;
		pLock->li_UseCount = 1;

		InitOtherKey(&pLock->li_Shared);
		InitOtherKey(&pLock->li_Pending);
		InitOtherKey(&pLock->li_Reserved);

		AddTail(&LockInfoList, &pLock->li_Node);
		}

	ReleaseSemaphore(&LockInfoListSema);

	d1(KPrintF(__FILE__ "/%s/%ld: END  pLock=%08lx\n", __FUNC__, __LINE__, pLock));

	return TRUE;
}


static void DisposeLockInfo(otherFile *id)
{
	struct LockInfo *pLock;

	d1(KPrintF(__FILE__ "/%s/%ld: START\n", __FUNC__, __LINE__));

	ObtainSemaphore(&LockInfoListSema);

	pLock = FindLockInfo(id->FileNameHash);
	d1(KPrintF(__FILE__ "/%s/%ld: pLock=%08lx\n", __FUNC__, __LINE__, pLock));
	if (pLock)
		{
		d1(KPrintF(__FILE__ "/%s/%ld: li_UseCount=%ld\n", __FUNC__, __LINE__, pLock->li_UseCount));

		if (0 == --pLock->li_UseCount)
			{
			Remove(&pLock->li_Node);
			d1(KPrintF(__FILE__ "/%s/%ld: \n", __FUNC__, __LINE__));
			sqlite3_free(pLock);
			}
		}

	ReleaseSemaphore(&LockInfoListSema);

	d1(KPrintF(__FILE__ "/%s/%ld: END\n", __FUNC__, __LINE__));
}


static struct LockInfo *FindLockInfo(ULONG Hash)
{
	struct LockInfo *pLock;

	for (pLock = (struct LockInfo *) LockInfoList.lh_Head;
		pLock != (struct LockInfo *) &LockInfoList.lh_Tail;
		pLock = (struct LockInfo *) pLock->li_Node.ln_Succ)
		{
		if (pLock->li_Hash == Hash)
			return pLock;
		}

	return NULL;
}


static struct LockInfo *FindLockInfoProtected(ULONG Hash)
{
	struct LockInfo *pLock;

	ObtainSemaphoreShared(&LockInfoListSema);
	pLock = FindLockInfo(Hash);
	ReleaseSemaphore(&LockInfoListSema);

	return pLock;
}


/*
** Control and query of the open file handle.
*/
static int otherFileControl(sqlite3_file *id, int op, void *pArg)
{
	otherFile *pFile = (otherFile *) id;

	switch( op )
		{
	case SQLITE_FCNTL_LOCKSTATE:
                {
		*(int*) pArg = pFile->locktype;
		return SQLITE_OK;
		}
	case SQLITE_LAST_ERRNO:
                {
		*(int*)pArg = pFile->lastErrno;
		return SQLITE_OK;
		}
#ifndef NDEBUG
	/* The pager calls this method to signal that it has done
	** a rollback and that the database is therefore unchanged and
	** it hence it is OK for the transaction change counter to be
	** unchanged.
	*/
	case SQLITE_FCNTL_DB_UNCHANGED:
		{
		pFile->dbUpdate = 0;
		return SQLITE_OK;
		}
#endif
	default:
		break;
		}
	return SQLITE_ERROR;
}

/*
** Return the sector size in bytes of the underlying block device for
** the specified file. This is almost always 512 bytes, but may be
** larger for some devices.
**
** SQLite code assumes this function cannot fail. It also assumes that
** if two files are created in the same file-system directory (i.e.
** a database and it's journal file) that the sector size will be the
** same for both.
*/
static int otherSectorSize(sqlite3_file *id)
{
	return SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return a vector of device characteristics.
*/
static int otherDeviceCharacteristics(sqlite3_file *id)
{
	return 0;
}


/*
** This vector defines all the methods that can operate on an sqlite3_file
** for win32.
*/
static const sqlite3_io_methods sqlite3OtherIoMethod =
{
	1,            	// iVersion
	otherClose,
	otherRead,
	otherWrite,
	otherTruncate,
	otherSync,
	otherFileSize,
	otherLock,
	otherUnlock,
	otherCheckReservedLock,
	otherFileControl,
	otherSectorSize,
	otherDeviceCharacteristics,
};

/*
** Allocate memory for an otherFile.  Initialize the new otherFile
** to the value given in pInit and return a pointer to the new
** sqlite3_file.  If we run out of memory, close the file and return NULL.
*/
static int allocateOtherFile(otherFile *pNew, const char *zFilename)
{
	if (!NewLockInfo(pNew, zFilename))
		{
		d1(KPrintF(__FILE__ "/%s/%ld: NewLockInfo failed\n", __FUNC__, __LINE__));
		return SQLITE_NOMEM;
		}
	pNew->pMethod = &sqlite3OtherIoMethod;
	OpenCounter(+1);
	return SQLITE_OK;
}


static void InitOtherKey(struct LockKey *lk)
{
	lk->lk_LockCount = 0;
	InitSemaphore(&lk->lk_Sema);
}


static LONG AttemptLockOtherKey(struct LockKey *lk)
{
	LONG Result;

	Result = AttemptSemaphore(&lk->lk_Sema);
	if (Result)
		lk->lk_LockCount++;
	else
		{
		ULONG delay;

		sqlite3_randomness(sizeof(delay), &delay);
		Delay(delay % 10);       // Semaphore is locked by another process - give way to allow other process to procede
		}

	return Result;
}


static LONG AttemptLockOtherKeyShared(struct LockKey *lk)
{
	LONG Result;

	Result = AttemptSemaphoreShared(&lk->lk_Sema);
	if (Result)
		lk->lk_LockCount++;
	else
		{
		ULONG delay;

		sqlite3_randomness(sizeof(delay), &delay);
		Delay(delay % 10);       // Semaphore is locked by another process - give way to allow other process to procede
		}

	return Result;
}


static void UnlockOtherKey(struct LockKey *lk)
{
	if (lk->lk_LockCount)
		{
		lk->lk_LockCount--;
		ReleaseSemaphore(&lk->lk_Sema);
		}
}


static int otherGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf)
{
	return 0;
}

/*
** Initialize the operating system interface.
*/
int sqlite3_os_init(void)
{
	int rc;
	static sqlite3_vfs otherVfs =
	{
	1,                 	/* iVersion */
	sizeof(otherFile),	/* szOsFile */
	512,          		/* mxPathname */
	0,                 	/* pNext */
	"amiga",           	/* zName */
	0,                 	/* pAppData */

	otherOpen,           	/* xOpen */
	otherDelete,         	/* xDelete */
	otherAccess,         	/* xAccess */
	otherFullPathname,   	/* xFullPathname */
	otherDlOpen,         	/* xDlOpen */
	otherDlError,        	/* xDlError */
	otherDlSym,          	/* xDlSym */
	otherDlClose,        	/* xDlClose */
	otherRandomness,     	/* xRandomness */
	otherSleep,		/* xSleep */
	otherCurrentTime,	/* xCurrentTime */
	otherGetLastError      	/* xGetLastError */
	};

	rc = sqlite3_config(SQLITE_CONFIG_MUTEX, sqlite3OtherMutex());
	d1(KPrintF(__FILE__ "/%s/%ld: SQLITE_CONFIG_MUTEX rc=%ld\n", __FUNC__, __LINE__, rc));
	if (SQLITE_OK != rc)
		return rc;

	rc = sqlite3_vfs_register(&otherVfs, 1);
	if (SQLITE_OK != rc)
		return rc;

	rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	d1(KPrintF(__FILE__ "/%s/%ld: SQLITE_CONFIG_SERIALIZED rc=%ld\n", __FUNC__, __LINE__, rc));
	if (SQLITE_OK != rc)
		return rc;

	rc = sqlite3_config(SQLITE_CONFIG_LOOKASIDE, 100, 500);
	d1(KPrintF(__FILE__ "/%s/%ld: SQLITE_CONFIG_LOOKASIDE rc=%ld\n", __FUNC__, __LINE__, rc));
	if (SQLITE_OK != rc)
		return rc;

        return SQLITE_OK;
}

/*
** Shutdown the operating system interface. This is a no-op for unix.
*/
int sqlite3_os_end(void)
{
	return SQLITE_OK;
}

#endif /* SQLITE_OS_OTHER */
