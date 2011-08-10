
/****************************************************************

   This file was created automatically by `FlexCat 2.4'
   from "Updater.cd"

   using CatComp.sd 1.2 (24.09.1999)

   Do NOT edit by hand!

****************************************************************/

#ifndef Updater_STRINGS_H
#define Updater_STRINGS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef  Updater_BASIC_CODE
#undef  Updater_BASIC
#undef  Updater_CODE
#define Updater_BASIC
#define Updater_CODE
#endif

#ifdef  Updater_BASIC
#undef  Updater_ARRAY
#undef  Updater_BLOCK
#define Updater_ARRAY
#define Updater_BLOCK
#endif

#ifdef  Updater_ARRAY
#undef  Updater_NUMBERS
#undef  Updater_STRINGS
#define Updater_NUMBERS
#define Updater_STRINGS
#endif

#ifdef  Updater_BLOCK
#undef  Updater_STRINGS
#define Updater_STRINGS
#endif


#ifdef Updater_NUMBERS

#define MSGID_TITLENAME 0
#define MSGID_MENU_PROJECT 1000
#define MSGID_MENU_PROJECT_ABOUT 1001
#define MSGID_MENU_PROJECT_ABOUTMUI 1002
#define MSGID_MENU_PROJECT_QUIT 1003
#define MSGID_MENU_PROJECT_QUIT_SHORT 1004
#define MSGID_ABOUTREQOK 2000
#define MSGID_ABOUTREQFORMAT 2001
#define MSGID_COMPONENTLIST_DIRECTORY 3000
#define MSGID_COMPONENTLIST_FILE 3001
#define MSGID_COMPONENTLIST_LATEST_VERSION 3002
#define MSGID_COMPONENTLIST_INSTALLED_VERSION 3003
#define MSGID_COMPONENTLIST_UPDATE 3004
#define MSGID_BUTTON_CHECKFORUPDATES 3500
#define MSGID_BUTTON_CHECKFORUPDATES_SHORT 3501
#define MSGID_BUTTON_CHECKFORUPDATES_HELP 3502
#define MSGID_BUTTON_STARTUPDATE 3503
#define MSGID_BUTTON_STARTUPDATE_SHORT 3504
#define MSGID_BUTTON_STARTUPDATE_HELP 3505
#define MSGID_BUTTON_SELECT_ALL 3506
#define MSGID_BUTTON_SELECT_ALL_SHORT 3507
#define MSGID_BUTTON_SELECT_ALL_HELP 3508
#define MSGID_BUTTON_DESELECT_ALL 3509
#define MSGID_BUTTON_DESELECT_ALL_SHORT 3510
#define MSGID_BUTTON_DESELECT_ALL_HELP 3511
#define MSGID_TEXT_SELECTED_COUNT 3512
#define MSGID_REGISTER_MAIN 3513
#define MSGID_REGISTER_CONFIGURATION 3514
#define MSGID_REGISTER_LOG 3515
#define MSGID_GROUP_PROXY 3516
#define MSGID_CHECK_USE_PROXY 3517
#define MSGID_CHECK_USE_PROXY_SHORTHELP 3518
#define MSGID_STRING_PROXY_ADDR 3519
#define MSGID_STRING_PROXY_ADDR_SHORTHELP 3520
#define MSGID_STRING_PROXY_PORT 3521
#define MSGID_STRING_PROXY_PORT_SHORTHELP 3522
#define MSGID_CHECK_USE_PROXYAUTH 3523
#define MSGID_CHECK_USE_PROXYAUTH_SHORTHELP 3524
#define MSGID_STRING_PROXY_USERNAME 3525
#define MSGID_STRING_PROXY_USERNAME_SHORTHELP 3526
#define MSGID_STRING_PROXY_PASSWD 3527
#define MSGID_STRING_PROXY_PASSWD_SHORTHELP 3528
#define MSGID_REQFORMAT_OPENSSL_ERROR 3529
#define MSGID_REQFORMAT_NO_UPDATES_FOUND 3530
#define MSGID_CHECK_SHOW_ALL_COMPONENTS 3531
#define MSGID_CHECK_SHOW_ALL_COMPONENTS_SHORTHELP 3532
#define MSGID_CHECK_ASK_EVERY_UPDATE 3533
#define MSGID_CHECK_ASK_EVERY_UPDATE_SHORTHELP 3534
#define MSGID_ABOUTREQ_YES_NO_ABORT 3535
#define MSGID_REQFORMAT_ASK_UPDATE 3536
#define MSGID_ERROR_DOWNLOADING_UPDATE 3537
#define MSGID_REQ_CONTINUE_ABORT 3538
#define MSGID_ERROR_LHA1 3539
#define MSGID_ERROR_LHA2 3540
#define MSGID_ERROR_OPEN_LHAFILE 3541
#define MSGID_STRING_SCALOS_SERVER 3542
#define MSGID_STRING_SCALOS_SERVER_SHORTHELP 3543
#define MSGID_REQ_INSTALL_UPDATES_GADGETS 3544
#define MSGID_REQ_INSTALL_UPDATES 3545
#define MSGID_GROUP_LOG 3546
#define MSGID_POPASL_TEMPPATH 3547
#define MSGID_GROUP_MISCELLANEOUS 3548
#define MSGID_BUTTON_CLEAR_LOG 3549
#define MSGID_BUTTON_CLEAR_LOG_SHORT 3550
#define MSGID_BUTTON_CLEAR_LOG_SHORTHELP 3551
#define MSGID_BUTTON_SAVE_LOG 3552
#define MSGID_BUTTON_SAVE_LOG_SHORT 3553
#define MSGID_BUTTON_SAVE_LOG_SHORTHELP 3554
#define MSGID_SAVE_LOG_ASLTITLE 3555
#define MSG_PROGRESS_RESOLVING_NAME 4000
#define MSG_PROGRESS_CONNECTING 4001
#define MSG_PROGRESS_SEND_HTTP_REQUEST 4002
#define MSG_PROGRESS_PROCESS_HTTP_RESPONSE 4003
#define MSG_PROGRESS_DONE 4004
#define MSG_PROGRESS_FAILURE 4005
#define MSG_PROGRESS_START_UPDATE 4006
#define MSG_PROGRESS_DOWNLOAD_UPDATE 4007
#define MSG_PROGRESS_FAILED 4008
#define MSG_PROGRESS_VERIFY_SIGNATURE 4009
#define MSGID_MENU_COMPONENTS_TITLE 5000
#define MSGID_MENU_COMPONENTS_SELECT_ALL 5001
#define MSGID_MENU_COMPONENTS_SELECT_NONE 5002
#define MSGID_LOG_READ_VERSIONINFO 5500
#define MSGID_LOG_ERROR_READ_VERSIONINFO 5501
#define MSGID_LOG_CHECKING_VERSIONS 5502
#define MSGID_LOG_UPDATES_FOUND 5503
#define MSGID_LOG_UPDATING 5504
#define MSGID_LOG_DOWNLOAD_PACKAGE 5505
#define MSGID_LOG_VERIFY_SIGNATURE 5506
#define MSGID_LOG_VERIFY_FAIL_SIGNATURE 5507
#define MSGID_LOG_DOWNLOAD_FAILED_PACKAGE 5508
#define MSGID_LOG_SKIP_PACKAGE 5509
#define MSGID_LOG_UNPACK_PACKAGE 5510
#define MSGID_LOG_UNPACK_FAIL_PACKAGE 5511
#define MSGID_LOG_UNPACK_CATALOGS_FAIL_PACKAGE 5512
#define MSGID_LOG_ERROR_OPEN_LHAFILE 5513
#define MSGID_LOG_ERROR_OPEN_INPUTFD 5514
#define MSGID_STARTUP_FAILURE 6000
#define MSGID_STARTUP_RETRY_QUIT_GAD 6001
#define MSGID_STARTUP_MCC_NOT_FOUND 6002
#define MSGID_STARTUP_OLD_MCC 6003
#define MSGID_STARTUP_MCC_IN_USE 6004
#define MSGID_AMISSL_LIBOPEN_FAIL_AMISSLMASTER 6100
#define MSGID_AMISSL_FAIL_INITAMISSLMASTER 6101
#define MSGID_AMISSL_FAIL_OPENAMISSL 6102
#define MSGID_AMISSL_FAIL_AMISSL_INTERFACE 6103
#define MSGID_AMISSL_FAIL_AMISSLMASTER_INTERFACE 6104
#define MSGID_AMISSL_FAIL_INITAMISSL 6105
#define MSGID_AMISSL_FAIL_REQ_FORMAT 6106

