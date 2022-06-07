// The Engine is the main part running the actual simulation

#include <stdexcept>
#include "Engine.h"
#include "../utils/Logical_Address_Partitioning_Unit.h"

namespace MQSimEngine
{
	Engine* Engine::_instance = NULL;

	// Constuctor. Create EventTree as EventList
	Engine::Engine() {
		this->_EventList = new EventTree;
		started = false;
	}

	// Destructor. Delete EventList
	Engine::~Engine() {
		delete _EventList;
	}

	// If there is not yet an instance of the Engine, create one
	Engine* Engine::Instance() {
		if (_instance == 0) {
			_instance = new Engine;
		}
		return _instance;
	}

	// Reset the engine by setting values to false and clearing the EventTree
	void Engine::Reset()
	{
		_EventList->Clear();
		_ObjectList.clear();
		_sim_time = 0;
		stop = false;
		started = false;
		Utils::Logical_Address_Partitioning_Unit::Reset();
	}


	// Add an object to the simulator object list
	void Engine::AddObject(Sim_Object* obj)
	{
		if (_ObjectList.find(obj->ID()) != _ObjectList.end()) {
			throw std::invalid_argument("Duplicate object key: " + obj->ID());
		}
		_ObjectList.insert(std::pair<sim_object_id_type, Sim_Object*>(obj->ID(), obj));
	}
	
	// Get an object out of the simulator object list
	Sim_Object* Engine::GetObject(sim_object_id_type object_id)
	{
		auto itr = _ObjectList.find(object_id);
		if (itr == _ObjectList.end()) {
			return NULL;
		}

		return (*itr).second;
	}

	// Remove an object out of the simulator object list
	void Engine::RemoveObject(Sim_Object* obj)
	{
		std::unordered_map<sim_object_id_type, Sim_Object*>::iterator it = _ObjectList.find(obj->ID());
		if (it == _ObjectList.end()) {
			throw std::invalid_argument("Removing an unregistered object.");
		}
		_ObjectList.erase(it);
	}

	/// This is the main method of simulator which starts simulation process.
	void Engine::StartSimulation()
	{
		started = true;

		// Check all elements before executing
		for (std::unordered_map<sim_object_id_type, Sim_Object*>::iterator obj = _ObjectList.begin();
			obj != _ObjectList.end();
			++obj) {
			// Set triggers if not yet done
			if (!obj->second->IsTriggersSetUp()) {
				obj->second->Setup_triggers();
			}

			obj->second->Validate_simulation_config();
			obj->second->StartSimulation();
		}
			
		while (true) {
			// If EventList is empty or the stop flag has been set, end execution
			if (_EventList->Size() == 0 || stop) {
				break;
			}
			
			// Get the event which is next to execute
			sim_time_type key = _EventList->Get_min_key();
			Sim_Event* ev = _EventList->GetData(key);

			_sim_time = ev->Fire_time;

			// In case there are multiple events at the same timestamp, iterate through them and execute all
			while (ev != NULL)
			{
				if(!ev->Ignore) {
					ev->Target_sim_object->Execute_simulator_event(ev);
				}

				ev = ev->Next_event;
			}

			_EventList->Remove(key);
		}
	}

	// Set the stop flag, so that the simulation stops after finishing the current event
	void Engine::StopSimulation()
	{
		stop = true;
	}

	// Returns true if the simulation has started
	bool Engine::HasStarted()
	{
		return started;
	}

	// Returns the simulation time
	sim_time_type Engine::Time()
	{
		return _sim_time;
	}

	// Register a new sim_event and pack it into the EventTree
	Sim_Event* Engine::RegisterSimEvent(sim_time_type fireTime, Sim_Object* targetObject, void* parameters, int type)
	{
		Sim_Event* ev = new Sim_Event(fireTime, targetObject, parameters, type);
		DEBUG("RegisterEvent " << fireTime << " " << targetObject)
		_EventList->Insert_sim_event(ev);
		return ev;
	}

	// Set the flag for an event to ignore / not execute it
	void Engine::IgnoreSimEvent(Sim_Event* ev)
	{
		ev->Ignore = true;
	}

	bool Engine::IsIntegratedExecutionMode()
	{
		return false;
	}
}

