#ifndef __NEWRES_H__
#define __NEWRES_H__

#include "config.h"

#define  SHMENUBAR RCDATA
#if !POCKETPC
	#undef HDS_HORZ
	#undef HDS_BUTTONS
	#undef HDS_HIDDEN

	#include <commctrl.h>
	// for MenuBar
	#define I_IMAGENONE		(-2)
	#define NOMENU			0xFFFF
	#define IDS_SHNEW		1
	#define IDM_SHAREDNEW        10
	#define IDM_SHAREDNEWDEFAULT 11

	// for Tab Control
	#define TCS_SCROLLOPPOSITE      0x0001   // assumes multiline tab
	#define TCS_BOTTOM              0x0002
	#define TCS_RIGHT               0x0002
	#define TCS_VERTICAL            0x0080
	#define TCS_MULTISELECT         0x0004  // allow multi-select in button mode
	#define TCS_FLATBUTTONS         0x0008
#endif //!POCKETPC

#if BE300
#define _WIN32_WCE_PSPC
#endif

#if defined(_WIN32_WCE)
#if !defined(_WIN32_WCE_PSPC)
#define _WIN32_WCE_HPC
#endif

#if defined(FORCEPSPC) && FORCEPSPC
#undef _WIN32_WCE_HPC
#undef _WIN32_WCE_PSPC
#define _WIN32_WCE_PSPC
#endif
#else
#define _WIN32_DESKTOP
#endif

#if defined(APSTUDIO_INVOKED)
#undef _WIN32_WCE_HPC
#undef _WIN32_WCE_PSPC
#undef _WIN32_WCE
#undef _WIN32_DESKTOP
#endif

#endif //__NEWRES_H__
