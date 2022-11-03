
#include "PMC_Interface.h"

#include "PMC_ExceptionPrint.h"
#include "PMC_RPMFramework.h"
#include "PMC_Common.h"
#include "nds/fs.h"
#include "nds/cp15.h"
#include "nds/overlay.h"
#include "nds/hw.h"
#include "nds/compression.h"
#include "gfl/core/gfl_overlay.h"
#include "Util/exl_StrEq.h"
#include <cstdlib>

namespace pmc {
    namespace hooks {
        extern "C" b32 THUMB_BRANCH_LINK_GFL_OvlLoad_0x76(int target, int ovlId) { //hooks into GFL_OvlLoad's LoadOverlay call
            System::LoadOverlay(target, ovlId);
            return true;
        }

        extern "C" void THUMB_BRANCH_LINK_GFL_OvlEntryUnload_0xA(int target, int ovlId) { //hooks into GFL_OvlEntryUnload's UnloadOverlay call
            nn::os::Overlay::Unload(target, ovlId);
            //Check for no longer needed modules
            System::NotifyUnloadOverlay(ovlId);
        }
    }

    ModuleState::ModuleState(nn::fs::FileID fileId) {
        m_FileID = fileId;
        m_Module = nullptr;
        m_IsModuleStarted = false;
        m_NextNode = nullptr;
        m_PrevNode = nullptr;

        LoadExecutable();
        m_Priority = fwk::GetPMCModulePriority(m_Module);
        PrefetchOverlays();

        if (!IsResident()) {
            UnloadExecutable();
        }
    }

    ModuleState::~ModuleState() {
        fwk::UnloadModule(m_Module);
        delete m_OverlayList;
        if (m_ExternList) {
            delete m_ExternList;
        }
    }

    char *ReadModule(nn::fs::FileID fileId) {
        nn::fs::File file;
        rpm::Module moduleHeader;

        file.Init();
        file.OpenID(fileId);
        file.Read(&moduleHeader, sizeof(rpm::Module));
        
        char *buffer = new(fwk::g_ModuleHeapArea) char[moduleHeader.GetModuleSize()];
        file.Seek(0, nn::fs::SeekOrigin::IO_SEEK_SET);
        file.Read(buffer, file.GetSize());
        file.Close();
        
        return buffer;
    }

    void ModuleState::LoadExecutable() {
        char *buffer = ReadModule(m_FileID);
        m_Module = fwk::LoadModule(buffer);
        LinkModuleNames();
    }

    void ModuleState::UnloadExecutable() {
        if (m_Module) {
            fwk::UnloadModule(m_Module);
            m_IsModuleStarted = false;
            m_Module = NULL;
            if (m_ExternList) {
                delete m_ExternList;
            }
        }
    }

    void ModuleState::Start() {
        fwk::StartModule(m_Module);
    }

    void ModuleState::EnsureLoadAndStart() {
        if (!m_Module) {
            LoadExecutable();
        }
        if (!m_IsModuleStarted) {
            Start();
            m_IsModuleStarted = true;
        }
    }

    void ModuleState::LinkModuleNames() {
        if (!m_Module) {
            return; //ERROR
        }
        
        m_ExternList = fwk::GetExternModuleNames(m_Module);
    }

    void ModuleState::PrefetchOverlays() {
        if (!m_Module) {
            return; //ERROR
        }
        if (!m_ExternList) {
            return;
        }

        PMCExternModuleList* list = m_ExternList;

        m_OverlayList = new(fwk::g_UserHeapArea->Alloc(list->Count * sizeof(int) + sizeof(PMCOverlayList))) PMCOverlayList();
        m_OverlayList->Count = list->Count;

        for (u32 externIdx = 0; externIdx < list->Count; externIdx++) {
            b32 isOverlay = true;
            int ovlId = 0xFFFF;
            
            const char *externName = list->Names[externIdx];
            if (externName) {
                int nameLen = strlen(externName);
                
                for (int i = 0; i < nameLen; i++) {
                    if (externName[i] < '0' || externName[i] > '9') {
                        isOverlay = false;
                        break;
                    }
                }

                if (isOverlay) {
                    ovlId = strtoul(externName, 0, 10);
                }
                else if (strequal(externName, MODULENAME_ARM9)) {
                    ovlId = OVLID_ARM9_RESERVE;
                }
                else if (strequal(externName, MODULENAME_ARM7)) {
                    ovlId = OVLID_ARM7_RESERVE;
                }
            }

            m_OverlayList->IDs[externIdx] = ovlId;
        }
    }

    b32 IsOvlIDResident(int ovlId) {
        return ovlId == OVLID_ARM9_RESERVE || ovlId == OVLID_ARM7_RESERVE;
    }

    bool ModuleState::IsResident() {
        int count = m_OverlayList->Count;
        if (count == 0) {
            return true; //can not determine required segments - assume static library
        }
        for (int i = 0; i < count; i++) {
            if (IsOvlIDResident(m_OverlayList->IDs[i])) {
                return true;
            }
        }
        return false;
    }

    void System::Init() {
        ExceptionPrintInit();
        g_ModulesTail = nullptr;
        fwk::Initialize();
        LoadPatchRPMs();
        LinkOverlay(OVLID_NULL_RESERVE);
        LinkOverlay(OVLID_ARM7_RESERVE);
        LinkOverlay(OVLID_ARM9_RESERVE);
    }

