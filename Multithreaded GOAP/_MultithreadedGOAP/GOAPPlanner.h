#pragma once
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "ThreadedQueue.h"
#include "GOAPWorldState.h"
#include "GOAPPlan.h"

class GOAPActionBase;
struct PlanData;

class GOAPPlanner
{
public:
	GOAPPlanner();
	~GOAPPlanner();
	void PopulateEffectMap(std::vector<GOAPActionBase*> const& actionList);
	
	void StartThreaded(ThreadedQueue<PlanData*> & dataQueue, size_t threadCount);

	void Stop();
	void InputWake();

private:
	/*
		Internal data structure for workers to work on
	*/
	struct Plan
	{
		bool isComplete = false;
		GOAPPlan data;
		WorldState* worldState;
		size_t cost = 0;
	};

	void Worker();

	void ResultWake();
	void WakeWorkers();

	void DefaultPreferred();

	std::atomic<bool> m_bStopped = false;
	std::atomic<bool> m_bDoingWork = false;
	std::atomic<size_t> m_nCompletedThreads = 0;
	std::atomic<size_t> m_nAwakeWorkers = 0;
	size_t m_nThreadCount = 1;

	// Planner waiting on input
	std::mutex m_mxInputWake;
	std::condition_variable m_cvInputWake;
	volatile bool m_bInputWake;

	// Planner Waiting on result
	std::mutex m_mxResultWake;
	std::condition_variable m_cvResultWake;
	std::atomic<bool> m_bResultWake;

	// Worker Waiting on data
	std::mutex m_mxWakeWorker;
	std::condition_variable m_cvWakeWorker;
	std::atomic<bool> m_bWorkersAwake;

	// Worker requirements
	ThreadedQueue<Plan> m_planQueue;
	WorldState m_WorldState;
	WorldStateProperty* m_pGoalState;
	std::map<size_t, std::vector<GOAPActionBase*>>* m_pEffectMap;
	Plan m_preferredPlan;
	std::mutex m_mxPreferredPlan;
};

