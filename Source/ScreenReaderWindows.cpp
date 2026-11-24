// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception
//
// Windows-only screen reader speech for chat announcements.
//
// JUCE's AccessibilityHandler::postAnnouncement() on Windows speaks through a
// separate SAPI voice instead of the user's screen reader, so JAWS users never
// hear it. This file talks to the screen reader directly:
//
//  1. JAWS: if JAWS is running, use its COM automation object
//     (ProgID "FreedomSci.JawsApi") and call SayString(text, flush).
//  2. Everyone else (NVDA, Narrator, newer JAWS as a fallback): raise a UI
//     Automation notification event from the app's UIA provider.
//
// Deliberately does not include JuceHeader.h, so windows.h can be used freely.

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <oleauto.h>
#include <uiautomation.h>

#include "ScreenReaderWindows.h"

namespace
{
    // ---- JAWS -----------------------------------------------------------

    bool isJawsRunning()
    {
        return FindWindowW (L"JFWUI2", nullptr) != nullptr;
    }

    IDispatch* getJawsApi()
    {
        static IDispatch* jaws = nullptr;
        static bool triedAndFailed = false;

        if (jaws != nullptr || triedAndFailed)
            return jaws;

        CLSID clsid;
        if (FAILED (CLSIDFromProgID (L"FreedomSci.JawsApi", &clsid))) {
            triedAndFailed = true;
            return nullptr;
        }

        IDispatch* disp = nullptr;
        if (FAILED (CoCreateInstance (clsid, nullptr, CLSCTX_ALL, IID_IDispatch, (void**) &disp)) || disp == nullptr) {
            // JAWS may simply not be fully started yet, allow a retry later
            return nullptr;
        }

        jaws = disp;
        return jaws;
    }

    bool jawsSayString (const wchar_t* text, bool interrupt)
    {
        if (! isJawsRunning())
            return false;

        IDispatch* jaws = getJawsApi();
        if (jaws == nullptr)
            return false;

        static DISPID sayStringId = DISPID_UNKNOWN;
        if (sayStringId == DISPID_UNKNOWN) {
            LPOLESTR name = const_cast<LPOLESTR> (L"SayString");
            if (FAILED (jaws->GetIDsOfNames (IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &sayStringId)))
                return false;
        }

        BSTR bstr = SysAllocString (text);
        if (bstr == nullptr)
            return false;

        // IDispatch arguments are passed in reverse order:
        // SayString(StringToSpeak, bFlush)
        VARIANTARG args[2];
        VariantInit (&args[0]);
        VariantInit (&args[1]);
        args[0].vt = VT_BOOL;
        args[0].boolVal = interrupt ? VARIANT_TRUE : VARIANT_FALSE;
        args[1].vt = VT_BSTR;
        args[1].bstrVal = bstr;

        DISPPARAMS params = { args, nullptr, 2, 0 };
        VARIANT result;
        VariantInit (&result);

        HRESULT hr = jaws->Invoke (sayStringId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                                   &params, &result, nullptr, nullptr);

        VariantClear (&result);
        SysFreeString (bstr);

        if (FAILED (hr)) {
            // JAWS was probably restarted, drop the stale object so we reconnect next time
            sayStringId = DISPID_UNKNOWN;
            // (intentionally leaking the old pointer rather than risking a call into a dead server)
            return false;
        }

        return true;
    }

    // ---- UI Automation notification --------------------------------------

    typedef HRESULT (WINAPI *UiaRaiseNotificationEventFunc) (IRawElementProviderSimple*,
                                                              NotificationKind,
                                                              NotificationProcessing,
                                                              BSTR, BSTR);
    typedef BOOL (WINAPI *UiaClientsAreListeningFunc)();

    bool uiaNotify (const wchar_t* text, void* nativeProvider, bool interrupt)
    {
        if (nativeProvider == nullptr)
            return false;

        static HMODULE uiaCore = LoadLibraryW (L"UIAutomationCore.dll");
        if (uiaCore == nullptr)
            return false;

        // Only exists on Windows 10 1709 and later
        static auto raiseNotification = (UiaRaiseNotificationEventFunc) GetProcAddress (uiaCore, "UiaRaiseNotificationEvent");
        static auto clientsListening = (UiaClientsAreListeningFunc) GetProcAddress (uiaCore, "UiaClientsAreListening");

        if (raiseNotification == nullptr)
            return false;

        if (clientsListening != nullptr && ! clientsListening())
            return false;

        // JUCE's AccessibilityNativeHandle has IRawElementProviderSimple as its first COM base,
        // so ask it for the interface properly rather than trusting a cast.
        IUnknown* unknown = static_cast<IUnknown*> (nativeProvider);
        IRawElementProviderSimple* provider = nullptr;
        if (FAILED (unknown->QueryInterface (__uuidof (IRawElementProviderSimple), (void**) &provider)) || provider == nullptr)
            return false;

        BSTR display = SysAllocString (text);
        BSTR activity = SysAllocString (L"SonoBusChatMessage");

        HRESULT hr = raiseNotification (provider,
                                        NotificationKind_Other,
                                        interrupt ? NotificationProcessing_MostRecent : NotificationProcessing_All,
                                        display, activity);

        SysFreeString (display);
        SysFreeString (activity);
        provider->Release();

        return SUCCEEDED (hr);
    }
}

bool sonoWindowsScreenReaderSpeak (const wchar_t* text, void* nativeProvider, bool interrupt)
{
    if (text == nullptr || *text == 0)
        return false;

    if (jawsSayString (text, interrupt))
        return true;

    return uiaNotify (text, nativeProvider, interrupt);
}

bool sonoWindowsIsJawsRunning()
{
    return isJawsRunning();
}

bool sonoWindowsJawsSay (const wchar_t* text, bool interrupt)
{
    if (text == nullptr || *text == 0)
        return false;
    return jawsSayString (text, interrupt);
}

#endif // _WIN32
