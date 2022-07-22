#include "Host_System.h"

#include "../sim/Engine.h"
#include "../ssd/Host_Interface_Base.h"
#include "../ssd/Host_Interface_NVMe.h"
#include "../host/PCIe_Root_Complex.h"
#include "../host/IO_Flow_Synthetic.h"
#include "../host/IO_Flow_Trace_Based.h"
#include "../utils/StringTools.h"
#include "../utils/Logical_Address_Partitioning_Unit.h"

Host_System::Host_System(Host_Parameter_Set* parameters,
		bool preconditioning_required,
		SSD_Components::Host_Interface_Base* ssd_host_interface):
	MQSimEngine::Sim_Object("Host"),
	preconditioning_required(preconditioning_required)
{
	Simulator->AddObject(this);

	// Create the main components of the host system
	if (((SSD_Components::Host_Interface_NVMe*)ssd_host_interface)->GetType() == HostInterface_Types::SATA) {
		this->SATA_hba = new Host_Components::SATA_HBA(ID() + ".SATA_HBA", ((SSD_Components::Host_Interface_SATA*)ssd_host_interface)->Get_ncq_depth(), parameters->SATA_Processing_Delay, NULL, NULL);
	} else {
		this->SATA_hba = NULL;
	}

	this->Link = new Host_Components::PCIe_Link(this->ID() + ".PCIeLink", NULL, NULL, parameters->PCIe_Lane_Bandwidth, parameters->PCIe_Lane_Count);
	this->PCIe_root_complex = new Host_Components::PCIe_Root_Complex(this->Link, ssd_host_interface->GetType(), this->SATA_hba, NULL);
	this->Link->SetRootComplex(this->PCIe_root_complex);
	this->PCIe_switch = new Host_Components::PCIe_Switch(this->Link, ssd_host_interface);
	this->Link->SetPCIeSwitch(this->PCIe_switch);
	Simulator->AddObject(this->Link);

	// Create IO flows
	LHA_type address_range_per_flow = ssd_host_interface->GetMaxLogicalSectorAddress() / parameters->IO_Flow_Definitions.size();
	for (uint16_t flow_id = 0; flow_id < parameters->IO_Flow_Definitions.size(); flow_id++) {
		Host_Components::IO_Flow_Base* io_flow = NULL;
		// No flow should ask for I/O queue id 0, it is reserved for NVMe Admin command queue pair
		// Hence, we use flow_id + 1 (which is equal to 1, 2, ...) as the requested I/O queue id
		uint16_t nvme_sq_size = 0, nvme_cq_size = 0;
		switch (((SSD_Components::Host_Interface_NVMe*)ssd_host_interface)->GetType()) {
			case HostInterface_Types::NVME:
				nvme_sq_size = ((SSD_Components::Host_Interface_NVMe*)ssd_host_interface)->GetSubmissionQueueDepth();
				nvme_cq_size = ((SSD_Components::Host_Interface_NVMe*)ssd_host_interface)->GetCompletionQueueDepth();
				break;
			default:
				break;
		}

		switch (parameters->IO_Flow_Definitions[flow_id]->Type) {
			case Flow_Type::SYNTHETIC: {
				IO_Flow_Parameter_Set_Synthetic* flow_param = (IO_Flow_Parameter_Set_Synthetic*)parameters->IO_Flow_Definitions[flow_id];
				if (flow_param->Working_Set_Percentage > 100 || flow_param->Working_Set_Percentage < 1) {
					flow_param->Working_Set_Percentage = 100;
				}
				io_flow = new Host_Components::IO_Flow_Synthetic(this->ID() + ".IO_Flow.Synth.No_" + std::to_string(flow_id), flow_id,
					Utils::Logical_Address_Partitioning_Unit::StartLhaAvilableToFlow(flow_id),
					Utils::Logical_Address_Partitioning_Unit::EndLhaAvilableToFlow(flow_id),
					((double)flow_param->Working_Set_Percentage / 100.0), FLOW_ID_TO_Q_ID(flow_id), nvme_sq_size, nvme_cq_size,
					flow_param->PriorityClass, flow_param->Read_Percentage / double(100.0), flow_param->Address_Distribution, flow_param->Percentage_of_Hot_Region / double(100.0),
					flow_param->Request_Size_Distribution, flow_param->Average_Request_Size, flow_param->Variance_Request_Size,
					flow_param->Synthetic_Generator_Type, (flow_param->Bandwidth == 0? 0 :NanoSecondCoeff / ((flow_param->Bandwidth / SECTOR_SIZE_IN_BYTE) / flow_param->Average_Request_Size)),
					flow_param->Average_No_of_Reqs_in_Queue, flow_param->Generated_Aligned_Addresses, flow_param->Address_Alignment_Unit,
					flow_param->Seed, flow_param->Stop_Time, flow_param->Initial_Occupancy_Percentage / double(100.0), flow_param->Total_Requests_To_Generate, ssd_host_interface->GetType(), this->PCIe_root_complex, this->SATA_hba,
					parameters->Enable_ResponseTime_Logging, parameters->ResponseTime_Logging_Period_Length, parameters->Input_file_path + ".IO_Flow.No_" + std::to_string(flow_id) + ".log");
				this->IO_flows.push_back(io_flow);
				break;
			}
			case Flow_Type::TRACE: {
				IO_Flow_Parameter_Set_Trace_Based * flow_param = (IO_Flow_Parameter_Set_Trace_Based*)parameters->IO_Flow_Definitions[flow_id];
				io_flow = new Host_Components::IO_Flow_Trace_Based(this->ID() + ".IO_Flow.Trace." + flow_param->File_Path, flow_id,
					Utils::Logical_Address_Partitioning_Unit::StartLhaAvilableToFlow(flow_id), Utils::Logical_Address_Partitioning_Unit::EndLhaAvilableToFlow(flow_id),
					FLOW_ID_TO_Q_ID(flow_id), nvme_sq_size, nvme_cq_size,
					flow_param->PriorityClass, flow_param->Initial_Occupancy_Percentage / double(100.0),
					flow_param->File_Path, flow_param->Time_Unit, flow_param->Relay_Count, flow_param->Percentage_To_Be_Executed,
					ssd_host_interface->GetType(), this->PCIe_root_complex, this->SATA_hba,
					parameters->Enable_ResponseTime_Logging, parameters->ResponseTime_Logging_Period_Length, parameters->Input_file_path + ".IO_Flow.No_" + std::to_string(flow_id) + ".log");

				this->IO_flows.push_back(io_flow);
				break;
			}
			default:
				throw "The specified IO flow type is not supported.\n";
		}
		Simulator->AddObject(io_flow);
	}
	this->PCIe_root_complex->SetIOFlows(&this->IO_flows);
	if (((SSD_Components::Host_Interface_NVMe*)ssd_host_interface)->GetType() == HostInterface_Types::SATA) {
		this->SATA_hba->SetIOFlows(&this->IO_flows);
		this->SATA_hba->SetRootComplex(this->PCIe_root_complex);
	}
}

