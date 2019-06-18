#include "Unit.h"
#include "EnumPrecondition.h"

#include "GOAPActionAttack.h"
#include "GOAPActionGet.h"
#include "GOAPActionMove.h"

#include <iostream>

static double totalTimeTaken = 0;
static int count = 0;

Unit::Unit(GOAPWrapper * pPlanWrapper, size_t nUnitNumber)
{
	m_pPlanWrapper = pPlanWrapper;
	m_nUnitNumber = nUnitNumber;

	// Create everything it needs to make a plan
	m_pPlanData = new PlanData();
	m_pPlanData->pResult = new std::future<GOAPPlan>();
	m_pPlanData->pWorldState = new WorldState();
	m_pPlanData->pGoalState = new GoalState((size_t)EPreconditions::TargetDead, true);
	m_pPlanData->pActionList = new ActionList();

	// For the test case these a single static plan is created from a set world state 
	SetWorldState();
	SetActionList();
}

Unit::~Unit()
{
	if (m_pPlanData)
	{
		if (m_pPlanData->pResult)
			delete (m_pPlanData->pResult);
		if (m_pPlanData->pWorldState)
			delete (m_pPlanData->pWorldState);
		if (m_pPlanData->pGoalState)
			delete (m_pPlanData->pGoalState);

		if (m_pPlanData->pActionList)
		{
			while (m_pPlanData->pActionList->size() > 0)
			{
				delete m_pPlanData->pActionList->back();
				m_pPlanData->pActionList->pop_back();
			}

			delete (m_pPlanData->pActionList);
		}

		delete m_pPlanData;
	}
}

/*
	Tells the wrapper it wants a new plan, and starts the clock
*/
void Unit::GetPlan()
{
	m_startPlan = high_resolution_clock::now();
	m_pPlanWrapper->AddPlan(m_pPlanData);
}

/*
	Update that checks if the new plan is finished
*/
void Unit::Update()
{
	if (m_pPlanData->pResult->valid())
		ReportNewPlan();
}

/*
	Reports the new plan to the console with the time taken
*/
void Unit::ReportNewPlan()
{
	// Make new plan the active plan
	m_activePlan = m_pPlanData->pResult->get();
	auto endTime = high_resolution_clock::now();
	auto planTime = duration_cast<duration<double>>(endTime - m_startPlan);

	// Add time to total time	
	totalTimeTaken += planTime.count();
	count++;
	
	if (m_bPrintToConsole)
	{
		// Print time that it took for this plan to be finished and observed
		std::cout << planTime.count();
		std::cout << std::endl;

		// Print average time
		std::cout << "AVG Time Taken = " << totalTimeTaken / count;
		std::cout << std::endl;

		// Print new plan to console
		printf("Final Plan @ Unit #%i \n", m_nUnitNumber);
		if (m_activePlan.isSuccessful)
		{
			// Print all the actions
			for (auto i = m_activePlan.actions.size(); i > 0; --i)
			{
				printf(m_activePlan.actions[i - 1]->GetName());
				printf("\n");
			}
		}
		else
		{
			// Print if it failed to find a solution (Shouldnt happen with current setup)
			printf("Plan Failed \n");
		}
	}

	// create a new future
	delete (m_pPlanData->pResult);
	m_pPlanData->pResult = new std::future<GOAPPlan>();

	planComplete = true;
}

/*
	Sets the Units WorldState
*/
void Unit::SetWorldState()
{
	m_pPlanData->pWorldState->properties.clear();

	m_pPlanData->pWorldState->properties.push_back({ (size_t)EPreconditions::NONE, true });
	m_pPlanData->pWorldState->properties.push_back({ (size_t)EPreconditions::AtTarget, false });
	m_pPlanData->pWorldState->properties.push_back({ (size_t)EPreconditions::NearTarget, false });
	m_pPlanData->pWorldState->properties.push_back({ (size_t)EPreconditions::HaveWeapon_Melee, false });
	m_pPlanData->pWorldState->properties.push_back({ (size_t)EPreconditions::HaveWeapon_Ranged, false });
	m_pPlanData->pWorldState->properties.push_back({ (size_t)EPreconditions::TargetDead, false });
}


/*
	Adds the Actions that the unit knows how to perform to the action list
*/
void Unit::SetActionList()
{
	m_pPlanData->pActionList->clear();

	m_pPlanData->pActionList->push_back(new GOAPActionMoveToTarget());
	m_pPlanData->pActionList->push_back(new GOAPActionMoveNearTarget());
	m_pPlanData->pActionList->push_back(new GOAPActionAttackFist());
	m_pPlanData->pActionList->push_back(new GOAPActionAttackMelee());
	m_pPlanData->pActionList->push_back(new GOAPActionAttackRanged());
	m_pPlanData->pActionList->push_back(new GOAPActionGetWeaponMelee());
	m_pPlanData->pActionList->push_back(new GOAPActionGetWeaponRanged());
}
