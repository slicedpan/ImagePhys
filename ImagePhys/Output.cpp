#include "Output.h"
#include <lauxlib.h>
#include <lualib.h>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream>

static int getArray(lua_State* L)
{
	const std::vector<Vec2>& poly = (*(std::vector<Vec2>*)lua_touserdata(L, lua_upvalueindex(1)));
	int index = luaL_checkint(L, 2);
	--index;
	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushnumber(L, poly[index][0]);
	lua_settable(L, -3);
	lua_pushstring(L, "y");
	lua_pushnumber(L, poly[index][1]);
	lua_settable(L, -3);
	return 1;
}

static void printValue(lua_State* L, int index)
{
	int t = lua_type(L, index);
	switch (t)
	{

	case LUA_TSTRING:  /* strings */
		printf("`%s'", lua_tostring(L, index));
		break;

	case LUA_TBOOLEAN:  /* booleans */
		printf(lua_toboolean(L, index) ? "true" : "false");
		break;

	case LUA_TNUMBER:  /* numbers */
		printf("%g", lua_tonumber(L, index));
		break;

	default:  /* other values */
		printf("%s", lua_typename(L, t));
		break;

	}
}

static void stackDump (lua_State *L) {
	int i;
	int top = lua_gettop(L);
	printf("Stack contents:\n");
	for (i = 1; i <= top; i++) 
	{  /* repeat for each level */
		int t = lua_type(L, i);
		printf("%d: ", i);
		printValue(L, i);
		printf("\n");  /* put a separator */
	}
	printf("\n");  /* end the listing */
}

Output::Output(void)
{

}

Output::~Output(void)
{

}

static int printTable(lua_State* L, int index)
{
	lua_pushnil(L);
	if (index < 0)
		--index;
	stackDump(L);
	while (lua_next(L, index) != 0)
	{
		lua_pushvalue(L, -2);
		printValue(L, -1);
		printf(" - ");
		printValue(L, -2);
		lua_pop(L, 2);
	}
	lua_pop(L, 1);
	return 0;
}

static int super_callback(lua_State* L)
{
	printf("callback called\n");
	return 0;
}

static int testFunc(lua_State* L)
{
	stackDump(L);
	printTable(L, -2);
	return 0;
}

static int outputFunc(lua_State* L)	//emulates print functionality
{
	int numArgs = lua_gettop(L);
	std::ostringstream& outputStream = *(std::ostringstream*)lua_touserdata(L, lua_upvalueindex(1));
	for (int i = 1; i <= numArgs; ++i)
	{
		const char* str = lua_tostring(L, i);
		outputStream << str;
		if (i != numArgs)
			outputStream << "    ";
	}
	outputStream << std::endl;
	return 0;
}

void Output::Write(char* filename, std::vector<std::vector<Vec2> >& polys, Vec2 dimensions, char* outputScript)
{
	L = luaL_newstate();   /* opens Lua */
	luaL_openlibs(L);

	std::ostringstream outputStream;

	lua_pushglobaltable(L);
	lua_pushstring(L, "print");
	lua_pushlightuserdata(L, &outputStream);
	lua_pushcclosure(L, outputFunc, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "filename");
	lua_pushstring(L, filename);
	lua_settable(L, -3);

	lua_pop(L, 1);
	
	int result;
	if (!outputScript) result = luaL_loadfile(L, "default.lua");
	else result = luaL_loadfile(L, outputScript);

	switch (result)
	{
	case LUA_ERRFILE:
		{
			printf("could not open %s for reading!\n", outputScript);
			lua_close(L);
			return;
		}
	case LUA_ERRSYNTAX:
		{
			printf("Syntax error while parsing %s:\n", outputScript);
			printf("%s\n", lua_tostring(L, -1));
			lua_close(L);
			return;
		}
	case LUA_ERRMEM:
		{
			printf("Memory allocation error while loading %s\n", outputScript);
			lua_close(L);
			return;
		}
	case LUA_ERRGCMM:
		{
			printf("Garbage collector error!\n");
			lua_close(L);
			return;
		}
	case LUA_OK:
		printf("file: %s loaded ok\n", outputScript);
	}

	result = lua_pcall(L, 0, 0, 0);

	if (result)
	{
		printf("error reading %s:\n", outputScript);
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		return;
	}

	lua_getglobal(L, "writeBegin");	
	lua_pushnumber(L, polys.size());

	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushnumber(L, 256);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushnumber(L, 256);
	lua_settable(L, -3);

	lua_pushstring(L, filename);

	result = lua_pcall(L, 3, 0, 0);

	if (result)
	{
		printf("could not execute function 'writeBegin':\n%s\n", lua_tostring(L, -1));
	}

	for (int i = 0; i < polys.size(); ++i)
	{
		lua_getglobal(L, "writePoly");

		lua_newtable(L);
		lua_pushstring(L, "size");
		lua_pushnumber(L, polys[i].size());
		lua_settable(L, -3);

		lua_newtable(L);
		lua_pushstring(L, "__index");
		lua_pushlightuserdata(L, &polys[i]);
		lua_pushcclosure(L, getArray, 1);
		lua_settable(L, -3);

		lua_setmetatable(L, -2);
		//stackDump(L);
		lua_pushnumber(L, i + 1);
		lua_pcall(L, 2, 0, 0);
	}

	lua_getglobal(L, "writeEnd");
	result = lua_pcall(L, 0, 0, 0);
	if (result)
	{
		printf("could not execute function 'writeEnd':\n%s\n", lua_tostring(L, -1));
	}

	lua_pushglobaltable(L);
	lua_pushstring(L, "filename");
	lua_gettable(L, -2);
	std::string newFilename(lua_tostring(L, -1));
	if (!newFilename.compare(filename))
		newFilename.append(".phys");

	std::ofstream outputFile(newFilename);
	if (outputFile.is_open())
	{
		outputFile << outputStream.str();
	}
	lua_close(L);	
}

