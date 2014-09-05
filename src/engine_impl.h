#ifndef engine_h__
#define engine_h__

#include <memory>

#include "hge/hge.h"
#include "hge/hgefont.h"
#include "magic/magic.h"

class ParticlesWrapper;

class EngineImpl
{
public:
	EngineImpl(const char * title, bool(*update)(), bool(*render)(), int width, int height);
	~EngineImpl();

	void Run() const;
	void BeginScene();
	void EndScene();

	void BeginDrawQuad(int* max_prim);
	void EndDrawQuad(int count);

	void CopyParticleToBuffer(int current_particle, const MAGIC_PARTICLE_VERTEXES& v);

	void CreateTextureAtlas(unsigned int width, unsigned int height);
	void LoadTextureAndCopyToAtlas(const char* data, unsigned int lenght, int x, int y, int dest_width, int dest_height);
	
	double GetDeltaTimeInSeconds() const;

	void PrintPerfomance(int count_particles);
	void test_logic(ParticlesWrapper& wrapper);

private:
	EngineImpl& operator=(EngineImpl r);

private:
	HGE*        hge_;
	hgeVertex*  vertexes_;
	int         atlas_index_;

	std::unique_ptr<hgeFont> font_;
};
#endif // !engine_h__
