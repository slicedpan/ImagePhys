#include "Engine.h"
#include <GL/glew.h>
#include <GL/glfw.h>
#include "WindowSettings.h"
#include "GLGUI\Primitives.h"
#include "FrameBufferObject.h"
#include "Shader.h"
#include "QuadDrawer.h"
#include "MiscUtils.h"
#include "Colour.h"
#include <map>


Engine::Engine(int argc, char** argv, WindowSettings& w)
	: GLFWEngine(w)
{
	filename = argv[1];	
}

Engine::~Engine(void)
{
	if (edgeImage)
		free(edgeImage);
	free(imgData);
}

void Engine::Setup()
{
	edgeImage = 0;
	baseTex = new BasicTexture(filename);
	baseTex->Load();
	imgData = (unsigned char*)malloc(sizeof(unsigned char) * baseTex->Width * baseTex->Height * 4);
	glBindTexture(GL_TEXTURE_2D, baseTex->GetId());
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);	
	ResizeWindow(Window.Width, Window.Height);

	drawLines = true;
	reducing = false;
	drawPolys = false;
	drawImage = true;
	showCosts = false;
	costThreshold = 2.0f;

	outFBO = new FrameBufferObject(baseTex->Width, baseTex->Height, 0, 0, GL_RGBA, GL_TEXTURE_2D, "out");
	outFBO->AttachTexture("colour", GL_LINEAR, GL_LINEAR);

	drawFBO = true;
	edgeShader = new Shader("edgeVert.glsl", "edgeFrag.glsl", "edge");
	if (!edgeShader->Compile())
		printf("%s", edgeShader->GetErrorLog());
}

void Engine::ResizeWindow(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, baseTex->Width, baseTex->Height, 0.0, 0.0, 1.0);
	glViewport(0, 0, width, height);
}

void Engine::FindEdgePoints()
{
	outFBO->Bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	edgeShader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baseTex->GetId());
	edgeShader->Uniforms["baseTex"].SetValue(0);
	Vec2 pixSize = Vec2(baseTex->Width, baseTex->Height);
	edgeShader->Uniforms["pixSize"].SetValue(pixSize);
	glGetError();
	QuadDrawer::DrawQuad(Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f));
	glUseProgram(0);
	outFBO->Unbind();
	edgeImage = (float*)malloc(sizeof(float) * baseTex->Width * baseTex->Height * 4);
	glBindTexture(GL_TEXTURE_2D, outFBO->GetTextureID(0));
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, edgeImage);	
	FindInitialPoint();
}

void Engine::FindInitialPoint()
{
	Image<float, 4> image(edgeImage, baseTex->Width, baseTex->Height);
	bool found = false;
	std::vector<float> rayHeights;
	rayHeights.push_back(0.5);
	rayHeights.push_back(0.25);
	rayHeights.push_back(0.75);
	rayHeights.push_back(0.125);
	rayHeights.push_back(0.375);
	rayHeights.push_back(0.625);
	rayHeights.push_back(0.875);
	int currentRay = 0;
	while (!found)
	{
		if (currentRay < rayHeights.size())
		{
			int y = baseTex->Height * rayHeights[currentRay];
			for (int i = 0; i < baseTex->Width; ++i)
			{
				if (image[i][y].b > 0.01f)
				{
					found = true;
					initialX = i;
					initialY = y;
					break;
				}
			}
			++currentRay;
		}
		else
		{
			for (int j = 0; j < baseTex->Height && !found; ++j)
			{
				for (int i = 0; i < baseTex->Width; ++i)
				{
					if (image[i][j].b > 0.01f)
					{
						found = true;
						initialX = i;
						initialY = j;
						break;
					}
				}
			}
		}
	}
}

Vec2 NormFromPixel(const Pixel<float, 4>& pixel)
{
	return Vec2((pixel.r - 0.5f) * 2.0f, (pixel.g - 0.5f) * 2.0f);
}

void AddCandidateIfEdge(int x, int y, Image<float, 4>& image, std::vector<Vertex>& candidates)
{
	if (image[x][y].b > 0.01)
	{
		Vertex v(Vec2(x, y), NormFromPixel(image[x][y]), 0.0f);
		candidates.push_back(v);
	}
}

void Engine::PopulateCandidates(float fx, float fy, std::vector<Vertex>& candidates)
{
	int x = (int)fx;
	int y = (int)fy;
	Image<float, 4> image(edgeImage, baseTex->Width, baseTex->Height);
	AddCandidateIfEdge(x, y + 1, image, candidates);
	AddCandidateIfEdge(x, y - 1, image, candidates);
	AddCandidateIfEdge(x + 1, y, image, candidates);
	AddCandidateIfEdge(x + 1, y + 1, image, candidates);
	AddCandidateIfEdge(x + 1, y - 1, image, candidates);
	AddCandidateIfEdge(x - 1, y + 1, image, candidates);
	AddCandidateIfEdge(x - 1, y, image, candidates);
	AddCandidateIfEdge(x - 1, y - 1, image, candidates);
}

