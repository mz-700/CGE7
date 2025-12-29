#define OEMRESOURCE
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "CGE7.h"
#include "cpal.h"

int YOKO;
int TATE;
int CPALX;

HINSTANCE hInst;

HBRUSH cursorbrush;

/* temporary definition */
#define C(x)	((x) & 0x1f)
#define M(x)	((x) | 0x80)
#define ATTR(f,b,a)	(((a)<<7)|((f)<<4)|(b))

#define KEY_PRESSED 0x80

#define bkbufPtr(x,y) ((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y))*(bkbuf.w)*4 + (x)*4])

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void createBackBuffer(HWND, int, int, myBackBuf *);
void destroyBackBuffer(myBackBuf *);

int isDisposalOK(void);

extern void save_CGEdit(HANDLE);
extern int load_CGEdit(char *);
extern void save_palette(HANDLE);
extern void send_to_clipboard(HWND, int, int, int, int);
extern void floater_to_clipboard(HWND, int, int);
extern int check_clipboard(HWND);
extern int receive_from_clipboard(HWND, int *, int *);
extern void send_bitmap_to_clipboard(HWND, HDC, int, int);

extern int undoInit(void);
extern void undoTini(void);
extern void undoStartEntry(void);
extern void undoEntry(int, int, int, int, int, int);
extern int undoCanUndo(void);
extern int undoCanRedo(void);
extern int undoUndo(int *, int *, int *, int *);
extern int undoRedo(int *, int *, int *, int *);
extern void undoFlush(void);
extern void undoCancelRedo(void);
extern void undoTest(HWND);
extern int undoIsModified(void);
extern void undoResetModifiedFlag(void);

extern int openTextInputDialog(HINSTANCE, HWND);

extern void openAnimDialog(HWND, int);
extern int isAnimDlgOpen(void);

extern void load_config();
extern void save_config();

int add_recent_file(char *);
int del_recent_file(char *);
char *get_recent_file(int);

extern LPBITMAPINFO loadBMPfile(char *);
LPBITMAPINFO ovBMP;

myBackBuf bkbuf;
int forecolor = 7;
int backcolor = 1;
int chrpos = 0x70;
int cspalno = 0;
int eufont_ok = 0;
int lastchrpos = -1;

int lastflag = 0;

unsigned char cgrom[8192];
int chr[1000];
int atr[1000];

int showgrid = 1;
int expansion = 1;
int zoomratio = 1;
int showanimframe = 1;
int showoverlay = 0;
int showdispc00 = 0;
int showbit3 = 0;
int mark_is_tp = 0;
int showstatusbar = 0;
int gridtype = 0;

int semigrapen = 0;
int bit3pen = 0;

int selection = 0;
int selx1, sely1;
int selx2, sely2;
int selanim;

int floater;
int floaterchr[1000];
int floateratr[1000];

int lastvx = -1;
int lastvy = -1;
int dragx;
int dragy;
int dragmode;

int currtool = IDTBB_PENCIL;

extern DWORD colortable[];
extern int cspal[CPAL_NUM * 4];

extern int hrev[256];
extern int vrev[256];

AnimBox animsel[9];

DWORD gridcolor;
DWORD bit3color;

HFONT fmini;

myBackBuf ovbkbuf;

static char *wintitle = "MZ-700 Character Graphics Editor";

/*----------------------------------------------------------------------
	tool bar
 ----------------------------------------------------------------------*/
HWND hToolbar;
int tbHeight;

TBBUTTON tbb[] = {
	{0,  IDTBB_CURSOR,   TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{1,  IDTBB_PENCIL,   TBSTATE_ENABLED|TBSTATE_CHECKED, TBSTYLE_CHECKGROUP, 0, 0 },
	{2,  IDTBB_PAINT,    TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{21, IDTBB_BIT3,     TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{0,  0,		     TBSTATE_ENABLED,		      TBSTYLE_SEP,        0, 0 },
	{3,  IDTBB_SAVE,     TBSTATE_ENABLED,		      TBSTYLE_BUTTON,     0, 0 },
	{0,  0,		     TBSTATE_ENABLED,		      TBSTYLE_SEP,        0, 0 },
	{4,  IDTBB_F_BLACK,  TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{5,  IDTBB_F_BLUE,   TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{6,  IDTBB_F_RED,    TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{7,  IDTBB_F_MAGENTA,TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{8,  IDTBB_F_GREEN,  TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{9,  IDTBB_F_CYAN,   TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{10, IDTBB_F_YELLOW, TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{11, IDTBB_F_WHITE,  TBSTATE_ENABLED|TBSTATE_CHECKED, TBSTYLE_CHECKGROUP, 0, 0 },
	{12, IDTBB_SWAP,     TBSTATE_ENABLED,		      TBSTYLE_BUTTON,     0, 0 },
	{13, IDTBB_B_BLACK,  TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{14, IDTBB_B_BLUE,   TBSTATE_ENABLED|TBSTATE_CHECKED, TBSTYLE_CHECKGROUP, 0, 0 },
	{15, IDTBB_B_RED,    TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{16, IDTBB_B_MAGENTA,TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{17, IDTBB_B_GREEN,  TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{18, IDTBB_B_CYAN,   TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{19, IDTBB_B_YELLOW, TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 },
	{20, IDTBB_B_WHITE,  TBSTATE_ENABLED,		      TBSTYLE_CHECKGROUP, 0, 0 }
};
TBBUTTON tb = {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0};

void make_toolbar(HWND hwnd) {
	hToolbar = CreateToolbarEx(hwnd, WS_CHILD|WS_VISIBLE, ID_TOOLBAR,
				   22, hInst, ID_TOOLBARBMP, tbb,
				   24, 0, 0, 16, 15, sizeof(TBBUTTON));
	forecolor = 7;	/* white */
	backcolor = 1;	/* blue */
}

/*----------------------------------------------------------------------
	status bar
 ----------------------------------------------------------------------*/
static HWND hStatusWnd = NULL;
int sbHeight;

HWND CreateMyStatusbar(HWND hWnd) {

	int sb_size[] = {80, 200, -1};

	hStatusWnd = CreateWindowEx(
	        0, /*拡張スタイル*/
	        STATUSCLASSNAME, /*ウィンドウクラス*/
	        NULL, /*タイトル*/
	        WS_CHILD, /*ウィンドウスタイル*/
	        0, 0, /*位置*/
	        0, 0, /*幅、高さ*/
	        hWnd, /*親ウィンドウ*/
	        (HMENU)ID_STATUSBAR, /*ウィンドウのＩＤ*/
	        hInst, /*インスタンスハンドル*/
	        NULL);
	SendMessage(hStatusWnd, SB_SETPARTS, (WPARAM)3, (LPARAM)(LPINT)sb_size);
	ShowWindow(hStatusWnd, SW_SHOW);
	return hStatusWnd;
}

void setStatusBarStr(int i, char *str) {
	SendMessage(hStatusWnd, SB_SETTEXT, (WPARAM)i | 0, (LPARAM)str);
}

/*---------------------------------------------------------------------------*/
int loadmzfont(char *fn, int normal) {
	HANDLE *fp;
	DWORD bytesread;

	fp = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fp == INVALID_HANDLE_VALUE) return 0;
	if (!ReadFile(fp, &cgrom[normal ? 0 : 4096], 4096, &bytesread, NULL)) return 0;
	CloseHandle(fp);
	return 1;
}

/*---------------------------------------------------------------------------*/
HFONT getfont(char *name, int size) {
	LOGFONT lf;

	ZeroMemory(&lf, sizeof(LOGFONT));
	lf.lfHeight		= size;
	lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	lf.lfQuality		= DEFAULT_QUALITY;
	lf.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
	lf.lfCharSet		= OEM_CHARSET;
	wsprintf(lf.lfFaceName, "%s", name);
	return CreateFontIndirect(&lf);
}

/*---------------------------------------------------------------------------*/
int getYOKO() {
	if (expansion == 0) {
	    return (8+320+10+(16*8+15*2)+8) + 20;
	}
	return (8+(320*(expansion+1))+10+((16*8+15)*(expansion+1))+8);
}
int getTATE() {
	if (expansion == 0) {
	    return (8+(22*8+21*2)+8);
	}
	return (8+(200*(expansion+1))+8);
}
int getCPALX() {
	return (8+(320*(expansion+1))+10);
}

/*---------------------------------------------------------------------------*/
int winxy2vramxy(int gx, int gy, int *vx, int *vy) {
	int w;
	w = 8 * (expansion + 1);
	if (gx >= MAINX && gx < (MAINX+40*w) &&
	    gy >= MAINY && gy < (MAINY+25*w)) {
		*vx = (gx - MAINX)/w;
		*vy = (gy - MAINY)/w;
		return 1;
	}
	return 0;
}
int winxy2vramxy2(int gx, int gy, int *vx, int *vy) {
	int w;
	w = 8 * (expansion + 1);
	if (gx < MAINX) gx = MAINX;
	if (gy < MAINY) gy = MAINY;
	gx -= MAINX;
	gy -= MAINY;
	gx /= w;
	gy /= w;
	if (gx >= 40) gx = 39;
	if (gy >= 25) gy = 24;
	*vx = gx;
	*vy = gy;
	return 1;
}
void vramxy2winxy(int vx, int vy, int *gx, int *gy) {
	*gx = vx * 8 * (expansion + 1) + MAINX;
	*gy = vy * 8 * (expansion + 1) + MAINY;
}
int winxy2sgxy(int gx, int gy, int *sx, int *sy) {
	int w;
	w = 8 * (expansion + 1);
	if (gx >= MAINX && gx < (MAINX+40*w) &&
	    gy >= MAINY && gy < (MAINY+25*w)) {
		*sx = (gx - MAINX)/(w>>1);
		*sy = (gy - MAINY)/(w>>1);
		return 1;
	}
	return 0;
}
int sgxy2mask(int sx, int sy) {
	return (1 << (sx & 1)) << ((sy & 1) << 1);
}

int winxy2palindex(int gx, int gy) {
	int x, y, w;
	x = CPALX;
	y = CPALY;
	if (expansion == 0) {
	    w = 10;
	} else {
	    w = 9*(expansion+1);
	}
	if (gx >= (x-1) && gx < (x-1+w*16) &&
	    gy >= (y-1) && gy < (y-1+w*22)) {
		return (((gy - (y-1))/w) << 4) | ((gx - (x-1))/w);
	}
	return -1;
}
void palxy2winxy(int px, int py, int *gx, int *gy) {
	int w;
	if (expansion == 0) {
	    w = 10;
	} else {
	    w = 9*(expansion+1);
	}
	*gx = CPALX + px*w;
	*gy = CPALY + py*w;
}

const LPCTSTR onsel2cursor[10] = {
	IDC_ARROW,	IDC_SIZENESW,	IDC_SIZENS,	IDC_SIZENWSE,
	IDC_SIZEWE,	IDC_SIZEALL,	IDC_SIZEWE,	IDC_SIZENWSE,
	IDC_SIZENS,	IDC_SIZENESW
};
int currcursor = 0;
void mySetCursor(int drgmod) {
	SetCursor(LoadCursor(NULL, onsel2cursor[drgmod]));
}
int is_onselection(int x, int y) {
	int wx1, wy1, wx2, wy2, f;
	const int ttbl[16] = { 0, 4, 6, 0, 8, 7, 9, 0, 2, 1, 3, 0, 0, 0, 0, 0 };
	if (!selection) return 0;
	vramxy2winxy(selx1, sely1, &wx1, &wy1);
	vramxy2winxy(selx2+1, sely2+1, &wx2, &wy2);
	wx2--;
	wy2--;
	/* outside */
	if (x < wx1-4 || x >= wx2+4) return 0;
	if (y < wy1-4 || y >= wy2+4) return 0;
	/* inside */
	if (x >= wx1+4 && x <= wx2-4 &&
	    y >= wy1+4 && y <= wy2-4) return 5;
	if (floater) return 5;
	/* edge */
	f = 0;
	if (x < wx1+4) f |= 0x01;
	if (x > wx2-4) f |= 0x02;
	if (y < wy1+4) f |= 0x04;
	if (y > wy2-4) f |= 0x08;
	return ttbl[f];
}

/*---------------------------------------------------------------------------*/
void drawchr_m(int x, int y, int chr, int attr) {
	int i,j,k,l,w;
	int yy;
	unsigned char c, *cgr, m;
	DWORD fc, bc, *p1;
	w = 8 * (expansion+1);
	cgr = &cgrom[/*((attr & 0x08)<<9)+*/((attr & 0x0080)<<4)+(chr<<3)];
	fc = colortable[(attr & 0x70) >> 4];
	bc = colortable[(attr & 0x07)     ];
	yy = y;
	for (i=0; i<8; i++) {
	    m = *cgr++;
	    for (k=0; k<expansion+1; k++) {
		p1 = (DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-yy)*(bkbuf.w)*4 + x*4];
		c = m;
		for (j=0; j<8; j++) {
		    if (c & 0x80) {
			for (l=0; l<expansion+1; l++) *p1++ = fc;
		    } else {
			for (l=0; l<expansion+1; l++) *p1++ = bc;
		    }
		    c <<= 1;
		}
		yy++;
	    }
	}
	if (showgrid && x < CPALX) {
	    *bkbufPtr(x, y) = gridcolor;
	    if (gridtype != 0) {
		for (i=0; i<=expansion; i++) {
		    *bkbufPtr(x+1+i,   y) = gridcolor;  // top left
		    *bkbufPtr(x+w-1-i, y) = gridcolor;  // top right
		    *bkbufPtr(x, y+1+i  ) = gridcolor;  // left top
		    *bkbufPtr(x, y+w-1-i) = gridcolor;  // left bottom
		}
	    }
	}
	if (showdispc00 && !chr) {
	    for (i=0; i<w; i++) {
		*bkbufPtr(x,     y+i) = gridcolor;  // left
		*bkbufPtr(x+w-1, y+i) = gridcolor;  // right
		*bkbufPtr(x+i,   y  ) = gridcolor;  // top
		*bkbufPtr(x+i, y+w-1) = gridcolor;  // bottom
		*bkbufPtr(x+i, y+i  ) = gridcolor;  // backslash
		*bkbufPtr(x+w-1-i, y+i) = gridcolor;  // slash
	    }
	}
	if (showbit3 && (attr & 0x08)) {
	    for (i=0; i<w; i++) {
		*bkbufPtr(x,     y+i) = bit3color;  // left
		*bkbufPtr(x+w-1, y+i) = bit3color;  // right
		*bkbufPtr(x+i,   y  ) = bit3color;  // top
		*bkbufPtr(x+i, y+w-1) = bit3color;  // bottom
		*bkbufPtr(x+i, y+i  ) = bit3color;  // backslash
		*bkbufPtr(x+w-1-i, y+i) = bit3color;  // slash
	    }
	}
}
void drawchr_e(int x, int y, int chr, int attr) {
	int i,j;
	unsigned char c, *cgr;
	DWORD fc, bc, *p1, *p2;
	cgr = &cgrom[/*((attr & 0x08)<<9)+*/((attr & 0x0080)<<4)+(chr<<3)];
	fc = colortable[(attr & 0x70) >> 4];
	bc = colortable[(attr & 0x07)     ];
	for (i=0; i<8; i++) {
	    c = *cgr++;
	    p1 = (DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-y)*(bkbuf.w)*4 + x*4];
	    p2 = (DWORD *)&bkbuf.lpBMP[(bkbuf.h-2-y)*(bkbuf.w)*4 + x*4];
	    for (j=0; j<8; j++) {
		if (c & 0x80) {
		    *p1++ = fc;
		    *p1++ = fc;
		    *p2++ = fc;
		    *p2++ = fc;
		} else {
		    *p1++ = bc;
		    *p1++ = bc;
		    *p2++ = bc;
		    *p2++ = bc;
		}
		c <<= 1;
	    }
	    y += 2;
	}
	if (showgrid && x < CPALX)
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-16))*(bkbuf.w)*4 + x*4]) = gridcolor;
	if (showdispc00 && !chr) for (i=16; i; i--) {
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-   i))*(bkbuf.w)*4 + (x    )*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-   i))*(bkbuf.w)*4 + (x+ 15)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-   1))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  16))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-   i))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-17+i))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	}
}
void drawchr_n(int x, int y, int chr, int attr) {
	int i,j;
	unsigned char c, *cgr;
	DWORD fc, bc, *p;
	cgr = &cgrom[/*((attr & 0x08)<<9)+*/((attr & 0x0080)<<4)+(chr<<3)];
	fc = colortable[(attr & 0x70) >> 4];
	bc = colortable[(attr & 0x07)     ];
	for (i=0; i<8; i++) {
	    c = *cgr++;
	    p = (DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-y)*(bkbuf.w)*4 + x*4];
	    for (j=0; j<8; j++) {
		*p++ = (c & 0x80) ? fc : bc;
		c <<= 1;
	    }
	    y++;
	}
	if (showgrid && x < CPALX)
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-8))*(bkbuf.w)*4 + x*4]) = gridcolor;
	if (showdispc00 && !chr) for (i=8; i; i--) {
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  i))*(bkbuf.w)*4 + (x    )*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  i))*(bkbuf.w)*4 + (x+7  )*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  1))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  8))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  i))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-9+i))*(bkbuf.w)*4 + (x+i-1)*4]) = gridcolor;
	}
	if (showbit3 && (attr & 0x08)) for (i=8; i; i--) {
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  i))*(bkbuf.w)*4 + (x    )*4]) = bit3color;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  i))*(bkbuf.w)*4 + (x+7  )*4]) = bit3color;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  1))*(bkbuf.w)*4 + (x+i-1)*4]) = bit3color;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  8))*(bkbuf.w)*4 + (x+i-1)*4]) = bit3color;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-  i))*(bkbuf.w)*4 + (x+i-1)*4]) = bit3color;
	    *((DWORD *)&bkbuf.lpBMP[(bkbuf.h-1-(y-9+i))*(bkbuf.w)*4 + (x+i-1)*4]) = bit3color;
	}
}
void drawchr(int x, int y, int chr, int attr) {
	if (expansion)
	    drawchr_m(x, y, chr, attr);
	else
	    drawchr_n(x, y, chr, attr);
}

