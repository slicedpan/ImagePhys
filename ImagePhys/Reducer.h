#pragma once

#include <vector>
#include <map>
#include "svl\SVL.h"

class Reducer
{
public:
	Reducer(void);
	~Reducer(void);
	void CalculateCosts();
	void Reduce();
	void Init(std::vector<Vec2>& originalPoints);
	std::vector<Vec2>& GetPoints() { return points; }
	std::vector<float>& GetCosts() { return currentCost; }
	bool isOpaque(int xCoord, int yCoord);
	float GetMinCost();
private:
	std::vector<Vec2> points;
	std::multimap<float, int> costMap;
	std::vector<float> currentCost;	
	unsigned char* imgData;
};

