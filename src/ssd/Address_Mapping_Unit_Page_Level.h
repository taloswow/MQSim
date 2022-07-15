#ifndef ADDRESS_MAPPING_UNIT_PAGE_LEVEL
#define ADDRESS_MAPPING_UNIT_PAGE_LEVEL

#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include <list>

#include "Address_Mapping_Unit_Base.h"
#include "Flash_Block_Manager_Base.h"
#include "SSD_Defs.h"
#include "NVM_Transaction_Flash_RD.h"
#include "NVM_Transaction_Flash_WR.h"

namespace SSD_Components
{
#define MAKE_TABLE_INDEX(LPN,STREAM)

	enum class CMTEntryStatus {FREE, WAITING, VALID};

	struct GTDEntryType // Entry type for the Global Translation Directory
	{
		MPPN_type MPPN;
		data_timestamp_type TimeStamp;
	};

	struct CMTSlotType
	{
		PPA_type PPA;
		unsigned long long WrittenStateBitmap;
		bool Dirty;
		CMTEntryStatus Status;
		std::list<std::pair<LPA_type, CMTSlotType*>>::iterator listPtr;
		// used for fast implementation of LRU
		stream_id_type Stream_id;
	};

	struct GMTEntryType // Entry type for the Global Mapping Table
	{
		PPA_type PPA;
		uint64_t WrittenStateBitmap;
		data_timestamp_type TimeStamp;
	};
	
	class Cached_Mapping_Table
	{
	public:
		Cached_Mapping_Table(unsigned int capacity);
		~Cached_Mapping_Table();
		bool Exists(const stream_id_type streamID, const LPA_type lpa);
		PPA_type RetrievePPA(const stream_id_type streamID, const LPA_type lpa);
		void UpdateMappingInfo(const stream_id_type streamID,
				const LPA_type lpa,
				const PPA_type ppa,
				const page_status_type pageWriteState);
		void InsertNewMappingInfo(const stream_id_type streamID,
				const LPA_type lpa,
				const PPA_type ppa,
				const unsigned long long pageWriteState);
		page_status_type GetBitmapVectorOfWrittenSectors(const stream_id_type streamID, const LPA_type lpa);

		bool IsSlotReservedForLPNAndWaiting(const stream_id_type streamID, const LPA_type lpa);
		bool CheckFreeSlotAvailability();
		void ReserveSlotForLPN(const stream_id_type streamID, const LPA_type lpa);
		CMTSlotType EvictOneSlot(LPA_type& lpa);
		
		bool IsDirty(const stream_id_type streamID, const LPA_type lpa);
		void MakeClean(const stream_id_type streamID, const LPA_type lpa);
	private:
		std::unordered_map<LPA_type, CMTSlotType*> addressMap;
		std::list<std::pair<LPA_type, CMTSlotType*>> lruList;
		unsigned int capacity;
	};

	/* Each stream has its own address mapping domain. It helps isolation of GC interference
	* (e.g., multi-streamed SSD HotStorage 2014, and OPS isolation in FAST 2015)
	* However, CMT is shared among concurrent streams in two ways: 1) each address mapping domain
	* shares the whole CMT space with other domains, and 2) each address mapping domain has
	* its own share of CMT (equal partitioning of CMT space among concurrent streams).*/
	class AddressMappingDomain
	{
	public:
		AddressMappingDomain(unsigned int cmt_capacity, unsigned int cmt_entry_size, unsigned int no_of_translation_entries_per_page,
			Cached_Mapping_Table* CMT,
			Flash_Plane_Allocation_Scheme_Type PlaneAllocationScheme,
			flash_channel_ID_type* channel_ids, unsigned int channel_no, flash_chip_ID_type* chip_ids, unsigned int chip_no,
			flash_die_ID_type* die_ids, unsigned int die_no, flash_plane_ID_type* plane_ids, unsigned int plane_no,
			PPA_type total_physical_sectors_no, LHA_type total_logical_sectors_no, unsigned int sectors_no_per_page);
		~AddressMappingDomain();

		/*Stores the mapping of Virtual Translation Page Number (MVPN) to Physical Translation Page Number (MPPN).
		* It is always kept in volatile memory.*/
		GTDEntryType* GlobalTranslationDirectory;

		/*The cached mapping table that is implemented based on the DFLT (Gupta et al., ASPLOS 2009) proposal.
		* It is always stored in volatile memory.*/
		unsigned int CMT_entry_size;
		unsigned int Translation_entries_per_page;
		Cached_Mapping_Table* CMT;
		unsigned int No_of_inserted_entries_in_preconditioning;

