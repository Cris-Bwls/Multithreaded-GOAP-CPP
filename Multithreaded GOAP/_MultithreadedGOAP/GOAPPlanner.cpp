#include "GOAPPlanner.h"
#include "GOAPActionBase.h"
#include "GOAPWrapper.h"
#include <algorithm>
#include <thread>

using std::vector;

#define NO_ONE_ACCESSING -1

GOAPPlanner::GOAPPlanner()
{
	m_planQueue.SetSortFunc([](Plan const& lhs, Plan const& rhs) {return lhs.cost > rhs.cost; });
	m_pPreferredPlan = new Plan();
	DefaultPreferred();
	m_pEffectMap = new std::map<size_t, std::vector<GOAPActionBase*>>();
}


GOAPPlanner::~GOAPPlanner()
{
	delete m_pPreferredPlan;
}

void GOAPPlanner::PopulateEffectMap(std::vector<GOAPActionBase*> const& actionList)
{
	delete m_pEffectMap;
	m_pEffectMap = new std::map<size_t, std::vector<GOAPActionBase*>>();
	for (size_t i = 0; i < actionList.size(); ++i)
	{
		auto actionEffects = actionList[i]->GetEffectList();

		for (size_t j = 0; j < actionEffects.size(); ++j)
		{
			(*m_pEffectMap)[actionEffects[j]].push_back(actionList[i]);
		}
	}
}

//GOAPPlan GOAPPlanner::Planning(WorldStateProperty const & goalState, WorldState const & worldState, std::vector<GOAPActionBase*> const& actionList)
//{
//	ChangeWorldState(worldState);
//	PopulateEffectMap(actionList);
//	return MakePlan(goalState);
//}

void GOAPPlanner::StartThreaded(ThreadedQueue<PlanData> & dataQueue, size_t const & threadCount)
{
	// Start Threads
	vector<std::thread> threads;
	for (int i = 0; i < threadCount; ++i)
	{
		threads.push_back(std::thread(&GOAPPlanner::Worker, this));
	}

	// While Running
	while (!m_bStopped)
	{
		// Wait for work to do
		std::unique_lock<std::mutex> inputLock(m_mxInputWake);
		while (!m_bInputWake)
		{
			m_cvInputWake.wait(inputLock);
		}

		// While work in the queue
		while (!dataQueue.IsEmpty())
		{
			printf("Doing Work");

			// Get Work
			PlanData work; 
			bool exists = dataQueue.Pop(work);

			// If work is invalid skip
			if (!exists)
				continue;

			// Check if no plan is needed
			auto worldStateData = work.pWorldState->properties[work.pGoalState->nIdentifier].bData;
			auto goalStateData = work.pGoalState->bData;
			if (worldStateData == goalStateData)
			{
				work.pResult->isSuccessful = true;
				*(work.pNotification) = true;
				continue;
			}

			// Setup Planner with generalised data
			m_WorldState = (*(work.pWorldState));
			PopulateEffectMap(*(work.pActionList));

			m_pGoalState = work.pGoalState;

			// Add work to queue
			Plan plan;
			plan.isComplete = false;
			plan.cost = 0;
			plan.worldState = &m_WorldState;
			plan.data = &GOAPPlan();

			m_planQueue.PushSorted(plan);

			// Wake the workers
			WakeWorkers();

			// Wait for final result from workers
			std::unique_lock<std::mutex> resultLock(m_mxResultWake);
			while (!m_bResultWake)
			{
				m_cvResultWake.wait(resultLock);
			}

			// Send off result
			m_mxPreferredPlan.lock();
			work.pResult = m_pPreferredPlan->data;
			m_mxPreferredPlan.unlock();

			// Reset Preferred
			DefaultPreferred();
		}

		m_bInputWake = false;
	}

	return;
}

void GOAPPlanner::InputWake()
{
	std::unique_lock<std::mutex> inputLock(m_mxInputWake);
	m_bInputWake = true;
	m_cvInputWake.notify_all();
	printf("INPUT WAKE\n");
}

