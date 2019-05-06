#include "GOAPActionBase.h"

GOAPActionBase::GOAPActionBase(AI* pAI, Terrain* pTerrain)
{
	m_pAI = pAI; 
	m_pTerrain = pTerrain;

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
	m_Status = EACTIONSTATUS_ACTIVE;
}

void GOAPActionBase::Run(float fDeltaTime)
{
	if (m_Status != EACTIONSTATUS_ACTIVE)
		Start();
}

void GOAPActionBase::Finish()
{
	m_Status = EACTIONSTATUS_DONE;
}

bool GOAPActionBase::Act()
{
	return false;
}

bool GOAPActionBase::Move()
{
	return false;
}

bool GOAPActionBase::MoveToPos()
{
	auto targetPos = m_pAI->GetTargetPos();
	auto path = m_pTerrain->GetPathToPos(m_pAI->GetPos(), *targetPos, true);
	m_pAI->SetPath(path);
	
	if (path.size() == 0)
		return false;
	return true;

}

bool GOAPActionBase::MoveToObject()
{
	auto target = m_pAI->GetTarget();
	auto path = m_pTerrain->GetPathToObject(m_pAI->GetPos(), (StaticObject*)target, false);
	m_pAI->SetPath(path);

	if (path.size() == 0)
		return false;
	return true;
}

inline void GOAPActionBase::Reset()
{
	m_Status = EACTIONSTATUS_INACTIVE;
}
