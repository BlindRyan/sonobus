// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception
// Copyright (C) 2021 Jesse Chappell


#pragma once

#include <JuceHeader.h>

#include "SonobusPluginProcessor.h"
#include "SonoDrawableButton.h"
#include <map>

class FocusTextEditor : public TextEditor
{
public:
    
    FocusTextEditor(const String & name = {}) : TextEditor(name) {}

    void focusGained (FocusChangeType) override;
    bool keyPressed (const KeyPress & key) override;

    std::function<void(FocusChangeType)> onFocusGained;
};

class ChatView;

// One chat message as a screen reader should read it.
struct AccessibleChatEntry
{
    String text;   // full spoken text, e.g. "Alice: hello there, 5:42 PM"
    String url;    // first URL in the message, if any (Enter opens it)
};

// A keyboard-navigable list of chat messages, one row per message, laid
// invisibly over the visual chat transcript. Screen readers read each row as
// it is arrowed to; sighted users keep seeing (and clicking) the transcript.
class AccessibleChatList : public ListBox, public ListBoxModel
{
public:
    AccessibleChatList(ChatView & owner, std::vector<AccessibleChatEntry> & entries);

    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override {}
    String getNameForRow (int rowNumber) override;
    void returnKeyPressed (int lastRowSelected) override;
    void listBoxItemDoubleClicked (int row, const MouseEvent&) override { returnKeyPressed(row); }
    void selectedRowsChanged (int lastRowSelected) override;

    bool keyPressed (const KeyPress & key) override;
    void paintOverChildren (Graphics& g) override;
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override { repaint(); }
    void focusOfChildComponentChanged (FocusChangeType) override { repaint(); }

    void entriesChanged(bool wasAtEnd);

private:
    ChatView & owner;
    std::vector<AccessibleChatEntry> & entries;
};

class ChatView : public Component
{
public:
    ChatView(SonobusAudioProcessor& proc, AooServerConnectionInfo & connectinfo);
    ~ChatView();

    void paint (Graphics&) override;
    void resized() override;

    void addNewChatMessage(const SBChatEvent & mesg, bool refresh=true);
    void addNewChatMessages(const Array<SBChatEvent> & mesgs, bool refresh=false);
    void refreshMessages(); // only new ones not yet rendered
    void refreshAllMessages(); // re-render all messages

    void clearAll();

    bool haveNewSinceLastView() const;
    void setFocusToChat();
    void setFocusToMessageDisplay();

    void setUseFixedWidthFont(bool flag);

    bool keyPressed (const KeyPress & key) override;

    void mouseDown (const MouseEvent& event)  override;
    void mouseUp (const MouseEvent& event)  override;
    void mouseDrag (const MouseEvent& event)  override;
    void mouseMove (const MouseEvent& event)  override;

    void visibilityChanged() override;

    void chatTabChanged (int tabindex);
    void chatTabRightClicked (int tabindex);

    // screen reader support
    // speak the Nth most recent message in the current chat tab (1 = newest)
    void readRecentMessage(int nth);
    void setAnnounceMessages(bool flag);
    bool getAnnounceMessages() const;
    void announce(const String & text, bool interrupt=false);

protected:

    void processNewChatMessages(int index, int count);
    void announceNewChatMessages(int index, int count);
    String formatTimeForSpeech() const;
    void commitChatMessage();

    void showMenu(bool show);
    void showSaveChat();

    void showTabMenu(bool show);
    void updatePrivateChatMap();
    void appendPrivateChatTab(const String & name, bool setcurrent=false);
    void deletePrivateChatTab(int index);
    void setMesgUnreadForTab(int index, bool flag);
    void setMesgUnreadForTab(const String & name, bool flag);

    Colour getOrGenerateUserColor(const String & name);
    
    void updateFontSizes();
    void updateTitles();
    
    bool parseStringForUrl(const String & str, Array<Range<int> > & retranges);

    bool findUrlAtPos(juce::Point<int>, String & retstr);

    void chatTextGainedFocus(FocusChangeType ctype);
    void chatTextLostFocus();

    SonobusAudioProcessor& processor;
    AooServerConnectionInfo & currConnectionInfo;

    double mLastChatMessageStamp = 0.0;
    double mLastChatShowTimeStamp = 0.0;
    double mLastChatUserMessageStamp = 0.0;

    double mLastChatViewStamp = 0.0;

    std::map<String,int> mLastPrivateChatViewEventIndex;
    int mLastGlobalViewEventIndex = 0;
    
    bool mUserScrolled = false;

    bool mKeyboardVisible = false;
    uint32 mKeyboardShownStamp = 0;
    
    uint32 mLastUrlCheckStampMs = 0;
    bool mOverUrl = false;

    MouseCursor mHandCursor = { MouseCursor::PointingHandCursor };
    MouseCursor mTextCursor = { MouseCursor::IBeamCursor };

    int lastShownCount = 0;

    std::unique_ptr<TabbedButtonBar> mChatTabs;
    std::unique_ptr<Component> mChatContainer;
    std::unique_ptr<TextEditor> mChatTextEditor;
    std::vector<AccessibleChatEntry> mAccessibleEntries;
    std::unique_ptr<AccessibleChatList> mAccessibleList;
    std::unique_ptr<FocusTextEditor> mChatSendTextEditor;
    std::unique_ptr<SonoDrawableButton> mMenuButton;
    std::unique_ptr<SonoDrawableButton> mCloseButton;
    std::unique_ptr<Label> mTitleLabel;
    std::unique_ptr<SonoDrawableButton> mChatTabMenuButton;
    std::unique_ptr<SonoDrawableButton> mSendButton;

    // URL position list
    struct cmpRange {
        bool operator()(const Range<int>& a, const Range<int>& b) const {
            return a.getStart() < b.getStart();
        }
    };

    typedef std::map<Range<int>, String, cmpRange> RangeStringMap;
    RangeStringMap mUrlRanges;

    //Array<Range<int> > mUrlRanges;

    // key is username, value is tab index
    typedef std::map<String,int> PrivateChatMap;
    PrivateChatMap mPrivateChatMap;
    
    std::unique_ptr<FileChooser> mFileChooser;


    Font mChatNameFont;
    Font mChatMesgFont;
    Font mChatMesgFixedFont;
    Font mChatEditFont;
    Font mChatEditFixedFont;
    Font mChatSpacerFont;
    String mLastChatEventFrom;
    std::map<String,Colour> mChatUserColors;

    FlexBox mainBox;
    FlexBox titleBox;
    FlexBox chatContainerBox;
    FlexBox chatTextBox;
    FlexBox chatSendBox;
    FlexBox chatTabsBox;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChatView)
};
