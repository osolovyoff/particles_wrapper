#ifndef wrapper_h__
#define wrapper_h__

#include <magic/magic.h>

class EngineImpl;

class ParticlesWrapper
{
public:
		   ParticlesWrapper(const char* file_name, EngineImpl& engine, int emitter_number = 0);

	void   Update();
	void   Render() const;
	void   Restart();

	void   Play() { is_play_ = true; }
	void   Stop() { is_play_ = false; }

	bool   IsLoop() const;
	bool   IsPlay() const { return is_play_; }

	void   SetPos(float x, float y);
	void   SetLoopMode(bool loop_mode);
	float  GetX() const;
	float  GetY() const;
	int    GetEmitterCount(const char* file_name) const;

private:
	void   ResetAtlas();

private:
	HM_EMITTER  emitter_;
	EngineImpl& engine_;

	bool        is_play_;
	bool        is_loop_;
};

#endif // !wrapper_h__