#endif /* Updater_NUMBERS */


/****************************************************************************/


#ifdef Updater_STRINGS

#define MSGID_TITLENAME_STR "Scalos Updater"
#define MSGID_MENU_PROJECT_STR "Project"
#define MSGID_MENU_PROJECT_ABOUT_STR "About..."
#define MSGID_MENU_PROJECT_ABOUTMUI_STR "About MUI..."
#define MSGID_MENU_PROJECT_QUIT_STR "Quit"
#define MSGID_MENU_PROJECT_QUIT_SHORT_STR "Q"
#define MSGID_ABOUTREQOK_STR "_OK"
#define MSGID_ABOUTREQFORMAT_STR "\033c\033bScalos Updater.module V%ld.%ld\033n\n"\
	"%s\n"\
	"� 2009%s The Scalos Team"
#define MSGID_COMPONENTLIST_DIRECTORY_STR "Directory"
#define MSGID_COMPONENTLIST_FILE_STR "Filename"
#define MSGID_COMPONENTLIST_LATEST_VERSION_STR "Latest"
#define MSGID_COMPONENTLIST_INSTALLED_VERSION_STR "Installed"
#define MSGID_COMPONENTLIST_UPDATE_STR "Update"
#define MSGID_BUTTON_CHECKFORUPDATES_STR "Check For Updates"
#define MSGID_BUTTON_CHECKFORUPDATES_SHORT_STR "C"
#define MSGID_BUTTON_CHECKFORUPDATES_HELP_STR "...TBD..."
#define MSGID_BUTTON_STARTUPDATE_STR "Begin Update"
#define MSGID_BUTTON_STARTUPDATE_SHORT_STR "B"
#define MSGID_BUTTON_STARTUPDATE_HELP_STR "...TBD..."
#define MSGID_BUTTON_SELECT_ALL_STR "Select All"
#define MSGID_BUTTON_SELECT_ALL_SHORT_STR "A"
#define MSGID_BUTTON_SELECT_ALL_HELP_STR "...TBD..."
#define MSGID_BUTTON_DESELECT_ALL_STR "Deselect All"
#define MSGID_BUTTON_DESELECT_ALL_SHORT_STR "D"
#define MSGID_BUTTON_DESELECT_ALL_HELP_STR "...TBD..."
#define MSGID_TEXT_SELECTED_COUNT_STR "%ld Selected"
#define MSGID_REGISTER_MAIN_STR "Main"
#define MSGID_REGISTER_CONFIGURATION_STR "Configuration"
#define MSGID_REGISTER_LOG_STR "Log"
#define MSGID_GROUP_PROXY_STR "Proxy Settings"
#define MSGID_CHECK_USE_PROXY_STR "Use Proxy?"
#define MSGID_CHECK_USE_PROXY_SHORTHELP_STR "...TBD..."
#define MSGID_STRING_PROXY_ADDR_STR "Address:"
#define MSGID_STRING_PROXY_ADDR_SHORTHELP_STR "...TBD..."
#define MSGID_STRING_PROXY_PORT_STR "Port:"
#define MSGID_STRING_PROXY_PORT_SHORTHELP_STR "...TBD..."
#define MSGID_CHECK_USE_PROXYAUTH_STR "Proxy requires authentication?"
#define MSGID_CHECK_USE_PROXYAUTH_SHORTHELP_STR "...TBD..."
#define MSGID_STRING_PROXY_USERNAME_STR "User Name:"
#define MSGID_STRING_PROXY_USERNAME_SHORTHELP_STR "...TBD..."
#define MSGID_STRING_PROXY_PASSWD_STR "Password:"
#define MSGID_STRING_PROXY_PASSWD_SHORTHELP_STR "...TBD..."
#define MSGID_REQFORMAT_OPENSSL_ERROR_STR "In function \033b%s\033n,\n"\
	"OpenSSL call \033b%s\033n failed:\n"\
	"%s"
#define MSGID_REQFORMAT_NO_UPDATES_FOUND_STR "No Updates found for %s.\n"\
	"You already have the most recent\n"\
	"Scalos components installed."
