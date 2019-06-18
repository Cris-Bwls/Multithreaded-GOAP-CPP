#include "GOAPPlanner.h"
#include "GOAPActionBase.h"
#include "GOAPWrapper.h"
#include <algorithm>
#include <thread>
#include <future>

#include <iostream>

using std::vector;

#define DEBUG false
static int test = 0;

GOAPPlanner::GOAPPlanner()
{
	m_planQueue.SetSortFunc([](Plan const& lhs, Plan const& rhs) {return lhs.cost > rhs.cost; });
	m_preferredPlan;
	m_preferredPlan.cost = 0;
	DefaultPreferred();
	m_pEffectMap = new std::map<size_t, std::vector<GOAPActionBase*>>();
}


GOAPPlanner::~GOAPPlanner()
{
	if (m_pEffectMap)
		delete m_pEffectMap;
}

/*
	Populates the effect map with the given Actions
*/
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

/*
	Starts the master thread that processes plan requests 
	and gives initial data to the workers,
	also gives back the resulting plan,
	stop the thread by calling Stop()
*/
void GOAPPlanner::StartThreaded(ThreadedQueue<PlanData*> & dataQueue, size_t const & threadCount)
{
	// Start Threads
	m_nThreadCount = threadCount;
	vector<std::thread> threads;
	for (int i = 0; i < m_nThreadCount; ++i)
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
			m_bResultWake.store(false);

			if (DEBUG)
				printf("Doing Work\n");

			// Get Work
			PlanData* pWork; 
			bool exists = dataQueue.Pop(pWork);
			std::promise<GOAPPlan> promisedData;
			*(pWork->pResult) = promisedData.get_future();

			// If work is invalid skip
			if (!exists)
				continue;

			// Check if no plan is needed
			auto worldStateData = pWork->pWorldState->properties[pWork->pGoalState->nIdentifier].bData;
			auto goalStateData = pWork->pGoalState->bData;
			if (worldStateData == goalStateData)
			{
				GOAPPlan plan;
				plan.isSuccessful = true;

				promisedData.set_value(plan);
				continue;
			}

			// Setup Planner with generalised data
			m_WorldState = (*(pWork->pWorldState));
			PopulateEffectMap(*(pWork->pActionList));

			m_pGoalState = pWork->pGoalState;

			// Get Initial Actions that cause required effect
			ActionList currentEffectActions = (*m_pEffectMap)[m_pGoalState->nIdentifier];
		
			// Assign those actions to a plan each and add that to a thread workload
			for (size_t i = 0; i < currentEffectActions.size(); ++i)
			{
				auto pAction = currentEffectActions[i];

				// Add work to queue
				Plan plan;
				plan.isComplete = false;
				plan.cost = pAction->GetCost();
				plan.worldState = &m_WorldState;
				plan.data.actions.push_back(pAction);

				m_planQueue.PushSorted(plan);
			}
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
			promisedData.set_value(m_preferredPlan.data);
			if (DEBUG)
			{
				if (m_preferredPlan.data.isSuccessful)
				{
					printf("Success\n");
				}
				else
				{
					printf("FAILURE\n");
				}
			}
			m_mxPreferredPlan.unlock();

			// Reset Preferred
			DefaultPreferred();
		}

		m_bInputWake = false;
	}

	// join workers before ending
	for (int i = 0; i < threads.size(); ++i)
	{
		threads[i].join();
	}
	if (DEBUG)
		printf("Planner Joined");
	return;
}

/*
	Stops the threads
*/
void GOAPPlanner::Stop()
{
	m_bStopped.store(true);

	// Wakes any sleeping workers so they stop
	WakeWorkers();
}

/*
	Wakes the Planner if it is waiting for input
*/
void GOAPPlanner::InputWake()
{
	std::unique_lock<std::mutex> inputLock(m_mxInputWake);
	m_bInputWake = true;
	m_cvInputWake.notify_all();
	if(DEBUG)
		printf("INPUT WAKE\n");
}

