#ifndef EXECUTION_PARAMETER_SET_H
#define EXECUTION_PARAMETER_SET_H

#include <vector>

#include "Parameter_Set_Base.h"
#include "Device_Parameter_Set.h"
#include "IO_Flow_Parameter_Set.h"
#include "Host_Parameter_Set.h"

class Execution_Parameter_Set : public Parameter_Set_Base
{
public:
	static Host_Parameter_Set Host_Configuration;
	static Device_Parameter_Set SSD_Device_Configuration;

	void XMLSerialize(Utils::XmlWriter& xmlwriter);
	void XMLDeserialize(rapidxml::xml_node<> *node);
};

#endif // !EXECUTION_PARAMETER_SET_H
