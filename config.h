#ifndef CONFIG_H
#define CONFIG_H

#if ((defined(_WIN32_WCE_PSPC) || defined(WIN32_PLATFORM_PSPC)) && (_WIN32_WCE>=300 && _WIN32_WCE<400)) && !BE300
#define POCKETPC 1
#endif

#if _WIN32_WCE>=400
#define _WIN32_WCE_PSPC
#endif

#if defined(_WIN32_WCE_PSPC) || BE300
#define PSPC 1
#else
#define HPC 1
#endif

#ifdef _WIN32_WCE
#define	HR_REG_PERM 0
#else
#define	HR_REG_PERM KEY_READ|KEY_WRITE
#endif

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

// default font
#define	DEF_FACE	  _T("Tahoma")
#define	DEF_SIZE	  12
#define	DEF_BOLD	  1
#define	DEF_CLEARTYPE	  1
#define	DEF_FONTHACK	  0

// default sizes
#define	DEF_INDENT	  10
#define	DEF_MARGINS	  3
#define	DEF_BOTTOM_MARGIN 6
#define	DEF_JUSTIFY	  1
#define	DEF_ORIENTATION	  0
#define	DEF_HYPHENATE	  0
#define	DEF_COLUMNS	  1
#define DEF_SHOWPROGRESS  1

// default settings
#define	DEF_USEDICT	  1
#define	DEF_USESLOVOED	  0
#define	DEF_FONTCACHE	  6
#define	DEF_FBUF	  16384
#define	DEF_ROTB	  1
#define DEF_BOOKMARKS	  64
#define DEF_SAVETOFILES	  0

#define	DEF_AUTOREPEATLIMIT 0
#define	REPEAT_THRESHOLD  200

#define	DEF_SAVEINTERVAL  (300*1000)

#define	DEF_GAMMA	  2200000

#define	DEF_AS_DELAY	  800000 // 0.8s

#define	DEF_PROGRESSBAR	  5

#define	RECENT_BASE 500
#define	RECENT_FILES  8
#define	DICT_BASE   (RECENT_BASE+RECENT_FILES+1)
#define	DICT_FILES  100
#define	COLORS_BASE (DICT_BASE+DICT_FILES+1)
#define COLORS_MAX  100

UINT	getIDR_DIALOG();
UINT	getIDR_CONTENTS();
UINT	getIDR_MAINFRAME();
#define	cIDR_DIALOG	getIDR_DIALOG()
#define	cIDR_CONTENTS	getIDR_CONTENTS()
#define	cIDR_MAINFRAME	getIDR_MAINFRAME()

#endif
