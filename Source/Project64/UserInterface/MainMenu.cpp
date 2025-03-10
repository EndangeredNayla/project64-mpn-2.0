#include "stdafx.h"
#include "RomInformation.h"
#include "Debugger/Breakpoints.h"
#include "Debugger/ScriptSystem.h"
#include "DiscordRPC.h"
#include <Project64-core/N64System/N64Disk.h>
#include <Project64\UserInterface\About.h>
#include <windows.h>
#include <commdlg.h>

#include "Project64/Kaillera/CKaillera.h"

CMainMenu::CMainMenu(CMainGui * hMainWindow) :
    CBaseMenu(),
    m_ResetAccelerators(true),
    m_Gui(hMainWindow)
{
    ResetMenu();

    hMainWindow->SetWindowMenu(this);

    m_ChangeSettingList.push_back(GameRunning_LimitFPS);
    m_ChangeUISettingList.push_back(UserInterface_InFullScreen);
    m_ChangeUISettingList.push_back(UserInterface_AlwaysOnTop);
    m_ChangeUISettingList.push_back(UserInterface_ShowingNagWindow);
    m_ChangeSettingList.push_back(UserInterface_ShowCPUPer);
    m_ChangeSettingList.push_back(Logging_GenerateLog);
    m_ChangeSettingList.push_back(Debugger_RecordExecutionTimes);
    m_ChangeSettingList.push_back(Debugger_ShowTLBMisses);
    m_ChangeSettingList.push_back(Debugger_ShowUnhandledMemory);
    m_ChangeSettingList.push_back(Debugger_ShowPifErrors);
    m_ChangeSettingList.push_back(Debugger_ShowDListAListCount);
    m_ChangeSettingList.push_back(Debugger_DebugLanguage);
    m_ChangeSettingList.push_back(Debugger_ShowRecompMemSize);
    m_ChangeSettingList.push_back(Debugger_ShowDivByZero);
    m_ChangeSettingList.push_back(Debugger_RecordRecompilerAsm);
    m_ChangeSettingList.push_back(Debugger_DisableGameFixes);
    m_ChangeSettingList.push_back(Debugger_TraceMD5);
    m_ChangeSettingList.push_back(Debugger_TraceSettings);
    m_ChangeSettingList.push_back(Debugger_TraceUnknown);
    m_ChangeSettingList.push_back(Debugger_TraceAppInit);
    m_ChangeSettingList.push_back(Debugger_TraceAppCleanup);
    m_ChangeSettingList.push_back(Debugger_TraceN64System);
    m_ChangeSettingList.push_back(Debugger_TracePlugins);
    m_ChangeSettingList.push_back(Debugger_TraceGFXPlugin);
    m_ChangeSettingList.push_back(Debugger_TraceAudioPlugin);
    m_ChangeSettingList.push_back(Debugger_TraceControllerPlugin);
    m_ChangeSettingList.push_back(Debugger_TraceRSPPlugin);
    m_ChangeSettingList.push_back(Debugger_TraceRSP);
    m_ChangeSettingList.push_back(Debugger_TraceAudio);
    m_ChangeSettingList.push_back(Debugger_TraceRegisterCache);
    m_ChangeSettingList.push_back(Debugger_TraceRecompiler);
    m_ChangeSettingList.push_back(Debugger_TraceTLB);
    m_ChangeSettingList.push_back(Debugger_TraceProtectedMEM);
    m_ChangeSettingList.push_back(Debugger_TraceUserInterface);
    m_ChangeSettingList.push_back(Debugger_AppLogFlush);
    m_ChangeSettingList.push_back(Game_CurrentSaveState);
    m_ChangeSettingList.push_back(Setting_CurrentLanguage);

    for (UISettingList::const_iterator iter = m_ChangeUISettingList.begin(); iter != m_ChangeUISettingList.end(); iter++)
    {
        g_Settings->RegisterChangeCB((SettingID)*iter, this, (CSettings::SettingChangedFunc)SettingsChanged);
    }
    for (SettingList::const_iterator iter = m_ChangeSettingList.begin(); iter != m_ChangeSettingList.end(); iter++)
    {
        g_Settings->RegisterChangeCB(*iter, this, (CSettings::SettingChangedFunc)SettingsChanged);
    }

    g_Settings->RegisterChangeCB((SettingID)Info_ShortCutsChanged, this, (CSettings::SettingChangedFunc)stShortCutsChanged);

    ck = new CKaillera();
}

CMainMenu::~CMainMenu()
{
    g_Settings->UnregisterChangeCB((SettingID)Info_ShortCutsChanged, this, (CSettings::SettingChangedFunc)stShortCutsChanged);
    for (UISettingList::const_iterator iter = m_ChangeUISettingList.begin(); iter != m_ChangeUISettingList.end(); iter++)
    {
        g_Settings->UnregisterChangeCB((SettingID)*iter, this, (CSettings::SettingChangedFunc)SettingsChanged);
    }
    for (SettingList::const_iterator iter = m_ChangeSettingList.begin(); iter != m_ChangeSettingList.end(); iter++)
    {
        g_Settings->UnregisterChangeCB(*iter, this, (CSettings::SettingChangedFunc)SettingsChanged);
    }

    if (ck)
    {
        delete ck;
        ck = NULL;
    }
}

void CMainMenu::SettingsChanged(CMainMenu * _this)
{
    _this->ResetMenu();
}

int CMainMenu::ProcessAccelerator(HWND hWnd, void * lpMsg)
{
    if (m_ResetAccelerators)
    {
        m_ResetAccelerators = false;
        RebuildAccelerators();
    }
    if (!m_AccelTable) { return false; }
    return TranslateAccelerator((HWND)hWnd, (HACCEL)m_AccelTable, (LPMSG)lpMsg);
}

std::string CMainMenu::ChooseFileToOpen(HWND hParent)
{
    CPath FileName;
    const char * Filter = "N64 ROMs and disks (*.zip, *.7z, *.?64, *.rom, *.usa, *.jap, *.pal, *.bin, *.ndd, *.d64)\0*.?64;*.zip;*.7z;*.bin;*.rom;*.usa;*.jap;*.pal;*.ndd;*.d64\0All files (*.*)\0*.*\0";
    if (FileName.SelectFile(hParent, g_Settings->LoadStringVal(RomList_GameDir).c_str(), Filter, true))
    {
        return FileName;
    }
    return "";
}

std::string CMainMenu::ChooseROMFileToOpen(HWND hParent)
{
    CPath FileName;
    const char * Filter = "N64 ROMs (*.zip, *.7z, *.?64, *.rom, *.usa, *.jap, *.pal, *.bin)\0*.?64;*.zip;*.7z;*.bin;*.rom;*.usa;*.jap;*.pal\0All files (*.*)\0*.*\0";
    if (FileName.SelectFile(hParent, g_Settings->LoadStringVal(RomList_GameDir).c_str(), Filter, true))
    {
        return FileName;
    }
    return "";
}

std::string CMainMenu::ChooseDiskFileToOpen(HWND hParent)
{
    CPath FileName;
    const char * Filter = "N64DD disk images (*.ndd, *.d64)\0*.ndd;*.d64\0All files (*.*)\0*.*\0";
    if (FileName.SelectFile(hParent, g_Settings->LoadStringVal(RomList_GameDir).c_str(), Filter, true))
    {
        return FileName;
    }
    return "";
}

void CMainMenu::SetTraceModuleSetttings(SettingID Type)
{
    uint32_t value = g_Settings->LoadDword(Type) == TraceVerbose ? g_Settings->LoadDefaultDword(Type) : TraceVerbose;
    g_Settings->SaveDword(Type, value);
}

void CMainMenu::ShortCutsChanged(void)
{
    m_ShortCuts.Load();
    ResetMenu();
    m_ResetAccelerators = true;
}

void CMainMenu::OnOpenRom(HWND hWnd)
{
    std::string File = ChooseFileToOpen(hWnd);
    if (File.length() == 0)
    {
        return;
    }
    
    stdstr ext = CPath(File).GetExtension();
    if ((_stricmp(ext.c_str(), "ndd") != 0) && (_stricmp(ext.c_str(), "d64") != 0))
    {
        g_BaseSystem->RunFileImage(File.c_str());
        return;
    }
    else
    {
        g_BaseSystem->RunDiskImage(File.c_str());
    }
}

void CMainMenu::OnOpenCombo(HWND hWnd)
{
    std::string FileROM = ChooseROMFileToOpen(hWnd);
    if (FileROM.length() == 0)
    {
        return;
    }

    std::string FileDisk = ChooseDiskFileToOpen(hWnd);
    if (FileDisk.length() == 0)
    {
        return;
    }

    g_BaseSystem->RunDiskComboImage(FileROM.c_str(), FileDisk.c_str());
}

