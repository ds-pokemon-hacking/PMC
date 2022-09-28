#ifndef __PMC_RPMFRAMEWORK_H
#define __PMC_RPMFRAMEWORK_H

#include "Heap/exl_HeapArea.h"

#include "RPM_Api.h"
#include "PMC_NTRExternalRelocator.h"
#include "PMC_Common.h"

#include "data/heapid_def.h"

#define PMC_MODULEHEAP_SIZE 0x10000
#define PMC_USERHEAP_SIZE 0x1000

#define PMC_MODULEPRIORITY_METAVALUE "PMCModulePriority"

namespace pmc {
	namespace fwk {
		class CacheFlushModuleListener : public rpm::mgr::ModuleListener {
			virtual void OnEvent(rpm::mgr::ModuleManager* mgr, rpm::Module* module, rpm::mgr::ModuleEvent event) override;
		};

		extern void*					g_ModuleMemory;
		extern void*					g_UserMemory;
		extern exl::heap::HeapArea* 	g_ModuleHeapArea;
		extern exl::heap::HeapArea* 	g_UserHeapArea;

		extern rpm::mgr::ModuleManager* 	g_ModuleManager;
		extern NTRExternalRelocator*		g_ExternRelocator;
		extern CacheFlushModuleListener* 	g_CacheFlusher;

		void Initialize();

		void Terminate();

		INLINE rpm::mgr::ModuleManager* GetModuleManager() {
			return g_ModuleManager;
		}

		rpm::init::ModuleAllocation AllocModuleMemory(size_t size);
		void FreeModuleMemory(PMCModuleHandle p);

		void* AllocUserMemory(size_t size);
		void FreeUserMemory(void* p);

		PMCModuleHandle LoadModule(void* data);

		void UnloadModule(PMCModuleHandle handle);

		void StartModule(PMCModuleHandle handle);

		PMCModulePriority GetPMCModulePriority(PMCModuleHandle handle);

		void LinkModuleExtern(PMCModuleHandle handle, const char* externModule);

		PMCExternModuleList* GetExternModuleNames(PMCModuleHandle handle);

		void FreeExternModuleNames(PMCExternModuleList* list);
	}
}

#endif