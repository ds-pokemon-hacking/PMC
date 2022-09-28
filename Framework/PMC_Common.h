#ifndef __PMC_COMMON_H
#define __PMC_COMMON_H

#include "RPM_Types.h"

typedef void* PMCModuleHandle;

#ifndef __cplusplus
typedef enum PMCModulePriority PMCModulePriority;
typedef struct PMCExternModuleList PMCExternModuleList;
#endif

enum PMCModulePriority {
	PMC_SYSTEM_CRUCIAL = 0,
	PMC_COMMON_DEPENDENCY_1 = 1,
	PMC_COMMON_DEPENDENCY_2 = 2,
	PMC_COMMON_DEPENDENCY_3 = 3,
	PMC_PATCH = 4,

	PMC_PRIORITY_BEGIN = PMC_SYSTEM_CRUCIAL,
	PMC_PRIORITY_END = PMC_PATCH,
};

struct PMCExternModuleList {
	u32    Count;
	const char*  Names[];
};

#endif