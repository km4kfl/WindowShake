#include "SmartHandleClass.h"
#include "AppExceptions.h"

/*
SMARTHANDLECLASS_IMPL(BCryptKey, BCRYPT_KEY_HANDLE, BCryptDestroyKey(h));
SMARTHANDLECLASS_IMPL(BCryptAlgProv, BCRYPT_ALG_HANDLE, BCryptCloseAlgorithmProvider(h, 0));
SMARTHANDLECLASS_IMPL(CryptKey, HCRYPTKEY, CryptDestroyKey(h));
SMARTHANDLECLASS_IMPL(CryptContext, HCRYPTPROV, CryptReleaseContext(h, 0));
*/

SmartHDCInner::SmartHDCInner() : hdc(NULL), hwnd(NULL) {
}

SmartHDCInner::SmartHDCInner(HWND hwnd, HDC hdc) : hwnd(hwnd), hdc(hdc) {
}

SMARTHANDLECLASS_IMPL(SmartPrivateHDC, SmartHDCInner, BOOL_THROW(DeleteObject(h.hdc) != 0), SmartHDCInner(), (h.hdc == NULL));
SMARTHANDLECLASS_IMPL(SmartHDC, SmartHDCInner, BOOL_THROW(ReleaseDC(h.hwnd, h.hdc) != 0), SmartHDCInner(), (h.hdc == NULL));
SMARTHANDLECLASS_IMPL(SmartBitmap, HBITMAP, BOOL_THROW(DeleteObject(h)), NULL, (h == NULL));

static void _smartglobalalloc_deconstructor(HANDLE h) {
	BOOL_THROW(GlobalUnlock(h));
	BOOL_THROW(GlobalFree(h));
}

SMARTHANDLECLASS_IMPL(SmartGlobalAlloc, HANDLE, _smartglobalalloc_deconstructor(h), NULL, (h == NULL));
SMARTHANDLECLASS_IMPL(SmartGdiObj, HGDIOBJ, BOOL_THROW(DeleteObject(h)), NULL, (h == NULL));