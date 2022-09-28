#ifndef __PMC_NTREXTERNALRELOCATOR_H
#define __PMC_NTREXTERNALRELOCATOR_H

#include "RPM_Types.h"
#include "RPM_Module.h"
#include "RPM_Control.h"
#include "RPM_ExternalRelocator.h"
#include "RPM_Util.h"

namespace pmc {
	namespace fwk {
		class NTRExternalRelocator : public rpm::mgr::ExternalRelocator {
			public:
				void ProcessRelocation(rpm::Module* module, rpm::Relocation* rel) {
					u32 addr = rel->Target.Offset;
					rpm::Util::CutAlign16(&addr);

					u8* code = reinterpret_cast<u8*>(addr); //absolute memory address

					rpm::Util::DoRelocation(code, module, rel);
				}
		};
	}
}

#endif