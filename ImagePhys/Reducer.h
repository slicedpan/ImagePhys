#pragma once

#include <vector>
#include <map>
#include "svl\SVL.h"

struct CostEntry
{
	int id;
	float cost;
	CostEntry* nextId;
	CostEntry* prevId;
	CostEntry* nextCost;
	CostEntry* prevCost;
};

class Reducer
{
public:
	Reducer(void);
	~Reducer(void);
	void CalculateCosts();
	void Build();
	void Clear();
	void Reduce();
	void Init(std::vector<Vec2>& originalPoints);
	std::vector<Vec2>& GetPoints();
	std::vector<float>& GetCosts();
	bool isOpaque(int xCoord, int yCoord);
	float GetMinCost();	
private:
	CostEntry* firstId;
	CostEntry* lastId;
	CostEntry* firstCost;
	CostEntry* lastCost;
	std::vector<Vec2> points;
	std::multimap<float, int> costMap;
	std::vector<float> currentCost;	
	unsigned char* imgData;
	float CalculateCost(CostEntry* costEntry);
	void LinkCost(CostEntry* costEntry);
	void UnlinkCost(CostEntry* costEntry);
	void UnlinkId(CostEntry* costEntry);
	int numPoints;
};