/*
	Works on the plan one piece at a time,
	adding pieces if required and telling the master thread when a plan is complete
*/
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

		if (workerLock.owns_lock())
			workerLock.unlock();

		// While work in the queue
		while (!m_planQueue.IsEmpty())
		{
			isComplete = false;

			// Get Work
			Plan current;
			bool exists = m_planQueue.Pop(current);

			// IF current is invalid
			if (!exists)
			{
				if (DEBUG)
					printf("Current is Invalid, Skipping #%i\n", std::this_thread::get_id());
				continue;
			}

			// IF preferred plan is cheaper
			if (m_preferredPlan.cost < current.cost)
			{
				if (DEBUG)
					printf("Preferred is cheaper, Skipping #%i\n", std::this_thread::get_id());
				continue;
			}
			

			// Reset plan world state
			WorldState planWorldState = m_WorldState;

			// For each of the actions in the plan
			bool loopActions = true;
			int actionCount = current.data.actions.size() - 1;
			while (actionCount >= 0 && loopActions)
			{
				auto currentAction = current.data.actions[actionCount];

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
					ActionList actions = (*m_pEffectMap)[ident];

					// There are no actions to take
					if ((actions.size() == 0))
					{
						printf("No Actions to fulfill effect, Skipping #%i\n", std::this_thread::get_id());

						// Plan can not be completed
						current.cost = INT_MAX;
						current.isComplete = true;
						loopActions = false;
						continue;
					}
					for (int j = 1; j < actions.size(); ++j)
					{
						// actions assigned to new plans
						Plan newPlan;
						newPlan.isComplete = false;
						newPlan.data.isSuccessful = false;

						for (int k = 0; k < current.data.actions.size(); k++)
						{
							newPlan.data.actions.push_back(current.data.actions[k]);
						}

						newPlan.data.actions.push_back(actions[j]);
						newPlan.cost = current.cost + actions[j]->GetCost();

						// Add newPlan to list of plans
						newPlans.push_back(newPlan);
					}

					// First action assigned to this plan
					current.data.actions.push_back(actions[0]);
					current.cost += actions[0]->GetCost();

					// New Actions so break out of action loop
					loopActions = false;
				}

				for (int i = 0; i < newPlans.size(); ++i)
				{
					m_planQueue.PushSorted(newPlans[i]);

					// Reset signouts and wake workers
					m_nCompletedThreads.store(0);

					WakeWorkers();

					if (DEBUG)
						printf("RESET SIGNOUTS #%i\n", std::this_thread::get_id());
				}
			}// END Action loop

			// Check if plan completes the goal state
			bool goalData = m_pGoalState->bData;
			bool planData = planWorldState.properties[m_pGoalState->nIdentifier].bData;
			if (goalData == planData)
			{
				current.isComplete = true;
				current.data.isSuccessful = true;

				std::lock_guard<std::mutex> preferred(m_mxPreferredPlan);

				// IF current plan is cheaper replace preferred plan
				if (m_preferredPlan.cost > current.cost)
				{
					m_preferredPlan = current;
				}
				// ELIF cost is equal
				else if (m_preferredPlan.cost == current.cost)
				{
					// IF current plan is shorter replace preferred plan
					if (m_preferredPlan.data.actions.size() > current.data.actions.size())
						m_preferredPlan = current;
				}
			}
			else
			{
				m_planQueue.PushSorted(current);
			}

			// TEST
			if (DEBUG)
			{
				printf("PLAN #%i\n", std::this_thread::get_id());
				for (int i = 0; i < current.data.actions.size(); ++i)
				{
					printf("%i - %s\n", i, current.data.actions[i]->GetName());
				}
				printf("Remaining Plans in queue = %i\n", m_planQueue.Size());
				printf("\n");
			}
		}
		if (!isComplete)
		{
			// Sign out
			m_bWorkersAwake.store(false);
			m_nCompletedThreads.fetch_add(1);
			test++;
			isComplete = true;

			if (DEBUG)
				printf("signed out %i #%i\n", m_nCompletedThreads.load(), std::this_thread::get_id());

			// All threads finished
			if (m_nCompletedThreads.compare_exchange_strong(m_nThreadCount, 0))
			//if (m_nCompletedThreads.load() == m_nThreadCount)
			{
				// Wake the planner
				ResultWake();
				//m_nCompletedThreads.store(0);
			}
		}
	}

	// Join the thread
	if (DEBUG)
		printf("Worker Joined #%i\n", std::this_thread::get_id());

	// Wake the master thread, regardless of where it was waiting so it can join
	InputWake();
	ResultWake();
	return;
}

/*
	Wakes the planner if it is waiting for the result
*/
void GOAPPlanner::ResultWake()
{
	std::lock_guard<std::mutex> resultLock(m_mxResultWake);
	m_bResultWake.store(true);
	m_cvResultWake.notify_all();
	if (DEBUG)
		printf("RESULT WAKE #%i\n", std::this_thread::get_id);
}

/*
	Wakes the workers when there is a data for them to work on
*/
void GOAPPlanner::WakeWorkers()
{
	std::lock_guard<std::mutex> workerLock(m_mxWakeWorker);

	m_bWorkersAwake.store(true);
	m_cvWakeWorker.notify_all();
	if (DEBUG)
		printf("WAKE WORKERS #%i\n", std::this_thread::get_id());
}

/*
	Puts the preferred plan back to default settings
*/
void GOAPPlanner::DefaultPreferred()
{
	std::lock_guard<std::mutex> preferredLock(m_mxPreferredPlan);
	m_preferredPlan.isComplete = false;
	m_preferredPlan.worldState = nullptr;
	m_preferredPlan.data.actions.clear();
	m_preferredPlan.data.isSuccessful = false;
	m_preferredPlan.cost = SIZE_MAX;
}