void CMainMenu::OnRomInfo(HWND hWnd)
{
    if (g_Disk)
    {
        RomInformation Info(g_Disk);
        Info.DisplayInformation(hWnd);
    }
    else if (g_Rom)
    {
        RomInformation Info(g_Rom);
        Info.DisplayInformation(hWnd);
    }
}

void CMainMenu::OnEndEmulation(void)
{
    WriteTrace(TraceUserInterface, TraceDebug, "ID_FILE_ENDEMULATION");
    if (g_BaseSystem)
    {
        g_BaseSystem->CloseCpu();
    }
    m_Gui->SaveWindowLoc();

	if (UISettingsLoadBool(Setting_EnableDiscordRPC))
	{
		CDiscord::Update(false);
	}

    if (ck && ck->isPlayingKailleraGame)
    {
        ck->endGame();
        ck->isPlayingKailleraGame = false;
    }
}

void CMainMenu::OnScreenShot(void)
{
    stdstr Dir(g_Settings->LoadStringVal(Directory_SnapShot));
    WriteTrace(TraceGFXPlugin, TraceDebug, "CaptureScreen(%s): Starting", Dir.c_str());
    g_Plugins->Gfx()->CaptureScreen(Dir.c_str());
    WriteTrace(TraceGFXPlugin, TraceDebug, "CaptureScreen: Done");
}

void CMainMenu::OnSaveAs(HWND hWnd)
{
    char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
    char Directory[255], SaveFile[255];
    OPENFILENAMEA openfilename;

    memset(&SaveFile, 0, sizeof(SaveFile));
    memset(&openfilename, 0, sizeof(openfilename));

    UISettingsLoadStringVal(Directory_LastSave, Directory, sizeof(Directory));

    openfilename.lStructSize = sizeof(openfilename);
    openfilename.hwndOwner = (HWND)hWnd;
    openfilename.lpstrFilter = "Project64 saves (*.zip, *.pj)\0*.pj?;*.pj;*.zip;";
    openfilename.lpstrFile = SaveFile;
    openfilename.lpstrInitialDir = Directory;
    openfilename.nMaxFile = MAX_PATH;
    openfilename.Flags = OFN_HIDEREADONLY;

    g_BaseSystem->ExternalEvent(SysEvent_PauseCPU_SaveGame);
    if (GetSaveFileNameA(&openfilename))
    {
        _splitpath(SaveFile, drive, dir, fname, ext);
        if (_stricmp(ext, ".pj") == 0 || _stricmp(ext, ".zip") == 0)
        {
            _makepath(SaveFile, drive, dir, fname, nullptr);
            _splitpath(SaveFile, drive, dir, fname, ext);
            if (_stricmp(ext, ".pj") == 0)
            {
                _makepath(SaveFile, drive, dir, fname, nullptr);
            }
        }
        g_Settings->SaveString(GameRunning_InstantSaveFile, SaveFile);

        char SaveDir[MAX_PATH];
        _makepath(SaveDir, drive, dir, nullptr, nullptr);
        UISettingsSaveString(Directory_LastSave, SaveDir);
        g_BaseSystem->ExternalEvent(SysEvent_SaveMachineState);
    }
    g_BaseSystem->ExternalEvent(SysEvent_ResumeCPU_SaveGame);
}

void CMainMenu::OnLodState(HWND hWnd)
{
    g_BaseSystem->ExternalEvent(SysEvent_PauseCPU_LoadGame);

    char Directory[255];
    UISettingsLoadStringVal(Directory_LastSave, Directory, sizeof(Directory));

    CPath SaveFile;
    const char * Filter = "Project64 saves (*.zip, *.pj)\0*.pj?;*.pj;*.zip;";
    if (SaveFile.SelectFile(hWnd, Directory, Filter, false))
    {
        g_Settings->SaveString(GameRunning_InstantSaveFile, (const char *)SaveFile);
        if (!SaveFile.DirectoryExists())
        {
            SaveFile.DirectoryCreate();
        }
        UISettingsSaveString(Directory_LastSave, SaveFile.GetDriveDirectory());
        g_BaseSystem->ExternalEvent(SysEvent_LoadMachineState);
    }
    g_BaseSystem->ExternalEvent(SysEvent_ResumeCPU_LoadGame);
}

void CMainMenu::OnEnhancements(HWND /*hWnd*/)
{
    m_Gui->DisplayEnhancements(false);
}

void CMainMenu::OnCheats(HWND /*hWnd*/)
{
    m_Gui->DisplayCheatsUI(false);
}

void CMainMenu::OnSettings(HWND hWnd)
{
    CSettingConfig().Display(hWnd);
}

