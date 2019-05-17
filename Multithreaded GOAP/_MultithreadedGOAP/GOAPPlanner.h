#pragma once
#include <vector>
#include <map>

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

	void PopulateEffectMap(std::vector<GOAPActionBase*> actionList);
	void ChangeWorldState(WorldStateProperty pChange);

	GOAPPlan MakePlan(WorldStateProperty const& goalState);
	GOAPPlan NewPlan(WorldStateProperty const& goalState);

	inline void SetThreadCount(unsigned int count) { m_nThreadCount = count; };

private:
	struct Plan
	{
		bool isComplete;
		GOAPPlan data;
		WorldState worldState;
		int cost;
	};
	bool SortPlans(Plan const& lhs, Plan const& rhs) {return lhs.cost > rhs.cost; };

	void ThreadPlan(std::vector<Plan> & plans, Plan & preferredPlan, int & accessing, unsigned int const& threadNumber, WorldStateProperty const& goalState);

	unsigned int m_nThreadCount = 1;

	unsigned int m_WorldStateSize = 0;
	WorldState m_WorldState;

	std::map<unsigned int, std::vector<GOAPActionBase*>> m_EffectMap;
};

