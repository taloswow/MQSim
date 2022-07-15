#ifndef ADDRESS_MAPPING_UNIT_HYBRID_H
#define ADDRESS_MAPPING_UNIT_HYBRID_H

#include "Address_Mapping_Unit_Base.h"

namespace SSD_Components
{
	class Address_Mapping_Unit_Hybrid : public Address_Mapping_Unit_Base
	{
	public:
		Address_Mapping_Unit_Hybrid(sim_object_id_type id,
				FTL* ftl,
				NVM_PHY_ONFI* flash_controller,
				Flash_Block_Manager_Base* block_manager,
				bool ideal_mapping_table,
				unsigned int ConcurrentStreamNo,
				unsigned int ChannelCount,
				unsigned int chip_no_per_channel,
				unsigned int DieNoPerChip,
				unsigned int PlaneNoPerDie,
				unsigned int Block_no_per_plane,
				unsigned int Page_no_per_block,
				unsigned int SectorsPerPage,
				unsigned int PageSizeInBytes,
				double Overprovisioning_ratio,
				CMT_Sharing_Mode sharing_mode = CMT_Sharing_Mode::SHARED,
				bool fold_large_addresses = true);
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
		bool QueryCMT(NVM_Transaction_Flash* transaction);
		PPA_type OnlineCreateEntryForReads(LPA_type lpa, const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& read_address, uint64_t read_sectors_bitmap);
		void ManageUserTransactionFacingBarrier(NVM_Transaction_Flash* transaction);
		void ManageMappingTransactionFacingBarrier(stream_id_type stream_id, MVPN_type mvpn, bool read);
		bool IsLPALockedForGC(stream_id_type stream_id, LPA_type lpa);
		bool IsMVONLockedForGC(stream_id_type stream_id, MVPN_type mvpn);
	};
}

#endif // !ADDRESS_MAPPING_UNIT_HYBRID_H
