#pragma once

#include <GLFW\GLFWEngine.h>
#include <BasicTexture.h>
#include <vector>
#include <map>
#include <svl\SVL.h>
#include "Reducer.h"
#include "BoundaryFinder.h"
#include "Decomposer.h"
#include "Output.h"

class FrameBufferObject;
class Shader;

struct Vertex
{
	Vec2 position;
	Vec2 normal;
	float score;
	Vertex(Vec2 position, Vec2 normal, float score) { this->position = position; this->normal = normal; this->score = score; }
	Vertex() {}
	bool operator== (const Vertex& other)
	{
		return (position[0] == other.position[0] && position[1] == other.position[1]);
	}

};



class Engine :
	public GLFWEngine
{
public:
	Engine(int argc, char** argv, WindowSettings& w);
	~Engine(void);
	BasicTexture* baseTex;
	unsigned char* imgData;
	float* edgeImage;
	char* filename;
	void Setup();
	void Display();
	void ResizeWindow(int width, int height);
	bool isOpaque(int x, int y);
	void KeyPressed(int code);
	void Update(TimeInfo& timeInfo);
	void FindEdgePoints();
	void ConnectEdges();
	void FindEdges();
	void FindInitialPoint();
	void PopulateCandidates(float x, float y, std::vector<Vertex>& candidates);
private:	
	void DrawPolys();
	std::vector<Vertex> originalPoints;	
	std::vector<Vec2> joinedEdgePoints;
	std::vector<Vec4> polyColours;
	bool drawLines;
	bool reducing;
	bool showCosts;
	bool drawImage;
	Reducer reducer;
	float costThreshold;
	BoundaryFinder bf;
	FrameBufferObject* outFBO;
	bool drawFBO;
	Shader* edgeShader;
	int initialX;
	int initialY;
	Decomposer decomposer;
	bool drawPolys;
	Output output;
};

