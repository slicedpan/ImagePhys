#pragma once

#include <vector>
#include <svl\SVL.h>

class Decomposer
{
public:
	Decomposer(void);
	~Decomposer(void);
	void Decompose(const std::vector<Vec2>& points);
	std::vector<Vec2> basePoints;
	void RemovePoints(std::vector<Vec2>& points);
	void RemovePoints(int start, int end);
	bool TurnDir(Vec2 v1, Vec2 v2, Vec2 v3);
	bool IntersectsBoundary(Vec2 v1, Vec2 v2, int startPoint = 0);
	void CopyRange(std::vector<Vec2>& dest, int start, int end);
	std::vector<std::vector<Vec2> >& GetPolys() { return polys; }
	class vecWrapper
	{
	public:
		void Init(std::vector<Vec2>* points) { this->points = points; }		
		Vec2& operator[] (int index) 
		{
			while (index < 0)
				index += points->size();
			return (*points)[index % points->size()]; 
		}
	private:
		std::vector<Vec2>* points;		
	} Points;
private:
	std::vector<std::vector<Vec2> > polys;
};

