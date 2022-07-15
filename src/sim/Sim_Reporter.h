#ifndef SIM_REPORTER_H
#define SIM_REPORTER_H

#include <string>

#include "../utils/XMLWriter.h"

namespace MQSimEngine
{
	class Sim_Reporter
	{
	public:
		virtual void ReportResultsInXML(std::string name_prefix, Utils::XmlWriter& xmlwriter) = 0;
	};
}

#endif // !SIM_REPORTER_H
