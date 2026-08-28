// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Adds a lightweight video player to the Windows 11 File Explorer preview pane
// @version         0.1.0
// @author          Reyr
// @include         prevhost.exe
// @architecture    x86-64
// @compilerOptions -lole32 -lmfplay -lmfplat -lmfuuid -lshlwapi
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Useful Side View

Turns the Windows 11 File Explorer preview pane into a responsive video player.
It uses Windows Media Foundation, creates no decoder until a video is selected,
and runs only inside the isolated Preview Handler Host (`prevhost.exe`).

## Controls

- Click the video to play or pause.
- Use the bottom button to play/pause.
- Drag the timeline to seek.
- Use the speaker button to mute/unmute.

Sound and autoplay are disabled by default. Both can be enabled independently
in the mod settings. A file only plays when its installed Windows codecs are
supported by Media Foundation.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- autoplay: false
  $name: Autoplay videos
  $description: Start playing as soon as a video is ready.
- sound: false
  $name: Enable sound
  $description: Allow audio. Videos are muted by default.
*/
// ==/WindhawkModSettings==

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfplay.h>
#include <shlwapi.h>

#include <atomic>
#include <cwchar>
#include <mutex>
#include <string>

namespace {

constexpr wchar_t kWindowClass[] = L"WindhawkUsefulSideView";
constexpr wchar_t kVideoClass[] = L"WindhawkUsefulSideViewVideo";
constexpr UINT_PTR kUiTimer = 1;
constexpr UINT kRefreshMs = 200;
constexpr int kControlsHeight = 42;
constexpr int kPlayButtonWidth = 52;
constexpr int kSoundButtonWidth = 52;

struct Settings {
    bool autoplay;
    bool sound;
};

Settings g_settings{};
std::mutex g_pathMutex;
std::wstring g_pendingPath;
std::wstring g_activePath;
std::atomic<bool> g_stopping{false};
HANDLE g_workerThread;
HWND g_playerWindow;
HWND g_videoWindow;
HWND g_hostWindow;
IMFPMediaPlayer* g_player;

using CreateFileW_t = decltype(&CreateFileW);
CreateFileW_t CreateFileW_Original;

bool IsVideoPath(const wchar_t* path) {
    if (!path || !*path || PathIsDirectoryW(path)) return false;
    const wchar_t* extension = PathFindExtensionW(path);
    static const wchar_t* extensions[] = {
        L".mp4", L".m4v", L".mov", L".wmv", L".avi", L".mkv",
        L".webm", L".mpg", L".mpeg", L".mts", L".m2ts", L".3gp"};
    for (auto candidate : extensions) {
        if (_wcsicmp(extension, candidate) == 0) return true;
    }
    return false;
}

void RememberVideo(const wchar_t* path) {
    if (!IsVideoPath(path)) return;
    wchar_t fullPath[MAX_PATH * 4];
    DWORD length = GetFullPathNameW(path, ARRAYSIZE(fullPath), fullPath, nullptr);
    std::lock_guard<std::mutex> lock(g_pathMutex);
    g_pendingPath.assign(length && length < ARRAYSIZE(fullPath) ? fullPath : path);
}

HANDLE WINAPI CreateFileW_Hook(LPCWSTR fileName, DWORD access, DWORD shareMode,
                               LPSECURITY_ATTRIBUTES security, DWORD creation,
                               DWORD attributes, HANDLE templateFile) {
    HANDLE result = CreateFileW_Original(fileName, access, shareMode, security,
                                         creation, attributes, templateFile);
    if (result != INVALID_HANDLE_VALUE && creation == OPEN_EXISTING)
        RememberVideo(fileName);
    return result;
}

class PlayerCallback final : public IMFPMediaPlayerCallback {
    std::atomic<ULONG> references_{1};