bool CMainMenu::ProcessMessage(HWND hWnd, DWORD /*FromAccelerator*/, DWORD MenuID)
{
    switch (MenuID)
    {
    case ID_FILE_OPEN_ROM: OnOpenRom(hWnd); break;
    case ID_FILE_OPEN_COMBO: OnOpenCombo(hWnd); break;
    case ID_FILE_ROM_INFO: OnRomInfo(hWnd); break;
    case ID_FILE_STARTEMULATION:
        m_Gui->SaveWindowLoc();
        // Now we have created again, we can start up emulation
        if (g_BaseSystem)
        {
            if (g_Settings->LoadBool(Setting_AutoStart) == 0)
            {
                WriteTrace(TraceN64System, TraceDebug, "Manually starting ROM");
            }
            g_BaseSystem->StartEmulation(true);
        }
        else
        {
            g_Notify->BreakPoint(__FILE__, __LINE__);
        }
        break;
    case ID_FILE_ENDEMULATION: OnEndEmulation(); break;
    case ID_FILE_ROMDIRECTORY:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_FILE_ROMDIRECTORY 1");
        m_Gui->SelectRomDir();
        WriteTrace(TraceUserInterface, TraceDebug, "ID_FILE_ROMDIRECTORY 2");
        m_Gui->RefreshMenu();
        WriteTrace(TraceUserInterface, TraceDebug, "ID_FILE_ROMDIRECTORY 3");
        break;
    case ID_FILE_REFRESHROMLIST: m_Gui->RefreshRomList(); break;
    case ID_FILE_KAILLERA:
        ck->setInfos();
        ck->startDialogThread(hWnd);
        break;
    case ID_FILE_EXIT:           DestroyWindow((HWND)hWnd); PostQuitMessage(0);  break;
    case ID_SYSTEM_RESET_SOFT:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_RESET_SOFT");
        g_BaseSystem->ExternalEvent(SysEvent_ResetCPU_Soft);
        break;
    case ID_SYSTEM_RESET_HARD:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_RESET_HARD");
        g_BaseSystem->ExternalEvent(SysEvent_ResetCPU_Hard);
        break;
    case ID_SYSTEM_PAUSE:
        m_Gui->SaveWindowLoc();
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_PAUSE");
        g_BaseSystem->ExternalEvent(g_Settings->LoadBool(GameRunning_CPU_Paused) ? SysEvent_ResumeCPU_FromMenu : SysEvent_PauseCPU_FromMenu);
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_PAUSE 1");
        break;
    case ID_SYSTEM_BITMAP: OnScreenShot(); break;
        break;
    case ID_SYSTEM_LIMITFPS:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_LIMITFPS");
        g_Settings->SaveBool(GameRunning_LimitFPS, !g_Settings->LoadBool(GameRunning_LimitFPS));
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_LIMITFPS 1");
        break;
    case ID_SYSTEM_SWAPDISK:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_SWAPDISK");
        {
            // Open disk
            stdstr FileName = ChooseDiskFileToOpen(hWnd);
            if (FileName.length() != 0)
            {
                g_Disk->SaveDiskImage();
                g_Disk->SwapDiskImage(FileName.c_str());
            }
        }
        break;
    case ID_SYSTEM_SAVE:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_SAVE");
        g_BaseSystem->ExternalEvent(SysEvent_SaveMachineState);
        break;
    case ID_SYSTEM_SAVEAS: OnSaveAs(hWnd); break;
    case ID_SYSTEM_RESTORE:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_SYSTEM_RESTORE");
        g_BaseSystem->ExternalEvent(SysEvent_LoadMachineState);
        break;
    case ID_SYSTEM_LOAD: OnLodState(hWnd); break;
    case ID_SYSTEM_ENHANCEMENT: OnEnhancements(hWnd); break;
    case ID_SYSTEM_CHEAT: OnCheats(hWnd); break;
    case ID_SYSTEM_GSBUTTON:
        g_BaseSystem->ExternalEvent(SysEvent_GSButtonPressed);
        break;
    case ID_OPTIONS_DISPLAY_FR:
        g_Settings->SaveBool(UserInterface_DisplayFrameRate, !g_Settings->LoadBool(UserInterface_DisplayFrameRate));
        break;
    case ID_OPTIONS_CHANGE_FR:
        switch (g_Settings->LoadDword(UserInterface_FrameDisplayType))
        {
        case FR_VIs:
            g_Settings->SaveDword(UserInterface_FrameDisplayType, FR_DLs);
            break;
        case FR_DLs:
            g_Settings->SaveDword(UserInterface_FrameDisplayType, FR_PERCENT);
            break;
        default:
            g_Settings->SaveDword(UserInterface_FrameDisplayType, FR_VIs);
        }
        break;
    case ID_OPTIONS_INCREASE_SPEED:
        g_BaseSystem->AlterSpeed(CSpeedLimiter::INCREASE_SPEED);
        break;
    case ID_OPTIONS_DECREASE_SPEED:
        g_BaseSystem->AlterSpeed(CSpeedLimiter::DECREASE_SPEED);
        break;
    case ID_OPTIONS_FULLSCREEN:
        g_BaseSystem->ExternalEvent(SysEvent_ChangingFullScreen);
        break;
    case ID_OPTIONS_FULLSCREEN2:
        if (UISettingsLoadBool(UserInterface_InFullScreen))
        {
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN a");
            m_Gui->MakeWindowOnTop(false);
            Notify().SetGfxPlugin(nullptr);
            WriteTrace(TraceGFXPlugin, TraceDebug, "ChangeWindow: Starting");
            g_Plugins->Gfx()->ChangeWindow();
            WriteTrace(TraceGFXPlugin, TraceDebug, "ChangeWindow: Done");
            ShowCursor(true);
            m_Gui->ShowStatusBar(true);
            m_Gui->MakeWindowOnTop(UISettingsLoadBool(UserInterface_AlwaysOnTop));
            UISettingsSaveBool(UserInterface_InFullScreen, (DWORD)false);
        }
        else
        {
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b");
            ShowCursor(false);
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b 1");
            m_Gui->ShowStatusBar(false);
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b 2");
            try
            {
                WriteTrace(TraceGFXPlugin, TraceDebug, "ChangeWindow: Starting");
                g_Plugins->Gfx()->ChangeWindow();
                WriteTrace(TraceGFXPlugin, TraceDebug, "ChangeWindow: Done");
            }
            catch (...)
            {
                WriteTrace(TraceError, TraceDebug, "Exception when going to full screen");
                char Message[600];
                sprintf(Message, "Exception caught\nFile: %s\nLine: %d", __FILE__, __LINE__);
                MessageBox(nullptr, stdstr(Message).ToUTF16().c_str(), L"Exception", MB_OK);
            }
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b 4");
            m_Gui->MakeWindowOnTop(false);
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b 5");
            Notify().SetGfxPlugin(g_Plugins->Gfx());
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b 3");
            UISettingsSaveBool(UserInterface_InFullScreen, true);
            WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN b 6");
        }
        WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_FULLSCREEN 1");
        break;
    case ID_OPTIONS_ALWAYSONTOP:
        if (UISettingsLoadBool(UserInterface_AlwaysOnTop))
        {
            UISettingsSaveBool(UserInterface_AlwaysOnTop, false);
            m_Gui->MakeWindowOnTop(false);
        }
        else
        {
            UISettingsSaveBool(UserInterface_AlwaysOnTop, true);
            m_Gui->MakeWindowOnTop(g_Settings->LoadBool(GameRunning_CPU_Running));
        }
        break;
    case ID_OPTIONS_CONFIG_RSP:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_CONFIG_RSP");
        g_Plugins->ConfigPlugin(hWnd, PLUGIN_TYPE_RSP);
        break;
    case ID_OPTIONS_CONFIG_GFX:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_CONFIG_GFX");
        g_Plugins->ConfigPlugin(hWnd, PLUGIN_TYPE_GFX);
        break;
    case ID_OPTIONS_CONFIG_AUDIO:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_CONFIG_AUDIO");
        g_Plugins->ConfigPlugin(hWnd, PLUGIN_TYPE_AUDIO);
        break;
    case ID_OPTIONS_CONFIG_CONT:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_CONFIG_CONT");
        g_Plugins->ConfigPlugin(hWnd, PLUGIN_TYPE_CONTROLLER);
        break;
    case ID_OPTIONS_CPU_USAGE:
        WriteTrace(TraceUserInterface, TraceDebug, "ID_OPTIONS_CPU_USAGE");
        if (g_Settings->LoadBool(UserInterface_ShowCPUPer))
        {
            g_Settings->SaveBool(UserInterface_ShowCPUPer, false);
            g_Notify->DisplayMessage(0, EMPTY_STRING);
        }
        else
        {
            g_Settings->SaveBool(UserInterface_ShowCPUPer, true);
        }
        break;
    case ID_OPTIONS_SETTINGS: OnSettings(hWnd);  break;
    case ID_PROFILE_PROFILE:
        g_Settings->SaveBool(Debugger_RecordExecutionTimes, !g_Settings->LoadBool(Debugger_RecordExecutionTimes));
        g_BaseSystem->ExternalEvent(SysEvent_ResetFunctionTimes);
        break;
    case ID_PROFILE_RESETCOUNTER: g_BaseSystem->ExternalEvent(SysEvent_ResetFunctionTimes); break;
    case ID_PROFILE_GENERATELOG: g_BaseSystem->ExternalEvent(SysEvent_DumpFunctionTimes); break;
    case ID_DEBUG_SHOW_TLB_MISSES:
        g_Settings->SaveBool(Debugger_ShowTLBMisses, !g_Settings->LoadBool(Debugger_ShowTLBMisses));
        break;
    case ID_DEBUG_SHOW_UNHANDLED_MEM:
        g_Settings->SaveBool(Debugger_ShowUnhandledMemory, !g_Settings->LoadBool(Debugger_ShowUnhandledMemory));
        break;
    case ID_DEBUG_SHOW_PIF_ERRORS:
        g_Settings->SaveBool(Debugger_ShowPifErrors, !g_Settings->LoadBool(Debugger_ShowPifErrors));
        break;
    case ID_DEBUG_SHOW_DLIST_COUNT:
        g_Settings->SaveBool(Debugger_ShowDListAListCount, !g_Settings->LoadBool(Debugger_ShowDListAListCount));
        g_Notify->DisplayMessage(0, EMPTY_STRING);
        break;
    case ID_DEBUG_LANGUAGE:
        g_Settings->SaveBool(Debugger_DebugLanguage, !g_Settings->LoadBool(Debugger_DebugLanguage));
        g_Lang->LoadCurrentStrings();
        m_Gui->ResetRomBrowserColomuns();
        break;
    case ID_DEBUG_SHOW_RECOMP_MEM_SIZE:
        g_Settings->SaveBool(Debugger_ShowRecompMemSize, !g_Settings->LoadBool(Debugger_ShowRecompMemSize));
        g_Notify->DisplayMessage(0, EMPTY_STRING);
        break;
    case ID_DEBUG_SHOW_DIV_BY_ZERO:
        g_Settings->SaveBool(Debugger_ShowDivByZero, !g_Settings->LoadBool(Debugger_ShowDivByZero));
        break;
    case ID_DEBUG_RECORD_RECOMPILER_ASM:
        g_Settings->SaveBool(Debugger_RecordRecompilerAsm, !g_Settings->LoadBool(Debugger_RecordRecompilerAsm));
        break;
    case ID_DEBUG_DISABLE_GAMEFIX:
        g_Settings->SaveBool(Debugger_DisableGameFixes, !g_Settings->LoadBool(Debugger_DisableGameFixes));
        break;
    case ID_DEBUGGER_TRACE_MD5: SetTraceModuleSetttings(Debugger_TraceMD5); break;
    case ID_DEBUGGER_TRACE_SETTINGS: SetTraceModuleSetttings(Debugger_TraceSettings); break;
    case ID_DEBUGGER_TRACE_UNKNOWN: SetTraceModuleSetttings(Debugger_TraceUnknown); break;
    case ID_DEBUGGER_TRACE_APPINIT: SetTraceModuleSetttings(Debugger_TraceAppInit); break;
    case ID_DEBUGGER_TRACE_APPCLEANUP: SetTraceModuleSetttings(Debugger_TraceAppCleanup); break;
    case ID_DEBUGGER_TRACE_N64SYSTEM: SetTraceModuleSetttings(Debugger_TraceN64System); break;
    case ID_DEBUGGER_TRACE_PLUGINS: SetTraceModuleSetttings(Debugger_TracePlugins); break;
    case ID_DEBUGGER_TRACE_GFXPLUGIN: SetTraceModuleSetttings(Debugger_TraceGFXPlugin); break;
    case ID_DEBUGGER_TRACE_AUDIOPLUGIN: SetTraceModuleSetttings(Debugger_TraceAudioPlugin); break;
    case ID_DEBUGGER_TRACE_CONTROLLERPLUGIN: SetTraceModuleSetttings(Debugger_TraceControllerPlugin); break;
    case ID_DEBUGGER_TRACE_RSPPLUGIN: SetTraceModuleSetttings(Debugger_TraceRSPPlugin); break;
    case ID_DEBUGGER_TRACE_RSP: SetTraceModuleSetttings(Debugger_TraceRSP); break;
    case ID_DEBUGGER_TRACE_AUDIO: SetTraceModuleSetttings(Debugger_TraceAudio); break;
    case ID_DEBUGGER_TRACE_REGISTERCACHE: SetTraceModuleSetttings(Debugger_TraceRegisterCache); break;
    case ID_DEBUGGER_TRACE_RECOMPILER: SetTraceModuleSetttings(Debugger_TraceRecompiler); break;
    case ID_DEBUGGER_TRACE_TLB: SetTraceModuleSetttings(Debugger_TraceTLB); break;
    case ID_DEBUGGER_TRACE_PROTECTEDMEM: SetTraceModuleSetttings(Debugger_TraceProtectedMEM); break;
    case ID_DEBUGGER_TRACE_USERINTERFACE: SetTraceModuleSetttings(Debugger_TraceUserInterface); break;

    case ID_DEBUGGER_APPLOG_FLUSH:
        g_Settings->SaveBool(Debugger_AppLogFlush, !g_Settings->LoadBool(Debugger_AppLogFlush));
        break;
    case ID_DEBUGGER_LOGOPTIONS: m_Gui->EnterLogOptions(); break;
    case ID_DEBUGGER_GENERATELOG:
        g_Settings->SaveBool(Logging_GenerateLog, !g_Settings->LoadBool(Logging_GenerateLog));
        break;
    case ID_DEBUGGER_DUMPMEMORY: g_Debugger->OpenMemoryDump(); break;
    case ID_DEBUGGER_SEARCHMEMORY: g_Debugger->OpenMemorySearch(); break;
    case ID_DEBUGGER_MEMORY: g_Debugger->OpenMemoryWindow(); break;
    case ID_DEBUGGER_TLBENTRIES: g_Debugger->OpenTLBWindow(); break;
    case ID_DEBUGGER_INTERRUPT_SP: g_BaseSystem->ExternalEvent(SysEvent_Interrupt_SP); break;
    case ID_DEBUGGER_INTERRUPT_SI: g_BaseSystem->ExternalEvent(SysEvent_Interrupt_SI); break;
    case ID_DEBUGGER_INTERRUPT_AI: g_BaseSystem->ExternalEvent(SysEvent_Interrupt_AI); break;
    case ID_DEBUGGER_INTERRUPT_VI: g_BaseSystem->ExternalEvent(SysEvent_Interrupt_VI); break;
    case ID_DEBUGGER_INTERRUPT_PI: g_BaseSystem->ExternalEvent(SysEvent_Interrupt_PI); break;
    case ID_DEBUGGER_INTERRUPT_DP: g_BaseSystem->ExternalEvent(SysEvent_Interrupt_DP); break;
    case ID_DEBUGGER_BREAKPOINTS: g_Debugger->OpenCommandWindow(); break;
    case ID_DEBUGGER_SCRIPTS: g_Debugger->OpenScriptsWindow(); break;
    case ID_DEBUGGER_SYMBOLS: g_Debugger->OpenSymbolsWindow(); break;
    case ID_DEBUGGER_DMALOG: g_Debugger->OpenDMALogWindow(); break;
    case ID_DEBUGGER_CPULOG: g_Debugger->OpenCPULogWindow(); break;
    case ID_DEBUGGER_EXCBREAKPOINTS: g_Debugger->OpenExcBreakpointsWindow(); break;
    case ID_DEBUGGER_STACKTRACE: g_Debugger->OpenStackTraceWindow(); break;
    case ID_DEBUGGER_STACKVIEW: g_Debugger->OpenStackViewWindow(); break;
    case ID_CURRENT_SAVE_DEFAULT:
        g_Notify->DisplayMessage(3, stdstr_f(GS(MENU_SLOT_SAVE), GetSaveSlotString(MenuID - ID_CURRENT_SAVE_DEFAULT).c_str()).c_str());
        g_Settings->SaveDword(Game_CurrentSaveState, (DWORD)(MenuID - ID_CURRENT_SAVE_DEFAULT));
        break;
    case ID_CURRENT_SAVE_1:
    case ID_CURRENT_SAVE_2:
    case ID_CURRENT_SAVE_3:
    case ID_CURRENT_SAVE_4:
    case ID_CURRENT_SAVE_5:
    case ID_CURRENT_SAVE_6:
    case ID_CURRENT_SAVE_7:
    case ID_CURRENT_SAVE_8:
    case ID_CURRENT_SAVE_9:
    case ID_CURRENT_SAVE_10:
        g_Notify->DisplayMessage(3, stdstr_f(GS(MENU_SLOT_SAVE), GetSaveSlotString((MenuID - ID_CURRENT_SAVE_1) + 1).c_str()).c_str());
        g_Settings->SaveDword(Game_CurrentSaveState, (DWORD)((MenuID - ID_CURRENT_SAVE_1) + 1));
        break;
    case ID_HELP_MPN_DISCORD: ShellExecute(nullptr, L"open", L"https://discord.gg/marioparty", nullptr, nullptr, SW_SHOWMAXIMIZED); break;
    case ID_HELP_DISCORD: ShellExecute(nullptr, L"open", L"https://discord.gg/Cg3zquF", nullptr, nullptr, SW_SHOWMAXIMIZED); break;
    case ID_HELP_WEBSITE: ShellExecute(nullptr, L"open", L"http://www.pj64-emu.com", nullptr, nullptr, SW_SHOWMAXIMIZED); break;
    case ID_HELP_ABOUT: CAboutDlg(m_Gui->Support()).DoModal(); break;
    default:
        if (MenuID >= ID_RECENT_ROM_START && MenuID < ID_RECENT_ROM_END)
        {
            stdstr FileName;
            if (UISettingsLoadStringIndex(File_RecentGameFileIndex, MenuID - ID_RECENT_ROM_START, FileName) &&
                FileName.length() > 0)
            {
                if ((CPath(FileName).GetExtension() != "ndd") && (CPath(FileName).GetExtension() != "d64"))
                    g_BaseSystem->RunFileImage(FileName.c_str());
                else
                    g_BaseSystem->RunDiskImage(FileName.c_str());
            }
        }
        if (MenuID >= ID_RECENT_DIR_START && MenuID < ID_RECENT_DIR_END)
        {
            int Offset = MenuID - ID_RECENT_DIR_START;
            stdstr Dir = UISettingsLoadStringIndex(Directory_RecentGameDirIndex, Offset);
            if (Dir.length() > 0)
            {
                g_Settings->SaveString(RomList_GameDir, Dir.c_str());
                Notify().AddRecentDir(Dir.c_str());
                m_Gui->RefreshMenu();
                if (m_Gui->RomBrowserVisible())
                {
                    m_Gui->RefreshRomList();
                }
            }
        }
        if (MenuID >= ID_LANG_START && MenuID < ID_LANG_END)
        {
            MENUITEMINFO menuinfo;
            wchar_t String[300];

            menuinfo.cbSize = sizeof(MENUITEMINFO);
            menuinfo.fMask = MIIM_TYPE;
            menuinfo.fType = MFT_STRING;
            menuinfo.dwTypeData = String;
            menuinfo.cch = sizeof(String);
            GetMenuItemInfo((HMENU)m_MenuHandle, MenuID, FALSE, &menuinfo);

            g_Lang->SetLanguage(stdstr().FromUTF16(String).c_str());
            m_Gui->ResetRomBrowserColomuns();
            break;
        }
        return false;
    }
    return true;
}

