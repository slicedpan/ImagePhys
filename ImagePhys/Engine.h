#pragma once

#include <GLFW\GLFWEngine.h>
#include <BasicTexture.h>
#include <vector>
#include <map>
#include <svl\SVL.h>
#include "Reducer.h"

class Engine :
	public GLFWEngine
{
public:
	Engine(int argc, char** argv, WindowSettings& w);
	~Engine(void);
	BasicTexture* baseTex;
	unsigned char* imgData;
	char* filename;
	void Setup();
	void Display();
	void ResizeWindow(int width, int height);
	void FindEdges();
	void GetInitialPoints();
	bool isOpaque(int x, int y);
	void KeyPressed(int code);
	void Update(TimeInfo& timeInfo);
private:	
	std::vector<Vec2> originalPoints;
	
	bool drawLines;
	bool reducing;
	Reducer reducer;
	float costThreshold;
};

