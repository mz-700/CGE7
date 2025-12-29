/*=====================================================================
	save/load files
 ======================================================================*/
#include <windows.h>
#include "CGE7.h"

extern int TATE;
extern int chr[1000];
extern int atr[1000];
extern int floaterchr[1000];
extern int floateratr[1000];
extern AnimBox animsel[9];

int RecentFileIndex[9] = { -1,-1, -1, -1, -1, -1, -1, -1, -1 };
char RecentFiles[9][MAX_PATH];

/*----------------------------------------------------------------------
	CGEdit format
 ----------------------------------------------------------------------*/
void save_CGEdit(HANDLE hFile) {
	char buf[520];
	long l, w;
	int i;

	l = 0;

	/* CGROM index */
	for (i=0; i<1000; i++) {
	    l += wsprintf(&buf[l], "%d,", chr[i]+((atr[i] & 0x80)<<1)+((atr[i] & 0x08)<<6));
	    if (l > 512) {
		WriteFile(hFile, buf, l, &w, NULL);
		l = 0;
	    }
	}
	/* fore color */
	for (i=0; i<1000; i++) {
	    l += wsprintf(&buf[l], "%d,", ((atr[i] & 0x70)>>4));
	    if (l > 512) {
		WriteFile(hFile, buf, l, &w, NULL);
		l = 0;
	    }
	}
	/* background color */
	for (i=0; i<999; i++) {
	    l += wsprintf(&buf[l], "%d,", (atr[i] & 0x07));
	    if (l > 512) {
		WriteFile(hFile, buf, l, &w, NULL);
		l = 0;
	    }
	}
	l += wsprintf(&buf[l], "%d", (atr[i] & 0x07));
	WriteFile(hFile, buf, l, &w, NULL);
}

int load_CGEdit(char *buf) {
	int i, j;
	char c;

	/* CGROM index */
	for (i=0; i<1000; i++) {
	    j = 0;
	    while ( (c=*buf++) && c >= '0' && c <= '9') j = j * 10 + (int)(c - '0');
	    atr[i] = ((j & 0x0100) >> 1) + ((j & 0x0200) >> 6);
	    chr[i] = (j & 0x00ff);
	    if (!c || c != ',') return 0;
	}
	/* fore color */
	for (i=0; i<1000; i++) {
	    j = 0;
	    while ( (c=*buf++) && c >= '0' && c <= '9') j = j * 10 + (int)(c - '0');
	    atr[i] |= j << 4;
	    if (!c || c != ',') return 0;
	}
	/* background color */
	for (i=0; i<1000; i++) {
	    j = 0;
	    while ( (c=*buf++) && c >= '0' && c <= '9') j = j * 10 + (int)(c - '0');
	    atr[i] |= j;
	    if ((i < 999) & (!c || c != ',')) return 0;
	}
	return 1;
}

/*----------------------------------------------------------------------
	clipboard
	format:
	  xx,yy			(decimal)
	  dd dd dd dd ...	(hex, xxx*yyy counts)
 ----------------------------------------------------------------------*/
