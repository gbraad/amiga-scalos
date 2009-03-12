// FileTransClass.h
// $Date$
// $Revision$


#ifndef	FILETRANSCLASS_H
#define	FILETRANSCLASS_H

#include "int64.h"

//----------------------------------------------------------------------------

// class instance data for "FileTransfer.sca"

struct FileTransClassInstance
	{
	ULONG	ftci_Number;

	ULONG	ftci_ReplaceMode;		// +jl+ 20010713
	ULONG	ftci_MostCurrentReplaceMode;	// effective ReplaceMode for last copy/move operation

	ULONG	ftci_OverwriteMode;
	ULONG	ftci_MostCurrentOverwriteMode;

	struct	Screen *ftci_Screen;
	struct	Window *ftci_Window;
	struct	Gadget *ftci_GadgetList;	// List of allocated gadgets

	struct	TextAttr ftci_TextAttr;		// TextAttr for Gadget texts

	T_TIMEVAL ftci_ProcessStartTime;	// time when processing of FTOps started (start counting)
	T_TIMEVAL ftci_CopyStartTime;		// time when processing of FTOps started (start copy/move)
	T_TIMEVAL ftci_LastRemainTimeDisplay;	// last time when remaining time was updated

	struct	Gadget *ftci_TextLine1Gadget;
	struct	Gadget *ftci_TextLine2Gadget;
	struct	Gadget *ftci_TextLine3Gadget;
	struct	Gadget *ftci_GaugeGadget;
	struct	Gadget *ftci_CancelButtonGadget;

	STRPTR	ftci_Line1Buffer;		// Text Buffer for ftci_TextLine1Gadget
	STRPTR	ftci_Line2Buffer;		// Text Buffer for ftci_TextLine2Gadget
	char	ftci_Line3Buffer[60];

	ULONG	ftci_CopyCount;
	ULONG	ftci_HookResult;

	struct 	List ftci_OpList;
	SCALOSSEMAPHORE	ftci_OpListSemaphore;

	ULONG64	ftci_TotalBytes;
	ULONG64	ftci_CurrentBytes;

	ULONG	ftci_TotalItems;
	ULONG	ftci_CurrentItems;

	ULONG	ftci_TotalFiles;
	ULONG	ftci_TotalDirs;
	ULONG	ftci_TotalLinks;

	ULONG	ftci_CurrentOperation;		// type of current operation (FTOPCODE_Copy/FTOPCODE_Move)

	BOOL	ftci_CountValid;		// TRUE if ftci_TotalItems and ftci_TotalBytes are valid

	struct	DatatypesImage *ftci_Background;
	struct	Hook ftci_BackFillHook;

	LONG	ftci_LastErrorCode;		// code of last error
	STRPTR	ftci_LastErrorFileName;		// name of object on which last error occured
	};

// Gadget IDs
#define	GID_CancelButton	1013

#endif	/* FILETRANSCLASS_H */
