#pragma once
#include <vector>
#include <map>
#include <atomic>

#include "GOAPWorldState.h"

class GOAPActionBase;

struct GOAPPlan
{
	bool isSuccessful = false;
	std::vector<GOAPActionBase*> actions;
};

class GOAPPlanner
{
public:
	GOAPPlanner(unsigned int worldStateSize);
	~GOAPPlanner();

	void PopulateEffectMap(std::vector<GOAPActionBase*> const& actionList);
	void ChangeWorldState(WorldState const& worldState);
	GOAPPlan MakePlan(WorldStateProperty const& goalState);

	GOAPPlan Planning(WorldStateProperty const& goalState, WorldState const& worldState, std::vector<GOAPActionBase*> const& actionList);

	inline void SetThreadCount(unsigned int count) { m_nThreadCount = count; };

private:
	struct Plan
	{
		bool isComplete;
		GOAPPlan data;
		WorldState worldState;
		int cost;
	};
	bool SortPlans(Plan lhs, Plan rhs) {return lhs.cost > rhs.cost; };

	void ThreadPlan(std::vector<Plan> & plans, Plan & preferredPlan, std::atomic<int> & accessing, unsigned int const& threadNumber, WorldStateProperty const& goalState);

	unsigned int m_nThreadCount = 1;

	unsigned int m_WorldStateSize = 0;
	WorldState m_WorldState;

	std::map<unsigned int, std::vector<GOAPActionBase*>> m_EffectMap;
};