stdstr CMainMenu::GetFileLastMod(const CPath & FileName)
{
    HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return "";
    }
    FILETIME CreationTime, LastAccessTime, LastWriteTime;
    stdstr LastMod;
    if (GetFileTime(hFile, &CreationTime, &LastAccessTime, &LastWriteTime))
    {
        SYSTEMTIME stUTC, stLocal;

        // Convert the last-write time to local time
        FileTimeToSystemTime(&LastWriteTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);

        LastMod.Format(" [%d/%02d/%02d %02d:%02d]", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute);
    }
    CloseHandle(hFile);

    return LastMod;
}

std::wstring CMainMenu::GetSaveSlotString(int Slot)
{
    stdstr SlotName;
    switch (Slot)
    {
    case 0: SlotName = GS(MENU_SLOT_DEFAULT); break;
    case 1: SlotName = GS(MENU_SLOT_1); break;
    case 2: SlotName = GS(MENU_SLOT_2); break;
    case 3: SlotName = GS(MENU_SLOT_3); break;
    case 4: SlotName = GS(MENU_SLOT_4); break;
    case 5: SlotName = GS(MENU_SLOT_5); break;
    case 6: SlotName = GS(MENU_SLOT_6); break;
    case 7: SlotName = GS(MENU_SLOT_7); break;
    case 8: SlotName = GS(MENU_SLOT_8); break;
    case 9: SlotName = GS(MENU_SLOT_9); break;
    case 10: SlotName = GS(MENU_SLOT_10); break;
    }

    if (!g_Settings->LoadBool(GameRunning_CPU_Running)) { return SlotName.ToUTF16(); }

    stdstr LastSaveTime;

    // Check first save name
    CPath FileName(g_Settings->LoadStringVal(Directory_InstantSave).c_str(), "");
    FileName.NormalizePath(CPath(CPath::MODULE_DIRECTORY));
    if (Slot != 0)
    {
        FileName.SetNameExtension(stdstr_f("%s.pj%d", g_Settings->LoadStringVal(Rdb_GoodName).c_str(), Slot).c_str());
    }
    else
    {
        FileName.SetNameExtension(stdstr_f("%s.pj", g_Settings->LoadStringVal(Rdb_GoodName).c_str()).c_str());
    }

    if (g_Settings->LoadDword(Setting_AutoZipInstantSave))
    {
        CPath ZipFileName(FileName.GetDriveDirectory(), stdstr_f("%s.zip", FileName.GetNameExtension().c_str()).c_str());
        LastSaveTime = GetFileLastMod(ZipFileName);
    }
    if (LastSaveTime.empty())
    {
        LastSaveTime = GetFileLastMod(FileName);
    }

    // Check old file name
    if (LastSaveTime.empty())
    {
        if (Slot > 0)
        {
            FileName.SetNameExtension(stdstr_f("%s.pj%d", g_Settings->LoadStringVal(Game_GameName).c_str(), Slot).c_str());
        }
        else
        {
            FileName.SetNameExtension(stdstr_f("%s.pj", g_Settings->LoadStringVal(Game_GameName).c_str()).c_str());
        }

        if (g_Settings->LoadBool(Setting_AutoZipInstantSave))
        {
            CPath ZipFileName(FileName.GetDriveDirectory(), stdstr_f("%s.zip", FileName.GetNameExtension().c_str()).c_str());
            LastSaveTime = GetFileLastMod(ZipFileName);
        }
        if (LastSaveTime.empty())
        {
            LastSaveTime = GetFileLastMod(FileName);
        }
    }
    SlotName += LastSaveTime;
    return SlotName.ToUTF16();
}

