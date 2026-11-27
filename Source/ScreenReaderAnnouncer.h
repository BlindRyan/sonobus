// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>

namespace ScreenReaderAnnouncer
{
    // Speak text through the active screen reader without moving keyboard focus.
    // On Windows this goes to JAWS directly (or a UI Automation notification for
    // NVDA / Narrator); on other platforms it uses JUCE's announcement support.
    // source is any visible component in the window, used for the UIA event.
    void announce (juce::Component* source, const juce::String& text, bool interrupt = false);

    // Speak the newly selected item of a focused custom list. JUCE reports list
    // selection on the list itself, which JAWS doesn't read, so on Windows this
    // speaks directly through JAWS when it is running. Other screen readers get
    // the item from the normal accessibility tree, so nothing extra is spoken.
    void announceSelectedItem (const juce::String& text, bool interrupt = true);
}
