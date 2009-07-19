// HiddenDevices.c
// $Date$
// $Revision$


#include <exec/types.h>
#include <scalos/preferences.h>
#include <scalos/scalos.h>
#include <intuition/intuition.h>
#include <devices/inputevent.h>
#include <libraries/iffparse.h>
#include <libraries/mui.h>
#include <libraries/asl.h>
#include <libraries/mcpgfx.h>
#include <libraries/ttengine.h>
#include <graphics/view.h>
#include <prefs/prefhdr.h>
#include <prefs/font.h>
#include <prefs/workbench.h>
#include <MUI/NListview_mcc.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <proto/scalos.h>
#include <proto/utility.h>
#include <proto/wb.h>
#define	NO_INLINE_STDARG
#include <proto/muimaster.h>

#include <clib/alib_protos.h>

#include <defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>

#include "HiddenDevices.h"
#include "ScalosPrefs.h"

//-----------------------------------------------------------------

struct HiddenDevice
	{
	struct Node hd_Node;
	char hd_Name[1];	// variable length
	};

//-----------------------------------------------------------------

static void HandleWBHD(const UBYTE *data, size_t length);
static void HandleWBNC(const UWORD *Data, size_t Length);
static void AddHiddenDeviceFromDosList(struct SCAModule *app, struct DosList *dl, struct InfoData *id, BOOL Hidden);
static void BtoCString(BPTR bstring, STRPTR Buffer, size_t Length);
static BOOL FindHiddenDevice(CONST_STRPTR Name);
static void StripTrailingColon(STRPTR Line);

//-----------------------------------------------------------------

static struct WorkbenchPrefs WBPrefs =
	{
	8192,			//wbp_DefaultStackSize;
	3,			//wbp_TypeRestartTime;
	16,			//wbp_IconPrecision;
	{ -1, -1, 1, 1 },	//wbp_EmbossRect;
	FALSE,			//wbp_Borderless;
	25,			//wbp_MaxNameLength;
	TRUE,			//wbp_NewIconsSupport;
	TRUE,			//wbp_ColorIconSupport;
	MEMF_CHIP,              //wbp_ImageMemType;
	FALSE,			//wbp_LockPens;
	FALSE,			//wbp_NoTitleBar;
	FALSE,			//wbp_NoGauge;
	};

static struct List HiddenDevicesList;

//-----------------------------------------------------------------

BOOL HiddenDevicesInit(void)
{
	NewList(&HiddenDevicesList);

	return TRUE;
}

//-----------------------------------------------------------------

void HiddenDevicesCleanup(void)
{
	struct HiddenDevice *hd;

	while ((hd = (struct HiddenDevice *) RemHead(&HiddenDevicesList)))
		{
		free(hd);
		}
}

//-----------------------------------------------------------------

// Read workbench.prefs and build list of hidden devices
BOOL ReadWorkbenchPrefs(CONST_STRPTR filename)
{
	struct IFFHandle *iff;
	LONG error;
	BOOL IffOpened = FALSE;
	static const ULONG pairs[] = {ID_PREF, ID_WBNC, ID_PREF, ID_WBHD};


	HiddenDevicesCleanup();


	d1(KPrintF(__FILE__ "/%s/%ld: filename=<%s>\n", __FUNC__, __LINE__, filename));

	do	{
		iff = AllocIFF();
		if (NULL == iff)
			break;

		iff->iff_Stream = (ULONG) Open((STRPTR) filename, MODE_OLDFILE);
		d1(KPrintF(__FILE__ "/%s/%ld: iff_Stream=%08lx\n", __FUNC__, __LINE__, iff->iff_Stream));
		if ((BPTR)NULL == iff->iff_Stream)
			break;

		InitIFFasDOS( iff );

		error = OpenIFF( iff, IFFF_READ );
		if (RETURN_OK != error)
			break;

		IffOpened = TRUE;

		error = CollectionChunks( iff, (LONG*) pairs, Sizeof(pairs) / 2);
		if (RETURN_OK != error)
			break;

		StopOnExit( iff, ID_PREF, ID_FORM );

		while(TRUE)
			{
			if( ( error = ParseIFF( iff, IFFPARSE_SCAN ) ) == IFFERR_EOC )
				{
				struct CollectionItem *ci;

				ci = FindCollection( iff, ID_PREF, ID_WBNC);
				while( ci )
					{
					HandleWBNC(ci->ci_Data, ci->ci_Size);
					ci = ci->ci_Next;
					}

				ci = FindCollection( iff, ID_PREF, ID_WBHD);
				while( ci )
					{
					HandleWBHD(ci->ci_Data, ci->ci_Size);
					ci = ci->ci_Next;
					}

				}
			else
				break;
			}
		} while (0);

	if (iff)
		{
		if (IffOpened)
			CloseIFF( iff );
		if (iff->iff_Stream)
			Close( iff->iff_Stream );
		FreeIFF( iff );
		}

	return TRUE;
}

