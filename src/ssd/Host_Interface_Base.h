#ifndef HOST_INTERFACE_BASE_H
#define HOST_INTERFACE_BASE_H

#include <vector>
#include <stdint.h>
#include <cstring>

#include "../sim/Sim_Object.h"
#include "../sim/Sim_Reporter.h"
#include "../host/PCIe_Switch.h"
#include "../host/PCIe_Message.h"
#include "User_Request.h"
#include "Data_Cache_Manager_Base.h"

namespace Host_Components
{
	class PCIe_Switch;
}

namespace SSD_Components
{
#define COPYDATA(DEST,SRC,SIZE) if(Simulator->IsIntegratedExecutionMode()) {DEST = new char[SIZE]; memcpy(DEST, SRC, SIZE);} else DEST = SRC;
#define DELETE_REQUEST_NVME(REQ) \
	delete (Submission_Queue_Entry*)REQ->IO_command_info; \
	if(Simulator->IsIntegratedExecutionMode())\
		{if(REQ->Data != NULL) delete[] (char*)REQ->Data;} \
	if(REQ->Transaction_list.size() != 0) PRINT_ERROR("Deleting an unhandled user requests in the host interface! MQSim thinks something is going wrong!")\
	delete REQ;

	class Data_Cache_Manager_Base;
	class Host_Interface_Base;

	class Input_Stream_Base
	{
	public:
		Input_Stream_Base();
		virtual ~Input_Stream_Base();
		unsigned int STAT_number_of_read_requests;
		unsigned int STAT_number_of_write_requests;
		unsigned int STAT_number_of_read_transactions;
		unsigned int STAT_number_of_write_transactions;
		sim_time_type STAT_sum_of_read_transactions_execution_time, STAT_sum_of_read_transactions_transfer_time, STAT_sum_of_read_transactions_waiting_time;
		sim_time_type STAT_sum_of_write_transactions_execution_time, STAT_sum_of_write_transactions_transfer_time, STAT_sum_of_write_transactions_waiting_time;
	};

	class Input_Stream_Manager_Base
	{
		friend class Request_Fetch_Unit_Base;
		friend class Request_Fetch_Unit_NVMe;
		friend class Request_Fetch_Unit_SATA;
	public:
		Input_Stream_Manager_Base(Host_Interface_Base* host_interface);
		virtual ~Input_Stream_Manager_Base();
		virtual void HandleNewArrivedRequest(User_Request* request) = 0;
		virtual void HandleArrivedWriteData(User_Request* request) = 0;
		virtual void HandleServicedRequest(User_Request* request) = 0;
		void UpdateTransactionStatistics(NVM_Transaction* transaction);
		uint32_t GetAverageReadTransactionTurnaroundTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageReadTransactionExecutionTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageReadTransactionTransferTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageReadTransactionWaitingTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageWriteTransactionTurnaroundTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageWriteTransactionExecutionTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageWriteTransactionTransferTime(stream_id_type stream_id);//in microseconds
		uint32_t GetAverageWriteTransactionWaitingTime(stream_id_type stream_id);//in microseconds
	protected:
		Host_Interface_Base* host_interface;
		virtual void SegmentUserRequest(User_Request* user_request) = 0;
		std::vector<Input_Stream_Base*> input_streams;
	};

	class Request_Fetch_Unit_Base
	{
	public:
		Request_Fetch_Unit_Base(Host_Interface_Base* host_interface);
		virtual ~Request_Fetch_Unit_Base();
		virtual void FetchNextRequest(stream_id_type stream_id) = 0;
		virtual void FetchWriteData(User_Request* request) = 0;
		virtual void SendReadData(User_Request* request) = 0;
		virtual void ProcessPCIeWriteMessage(uint64_t, void *, unsigned int) = 0;
		virtual void ProcessPCIeReadMessage(uint64_t, void *, unsigned int) = 0;
	protected:
		enum class DMA_Req_Type { REQUEST_INFO, WRITE_DATA };
		struct DMA_Req_Item
		{
			DMA_Req_Type Type;
			void * object;
		};
		Host_Interface_Base* host_interface;
		std::list<DMA_Req_Item*> dma_list;
	};

	class Host_Interface_Base : public MQSimEngine::Sim_Object, public MQSimEngine::Sim_Reporter
	{
		friend class Input_Stream_Manager_Base;
		friend class Input_Stream_Manager_NVMe;
		friend class Input_Stream_Manager_SATA;
		friend class Request_Fetch_Unit_Base;
		friend class Request_Fetch_Unit_NVMe;
		friend class Request_Fetch_Unit_SATA;
	public:
		Host_Interface_Base(const sim_object_id_type& id, HostInterface_Types type, LHA_type max_logical_sector_address, 
			unsigned int sectors_per_page, Data_Cache_Manager_Base* cache);
		virtual ~Host_Interface_Base();
		void SetupTriggers();
		void ValidateSimulationConfig();

		typedef void(*UserRequestArrivedSignalHandlerType) (User_Request*);
		void ConnectToUserRequestArrivedSignal(UserRequestArrivedSignalHandlerType function)
		{
			connected_user_request_arrived_signal_handlers.push_back(function);
		}

		void ConsumePCIeMessage(Host_Components::PCIe_Message* message)
		{
			if (message->Type == Host_Components::PCIe_Message_Type::READ_COMP) {
				request_fetch_unit->ProcessPCIeReadMessage(message->Address, message->Payload, message->Payload_size);
			} else {
				request_fetch_unit->ProcessPCIeWriteMessage(message->Address, message->Payload, message->Payload_size);
			}
			delete message;
		}
	
		void SendReadMessageToHost(uint64_t addresss, unsigned int request_read_data_size);
		void SendWriteMessageToHost(uint64_t addresss, void* message, unsigned int message_size);

		HostInterface_Types GetType() { return type; }
		void AttachToDevice(Host_Components::PCIe_Switch* pcie_switch);
		LHA_type GetMaxLogicalSectorAddress();
		unsigned int GetNoOfLHAsInAnNVMWriteUnit();
	protected:
		HostInterface_Types type;
		LHA_type max_logical_sector_address;
		unsigned int sectors_per_page;
		static Host_Interface_Base* _my_instance;
		Input_Stream_Manager_Base* input_stream_manager;
		Request_Fetch_Unit_Base* request_fetch_unit;
		Data_Cache_Manager_Base* cache;
		std::vector<UserRequestArrivedSignalHandlerType> connected_user_request_arrived_signal_handlers;

		void BroadcastUserRequestArrivalSignal(User_Request* user_request)
		{
			for (std::vector<UserRequestArrivedSignalHandlerType>::iterator it = connected_user_request_arrived_signal_handlers.begin();
				it != connected_user_request_arrived_signal_handlers.end(); it++) {
				(*it)(user_request);
			}
		}

		static void HandleUserRequestServicedSignalFromCache(User_Request* user_request)
		{
			_my_instance->input_stream_manager->HandleServicedRequest(user_request);
		}

		static void HandleUserMemoryTransactionServicedSignalFromCache(NVM_Transaction* transaction)
		{
			_my_instance->input_stream_manager->UpdateTransactionStatistics(transaction);
		}
	private:
		Host_Components::PCIe_Switch* pcie_switch;
	};
}

#endif // !HOST_INTERFACE_BASE_H
