// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "ScreenReaderAnnouncer.h"

#if JUCE_WINDOWS
#include "ScreenReaderWindows.h"
#endif

namespace ScreenReaderAnnouncer
{

void announce (juce::Component* source, const juce::String& text, bool interrupt)
{
    if (text.trim().isEmpty())
        return;

#if JUCE_WINDOWS
    void* provider = nullptr;

    // walk up from the source to find a component with a live accessibility element
    for (auto* comp = source; comp != nullptr && provider == nullptr; comp = comp->getParentComponent()) {
        if (auto* handler = comp->getAccessibilityHandler())
            provider = handler->getNativeImplementation();
    }

    if (sonoWindowsScreenReaderSpeak (text.toWideCharPointer(), provider, interrupt))
        return;

    // Nothing to talk to: don't fall back to JUCE's SAPI voice, which would talk over
    // (or instead of) the user's screen reader.
    juce::ignoreUnused (interrupt);
#else
    juce::AccessibilityHandler::postAnnouncement (text,
                                                  interrupt ? juce::AccessibilityHandler::AnnouncementPriority::high
                                                            : juce::AccessibilityHandler::AnnouncementPriority::medium);
    juce::ignoreUnused (source);
#endif
}

void announceSelectedItem (const juce::String& text, bool interrupt)
{
#if JUCE_WINDOWS
    if (text.isNotEmpty())
        sonoWindowsJawsSay (text.toWideCharPointer(), interrupt);
#else
    juce::ignoreUnused (text, interrupt);
#endif
}

}