#define MSGID_CHECK_SHOW_ALL_COMPONENTS_STR "Show up-to-date components?"
#define MSGID_CHECK_SHOW_ALL_COMPONENTS_SHORTHELP_STR "...TBD..."
#define MSGID_CHECK_ASK_EVERY_UPDATE_STR "Ask about every component update?"
#define MSGID_CHECK_ASK_EVERY_UPDATE_SHORTHELP_STR "...TBD..."
#define MSGID_ABOUTREQ_YES_NO_ABORT_STR "_Yes|_No|_Abort"
#define MSGID_REQFORMAT_ASK_UPDATE_STR "Are you sure you want to update\n"\
	"\"%s\" to version %ld.%ld\n"\
	"(Installed Version: %ld.%ld) ?"
#define MSGID_ERROR_DOWNLOADING_UPDATE_STR "Error downloading update package \"%s\"\n"\
	"%s"
#define MSGID_REQ_CONTINUE_ABORT_STR "_Continue|_Abort"
#define MSGID_ERROR_LHA1_STR "Error: LHA failed to unpack \"%s\"."
#define MSGID_ERROR_LHA2_STR "Error: LHA failed to unpack\n"\
	"catalog files for \"%s\"."
#define MSGID_ERROR_OPEN_LHAFILE_STR "Error: failed to open temporary\n"\
	"archive \"%s\" for updating \"%s\"."
#define MSGID_STRING_SCALOS_SERVER_STR "Scalos Update Server"
#define MSGID_STRING_SCALOS_SERVER_SHORTHELP_STR "...TBD..."
#define MSGID_REQ_INSTALL_UPDATES_GADGETS_STR "_Update|_Abort"
#define MSGID_REQ_INSTALL_UPDATES_STR "Are you sure you want to install\n"\
	"%ld Scalos updates now?"
#define MSGID_GROUP_LOG_STR "Update Log"
#define MSGID_POPASL_TEMPPATH_STR "Path for Temporary Files:"
#define MSGID_GROUP_MISCELLANEOUS_STR "Miscellaneous"
#define MSGID_BUTTON_CLEAR_LOG_STR "Clear Log"
#define MSGID_BUTTON_CLEAR_LOG_SHORT_STR "C"
#define MSGID_BUTTON_CLEAR_LOG_SHORTHELP_STR "...TBD.."
#define MSGID_BUTTON_SAVE_LOG_STR "Save Log..."
#define MSGID_BUTTON_SAVE_LOG_SHORT_STR "S"
#define MSGID_BUTTON_SAVE_LOG_SHORTHELP_STR "...TBD.."
#define MSGID_SAVE_LOG_ASLTITLE_STR "Save Update Log..."
#define MSG_PROGRESS_RESOLVING_NAME_STR "Resolving name %s..."
#define MSG_PROGRESS_CONNECTING_STR "Connecting to %s..."
#define MSG_PROGRESS_SEND_HTTP_REQUEST_STR "Sending HTTP Request..."
#define MSG_PROGRESS_PROCESS_HTTP_RESPONSE_STR "Processing HTTP Response..."
#define MSG_PROGRESS_DONE_STR "Finished!"
#define MSG_PROGRESS_FAILURE_STR "Failed to connect to %s : %s"
#define MSG_PROGRESS_START_UPDATE_STR "Starting Update..."
#define MSG_PROGRESS_DOWNLOAD_UPDATE_STR "Downloading update %s..."
#define MSG_PROGRESS_FAILED_STR "Failed."
#define MSG_PROGRESS_VERIFY_SIGNATURE_STR "Verifying signature..."
#define MSGID_MENU_COMPONENTS_TITLE_STR "Components"
#define MSGID_MENU_COMPONENTS_SELECT_ALL_STR "Select All"
#define MSGID_MENU_COMPONENTS_SELECT_NONE_STR "Deselect All"
#define MSGID_LOG_READ_VERSIONINFO_STR "Reading version information from Scalos update server %s.\n"
#define MSGID_LOG_ERROR_READ_VERSIONINFO_STR "Failed to get version information from Scalos update server %s : %s.\n"
#define MSGID_LOG_CHECKING_VERSIONS_STR "Processing information about available updates.\n"
#define MSGID_LOG_UPDATES_FOUND_STR "Found %ld components to update.\n"
#define MSGID_LOG_UPDATING_STR "Updating component %ld of %ld.\n"
#define MSGID_LOG_DOWNLOAD_PACKAGE_STR "Downloading update package for %s\n."
#define MSGID_LOG_VERIFY_SIGNATURE_STR "Verifying signature of %s update package\n."
#define MSGID_LOG_VERIFY_FAIL_SIGNATURE_STR "Failed to update %s - invalid signature!\n"
#define MSGID_LOG_DOWNLOAD_FAILED_PACKAGE_STR "Failed to download update package for %s - %s.\n"
#define MSGID_LOG_SKIP_PACKAGE_STR "Skipping update of %s, due to user's request.\n"
#define MSGID_LOG_UNPACK_PACKAGE_STR "Unpacking update package for %s.\n"
#define MSGID_LOG_UNPACK_FAIL_PACKAGE_STR "Failed to unpack update for %s.\n"
#define MSGID_LOG_UNPACK_CATALOGS_FAIL_PACKAGE_STR "Failed to unpack language catalogs for %s.\n"
#define MSGID_LOG_ERROR_OPEN_LHAFILE_STR "Error: failed to open temporary archive \"%s\" for updating \"%s\".\n"
#define MSGID_LOG_ERROR_OPEN_INPUTFD_STR "Error: failed to open input file handle for lha: %s\n"
#define MSGID_STARTUP_FAILURE_STR "Updater Module startup failed"
#define MSGID_STARTUP_RETRY_QUIT_GAD_STR "Try again|Quit"
#define MSGID_STARTUP_MCC_NOT_FOUND_STR "Couldn't open MUI custom class '%s' V%lu.%lu.\n"\
	"The class is not installed."
#define MSGID_STARTUP_OLD_MCC_STR "Couldn't open MUI custom class '%s' V%lu.%lu.\n"\
	"\n"\
	"Currently installed is V%lu.%lu, please upgrade!"
#define MSGID_STARTUP_MCC_IN_USE_STR "Couldn't open MUI custom class '%s' V%lu.%lu.\n"\
	"%lu.%lu is currently in use by other applications.\n"\
	"\n"\
	"Once you have installed the required version,\n"\
	"close all MUI programs, make sure the old class\n"\
	"is flushed from memory and try again."
