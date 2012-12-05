#include "Engine.h"
#include <GL\glew.h>
#include <GL\glfw.h>
#include "WindowSettings.h"
#include "GLGUI\Primitives.h"
#include <map>


Engine::Engine(int argc, char** argv, WindowSettings& w)
	: GLFWEngine(w)
{
	filename = argv[1];	
}

bool Engine::isOpaque(int x, int y)
{
	unsigned char* alpha = imgData + (x + (y * baseTex->Width)) * 4 + 3;
	return (*alpha > 2);
}

void Engine::GetInitialPoints()
{
	for (int i = topEdge; i <= bottomEdge; ++i)
	{
		int j = leftEdge;
		while (!isOpaque(j, i))
			++j;
		originalPoints.emplace_back(j, i);
	}

	for (int i = bottomEdge; i >= topEdge; --i)
	{
		int j = rightEdge;
		while (!isOpaque(j, i))
			--j;
		originalPoints.emplace_back(j, i);
	}



}

void Engine::FindEdges()
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

Engine::~Engine(void)
{
}

void Engine::Setup()
{
	baseTex = new BasicTexture(filename);
	baseTex->Load();
	imgData = (unsigned char*)malloc(sizeof(unsigned char) * baseTex->Width * baseTex->Height * 4);
	glBindTexture(GL_TEXTURE_2D, baseTex->GetId());
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);	
	ResizeWindow(Window.Width, Window.Height);
	FindEdges();
	GetInitialPoints();
	drawLines = true;
	reducing = false;
	costThreshold = 2.0f;
}

void Engine::ResizeWindow(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, baseTex->Width, baseTex->Height, 0.0, 0.0, 1.0);
	glViewport(0, 0, width, height);
}

void Engine::Display()
{
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	int baseX = Window.Width - baseTex->Width;
	baseX /= 2;
	int baseY = Window.Height - baseTex->Height;
	baseY /= 2;

	baseX = 0;
	baseY = 0;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBindTexture(GL_TEXTURE_2D, baseTex->GetId());
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glVertex2f(baseX, baseY);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(baseX, baseY + baseTex->Height);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(baseX + baseTex->Width, baseY + baseTex->Height);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(baseX + baseTex->Width, baseY);
	glTexCoord2f(0.0, 0.0);
	glEnd();
	glDisable(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<Vec2>& points = reducer.GetPoints();
	
	if (!points.empty() && drawLines)
	{
		glBegin(GL_LINE_STRIP);
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
		for (int i = 0; i < points.size(); ++i)
		{
			glVertex2f(baseX + points[i][0] + 0.5f, baseY + points[i][1] + 0.5f);
		}
		glVertex2f(baseX + points[0][0] + 0.5f, baseY + points[0][1] + 0.5f);
		glEnd();
			
		for (int i = 0; i < points.size(); ++i)
		{
			glBegin(GL_LINE_STRIP);	
			glColor4f(1.0f, 0.0, 0.0f, 0.4f);
			Vec2 centre(baseX + points[i][0] + 0.5f, baseY + points[i][1] + 0.5f);
			float pointRadius = reducer.GetCosts()[(i + 1) % points.size()] * 2.0f;
			glVertex2f(centre[0] - pointRadius, centre[1] - pointRadius);
			glVertex2f(centre[0] - pointRadius, centre[1] + pointRadius);
			glVertex2f(centre[0] + pointRadius, centre[1] + pointRadius);
			glVertex2f(centre[0] + pointRadius, centre[1] - pointRadius);
			glVertex2f(centre[0] - pointRadius, centre[1] - pointRadius);
			glEnd();
		}
	}

	char buf[1024];
	sprintf(buf, "Cost Threshold: %lf, Min Cost: %lf, No. of points: %d", 
				costThreshold, reducer.GetMinCost(), reducer.GetPoints().size());
	glfwSetWindowTitle(buf);

}

void Engine::KeyPressed(int code)
{
	if (code == GLFW_KEY_ESC)
		Exit();
	if (code == 'H')
		drawLines = !drawLines;
	if (code == 'I')
	{
		reducer.Init(originalPoints);
		while (reducer.GetMinCost() < costThreshold)
			reducer.Reduce();
	}
}

void Engine::Update(TimeInfo& timeInfo)
{
	if (KeyState[GLFW_KEY_UP])
		costThreshold += 0.01f;
	if (KeyState[GLFW_KEY_DOWN])
		costThreshold -= 0.01f;
}
