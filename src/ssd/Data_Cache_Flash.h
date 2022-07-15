#ifndef DATA_CACHE_FLASH_H
#define DATA_CACHE_FLASH_H

#include <list>
#include <queue>
#include <unordered_map>

#include "SSD_Defs.h"
#include "Data_Cache_Manager_Base.h"
#include "NVM_Transaction_Flash.h"

#include "../nvm_chip/flash_memory/FlashTypes.h"

namespace SSD_Components
{
	enum class Cache_Slot_Status { EMPTY, CLEAN, DIRTY_NO_FLASH_WRITEBACK, DIRTY_FLASH_WRITEBACK };
	struct Data_Cache_Slot_Type
	{
		unsigned long long State_bitmap_of_existing_sectors;
		LPA_type LPA;
		data_cache_content_type Content;
		data_timestamp_type Timestamp;
		Cache_Slot_Status Status;
		std::list<std::pair<LPA_type, Data_Cache_Slot_Type*>>::iterator lru_list_ptr;
		// used for fast implementation of LRU
	};

	enum class Data_Cache_Simulation_Event_Type {
		MEMORY_READ_FOR_CACHE_EVICTION_FINISHED,
		MEMORY_WRITE_FOR_CACHE_FINISHED,
		MEMORY_READ_FOR_USERIO_FINISHED,
		MEMORY_WRITE_FOR_USERIO_FINISHED
	};

	struct Memory_Transfer_Info
	{
		unsigned int Size_in_bytes;
		void* Related_request;
		Data_Cache_Simulation_Event_Type next_event_type;
		stream_id_type Stream_id;
	};

	class Data_Cache_Flash
	{
	public:
		Data_Cache_Flash(unsigned int capacity_in_pages = 0);
		~Data_Cache_Flash();
		bool Exists(const stream_id_type streamID, const LPA_type lpn);
		bool CheckFreeSlotAvailability();
		bool CheckFreeSlotAvailability(unsigned int no_of_slots);
		bool Empty();
		bool Full();
		Data_Cache_Slot_Type GetSlot(const stream_id_type stream_id, const LPA_type lpn);
		Data_Cache_Slot_Type EvictOneDirtySlot();
		Data_Cache_Slot_Type EvictOneSlotLRU();
		void ChangeSlotStatusToWriteback(const stream_id_type stream_id, const LPA_type lpn);
		void RemoveSlot(const stream_id_type stream_id, const LPA_type lpn);
		void InsertReadData(const stream_id_type stream_id,
				const LPA_type lpn,
				const data_cache_content_type content,
				const data_timestamp_type timestamp,
				const page_status_type state_bitmap_of_read_sectors);
		void InsertWriteData(const stream_id_type stream_id,
				const LPA_type lpn,
				const data_cache_content_type content,
				const data_timestamp_type timestamp,
				const page_status_type state_bitmap_of_write_sectors);
		void UpdataData(const stream_id_type stream_id,
				const LPA_type lpn,
				const data_cache_content_type content,
				const data_timestamp_type timestamp,
				const page_status_type state_bitmap_of_write_sectors);
	private:
		std::unordered_map<LPA_type, Data_Cache_Slot_Type*> slots;
		std::list<std::pair<LPA_type, Data_Cache_Slot_Type*>> lru_list;
		unsigned int capacity_in_pages;
	};
}

#endif // !DATA_CACHE_FLASH_H
