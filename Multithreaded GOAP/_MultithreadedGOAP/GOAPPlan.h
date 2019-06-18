#pragma once
#include<vector>
#include"GOAPActionBase.h"

typedef std::vector<GOAPActionBase*> ActionList;

/*
	Data structure that holds the actual plan, and if it acheives its goal
*/
struct GOAPPlan
{
	bool isSuccessful = false;
	ActionList actions;
};