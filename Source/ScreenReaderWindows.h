// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#ifdef _WIN32
// Speak text through the running screen reader (JAWS via its COM API, otherwise
// a UI Automation notification raised from nativeProvider, which should be a
// JUCE AccessibilityNativeHandle*). Returns true if something handled it.
bool sonoWindowsScreenReaderSpeak (const wchar_t* text, void* nativeProvider, bool interrupt);
bool sonoWindowsIsJawsRunning();
// Speak through JAWS only. Returns false if JAWS isn't running.
bool sonoWindowsJawsSay (const wchar_t* text, bool interrupt);
#endif
