#ifndef SIMULATOR_OBJECT_H
#define SIMULATOR_OBJECT_H

#include <string>
#include "Sim_Event.h"

namespace MQSimEngine
{
	class Sim_Event;
	class Sim_Object
	{
	public:
		// Initialization of a Sim_Object
		// set triggers to False
		Sim_Object(const sim_object_id_type &id)
		{
			_id = id;
			_triggersSetUp = false;
		}

		sim_object_id_type ID()
		{
			return this->_id;
		}

		bool IsTriggersSetUp()
		{
			return _triggersSetUp;
		}

		// The Start function is invoked at the start phase
		// of simulation to perform initialization
		virtual void StartSimulation() = 0;

		// The ValidateSimulationConfig function is invoked
		// to check if the objected is correctly configured or not.
		virtual void ValidateSimulationConfig() = 0;

		// The object connects its internal functions to the outside
		// triggering events from other objects
		virtual void Setup_triggers()
		{
			_triggersSetUp = true;
		}
		
		virtual void ExecuteSimulatorEvent(Sim_Event*) = 0;
		 
	private:
		sim_object_id_type _id;
		bool _triggersSetUp;
	};
}

#endif // !SIMULATOR_OBJECT_H
