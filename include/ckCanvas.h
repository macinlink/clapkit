/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKCanvas
 * ----------------------------------------------------------------------
 * Defines a drawable area.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckControl.h"

class CKCanvas : public CKControl {

	public:
		CKCanvas(int width, int height, const CKControlInitParams& params);
		virtual ~CKCanvas();

		virtual void Redraw();
		void Clear();
		void FillRect(CKRect rect, u_int8_t r, u_int8_t g, u_int8_t b);
		void Fill(u_int8_t r, u_int8_t g, u_int8_t b);
		void SetPixel(int x, int y, u_int8_t r, u_int8_t g, u_int8_t b);
		void DrawLine(int x1, int y1, int x2, int y2, u_int8_t r, u_int8_t g, u_int8_t b);

	private:
		int __width;
		int __height;
		GWorldPtr __gworldptr;
};