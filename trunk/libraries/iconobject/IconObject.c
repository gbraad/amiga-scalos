// IconObject.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/lists.h>
#include <exec/resident.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/stdio.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/iconobject.h>

#include <clib/alib_protos.h>
#include <proto/iffparse.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/dtclass.h>

#include <string.h>

#include "iconobject_base.h"
#include "iconnode.h"
#include "IconObject.h"
#include <defs.h>
#include <Year.h>


#define ICON_ID             MAKE_ID('i', 'c', 'o', 'n')
#define AMIGAICON_ID        MAKE_ID('a', 'i', 'c', 'o')



/*
 * macro to convert offsets in datatype header into
 * string pointers.
 */
#define MAKE_STRPTR( base, offset )  ( (STRPTR) ((ULONG)(base)) + ((ULONG)(offset)) )

//----------------------------------------------------------------------------

static struct IconNode *IconNode_New( STRPTR base_name, ULONG id, BYTE priority, struct IconObjectBase *IconObjectBase );
static VOID IconNode_Free( struct IconNode *node, struct IconObjectBase *IconObjectBase );
static ULONG HandleIconDTHD( struct DataTypeHeader *dthd, struct IconObjectBase *IconObjectBase );
static BOOL ParseDTDescriptor( STRPTR filename, struct IconObjectBase *IconObjectBase );
static BOOL ReadClassList( struct IconObjectBase *IconObjectBase );
static VOID FreeClassList( struct IconObjectBase *IconObjectBase );
static APTR MyAllocVecPooled(size_t Size);
static void MyFreeVecPooled(APTR mem);

static struct Iconobject *InternalConvert2IconObjectA(
	struct DiskObject *diskobject, const struct TagItem *TagList,
	struct IconObjectBase *IconObjectBase);

//----------------------------------------------------------------------------

struct ExecBase *SysBase;
struct IntuitionBase *IntuitionBase;
T_UTILITYBASE UtilityBase;
struct DosLibrary * DOSBase;
struct Library *IFFParseBase;
#ifdef __amigaos4__
struct Library *NewlibBase;
struct Interface *INewlib;
struct ExecIFace *IExec;
struct IntuitionIFace *IIntuition;
struct UtilityIFace *IUtility;
struct DOSIFace *IDOS;
struct IFFParseIFace *IIFFParse;
#endif

static void *MemPool;
static struct SignalSemaphore PubMemPoolSemaphore;

//----------------------------------------------------------------------------

char ALIGNED libName[] = "iconobject.library";
char ALIGNED libIdString[] = "$VER: iconobject.library "
	STR(LIB_VERSION) "." STR(LIB_REVISION) " (27 Dec 2007 13:57:02) "
	COMPILER_STRING
	" ©1999" CURRENTYEAR " The Scalos Team";

//----------------------------------------------------------------------------

BOOL IconObjectInit(struct IconObjectBase *IconObjectBase)
{
	/* store pointer to execbase for global access */

	d1(kprintf(__FILE__ "/%s/%ld:   IconObjectBase=%08lx  procName=<%s>\n", __FUNC__ , __LINE__, \
		IconObjectBase, FindTask(NULL)->tc_Node.ln_Name));

	InitSemaphore(&PubMemPoolSemaphore);
	InitSemaphore(&IconObjectBase->iob_Semaphore);
	NewList(&IconObjectBase->iob_ClassList);

	IFFParseBase = (APTR) OpenLibrary( "iffparse.library", 39 );
	DOSBase = (APTR) OpenLibrary( "dos.library", 39 );
	IntuitionBase = (APTR) OpenLibrary( "intuition.library", 39 );
	UtilityBase = (APTR) OpenLibrary( "utility.library", 39 );
#ifdef __amigaos4__
	if (IFFParseBase)
		{
		IIFFParse = (struct IFFParseIFace *)GetInterface(IFFParseBase, "main", 1, NULL);
		if (!IIFFParse)
			{
			CloseLibrary((struct Library *)IFFParseBase);
			IFFParseBase = NULL;
			}
		}
	if (DOSBase)
		{
		IDOS = (struct DOSIFace *)GetInterface((struct Library *)DOSBase, "main", 1, NULL);
		if (!IDOS)
			{
			CloseLibrary((struct Library *)DOSBase);
			DOSBase = NULL;
			}
		}
	if (IntuitionBase)
		{
		IIntuition = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase, "main", 1, NULL);
		if (!IIntuition)
			{
			CloseLibrary((struct Library *)IIntuition);
			IntuitionBase = NULL;
			}
		}
	if (UtilityBase)
		{
		IUtility = (struct UtilityIFace *)GetInterface(UtilityBase, "main", 1, NULL);
		if (!IUtility)
			{
			CloseLibrary((struct Library *)UtilityBase);
			UtilityBase = NULL;
			}
		}
	NewlibBase = OpenLibrary("newlib.library", 0);
	if (NULL == NewlibBase)
		return 0;
	INewlib = GetInterface(NewlibBase, "main", 1, NULL);
	if (NULL == INewlib)
		return 0;
