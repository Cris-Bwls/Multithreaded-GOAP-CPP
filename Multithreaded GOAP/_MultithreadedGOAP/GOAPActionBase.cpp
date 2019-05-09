#include "GOAPActionBase.h"

GOAPActionBase::GOAPActionBase()
{
	m_bUsed = false;
	m_nCost = 10;

	m_nFScore = 0;
	m_nGScore = 0;
	m_nHScore = 0;
}


GOAPActionBase::~GOAPActionBase()
{
}

bool GOAPActionBase::CheckProceduralPreconditions()
{
	return true;
}

void GOAPActionBase::Start()
{
	m_Status = EActionStatus::ACTIVE;
}

void GOAPActionBase::Run(float fDeltaTime)
{
	if (m_Status != EActionStatus::ACTIVE)
		Start();
}

void GOAPActionBase::Finish()
{
	m_Status = EActionStatus::DONE;
}

inline void GOAPActionBase::Reset()
{
	m_Status = EActionStatus::INACTIVE;
}