//-----------------------------------------------------------------

static void HandleWBHD(const UBYTE *data, size_t length)
{
	d1(KPrintF(__FILE__ "/%s/%ld: data=<%s>  length=%lu\n", __FUNC__, __LINE__, data, length));

	if (!FindHiddenDevice((STRPTR)data))	// make sure we get no cuplicate entries
		AddHiddenDevice((STRPTR)data);
}

//-----------------------------------------------------------------

static void HandleWBNC(const UWORD *Data, size_t Length)
{
	const struct WorkbenchPrefs *prefs = (const struct WorkbenchPrefs *) Data;
	size_t CopyLength = min(Length, sizeof(struct WorkbenchPrefs));

	memcpy(&WBPrefs, prefs, CopyLength);
}

//-----------------------------------------------------------------

// save workbench.prefs, including WBHD entries for each hidden device
LONG WriteWorkbenchPrefs(CONST_STRPTR Filename)
{
	struct IFFHandle *iff;
	LONG Result;
	BOOL IffOpen = FALSE;

	do	{
		static const struct PrefHeader prefHeader = { 0, 0, 0L };
		struct HiddenDevice *hd;
		LONG Length;

		d1(KPrintF(__FILE__ "/%s/%ld: Filename=<%s>\n", __FUNC__, __LINE__, Filename));

		iff = AllocIFF();
		if (NULL == iff)
			{
			Result = IoErr();
			break;
			}

		InitIFFasDOS(iff);

		iff->iff_Stream = Open(Filename, MODE_NEWFILE);
		if ((BPTR)NULL == iff->iff_Stream)
			{
			// ... try to create missing directories here
			STRPTR FilenameCopy;

			Result = IoErr();
			d1(KPrintF(__FILE__ "/%s/%ld: Result=%ld\n", __FUNC__, __LINE__, Result));

			FilenameCopy = AllocVec(1 + strlen(Filename), MEMF_PUBLIC);

			if (FilenameCopy)
				{
				STRPTR lp;

				strcpy(FilenameCopy, Filename);

				lp = PathPart(FilenameCopy);
				if (lp)
					{
					BPTR dirLock;

					*lp = '\0';
					dirLock = CreateDir(FilenameCopy);

					if (dirLock)
						UnLock(dirLock);

					iff->iff_Stream = Open(Filename, MODE_NEWFILE);
				if ((BPTR)NULL == iff->iff_Stream)
						Result = IoErr();
					else
						Result = RETURN_OK;
					}

				FreeVec(FilenameCopy);
				}

			d1(KPrintF(__FILE__ "/%s/%ld: Result=%ld\n", __FUNC__, __LINE__, Result));
			if (RETURN_OK != Result)
				break;
			}

		Result = OpenIFF(iff, IFFF_WRITE);
		if (RETURN_OK != Result)
			break;

		IffOpen = TRUE;

		Result = PushChunk(iff, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN);
		if (RETURN_OK != Result)
			break;

		Result = PushChunk(iff, 0, ID_PRHD, IFFSIZE_UNKNOWN);
		if (RETURN_OK != Result)
			break;

		if (WriteChunkBytes(iff, (APTR) &prefHeader, sizeof(prefHeader)) < 0)
			{
			Result = IoErr();
			break;
			}

		Result = PopChunk(iff);		// PRHD
		if (RETURN_OK != Result)
			break;

		Result = PushChunk(iff, 0, ID_WBNC, IFFSIZE_UNKNOWN);
		if (RETURN_OK != Result)
			break;

		Length = sizeof(WBPrefs);
		if (Length != WriteChunkBytes(iff, (APTR) &WBPrefs, Length))
			{
			Result = IoErr();
			break;
			}

		Result = PopChunk(iff);		// WBNC
		if (RETURN_OK != Result)
			break;

		for (hd = (struct HiddenDevice *) HiddenDevicesList.lh_Head;
			hd != (struct HiddenDevice *) &HiddenDevicesList.lh_Tail;
			hd = (struct HiddenDevice *) hd->hd_Node.ln_Succ)
			{
			size_t wbhdLength = 1 + strlen(hd->hd_Name);

			d1(KPrintF(__FILE__ "/%s/%ld: hd_Name=<%s>\n", __FUNC__, __LINE__, hd->hd_Name));

			Result = PushChunk(iff, 0, ID_WBHD, IFFSIZE_UNKNOWN);
			if (RETURN_OK != Result)
				break;

			if (wbhdLength != WriteChunkBytes(iff, (APTR) hd->hd_Name, wbhdLength))
				{
				Result = IoErr();
				break;
				}
			if (wbhdLength & 0x01)
				{
				// write dummy byte to get even chunk length
				ULONG Dummy = 0;

				if (1 != WriteChunkBytes(iff, (APTR) &Dummy, 1))
					{
					Result = IoErr();
					break;
					}
				}

			Result = PopChunk(iff);		// WBHD
			if (RETURN_OK != Result)
				break;
			}

		if (RETURN_OK != Result)
			break;

		Result = PopChunk(iff);		// FORM
		if (RETURN_OK != Result)
			break;

		} while (0);

	if (iff)
		{
		if (IffOpen)
			CloseIFF(iff);

		if (iff->iff_Stream)
			{
			Close(iff->iff_Stream);
			iff->iff_Stream = (BPTR)NULL;
			}

		FreeIFF(iff);
		}

	d1(KPrintF(__FILE__ "/%s/%ld: Result=%ld\n", __FUNC__, __LINE__, Result));

	if (RETURN_OK == Result && fCreateIcons)
		SaveIcon(Filename);

	return Result;
}

