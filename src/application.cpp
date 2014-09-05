#include "application.h"

#include <cassert>

#include "engine_impl.h"
#include "wrapper.h"

Application::Application(const char* file_name, int width, int height) : 
file_name_(file_name ? file_name : ""),
engine_(new EngineImpl("Particles Viewer", 
				&Application::UpdateFunction,
				&Application::RenderFunction,
				width,
				height)
)
{
	assert(!file_name_.empty());
}

Application::~Application()
{
	wrapper_.reset();
}

void Application::Run()
{
	wrapper_.reset(new ParticlesWrapper(file_name_.c_str(), *engine_, 1));
	wrapper_->SetPos(400, 400);

	engine_->Run();
}

bool Application::Update()
{
	engine_->test_logic(*wrapper_);

	wrapper_->Update();
	return false;
}

void Application::Render()
{
	engine_->BeginScene();
	wrapper_->Render();
	engine_->EndScene();
}

// static or free function
bool Application::UpdateFunction()
{
	return GetApp().Update();
}

bool Application::RenderFunction()
{
	GetApp().Render();
	return false;
}

Application& GetApp()
{
	assert(g_app);
	return *g_app;
}