void Engine::ConnectEdges()
{
	joinedEdgePoints.clear();
	Image<float, 4> image(edgeImage, baseTex->Width, baseTex->Height);
	Vertex first(Vec2(initialX, initialY), NormFromPixel(image[initialX][initialY]), 0.0f);
	joinedEdgePoints.push_back(first.position);
	bool done = false;
	Vertex last = first;	
	Vec2 lastDir = Vec2(0.0, -1.0);
	while (!done)
	{
		std::vector<Vertex> candidates;
		PopulateCandidates(last.position[0], last.position[1], candidates);
		float bestScore = 0.0f;
		int best = 0;
		Vec2 dir;
		for (int i = 0; i < candidates.size(); ++i)
		{
			if (last == candidates[i]) continue;
			dir = candidates[i].position - last.position;
			float curScore = dot(dir, lastDir) + dot(candidates[i].normal, last.normal);
			if (curScore > bestScore)
			{
				bestScore = curScore;
				best = i;
			}
		}
		lastDir = candidates[best].position - last.position;
		last.position = candidates[best].position;
		last.normal = candidates[best].normal;
		if (last == first)
		{
			done = true;
			break;
		}
		joinedEdgePoints.push_back(last.position);
	}
}

void Engine::DrawPolys()
{

	while (polyColours.size() < decomposer.GetPolys().size())
	{
		polyColours.push_back(Colour::Random());
	}
	for (int i = 0; i < decomposer.GetPolys().size(); ++i)
	{
		std::vector<Vec2>& poly = decomposer.GetPolys()[i];
		glBegin(GL_POLYGON);
		glColor4f(polyColours[i][0], polyColours[i][1], polyColours[i][2], 0.5f);
		for (int j = 0; j < poly.size(); ++j)
		{
			glVertex2f(poly[j][0], poly[j][1]);
		}
		glVertex2f(poly[0][0], poly[0][1]);
		glEnd();
	}
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
	if (drawImage)
	{
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
	}

	if (drawFBO)
	{
		glBindTexture(GL_TEXTURE_2D, outFBO->GetTextureID(0));
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
	}

	glDisable(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<Vec2>& points = reducer.GetPoints();

	char buf[1024];
	sprintf(buf, "Cost Threshold: %lf, Min Cost: %lf, No. of points: %d", 
		costThreshold, reducer.GetMinCost(), reducer.GetPoints().size());
	glfwSetWindowTitle(buf);

	if (drawPolys)
	{
		DrawPolys();
		return;
	}

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
			if (i == 0)
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			if (i == 1)
				glColor4f(0.0, 0.0, 1.0f, 1.0f);
			Vec2 centre(baseX + points[i][0] + 0.5f, baseY + points[i][1] + 0.5f);
			float pointRadius = showCosts ? reducer.GetCosts()[(i + 1) % points.size()] * 2.0f : 1.5f;
			glVertex2f(centre[0] - pointRadius, centre[1] - pointRadius);
			glVertex2f(centre[0] - pointRadius, centre[1] + pointRadius);
			glVertex2f(centre[0] + pointRadius, centre[1] + pointRadius);
			glVertex2f(centre[0] + pointRadius, centre[1] - pointRadius);
			glVertex2f(centre[0] - pointRadius, centre[1] - pointRadius);
			glEnd();
		}
	}

	

}

void Engine::KeyPressed(int code)
{
	if (code == GLFW_KEY_ESC)
		Exit();
	if (code == 'H')
		drawLines = !drawLines;
	if (code == 'B')
	{
		FindEdgePoints();
		ConnectEdges();
		reducer.Init(joinedEdgePoints);
		while (reducer.GetMinCost() < costThreshold)
			reducer.Reduce();
		decomposer.Decompose(reducer.GetPoints());
	}
	if (code == 'I')
		drawImage = !drawImage;
	if (code == 'F')
		drawFBO = !drawFBO;
	if (code == 'R')
	{
		if (!edgeShader->Compile())
			printf("%s", edgeShader->GetErrorLog());
		else
			printf("shader compiled\n");
		int numColours = polyColours.size();
		polyColours.clear();
		for (int i = 0; i < numColours; ++i)
			polyColours.push_back(Colour::Random());
	}
	if (code == 'C')
	{
		showCosts = !showCosts;
	}	
	if (code == 'P')
		drawPolys = !drawPolys;
	if (code == 'O')
		output.Write(filename, decomposer.GetPolys(), Vec2(baseTex->Width, baseTex->Height));
}

void Engine::Update(TimeInfo& timeInfo)
{
	if (KeyState[GLFW_KEY_UP])
		costThreshold += 0.01f;
	if (KeyState[GLFW_KEY_DOWN])
		costThreshold -= 0.01f;
}
