#pragma once
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "ThreadedQueue.h"
#include "GOAPWorldState.h"


class GOAPActionBase;
struct GOAPPlan;
struct PlanData;


class GOAPPlanner
{
public:
	GOAPPlanner();
	~GOAPPlanner();
	void PopulateEffectMap(std::vector<GOAPActionBase*> const& actionList);

	//GOAPPlan & MakePlan(WorldStateProperty const& goalState);
	//GOAPPlan & Planning(WorldStateProperty const& goalState, WorldState const& worldState, std::vector<GOAPActionBase*> const& actionList);

	void StartThreaded(ThreadedQueue<PlanData> & dataQueue, size_t const& threadCount);

	inline void Stop() { m_bStopped.store(true); }
	void InputWake();

private:
	struct Plan
	{
		bool isComplete = false;
		GOAPPlan* data;
		WorldState* worldState;
		size_t cost = 0;
	};

	//void ThreadPlan(std::vector<Plan> & plans, Plan & preferredPlan, std::atomic<int> & accessing, size_t const& threadNumber, WorldStateProperty const& goalState);

	void Worker();

	void ResultWake();
	void WakeWorkers();

	void DefaultPreferred();

	std::atomic<bool> m_bStopped = false;
	std::atomic<bool> m_bDoingWork = false;
	std::atomic<size_t> m_nCompletedThreads = 0;
	size_t m_nThreadCount = 1;

	// Planner waiting on input
	std::mutex m_mxInputWake;
	std::condition_variable m_cvInputWake;
	volatile bool m_bInputWake;

	// Planner Waiting on result
	std::mutex m_mxResultWake;
	std::condition_variable m_cvResultWake;
	std::atomic<bool> m_bResultWake;

	// Sleeping Worker Variables
	std::mutex m_mxWakeWorker;
	std::condition_variable m_cvWakeWorker;
	std::atomic<bool> m_bWorkersAwake;

	// Worker requirements
	ThreadedQueue<Plan> m_planQueue;
	WorldState m_WorldState;
	WorldStateProperty* m_pGoalState;
	std::map<size_t, std::vector<GOAPActionBase*>>* m_pEffectMap;
	volatile Plan* m_pPreferredPlan;
	std::mutex m_mxPreferredPlan;
};

