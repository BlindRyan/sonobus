# Chat screen reader accessibility: project notes

Notes for anyone (including a future Claude session) picking up this work.

## Where things are

- Fork: https://github.com/BlindRyan/sonobus (forked from mendeni/sonobus, branch 1.7.6)
- Work branch: `chat-screen-reader-access`
- Upstream of the fork: SonoBus Mendeni 1.7.6 (an OSC/TouchOSC-enabled fork of SonoBus, JUCE 8)
- Owner/tester: BlindRyan, a JAWS for Windows user

## The problem (as found in SonoBus Mendeni 1.7.6)

1. JUCE's `AccessibilityHandler::postAnnouncement()` on Windows speaks with a
   separate SAPI voice, not through the screen reader. JAWS never received
   new-message announcements.
2. The fork moved keyboard focus to the chat transcript on every incoming
   message (to make screen readers read it), which interrupted typing.
3. The transcript is one big read-only `TextEditor`, which JAWS reads poorly.

## What was changed

- `Source/ScreenReaderWindows.cpp/.h` (Windows only, no JuceHeader):
  - JAWS: if the `JFWUI2` window exists, uses COM ProgID `FreedomSci.JawsApi`
    and calls `SayString(text, flush)` via IDispatch.
  - Otherwise raises a UI Automation notification (`UiaRaiseNotificationEvent`,
    loaded dynamically from UIAutomationCore.dll, Win10 1709+) from JUCE's
    native accessibility element (NVDA, Narrator).
- `Source/ScreenReaderAnnouncer.cpp/.h`: cross-platform wrapper.
  `announce()` for new messages and readouts; `announceSelectedItem()` speaks
  the selected row of the message list through JAWS only (JUCE reports list
  selection on the list, not the row, so JAWS can stay silent otherwise).
  Non-Windows platforms use JUCE's `postAnnouncement`.
- `Source/ChatView.cpp/.h`:
  - `AccessibleChatList`: a transparent `ListBox` laid over the transcript,
    one row per message ("Alice: hello, 5:42pm"). Mouse clicks pass through to
    the visual transcript. The transcript `TextEditor` is now not focusable and
    hidden from accessibility.
  - New messages are announced (not focused). Private messages: "Private from X: ...".
  - Removed the focus-stealing on new messages.
  - `FocusTextEditor::keyPressed`: Alt shortcuts (not Ctrl+Alt/AltGr) pass
    through the chat input on Windows/Linux so Alt+1..9 work while typing.
  - Chat menu item to turn announcements on/off.
- `Source/SonobusPluginProcessor.*`: persisted `chatAnnounceMessages` setting (default on).
- `Source/SonobusTypes.h`, `Source/SonobusPluginEditor.cpp`: new commands
  `ToggleChatAnnouncements` (Ctrl+Shift+A) and `ReadChatMessage1..9` (Alt+1..9).
- `doc/SonoBus User Guide.md`: shortcuts documented.

## Keyboard summary

| Keys | Action |
|---|---|
| Ctrl+Y | Focus chat message list (lands on newest) |
| Up/Down, Home/End | Read messages in the list |
| Enter | Open the link in the selected message |
| Ctrl+C | Copy the selected message |
| Escape | Back to the message input |
| Ctrl+Alt+Y | Focus chat input (existing) |
| Alt+1 ... Alt+9 | Speak the newest ... 9th newest message |
| Ctrl+Shift+A | Toggle automatic announcements of new messages |

## Building

Pushing to any branch of the fork runs `.github/workflows/build-desktop.yml`
(Windows, macOS, Linux). Windows output: artifact `SonoBusMendeni-Windows`
(Standalone exe, VST3, TouchOSC layout). Actions had to be enabled manually
on the fork. Local Windows builds need Visual Studio 2022 (`setupcmakewin.sh`).

## Status / things to verify with JAWS

- First Windows build: run 1 on 2026-11-23, succeeded. Not yet tested with JAWS
  at the time these notes were written.
- Watch for: messages read twice when arrowing the list (JAWS SayString plus
  JAWS reading the UIA selection). If so, drop the `announceSelectedItem` call
  in `AccessibleChatList::selectedRowsChanged`.
- Watch for: announcements silent in JAWS. Check that the JAWS COM API is
  registered (`FreedomSci.JawsApi`); the UIA fallback should still work.
