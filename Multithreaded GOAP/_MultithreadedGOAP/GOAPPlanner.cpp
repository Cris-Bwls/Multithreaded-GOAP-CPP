#include "GOAPPlanner.h"
#include "GOAPActionBase.h"
#include <algorithm>
#include <thread>

using std::vector;
using std::atomic;
typedef unsigned int uint;

#define NO_ONE_ACCESSING -1

static auto SortHeapFunc = ([](GOAPActionBase* lhs, GOAPActionBase* rhs) {return lhs->GetFScore() > rhs->GetFScore(); });

GOAPPlanner::GOAPPlanner(unsigned int worldStateSize)
{
	m_WorldStateSize = worldStateSize;
	m_WorldState.WorldStateProperties.resize(worldStateSize, {0,false});

	// Initialize Effect Map
	for (uint i = 0; i < m_WorldStateSize; ++i)
	{
		std::vector<GOAPActionBase*> newList;

		m_EffectMap[i] = newList;
	}
}


GOAPPlanner::~GOAPPlanner()
{
}

void GOAPPlanner::PopulateEffectMap(std::vector<GOAPActionBase*> actionList)
{
	m_EffectMap.clear();
	for (uint i = 0; i < actionList.size(); ++i)
	{
		auto actionEffects = actionList[i]->GetEffectList();

		for (uint j = 0; j < actionEffects.size(); ++j)
		{
			m_EffectMap[actionEffects[j]].push_back(actionList[i]);
		}
	}
}

void GOAPPlanner::ChangeWorldState(WorldStateProperty pChange)
{
	m_WorldState.WorldStateProperties[pChange.nIdentifier].bData = pChange.bData;
}