#endif /* __amigaos4__ */

	MemPool = CreatePool(MEMPOOL_MEMFLAGS, MEMPOOL_PUDDLESIZE, MEMPOOL_THRESHSIZE);
	d1(kprintf("%s/%s/%ld: MemPool=%08lx\n", __FILE__, __FUNC__, __LINE__, MemPool));
	if (NULL == MemPool)
		return 0;

	if (IFFParseBase &&
		DOSBase &&
		IntuitionBase &&
		UtilityBase)
		{
		d1(kprintf("%s/%ld: Success\n", __FUNC__, __LINE__));
		return TRUE;	// Success
		}

	d1(kprintf("%s/%ld:  Fail\n", __FUNC__, __LINE__));

//	  IconObjectCleanup(IconObjectBase);

	return FALSE;	// Failure
}

//-----------------------------------------------------------------------------

BOOL IconObjectOpen(struct IconObjectBase *IconObjectBase)
{
	d1(kprintf(__FILE__ "/%s/%ld:   IconObjectBase=%08lx  procName=<%s>\n", __FUNC__ , __LINE__, \
		IconObjectBase, FindTask(NULL)->tc_Node.ln_Name));
	if (IsListEmpty(&IconObjectBase->iob_ClassList))
		{
		if (ReadClassList(IconObjectBase))
			{
			d1(kprintf(__FILE__ "/%s/%ld: Success\n", __FUNC__, __LINE__));
			return TRUE;	// Success
			}
		else
			{
			d1(kprintf(__FILE__ "/%s/%ld:  Fail\n", __FUNC__, __LINE__));
			return FALSE;	// Fail
			}
		}

	d1(kprintf(__FILE__ "/%s/%ld: Success\n", __FUNC__, __LINE__));

	return TRUE;	// Success
}

//-----------------------------------------------------------------------------

void IconObjectCleanup(struct IconObjectBase *IconObjectBase)
{
	d1(kprintf("%s/%ld:\n", __FUNC__, __LINE__));

	FreeClassList(IconObjectBase);

#ifdef __amigaos4__
	if (INewlib)
		{
		DropInterface(INewlib);
		INewlib = NULL;
		}
	if (NewlibBase)
		{
		CloseLibrary(NewlibBase);
		NewlibBase = NULL;
		}
#endif /* __amigaos4__ */
	if (IFFParseBase)
		{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IIFFParse);
#endif /* __amigaos4__ */
		CloseLibrary(IFFParseBase);
		IFFParseBase = NULL;
		}
	if (UtilityBase)
		{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IUtility);
#endif /* __amigaos4__ */
		CloseLibrary((struct Library *) UtilityBase);
		UtilityBase = NULL;
		}
	if (IntuitionBase)
		{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IIntuition);
#endif /* __amigaos4__ */
		CloseLibrary((struct Library *) IntuitionBase);
		IntuitionBase = NULL;
	       }
	if (DOSBase)
		{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IDOS);
