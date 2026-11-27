// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception
// Copyright (C) 2020 Jesse Chappell

#pragma once


class SonobusCommands
{
public:
    
    enum {
        MuteAllInput = 1,
        MuteAllPeers,
        TogglePlayPause,
        ToggleLoop,
        TrimSelectionToNewFile,
        CloseFile,
        Connect,
        Disconnect,
        ShareFile,
        RevealFile,
        ShowOptions,
        OpenFile,
        RecordToggle,
        CheckForNewVersion,
        LoadSetupFile,
        SaveSetupFile,
        ChatToggle,
        SoundboardToggle,
        SkipBack,
        ShowFileMenu,
        ShowTransportMenu,
        ShowViewMenu,
        ShowConnectMenu,
        ShowGroupMenu,
        ToggleFullInfoView,
        StopAllSoundboardPlayback,
        ToggleAllMonitorDelay,
        CopyGroupLink,
        GroupLatencyMatch,
        VDONinjaVideoLink,
        SuggestNewGroup,
        ResetAllJitterBuffers,
        RecvSyncToggle,
        FocusChatInput,
        ClearChatMessages,
        HideChatView,
        ToggleChatAnnouncements,
        ReadChatMessage1,   // Alt-1 reads newest message ... Alt-9 the 9th newest
        ReadChatMessage2,
        ReadChatMessage3,
        ReadChatMessage4,
        ReadChatMessage5,
        ReadChatMessage6,
        ReadChatMessage7,
        ReadChatMessage8,
        ReadChatMessage9
    };
    
};
