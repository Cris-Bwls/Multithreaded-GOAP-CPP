#pragma once
#include "GOAPWrapper.h"

#include <chrono>
using namespace std::chrono;

class Unit
{
public:
	Unit(GOAPWrapper* pPlanWrapper, size_t nUnitNumber);
	~Unit();

	void GetPlan();
	void Update();

	bool planComplete = true;
	bool m_bPrintToConsole = true;

private:
	void ReportNewPlan();
	void SetWorldState();
	void SetActionList();

	size_t m_nUnitNumber;
	GOAPPlan m_activePlan;
	PlanData* m_pPlanData;
	GOAPWrapper* m_pPlanWrapper;

	std::chrono::steady_clock::time_point m_startPlan;
};

