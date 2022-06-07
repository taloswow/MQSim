// This file implements the event tree, which is used to store all future events for the engine
// This is implemented using the C++ map library function
// The key of each element is the timestamp at which it will be executed in simulation time, the
// data is the element itself.
// In case there are multiple elements with the same timestamp, they are linked via the pointer Next_event inside the data_element (which is of type SimEvent*)

#include "EventTree.h"

#include <exception>
#include <map>

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
		// If key is not yet in map, insert it
		if (map.find(key) == map.end())
		{
			map.insert({key, data});
		}
		// If key is in map, start at that element, and iterate through the Next_events
		// until we reach the end, then insert it
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
	int EventTree::Size()
	{
		return map.size();
	}

	// Returns the value of the minimum key
	sim_time_type EventTree::Get_min_key()
	{
		return map.begin()->first;
	}

	// removes the key and data object (delete)
	void EventTree::Remove(sim_time_type key)
	{
		map.erase(key);
	}


	/// Empties or clears the tree
	void EventTree::Clear()
	{
		map.clear();
	}
}
