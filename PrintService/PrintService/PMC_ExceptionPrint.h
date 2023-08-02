#ifndef __PMC_EXCEPTIONPRINT_H
#define __PMC_EXCEPTIONPRINT_H

namespace pmc {
    namespace debug {
        void ExceptionPrintInit();
        void PrintException(int* regDump, void* data);
    }
}

#endif