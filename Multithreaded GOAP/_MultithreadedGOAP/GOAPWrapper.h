#pragma once
#include "GOAPPlanner.h"
#include "GOAPWorldState.h"
#include "ThreadedQueue.h"


struct GOAPPlan
{
	bool isSuccessful = false;
	std::vector<GOAPActionBase*> actions;
};

struct PlanData
{
	GOAPPlan* pResult;
	bool* pNotification;

	WorldState* pWorldState;
	WorldStateProperty* pGoalState;
	std::vector<GOAPActionBase*>* pActionList;
};

class GOAPWrapper
{
public:
	GOAPWrapper(size_t const& threadCount);
	~GOAPWrapper();

	void AddPlan(PlanData planData);
	
	inline size_t QueueLength() { return m_queue.Size(); };

private:
	GOAPPlanner* m_planner;
	ThreadedQueue<PlanData> m_queue;
	std::thread m_plannerThread;
};

