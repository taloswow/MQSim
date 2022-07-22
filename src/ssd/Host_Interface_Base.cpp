#include "Host_Interface_Base.h"
#include "Data_Cache_Manager_Base.h"

namespace SSD_Components
{
	Input_Stream_Base::Input_Stream_Base() :
		STAT_number_of_read_requests(0),
		STAT_number_of_write_requests(0), 
		STAT_number_of_read_transactions(0),
		STAT_number_of_write_transactions(0),
		STAT_sum_of_read_transactions_execution_time(0),
		STAT_sum_of_read_transactions_transfer_time(0),
		STAT_sum_of_read_transactions_waiting_time(0),
		STAT_sum_of_write_transactions_execution_time(0),
		STAT_sum_of_write_transactions_transfer_time(0),
		STAT_sum_of_write_transactions_waiting_time(0)
	{}
	
	Input_Stream_Manager_Base::~Input_Stream_Manager_Base()
	{
		for (auto &stream : input_streams) {
			delete stream;
		}
	}

	Input_Stream_Base::~Input_Stream_Base()
	{
	}

	Request_Fetch_Unit_Base::~Request_Fetch_Unit_Base()
	{
		for (auto &dma_info : dma_list) {
			delete dma_info;
		}
	}

	Host_Interface_Base* Host_Interface_Base::_my_instance = NULL;

	Host_Interface_Base::Host_Interface_Base(const sim_object_id_type& id, HostInterface_Types type, LHA_type max_logical_sector_address, unsigned int sectors_per_page, 
		Data_Cache_Manager_Base* cache)
		: MQSimEngine::Sim_Object(id), type(type), max_logical_sector_address(max_logical_sector_address), 
		sectors_per_page(sectors_per_page), cache(cache)
	{
		_my_instance = this;
	}
	
	Host_Interface_Base::~Host_Interface_Base()
	{
		delete input_stream_manager;
		delete request_fetch_unit;
	}

	void Host_Interface_Base::SetupTriggers()
	{
		Sim_Object::SetupTriggers();
		cache->ConnectToUserRequestServicedSignal(HandleUserRequestServicedSignalFromCache);
		cache->ConnectToUserMemoryTransactionServicedSignal(HandleUserMemoryTransactionServicedSignalFromCache);
	}

	void Host_Interface_Base::ValidateSimulationConfig()
	{
	}

	void Host_Interface_Base::SendReadMessageToHost(uint64_t addresss, unsigned int request_read_data_size)
	{
		Host_Components::PCIe_Message* pcie_message = new Host_Components::PCIe_Message;
		pcie_message->Type = Host_Components::PCIe_Message_Type::READ_REQ;
		pcie_message->Destination = Host_Components::PCIe_Destination_Type::HOST;
		pcie_message->Address = addresss;
		pcie_message->Payload = (void*)(intptr_t)request_read_data_size;
		pcie_message->Payload_size = sizeof(request_read_data_size);
		pcie_switch->SendToHost(pcie_message);
	}

	void Host_Interface_Base::SendWriteMessageToHost(uint64_t addresss, void* message, unsigned int message_size)
	{
		Host_Components::PCIe_Message* pcie_message = new Host_Components::PCIe_Message;
		pcie_message->Type = Host_Components::PCIe_Message_Type::WRITE_REQ;
		pcie_message->Destination = Host_Components::PCIe_Destination_Type::HOST;
		pcie_message->Address = addresss;
		COPYDATA(pcie_message->Payload, message, pcie_message->Payload_size);
		pcie_message->Payload_size = message_size;
		pcie_switch->SendToHost(pcie_message);
	}

	void Host_Interface_Base::AttachToDevice(Host_Components::PCIe_Switch* pcie_switch)
	{
		this->pcie_switch = pcie_switch;
	}

	LHA_type Host_Interface_Base::GetMaxLogicalSectorAddress()
	{
		return max_logical_sector_address;
	}

	unsigned int Host_Interface_Base::GetNoOfLHAsInAnNVMWriteUnit()
	{
		return sectors_per_page;
	}

	Input_Stream_Manager_Base::Input_Stream_Manager_Base(Host_Interface_Base* host_interface) :
		host_interface(host_interface)
	{
	}

