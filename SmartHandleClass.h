#pragma once
#include "framework.h"

// TYPE& operator=(TYPE&& other) noexcept;
#define SMARTHANDLECLASS_DEF(TYPE, HANDLE_TYPE) \
    class TYPE { \
        private: \
        HANDLE_TYPE h; \
        public: \
        TYPE (); \
        TYPE(const TYPE& other) = delete; \
        TYPE& operator=(TYPE& other) = delete; \
        TYPE& operator=(TYPE&& other); \
        TYPE(HANDLE_TYPE h); \
        HANDLE_TYPE* GetPointer(); \
        HANDLE_TYPE GetHandle(); \
        bool IsNull(); \
        ~TYPE(); \
        void Dealloc(); \
        void Move(TYPE other); \
        void Replace(HANDLE_TYPE new_h); \
    };


// TODO: why is TYPE& TYPE::operator=(TYPE&& other) noexcept 
//       a good idea? versus with an exception possibility?
#define SMARTHANDLECLASS_IMPL(TYPE, HANDLE_TYPE, DEALLOC_CALL, DEFVAL, ISNULL) \
    TYPE::TYPE() { \
        h = DEFVAL; \
    } \
    TYPE& TYPE::operator=(TYPE&& other) { \
        if (!ISNULL) { \
            DEALLOC_CALL; \
        } \
        h = other.h; \
        other.h = DEFVAL; \
        return *this; \
    } \
    TYPE::TYPE(HANDLE_TYPE h) { \
        this->h = h; \
    } \
    HANDLE_TYPE* TYPE::GetPointer() { \
        if (!ISNULL) { \
            throw SuspectBuggyUsage(); \
        } \
        return &h; \
    } \
    void TYPE::Replace(HANDLE_TYPE new_h) { \
        Dealloc(); \
        h = new_h; \
    } \
    HANDLE_TYPE TYPE::GetHandle() { \
        return h; \
    } \
    bool TYPE::IsNull() { \
        return ISNULL; \
    } \
    void TYPE::Dealloc() { \
        if (!ISNULL) { \
            DEALLOC_CALL; \
            this->h = DEFVAL; \
        } \
    } \
    void TYPE::Move(TYPE other) { \
        Dealloc(); \
        h = other.h; \
        other.h = DEFVAL; \
    } \
    TYPE::~TYPE() { \
        Dealloc(); \
    }

/*

    The old style assumed handle was an integer type of default value NULL.
 
SMARTHANDLECLASS_DEF(BCryptKey, BCRYPT_KEY_HANDLE, BCryptDestroyKey(h))
SMARTHANDLECLASS_DEF(BCryptAlgProv, BCRYPT_ALG_HANDLE, BCryptCloseAlgorithmProvider(h, 0))
SMARTHANDLECLASS_DEF(CryptKey, HCRYPTKEY, CryptDestroyKey(h))
SMARTHANDLECLASS_DEF(CryptContext, HCRYPTPROV, CryptReleaseContext(h, 0))
*/

class SmartHDCInner {
    public:
        HDC hdc;
        HWND hwnd;
        SmartHDCInner();
        SmartHDCInner(HWND hwnd, HDC hdc);
};

SMARTHANDLECLASS_DEF(SmartPrivateHDC, SmartHDCInner);
SMARTHANDLECLASS_DEF(SmartHDC, SmartHDCInner);
SMARTHANDLECLASS_DEF(SmartBitmap, HBITMAP);
SMARTHANDLECLASS_DEF(SmartGlobalAlloc, HANDLE);
SMARTHANDLECLASS_DEF(SmartGdiObj, HGDIOBJ);