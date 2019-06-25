#pragma once
#include "GOAPPlanner.h"
#include "GOAPWorldState.h"
#include "ThreadedQueue.h"
#include "GOAPPlan.h"
#include <future>


typedef WorldStateProperty GoalState;
typedef std::vector<GOAPActionBase*> ActionList;

/*
	Data structure that holds all relevant information for a plan
*/
struct PlanData
{
	std::future<GOAPPlan>* pResult;
	WorldState* pWorldState;
	GoalState* pGoalState;
	ActionList* pActionList;
};

class GOAPWrapper
{
public:
	GOAPWrapper(size_t const& threadCount);
	~GOAPWrapper();

	void AddPlan(PlanData* planData);
	
	inline size_t QueueLength() { return m_queue.Size(); };

private:
	GOAPPlanner* m_planner;
	ThreadedQueue<PlanData*> m_queue;
	std::thread m_plannerThread;
	size_t m_nThreadCount;
};

