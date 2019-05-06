#pragma once
#include <vector>
#include "GOAPWorldState.h"

enum ActionStatus
{
	// EACTIONSTATUS_

	EACTIONSTATUS_INACTIVE = 0,
	EACTIONSTATUS_ACTIVE,
	EACTIONSTATUS_DONE,
	EACTIONSTATUS_FAILED
};

class GOAPActionBase
{
public:
	GOAPActionBase(AI* pAI, Terrain* pTerrain);
	virtual ~GOAPActionBase();
	
	inline std::vector<WorldStateProperty> GetPreConditionList() { return m_PreConditionList; };
	inline std::vector<EGOAPSymbol> GetEffectList() { return m_EffectList; };

	virtual void Start();
	virtual void Run(float fDeltaTime);
	virtual void Finish();

	virtual bool Act();
	virtual bool Move();
	virtual bool MoveToPos();
	virtual bool MoveToObject();

	virtual void Reset();

	inline void SetPrev(GOAPActionBase* pPrev) { m_pPrev = pPrev; };
	inline GOAPActionBase* GetPrev() { return m_pPrev; };

	inline void SetUsed(bool bUsed) { m_bUsed = bUsed; };
	inline bool GetUsed() { return m_bUsed; };

	inline int GetCost() { return m_nCost; };

	inline void SetFScore(int nFScore) { m_nFScore = nFScore; };
	inline int GetFScore() { return m_nFScore; };

	inline void SetGScore(int nGScore) { m_nGScore = nGScore; };
	inline int GetGScore() { return m_nGScore; };

	inline void SetHScore(int nHScore) { m_nHScore = nHScore; };
	inline int GetHScore() { return m_nHScore; };

	inline char* GetName() { return m_ActionName; };

	virtual bool CheckProceduralPreconditions();
	//TEST
	int m_nFailureCost;
protected:
	char* m_ActionName;

	std::vector<WorldStateProperty> m_PreConditionList;
	std::vector<EGOAPSymbol> m_EffectList;

	ActionStatus m_Status;

	AI* m_pAI;
	Terrain* m_pTerrain;

	GOAPActionBase* m_pPrev;

	bool m_bUsed;

	int m_nFScore;
	int m_nGScore;
	int m_nHScore;

	int m_nCost;
};

