#ifndef __NEWRES_H__
#define __NEWRES_H__

#include "config.h"

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

#endif //__NEWRES_H__