//GOAPPlan GOAPPlanner::MakePlan(WorldStateProperty const& goalState)
//{
//	Plan preferredPlan;
//	preferredPlan.cost = INT_MAX;
//
//	vector<vector<Plan>> threadedPlans;
//	for (int i = 0; i < m_nThreadCount; ++i)
//	{
//		threadedPlans.push_back(vector<Plan>());
//	}
//
//	auto worldStateData = m_WorldState.properties[goalState.nIdentifier].bData;
//	auto goalStateData = goalState.bData;
//
//	// Check if goal state is current world state (No actions needed)
//	if (worldStateData == goalStateData)
//		return preferredPlan.data;
//
//	// Get Initial Actions that cause required effect
//	auto currentEffectActions = m_EffectMap[goalState.nIdentifier];
//
//	// Assign those actions to a plan each and add that to a thread workload
//	for (size_t i = 0; i < currentEffectActions.size(); ++i)
//	{
//		auto pAction = currentEffectActions[i];
//
//		// Plan for action
//		Plan currentPlan;
//		currentPlan.isComplete = false;
//		currentPlan.cost = pAction->GetCost();
//		currentPlan.worldState = m_WorldState;
//		currentPlan.data.actions.push_back(pAction);
//
//		// Add to thread workload and sort by cost
//		int nThread = i % m_nThreadCount;
//		threadedPlans[nThread].push_back(currentPlan);
//		std::push_heap(threadedPlans[nThread].begin(), threadedPlans[nThread].end(), 
//			[](Plan const& lhs, Plan const& rhs) {return lhs.cost < rhs.cost; });
//	}
//
//	vector<std::thread> threads;
//
//	// Bad Threading
//	/*
//	for (int i = 1; i < m_nThreadCount; ++i)
//	{
//		// START THREADS HERE
//		threads.push_back(std::thread(&GOAPPlanner::ThreadPlan, this, std::ref(threadedPlans[i]), 
//										std::ref(preferredPlan), std::ref(accessing), 
//										std::ref(i), std::ref(goalState)));
//	}
//	*/
//
//	//MAIN THREAD
//	ThreadPlan(threadedPlans[0], preferredPlan, accessing, 0, goalState);
//
//	// JOIN THREADS HERE
//	for (int i = 0; i < threads.size(); ++i)
//	{
//		threads[i].join();
//	}
//
//	// TEMP
//	return preferredPlan.data;
//}