void putchr(int x, int y, int chr, int attr) {
	int gx, gy;
	vramxy2winxy(x, y, &gx, &gy);
	drawchr(gx, gy, chr, attr);
}

void pset(int x, int y, int c, int a) {
	int i;
	int oc, oa;
	i = y*40+x;
	oc = chr[i];
	oa = atr[i];
	chr[i] = c;
	atr[i] = a;
	undoEntry(x, y, oc, oa, c, a);
	putchr(x, y, c, a);
}

void drawgrid(void) {
	int i, j, k, x, y, w;
	DWORD c;
	w = 8 * (expansion+1);
	c = showgrid ? gridcolor : 0;
	if (showgrid) {
	    for (i=0; i<=24; i++) {
		for (j=0; j<=39; j++) {
		    vramxy2winxy(j, i, &x, &y);
		    *bkbufPtr(x, y) = gridcolor;
		    if (gridtype != 0) {
			for (k=0; k<=expansion; k++) {
			    *bkbufPtr(x+1+k,   y) = gridcolor;  // top left
			    *bkbufPtr(x+w-1-k, y) = gridcolor;  // top right
			    *bkbufPtr(x, y+1+k  ) = gridcolor;  // left top
			    *bkbufPtr(x, y+w-1-k) = gridcolor;  // left bottom
			}
		    }
		}
	    }
	}
	for (i=0; i<25; i++) {
	    vramxy2winxy(40, i, &x, &y);
	    *bkbufPtr(x, y) = c;
	    if (expansion && gridtype != 0) {
		for (k=0; k<=expansion; k++) {
		    *bkbufPtr(x, y+1+k  ) = c;  // left top
		    *bkbufPtr(x, y+w-1-k) = c;  // left bottom
		}
	    }
	}
	for (j=0; j<40; j++) {
	    vramxy2winxy(j, 25, &x, &y);
	    *bkbufPtr(x, y) = c;
	    if (expansion && gridtype != 0) {
		for (k=0; k<=expansion; k++) {
		    *bkbufPtr(x+1+k,   y) = c;  // top left
		    *bkbufPtr(x+w-1-k, y) = c;  // top right
		}
	    }
	}
	vramxy2winxy(40, 25, &x, &y);
	*bkbufPtr(x, y) = c;
}

void drawchrpalette(void) {
	int i,j,k,c,a;
	int *p;
//	p = (cspalno) ? cspal2 : cspal;
	p = &cspal[CPAL_NUM * cspalno];
	for (i=0; i<22; i++) {
	    for (j=0; j<16; j++) {
		k = *p++;
		c = k & 0x00ff;
		if (k & CPAL_PRCOLOR) {
		    a = (k & (CPAL_FCMASK|CPAL_BCMASK)) >> 16;
		} else {
		    if (k & CPAL_REVERSE)
			a = (backcolor << 4) | forecolor;
		    else
			a = (forecolor << 4) | backcolor;
		}
		a |= (k & 0x0100) >> 1;
		a |= (cspalno >= 2) ? 0x08 : 0;
		if (expansion)
		    drawchr_m(CPALX+j*9*(expansion+1), CPALY+i*9*(expansion+1), c, a);
		else
		    drawchr_n(CPALX+j*10, CPALY+i*10, c, a);
	    }
	}
}

void get_chr_attr(int *c, int *a) {
	int k;
//	k = (cspalno) ? cspal2[chrpos] : cspal[chrpos];
	k = cspal[CPAL_NUM * cspalno + chrpos];
	*c = k & 0x00ff;
	if (k & CPAL_PRCOLOR)
	    *a = ((k & (CPAL_FCMASK|CPAL_BCMASK)) >> 16) | ((k & 0x0100) >> 1);
	else {
	    if (k & CPAL_REVERSE)
		*a = ((k & 0x0100) >> 1) | (backcolor << 4) | forecolor | ((cspalno >= 2) ? 0x08 : 0);
	    else
		*a = ((k & 0x0100) >> 1) | (forecolor << 4) | backcolor | ((cspalno >= 2) ? 0x08 : 0);
	}
}