GOAPPlan GOAPPlanner::MakePlan(WorldStateProperty const& goalState)
{	
	std::vector<GOAPActionBase*> plan;
	std::vector<GOAPActionBase*> openList;

	WorldState planWorldState = m_WorldState;
	
	// Reset Effect Map
	for (uint i = 0; i < m_WorldStateSize; ++i)
	{
		for (uint j = 0; j < m_EffectMap[i].size(); ++j)
		{
			m_EffectMap[i][j]->SetUsed(false);
			m_EffectMap[i][j]->SetPrev(nullptr);
			m_EffectMap[i][j]->m_nFailureCost = 0;
		}
	}

	auto worldStateData = m_WorldState.WorldStateProperties[goalState.nIdentifier].bData;
	auto goalStateData = goalState.bData;

	// Check if goal state is current world state
	if (worldStateData == goalStateData)
		return GOAPPlan({ true, plan });

	// Get Actions that cause required effect
	auto currentEffectActions = m_EffectMap[goalState.nIdentifier];

	for (uint j = 0; j < currentEffectActions.size(); ++j)
	{
		auto pAction = currentEffectActions[j];
		openList.push_back(pAction);
		std::push_heap(openList.begin(), openList.end(), SortHeapFunc);
		pAction->SetGScore(pAction->GetCost());
		pAction->SetUsed(true);
	}

	GOAPActionBase* planFailPoint = nullptr;

	while (openList.size() > 0)
	{
		// Remove lowest node from open list and make it used
		GOAPActionBase* pCurrent = openList[0];

		std::pop_heap(openList.begin(), openList.end(), SortHeapFunc);
		openList.pop_back();

		// Check if Plan is Complete
		bool bPlanComplete = false;
		int nConditionSuccessCount = 0;
		std::vector<uint> requiredEffects;

		// Check for required preconditions for current node
		auto currentPreConditions = pCurrent->GetPreConditionList();
		for (uint i = 0; i < currentPreConditions.size(); ++i)
		{
			auto planStateData = planWorldState.WorldStateProperties[currentPreConditions[i].nIdentifier].bData;
			auto preConditionData = currentPreConditions[i].bData;

			//Check if precondition is satisfied
			if (planStateData == preConditionData)
				++nConditionSuccessCount;
			else
			{
				requiredEffects.push_back(currentPreConditions[i].nIdentifier);
			}
		}

		// IF all preconditions satisfied
		if (nConditionSuccessCount == currentPreConditions.size())
		{
			int nEffectCount = 0;
			for (int i = 0; i < pCurrent->GetEffectList().size(); ++i)
			{
				if (planWorldState.WorldStateProperties[pCurrent->GetEffectList()[i]].bData)
					++nEffectCount;
			}
			if (nEffectCount != pCurrent->GetEffectList().size())
				// Plan Segment Complete
				bPlanComplete = true;
		}

		// IF Plan segment Complete
		if (bPlanComplete)
		{
			if (planFailPoint)
			{
				bool newPlan = true;
				auto temp = pCurrent->GetPrev();
				while (temp)
				{
					if (temp == planFailPoint)
						newPlan = false;

					temp = temp->GetPrev();
				}

				if (newPlan)
				{
					plan.clear();
					planWorldState = m_WorldState;
				}
			}

			planFailPoint = nullptr;

			// Add to plan
			plan.insert(plan.begin(), pCurrent);

			pCurrent->SetUsed(true);

			// Enable effects of action
			auto currentEffects = pCurrent->GetEffectList();
			for (int i = 0; i < currentEffects.size(); ++i)
			{
				planWorldState.WorldStateProperties[currentEffects[i]].bData = true;
			}

			// While there are previous actions
			while (pCurrent->GetPrev())
			{
				// Make the previous action the current action
				pCurrent = pCurrent->GetPrev();		

				// Check if all effects of current action are satisfied
				int nEffectCount = 0;
				auto currentEffects = pCurrent->GetEffectList();
				for (int i = 0; i < currentEffects.size(); ++i)
				{
					if (planWorldState.WorldStateProperties[currentEffects[i]].bData == true)
						++nEffectCount;
				}
				// If so skip
				if (nEffectCount == currentEffects.size())
					continue;
				
				int nPlanConditionSuccessCount = 0;
				auto currentPreConditions = pCurrent->GetPreConditionList();
				for (uint i = 0; i < currentPreConditions.size(); ++i)
				{
					auto worldStateData = planWorldState.WorldStateProperties[currentPreConditions[i].nIdentifier].bData;
					auto preConditionData = currentPreConditions[i].bData;

					if (worldStateData == preConditionData)
						++nPlanConditionSuccessCount;
					else
						requiredEffects.push_back(currentPreConditions[i].nIdentifier);
				}

				if (nPlanConditionSuccessCount != currentPreConditions.size())
				{
					bPlanComplete = false;
					pCurrent->m_nFailureCost += 10;

					planFailPoint = pCurrent;
				}

				if (bPlanComplete)
				{
					pCurrent->SetUsed(true);
					plan.insert(plan.begin(), pCurrent);

					for (int i = 0; i < currentEffects.size(); ++i)
					{
						planWorldState.WorldStateProperties[currentEffects[i]].bData = true;
					}

					// GOAL state reached return plan
					if (goalState.bData == planWorldState.WorldStateProperties[goalState.nIdentifier].bData)
						return GOAPPlan({ true, plan });
				}
			}

			// Return Plan (???REDUNDANT???)
			if(bPlanComplete)
				return GOAPPlan({ true, plan });
		}

		for (uint i = 0; i < requiredEffects.size(); ++i)
		{
			auto currentEffectActions = m_EffectMap[requiredEffects[i]];
			
			for (uint j = 0; j < currentEffectActions.size(); ++j)
			{
				auto pNeighbour = currentEffectActions[j];

				// Skip null
				if (!pNeighbour)
					continue;

				// Skip If Procedural Preconditions check fails
				if (!pNeighbour->CheckProceduralPreconditions())
					continue;

				// Skip used
				if (pNeighbour->GetUsed())
					continue;

				//IF in open list
				if (std::find(openList.begin(), openList.end(), pNeighbour) != openList.end())
				{
					// Check if better plan
					int newGScore = pCurrent->GetGScore() + pNeighbour->GetCost();
					if (newGScore < pNeighbour->GetGScore())
					{
						// Update to use better plan
						pNeighbour->SetGScore(newGScore);

						// Calculate H Score
						int nHScore = 0;

						auto neighbourPreConditions = pNeighbour->GetPreConditionList();
						for (uint k = 0; k < neighbourPreConditions.size(); ++k)
						{
							auto worldStateData = m_WorldState.WorldStateProperties[neighbourPreConditions[k].nIdentifier].bData;
							auto preConditionData = neighbourPreConditions[k].bData;

							// IF effect needed
							if (worldStateData != preConditionData)
								++nHScore;
						}

						// Multiply
						nHScore *= 10;

						// Set HScore
						pNeighbour->SetHScore(nHScore);
						pNeighbour->SetFScore(pNeighbour->GetGScore() + pNeighbour->GetHScore() + pNeighbour->m_nFailureCost);

						pNeighbour->SetPrev(pCurrent);

						std::sort(openList.begin(), openList.end(), SortHeapFunc);
					}
				}
				// ELSE add to open list and calculate scores
				else
				{
					// Calculate GScore
					pNeighbour->SetGScore(pCurrent->GetGScore() + pNeighbour->GetCost());

					// Calculate H Score
					int nHScore = 0;
					
					auto neighbourPreConditions = pNeighbour->GetPreConditionList();
					for (uint k = 0; k < neighbourPreConditions.size(); ++k)
					{
						auto worldStateData = m_WorldState.WorldStateProperties[neighbourPreConditions[k].nIdentifier].bData;
						auto preConditionData = neighbourPreConditions[k].bData;

						// IF effect needed
						if (worldStateData != preConditionData)
							++nHScore;
					}

					// Multiply
					nHScore *= 10;

					// Set HScore
					pNeighbour->SetHScore(nHScore);

					// Set FScore
					pNeighbour->SetFScore(pNeighbour->GetGScore() + pNeighbour->GetHScore() + pNeighbour->m_nFailureCost);

					// Add to List
					pNeighbour->SetPrev(pCurrent);

					openList.push_back(pNeighbour);
					std::push_heap(openList.begin(), openList.end(), SortHeapFunc);
				}
			}
		}

		requiredEffects.clear();
	}
	
	// Returns if no plan found
	plan.clear();
	return GOAPPlan({ false, plan });
}