    extern "C" void _PMCSystemInit() {
        System::Init();
    }

    void System::Terminate() {
        fwk::Terminate();
    }

    b32 IsRPM(nn::fs::FileID fileId) {
        nn::fs::File file;

        file.Init();
        file.OpenID(fileId);

        u32 magic = 0;

        if (file.GetSize() > 0x20) {
            file.Seek(0, IO_SEEK_SET);      
            file.Read(&magic, sizeof(u32));
        }

        file.Close();

        return magic == ('D' | ('L' << 8) | ('X' << 16) | ('F' << 24));
    }

    void System::LoadPatchRPMs() {
        nn::fs::File rootDir;

        rootDir.Init();

        char normPath[260];
        u32 baseId;

        rootDir.m_FileSystem = fs_normalize_path("patches", &baseId, normPath);

        nn::fs::FileOpenRequest openReq;
        openReq.BaseID = baseId;
        openReq.Path = normPath;
        openReq.Mode = 1;

        rootDir.Request = &openReq;

        rootDir.CallSystemCommand(nn::fs::SystemCommand::FS_OPEN_DIR, true);

        FSFileIterDirResult iterDirResult;
        rootDir.IterDir.DontNeedReadName = true;
        rootDir.IterDir.ResultPtr = &iterDirResult;

        while (!rootDir.CallFileCommand(nn::fs::FileCommand::FSF_ITERATE_DIR, true)) {
            if (!iterDirResult.IsDirectory) {
                if (IsRPM(iterDirResult.FileID)) {
                    RegistModule(new(fwk::g_UserHeapArea) ModuleState(iterDirResult.FileID));
                }
            }
        }
    }

    void System::RegistModule(ModuleState* module) {
        module->m_PrevNode = g_ModulesTail;
        module->m_NextNode = nullptr;
        if (g_ModulesTail) {
            g_ModulesTail->m_NextNode = module;
        }
        g_ModulesTail = module;
    }

    b32 System::LoadOverlay(int target, u32 ovlId) {
        nn::os::Overlay overlay;
        if (overlay.LoadHeader(target, ovlId) && overlay.Mount()) {
            StartOverlay(overlay);
            return true;
        }
        return false;
    }

    void System::StartOverlay(nn::os::Overlay& ovl) {
        u32 codeSize = ovl.GetCodeSize();
        int *Flag = (int *)0x214BFA0;

        if (ovl.Header.MountAddress >= (char*)0x2700000 && ovl.Header.MountAddress < (char*)0x276DDE0 && hw_isDSi() && !*Flag) {
            *Flag = 1;
        }

        u32 compFlag = (ovl.Header.Extra & 0xF000000);
        if (compFlag & OVLFLAG_SIGNED) {

        }
        if (compFlag & OVLFLAG_COMPRESSED) {
            // Decompress before continuing.
            sys_uncomp_blz(ovl.Header.MountAddress + codeSize);
        }
        cp15_flushDC(ovl.Header.MountAddress, ovl.Header.MountSize); // Prepare region for loading.
        LinkOverlay(ovl.Header.OvlID);
        for (OvlStaticInitializer *SInit = ovl.Header.StaticInitStart; SInit < ovl.Header.StaticInitEnd; ++SInit) {
            if (*SInit) {
                (*SInit)();
            }
        }
    }

    void System::LinkOverlay(u32 ovlId) {
        ModuleState *m;

        for (int priority = PMC_PRIORITY_BEGIN; priority <= PMC_PRIORITY_END; priority++) {
            m = g_ModulesTail;
            while (m) {
                if (m->m_Priority == priority) {
                    if (ovlId == OVLID_NULL_RESERVE && m->m_OverlayList->Count == 0) {
                        m->EnsureLoadAndStart();
                    }
                    else {
                        for (int index = 0; index < m->m_OverlayList->Count; index++) {
                            if (m->m_OverlayList->IDs[index] == ovlId) {
                                m->EnsureLoadAndStart();

                                fwk::LinkModuleExtern(m->m_Module, m->m_ExternList->Names[index]);
                                break;
                            }
                        }
                    }
                }
                m = m->m_PrevNode;
            }
        }
    }

    b32 CheckOvlLoaded(u32 ovlId) {
        gfl::ovl::Region rgn = gfl::ovl::Manager::GetMemoryRegion(ovlId);
        gfl::ovl::Handle* list = gfl::ovl::Manager::GetOvlList(rgn);
        u8 count = gfl::ovl::Manager::GetOvlCount(rgn);
          
        for (int i = 0; i < count; i++, list++) {
            if (list->Exists && list->OvlID == ovlId) {
                return true;
            }
        }
        return false;
    }

    void System::NotifyUnloadOverlay(u32 unloadedOvlId) {    
        ModuleState* m = g_ModulesTail;

        while (m) {
            if (m->m_Module && !m->IsResident()) {
                b32 existAnyModule = false;

                for (int index = 0; index < m->m_OverlayList->Count; index++) {
                    int ovlId = m->m_OverlayList->IDs[index];

                    if (ovlId == unloadedOvlId) {
                        continue;
                    }

                    if (CheckOvlLoaded(ovlId)) {
                        existAnyModule = true;
                        break;
                    }
                }

                if (!existAnyModule) {
                    m->UnloadExecutable();
                }
            }

            m = m->m_PrevNode;
        }   
    }
}