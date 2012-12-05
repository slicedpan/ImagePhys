#include "Reducer.h"

Reducer::Reducer(void)
{
}

Reducer::~Reducer(void)
{
}

void Reducer::CalculateCosts()
{
	costMap.clear();
	int pointNum = points.size();
	if (pointNum < 3)
		return;
	for (int i = 0; i < pointNum; ++i)
	{
		Vec2 current;
		Vec2 partner;
		Vec2 middle;
		current = points[i];
		partner = points[(i + 2) % pointNum];
		middle = points[(i + 1) % pointNum];
		float length = len(current - partner);
		float cost = ((middle[0] - current[0]) * (partner[1] - current[1]) - (middle[1] - current[1]) * (partner[0] - current[0]));
		cost = abs(cost) / length;
		costMap.insert(std::pair<float, int>(cost, i));
		currentCost[i] = cost;
	}
}

void Reducer::Init(std::vector<Vec2>& originalPoints)
{
	points.clear();
	points.insert(points.begin(), originalPoints.begin(), originalPoints.end());
	currentCost.resize(points.size());
	CalculateCosts();
}

float Reducer::GetMinCost()
{
	if (costMap.empty())
		return 0.0f;
	return costMap.begin()->first;
}

void Reducer::Reduce()
{
	auto iter = costMap.begin();
	float firstCost = iter->first;
	++iter;
	float secondCost = iter->first;
	if (firstCost > secondCost)
		throw;
	points.erase(points.begin() + (costMap.begin()->second + 1) % points.size());
	CalculateCosts();
}


