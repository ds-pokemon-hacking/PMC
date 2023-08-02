#ifndef __PMC_RPMFRAMEWORK_CPP
#define __PMC_RPMFRAMEWORK_CPP

#include "Heap/exl_HeapArea.h"

#include "RPM_Api.h"
#include "PMC_RPMFramework.h"
#include "PMC_Common.h"
#include "PMC_NTRExternalRelocator.h"

#define CAST_HANDLE_TO_MODULE(handle) static_cast<rpm::Module*>((handle))

#include "nds/cp15.h"

namespace pmc {
	namespace fwk {
		exl::heap::Allocator* 	g_MainAllocator;
		exl::heap::HeapArea* 	g_UserHeapArea;

		rpm::mgr::ModuleManager* 	g_ModuleManager;
		NTRExternalRelocator*		g_ExternRelocator;
		CacheFlushModuleListener* 	g_CacheFlusher;

		void Initialize(exl::heap::Allocator* allocator) {
			g_MainAllocator = allocator;
			g_UserHeapArea = exl::heap::HeapArea::CreateFrom(g_MainAllocator, "UserMemory", PMC_USERHEAP_SIZE);
			g_ModuleManager = new(g_UserHeapArea) rpm::mgr::ModuleManager(g_MainAllocator);
			g_ExternRelocator = new(g_UserHeapArea) pmc::fwk::NTRExternalRelocator();
			g_CacheFlusher = new(g_UserHeapArea) pmc::fwk::CacheFlushModuleListener();
			g_ModuleManager->BindExternalRelocator(g_ExternRelocator);
			g_ModuleManager->BindModuleListener(g_CacheFlusher);
		}

		void Terminate() {
			delete g_ModuleManager;
			delete g_ExternRelocator;
			delete g_UserHeapArea;
		}

		rpm::init::ModuleAllocation AllocModuleMemory(size_t size) {
			return GetModuleManager()->AllocModule(size);
		}

		void FreeModuleMemory(PMCModuleHandle p) {
			return GetModuleManager()->FreeModule(CAST_HANDLE_TO_MODULE(p));
		}

		void* AllocUserMemory(size_t size) {
			return g_UserHeapArea->Alloc(size);
		}

		void FreeUserMemory(void* p) {
			g_UserHeapArea->Free(p);
		}

		PMCModuleHandle LoadModule(void* data) {
			return GetModuleManager()->LoadModule(static_cast<rpm::init::ModuleAllocation>(data));
		}

		void UnloadModule(PMCModuleHandle handle) {
			GetModuleManager()->UnloadModule(CAST_HANDLE_TO_MODULE(handle));
		}

		void StartModule(PMCModuleHandle handle) {
			rpm::mgr::ModuleManager* mgr = GetModuleManager();
			rpm::Module* mod = CAST_HANDLE_TO_MODULE(handle);
			mgr->StartModule(mod, rpm::FixLevel::INTERNAL_RELOCATIONS);
		}

		void* GetProcAddress(PMCModuleHandle handle, const char* name) {
			return CAST_HANDLE_TO_MODULE(handle)->GetProcAddress(name);
		}

		PMCModulePriority GetPMCModulePriority(PMCModuleHandle handle) {
			rpm::Module* mod = CAST_HANDLE_TO_MODULE(handle);
			rpm::MetaData* metaData = mod->GetMetaData();
			if (metaData) {
				return (PMCModulePriority) metaData->GetInt(mod, PMC_MODULEPRIORITY_METAVALUE, PMCModulePriority::PMC_PATCH);
			}
			else {
				return PMCModulePriority::PMC_PATCH;
			}
		}

		void LinkModuleExtern(PMCModuleHandle handle, const char* externModule) {
			GetModuleManager()->LinkModuleExtern(CAST_HANDLE_TO_MODULE(handle), externModule);
		}

		PMCExternModuleList* GetExternModuleNames(PMCModuleHandle handle) {
			rpm::Module* m = CAST_HANDLE_TO_MODULE(handle);
			u16 count = m->GetRelExternModuleCount();

			PMCExternModuleList* list = static_cast<PMCExternModuleList*>(AllocUserMemory(count * sizeof(char*) + sizeof(PMCExternModuleList))); //can not use new because of dynamic size
			list->Count = count;
			for (u16 i = 0; i < count; i++) {
				list->Names[i] = m->GetRelExternModuleName(i);
			}

			return list;
		}

		void FreeExternModuleNames(PMCExternModuleList* list) {
			FreeUserMemory(list);
		}

		void CacheFlushModuleListener::OnEvent(rpm::mgr::ModuleManager* mgr, rpm::Module* module, rpm::mgr::ModuleEvent event) {
			if (event == rpm::mgr::EXEC_UPDATED) {
				//loading, relocations, imports
				cp15_flushDC(module, module->GetModuleSize());
			}
		}
	}
}

#endif