void send_to_clipboard(HWND hwnd, int x1, int y1, int x2, int y2) {
	HGLOBAL hG;
	char *p;
	int len, c;
	int i, j, w, h;
	w = (x2-x1+1);
	h = (y2-y1+1);
	len = w*h*3;				/* length of data string */
	len += (len + 15) >> 4;			/* add size of CR+LF */
	len *= 2;				/* chr + attr */
	len += 8;				/* add width&height, CR+LF, EOF */
	hG = GlobalAlloc(GHND, len);
	p = (char *)GlobalLock(hG);
	p += wsprintf(p, "%2d %2d\n", w, h);
	/* chr */
	c = 0;
	for (i = y1; i <= y2; i++) {
	    for (j = x1; j <= x2; j++) {
		p += wsprintf(p, "%02X ", chr[i*40+j]);
		c++;
		if (c == 16) {
		    c = 0;
		    p--;
		    p += wsprintf(p, "\n");
		}
	    }
	}
	if (c != 0) {
	    c = 0;
	    p--;
	    p += wsprintf(p, "\n");
	}
	/* attr */
	c = 0;
	for (i = y1; i <= y2; i++) {
	    for (j = x1; j <= x2; j++) {
		p += wsprintf(p, "%02X ", atr[i*40+j]);
		c++;
		if (c == 16) {
		    c = 0;
		    p--;
		    p += wsprintf(p, "\n");
		}
	    }
	}
	if (c != 0) {
	    c = 0;
	    p--;
	    p += wsprintf(p, "\n");
	}
	GlobalUnlock(hG);
	if (!OpenClipboard(hwnd)) {
	    GlobalFree(hG);
	    return;
	}
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hG);
	CloseClipboard();
}
void floater_to_clipboard(HWND hwnd, int w, int h) {
	HGLOBAL hG;
	char *p;
	int len, c;
	int i, j;
	len = w*h*3;				/* length of data string */
	len += (len + 15) >> 4;			/* add size of CR+LF */
	len *= 2;				/* chr + attr */
	len += 8;				/* add width&height, CR+LF, EOF */
	hG = GlobalAlloc(GHND, len);
	p = (char *)GlobalLock(hG);
	p += wsprintf(p, "%2d %2d\n", w, h);
	/* chr */
	c = 0;
	for (i = 0; i < h; i++) {
	    for (j = 0; j < w; j++) {
		p += wsprintf(p, "%02X ", floaterchr[i*40+j]);
		c++;
		if (c == 16) {
		    c = 0;
		    p--;
		    p += wsprintf(p, "\n");
		}
	    }
	}
	if (c != 0) {
	    c = 0;
	    p--;
	    p += wsprintf(p, "\n");
	}
	/* attr */
	c = 0;
	for (i = 0; i < h; i++) {
	    for (j = 0; j < w; j++) {
		p += wsprintf(p, "%02X ", floateratr[i*40+j]);
		c++;
		if (c == 16) {
		    c = 0;
		    p--;
		    p += wsprintf(p, "\n");
		}
	    }
	}
	if (c != 0) {
	    c = 0;
	    p--;
	    p += wsprintf(p, "\n");
	}
	GlobalUnlock(hG);
	if (!OpenClipboard(hwnd)) {
	    GlobalFree(hG);
	    return;
	}
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hG);
	CloseClipboard();
}
char *skipspaces(char *p) {
	char c;
	while (1) {
	    c = *p;
	    if (c == 0) return p;
	    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z')) return p;
	    p++;
	}
}
char *get_dec(char *p, int *ip) {
	int i;
	p = skipspaces(p);
	i = 0;
	while (*p >= '0' && *p <= '9') {
	    i *= 10;
	    i += (int)(*p - '0');
	    p++;
	}
	*ip = i;
	return p;
}
char *get_hex(char *p, int *ip) {
	char c;
	int i;
	p = skipspaces(p);
	*ip = 0;
	while (1) {
	    c = *p;
	    if (c == 0) return p;
	    if (*p >= '0' && *p <= '9') {
		i = (int)(c - '0');
	    } else if (c >= 'a' && c <= 'z') {
		i = (int)(c - 'a' + 10);
	    } else if (c >= 'A' && c <= 'Z') {
		i = (int)(c - 'A' + 10);
	    } else return p;
	    *ip = ((*ip)<<4) + i;
	    p++;
	}
}
int check_clipboard(HWND hwnd) {
	HGLOBAL hG;
	char *p;
	int i, j, w, h;
	int res = 0;
	if (!IsClipboardFormatAvailable(CF_TEXT)) return 0;
	OpenClipboard(hwnd);
	hG = (HGLOBAL)GetClipboardData(CF_TEXT);
	if (hG == NULL) {
	    CloseClipboard();
	    return 0;
	}
	p = (char *)GlobalLock(hG);
	p = get_dec(p, &w);
	if (!w || w > 40) goto xit;
	p = get_dec(p, &h);
	if (!h || h > 25) goto xit;
	for (i = 0; i < w*h*2; i++) {
	    p = get_hex(p, &j);
	    if (!*p) goto xit;
	}
	res = 1;
xit:	GlobalUnlock(hG);
	CloseClipboard();
	return res;
}
int receive_from_clipboard(HWND hwnd, int *wp, int *hp) {
	HGLOBAL hG;
	char *p;
	int d, i, j, w, h;
	int res = 0;
	if (!IsClipboardFormatAvailable(CF_TEXT)) return 0;
	OpenClipboard(hwnd);
	hG = (HGLOBAL)GetClipboardData(CF_TEXT);
	if (hG == NULL) {
	    CloseClipboard();
	    return 0;
	}
	p = (char *)GlobalLock(hG);
	p = get_dec(p, &w);
	if (!w || w > 40) goto xit;
	p = get_dec(p, &h);
	if (!h || h > 25) goto xit;
	for (i = 0; i < h; i++) {
	    for (j = 0; j < w; j++) {
		p = get_hex(p, &d);
		if (!*p) goto xit;
		floaterchr[i*40+j] = d;
	    }
	}
	for (i = 0; i < h; i++) {
	    for (j = 0; j < w; j++) {
		p = get_hex(p, &d);
		if (!*p) goto xit;
		floateratr[i*40+j] = d;
	    }
	}
	*wp = w;
	*hp = h;
	res = 1;
xit:	GlobalUnlock(hG);
	CloseClipboard();
	return res;
}

