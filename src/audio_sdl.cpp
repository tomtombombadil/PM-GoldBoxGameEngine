// Sound out (cyd/src/platform/audio.h) through SDL2: the engine hands over
// a filler of 8-bit unsigned samples at kAudioHz - or, for the music
// (engine 0.198), of 16-bit ones (audio_wide) - and SDL's audio thread
// pulls from it; the device is 16-bit, the 8-bit sounds widened. The boards' rules still hold - the filler runs on another thread
// than the game's (their sound task), and audio_stop returns only once
// that thread is out of the filler.
#include "host.h"

#include "platform/audio.h"

#include <SDL.h>

#include <cstring>

namespace {

SDL_AudioDeviceID dev = 0;
AudioFill   fill_fn = nullptr;      // (changed only inside SDL_LockAudioDevice)
AudioFill16 fill16_fn = nullptr;    // (the one or the other)
void*       fill_ctx = nullptr;
bool      held = false;

void pull(void*, Uint8* out8, int len)
{
    int16_t* out = reinterpret_cast<int16_t*>(out8);
    const int total = len / 2;
    int done = 0;
    // (the engine fills whole buffers: silence after a sound's end)
    if (fill16_fn) {
        if (!fill16_fn(out, total, fill_ctx)) fill16_fn = nullptr;
        done = total;
    } else if (fill_fn) {
        uint8_t part[512];
        while (fill_fn && done < total) {
            const int n = total - done < static_cast<int>(sizeof part) ? total - done : static_cast<int>(sizeof part);
            if (!fill_fn(part, n, fill_ctx)) fill_fn = nullptr;
            for (int i = 0; i < n; ++i) out[done + i] = static_cast<int16_t>((part[i] - 128) * 256);
            done += n;
        }
    }
    if (done < total) memset(out + done, 0, static_cast<size_t>(total - done) * 2);
}

} // namespace

namespace pm {

bool audio_begin()
{
    if (dev) return true;
    SDL_AudioSpec want{}, got{};
    want.freq = kAudioHz;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = pull;
    dev = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0);     // (SDL converts to what the device takes)
    if (!dev) {
        SDL_Log("no sound: %s", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(dev, 0);
    return true;
}

void audio_end()
{
    if (dev) SDL_CloseAudioDevice(dev);
    dev = 0;
    fill_fn = nullptr;
    fill16_fn = nullptr;
}

} // namespace pm

void audio_play(AudioFill fill, void* ctx)
{
    if (!dev || !fill) return;
    SDL_LockAudioDevice(dev);
    if (!fill_fn && !fill16_fn) {   // (one running: it picks up new sounds itself)
        fill_fn = fill;
        fill_ctx = ctx;
    }
    SDL_UnlockAudioDevice(dev);
}

bool audio_wide() { return true; }

void audio_play16(AudioFill16 fill, void* ctx)
{
    if (!dev || !fill) return;
    SDL_LockAudioDevice(dev);
    if (!fill_fn && !fill16_fn) {   // (the caller stops what plays first)
        fill16_fn = fill;
        fill_ctx = ctx;
    }
    SDL_UnlockAudioDevice(dev);
}

void audio_stop()
{
    if (!dev) return;
    SDL_LockAudioDevice(dev);       // (waits for the callback to return)
    fill_fn = nullptr;
    fill16_fn = nullptr;
    SDL_UnlockAudioDevice(dev);
}

bool audio_running()
{
    if (!dev) return false;
    SDL_LockAudioDevice(dev);
    const bool on = fill_fn != nullptr || fill16_fn != nullptr;
    SDL_UnlockAudioDevice(dev);
    return on;
}

// A fight's hold on the output: the device is always open here
void audio_hold(bool on, AudioFill, void*) { held = on && dev; }
bool audio_held() { return held; }
void audio_prepare() {}
