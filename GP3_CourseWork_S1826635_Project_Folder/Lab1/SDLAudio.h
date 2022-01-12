#pragma once
#include <SDL/SDL_mixer.h>
#include <vector>
class SDLAudio
{
	public:
		SDLAudio();
		~SDLAudio();
		
		void addSoundEffect(const char* path);
		void addAudioTrack(const char* path);
		void playSoundEffect(const int which) const;
		void playMusic();

	private:
		Mix_Music* gMusic;
		std::vector<Mix_Chunk*> mSoundEffectBank;
};