#define MSGID_AMISSL_LIBOPEN_FAIL_AMISSLMASTER_STR "Failed to open amisslmaster.library"
#define MSGID_AMISSL_FAIL_INITAMISSLMASTER_STR "InitAmiSSLMaster failed, probably your AmiSSL installation is too old"
#define MSGID_AMISSL_FAIL_OPENAMISSL_STR "OpenAmiSSL failed"
#define MSGID_AMISSL_FAIL_AMISSL_INTERFACE_STR "Couldn't get AmiSSL interface"
#define MSGID_AMISSL_FAIL_AMISSLMASTER_INTERFACE_STR "Couldn't get AmiSSLMaster interface"
#define MSGID_AMISSL_FAIL_INITAMISSL_STR "InitAmiSSL failed"
#define MSGID_AMISSL_FAIL_REQ_FORMAT_STR "Could not initialize AmiSSL:\n"\
	"%s\n"\
	"Sorry, but Updater needs a working AmiSSL V3 installation\n"\
	"(http://www.heightanxiety.com/AmiSSL)/\n"\
	"to verify the signatures of the downloaded files."

#endif /* Updater_STRINGS */


/****************************************************************************/


#ifdef Updater_ARRAY

struct Updater_ArrayType
{
    LONG   cca_ID;
    CONST_STRPTR cca_Str;
};

