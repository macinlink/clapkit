/*
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

/**
 * @ingroup UIControls
 * @brief Defines a drawable canvas.
 */
class CKCanvas : public CKControl {

	public:
		CKCanvas(const CKControlInitParams& params);
		virtual ~CKCanvas();
		virtual void Redraw();
		const GWorldPtr GetOSPointer();

		void Clear();
		void FillRect(CKRect rect, CKColor c);
		void Fill(CKColor c);
		void SetPixel(CKPoint p, CKColor c);
		void DrawLine(CKPoint start, CKPoint end, CKColor c);
		bool DrawResourceIcon(short resourceId, CKPoint where);

	private:
		GWorldPtr __gworldptr = NULL;
		bool __hasQueuedIcon = false;
		short __queuedIconResourceId = 0;
		CKPoint __queuedIconWhere;
};
