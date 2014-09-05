#include "engine_impl.h"

#pragma warning(disable:4996)

#include <cassert>
#include "wrapper.h"

const unsigned long COLOR_BLACK = 0xFF000000; // pure black

EngineImpl::EngineImpl(const char* title, bool(*update)(), bool(*render)(), int width, int height) :
							hge_(hgeCreate(HGE_VERSION)),
                            vertexes_(0)
{
	assert(hge_);

	hge_->System_SetState(HGE_FRAMEFUNC, update);
	hge_->System_SetState(HGE_RENDERFUNC, render);
	hge_->System_SetState(HGE_FPS, HGEFPS_UNLIMITED);
	hge_->System_SetState(HGE_SCREENBPP, 32);
	hge_->System_SetState(HGE_USESOUND, false);
	hge_->System_SetState(HGE_TITLE, title);
	hge_->System_SetState(HGE_HIDEMOUSE, false);
	hge_->System_SetState(HGE_ZBUFFER, false);
	hge_->System_SetState(HGE_WINDOWED, true);
	hge_->System_SetState(HGE_SHOWSPLASH, true);
	hge_->System_SetState(HGE_SCREENWIDTH, width);
	hge_->System_SetState(HGE_SCREENHEIGHT, height);

	auto result = hge_->System_Initiate();
	assert(result == true);

	font_.reset(new hgeFont("font/font.fnt"));
}

EngineImpl::~EngineImpl()
{
	hge_->Release();
	hge_ = 0;
}


void EngineImpl::BeginScene()
{
	hge_->Gfx_BeginScene();
	hge_->Gfx_Clear(COLOR_BLACK);
}

void EngineImpl::EndScene()
{
	hge_->Gfx_EndScene();
}


void EngineImpl::CreateTextureAtlas(unsigned int width, unsigned int height)
{
	atlas_index_ = hge_->Texture_Create(width, height);
	assert(atlas_index_);
}

void EngineImpl::LoadTextureAndCopyToAtlas(const char* data, unsigned int lenght, int x, int y, int dst_width, int dst_height)
{
	HTEXTURE src_tex = hge_->Texture_Load(data, lenght, false);
	assert(src_tex);

	int src_w = hge_->Texture_GetWidth(src_tex, true);
	int src_h = hge_->Texture_GetHeight(src_tex, true);

	DWORD* src = hge_->Texture_Lock(src_tex, true, 0, 0, src_w, src_h);
	DWORD* dst = hge_->Texture_Lock(atlas_index_, false, x, y, dst_width, dst_height);

	auto src_pitch = hge_->Texture_GetWidth(src_tex);
	auto dst_pitch = hge_->Texture_GetWidth(atlas_index_);

	/* 
	Поля x, y указывают координаты, куда должен быть помещен файл - кадр,
	а width, height ширину и высоту области, которая выделена в атласе под файл - кадр.
	Важно, что ширина и высота загружаемого файла могут быть больше, чем width, height !!!
	Это связано с тем, что при создании атласа может быть применено масштабирование.
	Поэтому необходимо уметь загружать файл - кадр на атлас с учетом масштаба. */

	if (src_w <= dst_width)
	{
		for (int i = 0; i < src_h; ++i)
		{
			int src_pos = i*src_pitch;
			int dst_pos = i*dst_pitch;
			size_t size = sizeof(DWORD)*src_w;
			DWORD* start_ptr = src + src_pos;
			DWORD* last_ptr = start_ptr + src_w;
			DWORD* output_ptr = dst + dst_pos;
			std::copy(start_ptr, last_ptr, output_ptr);
		}
	}
	else
	{
		float ratio_w = float(dst_width) / float(src_w);
		float ratio_h = float(dst_height) / float(src_h);

		for (int i = 0; i < src_w; i++)
		{
			for (int j = 0; j < src_h; j++)
			{
				DWORD color = src[j*src_pitch + i];
				int w = static_cast<int>(float(i) / ratio_w);
				int h = static_cast<int>(float(j) / ratio_h);
				dst[w + dst_pitch*h] = color;
			}
		}
	}

	hge_->Texture_Unlock(atlas_index_);
	hge_->Texture_Unlock(src_tex);
	hge_->Texture_Free(src_tex);
}

void EngineImpl::BeginDrawQuad(int* max_primitives)
{
	vertexes_ = hge_->Gfx_StartBatch(HGEPRIM_QUADS, atlas_index_, BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE, max_primitives);
}

void EngineImpl::CopyParticleToBuffer(int current_particle, const MAGIC_PARTICLE_VERTEXES& v)
{
	int base_pos = current_particle * 4;
	int base_v1 = base_pos + 0;
	vertexes_[base_v1].x = v.vertex1.x;
	vertexes_[base_v1].y = v.vertex1.y;
	vertexes_[base_v1].z = 0;
	vertexes_[base_v1].col = v.color;
	vertexes_[base_v1].tx = v.u1;
	vertexes_[base_v1].ty = v.v1;

	int base_v2 = base_pos + 1;
	vertexes_[base_v2].x = v.vertex2.x;
	vertexes_[base_v2].y = v.vertex2.y;
	vertexes_[base_v2].z = 0;
	vertexes_[base_v2].col = v.color;
	vertexes_[base_v2].tx = v.u2;
	vertexes_[base_v2].ty = v.v2;

	int base_v3 = base_pos + 2;
	vertexes_[base_v3].x = v.vertex3.x;
	vertexes_[base_v3].y = v.vertex3.y;
	vertexes_[base_v3].z = 0;
	vertexes_[base_v3].col = v.color;
	vertexes_[base_v3].tx = v.u3;
	vertexes_[base_v3].ty = v.v3;

	int base_v4 = base_pos + 3;
	vertexes_[base_v4].x = v.vertex4.x;
	vertexes_[base_v4].y = v.vertex4.y;
	vertexes_[base_v4].z = 0;
	vertexes_[base_v4].col = v.color;
	vertexes_[base_v4].tx = v.u4;
	vertexes_[base_v4].ty = v.v4;
}

void EngineImpl::EndDrawQuad(int count)
{
	assert(vertexes_);
	assert(count > 0);
	hge_->Gfx_FinishBatch(count);
	vertexes_ = 0;
}

void EngineImpl::PrintPerfomance(int count_particles)
{
	auto fps = hge_->Timer_GetFPS();
	font_->printf(0, 0, HGETEXT_LEFT, "\nFPS:%d\nparticles:%d", fps, count_particles);
}

double EngineImpl::GetDeltaTimeInSeconds() const
{
	double delta_in_msec = static_cast<double>(hge_->Timer_GetDelta());
	double delta_in_sec = delta_in_msec * 1000;
	return delta_in_sec;
}

void EngineImpl::Run() const
{
	hge_->System_Start();
}

void EngineImpl::test_logic(ParticlesWrapper& wrapper)
{
	if (hge_->Input_GetKeyState(HGEK_SPACE))
	{
		wrapper.Stop();
	}

	if (hge_->Input_GetKeyState(HGEK_ENTER))
	{
		wrapper.Play();
	}

	if (hge_->Input_GetKeyState(HGEK_F5))
	{
		wrapper.Restart();
	}
}