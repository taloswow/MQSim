#ifndef HOST_INTERFACE_NVME_H
#define HOST_INTERFACE_NVME_H

#include <vector>
#include <list>
#include <map>

#include "../sim/Sim_Event.h"
#include "Host_Interface_Base.h"
#include "User_Request.h"
#include "Host_Interface_Defs.h"

namespace SSD_Components
{
class Input_Stream_NVMe : public Input_Stream_Base
{
public:
	Input_Stream_NVMe(IO_Flow_PriorityClass::Priority priority_class,
			LHA_type start_logical_sector_address,
			LHA_type end_logical_sector_address,
			uint64_t submission_queue_base_address,
			uint16_t submission_queue_size,
			uint64_t completion_queue_base_address,
			uint16_t completion_queue_size) : 
		Input_Stream_Base(),
		Priority_class(priority_class),
		Start_logical_sector_address(start_logical_sector_address),
		End_logical_sector_address(end_logical_sector_address),
		Submission_queue_base_address(submission_queue_base_address),
		Submission_queue_size(submission_queue_size),
		Completion_queue_base_address(completion_queue_base_address),
		Completion_queue_size(completion_queue_size),
		Submission_head(0),
		Submission_head_informed_to_host(0),
		Submission_tail(0),
		Completion_head(0),
		Completion_tail(0),
		On_the_fly_requests(0) {}
	~Input_Stream_NVMe();
	IO_Flow_PriorityClass::Priority Priority_class;
	LHA_type Start_logical_sector_address;
	LHA_type End_logical_sector_address;
	uint64_t Submission_queue_base_address;
	uint16_t Submission_queue_size;
	uint64_t Completion_queue_base_address;
	uint16_t Completion_queue_size;
	uint16_t Submission_head;
	uint16_t Submission_head_informed_to_host;
	// To avoide race condition, the submission head must not be informed to host until the request info of the head is successfully arrived
	uint16_t Submission_tail;
	uint16_t Completion_head;
	uint16_t Completion_tail;
	std::list<User_Request *> Waiting_user_requests; // The list of requests that have been fetch to the device queue and are getting serviced
	std::list<User_Request *> Completed_user_requests; // The list of requests that are completed but have not been informed to the host due to full CQ
	std::list<User_Request *> Waiting_write_data_transfers; // The list of write requests that are waiting for data
	uint16_t On_the_fly_requests; // the number of requests that are either being fetch from host or waiting in the device queue
};

class Input_Stream_Manager_NVMe : public Input_Stream_Manager_Base
{
public:
	Input_Stream_Manager_NVMe(Host_Interface_Base *host_interface, uint16_t queue_fetch_szie);
	unsigned int Queue_fetch_size;
	stream_id_type CreateNewStream(IO_Flow_PriorityClass::Priority priority_class,
			LHA_type start_logical_sector_address,
			LHA_type end_logical_sector_address,
			uint64_t submission_queue_base_address,
			uint16_t submission_queue_size,
			uint64_t completion_queue_base_address,
			uint16_t completion_queue_size);
	void SubmissionQueueTailPointerUpdate(stream_id_type stream_id, uint16_t tail_pointer_value);
	void CompletionQueueHeadPointerUpdate(stream_id_type stream_id, uint16_t head_pointer_value);
	void HandleNewArrivedRequest(User_Request *request);
	void HandleArrivedWriteData(User_Request *request);
	void HandleServicedRequest(User_Request *request);
	uint16_t GetSubmissionQueueDepth(stream_id_type stream_id);
	uint16_t GetCompletionQueueDepth(stream_id_type stream_id);
	IO_Flow_PriorityClass::Priority GetPriorityClass(stream_id_type stream_id);

private:
	void SegmentUserRequest(User_Request *user_request);
	void InformHostRequestCompleted(stream_id_type stream_id, User_Request *request);
};

class Request_Fetch_Unit_NVMe : public Request_Fetch_Unit_Base
{
public:
	Request_Fetch_Unit_NVMe(Host_Interface_Base *host_interface);
	void FetchNextRequest(stream_id_type stream_id);
	void FetchWriteData(User_Request *request);
	void SendReadData(User_Request *request);
	void SendCompletionQueueElement(User_Request *request, uint16_t sq_head_value);
	void ProcessPCIeWriteMessage(uint64_t, void *, unsigned int);
	void ProcessPCIeReadMessage(uint64_t, void *, unsigned int);

private:
	uint16_t current_phase;
	uint32_t number_of_sent_cqe;
};

class Host_Interface_NVMe : public Host_Interface_Base
{
	friend class Input_Stream_Manager_NVMe;
	friend class Request_Fetch_Unit_NVMe;

public:
	Host_Interface_NVMe(const sim_object_id_type &id,
			LHA_type max_logical_sector_address,
			uint16_t submission_queue_depth,
			uint16_t completion_queue_depth,
			unsigned int no_of_input_streams,
			uint16_t queue_fetch_size,
			unsigned int sectors_per_page,
			Data_Cache_Manager_Base *cache);
	stream_id_type CreateNewStream(IO_Flow_PriorityClass::Priority priority_class,
			LHA_type start_logical_sector_address,
			LHA_type end_logical_sector_address,
			uint64_t submission_queue_base_address,
			uint64_t completion_queue_base_address);
	void StartSimulation();
	void ValidateSimulationConfig();
	void ExecuteSimulatorEvent(MQSimEngine::Sim_Event *);
	uint16_t GetSubmissionQueueDepth();
	uint16_t GetCompletionQueueDepth();
	void ReportResultsInXML(std::string name_prefix, Utils::XmlWriter &xmlwriter);

private:
	uint16_t submission_queue_depth, completion_queue_depth;
	unsigned int no_of_input_streams;
};
} // namespace SSD_Components

#endif // !HOSTINTERFACE_NVME_H
