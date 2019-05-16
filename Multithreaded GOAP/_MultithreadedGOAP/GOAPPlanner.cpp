#include "GOAPPlanner.h"
#include "GOAPActionBase.h"
#include <algorithm>

using std::vector;

GOAPPlanner::GOAPPlanner(unsigned int worldStateSize)
{
	m_WorldStateSize = worldStateSize;
	m_WorldState.WorldStateProperties.resize(worldStateSize, {0,false});

	// Initialize Effect Map
	for (unsigned int i = 0; i < m_WorldStateSize; ++i)
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
	for (unsigned int i = 0; i < actionList.size(); ++i)
	{
		auto actionEffects = actionList[i]->GetEffectList();

		for (unsigned int j = 0; j < actionEffects.size(); ++j)
		{
			m_EffectMap[actionEffects[j]].push_back(actionList[i]);
		}
	}
}

void GOAPPlanner::ChangeWorldState(WorldStateProperty pChange)
{
	m_WorldState.WorldStateProperties[pChange.nIdentifier].bData = pChange.bData;
}

GOAPPlan GOAPPlanner::MakePlan(WorldStateProperty goalState)
{
	auto SortHeapFunc = ([](GOAPActionBase* lhs, GOAPActionBase* rhs) {return lhs->GetFScore() > rhs->GetFScore(); });
	
	std::vector<GOAPActionBase*> plan;
	std::vector<GOAPActionBase*> openList;

	WorldState planWorldState = m_WorldState;
	
	// Reset Effect Map
	for (unsigned int i = 0; i < m_WorldStateSize; ++i)
	{
		for (unsigned int j = 0; j < m_EffectMap[i].size(); ++j)
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

	for (unsigned int j = 0; j < currentEffectActions.size(); ++j)
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
		std::vector<unsigned int> requiredEffects;

		// Check for required preconditions for current node
		auto currentPreConditions = pCurrent->GetPreConditionList();
		for (unsigned int i = 0; i < currentPreConditions.size(); ++i)
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
				for (unsigned int i = 0; i < currentPreConditions.size(); ++i)
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

		for (unsigned int i = 0; i < requiredEffects.size(); ++i)
		{
			auto currentEffectActions = m_EffectMap[requiredEffects[i]];
			
			for (unsigned int j = 0; j < currentEffectActions.size(); ++j)
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
						for (unsigned int k = 0; k < neighbourPreConditions.size(); ++k)
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
					for (unsigned int k = 0; k < neighbourPreConditions.size(); ++k)
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

GOAPPlan GOAPPlanner::NewPlan(WorldStateProperty goalState)
{

	auto SortHeapFunc = ([](GOAPActionBase* lhs, GOAPActionBase* rhs) {return lhs->GetFScore() > rhs->GetFScore(); });

	Plan preferredPlan;
	vector<vector<Plan>> threadedPlans;
	//vector<Plan> openList;

	WorldState planWorldState = m_WorldState;

	// Reset Effect Map
	for (unsigned int i = 0; i < m_WorldStateSize; ++i)
	{
		for (unsigned int j = 0; j < m_EffectMap[i].size(); ++j)
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
		return preferredPlan.data;

	// Get Actions that cause required effect
	auto currentEffectActions = m_EffectMap[goalState.nIdentifier];

	return preferredPlan.data;
}

void GOAPPlanner::ThreadPlan(std::vector<Plan>& plans)
{
}