GOAPPlan GOAPPlanner::NewPlan(WorldStateProperty const& goalState)
{
	auto SortHeapFunc = ([](GOAPActionBase* lhs, GOAPActionBase* rhs) {return lhs->GetFScore() > rhs->GetFScore(); });

	Plan preferredPlan;
	preferredPlan.cost = INT_MAX;

	atomic<int> accessing;
	accessing.store(NO_ONE_ACCESSING, std::memory_order::memory_order_relaxed);

	vector<vector<Plan>> threadedPlans;
	for (int i = 0; i < m_nThreadCount; ++i)
	{
		threadedPlans.push_back(vector<Plan>());
	}

	// Reset Effect Map
	// STR??
	//for (uint i = 0; i < m_WorldStateSize; ++i)
	//{
	//	for (uint j = 0; j < m_EffectMap[i].size(); ++j)
	//	{
	//		m_EffectMap[i][j]->SetUsed(false);
	//		m_EffectMap[i][j]->SetPrev(nullptr);
	//	}
	//}

	auto worldStateData = m_WorldState.WorldStateProperties[goalState.nIdentifier].bData;
	auto goalStateData = goalState.bData;

	// Check if goal state is current world state (No actions needed)
	if (worldStateData == goalStateData)
		return preferredPlan.data;

	// Get Initial Actions that cause required effect
	auto currentEffectActions = m_EffectMap[goalState.nIdentifier];

	// Assign those actions to a plan each and add that to a thread workload
	for (uint i = 0; i < currentEffectActions.size(); ++i)
	{
		auto pAction = currentEffectActions[i];

		// Plan for action
		Plan currentPlan;
		currentPlan.isComplete = false;
		currentPlan.cost = pAction->GetCost();
		currentPlan.worldState = m_WorldState;
		currentPlan.data.actions.push_back(pAction);

		// Add to thread workload and sort by cost
		int nThread = i % m_nThreadCount;
		threadedPlans[nThread].push_back(currentPlan);
		std::push_heap(threadedPlans[nThread].begin(), threadedPlans[nThread].end(), 
			[](Plan const& lhs, Plan const& rhs) {return lhs.cost < rhs.cost; });
	}

	vector<std::thread> threads;
	for (int i = 1; i < m_nThreadCount; ++i)
	{
		// START THREADS HERE
		threads.push_back(std::thread(&GOAPPlanner::ThreadPlan, this, std::ref(threadedPlans[i]), std::ref(preferredPlan), std::ref(accessing), std::ref(i), std::ref(goalState)));
	}

	//MAIN THREAD
	ThreadPlan(threadedPlans[0], preferredPlan, accessing, 0, goalState);

	// JOIN THREADS HERE
	for (int i = 0; i < threads.size(); ++i)
	{
		threads[i].join();
	}

	// TEMP
	return preferredPlan.data;
}

