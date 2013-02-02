#include "BoundaryFinder.h"
#include <map>
#include <tuple>


void BoundaryFinder::Init(unsigned char* imgData, int width, int height)
{
	this->imgData = imgData;
	this->width = width;
	this->height = height;
}

bool BoundaryFinder::isOpaque(int x, int y)
{
	unsigned char* alpha = imgData + (x + (y * width)) * 4 + 3;
	return (*alpha > 2);
}

void BoundaryFinder::FindEdges()
{
	leftEdge = -1;
	rightEdge = width;
	topEdge = -1;
	bottomEdge = height;
	bool edgeFound = false;
	while (!edgeFound)
	{		
		++leftEdge;
		for (int i = 0; i < height; ++i)
		{
			if (isOpaque(leftEdge, i))
			{
				edgeFound = true;
				break;
			}
		}
	}
	edgeFound = false;
	while (!edgeFound)
	{
		--rightEdge;
		for (int i = 0; i < height; ++i)
		{
			if (isOpaque(rightEdge, i))
			{
				edgeFound = true;
				break;
			}
		}
	}
	edgeFound = false;
	while (!edgeFound)
	{		
		++topEdge;
		for (int i = 0; i < width; ++i)
		{
			if (isOpaque(i, topEdge))
			{
				edgeFound = true;
				break;
			}
		}
	}
	edgeFound = false;
	while (!edgeFound)
	{
		--bottomEdge;
		for (int i = 0; i < width; ++i)
		{
			if (isOpaque(i, bottomEdge))
			{
				edgeFound = true;
				break;
			}
		}
	}
}

BoundaryFinder::~BoundaryFinder(void)
{
}

void BoundaryFinder::GetInitialPoints()
{
	points.clear();
	for (int i = topEdge; i <= bottomEdge; ++i)
	{
		int j = leftEdge;
		while (!isOpaque(j, i))
			++j;
		points.emplace_back(j, i);
	}

	for (int i = bottomEdge; i >= topEdge; --i)
	{
		int j = rightEdge;
		while (!isOpaque(j, i))
			--j;
		points.emplace_back(j, i);
	}
	
}
