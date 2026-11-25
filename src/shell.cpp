#include "shell.h"

#ifdef __ANDROID__

#include "video.h"

#include <jni.h>

#include <cstring>

#include <atomic>
#include <mutex>
#include <string>

namespace shell {
namespace {

std::atomic<uint32_t> ev_type{0};
bool                  laid_out = false;      // (the game's thread only)
State                 told = State::Pages;
bool                  told_any = false;

// The latest layout from the app (any thread writes, the game's thread reads)
std::mutex lay_mu;
int        lay[9] = {};
bool       lay_new = false;

enum : Sint32 { kButton = 1, kLayout = 2, kMenu = 3 };

void (*menu_fn)(int, int, int) = nullptr;

void push(Sint32 code, intptr_t a, intptr_t b)
{
    const uint32_t t = ev_type.load();
    if (!t) return;
    SDL_Event e;
    SDL_zero(e);
    e.type = t;
    e.user.code = code;
    e.user.data1 = reinterpret_cast<void*>(a);
    e.user.data2 = reinterpret_cast<void*>(b);
    SDL_PushEvent(&e);
}

// A call to the activity (GoldBoxActivity) from the game's thread
void call(const char* name, const char* sig, int arg, bool with_arg)
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, name, sig);
    if (m) {
        if (with_arg) env->CallVoidMethod(act, m, static_cast<jint>(arg));
        else env->CallVoidMethod(act, m);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
}

} // namespace

bool on() { return laid_out; }
bool wanted() { return true; }

void begin()
{
    if (ev_type.load()) return;
    const Uint32 t = SDL_RegisterEvents(1);
    if (t != static_cast<Uint32>(-1)) ev_type.store(t);
}

bool take(const SDL_Event& e, void (*press)(int, bool))
{
    const uint32_t t = ev_type.load();
    if (!t || e.type != t) return false;
    if (e.user.code == kButton) {
        if (press) press(static_cast<int>(reinterpret_cast<intptr_t>(e.user.data1)),
                         reinterpret_cast<intptr_t>(e.user.data2) != 0);
    } else if (e.user.code == kMenu) {
        const intptr_t packed = reinterpret_cast<intptr_t>(e.user.data2);
        if (menu_fn)
            menu_fn(static_cast<int>(reinterpret_cast<intptr_t>(e.user.data1)), static_cast<int>(packed >> 16),
                    static_cast<int16_t>(packed & 0xFFFF));
    } else if (e.user.code == kLayout) {
        int v[9];
        {
            std::lock_guard<std::mutex> g(lay_mu);
            if (!lay_new) return true;
            lay_new = false;
            for (int i = 0; i < 9; ++i) v[i] = lay[i];
        }
        laid_out = true;
        video::set_shell(SDL_Rect{v[0], v[1], v[2], v[3]}, SDL_Rect{v[4], v[5], v[6], v[7]}, v[8]);
    }
    return true;
}

void state(State s)
{
    if (told_any && s == told) return;
    told_any = true;
    told = s;
    call("onShellState", "(I)V", static_cast<int>(s), true);
}

void companion() { call("onShellCompanion", "()V", 0, false); }

void set_menu(void (*fn)(int, int, int)) { menu_fn = fn; }

void pad_style(int size, int alpha)
{
    static int last = -1;
    const int v = size * 1000 + alpha;
    if (v == last) return;
    last = v;
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetMethodID(c, "onShellPad", "(II)V");
    if (m) env->CallVoidMethod(act, m, static_cast<jint>(size), static_cast<jint>(alpha));
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
}