static const struct Updater_ArrayType Updater_Array[] =
{
    { MSGID_TITLENAME, MSGID_TITLENAME_STR },
    { MSGID_MENU_PROJECT, MSGID_MENU_PROJECT_STR },
    { MSGID_MENU_PROJECT_ABOUT, MSGID_MENU_PROJECT_ABOUT_STR },
    { MSGID_MENU_PROJECT_ABOUTMUI, MSGID_MENU_PROJECT_ABOUTMUI_STR },
    { MSGID_MENU_PROJECT_QUIT, MSGID_MENU_PROJECT_QUIT_STR },
    { MSGID_MENU_PROJECT_QUIT_SHORT, MSGID_MENU_PROJECT_QUIT_SHORT_STR },
    { MSGID_ABOUTREQOK, MSGID_ABOUTREQOK_STR },
    { MSGID_ABOUTREQFORMAT, MSGID_ABOUTREQFORMAT_STR },
    { MSGID_COMPONENTLIST_DIRECTORY, MSGID_COMPONENTLIST_DIRECTORY_STR },
    { MSGID_COMPONENTLIST_FILE, MSGID_COMPONENTLIST_FILE_STR },
    { MSGID_COMPONENTLIST_LATEST_VERSION, MSGID_COMPONENTLIST_LATEST_VERSION_STR },
    { MSGID_COMPONENTLIST_INSTALLED_VERSION, MSGID_COMPONENTLIST_INSTALLED_VERSION_STR },
    { MSGID_COMPONENTLIST_UPDATE, MSGID_COMPONENTLIST_UPDATE_STR },
    { MSGID_BUTTON_CHECKFORUPDATES, MSGID_BUTTON_CHECKFORUPDATES_STR },
    { MSGID_BUTTON_CHECKFORUPDATES_SHORT, MSGID_BUTTON_CHECKFORUPDATES_SHORT_STR },
    { MSGID_BUTTON_CHECKFORUPDATES_HELP, MSGID_BUTTON_CHECKFORUPDATES_HELP_STR },
    { MSGID_BUTTON_STARTUPDATE, MSGID_BUTTON_STARTUPDATE_STR },
    { MSGID_BUTTON_STARTUPDATE_SHORT, MSGID_BUTTON_STARTUPDATE_SHORT_STR },
    { MSGID_BUTTON_STARTUPDATE_HELP, MSGID_BUTTON_STARTUPDATE_HELP_STR },
    { MSGID_BUTTON_SELECT_ALL, MSGID_BUTTON_SELECT_ALL_STR },
    { MSGID_BUTTON_SELECT_ALL_SHORT, MSGID_BUTTON_SELECT_ALL_SHORT_STR },
    { MSGID_BUTTON_SELECT_ALL_HELP, MSGID_BUTTON_SELECT_ALL_HELP_STR },
    { MSGID_BUTTON_DESELECT_ALL, MSGID_BUTTON_DESELECT_ALL_STR },
    { MSGID_BUTTON_DESELECT_ALL_SHORT, MSGID_BUTTON_DESELECT_ALL_SHORT_STR },
    { MSGID_BUTTON_DESELECT_ALL_HELP, MSGID_BUTTON_DESELECT_ALL_HELP_STR },
    { MSGID_TEXT_SELECTED_COUNT, MSGID_TEXT_SELECTED_COUNT_STR },
    { MSGID_REGISTER_MAIN, MSGID_REGISTER_MAIN_STR },
    { MSGID_REGISTER_CONFIGURATION, MSGID_REGISTER_CONFIGURATION_STR },
    { MSGID_REGISTER_LOG, MSGID_REGISTER_LOG_STR },
    { MSGID_GROUP_PROXY, MSGID_GROUP_PROXY_STR },
    { MSGID_CHECK_USE_PROXY, MSGID_CHECK_USE_PROXY_STR },
    { MSGID_CHECK_USE_PROXY_SHORTHELP, MSGID_CHECK_USE_PROXY_SHORTHELP_STR },
    { MSGID_STRING_PROXY_ADDR, MSGID_STRING_PROXY_ADDR_STR },
    { MSGID_STRING_PROXY_ADDR_SHORTHELP, MSGID_STRING_PROXY_ADDR_SHORTHELP_STR },
    { MSGID_STRING_PROXY_PORT, MSGID_STRING_PROXY_PORT_STR },
    { MSGID_STRING_PROXY_PORT_SHORTHELP, MSGID_STRING_PROXY_PORT_SHORTHELP_STR },
    { MSGID_CHECK_USE_PROXYAUTH, MSGID_CHECK_USE_PROXYAUTH_STR },
    { MSGID_CHECK_USE_PROXYAUTH_SHORTHELP, MSGID_CHECK_USE_PROXYAUTH_SHORTHELP_STR },
    { MSGID_STRING_PROXY_USERNAME, MSGID_STRING_PROXY_USERNAME_STR },
    { MSGID_STRING_PROXY_USERNAME_SHORTHELP, MSGID_STRING_PROXY_USERNAME_SHORTHELP_STR },
    { MSGID_STRING_PROXY_PASSWD, MSGID_STRING_PROXY_PASSWD_STR },
    { MSGID_STRING_PROXY_PASSWD_SHORTHELP, MSGID_STRING_PROXY_PASSWD_SHORTHELP_STR },
    { MSGID_REQFORMAT_OPENSSL_ERROR, MSGID_REQFORMAT_OPENSSL_ERROR_STR },
    { MSGID_REQFORMAT_NO_UPDATES_FOUND, MSGID_REQFORMAT_NO_UPDATES_FOUND_STR },
    { MSGID_CHECK_SHOW_ALL_COMPONENTS, MSGID_CHECK_SHOW_ALL_COMPONENTS_STR },
    { MSGID_CHECK_SHOW_ALL_COMPONENTS_SHORTHELP, MSGID_CHECK_SHOW_ALL_COMPONENTS_SHORTHELP_STR },
    { MSGID_CHECK_ASK_EVERY_UPDATE, MSGID_CHECK_ASK_EVERY_UPDATE_STR },
    { MSGID_CHECK_ASK_EVERY_UPDATE_SHORTHELP, MSGID_CHECK_ASK_EVERY_UPDATE_SHORTHELP_STR },
    { MSGID_ABOUTREQ_YES_NO_ABORT, MSGID_ABOUTREQ_YES_NO_ABORT_STR },
    { MSGID_REQFORMAT_ASK_UPDATE, MSGID_REQFORMAT_ASK_UPDATE_STR },
    { MSGID_ERROR_DOWNLOADING_UPDATE, MSGID_ERROR_DOWNLOADING_UPDATE_STR },
    { MSGID_REQ_CONTINUE_ABORT, MSGID_REQ_CONTINUE_ABORT_STR },
    { MSGID_ERROR_LHA1, MSGID_ERROR_LHA1_STR },
    { MSGID_ERROR_LHA2, MSGID_ERROR_LHA2_STR },
    { MSGID_ERROR_OPEN_LHAFILE, MSGID_ERROR_OPEN_LHAFILE_STR },
    { MSGID_STRING_SCALOS_SERVER, MSGID_STRING_SCALOS_SERVER_STR },
    { MSGID_STRING_SCALOS_SERVER_SHORTHELP, MSGID_STRING_SCALOS_SERVER_SHORTHELP_STR },
    { MSGID_REQ_INSTALL_UPDATES_GADGETS, MSGID_REQ_INSTALL_UPDATES_GADGETS_STR },
    { MSGID_REQ_INSTALL_UPDATES, MSGID_REQ_INSTALL_UPDATES_STR },
    { MSGID_GROUP_LOG, MSGID_GROUP_LOG_STR },
    { MSGID_POPASL_TEMPPATH, MSGID_POPASL_TEMPPATH_STR },
    { MSGID_GROUP_MISCELLANEOUS, MSGID_GROUP_MISCELLANEOUS_STR },
    { MSGID_BUTTON_CLEAR_LOG, MSGID_BUTTON_CLEAR_LOG_STR },
    { MSGID_BUTTON_CLEAR_LOG_SHORT, MSGID_BUTTON_CLEAR_LOG_SHORT_STR },
    { MSGID_BUTTON_CLEAR_LOG_SHORTHELP, MSGID_BUTTON_CLEAR_LOG_SHORTHELP_STR },
    { MSGID_BUTTON_SAVE_LOG, MSGID_BUTTON_SAVE_LOG_STR },
    { MSGID_BUTTON_SAVE_LOG_SHORT, MSGID_BUTTON_SAVE_LOG_SHORT_STR },
    { MSGID_BUTTON_SAVE_LOG_SHORTHELP, MSGID_BUTTON_SAVE_LOG_SHORTHELP_STR },
    { MSGID_SAVE_LOG_ASLTITLE, MSGID_SAVE_LOG_ASLTITLE_STR },
    { MSG_PROGRESS_RESOLVING_NAME, MSG_PROGRESS_RESOLVING_NAME_STR },
    { MSG_PROGRESS_CONNECTING, MSG_PROGRESS_CONNECTING_STR },
    { MSG_PROGRESS_SEND_HTTP_REQUEST, MSG_PROGRESS_SEND_HTTP_REQUEST_STR },
    { MSG_PROGRESS_PROCESS_HTTP_RESPONSE, MSG_PROGRESS_PROCESS_HTTP_RESPONSE_STR },
    { MSG_PROGRESS_DONE, MSG_PROGRESS_DONE_STR },
    { MSG_PROGRESS_FAILURE, MSG_PROGRESS_FAILURE_STR },
    { MSG_PROGRESS_START_UPDATE, MSG_PROGRESS_START_UPDATE_STR },
    { MSG_PROGRESS_DOWNLOAD_UPDATE, MSG_PROGRESS_DOWNLOAD_UPDATE_STR },
    { MSG_PROGRESS_FAILED, MSG_PROGRESS_FAILED_STR },
    { MSG_PROGRESS_VERIFY_SIGNATURE, MSG_PROGRESS_VERIFY_SIGNATURE_STR },
    { MSGID_MENU_COMPONENTS_TITLE, MSGID_MENU_COMPONENTS_TITLE_STR },
    { MSGID_MENU_COMPONENTS_SELECT_ALL, MSGID_MENU_COMPONENTS_SELECT_ALL_STR },
    { MSGID_MENU_COMPONENTS_SELECT_NONE, MSGID_MENU_COMPONENTS_SELECT_NONE_STR },
    { MSGID_LOG_READ_VERSIONINFO, MSGID_LOG_READ_VERSIONINFO_STR },
    { MSGID_LOG_ERROR_READ_VERSIONINFO, MSGID_LOG_ERROR_READ_VERSIONINFO_STR },
    { MSGID_LOG_CHECKING_VERSIONS, MSGID_LOG_CHECKING_VERSIONS_STR },
    { MSGID_LOG_UPDATES_FOUND, MSGID_LOG_UPDATES_FOUND_STR },
    { MSGID_LOG_UPDATING, MSGID_LOG_UPDATING_STR },
    { MSGID_LOG_DOWNLOAD_PACKAGE, MSGID_LOG_DOWNLOAD_PACKAGE_STR },
    { MSGID_LOG_VERIFY_SIGNATURE, MSGID_LOG_VERIFY_SIGNATURE_STR },
    { MSGID_LOG_VERIFY_FAIL_SIGNATURE, MSGID_LOG_VERIFY_FAIL_SIGNATURE_STR },
    { MSGID_LOG_DOWNLOAD_FAILED_PACKAGE, MSGID_LOG_DOWNLOAD_FAILED_PACKAGE_STR },
    { MSGID_LOG_SKIP_PACKAGE, MSGID_LOG_SKIP_PACKAGE_STR },
    { MSGID_LOG_UNPACK_PACKAGE, MSGID_LOG_UNPACK_PACKAGE_STR },
    { MSGID_LOG_UNPACK_FAIL_PACKAGE, MSGID_LOG_UNPACK_FAIL_PACKAGE_STR },
    { MSGID_LOG_UNPACK_CATALOGS_FAIL_PACKAGE, MSGID_LOG_UNPACK_CATALOGS_FAIL_PACKAGE_STR },
    { MSGID_LOG_ERROR_OPEN_LHAFILE, MSGID_LOG_ERROR_OPEN_LHAFILE_STR },
    { MSGID_LOG_ERROR_OPEN_INPUTFD, MSGID_LOG_ERROR_OPEN_INPUTFD_STR },
    { MSGID_STARTUP_FAILURE, MSGID_STARTUP_FAILURE_STR },
    { MSGID_STARTUP_RETRY_QUIT_GAD, MSGID_STARTUP_RETRY_QUIT_GAD_STR },
    { MSGID_STARTUP_MCC_NOT_FOUND, MSGID_STARTUP_MCC_NOT_FOUND_STR },
    { MSGID_STARTUP_OLD_MCC, MSGID_STARTUP_OLD_MCC_STR },
    { MSGID_STARTUP_MCC_IN_USE, MSGID_STARTUP_MCC_IN_USE_STR },
    { MSGID_AMISSL_LIBOPEN_FAIL_AMISSLMASTER, MSGID_AMISSL_LIBOPEN_FAIL_AMISSLMASTER_STR },
    { MSGID_AMISSL_FAIL_INITAMISSLMASTER, MSGID_AMISSL_FAIL_INITAMISSLMASTER_STR },
    { MSGID_AMISSL_FAIL_OPENAMISSL, MSGID_AMISSL_FAIL_OPENAMISSL_STR },
    { MSGID_AMISSL_FAIL_AMISSL_INTERFACE, MSGID_AMISSL_FAIL_AMISSL_INTERFACE_STR },
    { MSGID_AMISSL_FAIL_AMISSLMASTER_INTERFACE, MSGID_AMISSL_FAIL_AMISSLMASTER_INTERFACE_STR },
    { MSGID_AMISSL_FAIL_INITAMISSL, MSGID_AMISSL_FAIL_INITAMISSL_STR },
    { MSGID_AMISSL_FAIL_REQ_FORMAT, MSGID_AMISSL_FAIL_REQ_FORMAT_STR },
};