//void GOAPPlanner::ThreadPlan(std::vector<Plan>& plans, Plan & preferredPlan, atomic<int> & accessing, size_t const& threadCount, WorldStateProperty const& goalState)
//{	
//	// While there are incomplete plans
//	size_t completedPlans = 0;
//	while (completedPlans < plans.size())
//	{
//		// Sort plans by  
//		std::sort(plans.begin(), plans.end(), 
//			[](Plan const& lhs, Plan const& rhs) {return lhs.cost < rhs.cost; });
//
//		//Work on lowest cost plan
//		Plan* currentPlan = &plans[0];
//
//		// IF COST GREATER THAN PREFFERED
//		if (preferredPlan.cost < currentPlan->cost)
//			return;
//
//		// PLAN COMPLETE
//		if (currentPlan->isComplete)
//		{
//			bool waiting = true;
//			while (waiting)
//			{
//				// Get Access
//				if (accessing.load(std::memory_order::memory_order_acquire) == NO_ONE_ACCESSING)
//				{
//					// Lock Access
//					accessing.store(threadCount, std::memory_order::memory_order_release);
//
//					// Check Preferred
//
//					// IF current plan is cheaper replace preferred plan
//					if (preferredPlan.cost > currentPlan->cost)
//					{
//						preferredPlan = *currentPlan;
//					}
//					// ELIF cost is equal
//					else if (preferredPlan.cost == currentPlan->cost)
//					{
//						// IF current plan is shorter replace preferred plan
//						if (preferredPlan.data.actions.size() > currentPlan->data.actions.size())
//							preferredPlan = *currentPlan;
//					}
//
//					// Unlock Access
//					accessing.store(NO_ONE_ACCESSING, std::memory_order::memory_order_release);
//
//					waiting = false;
//				}
//
//			}
//			// EARLY EXIT (Not sure if optimal)
//			return;
//		}
//
//		// Reset plan world state
//		WorldState planWorldState = m_WorldState;
//
//		// For each of the actions in the plan
//		bool loopActions = true;
//		int actionCount = currentPlan->data.actions.size() - 1;
//		while (actionCount >= 0 && loopActions)
//		{
//			auto currentAction = currentPlan->data.actions[actionCount];
//
//			// Get Preconditions
//			auto preconditions = currentAction->GetPreConditionList();
//			bool allSatisfied = false;
//
//			vector<WorldStateProperty> requiredEffects;
//
//			// For each Precondition
//			for (int i = 0; i < preconditions.size(); ++i)
//			{
//				size_t nIdent = preconditions[i].nIdentifier;
//				bool neededData = preconditions[i].bData;
//
//				// Get related data from the world state
//				bool worldData = planWorldState.properties[nIdent].bData;
//
//				// IF precondition is satisfied
//				if (neededData == worldData)
//				{
//					allSatisfied = true;
//				}
//				// Precondition needs to be satisfied
//				else
//				{
//					allSatisfied = false;
//
//					// Add to Required effect list
//					requiredEffects.push_back(preconditions[i]);
//				}
//			} // END precondition check loop
//
//			// Action Ready to be used
//			if (allSatisfied)
//			{
//				// Add all effects onto plan world state
//				auto effects = currentAction->GetEffectList();
//				for (int i = 0; i < effects.size(); ++i)
//				{
//					size_t effectIdent = effects[i];
//					planWorldState.properties[effectIdent].bData = true;
//				}
//
//				// Continue to next action
//				actionCount--;
//				continue;
//			}
//			// List of new plans
//			vector<Plan> newPlans;
//			
//			// Add actions for all required effects
//			//for (int i = 0; i < requiredEffects.size(); ++i)
//			if (requiredEffects.size() > 0)
//			{
//				size_t ident = requiredEffects[0].nIdentifier;
//				auto actions = m_EffectMap[ident];
//				
//				// There are no actions to take
//				if (!(actions.size() > 0))
//				{
//					// Plan can not be completed
//					currentPlan->cost = INT_MAX;
//					currentPlan->isComplete = true;
//					completedPlans++;
//				}
//				for (int j = 1; j < actions.size(); ++j)
//				{
//						// actions assigned to new plans
//						Plan newPlan;
//						newPlan.isComplete = false;
//						newPlan.data.isSuccessful = false;
//
//						for (int k = 0; k < currentPlan->data.actions.size(); k++)
//						{
//							newPlan.data.actions.push_back(currentPlan->data.actions[k]);
//						}
//
//						newPlan.data.actions.push_back(actions[j]);
//						newPlan.cost = currentPlan->cost + actions[j]->GetCost();
//
//						// Add newPlan to list of plans
//						newPlans.push_back(newPlan);
//				}
//
//				// First action assigned to this plan
//				currentPlan->data.actions.push_back(actions[0]);
//				currentPlan->cost += actions[0]->GetCost();
//
//				// New Actions so break out of action loop
//				loopActions = false;
//			}
//			
//			for (int i = 0; i < newPlans.size(); ++i)
//			{
//				plans.push_back(newPlans[i]);
//			}
//		}// END Action loop
//
//		// Check if plan completes the goal state
//		bool goalData = goalState.bData;
//		bool planData = planWorldState.properties[goalState.nIdentifier].bData;
//		if (goalData == planData)
//		{
//			currentPlan->isComplete = true;
//			currentPlan->data.isSuccessful = true;
//		}
//
//	}// END Plan Loop
//}

