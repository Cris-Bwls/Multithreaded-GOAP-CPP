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
#define THREAD_COUNT 5
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
	
	actionList.push_back(new GOAPActionTest01());
	actionList.push_back(new GOAPActionTest02());
	actionList.push_back(new GOAPActionTest03());
	actionList.push_back(new GOAPActionTest04());
}

void OriginalRecipe(WorldState* state)
{
	(*state).properties.push_back({ (uint)EPreconditions::NONE, true });
	(*state).properties.push_back({ (uint)EPreconditions::AtTarget, false });
	(*state).properties.push_back({ (uint)EPreconditions::NearTarget, false });
	(*state).properties.push_back({ (uint)EPreconditions::HaveWeapon_Melee, false });
	(*state).properties.push_back({ (uint)EPreconditions::HaveWeapon_Ranged, false });
	(*state).properties.push_back({ (uint)EPreconditions::TargetDead, false });
}

void GoalState(GOAPWrapper* wrapper, bool & notifier, GOAPPlan & plan)
{
	WorldState worldState;
	OriginalRecipe(&worldState);

	std::vector<GOAPActionBase*> actions;
	Actions(actions);

	WorldStateProperty goal = { (uint)EPreconditions::TargetDead, true };
	
	PlanData data;
	data.pActionList = &actions;
	data.pGoalState = &goal;
	data.pNotification = &notifier;
	data.pResult = &plan;
	data.pWorldState = &worldState;

	//auto timeStart = high_resolution_clock::now();
	for (int i = 0; i < PLAN_COUNT; ++i)
	{
		wrapper->AddPlan(data);
	}

	//auto timeEnd = high_resolution_clock::now();
	//auto timeTaken = duration_cast<duration<double>>(timeEnd - timeStart);

	//std::cout << duration_cast<microseconds>(timeEnd - timeStart).count();
	//std::cout << std::endl;
	//
	//totalTimeTaken += timeTaken.count();
	//count++;
	//
	//std::cout << "AVG Time Taken = " << totalTimeTaken / count;
	//std::cout << std::endl;

	
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

	wrapper = new GOAPWrapper(THREAD_COUNT);

	return true;
}

void Application2D::shutdown() {
	
	delete m_font;
	delete m_2dRenderer;

	delete wrapper;
	
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
		GoalState(wrapper, m_bNotifier, m_plan);
	}

	if (m_bNotifier)
	{
		m_bNotifier = false;
		printf("Final Plan \n");
		if (m_plan.isSuccessful)
		{
			for (auto i = m_plan.actions.size(); i > 0; --i)
			{
				printf(m_plan.actions[i - 1]->GetName());
				printf("\n");
			}
		}
		else
		{
			printf("Plan Failed \n");
		}
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