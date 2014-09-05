#ifndef main_h__
#define main_h__

#include "application.h"
Application* g_app = nullptr;

int main(int argc, char* argv[])
{
	const char* file_name = argv[1];
	Application app(file_name, 800, 600);
	g_app = &app;

	app.Run();

	return 0;
}

#endif // main_h__
