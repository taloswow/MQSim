#ifndef EVENT_TREE_H
#define EVENT_TREE_H

#include "Sim_Defs.h"
#include "Sim_Event.h"

namespace MQSimEngine
{

	class EventTree
	{
	public:
		EventTree();
		~EventTree();

		int size();
		void Add(sim_time_type key, Sim_Event* data);
		Sim_Event* GetData(sim_time_type key);
		void Insert_sim_event(Sim_Event* data);
		sim_time_type Get_min_key();
		Sim_Event* Get_min_value();
		void Remove(sim_time_type key);
		void Remove_min();
		void Clear();
	};
}

#endif // !EVENT_TREE_H
