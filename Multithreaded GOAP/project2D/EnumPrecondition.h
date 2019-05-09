#pragma once

typedef unsigned int uint;

enum class EPreconditions : uint
{
	NONE = 0U,
	AtTarget,
	NearTarget,
	HaveWeapon,

	TOTAL
};