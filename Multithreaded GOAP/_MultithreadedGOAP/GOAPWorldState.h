#pragma once
#include <vector>

// DataTypes
enum EDataType
{
	EDATATYPE_BOOL = 0,
	EDATATYPE_INT,
	EDATATYPE_HASH,
	EDATATYPE_POINTER,

	EDATATYPE_TOTAL
};

// Data Container
struct Data
{
	EDataType eDataType;

	union
	{
		bool bData;
		int nData;
		void* pVoid;
	};
};

struct WorldStateProperty
{
	inline WorldStateProperty(unsigned int nIdentifier, bool bData)
		: nIdentifier(nIdentifier), bData(bData) {};

	unsigned int nIdentifier;
	// Data data;
	bool bData = false;
};

struct WorldState
{
	std::vector<WorldStateProperty> properties;
};
