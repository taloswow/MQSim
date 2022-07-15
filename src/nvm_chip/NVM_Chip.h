#ifndef NVM_CHIP_H
#define NVM_CHIP_H

#include "NVM_Memory_Address.h"

#include "../sim/Sim_Object.h"

namespace NVM
{
	class NVM_Chip : public MQSimEngine::Sim_Object
	{
	public:
		NVM_Chip(const sim_object_id_type& id) : Sim_Object(id) {}
		virtual void ChangeMemoryStatusPreconditioning(const NVM_Memory_Address* address, const void* status_info) = 0;
	};
}

#endif // !NVM_CHIP_H
