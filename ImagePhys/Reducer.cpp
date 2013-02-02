#include "Reducer.h"
#include <set>

Reducer::Reducer(void)
{
	firstId = 0;
	firstCost = 0;
	lastId = 0;
	lastCost = 0;
	numPoints = 0;
}

Reducer::~Reducer(void)
{
}

void Reducer::Clear()
{
	if (firstId == 0 && firstCost == 0)
		return;
	CostEntry* cur;
	cur = lastId;
	firstId->prevId = 0;
	while (cur)
	{
		CostEntry* next = cur->prevId;
		delete cur;
		cur = next;
	}
	firstId == 0;
	firstCost == 0;
	lastId == 0;
	lastCost == 0;
}

void Reducer::Build()
{
	Clear();
	firstId = new CostEntry();
	CostEntry* cur = firstId;
	for (int i = 0; i < points.size() - 1; ++i)
	{
		cur->id = i;
		cur->nextId = new CostEntry();
		cur->nextId->prevId = cur;		
		cur->nextCost = 0;
		cur->prevCost = 0;
		cur = cur->nextId;
	}
	lastId = cur;
	lastId->id = points.size() - 1;
	lastId->nextCost = 0;
	lastId->prevCost = 0;
	lastId->nextId = firstId;
	firstId->prevId = lastId;
}

void Reducer::UnlinkId(CostEntry* costEntry)
{
	if (costEntry == firstId)
	{
		costEntry->nextId->prevId = lastId;
		lastId->nextId = costEntry->nextId;
		firstId = costEntry->nextId;
	}
	else if (costEntry == lastId)
	{
		costEntry->prevId->nextId = firstId;
		firstId->prevId = costEntry->prevId;
		lastId = costEntry->prevId;
	}
	else
	{
		costEntry->nextId->prevId = costEntry->prevId;
		costEntry->prevId->nextId = costEntry->nextId;
	}
}

float Reducer::CalculateCost(CostEntry* costEntry)
{
	Vec2 prev;
	Vec2 next;
	Vec2 current;
	prev = points[costEntry->prevId->id];
	next = points[costEntry->nextId->id];
	current = points[costEntry->id];
	float length = len(prev - next);
	float cost = ((current[0] - prev[0]) * (next[1] - prev[1]) - (current[1] - prev[1]) * (next[0] - prev[0]));
	return abs(cost) / length;	
}

void Reducer::UnlinkCost(CostEntry* costEntry)
{
	if (costEntry == firstCost)
	{
		firstCost = costEntry->nextCost;
		firstCost->prevCost = 0;
	}
	else if (costEntry == lastCost)
	{
		lastCost = costEntry->prevCost;
		lastCost->nextCost = 0;
	}
	else
	{
		costEntry->nextCost->prevCost = costEntry->prevCost;
		costEntry->prevCost->nextCost = costEntry->nextCost;
	}
}

void Reducer::LinkCost(CostEntry* costEntry)
{
	if (costEntry->cost > lastCost->cost)
	{
		lastCost->nextCost = costEntry;
		costEntry->prevCost = lastCost;
		lastCost = costEntry;
	}
	else if (costEntry->cost <= firstCost->cost)
	{
		firstCost->prevCost = costEntry;
		costEntry->nextCost = firstCost;
		firstCost = costEntry;
	}
	else
	{
		CostEntry* cur = lastCost->prevCost;
		while (cur)
		{
			if (costEntry->cost > cur->cost)
				break;
			cur = cur->prevCost;
		}
		cur->nextCost->prevCost = costEntry;
		costEntry->nextCost = cur->nextCost;
		cur->nextCost = costEntry;
		costEntry->prevCost = cur;
	}
}

void Reducer::CalculateCosts()
{
	int pointNum = points.size();
	if (pointNum < 3)
		return;
	CostEntry* cur = firstId;
	firstCost = cur;
	lastCost = cur;
	cur->cost = CalculateCost(cur);
	cur = cur->nextId;
	while (cur)
	{				
		cur->cost = CalculateCost(cur);
		LinkCost(cur);
		if (cur == lastId)
			break;
		cur = cur->nextId;		
	}
}

void Reducer::Init(std::vector<Vec2>& originalPoints)
{
	points.clear();
	points.insert(points.begin(), originalPoints.begin(), originalPoints.end());
	numPoints = originalPoints.size();
	Build();
	CalculateCosts();
}

float Reducer::GetMinCost()
{
	if (!firstCost || numPoints <= 4)
		return 1e38f;
	return firstCost->cost;
}

void Reducer::Reduce()
{
	CostEntry* cur = firstCost;
	CostEntry* next = firstCost->nextId;
	CostEntry* prev = firstCost->prevId;
	UnlinkCost(cur);
	UnlinkId(cur);
	UnlinkCost(next);
	next->cost = CalculateCost(next);
	LinkCost(next);
	UnlinkCost(prev);
	prev->cost = CalculateCost(prev);
	LinkCost(prev);
	--numPoints;
	delete cur;
}

std::vector<Vec2>& Reducer::GetPoints()
{
	std::vector<Vec2> reducedPoints;
	if (numPoints != points.size())
	{		
		lastId->nextId = 0;
		CostEntry* cur = firstId;
		while (cur)
		{
			reducedPoints.push_back(points[cur->id]);
			cur = cur->nextId;
		}
		points.clear();
		points.insert(points.begin(), reducedPoints.begin(), reducedPoints.end());
		lastId->nextId = firstId;
	}
	return points;
}

std::vector<float>& Reducer::GetCosts()
{
	if (currentCost.size() != numPoints)
	{
		currentCost.clear();
		CostEntry* cur = firstId;
		lastId->nextId = 0;
		while (cur)
		{
			currentCost.push_back(cur->cost);
			cur = cur->nextId;
		}
		lastId->nextId = firstId;
	}
	return currentCost;
}