#endif /* Updater_ARRAY */


/****************************************************************************/


#ifdef Updater_BLOCK

static const char Updater_Block[] =
{

     "\x00\x00\x00\x00" "\x00\x0e"
    MSGID_TITLENAME_STR ""
     "\x00\x00\x03\xe8" "\x00\x08"
    MSGID_MENU_PROJECT_STR "\x00"
     "\x00\x00\x03\xe9" "\x00\x08"
    MSGID_MENU_PROJECT_ABOUT_STR ""
     "\x00\x00\x03\xea" "\x00\x0c"
    MSGID_MENU_PROJECT_ABOUTMUI_STR ""
     "\x00\x00\x03\xeb" "\x00\x04"
    MSGID_MENU_PROJECT_QUIT_STR ""
     "\x00\x00\x03\xec" "\x00\x02"
    MSGID_MENU_PROJECT_QUIT_SHORT_STR "\x00"
     "\x00\x00\x07\xd0" "\x00\x04"
    MSGID_ABOUTREQOK_STR "\x00"
     "\x00\x00\x07\xd1" "\x00\x40"
    MSGID_ABOUTREQFORMAT_STR ""
     "\x00\x00\x0b\xb8" "\x00\x0a"
    MSGID_COMPONENTLIST_DIRECTORY_STR "\x00"
     "\x00\x00\x0b\xb9" "\x00\x08"
    MSGID_COMPONENTLIST_FILE_STR ""
     "\x00\x00\x0b\xba" "\x00\x06"
    MSGID_COMPONENTLIST_LATEST_VERSION_STR ""
     "\x00\x00\x0b\xbb" "\x00\x0a"
    MSGID_COMPONENTLIST_INSTALLED_VERSION_STR "\x00"
     "\x00\x00\x0b\xbc" "\x00\x06"
    MSGID_COMPONENTLIST_UPDATE_STR ""
     "\x00\x00\x0d\xac" "\x00\x12"
    MSGID_BUTTON_CHECKFORUPDATES_STR "\x00"
     "\x00\x00\x0d\xad" "\x00\x02"
    MSGID_BUTTON_CHECKFORUPDATES_SHORT_STR "\x00"
     "\x00\x00\x0d\xae" "\x00\x0a"
    MSGID_BUTTON_CHECKFORUPDATES_HELP_STR "\x00"
     "\x00\x00\x0d\xaf" "\x00\x0c"
    MSGID_BUTTON_STARTUPDATE_STR ""
     "\x00\x00\x0d\xb0" "\x00\x02"
    MSGID_BUTTON_STARTUPDATE_SHORT_STR "\x00"
     "\x00\x00\x0d\xb1" "\x00\x0a"
    MSGID_BUTTON_STARTUPDATE_HELP_STR "\x00"
     "\x00\x00\x0d\xb2" "\x00\x0a"
    MSGID_BUTTON_SELECT_ALL_STR ""
     "\x00\x00\x0d\xb3" "\x00\x02"
    MSGID_BUTTON_SELECT_ALL_SHORT_STR "\x00"
     "\x00\x00\x0d\xb4" "\x00\x0a"
    MSGID_BUTTON_SELECT_ALL_HELP_STR "\x00"
     "\x00\x00\x0d\xb5" "\x00\x0c"
    MSGID_BUTTON_DESELECT_ALL_STR ""
     "\x00\x00\x0d\xb6" "\x00\x02"
    MSGID_BUTTON_DESELECT_ALL_SHORT_STR "\x00"
     "\x00\x00\x0d\xb7" "\x00\x0a"
    MSGID_BUTTON_DESELECT_ALL_HELP_STR "\x00"
     "\x00\x00\x0d\xb8" "\x00\x0c"
    MSGID_TEXT_SELECTED_COUNT_STR ""
     "\x00\x00\x0d\xb9" "\x00\x04"
    MSGID_REGISTER_MAIN_STR ""
     "\x00\x00\x0d\xba" "\x00\x0e"
    MSGID_REGISTER_CONFIGURATION_STR "\x00"
     "\x00\x00\x0d\xbb" "\x00\x04"
    MSGID_REGISTER_LOG_STR "\x00"
     "\x00\x00\x0d\xbc" "\x00\x0e"
    MSGID_GROUP_PROXY_STR ""
     "\x00\x00\x0d\xbd" "\x00\x0a"
    MSGID_CHECK_USE_PROXY_STR ""
     "\x00\x00\x0d\xbe" "\x00\x0a"
    MSGID_CHECK_USE_PROXY_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xbf" "\x00\x08"
    MSGID_STRING_PROXY_ADDR_STR ""
     "\x00\x00\x0d\xc0" "\x00\x0a"
    MSGID_STRING_PROXY_ADDR_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xc1" "\x00\x06"
    MSGID_STRING_PROXY_PORT_STR "\x00"
     "\x00\x00\x0d\xc2" "\x00\x0a"
    MSGID_STRING_PROXY_PORT_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xc3" "\x00\x1e"
    MSGID_CHECK_USE_PROXYAUTH_STR ""
     "\x00\x00\x0d\xc4" "\x00\x0a"
    MSGID_CHECK_USE_PROXYAUTH_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xc5" "\x00\x0a"
    MSGID_STRING_PROXY_USERNAME_STR ""
     "\x00\x00\x0d\xc6" "\x00\x0a"
    MSGID_STRING_PROXY_USERNAME_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xc7" "\x00\x0a"
    MSGID_STRING_PROXY_PASSWD_STR "\x00"
     "\x00\x00\x0d\xc8" "\x00\x0a"
    MSGID_STRING_PROXY_PASSWD_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xc9" "\x00\x32"
    MSGID_REQFORMAT_OPENSSL_ERROR_STR ""
     "\x00\x00\x0d\xca" "\x00\x56"
    MSGID_REQFORMAT_NO_UPDATES_FOUND_STR ""
     "\x00\x00\x0d\xcb" "\x00\x1c"
    MSGID_CHECK_SHOW_ALL_COMPONENTS_STR "\x00"
     "\x00\x00\x0d\xcc" "\x00\x0a"
    MSGID_CHECK_SHOW_ALL_COMPONENTS_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xcd" "\x00\x22"
    MSGID_CHECK_ASK_EVERY_UPDATE_STR "\x00"
     "\x00\x00\x0d\xce" "\x00\x0a"
    MSGID_CHECK_ASK_EVERY_UPDATE_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xcf" "\x00\x10"
    MSGID_ABOUTREQ_YES_NO_ABORT_STR "\x00"
     "\x00\x00\x0d\xd0" "\x00\x56"
    MSGID_REQFORMAT_ASK_UPDATE_STR ""
     "\x00\x00\x0d\xd1" "\x00\x28"
    MSGID_ERROR_DOWNLOADING_UPDATE_STR ""
     "\x00\x00\x0d\xd2" "\x00\x10"
    MSGID_REQ_CONTINUE_ABORT_STR ""
     "\x00\x00\x0d\xd3" "\x00\x22"
    MSGID_ERROR_LHA1_STR "\x00"
     "\x00\x00\x0d\xd4" "\x00\x34"
    MSGID_ERROR_LHA2_STR "\x00"
     "\x00\x00\x0d\xd5" "\x00\x40"
    MSGID_ERROR_OPEN_LHAFILE_STR "\x00"
     "\x00\x00\x0d\xd6" "\x00\x14"
    MSGID_STRING_SCALOS_SERVER_STR ""
     "\x00\x00\x0d\xd7" "\x00\x0a"
    MSGID_STRING_SCALOS_SERVER_SHORTHELP_STR "\x00"
     "\x00\x00\x0d\xd8" "\x00\x0e"
    MSGID_REQ_INSTALL_UPDATES_GADGETS_STR ""
     "\x00\x00\x0d\xd9" "\x00\x38"
    MSGID_REQ_INSTALL_UPDATES_STR ""
     "\x00\x00\x0d\xda" "\x00\x0a"
    MSGID_GROUP_LOG_STR ""
     "\x00\x00\x0d\xdb" "\x00\x1a"
    MSGID_POPASL_TEMPPATH_STR "\x00"
     "\x00\x00\x0d\xdc" "\x00\x0e"
    MSGID_GROUP_MISCELLANEOUS_STR "\x00"
     "\x00\x00\x0d\xdd" "\x00\x0a"
    MSGID_BUTTON_CLEAR_LOG_STR "\x00"
     "\x00\x00\x0d\xde" "\x00\x02"
    MSGID_BUTTON_CLEAR_LOG_SHORT_STR "\x00"
     "\x00\x00\x0d\xdf" "\x00\x08"
    MSGID_BUTTON_CLEAR_LOG_SHORTHELP_STR ""
     "\x00\x00\x0d\xe0" "\x00\x0c"
    MSGID_BUTTON_SAVE_LOG_STR "\x00"
     "\x00\x00\x0d\xe1" "\x00\x02"
    MSGID_BUTTON_SAVE_LOG_SHORT_STR "\x00"
     "\x00\x00\x0d\xe2" "\x00\x08"
    MSGID_BUTTON_SAVE_LOG_SHORTHELP_STR ""
     "\x00\x00\x0d\xe3" "\x00\x12"
    MSGID_SAVE_LOG_ASLTITLE_STR ""
     "\x00\x00\x0f\xa0" "\x00\x14"
    MSG_PROGRESS_RESOLVING_NAME_STR ""
     "\x00\x00\x0f\xa1" "\x00\x14"
    MSG_PROGRESS_CONNECTING_STR "\x00"
     "\x00\x00\x0f\xa2" "\x00\x18"
    MSG_PROGRESS_SEND_HTTP_REQUEST_STR "\x00"
     "\x00\x00\x0f\xa3" "\x00\x1c"
    MSG_PROGRESS_PROCESS_HTTP_RESPONSE_STR "\x00"
     "\x00\x00\x0f\xa4" "\x00\x0a"
    MSG_PROGRESS_DONE_STR "\x00"
     "\x00\x00\x0f\xa5" "\x00\x1c"
    MSG_PROGRESS_FAILURE_STR ""
     "\x00\x00\x0f\xa6" "\x00\x12"
    MSG_PROGRESS_START_UPDATE_STR ""
     "\x00\x00\x0f\xa7" "\x00\x18"
    MSG_PROGRESS_DOWNLOAD_UPDATE_STR ""
     "\x00\x00\x0f\xa8" "\x00\x08"
    MSG_PROGRESS_FAILED_STR "\x00"
     "\x00\x00\x0f\xa9" "\x00\x16"
    MSG_PROGRESS_VERIFY_SIGNATURE_STR ""
     "\x00\x00\x13\x88" "\x00\x0a"
    MSGID_MENU_COMPONENTS_TITLE_STR ""
     "\x00\x00\x13\x89" "\x00\x0a"
    MSGID_MENU_COMPONENTS_SELECT_ALL_STR ""
     "\x00\x00\x13\x8a" "\x00\x0c"
    MSGID_MENU_COMPONENTS_SELECT_NONE_STR ""
     "\x00\x00\x15\x7c" "\x00\x3a"
    MSGID_LOG_READ_VERSIONINFO_STR ""
     "\x00\x00\x15\x7d" "\x00\x46"
    MSGID_LOG_ERROR_READ_VERSIONINFO_STR "\x00"
     "\x00\x00\x15\x7e" "\x00\x30"
    MSGID_LOG_CHECKING_VERSIONS_STR ""
     "\x00\x00\x15\x7f" "\x00\x20"
    MSGID_LOG_UPDATES_FOUND_STR ""
     "\x00\x00\x15\x80" "\x00\x20"
    MSGID_LOG_UPDATING_STR "\x00"
     "\x00\x00\x15\x81" "\x00\x24"
    MSGID_LOG_DOWNLOAD_PACKAGE_STR "\x00"
     "\x00\x00\x15\x82" "\x00\x2a"
    MSGID_LOG_VERIFY_SIGNATURE_STR ""
     "\x00\x00\x15\x83" "\x00\x2a"
    MSGID_LOG_VERIFY_FAIL_SIGNATURE_STR "\x00"
     "\x00\x00\x15\x84" "\x00\x30"
    MSGID_LOG_DOWNLOAD_FAILED_PACKAGE_STR "\x00"
     "\x00\x00\x15\x85" "\x00\x2e"
    MSGID_LOG_SKIP_PACKAGE_STR ""
     "\x00\x00\x15\x86" "\x00\x22"
    MSGID_LOG_UNPACK_PACKAGE_STR "\x00"
     "\x00\x00\x15\x87" "\x00\x20"
    MSGID_LOG_UNPACK_FAIL_PACKAGE_STR ""
     "\x00\x00\x15\x88" "\x00\x2c"
    MSGID_LOG_UNPACK_CATALOGS_FAIL_PACKAGE_STR "\x00"
     "\x00\x00\x15\x89" "\x00\x40"
    MSGID_LOG_ERROR_OPEN_LHAFILE_STR ""
     "\x00\x00\x15\x8a" "\x00\x34"
    MSGID_LOG_ERROR_OPEN_INPUTFD_STR ""
     "\x00\x00\x17\x70" "\x00\x1e"
    MSGID_STARTUP_FAILURE_STR "\x00"
     "\x00\x00\x17\x71" "\x00\x0e"
    MSGID_STARTUP_RETRY_QUIT_GAD_STR ""
     "\x00\x00\x17\x72" "\x00\x4a"
    MSGID_STARTUP_MCC_NOT_FOUND_STR "\x00"
     "\x00\x00\x17\x73" "\x00\x60"
    MSGID_STARTUP_OLD_MCC_STR "\x00"
     "\x00\x00\x17\x74" "\x00\xe6"
    MSGID_STARTUP_MCC_IN_USE_STR "\x00"
     "\x00\x00\x17\xd4" "\x00\x24"
    MSGID_AMISSL_LIBOPEN_FAIL_AMISSLMASTER_STR "\x00"
     "\x00\x00\x17\xd5" "\x00\x46"
    MSGID_AMISSL_FAIL_INITAMISSLMASTER_STR "\x00"
     "\x00\x00\x17\xd6" "\x00\x12"
    MSGID_AMISSL_FAIL_OPENAMISSL_STR "\x00"
     "\x00\x00\x17\xd7" "\x00\x1e"
    MSGID_AMISSL_FAIL_AMISSL_INTERFACE_STR "\x00"
     "\x00\x00\x17\xd8" "\x00\x24"
    MSGID_AMISSL_FAIL_AMISSLMASTER_INTERFACE_STR "\x00"
     "\x00\x00\x17\xd9" "\x00\x12"
    MSGID_AMISSL_FAIL_INITAMISSL_STR "\x00"
     "\x00\x00\x17\xda" "\x00\xb2"
    MSGID_AMISSL_FAIL_REQ_FORMAT_STR ""

};

