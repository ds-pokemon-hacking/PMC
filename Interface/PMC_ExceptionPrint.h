#ifndef __PMC_EXCEPTIONPRINT_H
#define __PMC_EXCEPTIONPRINT_H

namespace pmc {
    void ExceptionPrintInit();
    void PrintException(int* regDump, void* data);
}

#endif