#endif /* __amigaos4__ */
		CloseLibrary((struct Library *) DOSBase);
		DOSBase = NULL;
		}
	if (NULL != MemPool)
		{
		DeletePool(MemPool);
		MemPool = NULL;
		}
	d1(kprintf("%s/%ld: Done!\n", __FUNC__, __LINE__));
}

//-----------------------------------------------------------------------------

/*
 * IconNode_New()
 *
 * PURPOSE: Constructor
 * INPUTS:  base_name - base name of this iconobject datatype
 *		  id - IFF ID for this datatype
 *		  priority - used placement in datatypes list
 * RESULT:
 */
static struct IconNode *IconNode_New( STRPTR base_name, ULONG id, BYTE priority, struct IconObjectBase *IconObjectBase)
{
#define	dt_path		"datatypes/"
#define	dt_suffix	".datatype"
#define DT_NAME_LEN	( sizeof(dt_path) + sizeof(dt_suffix) )
#define	icon_suffix	".info"
#define ICON_SUFFIX_LEN	( sizeof(icon_suffix) )

	struct IconNode *node;

	d1(kprintf(__FILE__ "/%s/%ld:  name=<%s>  id=%08lx  pri=%ld\n",
		__FUNC__, __LINE__, base_name, id, (ULONG) priority));

	if( node = (struct IconNode *) MyAllocVecPooled(sizeof(struct IconNode)))
		{
		STRPTR name;

		memset(node, 0, sizeof(struct IconNode));

		/* make some space for the datatype library's pathname */
		if( node->in_Name = name = MyAllocVecPooled(DT_NAME_LEN + strlen( base_name )) )
			{
			/* make datatype's pathname from descriptor's basename */
			/* name = 'datatypes/' + base_name + '.datatype */
			strcpy( name, dt_path );
			strcat( name, base_name );
			strcat( name, dt_suffix );

			/* get suffix for icon filename */
			/* Stefan's code extracted this from the DT descriptor's match pattern */

			/* It seems unnecessary, so I've hard-wired this for now - with a view to removing it */
			node->in_Suffix	= (STRPTR) icon_suffix;
			node->in_SuffixLen = strlen(node->in_Suffix);

			/* fill in some other stuff */
			node->in_ID	   = id;
			node->in_Priority = priority;

			d1(kprintf("%s/%ld:  node=%08lx\n", __FUNC__, __LINE__, node));
			}
		else
			{
			/* allocation failed - cleanup */
			d1(kprintf("%s/%ld:\n", __FUNC__, __LINE__));
			MyFreeVecPooled(node);
			node = NULL;
			}
		}

	return node;
}

//-----------------------------------------------------------------------------

/*
 * IconNode_Free()
 *
 */

static VOID IconNode_Free( struct IconNode *node, struct IconObjectBase *IconObjectBase )
{
	d1(kprintf(__FILE__ "/%s/%ld:  node=%08lx\n", __FUNC__, __LINE__, node));
	if(node)
		{
		if( node->in_Name )
			MyFreeVecPooled(node->in_Name);
		MyFreeVecPooled(node);
		}
}

//-----------------------------------------------------------------------------

/*
 * HandleIconDTHD()
 *
 */