#endif /* Updater_BLOCK */


/****************************************************************************/


#ifdef Updater_CODE

#ifndef PROTO_LOCALE_H
#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/locale.h>
#endif

#ifndef Updater_CODE_EXISTS
 #define Updater_CODE_EXISTS

CONST_STRPTR GetUpdaterString(struct Updater_LocaleInfo *li, LONG stringNum)
{
#ifndef __amigaos4__
    struct Library     *LocaleBase = li->li_LocaleBase;
#else
    struct LocaleIFace *ILocale    = li->li_ILocale;
#endif

    CONST_STRPTR  builtIn = "";
    ULONG n;

    for (n = 0; n < sizeof(Updater_Array) / sizeof(Updater_Array[0]); n++)
    {
        if (Updater_Array[n].cca_ID == stringNum)
        {
            builtIn = Updater_Array[n].cca_Str;
            break;
        }
    }

#ifndef __amigaos4__
    if (LocaleBase)
    {
        return GetCatalogStr(li->li_Catalog, stringNum, (STRPTR)builtIn);
    }
#else
    if (ILocale)
    {
#ifdef __USE_INLINE__
        return GetCatalogStr(li->li_Catalog, stringNum, (STRPTR)builtIn);
#else
        return ILocale->GetCatalogStr(li->li_Catalog, stringNum, (STRPTR)builtIn);
#endif
    }
#endif

    return builtIn;
}

#else

STRPTR GetUpdaterString(struct Updater_LocaleInfo *li, LONG stringNum);

#endif /* Updater_CODE_EXISTS */

#endif /* Updater_CODE */


/****************************************************************************/


#endif /* Updater_STRINGS_H */