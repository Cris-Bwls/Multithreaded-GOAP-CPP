#include "Application2D.h"
#include "Texture.h"
#include "Font.h"
#include "Input.h"
#include <chrono>
#include <iostream>

#include "GOAPActionAttack.h"
#include "GOAPActionGet.h"
#include "GOAPActionMove.h"
#include "EnumPrecondition.h"

#include "GOAPActionTest.h"

using namespace std::chrono;
#define PLAN_COUNT 1

static double totalTimeTaken = 0;
static int count = 0;

void Actions(std::vector<GOAPActionBase*> & actionList)
{
	actionList.push_back(new GOAPActionMoveToTarget());
	actionList.push_back(new GOAPActionMoveNearTarget());
	actionList.push_back(new GOAPActionAttackFist());
	actionList.push_back(new GOAPActionAttackMelee());
	actionList.push_back(new GOAPActionAttackRanged());
	actionList.push_back(new GOAPActionGetWeaponMelee());
	actionList.push_back(new GOAPActionGetWeaponRanged());
	
	//actionList.push_back(new GOAPActionTest01());
	//actionList.push_back(new GOAPActionTest02());
	//actionList.push_back(new GOAPActionTest03());
	//actionList.push_back(new GOAPActionTest04());
}

void OriginalRecipe(GOAPPlanner* planner)
{
	planner->ChangeWorldState({ (uint)EPreconditions::NONE, true });
	planner->ChangeWorldState({ (uint)EPreconditions::AtTarget, false });
	planner->ChangeWorldState({ (uint)EPreconditions::NearTarget, false });
	planner->ChangeWorldState({ (uint)EPreconditions::HaveWeapon_Melee, false });
	planner->ChangeWorldState({ (uint)EPreconditions::HaveWeapon_Ranged, false });
	planner->ChangeWorldState({ (uint)EPreconditions::TargetDead, false });
}

void GoalState(GOAPPlanner* planner)
{	
	GOAPPlan plan;
	auto timeStart = high_resolution_clock::now();
	for (int i = 0; i < PLAN_COUNT; ++i)
	{
		plan = planner->MakePlan({ (uint)EPreconditions::TargetDead, true });
		//for (int j = 0; j < plan.actions.size(); /*NO ITER*/)
		//{
		//	auto prev = plan.actions[j]->GetPrev();
		//	if (prev == nullptr)
		//		++j;
		//	else
		//	{
		//		if (std::find(plan.actions.begin(), plan.actions.end(), prev) != plan.actions.end())
		//		{
		//			++j;
		//		}
		//		else
		//		{
		//			//++j;
		//			plan.actions.erase(plan.actions.begin() + j);
		//		}
		//	}
		//}
	}
	auto timeEnd = high_resolution_clock::now();
	auto timeTaken = duration_cast<duration<double>>(timeEnd - timeStart);

	std::cout << duration_cast<microseconds>(timeEnd - timeStart).count();
	std::cout << std::endl;

	totalTimeTaken += timeTaken.count();
	count++;

	std::cout << "AVG Time Taken = " << totalTimeTaken / count;
	std::cout << std::endl;

	printf("Final Plan\n");
	if (plan.isSuccessful)
	{
		for (auto i = plan.actions.size(); i > 0; --i)
		{
			printf(plan.actions[i - 1]->GetName());
			printf("\n");
		}
	}
	else
	{
		printf("Plan Failed");
	}
}

Application2D::Application2D() {

}

Application2D::~Application2D() {

}

bool Application2D::startup() {
	
	m_2dRenderer = new aie::Renderer2D();

	m_font = new aie::Font("./font/consolas.ttf", 32);
	
	m_timer = 0;

	Actions(actionList);

	planner = new GOAPPlanner((uint)EPreconditions::TOTAL);
	planner->PopulateEffectMap(actionList);

	return true;
}

void Application2D::shutdown() {
	
	delete m_font;
	delete m_2dRenderer;

	delete planner;
	
	while (actionList.size() > 0)
	{
		delete actionList.back();
		actionList.pop_back();
	}
}

void Application2D::update(float deltaTime) {

	m_timer += deltaTime;

	// input example
	aie::Input* input = aie::Input::getInstance();

	// exit the application
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();

	if (input->isKeyDown(aie::INPUT_KEY_1))
	{
		OriginalRecipe(planner);
		GoalState(planner);
	}
}

void Application2D::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// begin drawing sprites
	m_2dRenderer->begin();
	
	// output some text, uses the last used colour
	char fps[32];
	sprintf_s(fps, 32, "FPS: %i", getFPS());
	m_2dRenderer->drawText(m_font, fps, 0, getWindowHeight() - 32);
	m_2dRenderer->drawText(m_font, "Press ESC to quit!", 0, getWindowHeight() - 64);

	// done drawing sprites
	m_2dRenderer->end();
}