static ULONG HandleIconDTHD( struct DataTypeHeader *dthd, struct IconObjectBase *IconObjectBase )
{
	d1(kprintf(__FILE__ "/%s/%ld:  IconObjectBase=%08lx\n", __FUNC__, __LINE__, IconObjectBase));

	if( dthd->dth_GroupID == ICON_ID )
		{
		struct IconNode *node;

		if( node = IconNode_New( MAKE_STRPTR( dthd, dthd->dth_BaseName ), 
				dthd->dth_ID, dthd->dth_Priority, IconObjectBase) )
			{
			struct Library *DTClassBase;
#ifdef __amigaos4__
			struct DTClassIFace *IDTClass;
#endif /* __amigaos4__ */
			d1(kprintf("%s/%ld:  node=%08lx  basename=<%s>\n", __FUNC__, __LINE__, \
				node, MAKE_STRPTR(dthd, dthd->dth_BaseName)));

			if( dthd->dth_ID == AMIGAICON_ID )
				IconObjectBase->iob_AmigaIcon = node;

			d1(kprintf("%s/%ld:  name=<%s>\n", __FUNC__, __LINE__, node->in_Name));

			node->in_LibBase = DTClassBase = OpenLibrary( node->in_Name, 0L );
			if (node->in_LibBase != NULL)
				{
#ifdef __amigaos4__
				node->in_IFace = IDTClass = (struct DTClassIFace *)GetInterface(DTClassBase, "main", 1, NULL);
#endif /* __amigaos4__ */

				d1(kprintf("%s/%ld:  OpenLib OK\n", __FUNC__, __LINE__));

				node->in_Class = ObtainEngine();

				d1(kprintf("%s/%ld:  Class=%08lx  SysBase=%08lx\n", __FUNC__, __LINE__, node->in_Class, SysBase));
				
				Enqueue(&IconObjectBase->iob_ClassList, (struct Node *) node );

				d1(kprintf("%s/%ld:  after Enqueue\n", __FUNC__, __LINE__));
				}
			else
				{
				d1(kprintf("%s/%ld:  OpenLib FAIL\n", __FUNC__, __LINE__));
				IconNode_Free(node, IconObjectBase);
				}
			}
		}

	d1(kprintf("%s/%ld:  finish\n", __FUNC__, __LINE__));

	return 0L;
}

//-----------------------------------------------------------------------------

/*
 * ParseDTDescriptor()
 *
 */

static BOOL ParseDTDescriptor( STRPTR filename, struct IconObjectBase *IconObjectBase )
{
	struct IFFHandle *iff;
	LONG			  error;

	d1(kprintf(__FILE__ "/%s/%ld:  IconObjectBase=%08lx\n", __FUNC__, __LINE__, IconObjectBase));
	d1(kprintf(__FILE__ "/%s/%ld:  filename=<%s>\n", __FUNC__, __LINE__, filename));

	if( iff = AllocIFF() )
	{
		if( iff->iff_Stream = (ULONG) Open( filename, MODE_OLDFILE ) )
		{
			InitIFFasDOS( iff );

			if( !(error = OpenIFF( iff, IFFF_READ ) ) )
			{
				if( !(error = CollectionChunk( iff, ID_DTYP, ID_DTHD ) ) )
				{
					StopOnExit( iff, ID_DTYP, ID_DTHD );

					while(TRUE)
					{
						if( ( error = ParseIFF( iff, IFFPARSE_SCAN ) ) == IFFERR_EOC )
						{
							struct CollectionItem *ci;

							ci = FindCollection( iff, ID_DTYP, ID_DTHD );

							while( ci )
							{
								HandleIconDTHD( ci->ci_Data, IconObjectBase );
								ci = ci->ci_Next;
							}
						}
						else
							break;
					}
				}

				CloseIFF( iff );
			}
			Close( iff->iff_Stream );
		}
		FreeIFF( iff );
	}

	return TRUE;
}

//-----------------------------------------------------------------------------

/*
 * ReadClassList()
 *
 */

#define ICON_NAME_PAT		"#?.info"
#define ICON_NAME_PAT_LEN	7
#define BUF_SIZE		( ICON_NAME_PAT_LEN * 2 + 2 )
#define DATATYPES_PATH		"DEVS:Datatypes/"