void CMainMenu::FillOutMenu(HMENU hMenu)
{
    CGuard Guard(m_CS);

    MENU_ITEM Item;

    // Get all flags
    bool inBasicMode = g_Settings->LoadBool(UserInterface_BasicMode);
    bool CPURunning = g_Settings->LoadBool(GameRunning_CPU_Running);
    bool RomLoading = g_Settings->LoadBool(GameRunning_LoadingInProgress);
    bool RomLoaded = g_Settings->LoadStringVal(Game_GameName).length() > 0;
    bool RomList = UISettingsLoadBool(RomBrowser_Enabled) && !CPURunning;
    bool Enhancement = g_Settings->LoadBool(Setting_Enhancement);

    CMenuShortCutKey::RUNNING_STATE RunningState = CMenuShortCutKey::RUNNING_STATE_NOT_RUNNING;
    if (g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        RunningState = UISettingsLoadBool(UserInterface_InFullScreen) ? CMenuShortCutKey::RUNNING_STATE_FULLSCREEN : CMenuShortCutKey::RUNNING_STATE_WINDOWED;
    }

    // Get the system information to make the menu
    LanguageList LangList = g_Lang->GetLangList();

    MenuItemList LangMenu;
    int Offset = 0;
    for (LanguageList::iterator Language = LangList.begin(); Language != LangList.end(); Language++)
    {
        Item.Reset(ID_LANG_START + Offset++, EMPTY_STRING, EMPTY_STDSTR, nullptr, stdstr(Language->LanguageName).ToUTF16().c_str());
        if (g_Lang->IsCurrentLang(*Language))
        {
            Item.SetItemTicked(true);
        }
        LangMenu.push_back(Item);
    }

    // Go through the settings to create a list of recent ROMS
    MenuItemList RecentRomMenu;
    DWORD count, RomsToRemember = UISettingsLoadDword(File_RecentGameFileCount);

    for (count = 0; count < RomsToRemember; count++)
    {
        stdstr LastRom = UISettingsLoadStringIndex(File_RecentGameFileIndex, count);
        if (LastRom.empty())
        {
            break;
        }
        stdstr_f MenuString("&%d %s", (count + 1) % 10, LastRom.c_str());

        RecentRomMenu.push_back(MENU_ITEM(ID_RECENT_ROM_START + count, EMPTY_STRING, EMPTY_STDSTR, nullptr, MenuString.ToUTF16(CP_ACP).c_str()));
    }

    // Recent directory
    MenuItemList RecentDirMenu;
    DWORD DirsToRemember = UISettingsLoadDword(Directory_RecentGameDirCount);

    for (count = 0; count < DirsToRemember; count++)
    {
        stdstr LastDir = UISettingsLoadStringIndex(Directory_RecentGameDirIndex, count);
        if (LastDir.empty())
        {
            break;
        }

        stdstr_f MenuString("&%d %s", (count + 1) % 10, LastDir.c_str());

        RecentDirMenu.push_back(MENU_ITEM(ID_RECENT_DIR_START + count, EMPTY_STRING, EMPTY_STDSTR, nullptr, MenuString.ToUTF16(CP_ACP).c_str()));
    }

    // File menu
    MenuItemList FileMenu;
    Item.Reset(ID_FILE_OPEN_ROM, MENU_OPEN, m_ShortCuts.ShortCutString(ID_FILE_OPEN_ROM, RunningState));
    FileMenu.push_back(Item);
    Item.Reset(ID_FILE_OPEN_COMBO, MENU_OPEN_COMBO, m_ShortCuts.ShortCutString(ID_FILE_OPEN_COMBO, RunningState));
    FileMenu.push_back(Item);
    if (!inBasicMode)
    {
        Item.Reset(ID_FILE_ROM_INFO, MENU_ROM_INFO, m_ShortCuts.ShortCutString(ID_FILE_ROM_INFO, RunningState));
        Item.SetItemEnabled(RomLoaded);
        FileMenu.push_back(Item);
        FileMenu.push_back(MENU_ITEM(SPLITER));
        Item.Reset(ID_FILE_STARTEMULATION, MENU_START, m_ShortCuts.ShortCutString(ID_FILE_STARTEMULATION, RunningState));
        Item.SetItemEnabled(RomLoaded && !CPURunning);
        FileMenu.push_back(Item);
    }
    Item.Reset(ID_FILE_ENDEMULATION, MENU_END, m_ShortCuts.ShortCutString(ID_FILE_ENDEMULATION, RunningState));
    Item.SetItemEnabled(CPURunning);
    FileMenu.push_back(Item);
    FileMenu.push_back(MENU_ITEM(SPLITER));
    Item.Reset(SUB_MENU, MENU_LANGUAGE, EMPTY_STDSTR, &LangMenu);
    FileMenu.push_back(Item);
    if (RomList)
    {
        FileMenu.push_back(MENU_ITEM(SPLITER));
        Item.Reset(ID_FILE_ROMDIRECTORY, MENU_CHOOSE_ROM, m_ShortCuts.ShortCutString(ID_FILE_ROMDIRECTORY, RunningState));
        FileMenu.push_back(Item);
        Item.Reset(ID_FILE_REFRESHROMLIST, MENU_REFRESH, m_ShortCuts.ShortCutString(ID_FILE_REFRESHROMLIST, RunningState));
        FileMenu.push_back(Item);
    }

    if (!inBasicMode && RomList)
    {
        FileMenu.push_back(MENU_ITEM(SPLITER));
        Item.Reset(SUB_MENU, MENU_RECENT_ROM, EMPTY_STDSTR, &RecentRomMenu);
        if (RecentRomMenu.size() == 0)
        {
            RecentRomMenu.push_back(MENU_ITEM(SPLITER));
            Item.SetItemEnabled(false);
        }
        FileMenu.push_back(Item);
        Item.Reset(SUB_MENU, MENU_RECENT_DIR, EMPTY_STDSTR, &RecentDirMenu);
        if (RecentDirMenu.size() == 0)
        {
            RecentDirMenu.push_back(MENU_ITEM(SPLITER));
            Item.SetItemEnabled(false);
        }
        FileMenu.push_back(Item);
    }
    else
    {
        if (RecentRomMenu.size() != 0)
        {
            FileMenu.push_back(MENU_ITEM(SPLITER));
            for (MenuItemList::iterator MenuItem = RecentRomMenu.begin(); MenuItem != RecentRomMenu.end(); MenuItem++)
            {
                FileMenu.push_back(*MenuItem);
            }
        }
    }
    FileMenu.push_back(MENU_ITEM(SPLITER));
    FileMenu.push_back(MENU_ITEM(ID_FILE_KAILLERA, MENU_KAILLERA, m_ShortCuts.ShortCutString(ID_FILE_KAILLERA, RunningState)));
    FileMenu.push_back(MENU_ITEM(SPLITER));
    FileMenu.push_back(MENU_ITEM(ID_FILE_EXIT, MENU_EXIT, m_ShortCuts.ShortCutString(ID_FILE_EXIT, RunningState)));

    // Current save
    MenuItemList CurrentSaveMenu;
    DWORD _CurrentSaveState = g_Settings->LoadDword(Game_CurrentSaveState);
    Item.Reset(ID_CURRENT_SAVE_DEFAULT, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_DEFAULT, RunningState), nullptr, GetSaveSlotString(0));
    if (_CurrentSaveState == 0) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    CurrentSaveMenu.push_back(MENU_ITEM(SPLITER));
    Item.Reset(ID_CURRENT_SAVE_1, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_1, RunningState), nullptr, GetSaveSlotString(1));
    if (_CurrentSaveState == 1) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_2, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_2, RunningState), nullptr, GetSaveSlotString(2));
    if (_CurrentSaveState == 2) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_3, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_3, RunningState), nullptr, GetSaveSlotString(3));
    if (_CurrentSaveState == 3) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_4, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_4, RunningState), nullptr, GetSaveSlotString(4));
    if (_CurrentSaveState == 4) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_5, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_5, RunningState), nullptr, GetSaveSlotString(5));
    if (_CurrentSaveState == 5) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_6, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_6, RunningState), nullptr, GetSaveSlotString(6));
    if (_CurrentSaveState == 6) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_7, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_7, RunningState), nullptr, GetSaveSlotString(7));
    if (_CurrentSaveState == 7) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_8, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_8, RunningState), nullptr, GetSaveSlotString(8));
    if (_CurrentSaveState == 8) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_9, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_9, RunningState), nullptr, GetSaveSlotString(9));
    if (_CurrentSaveState == 9) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);
    Item.Reset(ID_CURRENT_SAVE_10, EMPTY_STRING, m_ShortCuts.ShortCutString(ID_CURRENT_SAVE_10, RunningState), nullptr, GetSaveSlotString(10));
    if (_CurrentSaveState == 10) { Item.SetItemTicked(true); }
    CurrentSaveMenu.push_back(Item);

    // System menu
    MenuItemList SystemMenu;
    MenuItemList ResetMenu;
    if (inBasicMode)
    {
        SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_RESET_SOFT, MENU_RESET, m_ShortCuts.ShortCutString(ID_SYSTEM_RESET_SOFT, RunningState)));
    }
    else
    {
        ResetMenu.push_back(MENU_ITEM(ID_SYSTEM_RESET_SOFT, MENU_RESET_SOFT, m_ShortCuts.ShortCutString(ID_SYSTEM_RESET_SOFT, RunningState)));
        ResetMenu.push_back(MENU_ITEM(ID_SYSTEM_RESET_HARD, MENU_RESET_HARD, m_ShortCuts.ShortCutString(ID_SYSTEM_RESET_HARD, RunningState)));
        SystemMenu.push_back(MENU_ITEM(SUB_MENU, MENU_RESET, EMPTY_STDSTR, &ResetMenu));
    }
    if (g_Settings->LoadBool(GameRunning_CPU_Paused))
    {
        SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_PAUSE, MENU_RESUME, m_ShortCuts.ShortCutString(ID_SYSTEM_PAUSE, RunningState)));
    }
    else
    {
        SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_PAUSE, MENU_PAUSE, m_ShortCuts.ShortCutString(ID_SYSTEM_PAUSE, RunningState)));
    }
    SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_BITMAP, MENU_BITMAP, m_ShortCuts.ShortCutString(ID_SYSTEM_BITMAP, RunningState)));
    SystemMenu.push_back(MENU_ITEM(SPLITER));
    if (!inBasicMode)
    {
        Item.Reset(ID_SYSTEM_LIMITFPS, MENU_LIMIT_FPS, m_ShortCuts.ShortCutString(ID_SYSTEM_LIMITFPS, RunningState));
        if (g_Settings->LoadBool(GameRunning_LimitFPS)) { Item.SetItemTicked(true); }
        SystemMenu.push_back(Item);
        SystemMenu.push_back(MENU_ITEM(SPLITER));
    }
    Item.Reset(ID_SYSTEM_SWAPDISK, MENU_SWAPDISK, m_ShortCuts.ShortCutString(ID_SYSTEM_SWAPDISK, RunningState));
    if (g_Disk == nullptr) { Item.SetItemEnabled(false); }
    SystemMenu.push_back(Item);
    SystemMenu.push_back(MENU_ITEM(SPLITER));
    SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_SAVE, MENU_SAVE, m_ShortCuts.ShortCutString(ID_SYSTEM_SAVE, RunningState)));
    if (!inBasicMode)
    {
        SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_SAVEAS, MENU_SAVE_AS, m_ShortCuts.ShortCutString(ID_SYSTEM_SAVEAS, RunningState)));
    }
    SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_RESTORE, MENU_RESTORE, m_ShortCuts.ShortCutString(ID_SYSTEM_RESTORE, RunningState)));
    if (!inBasicMode)
    {
        SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_LOAD, MENU_LOAD, m_ShortCuts.ShortCutString(ID_SYSTEM_LOAD, RunningState)));
    }
    SystemMenu.push_back(MENU_ITEM(SPLITER));
    SystemMenu.push_back(MENU_ITEM(SUB_MENU, MENU_CURRENT_SAVE, EMPTY_STDSTR, &CurrentSaveMenu));
    SystemMenu.push_back(MENU_ITEM(SPLITER));
    if (Enhancement)
    {
        SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_ENHANCEMENT, MENU_ENHANCEMENT, m_ShortCuts.ShortCutString(ID_SYSTEM_ENHANCEMENT, RunningState)));
    }
    SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_CHEAT, MENU_CHEAT, m_ShortCuts.ShortCutString(ID_SYSTEM_CHEAT, RunningState)));
    SystemMenu.push_back(MENU_ITEM(ID_SYSTEM_GSBUTTON, MENU_GS_BUTTON, m_ShortCuts.ShortCutString(ID_SYSTEM_GSBUTTON, RunningState)));

    // Option menu
    MenuItemList OptionMenu;
    Item.Reset(ID_OPTIONS_FULLSCREEN, MENU_FULL_SCREEN, m_ShortCuts.ShortCutString(ID_OPTIONS_FULLSCREEN, RunningState));
    Item.SetItemEnabled(CPURunning);
    if (g_Plugins && g_Plugins->Gfx() && g_Plugins->Gfx()->ChangeWindow == nullptr)
    {
        Item.SetItemEnabled(false);
    }
    OptionMenu.push_back(Item);
    if (!inBasicMode)
    {
        Item.Reset(ID_OPTIONS_ALWAYSONTOP, MENU_ON_TOP, m_ShortCuts.ShortCutString(ID_OPTIONS_ALWAYSONTOP, RunningState));
        if (UISettingsLoadDword(UserInterface_AlwaysOnTop)) { Item.SetItemTicked(true); }
        Item.SetItemEnabled(CPURunning);
        OptionMenu.push_back(Item);
    }
    OptionMenu.push_back(MENU_ITEM(SPLITER));

    Item.Reset(ID_OPTIONS_CONFIG_GFX, MENU_CONFG_GFX, m_ShortCuts.ShortCutString(ID_OPTIONS_CONFIG_GFX, RunningState));
    if (g_Plugins && g_Plugins->Gfx() == nullptr || g_Plugins->Gfx()->DllConfig == nullptr)
    {
        Item.SetItemEnabled(false);
    }
    OptionMenu.push_back(Item);
    Item.Reset(ID_OPTIONS_CONFIG_AUDIO, MENU_CONFG_AUDIO, m_ShortCuts.ShortCutString(ID_OPTIONS_CONFIG_AUDIO, RunningState));
    if (g_Plugins->Audio() == nullptr || g_Plugins->Audio()->DllConfig == nullptr)
    {
        Item.SetItemEnabled(false);
    }
    OptionMenu.push_back(Item);
    if (!inBasicMode)
    {
        Item.Reset(ID_OPTIONS_CONFIG_RSP, MENU_CONFG_RSP, m_ShortCuts.ShortCutString(ID_OPTIONS_CONFIG_RSP, RunningState));
        if (g_Plugins->RSP() == nullptr || g_Plugins->RSP()->DllConfig == nullptr)
        {
            Item.SetItemEnabled(false);
        }
        OptionMenu.push_back(Item);
    }
    Item.Reset(ID_OPTIONS_CONFIG_CONT, MENU_CONFG_CTRL, m_ShortCuts.ShortCutString(ID_OPTIONS_CONFIG_CONT, RunningState));
    if (g_Plugins && g_Plugins->Control() == nullptr || g_Plugins->Control()->DllConfig == nullptr)
    {
        Item.SetItemEnabled(false);
    }
    OptionMenu.push_back(Item);

    OptionMenu.push_back(MENU_ITEM(SPLITER));
    if (!inBasicMode)
    {
        Item.Reset(ID_OPTIONS_CPU_USAGE, MENU_SHOW_CPU, m_ShortCuts.ShortCutString(ID_OPTIONS_CPU_USAGE, RunningState));
        if (g_Settings->LoadDword(UserInterface_ShowCPUPer)) { Item.SetItemTicked(true); }
        OptionMenu.push_back(Item);
    }
    OptionMenu.push_back(MENU_ITEM(ID_OPTIONS_SETTINGS, MENU_SETTINGS, m_ShortCuts.ShortCutString(ID_OPTIONS_SETTINGS, RunningState)));

    // Profile menu
    MenuItemList DebugProfileMenu;
    if (HaveDebugger())
    {
        Item.Reset(ID_PROFILE_PROFILE, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Record Execution Times");
        if (g_Settings->LoadBool(Debugger_RecordExecutionTimes)) { Item.SetItemTicked(true); }
        DebugProfileMenu.push_back(Item);
        Item.Reset(ID_PROFILE_RESETCOUNTER, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Reset Counters");
        if (!CPURunning) { Item.SetItemEnabled(false); }
        DebugProfileMenu.push_back(Item);
        Item.Reset(ID_PROFILE_GENERATELOG, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Generate Log File");
        if (!CPURunning) { Item.SetItemEnabled(false); }
        DebugProfileMenu.push_back(Item);
    }

    // Debugger menu
    MenuItemList DebugMenu;
    MenuItemList DebugLoggingMenu;
    MenuItemList DebugAppLoggingMenu;
    MenuItemList DebugR4300Menu;
    MenuItemList DebugMemoryMenu;
    MenuItemList DebugInterrupt;
    MenuItemList DebugNotificationMenu;
    if (HaveDebugger())
    {
        // Debug - interrupt
        Item.Reset(ID_DEBUGGER_INTERRUPT_SP, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"SP interrupt");
        Item.SetItemEnabled(CPURunning);
        DebugInterrupt.push_back(Item);
        Item.Reset(ID_DEBUGGER_INTERRUPT_SI, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"SI interrupt");
        Item.SetItemEnabled(CPURunning);
        DebugInterrupt.push_back(Item);
        Item.Reset(ID_DEBUGGER_INTERRUPT_AI, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"AI interrupt");
        Item.SetItemEnabled(CPURunning);
        DebugInterrupt.push_back(Item);
        Item.Reset(ID_DEBUGGER_INTERRUPT_VI, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"VI interrupt");
        Item.SetItemEnabled(CPURunning);
        DebugInterrupt.push_back(Item);
        Item.Reset(ID_DEBUGGER_INTERRUPT_PI, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"PI interrupt");
        Item.SetItemEnabled(CPURunning);
        DebugInterrupt.push_back(Item);
        Item.Reset(ID_DEBUGGER_INTERRUPT_DP, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"DP interrupt");
        Item.SetItemEnabled(CPURunning);
        DebugInterrupt.push_back(Item);

        // Debug - R4300i

        // ID_DEBUGGER_LOGOPTIONS
        Item.Reset(ID_DEBUGGER_BREAKPOINTS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"&Commands...");
        DebugR4300Menu.push_back(Item);
        Item.Reset(ID_DEBUGGER_CPULOG, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Command log...");
        DebugR4300Menu.push_back(Item);
        Item.Reset(ID_DEBUGGER_EXCBREAKPOINTS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Exceptions...");
        DebugR4300Menu.push_back(Item);
        Item.Reset(ID_DEBUGGER_STACKVIEW, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Stack...");
        DebugR4300Menu.push_back(Item);
        Item.Reset(ID_DEBUGGER_STACKTRACE, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Stack lrace...");
        DebugR4300Menu.push_back(Item);

        Item.Reset(ID_DEBUG_DISABLE_GAMEFIX, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Disable game fixes");
        if (g_Settings->LoadBool(Debugger_DisableGameFixes))
        {
            Item.SetItemTicked(true);
        }
        DebugR4300Menu.push_back(Item);
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugInterrupt, L"&Generate interrupt");
        DebugR4300Menu.push_back(Item);

        // Debug - memory
        Item.Reset(ID_DEBUGGER_MEMORY, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"View...");
        DebugMemoryMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_SEARCHMEMORY, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Search...");
        DebugMemoryMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_SYMBOLS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Symbols...");
        DebugMemoryMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_DUMPMEMORY, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Dump...");
        DebugMemoryMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_TLBENTRIES, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"TLB entries...");
        DebugMemoryMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_DMALOG, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"DMA log...");
        DebugMemoryMenu.push_back(Item);

        // Debug - app logging
        Item.Reset(ID_DEBUGGER_TRACE_MD5, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"MD5");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceMD5) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_SETTINGS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Settings");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceSettings) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_UNKNOWN, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Unknown");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceUnknown) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_APPINIT, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"App initialization");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceAppInit) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_APPCLEANUP, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"App cleanup");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceAppCleanup) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_N64SYSTEM, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"N64 system");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceN64System) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_PLUGINS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Plugins");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TracePlugins) == TraceVerbose);;
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_GFXPLUGIN, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"GFX plugin");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceGFXPlugin) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_AUDIOPLUGIN, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Audio plugin");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceAudioPlugin) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_CONTROLLERPLUGIN, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Controller plugin");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceControllerPlugin) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_RSPPLUGIN, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"RSP plugin");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceRSPPlugin) == TraceVerbose);;
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_RSP, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"RSP");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceRSP) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_AUDIO, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Audio");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceAudio) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_REGISTERCACHE, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Register cache");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceRegisterCache) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_RECOMPILER, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Recompiler");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceRecompiler) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_TLB, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"TLB");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceTLB) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_PROTECTEDMEM, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Protected MEM");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceProtectedMEM) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_TRACE_USERINTERFACE, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"User interface");
        Item.SetItemTicked(g_Settings->LoadDword(Debugger_TraceUserInterface) == TraceVerbose);
        DebugAppLoggingMenu.push_back(Item);

        DebugAppLoggingMenu.push_back(MENU_ITEM(SPLITER));

        Item.Reset(ID_DEBUGGER_APPLOG_FLUSH, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Auto flush file");
        if (g_Settings->LoadBool(Debugger_AppLogFlush)) { Item.SetItemTicked(true); }
        DebugAppLoggingMenu.push_back(Item);

        // Debug - logging
        Item.Reset(ID_DEBUGGER_LOGOPTIONS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Log options...");
        DebugLoggingMenu.push_back(Item);

        Item.Reset(ID_DEBUGGER_GENERATELOG, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Generate log");
        if (g_Settings->LoadBool(Logging_GenerateLog)) { Item.SetItemTicked(true); }
        DebugLoggingMenu.push_back(Item);

        // Debugger main menu
        Item.Reset(ID_DEBUGGER_BREAKPOINTS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Commands...");
        DebugMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_MEMORY, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"View memory...");
        DebugMenu.push_back(Item);
        Item.Reset(ID_DEBUGGER_SCRIPTS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Scripts...");
        DebugMenu.push_back(Item);

        DebugMenu.push_back(MENU_ITEM(SPLITER));

        // Debug - memory
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugMemoryMenu, L"Memory");
        DebugMenu.push_back(Item);

        // Debug - R4300i
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugR4300Menu, L"&R4300i");
        DebugMenu.push_back(Item);

        // Debug - RSP
        if (g_Plugins && g_Plugins->RSP() != nullptr && IsMenu((HMENU)g_Plugins->RSP()->GetDebugMenu()))
        {
            Item.Reset(ID_PLUGIN_MENU, EMPTY_STRING, EMPTY_STDSTR, g_Plugins->RSP()->GetDebugMenu(), L"&RSP");
            DebugMenu.push_back(Item);
        }

        // Debug - RDP
        if (g_Plugins && g_Plugins->Gfx() != nullptr && IsMenu((HMENU)g_Plugins->Gfx()->GetDebugMenu()))
        {
            Item.Reset(ID_PLUGIN_MENU, EMPTY_STRING, EMPTY_STDSTR, g_Plugins->Gfx()->GetDebugMenu(), L"&RDP");
            DebugMenu.push_back(Item);
        }

        // Notification menu
        Item.Reset(ID_DEBUG_SHOW_UNHANDLED_MEM, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"On unhandled memory actions");
        if (g_Settings->LoadBool(Debugger_ShowUnhandledMemory))
        {
            Item.SetItemTicked(true);
        }
        DebugNotificationMenu.push_back(Item);
        Item.Reset(ID_DEBUG_SHOW_PIF_ERRORS, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"On PIF errors");
        if (g_Settings->LoadBool(Debugger_ShowPifErrors))
        {
            Item.SetItemTicked(true);
        }
        DebugNotificationMenu.push_back(Item);
        Item.Reset(ID_DEBUG_SHOW_DIV_BY_ZERO, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"On division by zero errors");
        if (g_Settings->LoadBool(Debugger_ShowDivByZero))
        {
            Item.SetItemTicked(true);
        }
        DebugNotificationMenu.push_back(Item);

        DebugMenu.push_back(MENU_ITEM(SPLITER));
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugProfileMenu, L"Profile");
        DebugMenu.push_back(Item);
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugAppLoggingMenu, L"App logging");
        DebugMenu.push_back(Item);
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugLoggingMenu, L"Logging");
        DebugMenu.push_back(Item);
        Item.Reset(SUB_MENU, EMPTY_STRING, EMPTY_STDSTR, &DebugNotificationMenu, L"Notification");
        DebugMenu.push_back(Item);
        DebugMenu.push_back(MENU_ITEM(SPLITER));
        Item.Reset(ID_DEBUG_SHOW_TLB_MISSES, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Show TLB misses");
        if (g_Settings->LoadBool(Debugger_ShowTLBMisses))
        {
            Item.SetItemTicked(true);
        }
        DebugMenu.push_back(Item);
        Item.Reset(ID_DEBUG_SHOW_DLIST_COUNT, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Display Alist/Dlist count");
        if (g_Settings->LoadBool(Debugger_ShowDListAListCount))
        {
            Item.SetItemTicked(true);
        }
        DebugMenu.push_back(Item);
        Item.Reset(ID_DEBUG_LANGUAGE, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Debug language");
        if (g_Settings->LoadBool(Debugger_DebugLanguage))
        {
            Item.SetItemTicked(true);
        }
        DebugMenu.push_back(Item);
        Item.Reset(ID_DEBUG_SHOW_RECOMP_MEM_SIZE, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Display recompiler code buffer size");
        if (g_Settings->LoadBool(Debugger_ShowRecompMemSize))
        {
            Item.SetItemTicked(true);
        }
        DebugMenu.push_back(Item);
        DebugMenu.push_back(MENU_ITEM(SPLITER));
        Item.Reset(ID_DEBUG_RECORD_RECOMPILER_ASM, EMPTY_STRING, EMPTY_STDSTR, nullptr, L"Record recompiler ASM");
        if (g_Settings->LoadBool(Debugger_RecordRecompilerAsm))
        {
            Item.SetItemTicked(true);
        }
        DebugMenu.push_back(Item);
    }

    // Help menu
    MenuItemList HelpMenu;
    HelpMenu.push_back(MENU_ITEM(ID_HELP_MPN_DISCORD, MENU_MPN_DISCORD));
    HelpMenu.push_back(MENU_ITEM(ID_HELP_DISCORD, MENU_DISCORD));
    HelpMenu.push_back(MENU_ITEM(ID_HELP_WEBSITE, MENU_WEBSITE));
    HelpMenu.push_back(MENU_ITEM(SPLITER));
    HelpMenu.push_back(MENU_ITEM(ID_HELP_ABOUT, MENU_ABOUT_PJ64));

    // Main title bar Menu
    MenuItemList MainTitleMenu;
    Item.Reset(SUB_MENU, MENU_FILE, EMPTY_STDSTR, &FileMenu);
    if (RomLoading) { Item.SetItemEnabled(false); }
    MainTitleMenu.push_back(Item);
    if (CPURunning)
    {
        Item.Reset(SUB_MENU, MENU_SYSTEM, EMPTY_STDSTR, &SystemMenu);
        if (RomLoading) { Item.SetItemEnabled(false); }
        MainTitleMenu.push_back(Item);
    }
    Item.Reset(SUB_MENU, MENU_OPTIONS, EMPTY_STDSTR, &OptionMenu);
    if (RomLoading) { Item.SetItemEnabled(false); }
    MainTitleMenu.push_back(Item);
    if (!inBasicMode)
    {
        if (HaveDebugger())
        {
            Item.Reset(SUB_MENU, MENU_DEBUGGER, EMPTY_STDSTR, &DebugMenu);
            if (RomLoading) { Item.SetItemEnabled(false); }
            MainTitleMenu.push_back(Item);
        }
    }
    Item.Reset(SUB_MENU, MENU_HELP, EMPTY_STDSTR, &HelpMenu);
    if (RomLoading) { Item.SetItemEnabled(false); }
    MainTitleMenu.push_back(Item);

    if (UISettingsLoadBool(UserInterface_ShowingNagWindow))
    {
        for (MenuItemList::iterator MenuItem = MainTitleMenu.begin(); MenuItem != MainTitleMenu.end(); MenuItem++)
        {
            MenuItem->SetItemEnabled(false);
        }
    }
    AddMenu(hMenu, MainTitleMenu);
}

void CMainMenu::RebuildAccelerators(void)
{
    CGuard Guard(m_CS);

    // Delete the old accel list
    WriteTrace(TraceUserInterface, TraceDebug, "Start");

    HACCEL m_OldAccelTable = (HACCEL)m_AccelTable;
    m_AccelTable = m_ShortCuts.GetAcceleratorTable();
    if (m_OldAccelTable)
    {
        DestroyAcceleratorTable(m_OldAccelTable);
    }
    WriteTrace(TraceUserInterface, TraceDebug, "Done");
}

void CMainMenu::ResetMenu(void)
{
    WriteTrace(TraceUserInterface, TraceDebug, "Start");
    if (!UISettingsLoadBool(UserInterface_InFullScreen))
    {
        // Create a new window with all the items
        WriteTrace(TraceUserInterface, TraceDebug, "Create menu");
        HMENU hMenu = CreateMenu();
        FillOutMenu(hMenu);
        WriteTrace(TraceUserInterface, TraceDebug, "Creating menu done");

        // Save old menu to destroy latter
        HMENU OldMenuHandle;
        {
            CGuard Guard(m_CS);
            OldMenuHandle = m_MenuHandle;

            // Save handle and re-attach to a window
            WriteTrace(TraceUserInterface, TraceDebug, "Attach menu");
            m_MenuHandle = hMenu;
        }
        m_Gui->SetWindowMenu(this);

        WriteTrace(TraceUserInterface, TraceDebug, "Remove plugin menu");
        if (g_Plugins->Gfx() != nullptr && IsMenu((HMENU)g_Plugins->Gfx()->GetDebugMenu()))
        {
            RemoveMenu((HMENU)OldMenuHandle, (DWORD)g_Plugins->Gfx()->GetDebugMenu(), MF_BYCOMMAND);
        }
        if (g_Plugins->RSP() != nullptr && IsMenu((HMENU)g_Plugins->RSP()->GetDebugMenu()))
        {
            RemoveMenu((HMENU)OldMenuHandle, (DWORD)g_Plugins->RSP()->GetDebugMenu(), MF_BYCOMMAND);
        }
        WriteTrace(TraceUserInterface, TraceDebug, "Destroy old menu");

        // Destroy the old menu
        DestroyMenu((HMENU)OldMenuHandle);
    }

    ResetAccelerators();

    WriteTrace(TraceUserInterface, TraceDebug, "Done");
}
