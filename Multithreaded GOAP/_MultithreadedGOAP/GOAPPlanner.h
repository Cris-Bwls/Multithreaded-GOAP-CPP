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

	GOAPPlan MakePlan(WorldStateProperty goalState);
	GOAPPlan NewPlan(WorldStateProperty goalState);

private:
	unsigned int m_WorldStateSize = 0;

	WorldState m_WorldState;
	std::map<unsigned int, std::vector<GOAPActionBase*>> m_EffectMap;
};