static BOOL ReadClassList( struct IconObjectBase *IconObjectBase )
{
	char pattern_buf[ BUF_SIZE ];
	BOOL success = FALSE;
	struct List *list = &IconObjectBase->iob_ClassList;

	NewList(list);

	d1(kprintf(__FILE__ "/%s/%ld:\n", __FUNC__, __LINE__));

	if( ParsePatternNoCase( ICON_NAME_PAT, pattern_buf, BUF_SIZE ) >= 0L )
		{
		BPTR lock;

		if( lock = Lock( DATATYPES_PATH, SHARED_LOCK ) )
			{
			BPTR olddir = CurrentDir( lock );
			struct FileInfoBlock *fib;

			if( fib = (struct FileInfoBlock *) AllocDosObject( DOS_FIB, TAG_DONE ) )
				{
				if( Examine( lock, fib ) )
					{
					while( ExNext( lock, fib ) )
						{
						if( !MatchPatternNoCase( pattern_buf, fib->fib_FileName ) )
							ParseDTDescriptor( fib->fib_FileName, IconObjectBase );
						}
					if( IoErr() == ERROR_NO_MORE_ENTRIES )
						success = TRUE;
					}
				FreeDosObject( DOS_FIB, fib );
				}
			CurrentDir( olddir );

			UnLock( lock );
			}
		}

	d1(kprintf("%s/%ld:  success=%ld\n", __FUNC__, __LINE__, success));

	return success;
}

//-----------------------------------------------------------------------------

/*
 * FreeClassList()
 *
 */

static VOID FreeClassList( struct IconObjectBase *IconObjectBase )
{
	struct IconNode *n1;

	d1(kprintf(__FILE__ "/%s/%ld:\n", __FUNC__, __LINE__));

	while (n1 = (struct IconNode *) RemHead(&IconObjectBase->iob_ClassList))
		{
		if ( n1->in_LibBase )
			{
			//RemLibrary( n1->in_LibBase );
#ifdef __amigaos4__
			DropInterface((struct Interface *)n1->in_IFace);
#endif /* __amigaos4__ */
			CloseLibrary( n1->in_LibBase );
			}
		IconNode_Free(n1, IconObjectBase );
		}
}

//-----------------------------------------------------------------------------

// NewIconObject()
LIBFUNC_P3(struct Iconobject *, LIBNewIconObject,
	A0, APTR, Name,
	A1, CONST struct TagItem *, Taglist, 
	A6, struct IconObjectBase *, IconObjectBase)
{
	struct Iconobject *obj = NULL;
	BPTR IconFh = NULL;

	d1(kprintf(__FILE__ "/%s/%ld: Name=<%s>\n", __FUNC__, __LINE__, Name));

	if (Name)
		{
		STRPTR IconName;

		IconName = MyAllocVecPooled(strlen(Name) + 1 + 5);	// allocate space for Name and ".info"
		if (IconName)
			{
			strcpy(IconName, Name);
			strcat(IconName, ".info");

			IconFh = Open(IconName, MODE_OLDFILE);
			MyFreeVecPooled(IconName);
			}
		}

	d1(KPrintF("%s/%s/%ld: IconFh=%08lx\n", __FILE__, __FUNC__, __LINE__, IconFh));

	if (IconFh)
		{
		struct IconNode *in;

		SetVBuf(IconFh, NULL, BUF_FULL, 16384);

		for (in = (struct IconNode *) IconObjectBase->iob_ClassList.lh_Head;
			NULL == obj && in != (struct IconNode *) &IconObjectBase->iob_ClassList.lh_Tail;
			in = (struct IconNode *) in->Node.mln_Succ)
			{
			d1(KPrintF("%s/%ld:  datatype=<%s>  class=%08lx  in_name=<%s>\n", \
				__FUNC__, __LINE__, in->in_Name, in->in_Class, in->in_Name));

			// Make sure each datatype starts with 0 file offset
			Seek(IconFh, 0, OFFSET_BEGINNING);

			obj = NewObject(in->in_Class, NULL,
				DTA_Name, (ULONG) Name,
				DTA_Handle, (ULONG) IconFh,
				TAG_MORE, (ULONG) Taglist,
				TAG_END);

			d1(kprintf("%s/%ld:  datatype=<%s>  obj=%08lx  in_name=<%s>\n", \
				__FUNC__, __LINE__, in->in_Name, obj, in->in_Name));
			}

		Close(IconFh);
		}

	d1(kprintf("%s/%ld:   obj=%08lx\n", __FUNC__, __LINE__, obj));

	return obj;
}
LIBFUNC_END