void send_bitmap_to_clipboard(HWND hwnd, HDC hdc, int w, int h) {
	HDC hdc_mem, hdcwin;
	HBITMAP hBitmap;

	hdcwin = GetDC(hwnd);
	hdc_mem = CreateCompatibleDC(hdcwin);
	hBitmap = CreateCompatibleBitmap(hdcwin, w, h);
	if(hBitmap) {
	    SelectObject(hdc_mem, hBitmap);
	    BitBlt(hdc_mem, 0, 0, w, h, hdc, FLOATERX, FLOATERY, SRCCOPY);
	    OpenClipboard(hwnd);
	    EmptyClipboard();
	    SetClipboardData(CF_BITMAP, hBitmap);
	    CloseClipboard();
	}
	DeleteDC(hdc_mem);
	ReleaseDC(hwnd, hdcwin);
}

/*----------------------------------------------------------------------
	custom palette
 ----------------------------------------------------------------------*/
void save_palette(HANDLE hFile) {
	char buf[520];
	long l, w;
	int i, j, k;

	l = 0;

	for (i=0; i<8; i++) {
	    for (j=0; j<16; j++) {
		k = i*40 + j;
		l = wsprintf(buf, "0x%06x|CPAL_PRCOLOR,%s",
			    chr[k]+((atr[k] & 0x80)<<1)+((atr[k] & 0x77)<<16),
			    (j == 15) ? "\n" : " ");
		WriteFile(hFile, buf, l, &w, NULL);
	    }
	}
	for (i=8; i<22; i++) {
	    for (j=0; j<16; j++) {
		k = i*40 + j;
		l = wsprintf(buf, "0x%03x%s,%s", chr[k]+((atr[k] & 0x80)<<1),
			    (((atr[k] & 0x70)>>4) > (atr[k] & 0x07)) ? "" : "|CPAL_REVERSE",
			    (j == 15) ? "\n" : " ");
		WriteFile(hFile, buf, l, &w, NULL);
	    }
	}
}

/*----------------------------------------------------------------------
	Load BMP file
 ----------------------------------------------------------------------*/
LPBITMAPINFO loadBMPfile(char *fn) {
	HANDLE fh;
	BITMAPFILEHEADER header;
	LPBITMAPINFOHEADER lpInfo;
	LPBYTE lpBuf = NULL;
	DWORD size, dummy;

	fh = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE) {
//	    errmsg = "ファイルが開けません";
	    return NULL;
	}
	ReadFile(fh, &header, sizeof(BITMAPFILEHEADER), &dummy, NULL);
	if (header.bfType != 'BM') goto xit;
	size = GetFileSize(fh, NULL) - sizeof(BITMAPFILEHEADER);
	lpBuf = GlobalAlloc(GPTR, size);
	ReadFile(fh, lpBuf, size, &dummy, NULL);

	lpInfo = (LPBITMAPINFOHEADER)(lpBuf);
	if (lpInfo->biSize != sizeof(BITMAPINFOHEADER) ||
	    lpInfo->biPlanes != 1 ||
	    lpInfo->biHeight < 0 ||
	    lpInfo->biCompression != BI_RGB) {
//		errmsg = "サポートしていないBMPファイルです";
		goto xit;
	}
//	if (lpInfo->biBitCount != 8) {
//		errmsg = "256色BMPファイルではありません";
//		goto xit;
//	}
	if (lpInfo->biBitCount == 8 && lpInfo->biClrUsed == 0) {
	    lpInfo->biClrUsed =
		(header.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER)) / 4;
	}
xit:
	CloseHandle(fh);
	return (LPBITMAPINFO)lpBuf;
}

/*----------------------------------------------------------------------
	Recent Used Files
 ----------------------------------------------------------------------*/
int add_recent_file(char *fn) {
	int i, j, k;
	if (fn == NULL || fn[0] == 0) return 0;
	for (i=0; i<9; i++) {
	    if (!_strcmpi(&RecentFiles[i][0], fn)) {
		// すでにリスト中にあるファイルを先頭にする
		for (j=0; j<9; j++) {
		    if (RecentFileIndex[j] == i) {
			for (k=j; k>0; k--) RecentFileIndex[k] = RecentFileIndex[k-1];
			RecentFileIndex[0] = i;
			return 1;
		    }
		}
		return 0;
	    }
	}
	// リストの先頭にファイル名を追加する
	j = RecentFileIndex[8];
	if (j >= 0) RecentFiles[j][0] = 0; // 最後尾のファイル名を削除
	for (k=8; k>0; k--) RecentFileIndex[k] = RecentFileIndex[k-1];
	// 空きエントリを探す
	for (i=0; i<9; i++) {
	    if (RecentFiles[i][0] == 0) {
		strcpy(&RecentFiles[i][0], fn);
		RecentFileIndex[0] = i;
		return 1;
	    }
	}
	return 0;
}

