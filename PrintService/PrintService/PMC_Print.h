#ifndef __PMC_PRINT_H
#define __PMC_PRINT_H

namespace pmc {
    namespace debug {
        void InitPrinter();

        void AttachToNK(void* nkPrinterSetFunc);

        void Print(const char* str);

        void Printf(const char* fmt, ...);
    }
}

#endif