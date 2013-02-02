#pragma once

#include <lua.hpp>
#include <vector>
#include <svl\SVL.h>

struct luaPoly
{
	int numPoints;
	float data[1];	
};

class Output
{
public:
	Output(void);
	~Output(void);
	void Write(char* filename, std::vector<std::vector<Vec2> >& polys, Vec2 dimensions, char* outputScript = 0);
private:
	lua_State* L;
};