void GOAPPlanner::ThreadPlan(std::vector<Plan>& plans, Plan & preferredPlan, atomic<int> & accessing, unsigned int const& threadCount, WorldStateProperty const& goalState)
{	
	// While there are incomplete plans
	uint completedPlans = 0;
	while (completedPlans < plans.size())
	{
		// Sort plans by  
		std::sort(plans.begin(), plans.end(), 
			[](Plan const& lhs, Plan const& rhs) {return lhs.cost < rhs.cost; });

		//Work on lowest cost plan
		Plan* currentPlan = &plans[0];

		// IF COST GREATER THAN PREFFERED
		if (preferredPlan.cost < currentPlan->cost)
			return;

		// PLAN COMPLETE
		if (currentPlan->isComplete)
		{
			bool waiting = true;
			while (waiting)
			{
				// Get Access
				if (accessing.load(std::memory_order::memory_order_acquire) == NO_ONE_ACCESSING)
				{
					// Lock Access
					accessing.store(threadCount, std::memory_order::memory_order_release);

					// Check Preferred

					// IF current plan is cheaper replace preferred plan
					if (preferredPlan.cost > currentPlan->cost)
					{
						preferredPlan = *currentPlan;
					}
					// ELIF cost is equal
					else if (preferredPlan.cost == currentPlan->cost)
					{
						// IF current plan is shorter replace preferred plan
						if (preferredPlan.data.actions.size() > currentPlan->data.actions.size())
							preferredPlan = *currentPlan;
					}

					// Unlock Access
					accessing.store(NO_ONE_ACCESSING, std::memory_order::memory_order_release);

					waiting = false;
				}

			}
			// EARLY EXIT (Not sure if optimal)
			return;
		}

		// Reset plan world state
		WorldState planWorldState = m_WorldState;

		// For each of the actions in the plan
		bool loopActions = true;
		int actionCount = currentPlan->data.actions.size() - 1;
		while (actionCount >= 0 && loopActions)
		{
			auto currentAction = currentPlan->data.actions[actionCount];

			// Get Preconditions
			auto preconditions = currentAction->GetPreConditionList();
			bool allSatisfied = false;

			vector<WorldStateProperty> requiredEffects;

			// For each Precondition
			for (int i = 0; i < preconditions.size(); ++i)
			{
				uint nIdent = preconditions[i].nIdentifier;
				bool neededData = preconditions[i].bData;

				// Get related data from the world state
				bool worldData = planWorldState.WorldStateProperties[nIdent].bData;

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
					uint effectIdent = effects[i];
					planWorldState.WorldStateProperties[effectIdent].bData = true;
				}

				// Continue to next action
				actionCount--;
				continue;
			}
			// List of new plans
			vector<Plan> newPlans;
			
			// Add actions for all required effects
			//for (int i = 0; i < requiredEffects.size(); ++i)
			if (requiredEffects.size() > 0)
			{
				uint ident = requiredEffects[0].nIdentifier;
				auto actions = m_EffectMap[ident];
				
				// There are no actions to take
				if (!(actions.size() > 0))
				{
					// Plan can not be completed
					currentPlan->cost = INT_MAX;
					currentPlan->isComplete = true;
					completedPlans++;
				}
				for (int j = 1; j < actions.size(); ++j)
				{
						// actions assigned to new plans
						Plan newPlan;
						newPlan.isComplete = false;
						newPlan.data.isSuccessful = false;

						for (int k = 0; k < currentPlan->data.actions.size(); k++)
						{
							newPlan.data.actions.push_back(currentPlan->data.actions[k]);
						}

						newPlan.data.actions.push_back(actions[j]);
						newPlan.cost = currentPlan->cost + actions[j]->GetCost();

						// Add newPlan to list of plans
						newPlans.push_back(newPlan);
				}

				// First action assigned to this plan
				currentPlan->data.actions.push_back(actions[0]);
				currentPlan->cost += actions[0]->GetCost();

				// New Actions so break out of action loop
				loopActions = false;
			}
			
			for (int i = 0; i < newPlans.size(); ++i)
			{
				plans.push_back(newPlans[i]);
			}
		}// END Action loop

		// Check if plan completes the goal state
		bool goalData = goalState.bData;
		bool planData = planWorldState.WorldStateProperties[goalState.nIdentifier].bData;
		if (goalData == planData)
		{
			currentPlan->isComplete = true;
			currentPlan->data.isSuccessful = true;
		}

	}// END Plan Loop
}
