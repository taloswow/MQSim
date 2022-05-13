#include <exception>
#include <map>
#include "EventTree.h"
#include "Engine.h"

namespace MQSimEngine
{
	class Engine;

	std::map<sim_time_type, Sim_Event*> map;
	
	EventTree::EventTree()
	{
		return;
	}
	EventTree::~EventTree()
	{
	 	delete &map;
	}

	// Inserts a new element into the EventTree
	void EventTree::Add(sim_time_type key, Sim_Event* data)
	{
		if (map.find(key) == map.end())
		{
			map.insert({key, data});
		}
		else
		{
			Sim_Event* tmp = map.find(key)->second;
			while (tmp->Next_event != NULL)
			{
				tmp = tmp->Next_event;
			}
			tmp->Next_event = data;
		}

	}

	// Gets the Sim_Event connected to the key
	Sim_Event* EventTree::GetData(sim_time_type key)
	{
		auto search = map.find(key);
		return search->second;
	}

	// Extracts the fire time of an event and inserts it into the EventTree
	void EventTree::Insert_sim_event(Sim_Event* event)
	{
		if (event->Fire_time < Engine::Instance()->Time()) {
			PRINT_ERROR("Illegal request to register a simulation event before Now!")
		}

		Add(event->Fire_time, event);
	}

	// Returns the size of the Event_Tree
	int EventTree::size()
	{
		return map.size();
	}

	// Returns the value of the minimum key
	sim_time_type EventTree::Get_min_key()
	{
		return map.begin()->first;
	}

	// Returns the object with the minimum key value
	Sim_Event* EventTree::Get_min_value()
	{
		return map.begin()->second;
	}

	// removes the key and data object (delete)
	void EventTree::Remove(sim_time_type key)
	{
		map.erase(key);
	}

	// removes the entry with the minimum key
	void EventTree::Remove_min()
	{
		EventTree::Remove(Get_min_key());
	}

	/// Empties or clears the tree
	void EventTree::Clear()
	{
		map.clear();
	}
}
