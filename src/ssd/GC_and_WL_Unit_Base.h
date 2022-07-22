#ifndef GC_AND_WL_UNIT_BASE_H
#define GC_AND_WL_UNIT_BASE_H

#include "../sim/Sim_Object.h"
#include "../nvm_chip/flash_memory/Flash_Chip.h"
#include "../nvm_chip/flash_memory/Physical_Page_Address.h"
#include "Address_Mapping_Unit_Base.h"
#include "Flash_Block_Manager_Base.h"
#include "TSU_Base.h"
#include "NVM_PHY_ONFI.h"


namespace SSD_Components
{
	enum class GC_Block_Selection_Policy_Type {
		GREEDY,
		RGA,						/*The randomized-greedy algorithm described in: "B. Van Houdt, A Mean Field Model
									for a Class of Garbage Collection Algorithms in Flash - based Solid State Drives,
									SIGMETRICS, 2013" and "Stochastic Modeling of Large-Scale Solid-State Storage
									Systems: Analysis, Design Tradeoffs and Optimization, SIGMETRICS, 2013".*/
		RANDOM, RANDOM_P, RANDOM_PP,/*The RANDOM, RANDOM+, and RANDOM++ algorithms described in: "B. Van Houdt, A Mean
									Field Model  for a Class of Garbage Collection Algorithms in Flash - based Solid
									State Drives, SIGMETRICS, 2013".*/
		FIFO						/*The FIFO algortihm described in P. Desnoyers, "Analytic  Modeling  of  SSD Write
									Performance, SYSTOR, 2012".*/
	};

	class Address_Mapping_Unit_Base;
	class Flash_Block_Manager_Base;
	class TSU_Base;
	class NVM_PHY_ONFI;
	class PlaneBookKeepingType;
	class Block_Pool_Slot_Type;

	// This class implements thet the Garbage Collection and Wear Leveling module of MQSim.
	class GC_and_WL_Unit_Base : public MQSimEngine::Sim_Object
	{
	public:
		GC_and_WL_Unit_Base(const sim_object_id_type& id,
				Address_Mapping_Unit_Base* address_mapping_unit,
				Flash_Block_Manager_Base* block_manager,
				TSU_Base* tsu,
				NVM_PHY_ONFI* flash_controller,
				GC_Block_Selection_Policy_Type block_selection_policy,
				double gc_threshold,
				bool preemptible_gc_enabled,
				double gc_hard_threshold,
				unsigned int channel_count,
				unsigned int chip_no_per_channel,
				unsigned int die_no_per_chip,
				unsigned int plane_no_per_die,
				unsigned int block_no_per_plane,
				unsigned int page_no_per_block,
				unsigned int sector_no_per_page,
				bool use_copyback,
				double rho,
				unsigned int max_ongoing_gc_reqs_per_plane,
				bool dynamic_wearleveling_enabled,
				bool static_wearleveling_enabled,
				unsigned int static_wearleveling_threshold,
				int seed);
		void SetupTriggers();
		void StartSimulation();
		void ValidateSimulationConfig();
		void ExecuteSimulatorEvent(MQSimEngine::Sim_Event*);

		virtual bool GCIsInUrgentMode(const NVM::FlashMemory::Flash_Chip*) = 0;
		virtual void CheckGCRequired(const unsigned int BlockPoolSize, const NVM::FlashMemory::Physical_Page_Address& planeAddress) = 0;
		GC_Block_Selection_Policy_Type GetGCPolicy();
		unsigned int GetGCPolicySpecificParameter();
		// Returns the parameter specific to the GC block selection policy: threshold for random_pp, set_size for RGA

		unsigned int GetMinimumNumberOfFreePagesBeforeGC();
		bool UseDynamicWearleveling();
		bool UseStaticWearleveling();
		bool StopServicingWrites(const NVM::FlashMemory::Physical_Page_Address& plane_address);
	protected:
		GC_Block_Selection_Policy_Type block_selection_policy;
		static GC_and_WL_Unit_Base * _my_instance;
		Address_Mapping_Unit_Base* address_mapping_unit;
		Flash_Block_Manager_Base* block_manager;
		TSU_Base* tsu;
		NVM_PHY_ONFI* flash_controller;
		bool force_gc;
		double gc_threshold; // As the ratio of free pages to the total number of physical pages
		unsigned int block_pool_gc_threshold;
		static void HandleTransactionServicedSignalFromPHY(NVM_Transaction_Flash* transaction);
		bool IsSafeGCWLCandidate(const PlaneBookKeepingType* pbke, const flash_block_ID_type gc_wl_candidate_block_id);
		// Checks if block_address is a safe candidate for gc execution, i.e., 1) it is not a write frontier, and 2) there is no ongoing program operation

		bool CheckStaticWLRequired(const NVM::FlashMemory::Physical_Page_Address plane_address);
		void RunStaticWearleveling(const NVM::FlashMemory::Physical_Page_Address plane_address);
		bool use_copyback;
		bool dynamic_wearleveling_enabled;
		bool static_wearleveling_enabled;
		unsigned int static_wearleveling_threshold;

		// Used to implement: "Preemptible I/O Scheduling of Garbage Collection for Solid State Drives", TCAD 2013.
		bool preemptible_gc_enabled;
		double gc_hard_threshold;
		unsigned int block_pool_gc_hard_threshold;
		unsigned int max_ongoing_gc_reqs_per_plane;
		// This value has two important usages: 1) maximum number of concurrent gc operations
		// per plane, and 2) the value that determines urgent GC execution when there is a shortage
		// of flash blocks. If the block bool size drops below this value, all incomming user writes should be blocked

		// Following variables are used based on the type of GC block selection policy
		unsigned int rga_set_size; // The number of random flash blocks that are radnomly selected 
		Utils::RandomGenerator random_generator;
		std::queue<Block_Pool_Slot_Type*> block_usage_fifo;
		unsigned int random_pp_threshold;

		unsigned int channel_count;
		unsigned int chip_no_per_channel;
		unsigned int die_no_per_chip;
		unsigned int plane_no_per_die;
		unsigned int block_no_per_plane;
		unsigned int pages_no_per_block;
		unsigned int sector_no_per_page;
	};
}

#endif // !GC_AND_WL_UNIT_BASE_H
