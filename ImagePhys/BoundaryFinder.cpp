#include "BoundaryFinder.h"


BoundaryFinder::BoundaryFinder(unsigned char* imgData, int width, int height)
{
	this->imgData = imgData;
}


BoundaryFinder::~BoundaryFinder(void)
{
}

bool BoundaryFinder::isOpaque(int x, int y)
{

}

void BoundaryFinder::GetInitialPoints()
{

}

void BoundaryFinder::FindEdges()
{
	leftEdge = -1;
	rightEdge = baseTex->Width;
	topEdge = -1;
	bottomEdge = baseTex->Height;
	bool edgeFound = false;
	while (!edgeFound)
	{		
		++leftEdge;
		for (int i = 0; i < baseTex->Height; ++i)
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
		for (int i = 0; i < baseTex->Height; ++i)
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
		for (int i = 0; i < baseTex->Width; ++i)
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
		for (int i = 0; i < baseTex->Width; ++i)
		{
			if (isOpaque(i, bottomEdge))
			{
				edgeFound = true;
				break;
			}
		}
	}
}

