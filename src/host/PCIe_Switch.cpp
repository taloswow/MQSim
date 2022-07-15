#include "PCIe_Switch.h"

namespace Host_Components
{
	PCIe_Switch::PCIe_Switch(PCIe_Link* pcie_link, SSD_Components::Host_Interface_Base* host_interface) :
		pcie_link(pcie_link), host_interface(host_interface)
	{
	}

	void PCIe_Switch::DeliverToDevice(PCIe_Message* message)
	{
		host_interface->ConsumePCIeMessage(message);
	}

	void PCIe_Switch::SendToHost(PCIe_Message* message)
	{
		pcie_link->Deliver(message);
	}
	
	void PCIe_Switch::AttachSSDDevice(SSD_Components::Host_Interface_Base* host_interface)
	{
		this->host_interface = host_interface;
	}

	bool PCIe_Switch::IsSSDConnected()
	{
		return this->host_interface != NULL;
	}
}
