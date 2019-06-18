#include "GOAPWrapper.h"


GOAPWrapper::GOAPWrapper(size_t const & threadCount)
{
	m_planner = new GOAPPlanner();
	m_plannerThread = std::thread(&GOAPPlanner::StartThreaded, m_planner, std::ref(m_queue), std::ref(threadCount));
}

GOAPWrapper::~GOAPWrapper()
{
	m_planner->Stop();
	m_plannerThread.join();

	delete m_planner;
}

/*
	Pushes a plan to the back of the queue
*/
void GOAPWrapper::AddPlan(PlanData* planData)
{
	m_queue.Push(planData);
	m_planner->InputWake();
}