void GOAPPlanner::Worker()
{
	bool isComplete = false;

	// While Running
	while (!m_bStopped)
	{
		// Wait for work to do
		std::unique_lock<std::mutex> workerLock(m_mxWakeWorker);
		while (!m_bWorkersAwake.load())
		{
			m_cvWakeWorker.wait(workerLock);
		}
		// While work in the queue
		while (!m_planQueue.IsEmpty())
		{
			isComplete = false;

			// Get Work
			Plan pCurrent;
			bool exists = m_planQueue.Pop(pCurrent);

			// IF current is invalid
			if (!exists)
				continue;

			// IF cost greater than preferred
			if (m_pPreferredPlan->cost < pCurrent.cost)
				continue;
			

			// Reset plan world state
			WorldState planWorldState = m_WorldState;

			// For each of the actions in the plan
			bool loopActions = true;
			int actionCount = pCurrent.data->actions.size() - 1;
			while (actionCount >= 0 && loopActions)
			{
				auto currentAction = pCurrent.data->actions[actionCount];

				// Get Preconditions
				auto preconditions = currentAction->GetPreConditionList();
				bool allSatisfied = false;

				vector<WorldStateProperty> requiredEffects;

				// For each Precondition
				for (int i = 0; i < preconditions.size(); ++i)
				{
					size_t nIdent = preconditions[i].nIdentifier;
					bool neededData = preconditions[i].bData;

					// Get related data from the world state
					bool worldData = planWorldState.properties[nIdent].bData;

					// IF precondition is satisfied
					if (neededData == worldData)
					{
						allSatisfied = true;
					}
					// Precondition needs to be satisfied
					else
					{
						allSatisfied = false;

						// Add to Required effect list
						requiredEffects.push_back(preconditions[i]);
					}
				} // END precondition check loop

				// Action Ready to be used
				if (allSatisfied)
				{
					// Add all effects onto plan world state
					auto effects = currentAction->GetEffectList();
					for (int i = 0; i < effects.size(); ++i)
					{
						size_t effectIdent = effects[i];
						planWorldState.properties[effectIdent].bData = true;
					}

					// Continue to next action
					actionCount--;
					continue;
				}
				// List of new plans
				vector<Plan> newPlans;

				// Add actions for next required effect
				if (requiredEffects.size() > 0)
				{
					size_t ident = requiredEffects[0].nIdentifier;
					auto actions = (*m_pEffectMap)[ident];

					// There are no actions to take
					if (!(actions.size() > 0))
					{
						// Plan can not be completed
						pCurrent.cost = INT_MAX;
						pCurrent.isComplete = true;
					}
					for (int j = 1; j < actions.size(); ++j)
					{
						// actions assigned to new plans
						Plan newPlan;
						newPlan.isComplete = false;
						newPlan.data->isSuccessful = false;

						for (int k = 0; k < pCurrent.data->actions.size(); k++)
						{
							newPlan.data->actions.push_back(pCurrent.data->actions[k]);
						}

						newPlan.data->actions.push_back(actions[j]);
						newPlan.cost = pCurrent.cost + actions[j]->GetCost();

						// Add newPlan to list of plans
						newPlans.push_back(newPlan);
					}

					// First action assigned to this plan
					pCurrent.data->actions.push_back(actions[0]);
					pCurrent.cost += actions[0]->GetCost();

					// New Actions so break out of action loop
					loopActions = false;
				}

				for (int i = 0; i < newPlans.size(); ++i)
				{
					m_planQueue.PushSorted(newPlans[i]);

					// Reset signouts and wake workers
					m_nCompletedThreads.store(0);
					WakeWorkers();

					printf("RESET SIGNOUTS\n");
				}
			}// END Action loop

			// Check if plan completes the goal state
			bool goalData = m_pGoalState->bData;
			bool planData = planWorldState.properties[m_pGoalState->nIdentifier].bData;
			if (goalData == planData)
			{
				pCurrent.isComplete = true;
				pCurrent.data->isSuccessful = true;

				std::lock_guard<std::mutex> preferred(m_mxPreferredPlan);

				// IF current plan is cheaper replace preferred plan
				if (m_pPreferredPlan->cost > pCurrent.cost)
				{
					m_pPreferredPlan = &pCurrent;
				}
				// ELIF cost is equal
				else if (m_pPreferredPlan->cost == pCurrent.cost)
				{
					// IF current plan is shorter replace preferred plan
					if (m_pPreferredPlan->data->actions.size() > pCurrent.data->actions.size())
						m_pPreferredPlan = &pCurrent;
				}
			}
		}
		if (!isComplete)
		{
			// Sign out
			m_bWorkersAwake.store(false);
			m_nCompletedThreads.fetch_add(1);
			isComplete = true;
			printf("signed out %i\n", m_nCompletedThreads.load());

			// All threads finished
			if (m_nCompletedThreads.load() == m_nThreadCount)
			{
				// Wake the planner
				ResultWake();
			}
		}
	}

	return;
}

void GOAPPlanner::ResultWake()
{
	std::unique_lock<std::mutex> resultLock(m_mxResultWake);
	m_bResultWake.store(true);
	m_cvResultWake.notify_all();
	printf("RESULT WAKE\n");
}

void GOAPPlanner::WakeWorkers()
{
	std::unique_lock<std::mutex> workerLock(m_mxWakeWorker);
	m_bWorkersAwake.store(true);
	m_cvWakeWorker.notify_all();
	printf("WAKE WORKERS\n");
}

void GOAPPlanner::DefaultPreferred()
{
	std::lock_guard<std::mutex> preferredLock(m_mxPreferredPlan);
	m_pPreferredPlan->isComplete = false;
	m_pPreferredPlan->worldState = nullptr;
	m_pPreferredPlan->data = nullptr;
	m_pPreferredPlan->cost = SIZE_MAX;
}