//-----------------------------------------------------------------

// Traverse DosList for all devices, and build list of hideable devices
void FillHiddenDevicesList(struct SCAModule *app)
{
	ULONG LockDosListFlags = LDF_DEVICES | LDF_READ;
	struct HiddenDevice *hd;
	struct DosList *dl;
	struct InfoData *id;

	set(app->Obj[NLIST_HIDDENDEVICES], MUIA_NList_Quiet, TRUE);
	DoMethod(app->Obj[NLIST_HIDDENDEVICES], MUIM_NList_Clear);
	DoMethod(app->Obj[NLIST_HIDDENDEVICES], MUIM_NList_UseImage,
		NULL, MUIV_NList_UseImage_All, 0);

	id = malloc(sizeof(struct InfoData));
	if (id)
		{
		dl = LockDosList(LockDosListFlags);

		while (dl = NextDosEntry(dl, LockDosListFlags))
			{
			AddHiddenDeviceFromDosList(app, dl, id, FALSE);
			}

		UnLockDosList(LockDosListFlags);
		free(id);
		}

	// Now walk through the list of devices marked as hidden,
	// and add all entries.

	dl = LockDosList(LockDosListFlags);

	for (hd = (struct HiddenDevice *) HiddenDevicesList.lh_Head;
		hd != (struct HiddenDevice *) &HiddenDevicesList.lh_Tail;
		hd = (struct HiddenDevice *) hd->hd_Node.ln_Succ)
		{
		char HiddenDevName[128];
		struct DosList *dlFound;

		// Name must not have trailing ":" to use with FindDosEntry()
		stccpy(HiddenDevName, hd->hd_Name, sizeof(HiddenDevName));
		StripTrailingColon(HiddenDevName);
		d1(KPrintF(__FILE__ "/%s/%ld: HiddenDevName=<%s>\n", __FUNC__, __LINE__, HiddenDevName));

		dlFound = FindDosEntry(dl, HiddenDevName, LockDosListFlags);
		d1(KPrintF(__FILE__ "/%s/%ld: dl=%08lx\n", __FUNC__, __LINE__, dlFound));
		if (dlFound)
			{
			AddHiddenDeviceFromDosList(app, dlFound, id, TRUE);
			}
		else
			{
			struct NewHiddenDevice nhd;

			nhd.nhd_DeviceName = hd->hd_Name;
			nhd.nhd_VolumeName = "";
			nhd.nhd_Hidden = TRUE;

			d1(KPrintF(__FILE__ "/%s/%ld: DevName=<%s>  VolName=<%s>\n", \
				__FUNC__, __LINE__, nhd.nhd_DeviceName, nhd.nhd_VolumeName));

			DoMethod(app->Obj[NLIST_HIDDENDEVICES],
				MUIM_NList_InsertSingle,
				&nhd,
				MUIV_NList_Insert_Sorted);
			}
		}

	UnLockDosList(LockDosListFlags);

	set(app->Obj[NLIST_HIDDENDEVICES], MUIA_NList_Quiet, FALSE);
}

//-----------------------------------------------------------------