void drawframe(void) {
	HDC hdc;
	RECT r;
	hdc = bkbuf.hdcMem;
	/* background */
	r.left   = r.top = 0;
	r.right  = YOKO;
	r.bottom = TATE;
	FillRect(hdc, &r, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
	gridcolor = *((DWORD *)(&bkbuf.lpBMP[(bkbuf.h-1)*(bkbuf.w)*4]));
	/* main scratch area */
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	RoundRect(hdc, MAINX-3, MAINY-3, MAINX+(320*(expansion+1))+4, MAINY+(200*(expansion+1))+4, 8, 8);
	drawgrid();
	/* character selection area */
	drawchrpalette();
}

void updatechrpalette(HWND hwnd) {
	RECT r;
	if (expansion) {
	    r.left   = CPALX - 2;
	    r.top    = CPALY + tbHeight - 2;
	    r.right  = CPALX + 9*(expansion+1)*16;
	    r.bottom = CPALY + tbHeight + 9*(expansion+1)*22;
	} else {
	    r.left   = CPALX - 2;
	    r.top    = CPALY + tbHeight - 2;
	    r.right  = CPALX + 10*16;
	    r.bottom = CPALY + tbHeight + 10*22;
	}
	InvalidateRect(hwnd, &r, FALSE);
}
void updatescratcharea(HWND hwnd) {
	RECT r;
	int w;
	w = 8 * (expansion+1);
	r.left   = MAINX - 1;
	r.top    = MAINY - 1 + tbHeight;
	r.right  = MAINX + 40*w + 1;
	r.bottom = MAINY + tbHeight + 25*w + 1;
	InvalidateRect(hwnd, &r, FALSE);
}

void vram2disp(HWND hwnd) {
	int i, j, k;
	k = 0;
	for (i=0; i<25; i++) {
	    for (j=0; j<40; j++) {
		putchr(j, i, chr[k], atr[k]);
		k++;
	    }
	}
	updatescratcharea(hwnd);
}

/* chr palette cursor */
void drawpalcursor(HDC hdc) {
	RECT r;
	if (expansion) {
	    r.left   = CPALX + (chrpos & 0x0f)*9*(expansion+1) - 1;
	    r.top    = CPALY + (chrpos >> 4)*9*(expansion+1) + tbHeight - 1;
	    r.right  = r.left + 8*(expansion+1) + 2;
	    r.bottom = r.top  + 8*(expansion+1) + 2;
	} else {
	    r.left   = CPALX + (chrpos & 0x0f)*10 - 1;
	    r.top    = CPALY + (chrpos >> 4)*10 + tbHeight - 1;
	    r.right  = r.left + 8 + 2;
	    r.bottom = r.top + 8 + 2;
	}
	FrameRect(hdc, &r, cursorbrush);
	r.left--; r.top--; r.right++; r.bottom++;
	FrameRect(hdc, &r, cursorbrush);
}

/*----------------------------------------------------------------------
	selection
 ----------------------------------------------------------------------*/
void draw_selection(HDC hdc) {
	HBITMAP hbm;
	HBRUSH	hbrush, hbrushOld;
	BYTE bBits[16];
	int i, m;
	int x1, y1, x2, y2;
	int sx1, sy1, sx2, sy2;

	if (!selection) return;

	/* clipping */
	sx1 = (selx1 < 0) ? 0 : selx1;
	sy1 = (sely1 < 0) ? 0 : sely1;
	sx2 = (selx2+1 >= 40) ? 40 : selx2+1;
	sy2 = (sely2+1 >= 25) ? 25 : sely2+1;

	vramxy2winxy(sx1, sy1, &x1, &y1);
	y1 += tbHeight;
	vramxy2winxy(sx2, sy2, &x2, &y2);
	y2 += tbHeight;

	SetBkColor(hdc, RGB(0,0,0));
	SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));

	/* left */
	if (selx1 >= 0) {
	    m = 0x0f << selanim;
	    m |= (m >> 8);
	    for (i=0; i<8; i++) {
		bBits[i*2] = (m & 1) ? 0xff : 0x00;
		m >>= 1;
	    }
	    hbm = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBits); 
	    hbrush = CreatePatternBrush(hbm); 
	    hbrushOld = SelectObject(hdc, hbrush); 
	    PatBlt(hdc, x1-1, y1-1, 1, y2-y1+1, PATCOPY);
	    SelectObject(hdc, hbrushOld); 
	    DeleteObject(hbrush); 
	    DeleteObject(hbm); 
	}

	/* right */
	if (selx2 < 40) {
	    m = 0x0f << selanim;
	    m |= (m >> 8);
	    for (i=0; i<8; i++) {
		bBits[(7-i)*2] = (m & 1) ? 0xff : 0x00;
		m >>= 1;
	    }
	    hbm = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBits); 
	    hbrush = CreatePatternBrush(hbm); 
	    hbrushOld = SelectObject(hdc, hbrush); 
	    PatBlt(hdc, x2, y1, 1, y2-y1+1, PATCOPY);
	    SelectObject(hdc, hbrushOld); 
	    DeleteObject(hbrush); 
	    DeleteObject(hbm); 
	}

	/* top */
	if (sely1 >= 0) {
	    m = 0xf0f0 >> (8-selanim);
	    for (i=0; i<16; i++) bBits[i] = m;
	    hbm = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBits); 
	    hbrush = CreatePatternBrush(hbm); 
	    hbrushOld = SelectObject(hdc, hbrush); 
	    PatBlt(hdc, x1, y1-1, x2-x1+1, 1, PATCOPY);
	    SelectObject(hdc, hbrushOld); 
	    DeleteObject(hbrush); 
	    DeleteObject(hbm); 
	}

	/* bottom */
	if (sely2 < 25) {
	    m = 0x0f0f >> selanim;
	    for (i=0; i<16; i++) bBits[i] = m;
	    hbm = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBits); 
	    hbrush = CreatePatternBrush(hbm); 
	    hbrushOld = SelectObject(hdc, hbrush); 
	    PatBlt(hdc, x1-1, y2, x2-x1+1, 1, PATCOPY);
	    SelectObject(hdc, hbrushOld); 
	    DeleteObject(hbrush); 
	    DeleteObject(hbm); 
	}

	//SelectObject(hdc, hbrushOld); 
}

void CALLBACK anim_selection(HWND hwnd, UINT msg, UINT idtimer, DWORD dwtime) {
	HDC hdc;
	hdc = GetDC(hwnd);
	selanim = (selanim + 1) & 7;
	draw_selection(hdc);
	ReleaseDC(hwnd, hdc);
}
void hide_selection(void) {
	if (!selection) return;
	KillTimer(bkbuf.hWnd, 1);
	updatescratcharea(bkbuf.hWnd);
	selection = 0;
}
void show_selection(int x1, int y1, int x2, int y2) {
	HDC hdc;
	hide_selection();
	if (x1 < x2) { selx1 = x1; selx2 = x2; }
	else	     { selx1 = x2; selx2 = x1; }
	if (y1 < y2) { sely1 = y1; sely2 = y2; }
	else	     { sely1 = y2; sely2 = y1; }
	selection = 1;
	selanim = 0;
	hdc = GetDC(bkbuf.hWnd);
	draw_selection(hdc);
	ReleaseDC(bkbuf.hWnd, hdc);
	SetTimer(bkbuf.hWnd, 1, 100, (TIMERPROC)anim_selection);
}
void fill_selection(int c, int a) {
	int x1, y1, x2, y2;
	int i, j;
	x1 = (selx1 < 0) ? 0 : selx1;
	y1 = (sely1 < 0) ? 0 : sely1;
	x2 = (selx2 > 39) ? 39 : selx2;
	y2 = (sely2 > 24) ? 24 : sely2;
	for (i = y1; i <= y2; i++) {
	    for (j = x1; j <= x2; j++) {
		pset(j, i, c, a);
	    }
	}
}

void drawanimrect(HDC hdc, int x1, int y1, int x2, int y2, int num) {
	int gx1, gy1, gx2, gy2;
	HBRUSH btmp;
	char buf[8];
	RECT r;

	btmp = CreateSolidBrush(RGB(0xff, 0x80, 0x00));
	vramxy2winxy(x1, y1, &gx1, &gy1);
	gy1 += tbHeight;
	vramxy2winxy(x2+1, y2+1, &gx2, &gy2);
	gy2 += tbHeight;

	SetRect(&r, gx1-1, gy1-1, gx2+1, gy2+1);
	FrameRect(hdc, &r, btmp);
	wsprintf(buf, "%d ", num);
	SelectObject(hdc, fmini);
	SetTextColor(hdc, RGB(0,0,0));
	SetBkColor(hdc, RGB(0xff, 0x80, 0x00));
	TextOut(hdc, gx1, gy1, buf, (int)strlen(buf));
}

/*----------------------------------------------------------------------
	seed fill
 ----------------------------------------------------------------------*/
int issfillarea(int x, int y, int c, int a) {
	int i;
	i = y*40+x;
	return (chr[i] == c && atr[i] == a);
}
void seedfill(int x, int y, int ch, int attr) {
	int xl[1024];
	int xr[1024];
	int yy[1024];
	int sp = 0;
	int i;
	int cxl, cxr, cy;
	int dxl, dxr, dy;
	int fillc = chr[y*40+x];
	int filla = atr[y*40+x];
	if (fillc == ch && filla == attr) return;
	cxl = cxr = x;
	cy = y;
	while (cxl > 0  && issfillarea(cxl-1, cy, fillc, filla)) cxl--;
	while (cxr < 39 && issfillarea(cxr+1, cy, fillc, filla)) cxr++;
	xl[sp] = cxl;
	xr[sp] = cxr;
	yy[sp] = cy;
	sp++;
	undoStartEntry();
	while (sp) {
	    sp--;
	    cxl = xl[sp];
	    cxr = xr[sp];
	    cy = yy[sp];
	    for (i=cxl; i<=cxr; i++) pset(i, cy, ch, attr);
	    if (cy > 0) {
		dy = cy-1;
		for (i=cxl; i<=cxr; i++) {
		    if (issfillarea(i, dy, fillc, filla)) {
			dxl = dxr = i;
			while (dxl > 0  && issfillarea(dxl-1, dy, fillc, filla)) dxl--;
			while (dxr < 39 && issfillarea(dxr+1, dy, fillc, filla)) dxr++;
			xl[sp] = dxl;
			xr[sp] = dxr;
			yy[sp] = dy;
			sp++;
			if (sp >= 1024) return; /* buffer overflow */
			if (dxr > i && i <= cxr) i = dxr+1;
		    }
		}
	    }
	    if (cy < 24) {
		dy = cy+1;
		for (i=cxl; i<=cxr; i++) {
		    if (issfillarea(i, dy, fillc, filla)) {
			dxl = dxr = i;
			while (dxl > 0  && issfillarea(dxl-1, dy, fillc, filla)) dxl--;
			while (dxr < 39 && issfillarea(dxr+1, dy, fillc, filla)) dxr++;
			xl[sp] = dxl;
			xr[sp] = dxr;
			yy[sp] = dy;
			sp++;
			if (sp >= 1024) return; /* buffer overflow */
			if (dxr > i && i <= cxr) i = dxr+1;
		    }
		}
	    }
	}
}

int issfillarea_sg(int sx, int sy, int *tmpchr) {
	int i, m;
	i = (sy>>1)*40+(sx>>1);
	m = sgxy2mask(sx, sy);
	return tmpchr[i] == 0x00 ||
		((tmpchr[i] & 0xF0) == 0xF0 && !(tmpchr[i] & m));
}
void seedfill_sg(int sx, int sy, int attr) {
	char xl[1024];
	char xr[1024];
	char yy[1024];
	int tmpchr[1000];
	int sp = 0;
	int i;
	int cxl, cxr, cy;
	int dxl, dxr, dy;
	cxl = cxr = sx;
	cy = sy;
	for (i=0; i<1000; i++) tmpchr[i] = chr[i];
	if (!issfillarea_sg(sx, sy, &tmpchr[0])) return;
	while (cxl > 0  && issfillarea_sg(cxl-1, cy, &tmpchr[0])) cxl--;
	while (cxr < 79 && issfillarea_sg(cxr+1, cy, &tmpchr[0])) cxr++;
	xl[sp] = cxl;
	xr[sp] = cxr;
	yy[sp] = cy;
	sp++;
	while (sp) {
	    sp--;
	    cxl = xl[sp];
	    cxr = xr[sp];
	    cy = yy[sp];
	    for (i=cxl; i<=cxr; i++) tmpchr[(cy>>1)*40+(i>>1)] |= sgxy2mask(i, cy) | 0xF0;
	    if (cy > 0) {
		dy = cy-1;
		for (i=cxl; i<=cxr; i++) {
		    if (issfillarea_sg(i, dy, &tmpchr[0])) {
			dxl = dxr = i;
			while (dxl > 0  && issfillarea_sg(dxl-1, dy, &tmpchr[0])) dxl--;
			while (dxr < 79 && issfillarea_sg(dxr+1, dy, &tmpchr[0])) dxr++;
			xl[sp] = dxl;
			xr[sp] = dxr;
			yy[sp] = dy;
			sp++;
			if (sp >= 1024) return; /* buffer overflow */
			if (dxr > i && i <= cxr) i = dxr+1;
		    }
		}
	    }
	    if (cy < 49) {
		dy = cy+1;
		for (i=cxl; i<=cxr; i++) {
		    if (issfillarea_sg(i, dy, &tmpchr[0])) {
			dxl = dxr = i;
			while (dxl > 0  && issfillarea_sg(dxl-1, dy, &tmpchr[0])) dxl--;
			while (dxr < 79 && issfillarea_sg(dxr+1, dy, &tmpchr[0])) dxr++;
			xl[sp] = dxl;
			xr[sp] = dxr;
			yy[sp] = dy;
			sp++;
			if (sp >= 1024) return; /* buffer overflow */
			if (dxr > i && i <= cxr) i = dxr+1;
		    }
		}
	    }
	}
	undoStartEntry();
	for (i=0, cy=0; cy<25; cy++) {
	    for (cxl=0; cxl<40; cxl++) {
		if (tmpchr[i] != chr[i]) pset(cxl, cy, tmpchr[i], attr);
		i++;
	    }
	}
}

/*----------------------------------------------------------------------
	floater
 ----------------------------------------------------------------------*/
