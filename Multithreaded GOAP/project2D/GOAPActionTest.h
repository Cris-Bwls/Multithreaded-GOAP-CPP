#pragma once

#include "GOAPActionBase.h"
#include "EnumPrecondition.h"

class GOAPActionTest01 : public GOAPActionBase
{
public:
	GOAPActionTest01() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionTest01";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::HaveWeapon_Melee });

		m_nCost = 5;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionTest02 : public GOAPActionBase
{
public:
	GOAPActionTest02() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionTest02";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::HaveWeapon_Ranged });

		m_nCost = 5;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionTest03 : public GOAPActionBase
{
public:
	GOAPActionTest03() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionTest03";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::AtTarget });

		m_nCost = 4;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionTest04 : public GOAPActionBase
{
public:
	GOAPActionTest04() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionTest04";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::NearTarget });

		m_nCost = 2;
	}

	bool Act() { return true; };
	void Reset() {};
};