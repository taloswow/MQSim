#include "Execution_Parameter_Set.h"

Host_Parameter_Set Execution_Parameter_Set::Host_Configuration;
Device_Parameter_Set Execution_Parameter_Set::SSD_Device_Configuration;

void Execution_Parameter_Set::XMLSerialize(Utils::XmlWriter& xmlwriter)
{
	std::string tmp;
	tmp = "Execution_Parameter_Set";
	xmlwriter.WriteOpenTag(tmp);

	Host_Configuration.XMLSerialize(xmlwriter);
	SSD_Device_Configuration.XMLSerialize(xmlwriter);

	xmlwriter.WriteCloseTag();
}

void Execution_Parameter_Set::XMLDeserialize(rapidxml::xml_node<> *node)
{
	try {
		for (auto param = node->first_node(); param; param = param->next_sibling()) {
			if (strcmp(param->name(), "Host_Parameter_Set") == 0) {
				Host_Configuration.XMLDeserialize(param);
			} else if (strcmp(param->name(), "Device_Parameter_Set") == 0) {
				SSD_Device_Configuration.XMLDeserialize(param);
			}
		}
	} catch (...) {
		PRINT_ERROR("Error in the Execution_Parameter_Set!")
	}
}
