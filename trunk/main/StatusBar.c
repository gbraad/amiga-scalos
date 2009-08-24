// StatusBar.c
// $Date$
// $Revision$
// $Id$

#include <exec/types.h>
#include <graphics/gels.h>
#include <graphics/rastport.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <utility/hooks.h>
#include <intuition/gadgetclass.h>
#include <workbench/workbench.h>
#include <dos/dostags.h>
#include <dos/datetime.h>

#define	__USE_SYSBASE

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/locale.h>
#include "debug.h"
#include <proto/scalos.h>

#include <clib/alib_protos.h>

#include <defs.h>
#include <scalos/scalos.h>
#include <scalos/GadgetBar.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scalos_structures.h"
#include "functions.h"
#include "Variables.h"

//----------------------------------------------------------------------------

BOOL StatusBarAdd(struct internalScaWindowTask *iwt)
{
	ULONG Height = 0;

	d1(KPrintF("%s/%s/%ld: iwt=%08lx  <%s>\n", __FILE__, __FUNC__, __LINE__, iwt, iwt->iwt_WinTitle));

	if (iwt->iwt_StatusBar)
		return TRUE;		// StatusBar already present
	if (iwt->iwt_BackDrop)
		return FALSE;		// no statusbar for backdrop windows

	d1(kprintf("%s/%s/%ld: StatusBar BgPen=%ld\n", __FILE__, __FUNC__, __LINE__, \
		PalettePrefs.pal_PensList[PENIDX_STATUSBAR_BG]));

	iwt->iwt_StatusBar = (struct Gadget *) SCA_NewScalosObjectTags("GadgetBar.sca",
		GBDTA_WindowTask, (ULONG) iwt,
		GBDTA_Position, GBPOS_Bottom,
		GA_RelSpecial, TRUE,
		GA_GadgetHelp, TRUE,
		GA_ID, GADGETID_STATUSBAR,
		GBDTA_BGPen, PalettePrefs.pal_PensList[PENIDX_STATUSBAR_BG],
		GBDTA_BackgroundImageName, (ULONG) "THEME:Window/StatusBar/Background",
		TAG_END);

	if (NULL == iwt->iwt_StatusBar)
		return FALSE;

	d1(KPrintF("%s/%s/%ld: Height=%ld\n", __FILE__, __FUNC__, __LINE__, Height));
	d1(KPrintF("%s/%s/%ld: tf_YSize=%ld\n", __FILE__, __FUNC__, __LINE__, iInfos.xii_iinfos.ii_Screen->RastPort.Font->tf_YSize));

	iwt->iwt_StatusBarMembers[STATUSBARGADGET_StatusText] = (struct Gadget *) SCA_NewScalosObjectTags("GadgetBarText.sca",
		GBTDTA_Text, (ULONG) "Ä???g",
		GBTDTA_TextFont, (ULONG) iInfos.xii_iinfos.ii_Screen->RastPort.Font,
		GBTDTA_TTFont, (ULONG) &ScreenTTFont,
		GBTDTA_TextPen, PalettePrefs.pal_PensList[PENIDX_STATUSBAR_TEXT],
		GBTDTA_Justification, GACT_STRINGCENTER,
		GA_ID, SBAR_GadgetID_Text,
		TAG_END);
	if (NULL == iwt->iwt_StatusBarMembers[STATUSBARGADGET_StatusText])
		return FALSE;

	DoMethod(iwt->iwt_WindowTask.mt_MainObject, SCCM_IconWin_AddToStatusBar,
		iwt->iwt_StatusBarMembers[STATUSBARGADGET_StatusText],
		GBDTA_Weight, 100,
		TAG_END);

	if (IsShowAll(iwt->iwt_WindowTask.mt_WindowStruct))
		{
		d1(KPrintF("%s/%s/%ld: \n", __FILE__, __FUNC__, __LINE__));
		iwt->iwt_StatusBarMembers[STATUSBARGADGET_ShowAll] = (struct Gadget *) SCA_NewScalosObjectTags("GadgetBarImage.sca",
			DTA_Name, (ULONG) "THEME:Window/StatusBar/ShowAll",
			GBIDTA_WindowTask, (ULONG) iwt,
			GA_ID, SBAR_GadgetID_ShowAll,
			TAG_END);

		if (NULL == iwt->iwt_StatusBarMembers[STATUSBARGADGET_ShowAll])
			return FALSE;

		DoMethod(iwt->iwt_WindowTask.mt_MainObject,
			SCCM_IconWin_AddToStatusBar,
			iwt->iwt_StatusBarMembers[STATUSBARGADGET_ShowAll],
			TAG_END);
		}

	d1(KPrintF("%s/%s/%ld: Height=%ld\n", __FILE__, __FUNC__, __LINE__, Height));

	if (iwt->iwt_WindowTask.wt_Window)
		AddGList(iwt->iwt_WindowTask.wt_Window, iwt->iwt_StatusBar, ~0, 1, NULL);

	// Layout Status bar so we can query the required height
	DoGadgetMethod(iwt->iwt_StatusBar,
		iInfos.xii_iinfos.ii_MainWindowStruct->ws_Window, NULL,
		GM_LAYOUT,
		NULL,		// GadgetInfo is filled in by DoGadgetMethod
		TRUE);

	GetAttr(GA_Height, iwt->iwt_StatusBar, &Height);

	d1(KPrintF("%s/%s/%ld: Height=%ld  gg->Height=%ld  iwt_GadgetBarHeightBottom=%ld\n", \
		__FILE__, __FUNC__, __LINE__, Height, iwt->iwt_StatusBar->Height, iwt->iwt_GadgetBarHeightBottom));
	d1(KPrintF("%s/%s/%ld: iwt_ExtraHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_ExtraHeight));
	d1(KPrintF("%s/%s/%ld: iwt_InnerBottom=%ld\n", \
		__FILE__, __FUNC__, __LINE__, iwt->iwt_InnerBottom));

	iwt->iwt_ExtraHeight += Height;
	iwt->iwt_InnerBottom += iwt->iwt_GadgetBarHeightBottom;
	iwt->iwt_InnerHeight -= iwt->iwt_InnerBottom;

	d1(KPrintF("%s/%s/%ld: iwt_ExtraHeight=%ld  iwt_GadgetBarHeightBottom=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_ExtraHeight, iwt->iwt_GadgetBarHeightBottom));
	d1(KPrintF("%s/%s/%ld: iwt_InnerBottom=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_InnerBottom));

	return TRUE;
}

//----------------------------------------------------------------------------

void StatusBarRemove(struct internalScaWindowTask *iwt)
{
	if (iwt->iwt_StatusBar)
		{
		d1(kprintf("%s/%s/%ld: iwt_ExtraHeight=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_ExtraHeight));

		if (iwt->iwt_WindowTask.wt_Window)
			RemoveGList(iwt->iwt_WindowTask.wt_Window, iwt->iwt_StatusBar, 1);

		d1(KPrintF("%s/%s/%ld: iwt_InnerBottom=%ld\n", \
			__FILE__, __FUNC__, __LINE__, iwt->iwt_InnerBottom));

		iwt->iwt_ExtraHeight -= iwt->iwt_GadgetBarHeightBottom;
		iwt->iwt_InnerBottom -= iwt->iwt_GadgetBarHeightBottom;
		iwt->iwt_InnerHeight += iwt->iwt_GadgetBarHeightBottom;

		DisposeObject(iwt->iwt_StatusBar);
		iwt->iwt_StatusBar = NULL;

		memset(iwt->iwt_StatusBarMembers, 0, sizeof(iwt->iwt_StatusBarMembers));

		d1(KPrintF("%s/%s/%ld: iwt_InnerBottom=%ld\n", __FILE__, __FUNC__, __LINE__, iwt->iwt_InnerBottom));
		}
}

//----------------------------------------------------------------------------

UWORD StatusBarQueryGadgetID(struct internalScaWindowTask *iwt,
	const struct ExtGadget *gg, WORD x, WORD y)
{
	UWORD Code;
	ULONG xy;

	xy = ((x - gg->LeftEdge) << 16) + y - gg->TopEdge;

	Code = DoGadgetMethod((struct Gadget *) gg, iwt->iwt_WindowTask.wt_Window, NULL,
		GM_HELPTEST,
		NULL,	// GadgetInfo is inserted here
		xy);
	d1(KPrintF("%s/%s/%ld: Code=%04lx\n", __FILE__, __FUNC__, __LINE__, Code));

	return Code;
}

//----------------------------------------------------------------------------

