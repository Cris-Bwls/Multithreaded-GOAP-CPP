#pragma once

#include "GOAPActionBase.h"
#include "EnumPrecondition.h"

class GOAPActionGetWeaponMelee : public GOAPActionBase
{
public:
	GOAPActionGetWeaponMelee() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionGetWeaponMelee";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::HaveWeapon_Melee });

		m_nCost = 10;
	}

	bool Act() { return true; };
	void Reset() {};
};

class GOAPActionGetWeaponRanged : public GOAPActionBase
{
public:
	GOAPActionGetWeaponRanged() : GOAPActionBase::GOAPActionBase()
	{
		m_ActionName = "GOAPActionGetWeaponRanged";

		m_PreConditionList.push_back({ (uint)EPreconditions::NONE, true });
		m_EffectList.push_back({ (uint)EPreconditions::HaveWeapon_Ranged });

		m_nCost = 10;
	}

	bool Act() { return true; };
	void Reset() {};
};