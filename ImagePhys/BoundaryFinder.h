#pragma once

#include <vector>
#include "svl\SVL.h"

class BoundaryFinder
{
public:
	BoundaryFinder(unsigned char* imgData, int width, int height);
	~BoundaryFinder(void);
	bool isOpaque(int x, int y);
	void FindEdges();
	std::vector<Vec2>& GetPoints() { return points; }
	void GetInitialPoints();
	void FindEdges();
private:
	int leftEdge, rightEdge, bottomEdge, topEdge;
	unsigned char* imgData;
	std::vector<Vec2> points;
};

