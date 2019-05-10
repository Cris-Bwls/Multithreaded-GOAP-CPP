#pragma once

#include "GOAPActionBase.h"
#include "EnumPrecondition.h"

class GOAPActionAttackFist : public GOAPActionBase
{
public:
	GOAPActionAttackFist() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionAttackFist";

		m_PreConditionList.push_back({ (uint)EPreconditions::AtTarget, true });
		m_EffectList.push_back({ (uint)EPreconditions::TargetDead });

		m_nCost = 25;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionAttackMelee : public GOAPActionBase
{
public:
	GOAPActionAttackMelee() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionAttackMelee";

		m_PreConditionList.push_back({ (uint)EPreconditions::AtTarget, true });
		m_PreConditionList.push_back({ (uint)EPreconditions::HaveWeapon_Melee, true });
		m_EffectList.push_back({ (uint)EPreconditions::TargetDead });

		m_nCost = 5;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionAttackRanged : public GOAPActionBase
{
public:
	GOAPActionAttackRanged() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionAttackRanged";

		m_PreConditionList.push_back({ (uint)EPreconditions::NearTarget, true });
		m_PreConditionList.push_back({ (uint)EPreconditions::HaveWeapon_Ranged, true });
		m_EffectList.push_back({ (uint)EPreconditions::TargetDead });

		m_nCost = 5;
	}

	bool Act() { return true; };
	void Reset() {};
};