void text(int channel, const std::string& t)
{
    static std::string sent[ChCount];
    static bool any[ChCount] = {};
    if (channel < 0 || channel >= ChCount) return;
    if (any[channel] && sent[channel] == t) return;
    any[channel] = true;
    sent[channel] = t;
    // (bytes, not a jstring: the engine's words may hold any byte - Java reads them as UTF-8)
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return;
    jbyteArray b = env->NewByteArray(static_cast<jsize>(t.size()));
    if (b) {
        env->SetByteArrayRegion(b, 0, static_cast<jsize>(t.size()), reinterpret_cast<const jbyte*>(t.data()));
        jclass c = env->GetObjectClass(act);
        jmethodID m = env->GetMethodID(c, "onShellText", "(I[B)V");
        if (m) env->CallVoidMethod(act, m, static_cast<jint>(channel), b);
        env->DeleteLocalRef(c);
        env->DeleteLocalRef(b);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(act);
}

bool save_art(const std::string& jpeg, const uint8_t* rgba, int w, int h, const std::string& out)
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject act = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !act) return false;
    bool ok = false;
    jclass c = env->GetObjectClass(act);
    jmethodID m = env->GetStaticMethodID(c, "saveArt", "(Ljava/lang/String;[BIILjava/lang/String;)Z");
    if (m) {
        jstring jj = env->NewStringUTF(jpeg.c_str());
        jstring jo = env->NewStringUTF(out.c_str());
        jbyteArray px = nullptr;
        if (rgba && w > 0 && h > 0) {
            px = env->NewByteArray(w * h * 4);
            if (px) env->SetByteArrayRegion(px, 0, w * h * 4, reinterpret_cast<const jbyte*>(rgba));
        }
        ok = env->CallStaticBooleanMethod(c, m, jj, px, static_cast<jint>(w), static_cast<jint>(h), jo);
        env->DeleteLocalRef(jj);
        env->DeleteLocalRef(jo);
        if (px) env->DeleteLocalRef(px);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        ok = false;
    }
    env->DeleteLocalRef(c);
    env->DeleteLocalRef(act);
    return ok;
}

} // namespace shell

// ---- The app's calls (GoldBoxActivity's static natives; Android's UI thread)
extern "C" {

// A pad button down / up (main.cpp's Btn numbers)
JNIEXPORT void JNICALL Java_io_github_tomtombombadil_goldbox_GoldBoxActivity_nativeButton(JNIEnv*, jclass, jint b,
                                                                                         jboolean down)
{
    shell::push(shell::kButton, b, down ? 1 : 0);
}

// Where the game goes (gx, gy, gw, gh) and the Companion (cx, cy, cw, ch; w 0: none), in the surface's pixels;
// view: the Companion's pictures shown - 0 both, 1 the map (a fight: the battlefield), 2 the party, 3 both - the map
// as wide as the room at its top, the party under it in what is left (the app's column sideways)
JNIEXPORT void JNICALL Java_io_github_tomtombombadil_goldbox_GoldBoxActivity_nativeLayout(
    JNIEnv*, jclass, jint gx, jint gy, jint gw, jint gh, jint cx, jint cy, jint cw, jint ch, jint view)
{
    {
        std::lock_guard<std::mutex> g(shell::lay_mu);
        const int v[9] = {gx, gy, gw, gh, cx, cy, cw, ch, view};
        for (int i = 0; i < 9; ++i) shell::lay[i] = v[i];
        shell::lay_new = true;
    }
    shell::push(shell::kLayout, 0, 0);
}

// The menu drawer's taps (shell::MenuOp)
JNIEXPORT void JNICALL Java_io_github_tomtombombadil_goldbox_GoldBoxActivity_nativeMenu(JNIEnv*, jclass, jint op,
                                                                                       jint index, jint value)
{
    shell::push(shell::kMenu, op, (static_cast<intptr_t>(index) << 16) | (static_cast<intptr_t>(value) & 0xFFFF));
}

} // extern "C"

#else // not Android: nothing

namespace shell {
bool on() { return false; }
bool wanted() { return false; }
void begin() {}
bool take(const SDL_Event&, void (*)(int, bool)) { return false; }
void state(State) {}
void companion() {}
void set_menu(void (*)(int, int, int)) {}
void pad_style(int, int) {}
void text(int, const std::string&) {}
bool save_art(const std::string&, const uint8_t*, int, int, const std::string&) { return false; }
} // namespace shell

#endif
