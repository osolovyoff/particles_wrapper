#ifndef application_h__
#define application_h__

#include <string>
#include <memory>

class ParticlesWrapper;
typedef std::shared_ptr<ParticlesWrapper> ParticlesWrapperShared;
class EngineImpl;

class Application
{
public:
	explicit Application(const char* ptc_file_name, int width, int height);
    virtual  ~Application();

	void Run();
	void Render();
	bool Update();

private:
	static bool UpdateFunction();
	static bool RenderFunction();

private:
	std::string				    file_name_;
	ParticlesWrapperShared      wrapper_;
	std::unique_ptr<EngineImpl> engine_;
};

extern Application* g_app;
Application& GetApp();

#endif // !application_h__
