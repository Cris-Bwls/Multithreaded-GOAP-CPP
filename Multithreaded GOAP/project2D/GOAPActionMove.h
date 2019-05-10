#pragma once

#include "GOAPActionBase.h"
#include "EnumPrecondition.h"

class GOAPActionMoveToTarget : public GOAPActionBase
{
public:
	GOAPActionMoveToTarget() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionMoveToTarget";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::AtTarget });

		m_nCost = 5;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionMoveNearTarget : public GOAPActionBase
{
public:
	GOAPActionMoveNearTarget() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionMoveNearTarget";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::NearTarget });

		m_nCost = 5;
	}

	bool Act() { return true; };
	void Reset() {};
};