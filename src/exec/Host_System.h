#ifndef HOST_SYSTEM_H
#define HOST_SYSTEM_H

#include <vector>

#include "Host_Parameter_Set.h"
#include "SSD_Device.h"

#include "../sim/Sim_Object.h"
#include "../sim/Sim_Reporter.h"
#include "../host/PCIe_Root_Complex.h"
#include "../host/PCIe_Link.h"
#include "../host/PCIe_Switch.h"
#include "../host/PCIe_Message.h"
#include "../host/IO_Flow_Base.h"
#include "../host/Host_IO_Request.h"
#include "../ssd/Host_Interface_Base.h"

#include "../utils/Workload_Statistics.h"

class Host_System : public MQSimEngine::Sim_Object, public MQSimEngine::Sim_Reporter
{
public:
	Host_System(Host_Parameter_Set* parameters,
			bool preconditioning_required,
			SSD_Components::Host_Interface_Base* ssd_host_interface);
	~Host_System();
	void StartSimulation();
	void ValidateSimulationConfig();
	void ExecuteSimulatorEvent(MQSimEngine::Sim_Event* event);
	void ReportResultsInXML(std::string name_prefix, Utils::XmlWriter& xmlwriter);

	void AttachSSDDevice(SSD_Device* ssd_device);
	const std::vector<Host_Components::IO_Flow_Base*> GetIOFlows();
private:
	Host_Components::PCIe_Root_Complex* PCIe_root_complex;
	Host_Components::PCIe_Link* Link;
	Host_Components::PCIe_Switch* PCIe_switch;
	Host_Components::SATA_HBA* SATA_hba;
	std::vector<Host_Components::IO_Flow_Base*> IO_flows;
	SSD_Device* ssd_device;
	std::vector<Utils::Workload_Statistics*> GetWorkloadsStatistics();
	bool preconditioning_required;
};

#endif // !HOST_SYSTEM_H