		/*The logical to physical address mapping of all data pages that is implemented based on the DFTL (Gupta et al., ASPLOS 2009(
		* proposal. It is always stored in non-volatile flash memory.*/
		GMTEntryType* GlobalMappingTable;
		void UpdateMappingInfo(const bool ideal_mapping, const stream_id_type stream_id, const LPA_type lpa, const PPA_type ppa, const page_status_type page_status_bitmap);
		page_status_type GetPageStatus(const bool ideal_mapping, const stream_id_type stream_id, const LPA_type lpa);
		PPA_type GetPPA(const bool ideal_mapping, const stream_id_type stream_id, const LPA_type lpa);
		PPA_type GetPPAForPreconditioning(const stream_id_type stream_id, const LPA_type lpa);
		bool MappingEntryAccessible(const bool ideal_mapping, const stream_id_type stream_id, const LPA_type lpa);
	
		std::multimap<LPA_type, NVM_Transaction_Flash*> Waiting_unmapped_read_transactions;
		std::multimap<LPA_type, NVM_Transaction_Flash*> Waiting_unmapped_program_transactions;
		std::multimap<MVPN_type, LPA_type> ArrivingMappingEntries;
		std::set<MVPN_type> DepartingMappingEntries;
		std::set<LPA_type> Locked_LPAs;
		// Used to manage race conditions, i.e. a user request accesses
		// and LPA while GC is moving that LPA 
		std::set<MVPN_type> Locked_MVPNs; // Used to manage race conditions
		std::multimap<LPA_type, NVM_Transaction_Flash*> Read_transactions_behind_LPA_barrier;
		std::multimap<LPA_type, NVM_Transaction_Flash*> Write_transactions_behind_LPA_barrier;
		std::set<MVPN_type> MVPN_read_transactions_waiting_behind_barrier;
		std::set<MVPN_type> MVPN_write_transaction_waiting_behind_barrier;

		Flash_Plane_Allocation_Scheme_Type PlaneAllocationScheme;
		flash_channel_ID_type* Channel_ids;
		unsigned int Channel_no;
		flash_chip_ID_type* Chip_ids;
		unsigned int Chip_no;
		flash_die_ID_type* Die_ids;
		unsigned int Die_no;
		flash_plane_ID_type* Plane_ids;
		unsigned int Plane_no;

		LHA_type max_logical_sector_address;
		LPA_type Total_logical_pages_no;
		PPA_type Total_physical_pages_no;
		MVPN_type Total_translation_pages_no;
	};

	class Address_Mapping_Unit_Page_Level : public Address_Mapping_Unit_Base
	{
		friend class GC_and_WL_Unit_Page_Level;
	public:
		Address_Mapping_Unit_Page_Level(const sim_object_id_type& id,
				FTL* ftl,
				NVM_PHY_ONFI* flash_controller,
				Flash_Block_Manager_Base* block_manager,
				bool ideal_mapping_table,
				unsigned int cmt_capacity_in_byte,
				Flash_Plane_Allocation_Scheme_Type PlaneAllocationScheme,
				unsigned int ConcurrentStreamNo,
				unsigned int ChannelCount,
				unsigned int chip_no_per_channel,
				unsigned int DieNoPerChip,
				unsigned int PlaneNoPerDie,
				std::vector<std::vector<flash_channel_ID_type>> stream_channel_ids,
				std::vector<std::vector<flash_chip_ID_type>> stream_chip_ids,
				std::vector<std::vector<flash_die_ID_type>> stream_die_ids,
				std::vector<std::vector<flash_plane_ID_type>> stream_plane_ids,
				unsigned int Block_no_per_plane,
				unsigned int Page_no_per_block,
				unsigned int SectorsPerPage,
				unsigned int PageSizeInBytes,
				double Overprovisioning_ratio,
				CMT_Sharing_Mode sharing_mode = CMT_Sharing_Mode::SHARED,
				bool fold_large_addresses = true);
		~Address_Mapping_Unit_Page_Level();
		void SetupTriggers();
		void StartSimulation();
		void ValidateSimulationConfig();
		void ExecuteSimulatorEvent(MQSimEngine::Sim_Event*);

