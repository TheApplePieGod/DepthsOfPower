#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <vector>
#include <map>
#include <string>
#include <DepthsOfPower/util/basic.h>
#include <glm/vec2.hpp>

#define SOUND_BUFFER_COUNT 4
#define SOUND_BUFFER_SIZE 65536
#define SOUND_MAX_SOURCES 255
#define SOUND_MAX_STREAMING 20

struct file_chunk
{
    char ID[4];
    unsigned long size;
};

struct  wav_file_header
{
    char                RIFF[4];        // RIFF Header      Magic header
    unsigned long       ChunkSize;      // RIFF Chunk Size  
    char                WAVE[4];        // WAVE Header      
    char                fmt[4];         // FMT header       
    unsigned long       Subchunk1Size;  // Size of the fmt chunk                                
    unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
    unsigned short      NumOfChan;      // Number of channels 1=Mono 2=Sterio                   
    unsigned long       SamplesPerSec;  // Sampling Frequency in Hz                             
    unsigned long       bytesPerSec;    // bytes per second 
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
    unsigned short      bitsPerSample;  // Number of bits per sample      
};

struct wav_file
{
    wav_file_header header;
    std::vector<char> data;
};

struct sound
{
    ALuint buffers[SOUND_BUFFER_COUNT];
    bool stream; // enabled for longer songs, but cannot be played from multiple sources
    bool freed = false;
    wav_file soundFile;
    ALenum format;
};

struct sound_settings
{
    bool looping = false;
    f32 gain = 1.f;

    // attenuation
    f32 rolloffFactor = 1.f;
    f32 referenceDistance = 1.f;
    f32 maxDistance = std::numeric_limits<f32>::max();
};

struct sound_source
{
    ALuint id;
    bool stream;
    bool inUse = false;
    bool queuedThisFrame = true;
};

struct streaming_sound
{
    sound* soundData;
    std::string soundIdentifier;
    ALuint source;
    bool inUse = false;
    bool queuedThisFrame = true;
    u64 cursor = SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT;
};

struct looping_sound
{
    bool inUse = false;
    ALuint source;
};

class sound_manager
{
public:
    void Initialize();
    void Cleanup();
    void Tick();

    // returns -1 for non-looping sounds, otherwise index to reference the sound to stop it
    int PlaySoundAtLocation(std::string identifier, glm::vec2 location, sound_settings settings = sound_settings());
    int PlaySound(std::string identifier, sound_settings settings = sound_settings());

    // these only work on looping sounds for now, but that will be changed in the future
    void StopSound(int loopingSoundIndex);
    void UpdateSoundLocation(int loopingSoundIndex, glm::vec2 newLocation);

    void RegisterSound(std::string identifier, const char* path);
    void RemoveSound(std::string identifier);
    wav_file ReadWavFile(const char* path);

    static void UpdateBufferStream(const ALuint source, const ALenum& format, const int& sampleRate, const std::vector<char>& soundData, u64& cursor);
    streaming_sound& GetStreamingSound(int index) { return streamingSounds[index]; }

private:
    bool FindAvailableSource(ALuint& outId, bool isStream);
    bool FindAvailableStream(int& outIndex, ALuint source, sound* soundData, std::string soundIdentifier);

    ALCdevice* oalDevice = nullptr;
    ALCcontext* oalContext = nullptr;
    std::map<std::string, sound> sounds;
    sound_source* sources;
    streaming_sound* streamingSounds;
    std::vector<looping_sound> loopingSounds;
};