//-----------------------------------------------------------------------------

// DisposeIconObject()
LIBFUNC_P2(void, LIBDisposeIconObject,
	A0, struct Iconobject *, iconobject, 
	A6, struct IconObjectBase *, IconObjectBase)
{
	d1(kprintf(__FILE__ "/%s/%ld:  iconobject=%08lx\n", __FUNC__ , __LINE__, iconobject));
	(void) IconObjectBase;

	DisposeObject((Object *) iconobject);
}
LIBFUNC_END

//-----------------------------------------------------------------------------

// GetDefIconObject()
LIBFUNC_P3(struct Iconobject *, LIBGetDefIconObject,
	D0, ULONG, IconType,
	A0, CONST struct TagItem *, TagList, 
	A6, struct IconObjectBase *, IconObjectBase)
{
	struct IconNode *in;
	struct Iconobject *obj = NULL;

	(void) IconObjectBase;

	d1(kprintf(__FILE__ "/%s/%ld:  type=%08lx\n", __FUNC__, __LINE__, IconType));

	for (in = (struct IconNode *) IconObjectBase->iob_ClassList.lh_Head;
		NULL == obj && in != (struct IconNode *) &IconObjectBase->iob_ClassList.lh_Tail;
		in = (struct IconNode *) in->Node.mln_Succ)
		{
		obj = NewObject(in->in_Class, NULL, 
			IDTA_DefType, IconType,
			DTA_Name, (ULONG) GetTagData(DTA_Name, (ULONG) "", TagList),
			TAG_MORE, (ULONG) TagList,
			TAG_END);

		d1(kprintf("%s/%ld:   obj=%08lx  in_name=<%s>\n", __FUNC__, __LINE__, obj, in->in_Name));
		}

	d1(kprintf("%s/%ld:   obj=%08lx\n", __FUNC__, __LINE__, obj));

	return obj;
}
LIBFUNC_END

//-----------------------------------------------------------------------------

// PutIconObject()
LIBFUNC_P4(LONG, LIBPutIconObject,
	A0, struct Iconobject *, iconobject, 
	A1, APTR, path, 
	A2, CONST struct TagItem *, TagList, 
	A6, struct IconObjectBase *, IconObjectBase)
{
	d1(kprintf(__FILE__ "/%s/%ld:  obj=%08lx\n", __FUNC__, __LINE__, iconobject));

	(void) IconObjectBase;

	return (LONG) DoMethod((Object *) iconobject, IDTM_Write, path, NULL, TagList);
}
LIBFUNC_END

//-----------------------------------------------------------------------------

// Input: A0 = Filename
// Output: D0 = address of extension ( or NULL or -1 )
// IsIconName()
LIBFUNC_P2(ULONG, LIBIsIconName,
	A0, const char *, filename, 
	A6, struct IconObjectBase *, IconObjectBase);
{
	struct IconNode *in;

	d1(kprintf(__FILE__ "/%s/%ld:  name=<%s>\n", __FUNC__, __LINE__, filename));

	filename += strlen(filename);

	for (in = (struct IconNode *) IconObjectBase->iob_ClassList.lh_Head;
		in != (struct IconNode *) &IconObjectBase->iob_ClassList.lh_Tail;
		in = (struct IconNode *) in->Node.mln_Succ)
		{
		d1(kprintf("%s/%ld:  Suffix=<%s>  Len=%ld\n", __FUNC__, __LINE__, in->in_Suffix, in->in_SuffixLen));

		if (0 == in->in_SuffixLen)
			{
			d1(kprintf("%s/%ld:  Found (1)\n", __FUNC__, __LINE__));
			return -1;		// Found
			}
		if (0 == Stricmp(in->in_Suffix, filename - in->in_SuffixLen))
			{
			d1(kprintf("%s/%ld:  Found (2)\n", __FUNC__, __LINE__));
			return (ULONG) (filename - in->in_SuffixLen);
			}
		}

	d1(kprintf("%s/%ld:  Not Found\n", __FUNC__, __LINE__));

	return 0;
}
LIBFUNC_END

