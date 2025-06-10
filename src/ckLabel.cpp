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

#include "ckLabel.h"
#include "ckTextArea.h"
#include "ckWindow.h"
#include <Appearance.h>
#include <TextEdit.h>

CKLabel::CKLabel(const CKControlInitParams& params)
	: CKControl(params, CKControlType::Label) {

	this->__text = 0;
	this->__fontNumber = 0;

	this->bold = false;
	this->bold.onChange = CKOBSERVEVALUE("bold");

	this->italic = false;
	this->italic.onChange = CKOBSERVEVALUE("italic");

	this->underline = false;
	this->underline.onChange = CKOBSERVEVALUE("underline");

	this->color = {0, 0, 0};
	this->color.onChange = CKOBSERVEVALUE("color");

	this->fontSize = 12;
	this->fontSize.onChange = CKOBSERVEVALUE("fontSize");

	this->justification = CKTextJustification::Left;
	this->justification.onChange = CKOBSERVEVALUE("justification");

	this->__teHandle = nullptr;

	this->SetText(params.title);
}

CKLabel::~CKLabel() {

	if (this->__teHandle != nullptr) {
		TEDispose(this->__teHandle);
		this->__teHandle = nullptr;
	}
}

void CKLabel::AddedToWindow(CKWindow* window) {

	// Delete the old one if there is one?
	// Should not be the case but just in case.
	if (this->__teHandle != nullptr) {
		TEDispose(this->__teHandle);
		this->__teHandle = nullptr;
	}

	// TODO: I forget if grafport changing is necessary, so just in case.
	// If this is not the case, remove.

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(window->GetWindowPtr());

	Rect r = this->rect->ToOS();
	this->__teHandle = TEStyleNew(&r, &r);
	(*this->__teHandle)->txMode = srcCopy;
	this->ResizeTE();

	SetPort(oldPort);

	if (this->__text) {
		TESetText(this->__text, strlen(this->__text), this->__teHandle);
	}

	TECreated();
}

void CKLabel::RemovedFromWindow() {

	if (this->__teHandle != nullptr) {
		TEDispose(this->__teHandle);
		this->__teHandle = nullptr;
	}
}

void CKLabel::PrepareForDraw() {

	// TODO: We probably shouldn't be doing this every time we draw it..
	// But this is probably the quickest way of doing this for now.

	// TODO: We are not handling `multiline` for now.
	// If `multiline` is false, we need a single line, vertically-centered.

	if (!this->__teHandle) {
		return;
	}

	if (!dynamic_cast<CKTextArea*>(this)) {
		this->ResizeTE();
	}

	HLock((Handle)this->__teHandle);

	short oldSelStart = (*this->__teHandle)->selStart;
	short oldSelEnd = (*this->__teHandle)->selEnd;

	if (this->__text) {
		TESetText(this->__text, strlen(this->__text), this->__teHandle);

		// Select all text
		(*this->__teHandle)->selStart = 0;
		(*this->__teHandle)->selEnd = strlen(this->__text);
		TEPtr trecord = *(this->__teHandle);

		switch (this->justification) {
			case CKTextJustification::Center:
				trecord->just = 1;
				break;
			case CKTextJustification::Right:
				trecord->just = -1;
				break;
			default:
				trecord->just = 0;
				break;
		}

		TextStyle style;

		style.tsFont = this->__fontNumber;
		style.tsSize = this->fontSize;
		style.tsFace = 0;

		if (this->bold) {
			style.tsFace = style.tsFace | QD_BOLD;
		} else {
			style.tsFace = style.tsFace & ~QD_BOLD;
		}

		if (this->italic) {
			style.tsFace = style.tsFace | QD_ITALIC;
		} else {
			style.tsFace = style.tsFace & ~QD_ITALIC;
		}

		if (this->underline) {
			style.tsFace = style.tsFace | QD_UNDERLINE;
		} else {
			style.tsFace = style.tsFace & ~QD_UNDERLINE;
		}

		CKColor black = {0, 0, 0};
		if (this->color.get() != black || !CKHasAppearanceManager()) {
			style.tsColor = this->color.get().ToOS();
		} else {
			if (this->color.get() == black && CKHasAppearanceManager()) {
				RGBColor color;
				GDHandle deviceHdl = LMGetMainDevice();
				SInt16 gPixelDepth = (*(*deviceHdl)->gdPMap)->pixelSize;
				ThemeBrush brush = kThemeInactiveDialogTextColor;
				if (this->owner && this->owner->GetIsActive()) {
					brush = kThemeActiveDialogTextColor;
				}
				SetThemeTextColor(brush, gPixelDepth, true); // TODO: Maybe not hardcode 'isColorDevice'?
				style.tsColor = color;
			}
		}

		TESetStyle(doAll, &style, false, this->__teHandle);
	} else {
		TESetText("", 0, this->__teHandle);
	}

	(*this->__teHandle)->selStart = oldSelStart;
	(*this->__teHandle)->selEnd = oldSelEnd;

	HUnlock((Handle)this->__teHandle);
	TECalText(this->__teHandle);
}