void selection_to_floater(void) {
	int x1, y1, x2, y2;
	int i, j, k, l;
	if (!selection) return;
	x1 = (selx1 < 0) ? 0 : selx1;
	y1 = (sely1 < 0) ? 0 : sely1;
	x2 = (selx2 > 39) ? 39 : selx2;
	y2 = (sely2 > 24) ? 24 : sely2;
	for (i = 0; i < (y2-y1+1); i++) {
	    k = (y1+i)*40 + x1;
	    l = i*40;
	    for (j = 0; j < (x2-x1+1); j++) {
		drawchr(FLOATERX+j*8*(expansion+1),
			FLOATERY+i*8*(expansion+1), chr[k], atr[k]);
		floaterchr[l] = chr[k];
		floateratr[l] = atr[k];
		k++; l++;
	    }
	}
	floater = 1;
}
void draw_floater(HDC hdc) {
	int x1, y1, x2, y2;
	int gx, gy;
	int w;
	if (!floater) return;
	w = 8*(expansion+1);
	x1 = (selx1 < 0) ? -selx1 : 0;
	y1 = (sely1 < 0) ? -sely1 : 0;
	x2 = ((selx2 > 39) ? 39 : selx2) - selx1;
	y2 = ((sely2 > 24) ? 24 : sely2) - sely1;
	if (x2 > 39) x2 = 39;
	if (y2 > 24) y2 = 24;
	vramxy2winxy(((selx1 < 0) ? 0 : selx1), ((sely1 < 0) ? 0 : sely1), &gx, &gy);
	BitBlt(hdc, gx, gy+tbHeight, (x2-x1+1)*w, (y2-y1+1)*w,
		bkbuf.hdcMem, FLOATERX+x1*w, FLOATERY+y1*w, SRCCOPY);
}
void land_floater(void) {
	int x1, y1, x2, y2;
	int i, j, l;
	if (!selection || !floater) return;
	x1 = (selx1 < 0) ? -selx1 : 0;
	y1 = (sely1 < 0) ? -sely1 : 0;
	x2 = ((selx2 > 39) ? 39 : selx2) - selx1;
	y2 = ((sely2 > 24) ? 24 : sely2) - sely1;
	if (x2 > 39) x2 = 39;
	if (y2 > 24) y2 = 24;
	if (floater == 1) undoStartEntry();
	for (i = y1; i <= y2; i++) {
	    l = i*40 + x1;
	    for (j = x1; j <= x2; j++) {
		if (!mark_is_tp ||
		    !(showbit3 && (floateratr[l] & 0x08) ||
		      showdispc00 && (floaterchr[l] == 0))) {
		    pset(selx1+j, sely1+i, floaterchr[l], floateratr[l]);
		}
		l++;
	    }
	}
	floater = 0;
}
void refresh_floater(void) {
	int i, j, l;
	if (!selection || !floater) return;
	for (i = 0; i < (sely2-sely1+1); i++) {
	    l = i*40;
	    for (j = 0; j < (selx2-selx1+1); j++) {
		drawchr(FLOATERX+j*8*(expansion+1),
			FLOATERY+i*8*(expansion+1),
			floaterchr[l], floateratr[l]);
		l++;
	    }
	}
}
void blend_floater(void) {
	int x1, y1, x2, y2;
	int i, j, k, l;
	if (!selection || !floater) return;
	x1 = (selx1 < 0) ? -selx1 : 0;
	y1 = (sely1 < 0) ? -sely1 : 0;
	x2 = ((selx2 > 39) ? 39 : selx2) - selx1;
	y2 = ((sely2 > 24) ? 24 : sely2) - sely1;
	if (x2 > 39) x2 = 39;
	if (y2 > 24) y2 = 24;
	for (i = y1; i <= y2; i++) {
	    k = selx1+x1 + (sely1+i)*40;
	    l = x1 + i*40;
	    for (j = x1; j <= x2; j++) {
		if (showbit3 && (floateratr[l] & 0x08) ||
		    showdispc00 && floaterchr[l] == 0) {
		    drawchr(FLOATERX+j*8*(expansion+1),
			    FLOATERY+i*8*(expansion+1),
			    chr[k], atr[k]);
		}
		k++;
		l++;
	    }
	}
}

void hrev_floater(void) {
	int i, j, k, l;
	int lc, la, rc, ra;
	if (!selection || !floater) return;
	for (i = 0; i < (sely2-sely1+1); i++) {
	    k = i*40 + selx2 - selx1;
	    l = i*40;
	    for (j = 0; j < (selx2-selx1+1)/2; j++) {
		lc = hrev[floaterchr[l]];
		rc = hrev[floaterchr[k]];
		la = floateratr[l];
		ra = floateratr[k];
		if (la & 0x80) {
		    switch (lc) {
			case 0x10:  lc = 0x11; break; /* p → q */
			case 0x11:  lc = 0x10; break; /* q → p */
			case 0x02:  lc = 0x04; break; /* b → d */
			case 0x04:  lc = 0x02; break; /* d → b */
			default:	       break;
		    }
		}
		if (ra & 0x80) {
		    switch (rc) {
			case 0x10:  rc = 0x11; break; /* p → q */
			case 0x11:  rc = 0x10; break; /* q → p */
			case 0x02:  rc = 0x04; break; /* b → d */
			case 0x04:  rc = 0x02; break; /* d → b */
			default:	       break;
		    }
		}
		floaterchr[l] = rc;
		floaterchr[k] = lc;
		floateratr[l] = ra;
		floateratr[k] = la;
		k--;
		l++;
	    }
	    if ((selx2-selx1+1) & 1)
		floaterchr[l] = hrev[floaterchr[l]];
	}
	refresh_floater();
}

void vrev_floater(void) {
	int i, j, k, l;
	int lc, la, rc, ra;
	if (!selection || !floater) return;
	for (j = 0; j < (selx2-selx1+1); j++) {
	    k = j;
	    l = j + (sely2 - sely1)*40;
	    for (i = 0; i < (sely2-sely1+1)/2; i++) {
		lc = vrev[floaterchr[l]];
		rc = vrev[floaterchr[k]];
		la = floateratr[l];
		ra = floateratr[k];
		if (lc == 0xBD) la |= 0x80;
		if (rc == 0xBD) ra |= 0x80;
		floaterchr[l] = rc;
		floaterchr[k] = lc;
		floateratr[l] = ra;
		floateratr[k] = la;
		k += 40;
		l -= 40;
	    }
	    if ((sely2-sely1+1) & 1)
		floaterchr[l] = vrev[floaterchr[l]];
	}
	refresh_floater();
}

void shift_sg_in_floater(int dir) {
	int i, j, k;
	int c1, c2;
	int doland;
	if (!selection && !floater) return;
	doland = 0;
	if (!floater) {
	    selection_to_floater();
	    doland = 1;
	}
	switch (dir) {
	    case 0:
		/* left */
		for (i = 0; i < (sely2-sely1+1); i++) {
		    k = i*40;
		    for (j = 0; j < (selx2-selx1+1); j++, k++) {
			c1 = floaterchr[k];
			if ((c1 & 0xF0) != 0xF0 && c1 != 0x00) {
			     continue;
			}
			c1 = ((c1 & 0x0A) >> 1) | 0xF0;
			c2 = (j == (selx2-selx1)) ? 0xF0 : floaterchr[k+1];
			if ((c2 & 0xF0) == 0xF0) {
			     c2 &= 0x05;
			     c1 |= c2 << 1;
			}
			floaterchr[k] = c1;
		    }
		}
		break;
	    case 1:
		/* right */
		for (i = 0; i < (sely2-sely1+1); i++) {
		    k = i*40+(selx2-selx1);
		    for (j = 0; j < (selx2-selx1+1); j++, k--) {
			c1 = floaterchr[k];
			if ((c1 & 0xF0) != 0xF0 && c1 != 0x00) {
			     continue;
			}
			c1 = ((c1 & 0x05) << 1) | 0xF0;
			c2 = (j == (selx2-selx1)) ? 0xF0 : floaterchr[k-1];
			if ((c2 & 0xF0) == 0xF0) {
			     c2 &= 0x0A;
			     c1 |= c2 >> 1;
			}
			floaterchr[k] = c1;
		    }
		}
		break;
	    case 2:
		/* up */
		for (i = 0; i < (selx2-selx1+1); i++) {
		    k = i;
		    for (j = 0; j < (sely2-sely1+1); j++, k+=40) {
			c1 = floaterchr[k];
			if ((c1 & 0xF0) != 0xF0 && c1 != 0x00) {
			     continue;
			}
			c1 = ((c1 & 0x0C) >> 2) | 0xF0;
			c2 = (j == (sely2-sely1)) ? 0xF0 : floaterchr[k+40];
			if ((c2 & 0xF0) == 0xF0) {
			     c2 &= 0x03;
			     c1 |= c2 << 2;
			}
			floaterchr[k] = c1;
		    }
		}
		break;
	    case 3:
		/* down */
		for (i = 0; i < (selx2-selx1+1); i++) {
		    k = (sely2-sely1)*40+i;
		    for (j = 0; j < (sely2-sely1+1); j++, k-=40) {
			c1 = floaterchr[k];
			if ((c1 & 0xF0) != 0xF0 && c1 != 0x00) {
			     continue;
			}
			c1 = ((c1 & 0x03) << 2) | 0xF0;
			c2 = (j == (sely2-sely1)) ? 0xF0 : floaterchr[k-40];
			if ((c2 & 0xF0) == 0xF0) {
			     c2 &= 0x0C;
			     c1 |= c2 >> 2;
			}
			floaterchr[k] = c1;
		    }
		}
		break;
	}
	refresh_floater();
	if (doland) land_floater();
}

void getPastePos(int w, int h, int *x, int *y) {
	int dx, dy;
	dx = lastvx - w/2; dy = lastvy - h/2;
	if (dx < 0) dx = 0;
	if (dy < 0) dy = 0;
	if (dx+w > 40) dx = 40-w;
	if (dy+h > 25) dy = 25-h;
	*x = dx;
	*y = dy;
}

/*----------------------------------------------------------------------
	undo/redo
 ----------------------------------------------------------------------*/
void undo(void) {
	int x, y, c, a, i, r;
	if (!undoCanUndo()) return;
	do {
	    r = undoUndo(&x, &y, &c, &a);
	    i = y*40+x;
	    chr[i] = c;
	    atr[i] = a;
	    putchr(x, y, c, a);
	} while (r);
}
void redo(void) {
	int x, y, c, a, i, r;
	if (!undoCanRedo()) return;
	do {
	    r = undoRedo(&x, &y, &c, &a);
	    i = y*40+x;
	    chr[i] = c;
	    atr[i] = a;
	    putchr(x, y, c, a);
	} while (r);
}

/*----------------------------------------------------------------------
	file
 ----------------------------------------------------------------------*/
char szFile[MAX_PATH];
char szFileName[MAX_PATH];
int fformat;

char *filtertxt = "CGEdit File(*.txt)\0*.txt\0All files(*.*)\0*.*\0\0";

void setwtitle(HWND hWnd, int editflag) {
	char titlebuf[80];
	char *p;
	int i;
	p = szFile;
	if (!*p) p = "not saved yet";
	for (i=0; i<73; i++) {
	    if (!*p) break;
	    titlebuf[i] = *p++;
	}
	titlebuf[i++] = ' ';
	titlebuf[i++] = (editflag) ? '*' : '-';
	titlebuf[i++] = ' ';
	p = wintitle;
	for (   ; i<76; i++) {
	    if (!*p) break;
	    titlebuf[i] = *p++;
	}
	if (*p) {
	    titlebuf[i++] = '.';
	    titlebuf[i++] = '.';
	    titlebuf[i++] = '.';
	}
	titlebuf[i] = 0;
	SetWindowText(hWnd, titlebuf);
}

int OpenMyFileSub(HWND hWnd, char *fn)
{
	DWORD dwSize = 0L;
	HANDLE hFile;
	DWORD dwAccBytes;
	char *filebuf;
	char buf[256 + MAX_PATH];

	hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
	    wsprintf(buf, "%s cannot be opened", fn);
	    MessageBox(hWnd, buf, "CGE7: Error", MB_OK);
	    del_recent_file(fn);
	    szFile[0] = 0;
	    szFileName[0] = 0;
	    lastflag = -1; /* ウィンドウタイトルを強制アップデート */
	    setwtitle(hWnd, 0);
	    return 0;
	}
	dwSize = GetFileSize(hFile, NULL);
	filebuf = malloc(dwSize+1);
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);

	ReadFile(hFile, filebuf, dwSize, &dwAccBytes, NULL);
	filebuf[dwAccBytes] = '\0';
	CloseHandle(hFile);

	lastflag = -1; /* ウィンドウタイトルを強制アップデート */
	switch (fformat) {
	    case 1:
		undoFlush();
		if (!load_CGEdit(filebuf)) {
		    MessageBox(hWnd, "Not in CGEDIT Format", "CGE7: Error", MB_OK);
		    free(filebuf);
		    del_recent_file(fn);
		    szFile[0] = 0;
		    szFileName[0] = 0;
		    lastflag = -1; /* ウィンドウタイトルを強制アップデート */
		    setwtitle(hWnd, 0);
		    return 0;
		}
		vram2disp(hWnd);
		break;
	    default:
		break;
	}
	free(filebuf);