//-----------------------------------------------------------------------------

// Convert2IconObjectA()
LIBFUNC_P3(struct Iconobject *, LIBConvert2IconObjectA,
	A0, struct DiskObject *, diskobject, 
	A1, const struct TagItem *, TagList,
	A6, struct IconObjectBase *, IconObjectBase)
{
	return InternalConvert2IconObjectA(diskobject, TagList, IconObjectBase);
}
LIBFUNC_END

//-----------------------------------------------------------------------------

// Convert2IconObject()
LIBFUNC_P2(struct Iconobject *, LIBConvert2IconObject,
	A0, struct DiskObject *, diskobject, 
	A6, struct IconObjectBase *, IconObjectBase)
{
	return InternalConvert2IconObjectA(diskobject, NULL, IconObjectBase);
}
LIBFUNC_END

//-----------------------------------------------------------------------------

static struct Iconobject *InternalConvert2IconObjectA(
	struct DiskObject *diskobject, const struct TagItem *TagList,
	struct IconObjectBase *IconObjectBase)
{
	struct IconNode *in;
	struct Iconobject *obj = NULL;

	d1(kprintf(__FILE__ "/%s/%ld:  diskobj=%08lx\n", __FUNC__, __LINE__, diskobject));

	for (in = (struct IconNode *) IconObjectBase->iob_ClassList.lh_Head;
		NULL == obj && in != (struct IconNode *) &IconObjectBase->iob_ClassList.lh_Tail;
		in = (struct IconNode *) in->Node.mln_Succ)
		{
		obj = NewObject(in->in_Class, (ULONG) NULL,
			AIDTA_Icon, (ULONG) diskobject,
			DTA_Name, (ULONG) "",
			TAG_MORE, (ULONG) TagList,
			TAG_END);

		d1(kprintf("%s/%ld:   obj=%08lx  in_name=<%s>\n", __FUNC__, __LINE__, obj, in->in_Name));
		}

	return obj;
}

//----------------------------------------------------------------------------------------

static APTR MyAllocVecPooled(size_t Size)
{
	APTR ptr;

	if (MemPool)
		{
		ObtainSemaphore(&PubMemPoolSemaphore);
		ptr = AllocPooled(MemPool, Size + sizeof(size_t));
		ReleaseSemaphore(&PubMemPoolSemaphore);
		if (ptr)
			{
			size_t *sptr = (size_t *) ptr;

			sptr[0] = Size;

			d1(kprintf("%s/%s/%ld:  MemPool=%08lx  Size=%lu  mem=%08lx\n", \
				__FILE__, __FUNC__, __LINE__, MemPool, Size, &sptr[1]));
			return (APTR)(&sptr[1]);
			}
		}

	d1(kprintf("%s/%s/%ld:  MemPool=%08lx  Size=%lu\n", __FILE__, __FUNC__, __LINE__, MemPool, Size));

	return NULL;
}


static void MyFreeVecPooled(APTR mem)
{
	d1(kprintf("%s/%s/%ld:  MemPool=%08lx  mem=%08lx\n", __FILE__, __FUNC__, __LINE__, MemPool, mem));
	if (MemPool && mem)
		{
		size_t size;
		size_t *sptr = (size_t *) mem;

		mem = &sptr[-1];
		size = sptr[-1];

		ObtainSemaphore(&PubMemPoolSemaphore);
		FreePooled(MemPool, mem, size + sizeof(size_t));
		ReleaseSemaphore(&PubMemPoolSemaphore);
		}
}

//----------------------------------------------------------------------------------------

#if !defined(__SASC) && !defined(__amigaos4__)
void exit(int x)
{
   (void) x;
   while (1)
	  ;
}

#endif /* !defined(__SASC) */

//-----------------------------------------------------------------------------

