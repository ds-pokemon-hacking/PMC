/**
 * @file PMCCppInterface.h
 * @author Hello007
 * @brief PMC executable manager and code hooks.
 * @version 0.1
 * @date 2022-09-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __PMCCPPINTERFACE_H
#define __PMCCPPINTERFACE_H

#include "swantypes.h"
#include "nds/fs.h"
#include "nds/overlay.h"
#include "PMC_Common.h"

#define MODULENAME_ARM9 "ARM9"
#define MODULENAME_ARM7 "ARM7"

#define OVLID_NULL_RESERVE 0xFFFFFFFF
#define OVLID_ARM9_RESERVE 0xFFFE
#define OVLID_ARM7_RESERVE 0xFFFD

#define NK_PRINTER_INJECT_FUNC "kPrintSetSystemPrinter"

namespace pmc {
    /**
     * @brief List of overlay IDs which a module hooks into.
     */
    struct PMCOverlayList {
        int Count;
        int IDs[];
    };

    /**
     * @brief Information about a patch module.
     */
    class ModuleState {
    private:
        /**
         * @brief Internal RomFS ID of the executable file.
         */
        nn::fs::FileID      m_FileID;
        
        /**
         * @brief Handle to the executable in memory (if loaded).
         */
        PMCModuleHandle     m_Module;
        /**
         * @brief Concurrent loading priority.
         */
        PMCModulePriority   m_Priority;
        /**
         * @brief True if StartModule was called on this module.
         */
        bool                m_IsModuleStarted;
        
        /**
         * @brief List of extern modules (ARM9, overlays) linked to this module.
         */
        PMCExternModuleList*    m_ExternList;
        /**
         * @brief m_ExternList cached as overlay ID integers.
         */
        PMCOverlayList*         m_OverlayList;

        /**
         * @brief Next node in the module chain.
         */
        ModuleState*  m_NextNode;
        /**
         * @brief Previous node in the module chain.
         */
        ModuleState*  m_PrevNode;

        /**
         * @brief Constructs a ModuleState from an executable file.
         * 
         * @param fileId ID of the executable in the RomFS.
         */
        ModuleState(nn::fs::FileID fileId);
        /**
         * @brief Safely deletes a ModuleState and its fields.
         */
        ~ModuleState();

        /**
         * @brief Reads the executable file into memory.
         */
        void LoadExecutable();
        /**
         * @brief Frees the executable data from memory.
         */
        void UnloadExecutable();
        /**
         * @brief Links and starts the loaded executable.
         */
        void Start();
        /**
         * @brief Ensures that this module is loaded and started.
         */
        void EnsureLoadAndStart();
        /**
         * @brief Constructs the m_OverlayList cache.
         */
        void PrefetchOverlays();
        /**
         * @brief Fetches the m_ExternList.
         */
        void LinkModuleNames();

        /**
         * @brief Checks if a module can be considered "resident" (always loaded).
         * 
         * @return true if this module is either standalone or links to a static executable.
         * @return false if this module only links to overlays.
         */
        bool IsResident();

        friend class System;
    };

    /**
     * @brief Tail of the module chain linked list.
     */
    static ModuleState* g_ModulesTail = nullptr;

    class System {    
    public:
        /**
         * @brief Initializes PMC.
         */
        static void Init();
        /**
         * @brief Frees and terminates PMC.
         */
        static void Terminate();

        /**
         * @brief Detects RPM patches stored in RomFS.
         */
        static void LoadPatchRPMs();

        /**
         * @brief Registers a patch module into the module chain.
         * 
         * @param module The module to register.
         */
        static void RegistModule(ModuleState* module);

        /**
         * @brief Loads an NDS overlay and prepares its patches.
         * 
         * @param target Overlay table selection (ARM9/ARM7).
         * @param ovlId Index of the overlay in the overlay table.
         * @return b32 True if the operation finished successfully.
         */
        static b32 LoadOverlay(int target, u32 ovlId);
        /**
         * @brief Starts an NDS overlay and applies its patches.
         * 
         * @param ovl Handle to the overlay to start.
         */
        static void StartOverlay(nn::os::Overlay& ovl);

        /**
         * @brief Applies all patches that refer to an overlay.
         * 
         * @param ovlId ID of the overlay to query.
         */
        static void LinkOverlay(u32 ovlId);
        /**
         * @brief Notifies the system that an overlay has been unloaded in order to free redundant patches.
         * 
         * @param ovlId ID of the unloaded overlay.
         */
        static void NotifyUnloadOverlay(u32 ovlId);
    };
}

#endif