//	setwtitle(hWnd, 0);	/* done by undoFlush() */

	add_recent_file(fn);
	return 1;
}
int OpenMyFile(HWND hWnd)
{
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filtertxt;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = sizeof(szFile);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";
	ofn.lpstrTitle = "Open File";

	if(GetOpenFileName(&ofn) == 0)
	    return 0;
	fformat = ofn.nFilterIndex;

	return OpenMyFileSub(hWnd, szFileName);
}

int WriteMyFile(HWND hWnd, int nodlg)
{
	OPENFILENAME ofn;
	HANDLE hFile;

	if (!nodlg || !szFileName[0] || !fformat) {
	    memset(&ofn, 0, sizeof(OPENFILENAME));
	    ofn.lStructSize = sizeof(OPENFILENAME);
	    ofn.hwndOwner = hWnd;
	    ofn.lpstrFilter = filtertxt;
	    ofn.lpstrFile = szFileName;
	    ofn.lpstrFileTitle = szFile;
	    ofn.nMaxFile = MAX_PATH;
	    ofn.nMaxFileTitle = sizeof(szFile);
	    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	    ofn.lpstrDefExt = "txt";
	    ofn.lpstrTitle = "Save As";

	    if(GetSaveFileName(&ofn) == 0) return 0;
	} else {
	    ofn.nFilterIndex = fformat;
	}

	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
			   FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
	    switch (ofn.nFilterIndex) {
		case 1:
		    save_CGEdit(hFile);
		    break;
		default:
		    break;
	    }
	    fformat = ofn.nFilterIndex;
	    CloseHandle(hFile);
	    undoResetModifiedFlag();
	    if (!nodlg) setwtitle(hWnd, 0); /* undoResetModifiedFlag() が呼んでくれないとき用 */
	}

	add_recent_file(szFileName);
	return 1;
}

void drawOverlayToBackBuf(LPBITMAPINFO bmp) {
	RECT r;
	int bw, bh, x;
	int w, h;
	double dw, scaling;

	w = 320 * (expansion+1);
	h = 200 * (expansion+1);
	SetRect(&r, 0, TATE, w, TATE+h);
	FillRect(ovbkbuf.hdcMem, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
	bw = ovBMP->bmiHeader.biWidth;
	bh = ovBMP->bmiHeader.biHeight;
	scaling = (double)h / (double)bh;
	dw = (double)bw * scaling;
	x = (int)(((double)w - dw) / 2.0);
	StretchDIBits(ovbkbuf.hdcMem,
		x, TATE,		/* dest pos */
		(int)dw, h,		/* dest width, height */
		0, 0,			/* src pos */
		bw, bh,			/* src width, height */
		(LPBYTE)(&ovBMP->bmiColors[0]) + sizeof(RGBQUAD) * ovBMP->bmiHeader.biClrUsed,
		ovBMP,			/* BITMAPINFO ptr */
		DIB_RGB_COLORS,		/* use RGB palette */
		SRCCOPY);		/* copy */
}

char szBMPFile[MAX_PATH];
char szBMPFileName[MAX_PATH];
int OpenBMPFile(HWND hWnd)
{
	DWORD dwSize = 0L;
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "BMP file(*.BMP)\0*.bmp\0\0";
	ofn.lpstrFile = szBMPFileName;
	ofn.lpstrFileTitle = szBMPFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = sizeof(szBMPFile);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "bmp";
	ofn.lpstrTitle = "Open BMP Fileｭ";

	if(GetOpenFileName(&ofn) == 0)
	    return 0;

	if (ovBMP != NULL) GlobalFree(ovBMP);
	ovBMP = loadBMPfile(szBMPFileName);
	if (ovBMP == NULL) return 0;

	drawOverlayToBackBuf(ovBMP);

	return 1;
}

/*----------------------------------------------------------------------
	Editflag
 ----------------------------------------------------------------------*/
void updateWinTitle(int editflag) {
	if (lastflag != editflag) {
	    lastflag = editflag;
	    setwtitle(bkbuf.hWnd, editflag);
	}
}

int isDisposalOK(void) {
	return (
	    !undoIsModified() ||
	    MessageBox(bkbuf.hWnd, "Do you want to discard your changes?", "CGE7",
		       MB_YESNO|MB_ICONQUESTION) == IDYES);
}

/*---------------------------------------------------------------------------*/
void refreshBackBuffer(HWND hwnd) {
	YOKO = getYOKO();
	TATE = getTATE();
	CPALX = getCPALX();

	destroyBackBuffer(&bkbuf);
	destroyBackBuffer(&ovbkbuf);

	createBackBuffer(hwnd, YOKO, TATE*2, &bkbuf);
	createBackBuffer(hwnd, YOKO, TATE*2, &ovbkbuf);
	drawframe();

	if (ovBMP) drawOverlayToBackBuf(ovBMP);
}

void removeZoomMenuSub(HMENU hMenu, UINT rmvID) {
	HMENU hSubMenu;
	UINT id;
	int i, f;
	f = 0;
	for(i = GetMenuItemCount(hMenu) - 1; i >= 0 ; i--) {
	    hSubMenu = GetSubMenu(hMenu, i);
	    if(hSubMenu) {
		removeZoomMenuSub(hSubMenu, rmvID);
	    } else {
		id = GetMenuItemID(hMenu, i);
		if (id >= IDM_ZOOM1 && id <= IDM_ZOOMMAX) {
		    f = 1;
		    if (id >= rmvID) {
			DeleteMenu(hMenu, id, MF_BYCOMMAND);
		    }
		}
	    }
	}
	if (f) CheckMenuRadioItem(hMenu, IDM_ZOOM1, IDM_ZOOMMAX, IDM_ZOOM1 + expansion, MF_BYCOMMAND);
}
void removeZoomMenu(HWND hwnd) {
	HMENU hMenu;
	HDC hdcScr;
	RECT r;
	int desktop_width, desktop_height;
	int window_width, window_height;
	int frame_width, frame_height;
	int expsav;
	int z;

	hdcScr = GetDC(NULL);
	desktop_width  = GetDeviceCaps(hdcScr, HORZRES);
	desktop_height = GetDeviceCaps(hdcScr, VERTRES);

	GetWindowRect(hwnd, &r);
	frame_width  = r.right  - r.left - 320;
	frame_height = r.bottom - r.top  - 200;

	expsav = expansion;

	for (z=0; ; z++) {
	    expansion = z;
	    window_width = getYOKO()  + frame_width;
	    window_height = getTATE() + frame_height;
	    if (window_width  > desktop_width ||
		window_height > desktop_height) break;
	}
	expansion = expsav;

	hMenu = GetMenu(hwnd);
	removeZoomMenuSub(hMenu, IDM_ZOOM1 + z);
}

char *get1arg(char *str, char *buf, int bufsiz) {
	char c;
	while (isspace(*str)) str++; /* skip spaces */
	if (!*str) return 0;
	while ((c = *str) != 0 && !isspace(c) && --bufsiz) {
	  str++;
	  if (c == '\"') {
	    while ((c = *str++) != 0 && c != '\"' && --bufsiz) *buf++ = c;
	    if (!c || !bufsiz) return 0;
	  } else
	    *buf++ = c;
	}
	if (!bufsiz) return 0;
	*buf = 0; /* EOS */
	return str;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow){

	HWND hwnd;
	MSG  msg;
	WNDCLASSEX  wndclass;
	HACCEL hAccel;
	RECT r;
	int i;
	char path[MAX_PATH], *p;

	hInst = hInstance;

	InitCommonControls();

	/* get args */
	if (get1arg(szCmdLine, path, MAX_PATH)) {
	    GetFullPathName(path, MAX_PATH, szFileName, &p);
	    strcpy(szFile, p);
	} else {
	    szFileName[0] = 0;
	}

	/* Set current directory to the EXE file path */
	if (GetModuleFileName(NULL, path, MAX_PATH)) {
	    if (PathRemoveFileSpec(path)) {
		SetCurrentDirectory(path);
	    }
	}

	load_config();

	bit3color = RGB(0xCC, 0x00, 0xCC);

	if (!loadmzfont("mz700fon.dat", 1)) {
	    MessageBox(NULL, (LPCSTR)"MZ700FON.DAT not found", (LPCSTR)"Error", MB_OK);
	    return 0;
	}

	for (i=0; i<9; i++) animsel[i].x1 = -1;

	wndclass.cbSize        = sizeof(wndclass);        /* 構造体の大きさ */
	wndclass.style         = CS_HREDRAW | CS_VREDRAW; /* スタイル */
	wndclass.lpfnWndProc   = WndProc;                 /* メッセージ処理関数 */
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;               /* プログラムのハンドル */
	wndclass.hIcon         = LoadIcon(hInstance, "myIcon"); /* アイコン */
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);     /* カーソル */
	wndclass.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH); /* ブラシ */
	wndclass.lpszMenuName  = "CGE7Menu";              /* メニュー */
	wndclass.lpszClassName = "CGE7 Window";   /* クラス名 */
	wndclass.hIconSm       = LoadIcon(hInstance, "myIconS");

	RegisterClassEx(&wndclass); /* ウインドウクラスTest Windowを登録 */

	hwnd = CreateWindow(
           "CGE7 Window",				/* ウインドウクラス名 */
	    wintitle,					/* ウインドウのタイトル */
            WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME),	/* ウインドウスタイル */
            CW_USEDEFAULT, CW_USEDEFAULT,		/* ウインドウ表示位置 */
            320,/*temp*/				/* ウインドウの大きさ */
            200,/*temp*/
            NULL,					/* 親ウインドウのハンドル */
            NULL,					/* メニューのハンドル */
            hInstance,					/* インスタンスのハンドル */
            NULL);					/* 作成時の引数保存用ポインタ */

	removeZoomMenu(hwnd);

	hAccel = LoadAccelerators(hInst, "myAccel");

	if (!undoInit()) return 0;

	refreshBackBuffer(hwnd);
	vram2disp(hwnd);

	make_toolbar(hwnd);
	GetWindowRect(hToolbar, &r);
	tbHeight = r.bottom - r.top;

	CreateMyStatusbar(hwnd);
	GetWindowRect(hStatusWnd, &r);
	sbHeight = r.bottom - r.top;
	ShowWindow(hStatusWnd, showstatusbar ? SW_SHOW : SW_HIDE);

	r.left   = r.top = 0;
	r.right  = YOKO;
	r.bottom = TATE + tbHeight;
	if (showstatusbar) r.bottom += sbHeight;
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME), TRUE/*menu*/);
	SetWindowPos(hwnd, 0, 0, 0, r.right - r.left, r.bottom - r.top,
		     SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER/*|SWP_NOREDRAW*/);
	SetWindowPos(hToolbar, 0, 0, 0, bkbuf.w, tbHeight,
		     SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER/*|SWP_NOREDRAW*/);

	if (szFileName[0]) {
	    fformat = 1;
	    OpenMyFileSub(hwnd, szFileName);
	}

	ShowWindow(hwnd,iCmdShow);      /* ウインドウを表示 */
	UpdateWindow(hwnd);

	fmini = getfont("Terminal", 8);

	while (GetMessage(&msg, NULL, 0, 0)) {
	    if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	}

	if (fmini != NULL) DeleteObject(fmini);

	destroyBackBuffer(&bkbuf);
	destroyBackBuffer(&ovbkbuf);
	undoTini();

	if (ovBMP != NULL) GlobalFree(ovBMP);

	save_config();

	return msg.wParam;
}

// ============================================================================
// --- Export RAW/Compressed
// ============================================================================