	void Input_Stream_Manager_Base::UpdateTransactionStatistics(NVM_Transaction* transaction)
	{
		switch (transaction->Type)
		{
			case Transaction_Type::READ:
				this->input_streams[transaction->Stream_id]->STAT_sum_of_read_transactions_execution_time += transaction->STAT_execution_time;
				this->input_streams[transaction->Stream_id]->STAT_sum_of_read_transactions_transfer_time += transaction->STAT_transfer_time;
				this->input_streams[transaction->Stream_id]->STAT_sum_of_read_transactions_waiting_time += (Simulator->Time() - transaction->Issue_time) - transaction->STAT_execution_time - transaction->STAT_transfer_time;
				break;
			case Transaction_Type::WRITE:
				this->input_streams[transaction->Stream_id]->STAT_sum_of_write_transactions_execution_time += transaction->STAT_execution_time;
				this->input_streams[transaction->Stream_id]->STAT_sum_of_write_transactions_transfer_time += transaction->STAT_transfer_time;
				this->input_streams[transaction->Stream_id]->STAT_sum_of_write_transactions_waiting_time += (Simulator->Time() - transaction->Issue_time) - transaction->STAT_execution_time - transaction->STAT_transfer_time;
				break;
			default:
				break;
		}
	}

	uint32_t Input_Stream_Manager_Base::GetAverageReadTransactionTurnaroundTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_read_transactions == 0) {
			return 0;
		}
		return (uint32_t)((input_streams[stream_id]->STAT_sum_of_read_transactions_execution_time + input_streams[stream_id]->STAT_sum_of_read_transactions_transfer_time + input_streams[stream_id]->STAT_sum_of_read_transactions_waiting_time)
			/ input_streams[stream_id]->STAT_number_of_read_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageReadTransactionExecutionTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_read_transactions == 0) {
			return 0;
		}
		return (uint32_t)(input_streams[stream_id]->STAT_sum_of_read_transactions_execution_time / input_streams[stream_id]->STAT_number_of_read_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageReadTransactionTransferTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_read_transactions == 0) {
			return 0;
		}
		return (uint32_t)(input_streams[stream_id]->STAT_sum_of_read_transactions_transfer_time / input_streams[stream_id]->STAT_number_of_read_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageReadTransactionWaitingTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_read_transactions == 0) {
			return 0;
		}
		return (uint32_t)(input_streams[stream_id]->STAT_sum_of_read_transactions_waiting_time / input_streams[stream_id]->STAT_number_of_read_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageWriteTransactionTurnaroundTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_write_transactions == 0) {
			return 0;
		}
		return (uint32_t)((input_streams[stream_id]->STAT_sum_of_write_transactions_execution_time + input_streams[stream_id]->STAT_sum_of_write_transactions_transfer_time + input_streams[stream_id]->STAT_sum_of_write_transactions_waiting_time)
			/ input_streams[stream_id]->STAT_number_of_write_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageWriteTransactionExecutionTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_write_transactions == 0) {
			return 0;
		}
		return (uint32_t)(input_streams[stream_id]->STAT_sum_of_write_transactions_execution_time / input_streams[stream_id]->STAT_number_of_write_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageWriteTransactionTransferTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_write_transactions == 0) {
			return 0;
		}
		return (uint32_t)(input_streams[stream_id]->STAT_sum_of_write_transactions_transfer_time / input_streams[stream_id]->STAT_number_of_write_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}

	uint32_t Input_Stream_Manager_Base::GetAverageWriteTransactionWaitingTime(stream_id_type stream_id) // in microseconds
	{
		if (input_streams[stream_id]->STAT_number_of_write_transactions == 0) {
			return 0;
		}
		return (uint32_t)(input_streams[stream_id]->STAT_sum_of_write_transactions_waiting_time / input_streams[stream_id]->STAT_number_of_write_transactions / SIM_TIME_TO_MICROSECONDS_COEFF);
	}
	
	Request_Fetch_Unit_Base::Request_Fetch_Unit_Base(Host_Interface_Base* host_interface) :
		host_interface(host_interface)
	{
	}
}