  public:
    STDMETHODIMP QueryInterface(REFIID iid, void** object) override {
        if (!object) return E_POINTER;
        if (iid == IID_IUnknown || iid == __uuidof(IMFPMediaPlayerCallback)) {
            *object = static_cast<IMFPMediaPlayerCallback*>(this);
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++references_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG count = --references_;
        if (!count) delete this;
        return count;
    }
    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER* event) override {
        if (!event) return;
        if (event->eEventType == MFP_EVENT_TYPE_MEDIAITEM_CREATED &&
            SUCCEEDED(event->hrEvent) && g_player) {
            auto created = reinterpret_cast<MFP_MEDIAITEM_CREATED_EVENT*>(event);
            g_player->SetMediaItem(created->pMediaItem);
        }
        if (event->eEventType == MFP_EVENT_TYPE_MEDIAITEM_SET &&
            SUCCEEDED(event->hrEvent) && g_player) {
            g_player->SetMute(!g_settings.sound);
            if (g_settings.autoplay) g_player->Play();
        }
        if (g_playerWindow) PostMessageW(g_playerWindow, WM_APP + 1, 0, 0);
    }
};

PlayerCallback* g_callback;

void TogglePlayback() {
    if (!g_player) return;
    MFP_MEDIAPLAYER_STATE state;
    if (SUCCEEDED(g_player->GetState(&state)) && state == MFP_MEDIAPLAYER_STATE_PLAYING)
        g_player->Pause();
    else
        g_player->Play();
    InvalidateRect(g_playerWindow, nullptr, FALSE);
}

void ToggleSound() {
    if (!g_player) return;
    BOOL muted = TRUE;
    g_player->GetMute(&muted);
    g_player->SetMute(!muted);
    InvalidateRect(g_playerWindow, nullptr, FALSE);
}

void SeekFromPoint(int x) {
    if (!g_player || !g_playerWindow) return;
    RECT rect;
    GetClientRect(g_playerWindow, &rect);
    int left = kPlayButtonWidth + 10;
    int right = rect.right - kSoundButtonWidth - 10;
    if (right <= left) return;
    double fraction = static_cast<double>(max(left, min(x, right)) - left) /
                      static_cast<double>(right - left);
    PROPVARIANT duration;
    PropVariantInit(&duration);
    if (SUCCEEDED(g_player->GetDuration(MFP_POSITIONTYPE_100NS, &duration)) &&
        duration.vt == VT_I8) {
        PROPVARIANT position;
        PropVariantInit(&position);
        position.vt = VT_I8;
        position.hVal.QuadPart =
            static_cast<LONGLONG>(duration.hVal.QuadPart * fraction);
        g_player->SetPosition(MFP_POSITIONTYPE_100NS, &position);
    }
    PropVariantClear(&duration);
}

void PaintPlayer(HWND window) {
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(window, &paint);
    RECT rect;
    GetClientRect(window, &rect);
    RECT controls{0, max(0L, rect.bottom - kControlsHeight), rect.right, rect.bottom};
    HBRUSH background = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(dc, &controls, background);
    DeleteObject(background);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(245, 245, 245));

    MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;
    BOOL muted = TRUE;
    if (g_player) {
        g_player->GetState(&state);
        g_player->GetMute(&muted);
    }
    RECT play{0, controls.top, kPlayButtonWidth, controls.bottom};
    DrawTextW(dc, state == MFP_MEDIAPLAYER_STATE_PLAYING ? L"Pause" : L"Play",
              -1, &play, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT sound{rect.right - kSoundButtonWidth, controls.top, rect.right, controls.bottom};
    DrawTextW(dc, muted ? L"Muted" : L"Sound", -1, &sound,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    int left = kPlayButtonWidth + 10;
    int right = rect.right - kSoundButtonWidth - 10;
    int y = controls.top + kControlsHeight / 2;
    HPEN track = CreatePen(PS_SOLID, 3, RGB(100, 100, 100));
    HPEN oldPen = static_cast<HPEN>(SelectObject(dc, track));
    MoveToEx(dc, left, y, nullptr);
    LineTo(dc, right, y);
    SelectObject(dc, oldPen);
    DeleteObject(track);
    if (g_player) {
        PROPVARIANT position, duration;
        PropVariantInit(&position);
        PropVariantInit(&duration);
        if (SUCCEEDED(g_player->GetPosition(MFP_POSITIONTYPE_100NS, &position)) &&
            SUCCEEDED(g_player->GetDuration(MFP_POSITIONTYPE_100NS, &duration)) &&
            position.vt == VT_I8 && duration.vt == VT_I8 &&
            duration.hVal.QuadPart > 0) {
            int progress = left + static_cast<int>((right - left) *
                static_cast<double>(position.hVal.QuadPart) /
                duration.hVal.QuadPart);
            HPEN played = CreatePen(PS_SOLID, 4, RGB(0, 120, 215));
            oldPen = static_cast<HPEN>(SelectObject(dc, played));
            MoveToEx(dc, left, y, nullptr);
            LineTo(dc, progress, y);
            SelectObject(dc, oldPen);
            DeleteObject(played);
        }
        PropVariantClear(&position);
        PropVariantClear(&duration);
    }
    EndPaint(window, &paint);
}

LRESULT CALLBACK PlayerWindowProc(HWND window, UINT message, WPARAM wParam,
                                  LPARAM lParam) {
    switch (message) {
        case WM_LBUTTONDOWN: {
            RECT rect;
            GetClientRect(window, &rect);
            int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
            if (y >= rect.bottom - kControlsHeight) {
                if (x < kPlayButtonWidth) TogglePlayback();
                else if (x >= rect.right - kSoundButtonWidth) ToggleSound();
                else {
                    SeekFromPoint(x);
                    SetCapture(window);
                }
            } else {
                TogglePlayback();
            }
            return 0;
        }
        case WM_MOUSEMOVE:
            if (GetCapture() == window)
                SeekFromPoint(GET_X_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
            if (GetCapture() == window) ReleaseCapture();
            return 0;
        case WM_SIZE:
            if (g_videoWindow) {
                RECT rect;
                GetClientRect(window, &rect);
                SetWindowPos(g_videoWindow, nullptr, 0, 0, rect.right,
                             max(0L, rect.bottom - kControlsHeight),
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (g_player) g_player->UpdateVideo();
            return 0;
        case WM_TIMER:
        case WM_APP + 1:
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            PaintPlayer(window);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

LRESULT CALLBACK VideoWindowProc(HWND window, UINT message, WPARAM wParam,
                                 LPARAM lParam) {
    if (message == WM_LBUTTONDOWN) {
        TogglePlayback();
        return 0;
    }
    if (message == WM_ERASEBKGND) {
        RECT rect;
        GetClientRect(window, &rect);
        FillRect(reinterpret_cast<HDC>(wParam), &rect,
                 static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        return 1;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

BOOL CALLBACK FindHostWindow(HWND window, LPARAM parameter) {
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (processId != GetCurrentProcessId() || !IsWindowVisible(window) ||
        GetWindow(window, GW_OWNER)) return TRUE;
    RECT rect;
    GetClientRect(window, &rect);
    if (rect.right > 100 && rect.bottom > 100) {
        *reinterpret_cast<HWND*>(parameter) = window;
        return FALSE;
    }
    return TRUE;
}

void FitPlayerWindow() {
    if (!g_hostWindow || !g_playerWindow) return;
    RECT rect;
    GetClientRect(g_hostWindow, &rect);
    SetWindowPos(g_playerWindow, HWND_TOP, 0, 0, rect.right, rect.bottom,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

bool EnsurePlayer(const std::wstring& path) {
    if (!g_hostWindow || !IsWindow(g_hostWindow)) {
        g_hostWindow = nullptr;
        EnumWindows(FindHostWindow, reinterpret_cast<LPARAM>(&g_hostWindow));
    }
    if (!g_hostWindow) return false;
    if (!g_playerWindow) {
        g_playerWindow = CreateWindowExW(0, kWindowClass, L"", WS_CHILD | WS_VISIBLE |
                                         WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                         0, 0, 0, 0, g_hostWindow, nullptr,
                                         GetModuleHandleW(nullptr), nullptr);
        if (!g_playerWindow) return false;
        g_videoWindow = CreateWindowExW(0, kVideoClass, L"",
                                        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                                        g_playerWindow, nullptr,
                                        GetModuleHandleW(nullptr), nullptr);
        if (!g_videoWindow) return false;
        SetTimer(g_playerWindow, kUiTimer, kRefreshMs, nullptr);
        g_callback = new PlayerCallback();
        if (FAILED(MFPCreateMediaPlayer(nullptr, FALSE, 0, g_callback,
                                        g_videoWindow, &g_player))) return false;
    }
    FitPlayerWindow();
    g_player->Stop();
    g_player->SetMute(!g_settings.sound);
    return SUCCEEDED(g_player->CreateMediaItemFromURL(path.c_str(), FALSE, 0, nullptr));
}

DWORD WINAPI WorkerThread(void*) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(MFStartup(MF_VERSION))) {
        CoUninitialize();
        return 1;
    }
    while (!g_stopping) {
        std::wstring path;
        {
            std::lock_guard<std::mutex> lock(g_pathMutex);
            path = g_pendingPath;
        }
        if (!path.empty() && path != g_activePath && EnsurePlayer(path))
            g_activePath = path;
        if (g_playerWindow) FitPlayerWindow();
        MsgWaitForMultipleObjects(0, nullptr, FALSE, 100, QS_ALLINPUT);
        MSG message;
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    if (g_player) {
        g_player->Shutdown();
        g_player->Release();
        g_player = nullptr;
    }
    if (g_callback) {
        g_callback->Release();
        g_callback = nullptr;
    }
    if (g_playerWindow) DestroyWindow(g_playerWindow);
    MFShutdown();
    CoUninitialize();
    return 0;
}

void LoadSettings() {
    g_settings.autoplay = Wh_GetIntSetting(L"autoplay") != 0;
    g_settings.sound = Wh_GetIntSetting(L"sound") != 0;
}

}  // namespace

BOOL Wh_ModInit() {
    LoadSettings();
    WNDCLASSEXW windowClass{sizeof(windowClass)};
    windowClass.lpfnWndProc = PlayerWindowProc;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return FALSE;
    windowClass.lpfnWndProc = VideoWindowProc;
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    windowClass.lpszClassName = kVideoClass;
    if (!RegisterClassExW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return FALSE;

    HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
    auto createFile = reinterpret_cast<CreateFileW_t>(
        GetProcAddress(kernelBase, "CreateFileW"));
    if (!createFile || !Wh_SetFunctionHook(reinterpret_cast<void*>(createFile),
                                            reinterpret_cast<void*>(CreateFileW_Hook),
                                            reinterpret_cast<void**>(&CreateFileW_Original)))
        return FALSE;
    g_stopping = false;
    g_workerThread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    return g_workerThread != nullptr;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    if (g_player) g_player->SetMute(!g_settings.sound);
}

void Wh_ModUninit() {
    g_stopping = true;
    if (g_workerThread) {
        WaitForSingleObject(g_workerThread, 3000);
        CloseHandle(g_workerThread);
        g_workerThread = nullptr;
    }
    UnregisterClassW(kWindowClass, GetModuleHandleW(nullptr));
    UnregisterClassW(kVideoClass, GetModuleHandleW(nullptr));
}
