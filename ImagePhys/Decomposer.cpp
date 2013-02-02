#include "Decomposer.h"

Decomposer::Decomposer(void)
{
}

Decomposer::~Decomposer(void)
{
}

void Decomposer::Decompose(const std::vector<Vec2>& points)
{
	basePoints.clear();
	polys.clear();
	basePoints.insert(basePoints.begin(), points.begin(), points.end());
	Points.Init(&basePoints);
	int basePos = 0;
	bool done = false;
	int numIterations = 0;
	while(basePoints.size() > 3 && numIterations < 10000)
	{
		while (basePos > basePoints.size())
			basePos -= basePoints.size();
		int convexCount = 0;	//if all corners are convex, then we're already done
		while (TurnDir(Points[basePos - 1], Points[basePos], Points[basePos + 1]) && convexCount < basePoints.size())
		{
			++basePos;	//move to the next concave corner
			++convexCount;
		}
		if (convexCount == basePoints.size())	//all convex, can just add the remaining points
			break;
		int testPos = basePos + 2;
		int pointCount = 2;		//starting point and next one
		while (TurnDir(Points[testPos], Points[basePos], Points[basePos + 1]) && TurnDir(Points[testPos - 2], Points[testPos - 1], Points[testPos]))
		{
			if (IntersectsBoundary(Points[basePos], Points[testPos], testPos))
			{
				break;
			}
			++pointCount;
			++testPos;
		}
		if (pointCount > 2)
		{
			std::vector<Vec2> poly;
			CopyRange(poly, basePos, testPos);
			RemovePoints(basePos + 1, testPos - 1);
			polys.push_back(poly);			
		}
		++basePos;
		++numIterations;
	}
	polys.push_back(basePoints);
}

void Decomposer::RemovePoints(std::vector<Vec2>& points)
{
	int guess = 0;
	for (int i = 0; i < points.size(); ++i)
	{
		for (int j = 0; j < basePoints.size(); ++j)
		{
			if (Points[i + guess] == points[i])
			{
				basePoints.erase(basePoints.begin() + guess);
				guess += j;
				break;
			}
		}
	}
}

void Decomposer::CopyRange(std::vector<Vec2>& dest, int start, int end)
{
	while (start < 0)
		start += basePoints.size();
	while (end < 0)
		end += basePoints.size();
	start %= basePoints.size();
	end %= basePoints.size();
	if (start < end)
	{
		dest.insert(dest.end(), basePoints.begin() + start, basePoints.begin() + end);
	}
	else
	{
		dest.insert(dest.end(), basePoints.begin() + start, basePoints.end());
		dest.insert(dest.end(), basePoints.begin(), basePoints.begin() + end);
	}
}

void Decomposer::RemovePoints(int start, int end)
{
	while (start < 0)
		start += basePoints.size();
	while (end < 0)
		end += basePoints.size();
	start %= basePoints.size();
	end %= basePoints.size();
	if (start < end)
	{
		basePoints.erase(basePoints.begin() + start, basePoints.begin() + end);
	}
	else
	{
		basePoints.erase(basePoints.begin() + start, basePoints.end());
		basePoints.erase(basePoints.begin(), basePoints.begin() + end);
	}
}

bool Decomposer::TurnDir(Vec2 v1, Vec2 v2, Vec2 v3)
{
	float angle1 = atan2(v2[1] - v1[1], v2[0] - v1[0]) * 57.2957795f;
	float angle2 = atan2(v3[1] - v2[1], v3[0] - v2[0]) * 57.2957795f;
	while (angle1 < 0.0f)
		angle1 += 360.0f;
	while (angle1 > angle2)
		angle2 += 360.0f;
	return (angle2 - angle1) < 180.0f;
}

bool Decomposer::IntersectsBoundary(Vec2 v1, Vec2 v2, int startPoint)
{
	for (int i = 0; i < basePoints.size(); ++i)
	{
		Vec2& u1 = Points[i];
		Vec2& u2 = Points[i + 1];
		if (v1 == u1 || v1 == u2)
		{
			return false;
		}
		if (v2 == u1 || v2 == u2)
		{
			return false;
		}
		if (TurnDir(v1, u1, u2) != TurnDir(v2, u1, u2) && TurnDir(v1, v2, u1) != TurnDir(v1, v2, u2))
		{
			return true;
		}
	}
	return false;
}