void CKLabel::Redraw() {

	CKPROFILE

	if (!this->__text || !this->__teHandle) {
		return;
	}

	if (this->__needsPreparing) {
		this->PrepareForDraw();
		this->__needsPreparing = false;
	}

	HLock((Handle)this->__teHandle);
	TEPtr trecord = *(this->__teHandle);

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->owner->GetWindowPtr());

	// Clip area to our rect.
	// TODO: Does TEUpdate even respect this?
	RgnHandle clipHandle = NewRgn();
	GetClip(clipHandle);
	Rect cr = this->rect->ToOS();
	ClipRect(&cr);

	TEUpdate(&(trecord->viewRect), this->__teHandle);

	ForeColor(blackColor);

	SetClip(clipHandle);
	DisposeRgn(clipHandle);

	SetPort(oldPort);
	HUnlock((Handle)this->__teHandle);
}

void CKLabel::SetText(const char* text) {

	CKTextableControl::SetText(text);

	if (this->__teHandle) {
		if (this->__text) {
			TESetText(this->__text, strlen(this->__text), this->__teHandle);
		} else {
			TESetText("", 0, this->__teHandle);
		}
	}

	this->MarkAsDirty();
}
/**
 * @brief Increase the height until it fits.
 * @param maxHeight If set to non-zero value, will limit maximum height.
 */
void CKLabel::AutoHeight(int maxHeight) {
	if (!this->__text)
		return;

	// Create a temporary TEHandle with a dummy GrafPort
	GrafPtr oldPort;
	GetPort(&oldPort);

	CGrafPort tempPort;
	OpenPort((GrafPtr)&tempPort);
	SetPort((GrafPtr)&tempPort);

	// Define a text rectangle for wrapping
	Rect textRect;
	textRect.left = 0;
	textRect.top = 0;
	textRect.right = std::max(1, this->rect->size->width.get());
	textRect.bottom = std::min(maxHeight, 500);

	TEHandle tempTE = TEStyleNew(&textRect, &textRect);
	if (!tempTE) {
		ClosePort((GrafPtr)&tempPort);
		SetPort(oldPort);
		return;
	}

	// Set text (ensure non-empty)
	const char* safeText = this->__text ? this->__text : " ";
	TESetText(safeText, strlen(safeText), tempTE);

	// Apply font settings BEFORE calling TECalText()
	(*tempTE)->txFont = this->__fontNumber;
	(*tempTE)->txSize = this->fontSize;
	(*tempTE)->txFace = (this->bold ? QD_BOLD : 0) |
						(this->italic ? QD_ITALIC : 0) |
						(this->underline ? QD_UNDERLINE : 0);
	(*tempTE)->lineHeight = this->fontSize + 3;
	(*tempTE)->fontAscent = this->fontSize;

	TECalText(tempTE); // Recalculate text layout

	// Get total height using TEGetHeight
	int totalHeight = TEGetHeight((*tempTE)->nLines, 0, tempTE);

	// Limit max height if needed
	if (maxHeight > 0 && totalHeight > maxHeight) {
		totalHeight = maxHeight;
	}

	// Dispose of temp TEHandle and restore port
	TEDispose(tempTE);
	ClosePort((GrafPtr)&tempPort);
	SetPort(oldPort);

	// Update label's rect
	this->rect->size->height = totalHeight;
}

void CKLabel::SetFont(short fontId) {
	this->__fontNumber = fontId;
}

short CKLabel::GetFont() {
	return this->__fontNumber;
}

void CKLabel::TECreated() {
	// Nothing here, override me.
}

void CKLabel::ResizeTE() {

	if (!this->__teHandle) {
		return;
	}

	HLock((Handle)this->__teHandle);

	Rect cr = this->rect->ToOS();

	(*this->__teHandle)->destRect = cr;
	(*this->__teHandle)->viewRect = cr;

	TECalText(this->__teHandle);

	HUnlock((Handle)this->__teHandle);
}

void CKLabel::RaisePropertyChange(const char* propertyName) {
	this->__needsPreparing = true;
	CKControl::RaisePropertyChange(propertyName);
}