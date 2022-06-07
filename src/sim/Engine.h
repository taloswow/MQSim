#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <unordered_map>

#include "Sim_Defs.h"
#include "Sim_Object.h"
#include "EventTree.h"

namespace MQSimEngine {
	class Engine
	{
	public:
		Engine();
		~Engine();
		static Engine* Instance();
		sim_time_type Time();
		Sim_Event* RegisterSimEvent(sim_time_type fireTime, Sim_Object* targetObject, void* parameters = NULL, int type = 0);
		void IgnoreSimEvent(Sim_Event*);
		void Reset();
		void AddObject(Sim_Object* obj);
		Sim_Object* GetObject(sim_object_id_type object_id);
		void RemoveObject(Sim_Object* obj);
		void StartSimulation();
		void StopSimulation();
		bool HasStarted();
		bool IsIntegratedExecutionMode();
	private:
		sim_time_type _sim_time;
		EventTree* _EventList;
		std::unordered_map<sim_object_id_type, Sim_Object*> _ObjectList;
		bool stop;
		bool started;
		static Engine* _instance;
	};
}

#define Simulator MQSimEngine::Engine::Instance()
#endif // !ENGINE_H
