/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKLabel
 * ----------------------------------------------------------------------
 * Defines a static label.
 *
 */

#include "ckCanvas.h"
#include "ckWindow.h"

CKCanvas::CKCanvas(int width, int height, const CKControlInitParams& params)
	: CKControl(params, CKControlType::Canvas) {

	CKPROFILE

	this->__width = width;
	this->__height = height;

	Rect bounds = {0, 0, (short)height, (short)width};
	OSErr err = NewGWorld(&(this->__gworldptr), 16, &bounds, NULL, NULL, 0);
	if (err != noErr) {
		throw CKNew CKException("Can't create GWorld for canvas.");
	}

	this->Clear();
}

CKCanvas::~CKCanvas() {

	CKPROFILE

	CKLog("~CKCanvas called.");

	if (this->__gworldptr) {
		DisposeGWorld(this->__gworldptr);
		this->__gworldptr = NULL;
	}
}

void CKCanvas::Clear() {
	this->Fill(255, 255, 255);
}

void CKCanvas::FillRect(CKRect rect, u_int8_t r, u_int8_t g, u_int8_t b) {

	CKPROFILE

	RGBColor rgb = {
		(unsigned short)(r << 8),
		(unsigned short)(g << 8),
		(unsigned short)(b << 8)};
	PixPatHandle pixPat = NewPixPat();
	if (pixPat != NULL) {
		MakeRGBPat(pixPat, &rgb);
	} else {
		CKLog("NewPixPat failed!");
		return;
	}

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		return;
	}

	CGrafPtr oldPort;
	GDHandle oldGD;
	GetGWorld(&oldPort, &oldGD);		// Save the old port
	SetGWorld(this->__gworldptr, NULL); // Set the GWorld as the current port

	Rect osRect = rect.ToOS();
	FillCRect(&osRect, pixPat);

	DisposePixPat(pixPat);

	SetGWorld(oldPort, oldGD); // Restore the old port

	UnlockPixels(offscreenPixMap);
}

void CKCanvas::Fill(u_int8_t r, u_int8_t g, u_int8_t b) {

	CKPROFILE

	CKRect rect = {0, 0, this->__width, this->__height};
	this->FillRect(rect, r, g, b);

	// PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	// if (!LockPixels(offscreenPixMap)) {
	//     CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
	//     return;
	// }

	// u_int8_t*  gWorldData = (u_int8_t*)GetPixBaseAddr(offscreenPixMap);
	// memset(gWorldData, 255, (this->__width * this->__height * 2));
	// UnlockPixels(offscreenPixMap);

	// ---=======-=-=-=-=-=-=-=

	// PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	// if (!LockPixels(offscreenPixMap)) {
	//     CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
	//     return;
	// }

	// int trueRowBytes = (*offscreenPixMap)->rowBytes & 0x3FFF;
	// CKLog("rowBytes is %d. pixel size is %d", trueRowBytes, (*offscreenPixMap)->pixelSize);

	// u_int8_t*  gWorldData = (u_int8_t*)GetPixBaseAddr(offscreenPixMap);
	// for(int y = 0; y < this->__height; ++y) {
	//     for(int x = 0; x < this->__width; ++x) {
	//         int index = y * trueRowBytes + x * 4;
	//         gWorldData[index + 0] = r; // Blue
	//         gWorldData[index + 1] = g; // Green
	//         gWorldData[index + 2] = b; // Red
	//         gWorldData[index + 3] = 255; // Alpha
	//     }
	// }
	// UnlockPixels(offscreenPixMap);
}

void CKCanvas::SetPixel(int x, int y, u_int8_t r, u_int8_t g, u_int8_t b) {

	CKPROFILE

	if (x < 0 || x > this->__width) {
		return;
	}
	if (y < 0 || y > this->__height) {
		return;
	}

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		return;
	}

	int trueRowBytes = (*offscreenPixMap)->rowBytes & 0x3FFF;

	u_int16_t* gWorldData = (u_int16_t*)GetPixBaseAddr(offscreenPixMap);
	int index = y * (trueRowBytes / 2) + x;							 // trueRowBytes / 2 gives us the number of pixels per row
	u_int16_t color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3); // convert 8-bit RGB to 5-6-5 RGB

	gWorldData[index] = color;

	UnlockPixels(offscreenPixMap);

	// if (x < 0 || x > this->__width) { return; }
	// if (y < 0 || y > this->__height) { return; }

	// PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	// if (!LockPixels(offscreenPixMap)) {
	//     CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
	//     return;
	// }

	// u_int8_t* gWorldData = (u_int8_t*)GetPixBaseAddr(offscreenPixMap);

	// int trueRowBytes = (*offscreenPixMap)->rowBytes & 0x3FFF;
	// int index = y * trueRowBytes + x * 4;

	// CKLog("Will set %dx%d of canvas to %d - %d - %d ... index is %d", x, y, r, g, b, index);

	// gWorldData[index + 0] = 255; // Blue
	// gWorldData[index + 1] = r; // Green
	// gWorldData[index + 2] = g; // Red
	// gWorldData[index + 3] = b; // Alpha

	// UnlockPixels(offscreenPixMap);
}

void CKCanvas::DrawLine(int x1, int y1, int x2, int y2, u_int8_t r, u_int8_t g, u_int8_t b) {
	RGBColor rgb = {
		(unsigned short)(r << 8),
		(unsigned short)(g << 8),
		(unsigned short)(b << 8)};
	CGrafPtr oldPort;
	GDHandle oldGD;
	GetGWorld(&oldPort, &oldGD);		// Save the old port
	SetGWorld(this->__gworldptr, NULL); // Set the GWorld as the current port

	RGBForeColor(&rgb); // Set the color to draw with.
	MoveTo(x1, y1);		// Move to the start of the line.
	LineTo(x2, y2);		// Draw a line to the end point.

	SetGWorld(oldPort, oldGD); // Restore the old port
}

void CKCanvas::Redraw() {

	CKPROFILE

	if (this->owner == nil) {
		return;
	}

	long t_start = TickCount();

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->owner->GetWindowPtr());

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		SetPort(oldPort);
		return;
	}

	Rect destRect = {0, 0, (short)this->__height, (short)this->__width};

	CopyBits((BitMap*)&(**offscreenPixMap),
			 &(this->owner->GetWindowPtr()->portBits),
			 &destRect, &destRect, srcCopy, NULL);

	UnlockPixels(offscreenPixMap);

	CKLog("Redraw took %lu ticks.", (TickCount() - t_start));
	SetPort(oldPort);
}