#ifndef FLASH_BLOCK_MANAGER_H
#define FLASH_BLOCK_MANAGER_H

#include <list>

#include "Flash_Block_Manager_Base.h"
#include "../nvm_chip/flash_memory/FlashTypes.h"
#include "../nvm_chip/flash_memory/Physical_Page_Address.h"

namespace SSD_Components
{
	class Flash_Block_Manager : public Flash_Block_Manager_Base
	{
	public:
		Flash_Block_Manager(GC_and_WL_Unit_Base* gc_and_wl_unit,
				unsigned int max_allowed_block_erase_count,
				unsigned int total_concurrent_streams_no,
				unsigned int channel_count,
				unsigned int chip_no_per_channel,
				unsigned int die_no_per_chip,
				unsigned int plane_no_per_die,
				unsigned int block_no_per_plane,
				unsigned int page_no_per_block);
		~Flash_Block_Manager();
		void AllocateBlockAndPageInPlaneForUserWrite(const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& address);
		void AllocateBlockAndPageInPlaneForGCWrite(const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& address);
		void AllocatePagesInBlockAndInvalidateRemainingForPreconditioning(const stream_id_type stream_id, const NVM::FlashMemory::Physical_Page_Address& plane_address, std::vector<NVM::FlashMemory::Physical_Page_Address>& page_addresses);
		void AllocateBlockAndPageIinPlaneForTranslationWrite(const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& address, bool is_for_gc);
		void InvalidatePageInBlock(const stream_id_type streamID, const NVM::FlashMemory::Physical_Page_Address& address);
		void InvalidatePageInBlockForPreconditioning(const stream_id_type streamID, const NVM::FlashMemory::Physical_Page_Address& address);
		void AddErasedBlockToPool(const NVM::FlashMemory::Physical_Page_Address& address);
		unsigned int GetPoolSize(const NVM::FlashMemory::Physical_Page_Address& plane_address);
	private:
	};
}

#endif // !FLASH_BLOCK_MANAGER_H
