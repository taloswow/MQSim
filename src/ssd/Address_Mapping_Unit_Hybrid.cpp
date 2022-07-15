#include "Address_Mapping_Unit_Hybrid.h"

namespace SSD_Components
{
	Address_Mapping_Unit_Hybrid::Address_Mapping_Unit_Hybrid(sim_object_id_type id, FTL* ftl, NVM_PHY_ONFI* flash_controller, Flash_Block_Manager_Base* block_manager,
		bool ideal_mapping_table, unsigned int concurrent_streams_no,
		unsigned int channel_count, unsigned int chip_no_per_channel, unsigned int die_no_per_chip, unsigned int plane_no_in_die,
		unsigned int block_no_per_plane, unsigned int page_no_per_block, unsigned int sectors_per_page, unsigned int page_size_in_byte,
		double overprovisioning_ratio, CMT_Sharing_Mode sharing_mode, bool fold_out_of_range_addresses) :
		Address_Mapping_Unit_Base(id, ftl, flash_controller, block_manager, ideal_mapping_table,
			concurrent_streams_no, channel_count, chip_no_per_channel, die_no_per_chip, plane_no_in_die,
			block_no_per_plane, page_no_per_block, sectors_per_page, page_size_in_byte, overprovisioning_ratio, sharing_mode, fold_out_of_range_addresses) {}
	void Address_Mapping_Unit_Hybrid::Setup_triggers() {}
	void Address_Mapping_Unit_Hybrid::StartSimulation() {}
	void Address_Mapping_Unit_Hybrid::ValidateSimulationConfig() {}
	void Address_Mapping_Unit_Hybrid::ExecuteSimulatorEvent(MQSimEngine::Sim_Event* event) {}

	void Address_Mapping_Unit_Hybrid::AllocateAddressForPreconditioning(const stream_id_type stream_id, std::map<LPA_type, page_status_type>& lpa_list, std::vector<double>& steady_state_distribution) {}
	int Address_Mapping_Unit_Hybrid::BringToCMTForPrecondtioning(stream_id_type stream_id, LPA_type lpa) { return 0; }
	unsigned int Address_Mapping_Unit_Hybrid::GetCMTCapacity() { return 0; }
	unsigned int Address_Mapping_Unit_Hybrid::GetCurrentCMTOccupancyForStream(stream_id_type stream_id) { return 0; }
	void Address_Mapping_Unit_Hybrid::TranslateLPAtoPPAAndDispatch(const std::list<NVM_Transaction*>& transaction_list) {}
	void Address_Mapping_Unit_Hybrid::GetDataMappingInfoForGC(const stream_id_type stream_id, const LPA_type lpa, PPA_type& ppa, page_status_type& page_state) {}
	void Address_Mapping_Unit_Hybrid::GetTranslationMappingInfoForGC(const stream_id_type stream_id, const MVPN_type mvpn, MPPN_type& mppa, sim_time_type& timestamp) {}

	PPA_type Address_Mapping_Unit_Hybrid::online_create_entry_for_reads(LPA_type lpa, const stream_id_type stream_id, NVM::FlashMemory::Physical_Page_Address& read_address, uint64_t read_sectors_bitmap) { return 0; }

	bool Address_Mapping_Unit_Hybrid::query_cmt(NVM_Transaction_Flash* transaction) { return true; }
	NVM::FlashMemory::Physical_Page_Address Address_Mapping_Unit_Hybrid::ConvertPPAToAddress(const PPA_type ppa)
	{
		NVM::FlashMemory::Physical_Page_Address pa;
		return pa;
	}
	
	LPA_type Address_Mapping_Unit_Hybrid::GetLogicalPagesCount(stream_id_type stream_id)
	{
		return 0;
	}

	void Address_Mapping_Unit_Hybrid::ConvertPPAToAddress(const PPA_type ppa, NVM::FlashMemory::Physical_Page_Address& address) {}
	PPA_type Address_Mapping_Unit_Hybrid::ConvertAddressToPPA(const NVM::FlashMemory::Physical_Page_Address& pageAddress) { return 0; }
	void Address_Mapping_Unit_Hybrid::StoreMappingTableOnFlashAtStart() {}
	void Address_Mapping_Unit_Hybrid::AllocateNewPageForGC(NVM_Transaction_Flash_WR* transaction, bool is_translation_page) {}
	void Address_Mapping_Unit_Hybrid::SetBarrierForAccessingPhysicalBlock(const NVM::FlashMemory::Physical_Page_Address& block_address) {}
	void Address_Mapping_Unit_Hybrid::SetBarrierForAccessingPhysicalBlock(stream_id_type stream_id, LPA_type lpa) {}
	void Address_Mapping_Unit_Hybrid::RemoveBarrierForAccessingLPA(stream_id_type stream_id, LPA_type lpa) {}
	void Address_Mapping_Unit_Hybrid::SetBarrierForAccessingMVPN(stream_id_type stream_id, MVPN_type mpvn) {}
	void Address_Mapping_Unit_Hybrid::RemoveBarrierForAccessingMVPN(stream_id_type stream_id, MVPN_type mpvn) {}
	bool Address_Mapping_Unit_Hybrid::is_lpa_locked_for_gc(stream_id_type stream_id, LPA_type lpa) { return false; }
	bool Address_Mapping_Unit_Hybrid::is_mvpn_locked_for_gc(stream_id_type stream_id, MVPN_type mvpn) { return false; }
	void Address_Mapping_Unit_Hybrid::manage_user_transaction_facing_barrier(NVM_Transaction_Flash* transaction) {}
	void Address_Mapping_Unit_Hybrid::manage_mapping_transaction_facing_barrier(stream_id_type stream_id, MVPN_type mvpn, bool read) {}
	void Address_Mapping_Unit_Hybrid::StartServicingWritesForOverfullPlane(const NVM::FlashMemory::Physical_Page_Address plane_address) {}
}