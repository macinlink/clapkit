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
#include "ckWindow.h"

CKLabel::CKLabel(const CKControlInitParams& params): CKControl(params, CKControlType::Label) {

    this->__text = 0;
    this->__fontNumber = 0;

    this->bold = false;
    this->italic = false;
    this->underline = false;
    this->color = { 0, 0, 0 };
    this->fontSize = 12;
    this->justification = CKTextJustification::Left;

    this->__teHandle = nullptr;

    this->SetText(params.title);

}

CKLabel::~CKLabel() {

    CK_SAFE_COPY_STRING(this->__text, 0);

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
    SetPort(window->__windowPtr);

    Rect* r = this->GetRect()->ToOSPtr();
    this->__teHandle = TEStyleNew(r, r);
    (*this->__teHandle)->txMode = srcCopy;
    CKFree(r);

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

    HLock((Handle)this->__teHandle);
    TEPtr trecord = *(this->__teHandle);

    trecord->txSize = this->fontSize;

    trecord->lineHeight = this->fontSize + 3;
    trecord->fontAscent = this->fontSize;

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

    trecord->txFont = this->__fontNumber;
    trecord->txFace = 0;

    if (this->bold) {
        trecord->txFace = trecord->txFace | QD_BOLD;
    } else {
        trecord->txFace = trecord->txFace & ~QD_BOLD;
    }

    if (this->italic) {
        trecord->txFace = trecord->txFace | QD_ITALIC;
    } else {
        trecord->txFace = trecord->txFace & ~QD_ITALIC;
    }

    if (this->underline) {
        trecord->txFace = trecord->txFace | QD_UNDERLINE;
    } else {
        trecord->txFace = trecord->txFace & ~QD_UNDERLINE;
    }

    RGBColor color = this->color.ToOS();
    RGBForeColor(&color);
    
    HUnlock((Handle)this->__teHandle);

}

void CKLabel::Redraw() {

    CKPROFILE

    if (!this->__text || !this->__teHandle) {
        return;
    }

    this->PrepareForDraw();

    HLock((Handle)this->__teHandle);
    TEPtr trecord = *(this->__teHandle);

    GrafPtr oldPort;
    GetPort(&oldPort);
    SetPort(this->owner->__windowPtr);

    TECalText(this->__teHandle);
    TEUpdate(&(trecord->viewRect), this->__teHandle);

    ForeColor(blackColor);

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