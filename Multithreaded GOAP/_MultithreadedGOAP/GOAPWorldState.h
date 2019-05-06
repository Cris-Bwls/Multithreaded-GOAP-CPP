#pragma once

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
	inline WorldStateProperty(int nIdentifier, bool bData)
		: nIdentifier(nIdentifier), bData(bData) {};

	int nIdentifier;
	// Data data;
	bool bData = false;
};

struct WorldState
{
	WorldStateProperty WorldStateProperties[EGOAPSYMBOL_TOTAL];
};