HANDLE openExportFile(HWND hWnd) {

	OPENFILENAME ofn;
	HANDLE hFile;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Assembly File(*.asm)\0*.asm\0Assembly File(*.z80)\0*.z80\0All files(*.*)\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = sizeof(szFile);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "asm";
	ofn.lpstrTitle = "Export";

	if (GetSaveFileName(&ofn) == 0) return NULL;

	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		return hFile;
	}
	else {
		MessageBox(hWnd, "Save Error", "Error", MB_ICONERROR | MB_OK);
		return NULL;
	}
}

void exportRaw(HWND hWnd) {
	HANDLE file = openExportFile(hWnd);
	if (file == NULL) { return;  }
	exportRawFile(file);
	CloseHandle(file);
}

void exportCompressed( HWND hWnd ) {
	HANDLE file = openExportFile(hWnd);
	if (file == NULL) { return; }
	exportCompressedFile(file);
	CloseHandle(file);
}

void exportBinary(HWND hWnd) {
	HANDLE file = openExportFile(hWnd);
	if (file == NULL) { return; }
	exportBinaryFile(file);
	CloseHandle(file);
}

/*---------------------------------------------------------------------------*/
void treatmenu(HWND hwnd, HMENU hMenu) {
	int id, pos, cnt;
	pos = 0;
	cnt = GetMenuItemCount(hMenu);
	for (pos = 0; pos < cnt; pos++) {
	    switch (id = GetMenuItemID(hMenu, pos)) {
		case IDM_COPY:
		case IDM_CUT:
		case IDM_FILL:
		case IDM_HREVERSE:
		case IDM_VREVERSE:
		case IDM_BITMAPCOPY:
		    EnableMenuItem(hMenu, id, (selection) ? MF_ENABLED : MF_GRAYED);
		    break;
		case IDM_PASTE:
		    EnableMenuItem(hMenu, id, (check_clipboard(hwnd)) ? MF_ENABLED : MF_GRAYED);
		    break;
		case IDM_UNDO:
		    EnableMenuItem(hMenu, id, (floater || undoCanUndo()) ? MF_ENABLED : MF_GRAYED);
		    break;
		case IDM_REDO:
		    EnableMenuItem(hMenu, id, undoCanRedo() ? MF_ENABLED : MF_GRAYED);
		    break;
		case IDM_SHOWBIT3:
		    CheckMenuItem(hMenu, id, showbit3 ? MF_CHECKED : MF_UNCHECKED);
		    break;
		case IDM_ANIM1:
		case IDM_ANIM2:
		case IDM_ANIM3:
		case IDM_ANIM4:
		case IDM_ANIM5:
		case IDM_ANIM6:
		case IDM_ANIM7:
		case IDM_ANIM8:
		case IDM_ANIM9: {
		    UINT f;
		    int i;
		    char buf[32];
		    i = id - IDM_ANIM1;
		    f = MF_BYPOSITION|MF_STRING;
		    if (animsel[i].x1 < 0) {
			f |= (selection && (selx1 >= 0) && (sely1 >= 0) &&
				(selx2 < 40) && (sely2 < 25) && !isAnimDlgOpen()) ? MF_ENABLED : MF_GRAYED;
			wsprintf(buf, "%s%d", "Set to frame #", i+1);
		    } else {
			f |= !isAnimDlgOpen() ? MF_ENABLED : MF_GRAYED;
			wsprintf(buf, "%s%d", "Delete frame #", i + 1);
		    }
		    ModifyMenu(hMenu, pos, f, id, buf);
		    break;
		}
		case IDM_ANIM0:
		case IDM_DO_ANIM: {
		    int i, f;
		    for (i=0, f=0; i<9; i++)
			if (animsel[i].x1 >= 0) { f=1; break; }
		    EnableMenuItem(hMenu, id, f && !isAnimDlgOpen() ? MF_ENABLED : MF_GRAYED);
		    break;
		}
		case IDM_SG_LEFT:
		case IDM_SG_RIGHT:
		case IDM_SG_UP:
		case IDM_SG_DOWN:
		    EnableMenuItem(hMenu, id, (selection || floater) ? MF_ENABLED : MF_GRAYED);
		    break;
		case IDM_RECENT_NONE:
		case IDM_RECENT_1:
		case IDM_RECENT_2:
		case IDM_RECENT_3:
		case IDM_RECENT_4:
		case IDM_RECENT_5:
		case IDM_RECENT_6:
		case IDM_RECENT_7:
		case IDM_RECENT_8:
		case IDM_RECENT_9: {
		    char *rfn;
		    char buf[MAX_PATH+4];
		    int i;
		    for (pos = cnt-1; pos >= 0; pos--) DeleteMenu(hMenu, pos, MF_BYPOSITION);
		    rfn = get_recent_file(0);
		    if (rfn == NULL) {
			AppendMenu(hMenu, MF_GRAYED, IDM_RECENT_NONE, "(none)");
		    } else for (i=0; rfn != NULL; rfn = get_recent_file(++i)) {
			wsprintf(buf, "&%d: %s", i+1, rfn);
			AppendMenu(hMenu, 0, IDM_RECENT_1 + i, buf);
		    }
		    return;
		}
		default:
		    break;
	    }
	}
}

void initmenucheck(HMENU hMenu) {
	CheckMenuItem(hMenu, IDM_GRID,      showgrid    ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SHOWSPACE, showdispc00 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MARKISTP,  mark_is_tp  ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_STATUSBAR, showstatusbar ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuRadioItem(hMenu, IDM_GRIDTYPE0, IDM_GRIDTYPE1, IDM_GRIDTYPE0 + gridtype, MF_BYCOMMAND);
}

/*---------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	int i;

	switch (iMsg) {

	    case WM_CREATE:
		initmenucheck(GetMenu(hwnd));
		cursorbrush = CreateSolidBrush(RGB(0xff, 0x80, 0x00));
		ShowWindow(hwnd, SW_SHOW);
		DragAcceptFiles(hwnd, TRUE);
		return 0;

	    case WM_PAINT: {
		hdc=BeginPaint(hwnd,&ps);
		if (!showoverlay || ovBMP == NULL) {
		    /* update from back buffer */
		    BitBlt(hdc,0,tbHeight,bkbuf.w,bkbuf.h+tbHeight,bkbuf.hdcMem,0,0,SRCCOPY);
		} else {
		    /* alpha blending */
		    BLENDFUNCTION bf;
		    int w, h;
		    bf.BlendOp = AC_SRC_OVER;
		    bf.BlendFlags = 0;
		    bf.AlphaFormat = 0;
		    bf.SourceConstantAlpha = 128;
		    w = 320 * (expansion+1);
		    h = 200 * (expansion+1);
		    BitBlt(ovbkbuf.hdcMem,0,0,bkbuf.w,TATE,bkbuf.hdcMem,0,0,SRCCOPY);
		    AlphaBlend(ovbkbuf.hdcMem,MAINX,MAINY,w,h,ovbkbuf.hdcMem,0,TATE,w,h,bf);
		    BitBlt(hdc,0,tbHeight,bkbuf.w,TATE+tbHeight,ovbkbuf.hdcMem,0,0,SRCCOPY);
		}
		/* chr palette cursor */
		drawpalcursor(hdc);
		/* selection band */
		draw_selection(hdc);
		/* floater */
		if (mark_is_tp && floater && (showbit3 || showdispc00)) blend_floater();
		draw_floater(hdc);
		/* animation frames */
		if (showanimframe) {
		    for (i=0; i<9; i++) {
			if (animsel[i].x1 >= 0)
			    drawanimrect(hdc, animsel[i].x1, animsel[i].y1, animsel[i].x2, animsel[i].y2, i+1);
		    }
		}

		EndPaint(hwnd,&ps);
		return 0;
	    }

	    case WM_SIZE:
		SendMessage(hToolbar, WM_SIZE, wParam, lParam);
		SendMessage(hStatusWnd, WM_SIZE, wParam, lParam);
		break;

	    case WM_DESTROY:
		DeleteObject(cursorbrush);
		hide_selection();
		PostQuitMessage(0);
		return 0;

	    case WM_CLOSE:
		if (!isDisposalOK()) return 0;
		break; /* do default proc */

	    case WM_CHAR: {
		int i;
		char c;
		c = (char)wParam;
		if (c >= '1' && c <= '8') {
		    i = IDTBB_F_BLACK + ((c - '1' + 1) & 7);
		    SendMessage(hToolbar, TB_CHECKBUTTON, i, TRUE);
		    forecolor = i - IDTBB_F_BLACK;
		    drawchrpalette();
		    updatechrpalette(hwnd);
		    return 0;
		} else if (c >= '!' && c <= '(') {
		    i = IDTBB_B_BLACK + ((c - '!' + 1) & 7);
		    SendMessage(hToolbar, TB_CHECKBUTTON, i, TRUE);
		    backcolor = i - IDTBB_B_BLACK;
		    drawchrpalette();
		    updatechrpalette(hwnd);
		    return 0;
		} else if (c == 'x' || c == '@') {
		    goto colswap;
		} else if (c == 'y') {
		    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_PENCIL, TRUE);
		    goto sel_pencil;
		} else if (c == 'm' || c == 0x1b) {
		    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_CURSOR, TRUE);
		    goto sel_cursor;
		} else if (c == 'p') {
		    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_PAINT, TRUE);
		    goto sel_paint;
		} else if (c == ' ') {
		    goto palswap;
		} else if (c == '9') {
		    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_BIT3, TRUE);
		    goto sel_bit3pen;
		}
	    }

	    case WM_SYSCHAR:
		/* Process META-Key + Char */
		i = (int)wParam;
		if ((lParam & (1<<29)) &&
		    ((i >= 'a' && i <= 'z') ||
		     (i >= '0' && i <= '9'))) {
//			keybufput(M(tolower(i)));
		} else break;
		return 0;

	    case WM_KEYDOWN: {
		BYTE	kb_stat[256];
		int	shift_ks, ctrl_ks;
		/* get current keyboard status */
		ZeroMemory(&kb_stat, sizeof(kb_stat));
		GetKeyboardState(kb_stat);
		shift_ks = !!(kb_stat[VK_SHIFT  ] & KEY_PRESSED);
		ctrl_ks  = !!(kb_stat[VK_CONTROL] & KEY_PRESSED);
		if (ctrl_ks) switch (wParam) {
		    case VK_LEFT:
			shift_sg_in_floater(0);
			updatescratcharea(hwnd);
			return 0;
		    case VK_RIGHT:
			shift_sg_in_floater(1);
			updatescratcharea(hwnd);
			return 0;
		    case VK_UP:
			shift_sg_in_floater(2);
			updatescratcharea(hwnd);
			return 0;
		    case VK_DOWN:
			shift_sg_in_floater(3);
			updatescratcharea(hwnd);
			return 0;
		    default:
			break;
		} else       switch (wParam) {
		    case VK_UP:
			if (chrpos >= 16) chrpos -= 16;
			updatechrpalette(hwnd);
			return 0;
		    case VK_DOWN:
			if (chrpos < 16*21) chrpos += 16;
			updatechrpalette(hwnd);
			return 0;
		    case VK_LEFT:
			chrpos = (chrpos & 0x0ff0) | (((chrpos & 0x000f) - 1) & 0x000f);
			updatechrpalette(hwnd);
			return 0;
		    case VK_RIGHT:
			chrpos = (chrpos & 0x0ff0) | (((chrpos & 0x000f) + 1) & 0x000f);
			updatechrpalette(hwnd);
			return 0;
//		    case VK_F9:
//			if (!eufont_ok) return 0;
//			cspalno ^= 2;
//			drawchrpalette();
//			updatechrpalette(hwnd);
//			return 0;
		    case VK_F9:
			goto showbit3swap;
		    case VK_F11:
			goto dispc00swap;
		    default:
			break;
		}
		break;
	    }

	    case WM_COMMAND:
		switch (LOWORD(wParam)) {
		    case IDM_COPY:
			if (selection) {
			    if (floater) {
				floater_to_clipboard(hwnd, selx2-selx1+1, sely2-sely1+1);
			    } else {
				send_to_clipboard(hwnd, selx1, sely1, selx2, sely2);
				hide_selection();
			    }
			}
			return 0;

		    case IDM_CUT:
			if (selection) {
			    if (floater) {
				floater_to_clipboard(hwnd, selx2-selx1+1, sely2-sely1+1);
				floater = 0;
				hide_selection();
			    } else {
				send_to_clipboard(hwnd, selx1, sely1, selx2, sely2);
				undoStartEntry();
				fill_selection(0, 0x70);
				hide_selection();
			    }
			}
			return 0;

		    case IDM_PASTE: {
			int w, h;
			if (currtool != IDTBB_CURSOR) {
			    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_CURSOR, TRUE);
			    currtool = IDTBB_CURSOR;
			    currcursor = 0;
			}
			land_floater();
			hide_selection();
			i = receive_from_clipboard(hwnd, &w, &h);
			if (i) {
			    int dx, dy;
			    getPastePos(w, h, &dx, &dy);
			    show_selection(dx, dy, w+dx-1, h+dy-1);
			    floater = 1;
			    refresh_floater();
			    updatescratcharea(hwnd);
			}
			return 0;
		    }

		    case IDM_BITMAPCOPY:
			if (selection) {
			    int savegrid;
			    savegrid = showgrid;
			    showgrid = 0;
			    if (floater) {
				refresh_floater();
			    } else {
				selection_to_floater();
				floater = 0;
			    }
			    showgrid = savegrid;
			    send_bitmap_to_clipboard(hwnd, bkbuf.hdcMem,
				(selx2-selx1+1)*(8*(expansion+1)), (sely2-sely1+1)*(8*(expansion+1)));
			    if (floater) {
				refresh_floater();
				updatescratcharea(hwnd);
			    } else {
				hide_selection();
			    }
			}
			return 0;

		    case IDM_UNDO: {
			int f;
			f = floater;
			if (floater) {
			    floater = 0;
			    hide_selection();
			    if (f == 1) return 0;
			}
			undo();
			if (f == 2) undoCancelRedo();
			updatescratcharea(hwnd);
			return 0;
		    }

		    case IDM_REDO:
			redo();
			updatescratcharea(hwnd);
			return 0;

		    case IDM_SELALL:
			if (currtool != IDTBB_CURSOR) {
			    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_CURSOR, TRUE);
			    currtool = IDTBB_CURSOR;
			    currcursor = 0;
			}
			land_floater();
			show_selection(0, 0, 39, 24);
			return 0;

		    case IDM_FILL:
			if (selection) {
			    int c, a;
			    land_floater();
			    undoStartEntry();
			    get_chr_attr(&c, &a);
			    fill_selection(c, a);
			    hide_selection();
			}
			return 0;

		    case IDM_HREVERSE:
			if (selection) {
			    int doland;
			    doland = 0;
			    if (!floater) {
				selection_to_floater();
				doland = 1;
			    }
			    hrev_floater();
			    if (doland) land_floater();
			    updatescratcharea(hwnd);
			}
			return 0;

		    case IDM_VREVERSE:
			if (selection) {
			    int doland;
			    doland = 0;
			    if (!floater) {
				selection_to_floater();
				doland = 1;
			    }
			    vrev_floater();
			    if (doland) land_floater();
			    updatescratcharea(hwnd);
			}
			return 0;

		    case IDM_STRINPUT:
			land_floater();
			if (currtool != IDTBB_CURSOR) {
			    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_CURSOR, TRUE);
			    currtool = IDTBB_CURSOR;
			    currcursor = 0;
			}
			openTextInputDialog(hInst, hwnd);
			updatescratcharea(hwnd);
			return 0;

		    case IDM_SGMODE:
			semigrapen ^= 1;
			semigrapen &= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_SGMODE, (semigrapen & 1) ? MF_CHECKED : MF_UNCHECKED);
			return 0;

		    case IDM_SG_LEFT:
			shift_sg_in_floater(0);
			updatescratcharea(hwnd);
			return 0;

		    case IDM_SG_RIGHT:
			shift_sg_in_floater(1);
			updatescratcharea(hwnd);
			return 0;
		    case IDM_SG_UP:
			shift_sg_in_floater(2);
			updatescratcharea(hwnd);
			return 0;
		    case IDM_SG_DOWN:
			shift_sg_in_floater(3);
			updatescratcharea(hwnd);
			return 0;

		    case IDTBB_CURSOR:
sel_cursor:		currtool = IDTBB_CURSOR;
			land_floater();
			hide_selection();
			currcursor = 0;
			return 0;

		    case IDTBB_PENCIL:
sel_pencil:		currtool = IDTBB_PENCIL;
			land_floater();
			hide_selection();
			currcursor = 0;
			return 0;

		    case IDTBB_PAINT:
sel_paint:		currtool = IDTBB_PAINT;
			land_floater();
			hide_selection();
			currcursor = 0;
			return 0;

		    case IDTBB_BIT3:
sel_bit3pen:		currtool = IDTBB_BIT3;
			bit3pen = 1;
			land_floater();
			hide_selection();
			currcursor = 0;
			if (showbit3 == 0) {
			    showbit3 = 1;
			    vram2disp(hwnd);
			}
			return 0;

		    case IDTBB_SAVE:
			goto savefile;

		    case IDTBB_F_BLACK:
		    case IDTBB_F_BLUE:
		    case IDTBB_F_RED:
		    case IDTBB_F_MAGENTA:
		    case IDTBB_F_GREEN:
		    case IDTBB_F_CYAN:
		    case IDTBB_F_YELLOW:
		    case IDTBB_F_WHITE: {
			forecolor = LOWORD(wParam) - IDTBB_F_BLACK;
			drawchrpalette();
			updatechrpalette(hwnd);
			return 0;
		    }
		    case IDTBB_B_BLACK:
		    case IDTBB_B_BLUE:
		    case IDTBB_B_RED:
		    case IDTBB_B_MAGENTA:
		    case IDTBB_B_GREEN:
		    case IDTBB_B_CYAN:
		    case IDTBB_B_YELLOW:
		    case IDTBB_B_WHITE: {
			backcolor = LOWORD(wParam) - IDTBB_B_BLACK;
			drawchrpalette();
			updatechrpalette(hwnd);
			return 0;
		    }
		    case IDTBB_SWAP:
colswap:		SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_F_BLACK + backcolor, TRUE);
			SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_B_BLACK + forecolor, TRUE);
			i = forecolor; forecolor = backcolor; backcolor = i;
			drawchrpalette();
			updatechrpalette(hwnd);
			return 0;

		    case IDM_GRID:
			showgrid ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_GRID, showgrid ? MF_CHECKED : MF_UNCHECKED);
			drawgrid();
			if (floater) refresh_floater();
			vram2disp(hwnd);
			return 0;

		    case IDM_GRIDTYPE0:
		    case IDM_GRIDTYPE1: {
			UINT id;
			id = LOWORD(wParam);
			gridtype = id - IDM_GRIDTYPE0;
			CheckMenuRadioItem(GetMenu(hwnd), IDM_GRIDTYPE0, IDM_GRIDTYPE1, id, MF_BYCOMMAND);
			drawframe();
			if (floater) refresh_floater();
			vram2disp(hwnd);
			return 0;
		    }

		    case IDM_EXPAND:
		    case IDM_ZOOM1:
		    case IDM_ZOOM2:
		    case IDM_ZOOM3:
		    case IDM_ZOOM4:
		    case IDM_ZOOM5:
		    case IDM_ZOOM6: {
			RECT r;
			UINT id;
			id = LOWORD(wParam);
			if (id == IDM_EXPAND) {
			    expansion = (expansion) ? 0 : zoomratio;
			    id = IDM_ZOOM1 + expansion;
			} else {
			    expansion = id - IDM_ZOOM1;
			    if (expansion) zoomratio = expansion;
			}
			CheckMenuRadioItem(GetMenu(hwnd), IDM_ZOOM1, IDM_ZOOMMAX, id, MF_BYCOMMAND);
			refreshBackBuffer(hwnd);
			r.left   = r.top = 0;
			r.right  = YOKO;
			r.bottom = TATE;
			r.bottom += tbHeight;
			if (showstatusbar) r.bottom += sbHeight;
			AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME), TRUE/*menu*/);
			SetWindowPos(hwnd, 0, 0, 0, r.right - r.left, r.bottom - r.top,
				     SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER/*|SWP_NOREDRAW*/);
			if (floater) refresh_floater();
			vram2disp(hwnd);
			return 0;
		    }

		    case IDM_STATUSBAR: {
			RECT r;
			showstatusbar ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_STATUSBAR, showstatusbar ? MF_CHECKED : MF_UNCHECKED);
			ShowWindow(hStatusWnd, showstatusbar ? SW_SHOW : SW_HIDE);
			r.left   = r.top = 0;
			r.right  = YOKO;
			r.bottom = TATE;
			r.bottom += tbHeight;
			if (showstatusbar) r.bottom += sbHeight;
			AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME), TRUE/*menu*/);
			SetWindowPos(hwnd, 0, 0, 0, r.right - r.left, r.bottom - r.top,
				     SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER/*|SWP_NOREDRAW*/);
			return 0;
		    }

		    case IDM_ALTPAL:
palswap:		cspalno ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_ALTPAL, (cspalno & 1) ? MF_CHECKED : MF_UNCHECKED);
			drawchrpalette();
			updatechrpalette(hwnd);
			return 0;

		    case IDM_ANIMFRAME:
			showanimframe ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_ANIMFRAME, showanimframe ? MF_CHECKED : MF_UNCHECKED);
			updatescratcharea(hwnd);
			return 0;

		    case IDM_OVERLAY:
ovswap:			showoverlay ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_OVERLAY, showoverlay ? MF_CHECKED : MF_UNCHECKED);
			updatescratcharea(hwnd);
			return 0;

		    case IDM_SHOWSPACE:
dispc00swap:		showdispc00 ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_SHOWSPACE, showdispc00 ? MF_CHECKED : MF_UNCHECKED);
			if (mark_is_tp && floater && !showdispc00) refresh_floater();
			vram2disp(hwnd);
			return 0;

		    case IDM_SHOWBIT3:
showbit3swap:		showbit3 ^= 1;
			if (mark_is_tp && floater && !showbit3) refresh_floater();
			vram2disp(hwnd);
			if (!showbit3 && currtool == IDTBB_BIT3) {
			    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_PENCIL, TRUE);
			    goto sel_pencil;
			}
			return 0;

		    case IDM_MARKISTP:
			mark_is_tp ^= 1;
			CheckMenuItem(GetMenu(hwnd), IDM_MARKISTP, mark_is_tp ? MF_CHECKED : MF_UNCHECKED);
			if (mark_is_tp == 0 && floater) refresh_floater();
			updatescratcharea(hwnd);
			return 0;

		    case IDM_ANIM1:
		    case IDM_ANIM2:
		    case IDM_ANIM3:
		    case IDM_ANIM4:
		    case IDM_ANIM5:
		    case IDM_ANIM6:
		    case IDM_ANIM7:
		    case IDM_ANIM8:
		    case IDM_ANIM9:
			i = LOWORD(wParam) - IDM_ANIM1;
			if (animsel[i].x1 >= 0) {
			    animsel[i].x1 = -1;
			    updatescratcharea(hwnd);
			} else {
			    if (selection && (selx1 >= 0) && (sely1 >= 0) && (selx2 < 40) && (sely2 < 25)) {
				animsel[i].x1 = selx1;
				animsel[i].y1 = sely1;
				animsel[i].x2 = selx2;
				animsel[i].y2 = sely2;
				land_floater();
				hide_selection();
			    }
			}
			return 0;

		    case IDM_ANIM0:
			for (i=0; i<9; i++) animsel[i].x1 = -1;
			updatescratcharea(hwnd);
			return 0;

		    case IDM_DO_ANIM:
			if (!isAnimDlgOpen()) {
			    openAnimDialog(hwnd, 0);
			}
			return 0;

		    case IDM_TEST: {
			if (OpenBMPFile(hwnd)) {
			    showoverlay = 1;
			    InvalidateRect(hwnd, NULL, FALSE);
			}
//openAnimDialog(hwnd, 0);
//			HANDLE hFile;
//			hFile = CreateFile("pal.c", GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
//					   FILE_ATTRIBUTE_NORMAL, NULL);
//			save_palette(hFile);
//			CloseHandle(hFile);
			return 0;
		    }
		    case IDM_ABOUT:
			MessageBox(hwnd, "MZ-700 Character Graphics Editor v" TS_VERSTRING "\nBased on Youkan's version " VERSTRING, "About", MB_OK);
			return 0;

		    case IDM_SAVE:
savefile:		land_floater();
			hide_selection();
			WriteMyFile(hwnd, !!szFileName[0]);
			return 0;
		    case IDM_SAVEAS:
			land_floater();
			hide_selection();
			WriteMyFile(hwnd, 0);
			return 0;
		    case IDM_OPEN:
			if (!isDisposalOK()) return 0;
			OpenMyFile(hwnd);
			return 0;
		    case IDM_LOADBMP: {
			if (OpenBMPFile(hwnd)) {
			    if (!showoverlay) goto ovswap;
			    InvalidateRect(hwnd, NULL, FALSE);
			}
			return 0;
		    }
		    case IDM_RECENT_1:
		    case IDM_RECENT_2:
		    case IDM_RECENT_3:
		    case IDM_RECENT_4:
		    case IDM_RECENT_5:
		    case IDM_RECENT_6:
		    case IDM_RECENT_7:
		    case IDM_RECENT_8:
		    case IDM_RECENT_9: {
			char *fn;
			if (!isDisposalOK()) return 0;
			fn = get_recent_file(LOWORD(wParam) - IDM_RECENT_1);
			if (fn == NULL) {
			    MessageBox(hwnd, "Cannot find recent filename???", "Error", MB_OK);
			    return 0;
			}
			strcpy(szFileName, fn);
			strcpy(szFile, PathFindFileName(fn));
			fformat = 1;
			OpenMyFileSub(hwnd, szFileName);
			return 0;
		    }

		    case IDM_END:
			SendMessage(hwnd, WM_CLOSE, 0, 0L);
			return 0;


			case IDM_EXPORT_RAW:
				land_floater();
				hide_selection();
				exportRaw( hwnd );
				return 0;

			case IDM_EXPORT_COMPRESSED:
				land_floater();
				hide_selection();
				exportCompressed( hwnd );
				return 0;

			case IDM_EXPORT_BINARY:
				land_floater();
				hide_selection();
				exportBinary( hwnd );
				return 0;


		    default:
			break;
		}
		break;

	    case WM_LBUTTONDOWN: {
		int cx, cy, x, y, i;
		cx = LOWORD(lParam);
		cy = HIWORD(lParam) - tbHeight;
		if ((i = winxy2palindex(cx, cy)) >= 0) {
		    chrpos = i;
		    updatechrpalette(hwnd);
		    return 0;
		}
		if (currtool == IDTBB_PENCIL) {
		    if (winxy2vramxy(cx, cy, &x, &y)) {
			int i, c, a;
			i = y*40+x;
			get_chr_attr(&c, &a);
			a |= atr[i] & 0x08;
			if (semigrapen) {
			    int sx, sy, sr, m;
			    winxy2sgxy(cx, cy, &sx, &sy);
			    m = sgxy2mask(sx, sy);
			    sr = ((chr[i] & 0xF0) != 0xF0) || !(chr[i] & m);
			    if (sr) c =  chr[i] | 0xF0  |  m;
			    else    c = (chr[i] | 0xF0) & ~m;
			    semigrapen = (semigrapen & 1) | (sr << 1);
			}
			if (wParam & MK_SHIFT) {
			    c = chr[i];
			    a = (a & 0x7F) | (atr[i] & 0x80);
			} else if (wParam & MK_CONTROL) {
			    a = atr[i];
			}
			undoStartEntry();
			pset(x, y, c, a);
			updatescratcharea(hwnd);
			lastvx = x;
			lastvy = y;
			dragmode = 1 + !!(wParam & MK_CONTROL) + (!!(wParam & MK_SHIFT))*2;
		    }
		} else if (currtool == IDTBB_CURSOR) {
		    i = is_onselection(cx, cy);
		    if (i == 5) {
			if (!floater) {
			    selection_to_floater();
			    floater = 2;
			    undoStartEntry();
			    fill_selection(0, 0x70);
			}
			dragmode = 5;
			break;
		    } else if (i) {
			dragmode = i;
			break;
		    }
		    if (winxy2vramxy(cx, cy, &x, &y)) {
			land_floater();
			hide_selection();
			lastvx = dragx = x;
			lastvy = dragy = y;
		    }
		} else if (currtool == IDTBB_PAINT) {
		    if (winxy2vramxy(cx, cy, &x, &y)) {
			int c, a;
			get_chr_attr(&c, &a);
			if (semigrapen) {
			    int sx, sy;
			    winxy2sgxy(cx, cy, &sx, &sy);
			    seedfill_sg(sx, sy, a);
			} else
			    seedfill(x, y, c, a);
			updatescratcharea(hwnd);
		    }
		} else if (currtool == IDTBB_BIT3) {
		    if (winxy2vramxy(cx, cy, &x, &y)) {
			int i, c, a;
			i = y*40+x;
			c = chr[i];
			a = atr[i];
			if (!(a & 0x08)) {
			    bit3pen = 3;
			    a |= 0x08;
			} else {
			    bit3pen = 1;
			    a &= ~0x08;
			}
			undoStartEntry();
			pset(x, y, c, a);
			updatescratcharea(hwnd);
			lastvx = x;
			lastvy = y;
			dragmode = 1;
		    }
		}
		break;
	    }

	    case WM_LBUTTONUP:
		dragmode = 0;
		dragx = dragy = -1;
		break;

	    case WM_RBUTTONDOWN: {
		int cx, cy, x, y;
		cx = LOWORD(lParam);
		cy = HIWORD(lParam) - tbHeight;
		if (winxy2vramxy(cx, cy, &x, &y)) {
		    int i, a, c;
		    i = y*40+x;
		    a = atr[i];
		    c = chr[i] | ((a & 0x80) << 1);
		    if (!(wParam & MK_CONTROL)) {
			forecolor = (a & 0x70) >> 4;
			backcolor = (a & 0x07);
		    }
		    if (!(wParam & MK_SHIFT)) {
			int *p;
//			p = (cspalno) ? cspal2 : cspal;
			p = &cspal[CPAL_NUM * cspalno];
			for (i=0; i<352; i++)
			    if ((*p++ & (0x1ff|CPAL_REVERSE|CPAL_PRCOLOR)) == c) break;
			if (i == 352) { *(--p) = c; i--; }
			chrpos = i;
		    }
		    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_F_BLACK + forecolor, TRUE);
		    SendMessage(hToolbar, TB_CHECKBUTTON, IDTBB_B_BLACK + backcolor, TRUE);
		    drawchrpalette();
		    updatechrpalette(hwnd);
		}
		break;
	    }

	    case WM_MOUSEMOVE: {
		int cx, cy, x, y, i, j;
		char buf[256];
		cx = LOWORD(lParam);
		cy = HIWORD(lParam) - tbHeight;
		i = is_onselection(cx, cy);
		if (currtool == IDTBB_CURSOR && !(wParam & MK_LBUTTON)) currcursor = i;
		if (showstatusbar && cx >= CPALX && !(wParam & MK_LBUTTON) &&
		    (i = winxy2palindex(cx, cy)) >= 0) {
		    if (lastchrpos != i) {
			int c, a;
			buf[0] = 0;
			setStatusBarStr(0, buf);
			j = chrpos; // save
			chrpos = i;
			get_chr_attr(&c, &a);
			chrpos = j; // restore
			wsprintf(buf, "Chr: %02X h  Attr: %02X h", c, a);
			setStatusBarStr(1, buf);
			lastchrpos = i;
			lastvx = lastvy = -1;
		    }
		    break;
		}
		if (winxy2vramxy2(cx, cy, &x, &y) &&
		    (x != lastvx || y != lastvy || (semigrapen && currtool == IDTBB_PENCIL))) {
		    if (currtool == IDTBB_PENCIL && (wParam & MK_LBUTTON) && dragmode) {
			int i, c, a;
			i = y*40+x;
			get_chr_attr(&c, &a);
			a |= atr[i] & 0x08;
			if (semigrapen) {
			    int sx, sy, sr, m;
			    winxy2sgxy(cx, cy, &sx, &sy);
			    m = sgxy2mask(sx, sy);
			    sr = (semigrapen & 0x02);
			    if (sr) c =  chr[i] | 0xF0  |  m;
			    else    c = (chr[i] | 0xF0) & ~m;
			}
			if (dragmode == 3) {
			    c = chr[i];
			    a = (a & 0x7f) | (atr[i] & 0x80);
			} else if (dragmode == 2) {
			    a = atr[i];
			} else if (wParam & MK_SHIFT) {
			    a = (a & 0x7f) | (atr[i] & 0x80);
			}
			pset(x, y, c, a);
			updatescratcharea(hwnd);
		    } else if (currtool == IDTBB_CURSOR && (wParam & MK_LBUTTON)) {
			switch (dragmode) {
			    case 0:
				if (dragx < 0) break;
				show_selection(dragx, dragy, x, y);
				break;
			    case 1: /* left-bottom */
				if (x < 0     || x > selx2) x = -1;
				if (y < sely1 || y > 24   ) y = -1;
				if (x >= 0 || y >= 0)
				    show_selection((x >= 0) ? x : selx1, sely1, selx2, (y >= 0) ? y : sely2);
				break;
			    case 2: /* bottom */
				if (y >= sely1 && y <= 24)
				    show_selection(selx1, sely1, selx2, y);
				break;
			    case 3: /* right-bottom */
				if (x < selx1 || x > 39) x = -1;
				if (y < sely1 || y > 24) y = -1;
				if (x >= 0 || y >= 0)
				    show_selection(selx1, sely1, (x >= 0) ? x : selx2, (y >= 0) ? y : sely2);
				break;
			    case 4: /* left */
				if (x >= 0 && x <= selx2)
				    show_selection(x, sely1, selx2, sely2);
				break;
			    case 6: /* right */
				if (x >= selx1 && x <= 39)
				    show_selection(selx1, sely1, x, sely2);
				break;
			    case 7: /* left-top */
				if (x < 0 || x > selx2) x = -1;
				if (y < 0 || y > sely2) y = -1;
				if (x >= 0 || y >= 0)
				    show_selection((x >= 0) ? x : selx1, (y >= 0) ? y : sely1, selx2, sely2);
				break;
			    case 8: /* top */
				if (y >= 0 && y <= sely2)
				    show_selection(selx1, y, selx2, sely2);
				break;
			    case 9: /* right-top */
				if (x < selx1 || x > 39   ) x = -1;
				if (y < 0     || y > sely2) y = -1;
				if (x >= 0 || y >= 0)
				    show_selection(selx1, (y >= 0) ? y : sely1, (x >= 0) ? x : selx2, sely2);
				break;
			    case 5: /* center */
				j = (x - lastvx);
				i = (y - lastvy);
				show_selection(selx1+j, sely1+i, selx2+j, sely2+i);
				break;
			}
			wsprintf(buf, "[%2d, %2d]", selx2-selx1+1, sely2-sely1+1);
			setStatusBarStr(0, buf);
		    }
		    if (showstatusbar && !(wParam & MK_LBUTTON)) {
			if ((lastvx != x || lastvy != y)) {
			    wsprintf(buf, "(%2d, %2d)", x, y);
			    setStatusBarStr(0, buf);
			    wsprintf(buf, "Chr: %02X h  Attr: %02X h", chr[y*40+x], atr[y*40+x]);
			    setStatusBarStr(1, buf);
			    lastchrpos = -1;
			}
		    }
		    lastvx = x;
		    lastvy = y;
		} else if (currtool == IDTBB_BIT3 && (wParam & MK_LBUTTON) && dragmode) {
			int i, c, a;
			i = y*40+x;
			c = chr[i];
			a = atr[i];
			if (bit3pen & 0x02) a |= 0x08;
			else a &= ~0x08;
			pset(x, y, c, a);
			updatescratcharea(hwnd);
		}
		break;
	    }
	    case WM_SETCURSOR:
		if (currcursor) {
		    mySetCursor(currcursor);
		    return 0;
		}
		break;

	    case WM_INITMENUPOPUP:
		treatmenu(hwnd, (HMENU)wParam);
		break;

	    case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, 0, szFileName, MAX_PATH);
		if (isDisposalOK()) {
		    strcpy(szFile, PathFindFileName(szFileName));
		    fformat = 1;
		    OpenMyFileSub(hwnd, szFileName);
		}
		DragFinish((HDROP)wParam);
		break;

	    default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);

}
