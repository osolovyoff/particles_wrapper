#include "wrapper.h"

#include <cassert>
#include "engine_impl.h"

ParticlesWrapper::ParticlesWrapper(const char* file_name, EngineImpl& engine, int emitter_number) :
engine_(engine), emitter_(0), is_play_(false), is_loop_(false)
{
	assert(file_name);

	int emitter_count = GetEmitterCount(file_name);
	assert(emitter_number <= emitter_count);

	HM_STREAM stream = Magic_StreamOpenFile(file_name, MAGIC_STREAM_READ);
	assert(stream);
	HM_FILE file = Magic_OpenStream(stream);
	assert(MAGIC_ERROR != file);

	Magic_SetAxis(MAGIC_pXpY);
	Magic_SetCleverModeForAtlas(true);
	Magic_SetStartingScaleForAtlas(1.f);

	MAGIC_FIND_DATA find;
	const char* path = Magic_FindFirst(file, &find, MAGIC_EMITTER);
	assert(path != nullptr);
	
	while (path)
	{
		if (emitter_number == find.index)
		{
			emitter_ = Magic_LoadEmitter(file, path);
			assert(emitter_);
			break;
		}
		path = Magic_FindNext(file, &find);
	}
		
	int result = MAGIC_ERROR;
	result = Magic_SetInterpolationMode(emitter_, false);
	assert(MAGIC_SUCCESS == result );
	result = Magic_SetEmitterPositionMode(emitter_, true);
	assert(MAGIC_SUCCESS == result );
	result = Magic_SetEmitterDirectionMode(emitter_, true);
	assert(MAGIC_SUCCESS == result );
	result = Magic_SetLoopMode(emitter_, MAGIC_NOLOOP);
	assert(MAGIC_SUCCESS == result );

	//const int max_atlas_width = 1024;
	//const int max_atlas_height = 1024;
	Magic_CreateAtlasesForEmitters(1024, 
		1024, // max  
		1, // count emitters
		&emitter_, 
		1, 
		0.1f); // TODO future place for improve

	ResetAtlas();
	
	result = Magic_StreamClose(stream);
	assert(MAGIC_SUCCESS == result );

	result = Magic_CloseFile(file);
	assert(MAGIC_SUCCESS == result );

	is_play_ = true;
}

void ParticlesWrapper::Update()
{
	double delta = engine_.GetDeltaTimeInSeconds();

	if (is_play_)
	{
		bool emitter_is_complete = !Magic_Update(emitter_, delta);

		if (emitter_is_complete)
		{
			if (!is_loop_)
			{
				is_play_ = false;
			}
		}
	}
}

void ParticlesWrapper::Render() const
{
	MAGIC_RENDERING rendering;
	int count_particles = 0;

	if (Magic_InInterval(emitter_))
	{
		int result = Magic_CreateFirstRenderedParticlesList(emitter_, &rendering);
		assert(MAGIC_SUCCESS == result );

		count_particles = rendering.count;
		while (rendering.count)
		{
			int max_count = 0;
			engine_.BeginDrawQuad(&max_count);

			if (rendering.count <= max_count)
			{
				MAGIC_PARTICLE_VERTEXES v;
				for (int i = 0; i < rendering.count; ++i)
				{
					int result = Magic_GetNextParticleVertexes(&v);
					assert(MAGIC_SUCCESS == result );
					engine_.CopyParticleToBuffer(i, v);
					
				}
				engine_.EndDrawQuad(rendering.count);
			}
			else
			{
				int current_count = rendering.count;

				while (current_count > 0)
				{
					if (current_count > max_count)
					{
						engine_.BeginDrawQuad(&max_count);
						MAGIC_PARTICLE_VERTEXES v;
						for (int i = 0; i < max_count; ++i)
						{
							int result = Magic_GetNextParticleVertexes(&v);
							assert(MAGIC_SUCCESS == result);
							engine_.CopyParticleToBuffer(i, v);
						}
						engine_.EndDrawQuad(max_count);
						current_count -= max_count;
					}
					else
					{
						engine_.BeginDrawQuad(&max_count);
						MAGIC_PARTICLE_VERTEXES v;
						for (int i = 0; i < current_count; ++i)
						{
							int result = Magic_GetNextParticleVertexes(&v);
							assert(MAGIC_SUCCESS == result);
							engine_.CopyParticleToBuffer(i, v);
						}
						engine_.EndDrawQuad(current_count);
						current_count = 0;
					}
				}
			}
			Magic_CreateNextRenderedParticlesList(&rendering);
		}
	}
	engine_.PrintPerfomance(count_particles);
}

void ParticlesWrapper::ResetAtlas()
{
	MAGIC_CHANGE_ATLAS change;

	while (Magic_GetNextAtlasChange(&change) == MAGIC_SUCCESS)
	{
		switch (change.type)
		{
		case MAGIC_CHANGE_ATLAS_CREATE:
			engine_.CreateTextureAtlas(change.width, change.height);
			break;
		case MAGIC_CHANGE_ATLAS_LOAD:
			engine_.LoadTextureAndCopyToAtlas(change.data, change.length, change.x, change.y, change.width, change.height);
			break;
		default:
			assert(0);
			break;
		}
	}
}

void ParticlesWrapper::SetPos(float x, float y)
{
	MAGIC_POSITION pos;
	pos.x = x;
	pos.y = y;
	int result = Magic_SetEmitterPosition(emitter_, &pos);
	assert(MAGIC_SUCCESS == result );
}

float ParticlesWrapper::GetX() const
{
	MAGIC_POSITION pos;
	int result = Magic_GetEmitterPosition(emitter_, &pos);
	assert(MAGIC_SUCCESS == result );
	return pos.x;
}

float ParticlesWrapper::GetY() const
{
	MAGIC_POSITION pos;
	int result = Magic_GetEmitterPosition(emitter_, &pos);
	assert(MAGIC_SUCCESS == result );
	return pos.y;
}

void ParticlesWrapper::Restart()
{
	Magic_Restart(emitter_);
	is_play_ = true;
}

int ParticlesWrapper::GetEmitterCount(const char* file_name) const
{
	HM_STREAM stream = Magic_StreamOpenFile(file_name, MAGIC_STREAM_READ);
	assert(0 != stream);
	HM_FILE file = Magic_OpenStream(stream);
	assert(MAGIC_ERROR != file);
	MAGIC_FIND_DATA find;
	const char* path = Magic_FindFirst(file, &find, MAGIC_EMITTER);
	assert(nullptr != path );

	int emitter_count = 1;
	++emitter_count;

	while (path)
	{
		path = Magic_FindNext(file, &find);
		++emitter_count;
	}
	auto result = Magic_StreamClose(stream);
	assert(MAGIC_SUCCESS == result );
	result = Magic_CloseFile(file);
	assert(MAGIC_SUCCESS == result );
	return emitter_count;
}

bool ParticlesWrapper::IsLoop() const
{
	int mode = Magic_GetLoopMode(emitter_);
	assert(MAGIC_SUCCESS == mode);
	bool is_loop = false;
	switch (mode)
	{
	case MAGIC_LOOP:
		is_loop = true;
		break;
	case MAGIC_NOLOOP:
		break;
	case MAGIC_FOREVER:
		assert(0);
		break;
	default:
		assert(0);
		break;
	}
	return is_loop;
}

void ParticlesWrapper::SetLoopMode(bool loop_mode)
{
	int mode = loop_mode ? MAGIC_LOOP : MAGIC_NOLOOP;
	int result = Magic_SetLoopMode(emitter_, mode);
	assert(MAGIC_SUCCESS == result);
}