static void AddHiddenDeviceFromDosList(struct SCAModule *app, struct DosList *dl, struct InfoData *id, BOOL Hidden)
{
	char DevName[128];
	char VolName[128];
	struct DosList *VolumeNode;
	BOOL InfoDataValid = FALSE;

	strcpy(VolName, "");
	BtoCString(dl->dol_Name, DevName, sizeof(DevName));

	d1(KPrintF(__FILE__ "/%s/%ld: START  DevName=<%s>\n", __FUNC__, __LINE__, DevName));

#if defined(__amigaos4__)
	if (0 != Stricmp(DevName, "ENV"))
#endif //!defined(__amigaos4__)
		{
		d1(KPrintF(__FILE__ "/%s/%ld: \n", __FUNC__, __LINE__));
		if (dl->dol_Task &&
			DoPkt(dl->dol_Task, ACTION_DISK_INFO,
				MKBADDR(id),
				0, 0, 0, 0))
			{
			InfoDataValid = TRUE;
			}
		d1(KPrintF(__FILE__ "/%s/%ld: InfoDataValid=%ld\n", __FUNC__, __LINE__, InfoDataValid));
		}

	if (!Hidden && !InfoDataValid)
		return;

	if (InfoDataValid)
		{
		switch (id->id_DiskState)
			{
		case ID_WRITE_PROTECTED:
		case ID_VALIDATING:
		case ID_VALIDATED:
			VolumeNode = BADDR(id->id_VolumeNode);
			if (VolumeNode)
				{
				BtoCString(VolumeNode->dol_Name, VolName, sizeof(VolName));
				StripTrailingColon(VolName);
				strcat(VolName, ":");
				}
			break;
		default:
			break;
			}
		}

	StripTrailingColon(DevName);
	strcat(DevName, ":");

	// add only visible devices here
	if (Hidden || !FindHiddenDevice(DevName))
		{
		struct NewHiddenDevice nhd;

		nhd.nhd_DeviceName = DevName;
		nhd.nhd_VolumeName = VolName;
		nhd.nhd_Hidden = Hidden;

		d1(KPrintF(__FILE__ "/%s/%ld: DevName=<%s>  VolName=<%s>\n", \
			__FUNC__, __LINE__, nhd.nhd_DeviceName, nhd.nhd_VolumeName));

		DoMethod(app->Obj[NLIST_HIDDENDEVICES],
			MUIM_NList_InsertSingle,
			&nhd,
			MUIV_NList_Insert_Sorted);
		}
}

//-----------------------------------------------------------------

static void BtoCString(BPTR bstring, STRPTR Buffer, size_t Length)
{
	CONST_STRPTR Src = BADDR(bstring);
	size_t bLength;

	// get BSTRING length (1st octet)
	bLength = *Src++;

	while (bLength-- && (Length-- > 1))
		*Buffer++ = *Src++;

	*Buffer = '\0';
}

//-----------------------------------------------------------------

// Add entry to  hidden devices list
BOOL AddHiddenDevice(CONST_STRPTR Name)
{
	struct HiddenDevice *hd;
	BOOL Success = FALSE;

	hd = malloc(sizeof(struct HiddenDevice) + 1 + strlen(Name));
	if (hd)
		{
		strcpy(hd->hd_Name, Name);

		// make sure device name is terminated with exactly one ":"
		StripTrailingColon(hd->hd_Name);
		strcat(hd->hd_Name, ":");

		AddHead(&HiddenDevicesList, &hd->hd_Node);
		Success = TRUE;
		}

	d1(KPrintF(__FILE__ "/%s/%ld: Name=<%s>  hd=%08lx\n", __FUNC__, __LINE__, Name, hd));

	return Success;
}

//-----------------------------------------------------------------

// Remove entry from hidden devices list
BOOL RemoveHiddenDevice(CONST_STRPTR Name)
{
	struct HiddenDevice *hd;
	BOOL Found = FALSE;

	d1(KPrintF(__FILE__ "/%s/%ld: Name=<%s>", __FUNC__, __LINE__, Name));

	for (hd = (struct HiddenDevice *) HiddenDevicesList.lh_Head;
		hd != (struct HiddenDevice *) &HiddenDevicesList.lh_Tail;
		hd = (struct HiddenDevice *) hd->hd_Node.ln_Succ)
		{
		if (0 == Stricmp(Name, hd->hd_Name))
			{
			Found = TRUE;
			Remove(&hd->hd_Node);
			free(hd);
			break;
			}
		}

	return Found;
}

//-----------------------------------------------------------------

// search for entry in hidden devices list
static BOOL FindHiddenDevice(CONST_STRPTR Name)
{
	const struct HiddenDevice *hd;
	BOOL Found = FALSE;

	d1(KPrintF(__FILE__ "/%s/%ld: Name=<%s>", __FUNC__, __LINE__, Name));

	for (hd = (struct HiddenDevice *) HiddenDevicesList.lh_Head;
		hd != (struct HiddenDevice *) &HiddenDevicesList.lh_Tail;
		hd = (struct HiddenDevice *) hd->hd_Node.ln_Succ)
		{
		if (0 == Stricmp(Name, hd->hd_Name))
			{
			Found = TRUE;
			break;
			}
		}

	return Found;
}

//-----------------------------------------------------------------

static void StripTrailingColon(STRPTR Line)
{
	size_t Len = strlen(Line);

	Line += Len - 1;

	if (':' == *Line)
		*Line = '\0';
}

//-----------------------------------------------------------------