		void AllocateAddressForPreconditioning(const stream_id_type stream_id,
				std::map<LPA_type,
				page_status_type>& lpa_list,
				std::vector<double>& steady_state_distribution);
		int BringToCMTForPrecondtioning(stream_id_type stream_id, LPA_type lpa);
		unsigned int GetCMTCapacity();
		unsigned int GetCurrentCMTOccupancyForStream(stream_id_type stream_id);
		void TranslateLPAtoPPAAndDispatch(const std::list<NVM_Transaction*>& transactionList);
		void GetDataMappingInfoForGC(const stream_id_type stream_id,
				const LPA_type lpa,
				PPA_type& ppa,
				page_status_type& page_state);
		void GetTranslationMappingInfoForGC(const stream_id_type stream_id,
				const MVPN_type mvpn,
				MPPN_type& mppa,
				sim_time_type& timestamp);
		void AllocateNewPageForGC(NVM_Transaction_Flash_WR* transaction, bool is_translation_page);

		void StoreMappingTableOnFlashAtStart();
		LPA_type GetLogicalPagesCount(stream_id_type stream_id);
		NVM::FlashMemory::Physical_Page_Address ConvertPPAToAddress(const PPA_type ppa);
		void ConvertPPAToAddress(const PPA_type ppn, NVM::FlashMemory::Physical_Page_Address& address);
		PPA_type ConvertAddressToPPA(const NVM::FlashMemory::Physical_Page_Address& pageAddress);

		void SetBarrierForAccessingPhysicalBlock(const NVM::FlashMemory::Physical_Page_Address& block_address);
		void SetBarrierForAccessingPhysicalBlock(stream_id_type stream_id, LPA_type lpa);
		void SetBarrierForAccessingMVPN(stream_id_type stream_id, MVPN_type mpvn);
		void RemoveBarrierForAccessingLPA(stream_id_type stream_id, LPA_type lpa);
		void RemoveBarrierForAccessingMVPN(stream_id_type stream_id, MVPN_type mpvn);
		void StartServicingWritesForOverfullPlane(const NVM::FlashMemory::Physical_Page_Address plane_address);
	private:
		static Address_Mapping_Unit_Page_Level* _my_instance;
		unsigned int cmt_capacity;
		AddressMappingDomain** domains;
		unsigned int CMT_entry_size, GTD_entry_size;
		// In CMT MQSim stores (lpn, ppn, page status bits) but in GTD it only stores (ppn, page status bits)
		void AllocatePlaneForUserWrite(NVM_Transaction_Flash_WR* transaction);
		void AllocatePageInPlaneForUserWrite(NVM_Transaction_Flash_WR* transaction, bool is_for_gc);
		void AllocatePlaneForTranslationWrite(NVM_Transaction_Flash* transaction);
		void AllocatePageInPlaneForTranslationWrite(NVM_Transaction_Flash* transaction, MVPN_type mvpn, bool is_for_gc);
		void AllocatePlaneForPreconditioning(stream_id_type stream_id, LPA_type lpn, NVM::FlashMemory::Physical_Page_Address& targetAddress);
		bool RequestMappingEntry(const stream_id_type streamID, const LPA_type lpn);
		static void HandleTransactionServicedSignalFromPHY(NVM_Transaction_Flash* transaction);
		bool TranslateLPAToPPA(stream_id_type streamID, NVM_Transaction_Flash* transaction);
		std::set<NVM_Transaction_Flash_WR*>**** Write_transactions_for_overfull_planes;

		void GenerateFlashReadRequestForMappingData(const stream_id_type streamID, const LPA_type lpn);
		void GenerateFlashWritebackRequestForMappingData(const stream_id_type streamID, const LPA_type lpn);

		unsigned int no_of_translation_entries_per_page;
		MVPN_type GetMVPN(const LPA_type lpn, stream_id_type stream_id);
		LPA_type GetStartLPNInMVP(const MVPN_type);
		LPA_type GetEndLPNInMVP(const MVPN_type);

		bool QueryCMT(NVM_Transaction_Flash* transaction);
		PPA_type OnlineCreateEntryForReads(LPA_type lpa, const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& read_address, uint64_t read_sectors_bitmap);
		void ManageUnsuccessfulTranslation(NVM_Transaction_Flash* transaction);
		void ManageUserTransactionFacingBarrier(NVM_Transaction_Flash* transaction);
		void ManageMappingTransactionFacingBarrier(stream_id_type stream_id, MVPN_type mvpn, bool read);
		bool IsLPALockedForGC(stream_id_type stream_id, LPA_type lpa);
		bool IsMVONLockedForGC(stream_id_type stream_id, MVPN_type mvpn);
	};

}

#endif // !ADDRESS_MAPPING_UNIT_PAGE_LEVEL