int del_recent_file(char *fn) {
	int i, j, k;
	if (fn == NULL || fn[0] == 0) return 0;
	for (i=0; i<9; i++) {
	    if (!_strcmpi(&RecentFiles[i][0], fn)) {
		// リスト中にあるファイル名を削除する
		RecentFiles[i][0] = 0;
		for (j=0; j<9; j++) {
		    if (RecentFileIndex[j] == i) {
			for (k=j; k<9-1; k++) {
			    RecentFileIndex[k] = RecentFileIndex[k+1];
			}
			RecentFileIndex[8] = -1;
			return 1;
		    }
		}
	    }
	}
	return 0;
}

char *get_recent_file(int i) {
	int j;
	if (i < 0 || i >= 9) return NULL;
	j = RecentFileIndex[i];
	if (j < 0) return NULL;
	return &RecentFiles[j][0];
}


// ============================================================================
// --- Export Files
// ============================================================================

// --- Animations

void writeAnimations(HANDLE file) {
	char buf[256];
	long l, w;

	l = wsprintf(buf, "\n\nSCREEN_ANIMATIONS:\n\n");
	WriteFile(file, buf, l, &w, NULL);
	for (int i = 0; i < 9; i++) {
		int x1 = animsel[i].x1;
		l = wsprintf(buf, "  DB %02d,%02d,%02d,%02d\n", x1, x1 == -1 ? -1 : animsel[i].y1, x1 == -1 ? -1 : animsel[i].x2, x1 == -1 ? -1 : animsel[i].y2 );
		WriteFile(file, buf, l, &w, NULL);
	}
}


// --- Raw

void writeRaw( HANDLE file, char* label, int* vbuf) {
	char buf[512];
	long l, w;

	l = wsprintf(buf, "%s:\n\n  DB ", label);
	WriteFile(file, buf, l, &w, NULL);
	for (int y = 0; y < 25; y++) {
		for (int x = 0; x < 40; x++) {
			int p = (y * 40) + x;
			l = wsprintf(buf, "$%02X%s", vbuf[p], x == 39 ? y == 24 ? "\n" : "\n  DB " : ",");
			WriteFile(file, buf, l, &w, NULL);
		}
	}
}

void exportRawFile(HANDLE file) {
	writeRaw(file, "SCREEN_CHARS", chr);
	writeRaw(file, "\nSCREEN_ATTRIBUTES", atr);
	writeAnimations(file);
}

// --- RLE compressed

void writeCompressed(HANDLE file, char *label, int *vbuf) {
	char buf[512];
	long l, w;
	int cbuf[1000];
	int i = 1, j = 0, n = 1, k;

	// rle compression

	int v = cbuf[j++] = vbuf[0];
	while (i < 1000) {
		int v2 = vbuf[i];
		if (v2 == v) { i++; n++; }
		else { cbuf[j++] = n; cbuf[j++] = v2; v = v2; n = 0; }
	}
	cbuf[j++] = n;

	// breaks repeat > 0xFF into smaller chunks

	int cbuf2[1000];
	for (i = 0, k = 0; i < j; i += 2) {
		v = cbuf[i];
		n = cbuf[i + 1];
		if (n > 0xFF) {
			while (n > 0xFF) {
				cbuf2[k++] = v;
				cbuf2[k++] = 0xFF;
				n -= 0xFF;
			}
		}
		cbuf2[k++] = v;
		cbuf2[k++] = n;
	}

	// save

	l = wsprintf(buf, "%s:\n\n  DB ", label);
	WriteFile(file, buf, l, &w, NULL);
	for (i = 0, j = 1; i < k; i += 2, j++) {
		l = wsprintf(buf, "$%02X,$%02X%s", cbuf2[i], cbuf2[i + 1], j % 21 != 0 ? "," : "\n  DB ");
		WriteFile(file, buf, l, &w, NULL);
	}
	l = wsprintf(buf, "$00,$00\n");
	WriteFile(file, buf, l, &w, NULL);
}


void exportCompressedFile(HANDLE file) {
	writeCompressed(file, "SCREEN_CHARS", chr);
	writeCompressed(file, "\nSCREEN_ATTRIBUTES", atr);
	writeAnimations(file);
}

// --- Binary

void writeBinaryData(HANDLE file, int* vbuf) {
	char buf[1000];
	for (int i = 0; i < 1000; i++) { buf[i] = vbuf[i] & 0xFF; }
	long l;
	WriteFile(file, buf, 1000, &l, NULL );
}


void exportBinaryFile(HANDLE file) {
	writeBinaryData(file, chr);
	writeBinaryData(file, atr);
}

