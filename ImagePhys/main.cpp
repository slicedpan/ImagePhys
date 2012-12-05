#include "Engine.h"
#include "WindowSettings.h"

int main(int argc, char **argv)
{
	WindowSettings w;
	w.OpenGLVersionMajor = 2;
	w.OpenGLVersionMinor = 1;
	
	Engine engine(argc, argv, w);
	if (argc < 2)
		return 0;
	engine.Run();
}