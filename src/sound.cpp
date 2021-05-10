#include <DepthsOfPower/engine.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

extern engine* Engine;

// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

bool check_al_errors(const std::string& filename, const std::uint_fast32_t line)
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
        switch(error)
        {
        case AL_INVALID_NAME:
            std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename,
    const std::uint_fast32_t line,
    alFunction function,
    Params... params)
    ->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename,
    const std::uint_fast32_t line,
    alFunction function,
    Params... params)
    ->typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_al_errors(filename, line);
}

#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device)
{
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
        switch(error)
        {
        case ALC_INVALID_VALUE:
            std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case ALC_INVALID_DEVICE:
            std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
            break;
        case ALC_INVALID_CONTEXT:
            std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
            break;
        case ALC_INVALID_ENUM:
            std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
            break;
        case ALC_OUT_OF_MEMORY:
            std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
            break;
        default:
            std::cerr << "UNKNOWN ALC ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template<typename alcFunction, typename... Params>
auto alcCallImpl(const char* filename, 
                 const std::uint_fast32_t line, 
                 alcFunction function, 
                 ALCdevice* device, 
                 Params... params)
->typename std::enable_if_t<std::is_same_v<void,decltype(function(params...))>,bool>
{
    function(std::forward<Params>(params)...);
    return check_alc_errors(filename,line,device);
}

template<typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char* filename,
                 const std::uint_fast32_t line,
                 alcFunction function,
                 ReturnType& returnValue,
                 ALCdevice* device, 
                 Params... params)
->typename std::enable_if_t<!std::is_same_v<void,decltype(function(params...))>,bool>
{
    returnValue = function(std::forward<Params>(params)...);
    return check_alc_errors(filename,line,device);
}

std::mutex streamsMutex;
void Thread_StreamSounds()
{
    while (true)
    {
        for (int i = 0; i < SOUND_MAX_STREAMING; i++)
        {
            streaming_sound& streamingSound = Engine->GetSoundManager().GetStreamingSound(i);
            if (!streamingSound.inUse)
                continue;

            ALint state = AL_PLAYING;        
            alCall(alGetSourcei, streamingSound.source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
                continue;

            //streamsMutex.lock();
            const sound& soundRef = *streamingSound.soundData;
            sound_manager::UpdateBufferStream(streamingSound.source, soundRef.format, soundRef.soundFile.header.SamplesPerSec, soundRef.soundFile.data, streamingSound.cursor);
            //streamsMutex.unlock();
        }
    }
}

void sound_manager::Initialize()
{
    oalDevice = alcOpenDevice(nullptr);
    Assert(oalDevice != nullptr);

    Assert(alcCall(alcCreateContext, oalContext, oalDevice, oalDevice, nullptr) && oalContext != nullptr);

    ALCboolean madeCurrent = false;
    Assert(alcCall(alcMakeContextCurrent, madeCurrent, oalDevice, oalContext) && madeCurrent);

    streamingSounds = new streaming_sound[SOUND_MAX_STREAMING];
    sources = new sound_source[SOUND_MAX_SOURCES];

    for (int i = 0; i < SOUND_MAX_SOURCES; i++)
    {
        alCall(alGenSources, 1, &sources[i].id);
    }

    std::thread(Thread_StreamSounds).detach();
}

void sound_manager::Cleanup()
{
    for (int i = 0; i < SOUND_MAX_SOURCES; i++)
    {
        alCall(alDeleteSources, 1, &sources[i].id);
    }

    for (auto& sound : sounds)
    {
        RemoveSound(sound.first);
    }
    
    ALCboolean madeCurrent;
    alcCall(alcMakeContextCurrent, madeCurrent, oalDevice, nullptr);

    alcCall(alcDestroyContext, oalDevice, oalContext);

    ALCboolean closed;
    alcCall(alcCloseDevice, closed, oalDevice, oalDevice);
}

void sound_manager::Tick()
{
    for (int i = 0; i < SOUND_MAX_SOURCES; i++)
    {
        if (sources[i].inUse && !sources[i].queuedThisFrame)
        {
            ALint state = AL_PLAYING;        
            alCall(alGetSourcei, sources[i].id, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
                sources[i].inUse = false;
        }
        else
            sources[i].queuedThisFrame = false;
    }

    for (int i = 0; i < SOUND_MAX_STREAMING; i++)
    {
        if (streamingSounds[i].inUse && !streamingSounds[i].queuedThisFrame)
        {
            ALint state = AL_PLAYING;        
            alCall(alGetSourcei, streamingSounds[i].source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
            {
                streamingSounds[i].inUse = false;

                // unqueue all queued buffers                
                ALint buffersProcessed = 0;
                alCall(alGetSourcei, streamingSounds[i].source, AL_BUFFERS_PROCESSED, &buffersProcessed);

                if(buffersProcessed <= 0)
                    continue;

                while (buffersProcessed--)
                {
                    ALuint buffer;
                    alCall(alSourceUnqueueBuffers, streamingSounds[i].source, 1, &buffer);
                }

                // fill buffers with inital sound data
                for (int j = 0; j < SOUND_BUFFER_COUNT; j++)
                {
                    alCall(alBufferData, streamingSounds[i].soundData->buffers[j], streamingSounds[i].soundData->format, &streamingSounds[i].soundData->soundFile.data[j * SOUND_BUFFER_SIZE], SOUND_BUFFER_SIZE, streamingSounds[i].soundData->soundFile.header.SamplesPerSec);
                }
            }
        }
        else
            streamingSounds[i].queuedThisFrame = false;
    }
}

void sound_manager::RegisterSound(std::string identifier, const char* path)
{
    if (sounds.count(identifier) > 0)
        Assert(sounds[identifier].freed);

    sound newSound;
    newSound.soundFile = ReadWavFile(path);
    newSound.stream = newSound.soundFile.data.size() > Megabytes(5);
    
    if (newSound.stream)
        alCall(alGenBuffers, SOUND_BUFFER_COUNT, &newSound.buffers[0]);
    else
        alCall(alGenBuffers, 1, &newSound.buffers[0]);

    int sampleRate = newSound.soundFile.header.SamplesPerSec;
    int bitDepth = newSound.soundFile.header.bitsPerSample;
    int numChannels = newSound.soundFile.header.NumOfChan;

    ALenum format;
    if(numChannels == 1 && bitDepth == 8)
        format = AL_FORMAT_MONO8;
    else if(numChannels == 1 && bitDepth == 16)
        format = AL_FORMAT_MONO16;
    else if(numChannels == 2 && bitDepth == 8)
        format = AL_FORMAT_STEREO8;
    else if(numChannels == 2 && bitDepth == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr << "Unrecognized wav format";
        Assert(1==2);
    }
    newSound.format = format;

    if (newSound.stream)
    { // copy initial data into streaming buffers
        for (int i = 0; i < SOUND_BUFFER_COUNT; i++)
        {
            alCall(alBufferData, newSound.buffers[i], format, &newSound.soundFile.data[i * SOUND_BUFFER_SIZE], SOUND_BUFFER_SIZE, sampleRate);
        }
    }
    else // copy whole buffer
        alCall(alBufferData, newSound.buffers[0], format, newSound.soundFile.data.data(), newSound.soundFile.data.size(), sampleRate);

    sounds[identifier] = newSound;
}

void sound_manager::RemoveSound(std::string identifier)
{
    Assert(sounds.count(identifier) > 0);
    sound& soundRef = sounds[identifier];
    
    if (soundRef.stream)
        alCall(alDeleteBuffers, SOUND_BUFFER_COUNT, &soundRef.buffers[0]);
    else
        alCall(alDeleteBuffers, 1, &soundRef.buffers[0]);

    soundRef.freed = true;
}

bool sound_manager::FindAvailableSource(ALuint& outId, bool isStream)
{
    for (int i = 0; i < SOUND_MAX_SOURCES; i++)
    {
        if (!sources[i].inUse)
        {
            sources[i].inUse = true;
            sources[i].queuedThisFrame = true;
            sources[i].stream = isStream;
            outId = sources[i].id;
            return true;
        }
    }
    return false;
}

bool sound_manager::FindAvailableStream(int& outIndex, ALuint source, sound* soundData, std::string soundIdentifier)
{
    for (int i = 0; i < SOUND_MAX_STREAMING; i++)
    {
        if (!streamingSounds[i].inUse)
        {
            streamingSounds[i].cursor = SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT;
            streamingSounds[i].source = source;
            streamingSounds[i].soundData = soundData;
            streamingSounds[i].soundIdentifier = soundIdentifier;
            streamingSounds[i].queuedThisFrame = true;
            outIndex = i;
            return true;
        }
    }
    return false;
}

int sound_manager::PlaySoundAtLocation(std::string identifier, glm::vec2 location, sound_settings settings)
{
    Assert(sounds.count(identifier) > 0);
    sound& soundRef = sounds[identifier];

    ALuint source;
    if (!FindAvailableSource(source, soundRef.stream))
        return -1;

    alCall(alSourcef, source, AL_PITCH, 1);
    alCall(alSourcef, source, AL_GAIN, settings.gain);
    alCall(alSource3f, source, AL_POSITION, location.x, location.y, 0);
    alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
    alCall(alSourcei, source, AL_LOOPING, settings.looping);
    alCall(alSourcef, source, AL_REFERENCE_DISTANCE, settings.referenceDistance);
    alCall(alSourcef, source, AL_ROLLOFF_FACTOR, settings.rolloffFactor);
    alCall(alSourcef, source, AL_MAX_DISTANCE, settings.maxDistance);

    if (soundRef.stream)
    {
        //streamsMutex.lock();
        // check if the streaming sound is already playing, if so, don't play it again
        for (int i = 0; i < SOUND_MAX_STREAMING; i++)
        {
            if (streamingSounds[i].inUse && streamingSounds[i].soundIdentifier == identifier)
                return -1;
        }

        int streamIndex = 0;
        if (!FindAvailableStream(streamIndex, source, &soundRef, identifier))
            return -1;

        alCall(alSourceQueueBuffers, source, SOUND_BUFFER_COUNT, &soundRef.buffers[0]);
        alCall(alSourcePlay, source);

        streamingSounds[streamIndex].inUse = true;
        //streamsMutex.unlock();
    }
    else
    {
        alCall(alSourcei, source, AL_BUFFER, soundRef.buffers[0]);
        alCall(alSourcePlay, source);
    }

    if (settings.looping)
    {
        looping_sound newSound;
        newSound.inUse = true;
        newSound.source = source;

        for (u64 i = 0; i < loopingSounds.size(); i++)
        {
            if (!loopingSounds[i].inUse)
            {
                loopingSounds[i] = newSound;
                return static_cast<int>(i);
            }
        }
    
        loopingSounds.push_back(newSound);
        return static_cast<int>(loopingSounds.size() - 1);
    }

    return -1;
}

int sound_manager::PlaySound(std::string identifier, sound_settings settings)
{
    return PlaySoundAtLocation(identifier, glm::vec2(0.f, 0.f), settings);
}

void sound_manager::StopSound(int loopingSoundIndex)
{
    Assert(loopingSoundIndex != -1 && loopingSounds[loopingSoundIndex].inUse);

    alCall(alSourceStop, loopingSounds[loopingSoundIndex].source);
    loopingSounds[loopingSoundIndex].inUse = false;
}

void sound_manager::UpdateSoundLocation(int loopingSoundIndex, glm::vec2 newLocation)
{
    Assert(loopingSoundIndex != -1 && loopingSounds[loopingSoundIndex].inUse);

    alCall(alSource3f, loopingSounds[loopingSoundIndex].source, AL_POSITION, newLocation.x, newLocation.y, 0);
}

void sound_manager::UpdateBufferStream(const ALuint source, const ALenum& format, const int& sampleRate, const std::vector<char>& soundData, u64& cursor)
{
    ALint buffersProcessed = 0;
    alCall(alGetSourcei, source, AL_BUFFERS_PROCESSED, &buffersProcessed);

    if(buffersProcessed <= 0)
        return;

    while (buffersProcessed--)
    {
        ALuint buffer;
        alCall(alSourceUnqueueBuffers, source, 1, &buffer);

        u64 dataSizeToCopy = SOUND_BUFFER_SIZE;
        if (cursor + SOUND_BUFFER_SIZE > soundData.size())
            dataSizeToCopy = soundData.size() - cursor;

        // todo: fill extra data when dataSizeToCopy < SOUND_BUFFER_SIZE with zero ?

        alCall(alBufferData, buffer, format, soundData.data() + cursor, dataSizeToCopy, sampleRate);
        alCall(alSourceQueueBuffers, source, 1, &buffer);

        cursor += dataSizeToCopy;
    }
}

wav_file sound_manager::ReadWavFile(const char* path)
{
    std::ifstream ifs(path, std::ifstream::in | std::ifstream::binary);
    
    std::streampos fileSize = ifs.tellg();
    ifs.seekg(0, std::ifstream::end);
    fileSize = ifs.tellg() - fileSize;
    ifs.seekg(0, std::ifstream::beg);

    wav_file wavFile;
    ifs.read((char*)&wavFile.header, sizeof(wav_file_header));

    // navigate to data chunk
    file_chunk chunk;
    while (true)
    {
        ifs.read((char*)&chunk, sizeof(file_chunk));
        if (*(unsigned int*)&chunk.ID == 0x61746164)
            break;
        ifs.seekg(chunk.size, std::ifstream::cur); // skip chunk
    }
    
    wavFile.data.resize(chunk.size, 0);

    ifs.read(wavFile.data.data(), chunk.size);
    
    return wavFile;
}