Host_System::~Host_System() 
{
	delete this->Link;
	delete this->PCIe_root_complex;
	delete this->PCIe_switch;
	if (ssd_device->Host_interface->GetType() == HostInterface_Types::SATA) {
		delete this->SATA_hba;
	}
	for (uint16_t flow_id = 0; flow_id < this->IO_flows.size(); flow_id++) {
		delete this->IO_flows[flow_id];
	}
}

void Host_System::AttachSSDDevice(SSD_Device* ssd_device)
{
	ssd_device->AttachToHost(this->PCIe_switch);
	this->PCIe_switch->AttachSSDDevice(ssd_device->Host_interface);
	this->ssd_device = ssd_device;
}

const std::vector<Host_Components::IO_Flow_Base*> Host_System::GetIOFlows()
{
	return IO_flows;
}

void Host_System::StartSimulation()
{
	switch (ssd_device->Host_interface->GetType()) {
		case HostInterface_Types::NVME:
			for (uint16_t flow_cntr = 0; flow_cntr < IO_flows.size(); flow_cntr++) {
				((SSD_Components::Host_Interface_NVMe*) ssd_device->Host_interface)->CreateNewStream(
					IO_flows[flow_cntr]->Priority_class(),
					IO_flows[flow_cntr]->GetStartLSAOnDevice(), IO_flows[flow_cntr]->GetEndLSAonDevice(),
					IO_flows[flow_cntr]->GetNVMeQueuePairInfo()->Submission_queue_memory_base_address, IO_flows[flow_cntr]->GetNVMeQueuePairInfo()->Completion_queue_memory_base_address);
			}
			break;
		case HostInterface_Types::SATA:
			((SSD_Components::Host_Interface_SATA*) ssd_device->Host_interface)->Set_ncq_address(
				SATA_hba->GetSataNCQInfo()->Submission_queue_memory_base_address, SATA_hba->GetSataNCQInfo()->Completion_queue_memory_base_address);
		default:
			break;
	}

	if (preconditioning_required) {
		std::vector<Utils::Workload_Statistics*> workload_stats = GetWorkloadsStatistics();
		ssd_device->PerformPreconditioning(workload_stats);
		for (auto &stat : workload_stats) {
			delete stat;
		}
	}
}

void Host_System::ValidateSimulationConfig() 
{
	if (this->IO_flows.size() == 0) {
		PRINT_ERROR("No IO flow is set for host system")
	}
	if (this->PCIe_root_complex == NULL) {
		PRINT_ERROR("PCIe Root Complex is not set for host system");
	}
	if (this->Link == NULL) {
		PRINT_ERROR("PCIe Link is not set for host system");
	}
	if (this->PCIe_switch == NULL) {
		PRINT_ERROR("PCIe Switch is not set for host system")
	}
	if (!this->PCIe_switch->IsSSDConnected()) {
		PRINT_ERROR("No SSD is connected to the host system")
	}
}

void Host_System::ExecuteSimulatorEvent(MQSimEngine::Sim_Event* event)
{
}

void Host_System::ReportResultsInXML(std::string name_prefix, Utils::XmlWriter& xmlwriter)
{
	std::string tmp;
	tmp = ID();
	xmlwriter.WriteOpenTag(tmp);

	for (auto &flow : IO_flows) {
		flow->ReportResultsInXML("Host", xmlwriter);
	}

	xmlwriter.WriteCloseTag();
}

std::vector<Utils::Workload_Statistics*> Host_System::GetWorkloadsStatistics()
{
	std::vector<Utils::Workload_Statistics*> stats;

	for (auto &workload : IO_flows) {
		Utils::Workload_Statistics* s = new Utils::Workload_Statistics;
		workload->GetStatistics(*s, ssd_device->ConvertHostLogicToDeviceAddress, ssd_device->FindNVMSubunitAccessBitmap);
		stats.push_back(s);
	}

	return stats;
}

