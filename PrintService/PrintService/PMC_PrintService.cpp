#include "PMC_Print.h"
#include "PMC_ExceptionPrint.h"

#include "PMC_PrintService.h"

enum DllMainReason {
    DLL_MODULE_LOAD,
    DLL_MODULE_UNLOAD
};

extern "C" void kPrintSetSystemPrinter(void* printer);

void DllMain(void* mgr, void* module, int reason) {
    if (reason == DLL_MODULE_LOAD) {
        pmc::debug::ExceptionPrintInit();
        pmc::debug::InitPrinter();
        if (kPrintSetSystemPrinter) {
            pmc::debug::AttachToNK((void*)kPrintSetSystemPrinter);
        }
    }
}