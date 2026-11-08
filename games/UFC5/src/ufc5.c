/*
 * Cage Launcher - UFC 5 port
 * Starts EA SPORTS UFC 5 (PS5, PPSA03541) through the bundled KytyPS5 (UFC 5 build) emulator with
 * performance-oriented settings. All of the emulator's command-line options are exposed here.
 * Win32, no dependencies beyond Windows itself. MIT license, like Cage Launcher (Copyright (c) 2026 Guilherme Mendes).
 */
#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <powrprof.h>
#include <dwmapi.h>
#include <commdlg.h>
#include <cpuid.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <stdint.h>
#include <ntsecapi.h>

#define APP_TITLE L"CAGE LAUNCHER — EA SPORTS UFC 5"
#define TITLE_ID L"PPSA03541"
#define WM_APP_EXITED (WM_APP + 1)
#define ENGINE_EXE L"UFC5-Game.exe"
#define TIMER_FPS 1
#define HOTKEY_OVERLAY 1

/* ---------------------------------------------------------------- GDI+ flat API (subset) */
typedef struct { UINT32 GdiplusVersion; void *DebugEventCallback; BOOL SuppressBackgroundThread; BOOL SuppressExternalCodecs; } GdiplusStartupInput;
typedef void GpImage; typedef void GpGraphics; typedef void GpBrush;
int WINAPI GdiplusStartup(ULONG_PTR *, const GdiplusStartupInput *, void *);
int WINAPI GdipLoadImageFromFile(const WCHAR *, GpImage **);
int WINAPI GdipGetImageWidth(GpImage *, UINT *);
int WINAPI GdipGetImageHeight(GpImage *, UINT *);
int WINAPI GdipCreateFromHDC(HDC, GpGraphics **);
int WINAPI GdipSetInterpolationMode(GpGraphics *, int);
int WINAPI GdipSetSmoothingMode(GpGraphics *, int);
int WINAPI GdipDrawImageRectRectI(GpGraphics *, GpImage *, INT, INT, INT, INT, INT, INT, INT, INT, int, void *, void *, void *);
int WINAPI GdipDrawImageRectI(GpGraphics *, GpImage *, INT, INT, INT, INT);
int WINAPI GdipCreateSolidFill(UINT32, GpBrush **);
int WINAPI GdipCreateLineBrushI(const POINT *, const POINT *, UINT32, UINT32, int, GpBrush **);
int WINAPI GdipFillRectangleI(GpGraphics *, GpBrush *, INT, INT, INT, INT);
int WINAPI GdipDeleteBrush(GpBrush *);
int WINAPI GdipDeleteGraphics(GpGraphics *);
int WINAPI GdipDisposeImage(GpImage *);
int WINAPI GdipSetPixelOffsetMode(GpGraphics *, int);

/* ---------------------------------------------------------------- colours: preto, branco e vermelho UFC (a paleta do Cage Launcher) */
#define C_BG      RGB(10, 10, 12)     /* fundo */
#define C_EDIT    RGB(17, 17, 19)     /* superfície: campos e dropdowns */
#define C_LINE    RGB(38, 38, 43)     /* borda de 1px e divisores */
#define C_LINEHOT RGB(58, 58, 66)     /* borda em hover */
#define C_ROWSEL  RGB(28, 28, 32)     /* botão pressionado */
#define C_OFF     RGB(40, 40, 46)     /* trilho do switch desligado, JOGAR desabilitado */
#define C_TEXT    RGB(242, 242, 242)
#define C_SUB     RGB(138, 138, 147)  /* texto secundário */
#define C_ACCENT  RGB(210, 10, 10)    /* vermelho UFC */
#define C_ACCHOT  RGB(229, 37, 42)    /* acento em hover */
#define C_ACCENT2 RGB(168, 8, 8)      /* acento pressionado */
#define C_GOOD    RGB(76, 195, 138)   /* bom */
#define C_WARN    RGB(240, 160, 32)   /* aviso (âmbar) */
#define C_BAD     RGB(255, 107, 94)   /* erro: coral, diferente do acento */

/* ---------------------------------------------------------------- control ids */
enum {
    ID_GAME = 100, ID_GAME_BR, ID_EMU, ID_EMU_BR,
    ID_PROFILE, ID_RES, ID_PRESENT, ID_VBLANK, ID_SHADER, ID_GPU, ID_LANG, ID_USER,
    ID_FULL, ID_AMD, ID_PRIO, ID_DGPU, ID_POWER, ID_CURSOR, ID_TROPHY, ID_BINDLESS, ID_TESS, ID_PLAYGO, ID_CONSOLE,
    ID_VOL, ID_VIB, ID_X3D, ID_READBACK, ID_PATCH, ID_PATCH_BR, ID_EXTRA, ID_ENV,
    ID_PLAY, ID_AUTO, ID_KYTY, ID_LOGS, ID_CLEAR, ID_STATUS, ID_OVERLAY, ID_ECO, ID_FASTSH, ID_LPAGES, ID_STAGEPREP, ID_CALIB,
    ID_CFPU, ID_DIAG, ID_FRAMEGEN, ID_DIAGFULL
};

/* ---------------------------------------------------------------- settings */
typedef struct {
    wchar_t game[MAX_PATH], emu[MAX_PATH], patch[MAX_PATH], user[64], extra[1024], env[2048];
    int profile, res, present, vblank, shader, gpu, lang, vol, vib;
    int full, amd, prio, dgpu, power, cursor, trophy, bindless, tess, playgo, console, x3d, readback, overlay, eco, fastsh, lpages, pfwarned, shthreads, stageprep, calib, calibArm, calibStage, place;
    int cfpu, diag, expert, pgap, lowsh, midprio;   /* expert: lets the options that broke the fight (AMD patches, tessellation, bindless) through */   /* console floating-point mode for the game's threads; performance diagnostics in the log */
    int framegen, diagFull;   /* frame generation (0 off, 1 Lossless Scaling opened and switched on, 2 only opened); full engine diagnostic for the next session */
    int configured;
} Settings;

static Settings S;
static BOOL g_headless; static DWORD g_exitCode;   /* --headless: started by Cage Launcher, no window of its own, no overlay */
static HINSTANCE g_inst;
static HWND g_wnd, g_tip, g_status;
static HFONT g_font, g_fontBold, g_fontTitle, g_fontPlay, g_fontLabel, g_fontBtn;
static HBRUSH g_brBg, g_brEdit;
static int g_dpi = 96;
static wchar_t g_exeDir[MAX_PATH], g_ini[MAX_PATH], g_lastLog[MAX_PATH];
static HBITMAP g_header; static int g_headerW, g_headerH;
static ULONG_PTR g_gdip;
static HANDLE g_proc;
static BOOL g_applying;
static GUID g_prevScheme; static BOOL g_schemeChanged;
static COLORREF g_statusColor = C_SUB;
static wchar_t g_gameVer[32] = L"";
static DWORD g_pid; static HWND g_gameWnd; static DWORD g_startTick;
static float g_fps[36000]; static int g_nfps;   /* one sample per second, up to 10 h */

#define SC(x) MulDiv((x), g_dpi, 96)
static BOOL FpsStats(float *avg, float *low, float *best);
static BOOL GameplayFps(float *fps, int *seconds); static BOOL PerfSummary(const wchar_t *path, wchar_t *out, int cap);
static void CheckCommit(void); static void SaveSettings(void); static void PutStr(const wchar_t *k, const wchar_t *v); static void PutInt(const wchar_t *k, int v); static int IniInt(const wchar_t *k, int def);
static float g_ovFps; static int g_tick; static BOOL g_ovHidden; static HWND g_ov; static int g_runStagePrep, g_runPlace, g_runCfpu, g_runPgap, g_runLowsh, g_runMidprio;
static int g_runDiagFull; static wchar_t g_motorLog[MAX_PATH];   /* this session writes the engine's own log (GPU breakdown) to g_motorLog */
static BOOL g_lsSent, g_starting; static int g_lsTries, g_lsState;   /* Lossless Scaling: hotkey sent this session; 1 = running, 2 = could not be opened */

/* ---------------------------------------------------------------- tables */
static const struct { int w, h; const wchar_t *name; } RES[] = {
    {0, 0, L"Nativa do monitor"}, {1280, 720, L"1280 x 720"}, {1600, 900, L"1600 x 900"},
    {1920, 1080, L"1920 x 1080 (Full HD)"}, {2560, 1440, L"2560 x 1440 (2K)"}, {3840, 2160, L"3840 x 2160 (4K)"},
};
static const wchar_t *PRESENT_ARG[] = {L"Immediate", L"Mailbox", L"Fifo"};
static const wchar_t *PRESENT_NAME[] = {L"Immediate (sem VSync, m\u00e1x. FPS)", L"Mailbox (sem tearing)", L"Fifo (VSync, mais est\u00e1vel)"};
/* the virtual vblank: the engine flips the game's frames on these ticks, so at 120/144 Hz a frame that misses one
   waits 8.3/6.9 ms for the next instead of 16.7 ms, and menus can pass 60 FPS. At 120 Hz and up the game is told it
   is on a 119.88 Hz screen. */
static const int VBLANK[] = {60, 120, 144};
static const wchar_t *VBLANK_NAME[] = {L"60 Hz (padrão do PS5)", L"120 Hz (até 120 FPS, experimental)", L"144 Hz (até 144 FPS, experimental)"};
#define NVBLANK 3
static const wchar_t *FRAMEGEN_NAME[] = {L"Desligada", L"Lossless Scaling: abrir e ligar sozinho (Ctrl+Alt+S)", L"Lossless Scaling: só abrir (o perfil do LS liga)"};
static const wchar_t *SHADER_ARG[] = {L"Performance", L"Size", L"None"};
static const wchar_t *SHADER_NAME[] = {L"Performance (mais FPS)", L"Size", L"None (compila mais rápido)"};
static const wchar_t *PROFILE_NAME[] = {L"Máximo FPS", L"Equilibrado", L"Qualidade / estabilidade", L"PC fraco (leve)", L"Personalizado"};
static const struct { int code; const wchar_t *name; } LANGS[] = {
    {17, L"Português (Brasil)"}, {7, L"Português (Portugal)"}, {1, L"English (US)"}, {18, L"English (UK)"},
    {3, L"Español"}, {20, L"Español (Latinoamérica)"}, {2, L"Français"}, {4, L"Deutsch"}, {5, L"Italiano"},
    {6, L"Nederlands"}, {16, L"Polski"}, {8, L"Русский"}, {19, L"Türkçe"},
    {0, L"日本語"}, {9, L"한국어"}, {11, L"简体中文"}, {10, L"繁體中文"},
};
#define NLANGS (int)(sizeof(LANGS) / sizeof(LANGS[0]))
#define NRES (int)(sizeof(RES) / sizeof(RES[0]))

/* GPUs found through Vulkan (index = the emulator's --gpu index) */
typedef struct { wchar_t name[256]; uint32_t vendor, type; uint64_t vram, bar; } GpuInfo;   /* bar: the CPU-visible video memory (Resizable BAR on: all of it) */
static GpuInfo g_gpus[8]; static int g_ngpus;

/* ---------------------------------------------------------------- small helpers */
static BOOL FileExists(const wchar_t *p) { DWORD a = GetFileAttributesW(p); return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY); }
static BOOL DirExists(const wchar_t *p) { DWORD a = GetFileAttributesW(p); return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY); }
static void Join(wchar_t *out, const wchar_t *a, const wchar_t *b) { _snwprintf(out, MAX_PATH, L"%s\\%s", a, b); out[MAX_PATH - 1] = 0; }
static void StripSlash(wchar_t *p) { size_t n = wcslen(p); while (n > 3 && (p[n - 1] == L'\\' || p[n - 1] == L'/')) p[--n] = 0; }
static void DirOf(wchar_t *out, const wchar_t *file) { wcscpy(out, file); wchar_t *s = wcsrchr(out, L'\\'); if (s) *s = 0; }

static void SetStatus(COLORREF c, const wchar_t *fmt, ...)
{
    wchar_t buf[1024]; va_list ap; va_start(ap, fmt); _vsnwprintf(buf, 1024, fmt, ap); va_end(ap); buf[1023] = 0;
    g_statusColor = c; SetWindowTextW(g_status, buf); InvalidateRect(g_status, NULL, TRUE);
}

static BOOL GameDirValid(const wchar_t *d)
{
    wchar_t p[MAX_PATH];
    Join(p, d, L"eboot.bin"); if (!FileExists(p)) return FALSE;
    Join(p, d, L"sce_sys\\param.json"); return FileExists(p);
}

/* reads the title id and version from sce_sys\param.json without a JSON parser */
static BOOL ReadParam(const wchar_t *d, wchar_t *tid, wchar_t *ver)
{
    wchar_t p[MAX_PATH]; Join(p, d, L"sce_sys\\param.json");
    HANDLE f = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return FALSE;
    char buf[16384]; DWORD n = 0; ReadFile(f, buf, sizeof(buf) - 1, &n, NULL); CloseHandle(f); buf[n] = 0;
    tid[0] = ver[0] = 0;
    const char *keys[2] = {"\"titleId\"", "\"contentVersion\""}; wchar_t *outs[2] = {tid, ver};
    for (int k = 0; k < 2; k++) {
        char *s = strstr(buf, keys[k]); if (!s) continue;
        s = strchr(s + strlen(keys[k]), '"'); if (!s) continue; s++;
        char *e = strchr(s, '"'); if (!e || e - s > 30) continue;
        int i = 0; for (; s < e; s++) outs[k][i++] = (wchar_t)*s; outs[k][i] = 0;
    }
    return tid[0] != 0;
}

/* accepts the game folder itself, or a folder that contains it (UFC 5 -> UFC 5\[...]-PPSA03541-...) */
static BOOL ResolveGameDir(wchar_t *d)
{
    StripSlash(d);
    if (GameDirValid(d)) return TRUE;
    wchar_t pat[MAX_PATH], cand[MAX_PATH], best[MAX_PATH] = L""; int bestScore = -1;
    Join(pat, d, L"*");
    WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return FALSE;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
        Join(cand, d, fd.cFileName);
        if (!GameDirValid(cand)) continue;
        int score = 0; wchar_t data[MAX_PATH]; Join(data, cand, L"Data");
        if (DirExists(data)) score += 2;               /* the complete dump has the Data folder */
        if (StrStrIW(fd.cFileName, L"PPSA03541")) score += 1;
        if (score > bestScore) { bestScore = score; wcscpy(best, cand); }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    if (bestScore < 0) return FALSE;
    wcscpy(d, best); return TRUE;
}

static BOOL DesktopDir(wchar_t *out)
{
    PWSTR p = NULL;
    if (SHGetKnownFolderPath(&FOLDERID_Desktop, 0, NULL, &p) != S_OK) return FALSE;
    wcsncpy(out, p, MAX_PATH); out[MAX_PATH - 1] = 0; CoTaskMemFree(p); return TRUE;
}

static void FindDefaults(void)
{
    wchar_t desk[MAX_PATH] = L"", p[MAX_PATH], q[MAX_PATH];
    DesktopDir(desk);
    if (!S.emu[0] || !FileExists(S.emu)) {
        S.emu[0] = 0;
        Join(p, g_exeDir, L"bin\\" ENGINE_EXE);                  /* the engine bundled with the port */
        if (FileExists(p)) wcscpy(S.emu, p);
        if (!S.emu[0]) { Join(p, g_exeDir, L"emulator\\kyty_emulator.exe"); if (FileExists(p)) wcscpy(S.emu, p); }
        if (!S.emu[0] && desk[0]) { Join(p, desk, L"ufc 5.2\\kyty_emulator.exe"); if (FileExists(p)) wcscpy(S.emu, p); }
        if (!S.emu[0] && desk[0]) {           /* any Desktop folder with the UFC 5 build (it ships Play-UFC5.bat) */
            wchar_t pat[MAX_PATH]; Join(pat, desk, L"*"); WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                do { if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') {
                        wchar_t b[MAX_PATH]; Join(q, desk, fd.cFileName); Join(p, q, L"kyty_emulator.exe"); Join(b, q, L"Play-UFC5.bat");
                        if (FileExists(p) && FileExists(b)) { wcscpy(S.emu, p); break; } }
                } while (FindNextFileW(h, &fd));
                FindClose(h);
            }
        }
    }
    if (!S.game[0] || !GameDirValid(S.game)) {
        S.game[0] = 0;
        if (desk[0]) {
            Join(p, desk, L"UFC 5"); if (ResolveGameDir(p)) wcscpy(S.game, p);
            if (!S.game[0]) { Join(p, desk, L"UFC5"); if (ResolveGameDir(p)) wcscpy(S.game, p); }
            if (!S.game[0]) {
                wchar_t pat[MAX_PATH]; Join(pat, desk, L"*"); WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
                if (h != INVALID_HANDLE_VALUE) {
                    do { if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') {
                            Join(q, desk, fd.cFileName); wchar_t t[32], v[32];
                            if (ResolveGameDir(q) && ReadParam(q, t, v) && !wcscmp(t, TITLE_ID)) { wcscpy(S.game, q); break; } }
                    } while (FindNextFileW(h, &fd));
                    FindClose(h);
                }
            }
        }
    }
}

/* ---------------------------------------------------------------- hardware detection */
static BOOL CpuIsAmd(void)
{
    unsigned a, b, c, d; char v[13];
    __get_cpuid(0, &a, &b, &c, &d);
    memcpy(v, &b, 4); memcpy(v + 4, &d, 4); memcpy(v + 8, &c, 4); v[12] = 0;
    return !strcmp(v, "AuthenticAMD");
}

static void CpuName(wchar_t *out, int n)
{
    unsigned r[12]; char s[49];
    for (int i = 0; i < 3; i++) __get_cpuid(0x80000002 + i, &r[i * 4], &r[i * 4 + 1], &r[i * 4 + 2], &r[i * 4 + 3]);
    memcpy(s, r, 48); s[48] = 0;
    char *p = s; while (*p == ' ') p++;
    for (char *e = p + strlen(p); e > p && e[-1] == ' '; ) *--e = 0;
    MultiByteToWideChar(CP_UTF8, 0, p, -1, out, n);
}

static BOOL CpuIsX3d(void) { wchar_t n[64]; CpuName(n, 64); return StrStrIW(n, L"X3D") != NULL; }
static BOOL X3dOn(void) { return S.x3d != 2 && CpuIsX3d(); }   /* on a CPU without V-Cache the X3D placement only hurts */

/* Vulkan, loaded at run time; structures are read at their fixed offsets */
typedef int (WINAPI *PFN_vkVoid)(void);
typedef void *(WINAPI *PFN_GIPA)(void *, const char *);
typedef struct { int sType; const void *pNext; const char *app; uint32_t appVer; const char *eng; uint32_t engVer; uint32_t api; } VkAppInfo_;
typedef struct { int sType; const void *pNext; uint32_t flags; const VkAppInfo_ *app; uint32_t nl; const char *const *l; uint32_t ne; const char *const *e; } VkInstInfo_;

static void DetectGpus(void)
{
    g_ngpus = 0;
    HMODULE vk = LoadLibraryW(L"vulkan-1.dll"); if (!vk) return;
    PFN_GIPA gipa = (PFN_GIPA)(void *)GetProcAddress(vk, "vkGetInstanceProcAddr");
    if (!gipa) { FreeLibrary(vk); return; }
    int (WINAPI *create)(const VkInstInfo_ *, const void *, void **) = (int (WINAPI *)(const VkInstInfo_ *, const void *, void **))gipa(NULL, "vkCreateInstance");
    if (!create) { FreeLibrary(vk); return; }
    VkAppInfo_ ai = {0, NULL, "UFC5 port", 1, "none", 1, (1u << 22) | (1u << 12)};
    VkInstInfo_ ii = {1, NULL, 0, &ai, 0, NULL, 0, NULL};
    void *inst = NULL;
    if (create(&ii, NULL, &inst) != 0 || !inst) { FreeLibrary(vk); return; }
    int (WINAPI *enumDev)(void *, uint32_t *, void **) = (int (WINAPI *)(void *, uint32_t *, void **))gipa(inst, "vkEnumeratePhysicalDevices");
    void (WINAPI *props)(void *, void *) = (void (WINAPI *)(void *, void *))gipa(inst, "vkGetPhysicalDeviceProperties");
    void (WINAPI *mem)(void *, void *) = (void (WINAPI *)(void *, void *))gipa(inst, "vkGetPhysicalDeviceMemoryProperties");
    void (WINAPI *destroy)(void *, const void *) = (void (WINAPI *)(void *, const void *))gipa(inst, "vkDestroyInstance");
    if (enumDev && props && mem) {
        uint32_t n = 8; void *devs[8];
        if (enumDev(inst, &n, devs) >= 0) {
            static unsigned char buf[8192];
            for (uint32_t i = 0; i < n && i < 8; i++) {
                memset(buf, 0, sizeof(buf)); props(devs[i], buf);
                GpuInfo *g = &g_gpus[g_ngpus++];
                g->vendor = *(uint32_t *)(buf + 8); g->type = *(uint32_t *)(buf + 16);
                MultiByteToWideChar(CP_UTF8, 0, (char *)buf + 20, -1, g->name, 256);
                memset(buf, 0, sizeof(buf)); mem(devs[i], buf);
                uint32_t nh = *(uint32_t *)(buf + 260), nt = *(uint32_t *)buf; g->vram = 0; g->bar = 0;
                for (uint32_t h = 0; h < nh && h < 16; h++) {
                    uint64_t size = *(uint64_t *)(buf + 264 + h * 16); uint32_t fl = *(uint32_t *)(buf + 264 + h * 16 + 8);
                    if ((fl & 1) && size > g->vram) g->vram = size;
                }
                for (uint32_t t = 0; t < nt && t < 32; t++) {   /* device-local and host-visible: its heap is the BAR window */
                    uint32_t fl = *(uint32_t *)(buf + 4 + t * 8), hi = *(uint32_t *)(buf + 8 + t * 8);
                    if ((fl & 3) == 3 && hi < 16) { uint64_t hs = *(uint64_t *)(buf + 264 + hi * 16); if (hs > g->bar) g->bar = hs; }
                }
            }
        }
    }
    if (destroy) destroy(inst, NULL);
    FreeLibrary(vk);
}

static int BestGpu(void);
/* Intel Arc (discrete Intel): the driver needs Resizable BAR, and it handles running out of video memory worse */
static BOOL ArcGpu(void) { int b = BestGpu(); return b >= 0 && g_gpus[b].vendor == 0x8086 && g_gpus[b].type == 2; }
static BOOL ArcNoRebar(void) { int b = BestGpu(); return ArcGpu() && g_gpus[b].bar <= (1ull << 30); }
static int BestGpu(void)
{
    int best = -1; uint64_t bv = 0;
    for (int i = 0; i < g_ngpus; i++) {
        uint64_t score = g_gpus[i].vram + (g_gpus[i].type == 2 ? (1ull << 40) : 0); /* discrete first */
        if (best < 0 || score > bv) { best = i; bv = score; }
    }
    return best;
}

/* ---------------------------------------------------------------- settings file */
static int IniInt(const wchar_t *k, int def) { return GetPrivateProfileIntW(L"UFC5", k, def, g_ini); }
static void IniStr(const wchar_t *k, wchar_t *out, int n, const wchar_t *def) { GetPrivateProfileStringW(L"UFC5", k, def, out, n, g_ini); }
static void PutInt(const wchar_t *k, int v) { wchar_t b[32]; _snwprintf(b, 32, L"%d", v); WritePrivateProfileStringW(L"UFC5", k, b, g_ini); }
static void PutStr(const wchar_t *k, const wchar_t *v) { WritePrivateProfileStringW(L"UFC5", k, v, g_ini); }

static void LoadSettings(void)
{
    IniStr(L"game", S.game, MAX_PATH, L""); IniStr(L"emulator", S.emu, MAX_PATH, L"");
    IniStr(L"patch", S.patch, MAX_PATH, L""); IniStr(L"user", S.user, 64, L"Lutador");
    IniStr(L"extra_args", S.extra, 1024, L""); IniStr(L"env", S.env, 2048, L"");
    S.configured = IniInt(L"configured", 0);
    S.profile = IniInt(L"profile", 0); S.res = IniInt(L"resolution", 0); S.present = IniInt(L"present", 0);
    S.vblank = IniInt(L"vblank", 0); S.shader = IniInt(L"shader", 0); S.gpu = IniInt(L"gpu", -1);
    S.lang = IniInt(L"language", 0); S.vol = IniInt(L"controller_volume", 50); S.vib = IniInt(L"controller_vibration", 100);
    S.full = IniInt(L"fullscreen", 1); S.amd = IniInt(L"amd_cpu", 0); S.prio = IniInt(L"high_priority", 1);
    S.dgpu = IniInt(L"dedicated_gpu", 1); S.power = IniInt(L"power_plan", 1); S.cursor = IniInt(L"hide_cursor", 1);
    S.trophy = IniInt(L"trophies", 0); S.bindless = IniInt(L"bindless", 0); S.tess = IniInt(L"tessellation", 0);
    S.playgo = IniInt(L"playgo_hack", 0); S.console = IniInt(L"show_console", 0);
    S.stageprep = IniInt(L"stage_prep_parallel", 0); S.calib = IniInt(L"calibrate", 0); S.calibArm = IniInt(L"calib_arm", 0); S.shthreads = IniInt(L"shader_threads", 0); S.fastsh = IniInt(L"fast_shaders", 1); S.lpages = IniInt(L"large_pages", 0); S.pfwarned = IniInt(L"pagefile_warned", 0); S.overlay = IniInt(L"fps_overlay", 1); S.eco = IniInt(L"no_power_throttling", 1); S.x3d = IniInt(L"x3d_mode", 0); S.readback = IniInt(L"readback_linear", 1);
    S.cfpu = IniInt(L"console_fpu", 1); S.diag = IniInt(L"perf_diag", 1); S.calibStage = IniInt(L"calib_stage", 0); S.place = IniInt(L"cpu_place", 0); S.expert = IniInt(L"expert_options", 0); S.pgap = IniInt(L"protect_gap_wide", 0); S.lowsh = IniInt(L"shader_threads_low", 0); S.midprio = IniInt(L"priority_above_normal", 0);
    S.framegen = IniInt(L"framegen", 0); S.diagFull = IniInt(L"diag_full_next", 0);
}

static void SaveSettings(void)
{
    PutStr(L"game", S.game); PutStr(L"emulator", S.emu); PutStr(L"patch", S.patch); PutStr(L"user", S.user);
    PutStr(L"extra_args", S.extra); PutStr(L"env", S.env); PutInt(L"configured", S.configured);
    PutInt(L"profile", S.profile); PutInt(L"resolution", S.res); PutInt(L"present", S.present); PutInt(L"vblank", S.vblank);
    PutInt(L"shader", S.shader); PutInt(L"gpu", S.gpu); PutInt(L"language", S.lang);
    PutInt(L"controller_volume", S.vol); PutInt(L"controller_vibration", S.vib);
    PutInt(L"fullscreen", S.full); PutInt(L"amd_cpu", S.amd); PutInt(L"high_priority", S.prio); PutInt(L"dedicated_gpu", S.dgpu);
    PutInt(L"power_plan", S.power); PutInt(L"hide_cursor", S.cursor); PutInt(L"trophies", S.trophy); PutInt(L"bindless", S.bindless);
    PutInt(L"tessellation", S.tess); PutInt(L"playgo_hack", S.playgo); PutInt(L"show_console", S.console);
    PutInt(L"stage_prep_parallel", S.stageprep); PutInt(L"calibrate", S.calib); PutInt(L"calib_arm", S.calibArm); PutInt(L"shader_threads", S.shthreads); PutInt(L"fast_shaders", S.fastsh); PutInt(L"large_pages", S.lpages); PutInt(L"pagefile_warned", S.pfwarned); PutInt(L"fps_overlay", S.overlay); PutInt(L"no_power_throttling", S.eco); PutInt(L"x3d_mode", S.x3d); PutInt(L"readback_linear", S.readback);
    PutInt(L"console_fpu", S.cfpu); PutInt(L"perf_diag", S.diag); PutInt(L"calib_stage", S.calibStage); PutInt(L"cpu_place", S.place); PutInt(L"protect_gap_wide", S.pgap); PutInt(L"shader_threads_low", S.lowsh); PutInt(L"priority_above_normal", S.midprio);
    PutInt(L"framegen", S.framegen); PutInt(L"diag_full_next", S.diagFull);
}

/* ---------------------------------------------------------------- settings <-> controls */
static HWND Ctl(int id) { return GetDlgItem(g_wnd, id); }
static int  CbGet(int id) { return (int)SendMessageW(Ctl(id), CB_GETCURSEL, 0, 0); }
static void CbSet(int id, int i) { SendMessageW(Ctl(id), CB_SETCURSEL, i, 0); }
static int  ChkGet(int id) { return SendMessageW(Ctl(id), BM_GETCHECK, 0, 0) == BST_CHECKED; }
static void ChkSet(int id, int v) { SendMessageW(Ctl(id), BM_SETCHECK, v ? BST_CHECKED : BST_UNCHECKED, 0); }

static void SettingsToUi(void)
{
    g_applying = TRUE;
    SetDlgItemTextW(g_wnd, ID_GAME, S.game); SetDlgItemTextW(g_wnd, ID_EMU, S.emu); SetDlgItemTextW(g_wnd, ID_PATCH, S.patch);
    SetDlgItemTextW(g_wnd, ID_USER, S.user); SetDlgItemTextW(g_wnd, ID_EXTRA, S.extra); SetDlgItemTextW(g_wnd, ID_ENV, S.env);
    CbSet(ID_PROFILE, S.profile); CbSet(ID_RES, S.res); CbSet(ID_PRESENT, S.present); CbSet(ID_VBLANK, S.vblank);
    CbSet(ID_SHADER, S.shader); CbSet(ID_GPU, S.gpu + 1); CbSet(ID_LANG, S.lang);
    CbSet(ID_VOL, S.vol / 10); CbSet(ID_VIB, S.vib / 10);
    ChkSet(ID_FULL, S.full); ChkSet(ID_AMD, S.amd); ChkSet(ID_PRIO, S.prio); ChkSet(ID_DGPU, S.dgpu); ChkSet(ID_POWER, S.power);
    ChkSet(ID_CURSOR, S.cursor); ChkSet(ID_TROPHY, S.trophy); ChkSet(ID_BINDLESS, S.bindless); ChkSet(ID_TESS, S.tess);
    ChkSet(ID_PLAYGO, S.playgo); ChkSet(ID_CONSOLE, S.console);
    CbSet(ID_X3D, S.x3d); CbSet(ID_OVERLAY, S.overlay); ChkSet(ID_ECO, S.eco); ChkSet(ID_FASTSH, S.fastsh); ChkSet(ID_LPAGES, S.lpages); ChkSet(ID_STAGEPREP, S.stageprep); ChkSet(ID_CALIB, S.calib); ChkSet(ID_READBACK, S.readback);
    ChkSet(ID_CFPU, S.cfpu); ChkSet(ID_DIAG, S.diag); CbSet(ID_FRAMEGEN, S.framegen); ChkSet(ID_DIAGFULL, S.diagFull);
    if (!S.expert) {   /* they left the fight black or recompiled every shader: locked (expert_options=1 in UFC5.ini unlocks) */
        ChkSet(ID_AMD, 0); ChkSet(ID_TESS, 0); ChkSet(ID_BINDLESS, 0);
        EnableWindow(Ctl(ID_AMD), FALSE); EnableWindow(Ctl(ID_TESS), FALSE); EnableWindow(Ctl(ID_BINDLESS), FALSE);
    }
    g_applying = FALSE;
}

static void UiToSettings(void)
{
    GetDlgItemTextW(g_wnd, ID_GAME, S.game, MAX_PATH); GetDlgItemTextW(g_wnd, ID_EMU, S.emu, MAX_PATH);
    GetDlgItemTextW(g_wnd, ID_PATCH, S.patch, MAX_PATH); GetDlgItemTextW(g_wnd, ID_USER, S.user, 64);
    GetDlgItemTextW(g_wnd, ID_EXTRA, S.extra, 1024); GetDlgItemTextW(g_wnd, ID_ENV, S.env, 2048);
    StripSlash(S.game);
    S.profile = CbGet(ID_PROFILE); S.res = CbGet(ID_RES); S.present = CbGet(ID_PRESENT); S.vblank = CbGet(ID_VBLANK);
    S.shader = CbGet(ID_SHADER); S.gpu = CbGet(ID_GPU) - 1; S.lang = CbGet(ID_LANG);
    S.vol = CbGet(ID_VOL) * 10; S.vib = CbGet(ID_VIB) * 10;
    S.full = ChkGet(ID_FULL); S.amd = ChkGet(ID_AMD); S.prio = ChkGet(ID_PRIO); S.dgpu = ChkGet(ID_DGPU); S.power = ChkGet(ID_POWER);
    S.cursor = ChkGet(ID_CURSOR); S.trophy = ChkGet(ID_TROPHY); S.bindless = ChkGet(ID_BINDLESS); S.tess = ChkGet(ID_TESS);
    S.playgo = ChkGet(ID_PLAYGO); S.console = ChkGet(ID_CONSOLE);
    S.x3d = CbGet(ID_X3D); if (S.x3d < 0) S.x3d = 0; S.overlay = CbGet(ID_OVERLAY); if (S.overlay < 0) S.overlay = 0; S.eco = ChkGet(ID_ECO); S.fastsh = ChkGet(ID_FASTSH); S.lpages = ChkGet(ID_LPAGES); S.stageprep = ChkGet(ID_STAGEPREP); S.calib = ChkGet(ID_CALIB); S.readback = ChkGet(ID_READBACK);
    S.cfpu = ChkGet(ID_CFPU); S.diag = ChkGet(ID_DIAG);
    S.framegen = CbGet(ID_FRAMEGEN); if (S.framegen < 0) S.framegen = 0; S.diagFull = ChkGet(ID_DIAGFULL);
}

/* the profiles only touch performance settings; paths, name, language and the like stay */
static void ApplyProfile(int p)
{
    S.profile = p;
    switch (p) {
    case 0: /* Maximum FPS */
        S.present = 0; S.vblank = 0; S.shader = 0; S.full = 1; S.prio = 1; S.dgpu = 1; S.power = 1;
        S.cursor = 1; S.trophy = 0; S.bindless = 0; S.tess = 0; break;
    case 1: /* Balanced */
        S.present = 1; S.vblank = 0; S.shader = 0; S.full = 1; S.prio = 1; S.dgpu = 1; S.power = 1;
        S.cursor = 1; S.trophy = 1; S.bindless = 0; S.tess = 0; break;
    case 3: /* weak PC: everything that costs CPU/GPU off, 1080p window, shader stalls cut, VRAM budget */
        S.present = 0; S.vblank = 0; S.shader = 0; S.full = 1; S.prio = 1; S.dgpu = 1; S.power = 1;
        S.cursor = 1; S.trophy = 0; S.bindless = 0; S.tess = 0; S.fastsh = 1; S.eco = 1; S.res = 3; break;
    case 2: /* Quality / stability */
        S.present = 2; S.vblank = 0; S.shader = 0; S.full = 1; S.prio = 0; S.dgpu = 1; S.power = 0;
        S.cursor = 1; S.trophy = 1; S.bindless = 0; S.tess = 0; break;
    default: break;
    }
}

/* the PC's tier for the UFC 5 engine, which is limited by the processor first, then by video memory and RAM:
   2 = strong (12+ threads, 7.5+ GB VRAM, 16 GB RAM), 1 = weak, 0 = very weak (4 threads, < 4 GB VRAM or < 12 GB RAM) */
static const wchar_t *const TIER_NAME[3] = {L"PC muito fraco", L"PC fraco", L"PC forte"};
static int g_tier = 2;
static void AutoConfig(BOOL announce)
{
    S.amd = 0;   /* the UFC 5 build was tuned without the AMD instruction patches: with them the 3D world of a fight can render black */
    S.bindless = 0; S.tess = 0; S.vblank = 0; S.lpages = 0; S.x3d = 0; S.readback = 1;
    int bg = BestGpu();
    S.gpu = (g_ngpus > 1 && bg >= 0) ? bg : -1;   /* only pin the GPU when there is a choice (laptops, iGPU + dGPU) */
    MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms);
    SYSTEM_INFO si; GetSystemInfo(&si);
    uint64_t vram = bg >= 0 ? g_gpus[bg].vram : 0, vmb = vram >> 20;
    int threads = (int)si.dwNumberOfProcessors;
    uint64_t ramGb = (ms.ullTotalPhys + (512ull << 20)) >> 30;
    DEVMODEW dm = {.dmSize = sizeof(dm)}; EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm);
    int monH = dm.dmPelsHeight ? (int)dm.dmPelsHeight : 1080;
    g_tier = (threads <= 4 || (vmb && vmb < 3800) || ramGb < 12) ? 0
           : (threads < 12 || (vmb && vmb < 7500) || ramGb < 16) ? 1 : 2;
    S.fastsh = 1; S.eco = 1; S.diag = 1; S.shthreads = 0;
    if (g_tier == 2) {
        ApplyProfile(0);
        S.res = (monH > 1440 && vmb < 11500) ? 3 : 0;   /* a 4K monitor on an 8 GB card: 1080p window, scaled */
    } else {
        ApplyProfile(3);                                 /* 1080p, VRAM budget, every costly extra off */
        if (g_tier == 0) { S.res = monH > 720 ? 1 : 0; S.shthreads = 2; }   /* 720p, shader translation leaves cores for the game */
    }
    S.configured = 1;
    if (announce) {
        wchar_t cpu[64]; CpuName(cpu, 64);
        SetStatus(C_GOOD, L"Configurado para o seu PC (%s): %s (%llu GB), %s, %d threads, %llu GB de RAM — perfil %s, %s.",
                  TIER_NAME[g_tier], bg >= 0 ? g_gpus[bg].name : L"GPU não detectada", (unsigned long long)((vram + (512ull << 20)) >> 30), cpu, threads,
                  (unsigned long long)ramGb, PROFILE_NAME[S.profile], RES[S.res].name);
        if (ArcNoRebar()) SetStatus(C_BAD, L"Intel Arc sem Resizable BAR: ligue \"Re-Size BAR\" e \"Above 4G Decoding\" na BIOS. Sem isso a placa perde muito FPS. O resto já foi configurado (modo Intel Arc).");
    }
}

/* hardware signature: when it changes (new video card, more RAM, other PC), Cage Launcher configures UFC 5 again */
static void HwSig(wchar_t *out, int n)
{
    wchar_t cpu[64]; CpuName(cpu, 64); int bg = BestGpu();
    MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms); SYSTEM_INFO si; GetSystemInfo(&si);
    _snwprintf(out, n, L"%s|%lu|%llu|%s|%llu", cpu, si.dwNumberOfProcessors, (unsigned long long)((ms.ullTotalPhys + (512ull << 20)) >> 30),
               bg >= 0 ? g_gpus[bg].name : L"-", bg >= 0 ? (unsigned long long)(g_gpus[bg].vram >> 20) : 0ull);
    out[n - 1] = 0;
}

/* --auto-config [--force]: called by Cage Launcher when it opens. No window. Exit code 0 = already set for this PC,
   1 = configured now (summary in auto_summary), 2 = the UFC 5 launcher is open (nothing touched). */
static BOOL SameSig(const wchar_t *a, const wchar_t *b)   /* spaces do not count (older versions kept the CPU name's padding) */
{
    for (;;) { while (*a == L' ') a++; while (*b == L' ') b++; if (*a != *b) return FALSE; if (!*a) return TRUE; a++; b++; }
}
static int HeadlessAutoConfig(BOOL force)
{
    HANDLE m = OpenMutexW(SYNCHRONIZE, FALSE, L"CAGE_LAUNCHER_UFC5");
    if (m) { CloseHandle(m); return 2; }
    LoadSettings(); DetectGpus(); FindDefaults();
    if (S.gpu >= g_ngpus) S.gpu = -1;
    wchar_t sig[400], old[400]; HwSig(sig, 400); IniStr(L"hw_sig", old, 400, L"");
    BOOL same = SameSig(sig, old);
    if (!force && S.configured && same) return 0;
    BOOL newHw = !S.configured || (old[0] && !same) || force;
    AutoConfig(FALSE);
    if (newHw) {   /* another PC: what the calibration learned no longer holds, measure again */
        S.calib = 1; S.calibStage = 0; S.calibArm = 0; S.cfpu = 1; S.stageprep = 0; S.place = 0; S.pgap = 0; S.lowsh = 0; S.midprio = 0;
        PutInt(L"calib_a_sum", 0); PutInt(L"calib_a_n", 0); PutInt(L"calib_b_sum", 0); PutInt(L"calib_b_n", 0);
        PutStr(L"calib_status", L"");
    }
    SaveSettings(); PutStr(L"hw_sig", sig);
    wchar_t cpu[64], sum[400]; CpuName(cpu, 64); int bg = BestGpu();
    MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms); SYSTEM_INFO si; GetSystemInfo(&si);
    _snwprintf(sum, 400, L"%s: %s + %s (%llu GB), %lu threads, %llu GB RAM. Perfil %s, %s%s%s%s.", TIER_NAME[g_tier], cpu,
               bg >= 0 ? g_gpus[bg].name : L"GPU?", bg >= 0 ? (unsigned long long)((g_gpus[bg].vram + (512ull << 20)) >> 30) : 0ull,
               si.dwNumberOfProcessors, (unsigned long long)((ms.ullTotalPhys + (512ull << 20)) >> 30), PROFILE_NAME[S.profile], RES[S.res].name,
               S.calib ? L", calibração automática ligada" : L"", ArcGpu() ? L", modo Intel Arc (limite de VRAM)" : L"",
               ArcNoRebar() ? L". ATENÇÃO: Resizable BAR desligado na BIOS" : L"");
    sum[399] = 0; PutStr(L"auto_summary", sum);
    return 1;
}

/* ---------------------------------------------------------------- header image */
static void FillC(HDC dc, RECT r, COLORREF c) { HBRUSH b = CreateSolidBrush(c); FillRect(dc, &r, b); DeleteObject(b); }
static void FrameC(HDC dc, RECT r, COLORREF c) { HBRUSH b = CreateSolidBrush(c); FrameRect(dc, &r, b); DeleteObject(b); }
static const wchar_t *Caps(wchar_t *out, int n, const wchar_t *s) { wcsncpy(out, s, n - 1); out[n - 1] = 0; CharUpperW(out); return out; }
static void BuildHeader(void)
{   /* faixa sólida: ícone do jogo, título, subtítulo e teclas; linha fina embaixo */
    RECT rc; GetClientRect(g_wnd, &rc);
    int w = rc.right, h = SC(112);
    HDC wdc = GetDC(g_wnd), dc = CreateCompatibleDC(wdc);
    if (g_header) DeleteObject(g_header);
    g_header = CreateCompatibleBitmap(wdc, w, h); g_headerW = w; g_headerH = h;
    HGDIOBJ old = SelectObject(dc, g_header);
    RECT r = {0, 0, w, h}; FillC(dc, r, C_BG);
    RECT ln = {0, h - 1, w, h}; FillC(dc, ln, C_LINE);

    GpGraphics *g = NULL; GdipCreateFromHDC(dc, &g);
    if (g) {
        GdipSetInterpolationMode(g, 7 /* HighQualityBicubic */); GdipSetSmoothingMode(g, 4); GdipSetPixelOffsetMode(g, 4);
        wchar_t p[MAX_PATH]; GpImage *img = NULL; BOOL icon = FALSE;
        if (S.game[0]) Join(p, S.game, L"sce_sys\\icon0.png");
        if (S.game[0] && FileExists(p) && GdipLoadImageFromFile(p, &img) == 0 && img) {
            GdipDrawImageRectI(g, img, SC(18), SC(14), SC(84), SC(84)); GdipDisposeImage(img); icon = TRUE;
        }
        GdipDeleteGraphics(g);
        if (!icon) { RECT ir = {SC(18), SC(14), SC(102), SC(98)}; FrameC(dc, ir, C_LINE); SelectObject(dc, g_fontBtn); SetTextColor(dc, C_SUB); SetBkMode(dc, TRANSPARENT);
                    DrawTextW(dc, L"UFC 5", -1, &ir, DT_CENTER | DT_VCENTER | DT_SINGLELINE); }   /* sem o ícone do jogo: moldura reta de 1px */
    }
    SetBkMode(dc, TRANSPARENT);
    int tx = SC(128); wchar_t t[32];
    SelectObject(dc, g_fontTitle); SetTextColor(dc, C_TEXT); SetTextCharacterExtra(dc, SC(1));
    TextOutW(dc, tx, SC(14), Caps(t, 32, L"EA SPORTS UFC 5"), 15); SetTextCharacterExtra(dc, 0);
    SelectObject(dc, g_font); SetTextColor(dc, C_SUB);
    wchar_t sub[256];
    _snwprintf(sub, 256, L"%s  ·  versão %s  ·  Port para PC  ·  CAGE LAUNCHER", TITLE_ID, g_gameVer[0] ? g_gameVer : L"?");
    TextOutW(dc, tx, SC(56), sub, (int)wcslen(sub));
    SetTextColor(dc, C_SUB);
    const wchar_t *keys = L"Teclado:  J/L/I/K = X/O/△/□  ·  WASD analógico  ·  setas d-pad  ·  Q/E L1/R1  ·  Z/C L2/R2  ·  Enter Options";
    TextOutW(dc, tx, SC(78), keys, (int)wcslen(keys));
    SelectObject(dc, old); DeleteDC(dc); ReleaseDC(g_wnd, wdc);
    RECT hr = {0, 0, w, h}; InvalidateRect(g_wnd, &hr, FALSE);
}
static void RefreshGameInfo(void)
{
    wchar_t t[32], v[32];
    g_gameVer[0] = 0;
    if (S.game[0] && ReadParam(S.game, t, v)) {
        /* 01.004.000 -> 1.004 */
        wcsncpy(g_gameVer, v, 31); g_gameVer[31] = 0;
        if (wcslen(v) >= 6 && v[0] == L'0') { wcsncpy(g_gameVer, v + 1, 5); g_gameVer[5] = 0; }
        if (wcscmp(t, TITLE_ID)) SetStatus(C_BAD, L"Atenção: essa pasta é do jogo %s, não do UFC 5 (%s).", t, TITLE_ID);
    }
    BuildHeader();
}

/* ---------------------------------------------------------------- UI construction */
static HWND Mk(const wchar_t *cls, const wchar_t *text, DWORD style, int x, int y, int w, int h, int id)
{
    HWND c = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, SC(x), SC(y), SC(w), SC(h), g_wnd, (HMENU)(INT_PTR)id, g_inst, NULL);
    SendMessageW(c, WM_SETFONT, (WPARAM)g_font, TRUE);
    return c;
}
static void Tip(int id, const wchar_t *text)
{
    TOOLINFOW ti = {sizeof(ti)}; ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS; ti.hwnd = g_wnd; ti.uId = (UINT_PTR)Ctl(id); ti.lpszText = (LPWSTR)text;
    SendMessageW(g_tip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
}
static HWND Label(const wchar_t *t, int x, int y, int w)
{
    HWND l = Mk(L"STATIC", t, SS_LEFT | SS_CENTERIMAGE, x, y, w, 24, -1); SendMessageW(l, WM_SETFONT, (WPARAM)g_fontLabel, TRUE); return l;
}
static RECT g_grp[6]; static const wchar_t *g_grpName[6]; static int g_ngrp;
static void Group(const wchar_t *t, int x, int y, int w, int h)
{   /* painted in WM_PAINT: a section title over the controls of the group */
    if (g_ngrp < 6) { SetRect(&g_grp[g_ngrp], SC(x), SC(y + 6), SC(x + w), SC(y + h)); g_grpName[g_ngrp++] = t; }
}
static void PaintGroups(HDC dc)
{   /* seção: traço vermelho curto, título em caixa alta e linha fina até a borda direita */
    HGDIOBJ of = SelectObject(dc, g_fontBold); SetBkMode(dc, TRANSPARENT); SetTextColor(dc, C_TEXT); SetTextCharacterExtra(dc, SC(2));
    for (int i = 0; i < g_ngrp; i++) {
        RECT r = g_grp[i]; wchar_t t[64]; Caps(t, 64, g_grpName[i]);
        SIZE sz; GetTextExtentPoint32W(dc, t, (int)wcslen(t), &sz);
        RECT dash = {r.left, r.top - SC(1), r.left + SC(14), r.top + SC(1)}; FillC(dc, dash, C_ACCENT);
        TextOutW(dc, r.left + SC(24), r.top - sz.cy / 2, t, (int)wcslen(t));
        RECT ln = {r.left + SC(24) + sz.cx + SC(10), r.top, r.right, r.top + 1}; FillC(dc, ln, C_LINE);
    }
    SetTextCharacterExtra(dc, 0); SelectObject(dc, of);
}

/* ---------------------------------------------------------------- controls drawn in the launcher style
   Só o desenho muda: os controles continuam COMBOBOX, EDIT e BUTTON comuns (mesmas mensagens e notificações).
   Cada subclass guarda o procedimento original em GWLP_USERDATA e repassa tudo para ele. */
static HWND g_hotCtl;   /* dropdown ou botão sob o mouse */
static LRESULT Base(HWND h, UINT m, WPARAM wp, LPARAM lp) { return CallWindowProcW((WNDPROC)GetWindowLongPtrW(h, GWLP_USERDATA), h, m, wp, lp); }
static void TrackHot(HWND h, UINT m)
{
    if (m == WM_MOUSEMOVE && g_hotCtl != h) {
        HWND old = g_hotCtl; g_hotCtl = h; if (old) InvalidateRect(old, NULL, FALSE); InvalidateRect(h, NULL, FALSE);
        TRACKMOUSEEVENT t = {sizeof(t), TME_LEAVE, h, 0}; TrackMouseEvent(&t);
    } else if (m == WM_MOUSELEAVE && g_hotCtl == h) { g_hotCtl = NULL; InvalidateRect(h, NULL, FALSE); }
    else if (m == WM_SETFOCUS || m == WM_KILLFOCUS) InvalidateRect(h, NULL, FALSE);
}
static void Subclass(HWND c, WNDPROC p) { SetWindowLongPtrW(c, GWLP_USERDATA, SetWindowLongPtrW(c, GWLP_WNDPROC, (LONG_PTR)p)); }
/* dropdown: superfície, borda de 1px, chevron fino; hover cinza, foco ou aberto em vermelho */
static LRESULT CALLBACK ComboProc(HWND h, UINT m, WPARAM wp, LPARAM lp)
{
    TrackHot(h, m);
    if (m == WM_ERASEBKGND) return 1;
    if (m != WM_PAINT) return Base(h, m, wp, lp);
    PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps); RECT r; GetClientRect(h, &r);
    BOOL open = SendMessageW(h, CB_GETDROPPEDSTATE, 0, 0) != 0, dis = !IsWindowEnabled(h);
    FillC(dc, r, C_EDIT); FrameC(dc, r, open || GetFocus() == h ? C_ACCENT : g_hotCtl == h ? C_LINEHOT : C_LINE);
    wchar_t t[320] = L""; int i = (int)SendMessageW(h, CB_GETCURSEL, 0, 0);
    if (i >= 0 && SendMessageW(h, CB_GETLBTEXTLEN, i, 0) < 320) SendMessageW(h, CB_GETLBTEXT, i, (LPARAM)t);
    HGDIOBJ of = SelectObject(dc, (HFONT)SendMessageW(h, WM_GETFONT, 0, 0)); SetBkMode(dc, TRANSPARENT); SetTextColor(dc, dis ? C_SUB : C_TEXT);
    RECT tr = {r.left + SC(8), r.top, r.right - SC(26), r.bottom}; DrawTextW(dc, t, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    HPEN pen = CreatePen(PS_SOLID, 1, g_hotCtl == h ? C_TEXT : C_SUB); HGDIOBJ op = SelectObject(dc, pen);
    int cx = r.right - SC(13), cy = (r.top + r.bottom) / 2, k = SC(4);
    MoveToEx(dc, cx - k, cy - k / 2, NULL); LineTo(dc, cx, cy + k / 2); LineTo(dc, cx + k + 1, cy - k / 2 - 1);
    SelectObject(dc, op); DeleteObject(pen); SelectObject(dc, of);
    EndPaint(h, &ps); return 0;
}
/* campo de texto: borda de 1px na cor da borda, vermelha com o foco */
static LRESULT CALLBACK EditProc(HWND h, UINT m, WPARAM wp, LPARAM lp)
{
    if (m == WM_NCPAINT) {
        HDC dc = GetWindowDC(h); RECT r; GetWindowRect(h, &r); OffsetRect(&r, -r.left, -r.top);
        FrameC(dc, r, GetFocus() == h ? C_ACCENT : C_LINE); ReleaseDC(h, dc); return 0;
    }
    LRESULT res = Base(h, m, wp, lp);
    if (m == WM_SETFOCUS || m == WM_KILLFOCUS) RedrawWindow(h, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
    return res;
}
/* botão (owner-draw): só acompanha o hover; o desenho fica em DrawButton */
static LRESULT CALLBACK BtnProc(HWND h, UINT m, WPARAM wp, LPARAM lp) { TrackHot(h, m); return Base(h, m, wp, lp); }

static HWND Combo(int id, int x, int y, int w)
{
    HWND c = Mk(L"COMBOBOX", L"", CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP, x, y, w, 300, id); SetWindowTheme(c, L"DarkMode_CFD", NULL); Subclass(c, ComboProc); return c;
}
static HWND Edit(int id, int x, int y, int w)
{
    HWND e = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | WS_BORDER, SC(x), SC(y), SC(w), SC(24), g_wnd, (HMENU)(INT_PTR)id, g_inst, NULL);
    SendMessageW(e, WM_SETFONT, (WPARAM)g_font, TRUE); SetWindowTheme(e, L"DarkMode_CFD", NULL);
    SendMessageW(e, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(SC(6), SC(6))); Subclass(e, EditProc); return e;
}
static HWND Btn(int id, const wchar_t *t, int x, int y, int w, int h) { HWND b = Mk(L"BUTTON", t, BS_OWNERDRAW | WS_TABSTOP, x, y, w, h, id); Subclass(b, BtnProc); return b; }
static HWND Check(int id, const wchar_t *t, int x, int y, int w)
{   /* desenhada como switch em DrawSwitch (NM_CUSTOMDRAW); continua um BS_AUTOCHECKBOX */
    return Mk(L"BUTTON", t, BS_AUTOCHECKBOX | WS_TABSTOP, x, y, w, 22, id);   /* com tema: só assim o controle manda NM_CUSTOMDRAW */
}
static void Add(int id, const wchar_t *s) { SendMessageW(Ctl(id), CB_ADDSTRING, 0, (LPARAM)s); }

static void BuildUi(void)
{
    g_tip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, 0, 0, 0, 0, g_wnd, NULL, g_inst, NULL);
    SendMessageW(g_tip, TTM_SETMAXTIPWIDTH, 0, SC(420));
    SendMessageW(g_tip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);

    int y = 120;
    Group(L"Jogo e emulador", 16, y, 828, 88);
    Label(L"Jogo (pasta do dump)", 32, y + 22, 170); Edit(ID_GAME, 206, y + 22, 520); Btn(ID_GAME_BR, L"Procurar...", 734, y + 22, 96, 24);
    Label(L"Motor do jogo", 32, y + 54, 170); Edit(ID_EMU, 206, y + 54, 520); Btn(ID_EMU_BR, L"Procurar...", 734, y + 54, 96, 24);
    Tip(ID_GAME, L"A pasta do dump do UFC 5 (PPSA03541) com eboot.bin, sce_sys e Data. Pode escolher a pasta UFC 5 inteira: o port acha a pasta do jogo dentro dela.");
    Tip(ID_EMU, L"O motor do port (bin\\UFC5-Game.exe, baseado no KytyPS5). Saves e cache de shaders ficam na pasta bin.");

    y = 216;
    Group(L"Desempenho", 16, y, 828, 140);
    Label(L"Perfil", 32, y + 22, 130); Combo(ID_PROFILE, 166, y + 22, 250);
    Label(L"Resolução (janela)", 32, y + 52, 130); Combo(ID_RES, 166, y + 52, 250);
    Label(L"Apresentação", 32, y + 82, 130); Combo(ID_PRESENT, 166, y + 82, 250);
    Label(L"FPS máximo (vblank)", 32, y + 112, 130); Combo(ID_VBLANK, 166, y + 112, 250);
    Label(L"Shaders", 440, y + 22, 130); Combo(ID_SHADER, 574, y + 22, 256);
    Label(L"Placa de vídeo", 440, y + 52, 130); Combo(ID_GPU, 574, y + 52, 256);
    Label(L"Idioma do jogo", 440, y + 82, 130); Combo(ID_LANG, 574, y + 82, 256);
    Label(L"Nome do jogador", 440, y + 112, 130); Edit(ID_USER, 574, y + 112, 256);
    for (int i = 0; i < 5; i++) Add(ID_PROFILE, PROFILE_NAME[i]);
    for (int i = 0; i < NRES; i++) {
        if (i == 0) { DEVMODEW dm = {.dmSize = sizeof(dm)}; wchar_t b[64]; EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm);
                      if (dm.dmDisplayFrequency > 1) _snwprintf(b, 64, L"Nativa do monitor (%lu x %lu, %lu Hz)", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmDisplayFrequency);
                      else _snwprintf(b, 64, L"Nativa do monitor (%lu x %lu)", dm.dmPelsWidth, dm.dmPelsHeight);
                      Add(ID_RES, b); }
        else Add(ID_RES, RES[i].name);
    }
    for (int i = 0; i < 3; i++) Add(ID_PRESENT, PRESENT_NAME[i]);
    for (int i = 0; i < NVBLANK; i++) Add(ID_VBLANK, VBLANK_NAME[i]);
    for (int i = 0; i < 3; i++) Add(ID_SHADER, SHADER_NAME[i]);
    Add(ID_GPU, L"Automática (o emulador escolhe)");
    for (int i = 0; i < g_ngpus; i++) { wchar_t b[300]; _snwprintf(b, 300, L"%d: %s (%llu GB)", i, g_gpus[i].name, (unsigned long long)((g_gpus[i].vram + (256ull << 20)) >> 30)); Add(ID_GPU, b); }
    for (int i = 0; i < NLANGS; i++) Add(ID_LANG, LANGS[i].name);
    SendMessageW(Ctl(ID_USER), EM_LIMITTEXT, 16, 0);
    Tip(ID_PROFILE, L"Máximo FPS: sem VSync, prioridade alta, plano de energia de alto desempenho, sem notificações de troféu.\nEquilibrado: Mailbox (sem tearing) e troféus ligados.\nQualidade: VSync (Fifo), o mais estável.\nPC fraco: tudo do Máximo FPS, janela em 1080p, sem travadas de shader e limite de memória de vídeo (escolhido sozinho em PCs com menos de 8 threads, placa com menos de 6 GB ou menos de 16 GB de RAM).\nMudar qualquer opção abaixo vira Personalizado.");
    Tip(ID_RES, L"Tamanho da janela do jogo no modo janela. Em tela cheia o port usa sempre a resolução do monitor: com uma menor, o motor deixava a imagem pequena no meio da tela. Não muda o FPS: o jogo renderiza por dentro do mesmo jeito em qualquer tamanho.");
    Tip(ID_PRESENT, L"Immediate tira a espera do VSync (mais FPS e menos atraso, pode ter tearing). Mailbox é quase tão rápido sem tearing. Se o seu monitor/GPU não suportar o modo, o emulador volta sozinho para Fifo.");
    Tip(ID_VBLANK, L"O vblank virtual é o \"relógio\" em que o motor mostra cada quadro do jogo. Em 60 Hz um quadro que fica pronto logo depois do tique espera até 16,7 ms para aparecer; em 120/144 Hz espera no máximo 8,3/6,9 ms, e os menus passam de 60 FPS. Em 120 Hz ou mais o jogo acha que está numa tela de 120 Hz.\n\nExperimental: se o relógio do round ou as animações correrem mais rápido que o normal, volte para 60 Hz. A luta continua limitada pelo processador (perto de 30 FPS num Ryzen 5 5500); para ver 120/144 na tela use a Geração de quadros.");
    Tip(ID_SHADER, L"Otimização dos shaders traduzidos. Performance dá mais FPS; a primeira vez em cada arena compila mais devagar, depois fica no cache.");
    Tip(ID_GPU, L"Qual placa de vídeo o emulador usa. Em notebook com placa integrada + dedicada, escolha a dedicada.");

    y = 364;
    Group(L"Opções", 16, y, 828, 210);
    Check(ID_FULL, L"Tela cheia (sem bordas)", 32, y + 22, 270);
    Check(ID_PRIO, L"Prioridade alta para o jogo", 32, y + 44, 270);
    Check(ID_DGPU, L"Forçar GPU dedicada (Windows)", 32, y + 66, 270);
    Check(ID_POWER, L"Plano de energia de alto desempenho", 32, y + 88, 270);
    Check(ID_AMD, L"Patches de CPU AMD (experimental)", 308, y + 22, 270);
    Check(ID_CURSOR, L"Esconder o cursor do mouse", 308, y + 44, 270);
    Check(ID_TROPHY, L"Notificações de troféus", 308, y + 66, 270);
    Check(ID_CONSOLE, L"Mostrar console do motor", 308, y + 88, 270);
    Label(L"FPS na tela", 32, y + 112, 90); Combo(ID_OVERLAY, 126, y + 112, 166);
    Check(ID_ECO, L"Sem economia de energia (EcoQoS)", 308, y + 112, 270);
    Check(ID_FASTSH, L"Compilação de shaders rápida", 584, y + 112, 260);
    Check(ID_LPAGES, L"Large pages (precisa de admin uma vez)", 32, y + 136, 274);
    Check(ID_STAGEPREP, L"Preparação paralela (experimental)", 308, y + 136, 270);
    Check(ID_CALIB, L"Calibrar FPS sozinho (12 lutas)", 584, y + 136, 260);
    Check(ID_CFPU, L"Matemática do console (+FPS na luta)", 32, y + 158, 274);
    Check(ID_DIAG, L"Diagnóstico de desempenho no log", 308, y + 158, 270);
    Check(ID_DIAGFULL, L"Diagnóstico completo (1 partida)", 584, y + 158, 260);
    Label(L"Geração de quadros", 32, y + 182, 130); Combo(ID_FRAMEGEN, 166, y + 182, 400);
    for (int i = 0; i < 3; i++) Add(ID_FRAMEGEN, FRAMEGEN_NAME[i]);
    Tip(ID_FRAMEGEN, L"Mostra 120/144 FPS na tela com o Lossless Scaling (Steam), que cria quadros intermediários entre os do jogo. O jogo em si continua no FPS do motor (perto de 30 na luta): a imagem fica mais fluida, com um pouco mais de atraso no controle.\n\n\"Abrir e ligar sozinho\": o port abre o Lossless Scaling antes do jogo e aperta o atalho dele (Ctrl+Alt+S; outro atalho em ls_hotkey= no UFC5.ini) uns segundos depois que o jogo aparece.\n\"Só abrir\": para quem fez no Lossless Scaling um perfil do UFC5-Game.exe com Auto Scale.\n\nNo Lossless Scaling: Frame Generation LSFG (a mais nova), modo Adaptive com alvo 144 (ou 120), ou Fixed X4.");
    Tip(ID_DIAGFULL, L"Só na próxima partida: o motor grava o próprio log (_Logs\\UFC5_..._motor.txt) com o tempo da thread de comandos da GPU (KYTY_GPU_BREAKDOWN) e das sincronizações de memória (KYTY_SYNC_STATS). Mostra exatamente onde a luta perde tempo no seu PC. Essa partida pode ficar um pouco mais lenta; depois desliga sozinho. Jogue uma luta de pelo menos 1 minuto e mande os dois arquivos da pasta _Logs.");
    Tip(ID_CFPU, L"KYTY_GUEST_MXCSR: as threads do jogo rodam com o mesmo modo de ponto flutuante do PS5 (flush-to-zero / denormals-are-zero, MXCSR 0x9fc0). Sem isso, contas com números muito pequenos (física, animação e som da luta) caem no caminho lento do processador, que custa até 100 vezes mais. É o modo do console, então não muda o jogo: só tira essa lentidão. O port do UFC 4 ganhou FPS nas lutas com o mesmo ajuste.");
    Tip(ID_DIAG, L"Escreve no log, a cada poucos segundos, quanto tempo a thread de comandos da GPU passa ocupada, esperando o jogo ou esperando a placa de vídeo (KYTY_GPU_BREAKDOWN), e o custo da proteção de memória (KYTY_PROTECT_STATS). Custa quase nada e mostra qual é o gargalo no seu PC: o resumo aparece aqui quando o jogo fecha.");
    Tip(ID_STAGEPREP, L"KYTY_STAGE_PREP_PARALLEL: prepara os recursos dos shaders de vértice e de pixel de cada desenho em paralelo, em duas threads, em vez de uma depois da outra. Alivia o processador, que é o limite do UFC 5 na maioria dos PCs. É experimental no motor: se der problema, desligue.");
    Tip(ID_CALIB, L"O port mede o FPS de verdade no seu PC, só durante a luta (os menus a 60 FPS não entram na conta). São até 6 testes de 2 lutas cada (uma com, outra sem): matemática do console, preparação paralela, núcleos próprios para o motor, proteção de memória em blocos maiores, menos threads de shader e prioridade acima do normal. Cada opção só fica se ajudar; se uma delas fechar o jogo, é desligada na hora. Cada luta precisa de pelo menos 30 segundos.");
    Tip(ID_FASTSH, L"Compila os shaders com mais threads do processador (metade dos threads menos 1, em vez de 2). Cada shader novo fica pronto mais rápido, então as travadas da primeira vez em cada cena são mais curtas.");
    Tip(ID_LPAGES, L"Usa páginas de memória grandes para o código do jogo (menos falhas de TLB no processador, um pouco mais de FPS). O Windows exige a permissão \"Bloquear páginas na memória\": o port pede como administrador uma vez, e depois é preciso sair e entrar de novo no Windows.");
    Add(ID_OVERLAY, L"Desligado"); Add(ID_OVERLAY, L"Canto superior esquerdo"); Add(ID_OVERLAY, L"Canto superior direito");
    Tip(ID_OVERLAY, L"Mostra o FPS por cima do jogo, lá em cima, com a média e o 1% pior da sessão. Durante o jogo, F10 liga e desliga.");
    Tip(ID_ECO, L"Diz ao Windows para nunca colocar o jogo em modo de economia (EcoQoS / Power Throttling). Sem isso o Windows pode mandar as threads do jogo para os núcleos de eficiência (Intel 12ª geração ou mais nova) ou reduzir o clock em notebook.");
    Check(ID_BINDLESS, L"Texturas bindless (experimental)", 584, y + 22, 260);
    Check(ID_TESS, L"Tesselação (experimental)", 584, y + 44, 260);
    Check(ID_PLAYGO, L"PlayGo hack (se travar no load)", 584, y + 66, 260);
    Check(ID_READBACK, L"Readback de imagens (recomendado)", 584, y + 88, 260);
    Tip(ID_PRIO, L"Roda o motor do jogo com prioridade Alta no Windows, para outros programas não roubarem tempo de CPU.");
    Tip(ID_DGPU, L"Grava a preferência \"Alto desempenho\" do Windows (Configurações > Sistema > Tela > Gráficos) para o kyty_emulator.exe.");
    Tip(ID_POWER, L"Liga o plano de energia Alto desempenho enquanto o jogo está aberto e volta ao seu plano quando ele fecha.");
    Tip(ID_AMD, L"--amd-cpu: troca instruções de raiz quadrada aproximada do jogo. O build do UFC 5 foi ajustado SEM isso; ligado, a luta pode ficar com o cenário e os lutadores pretos. Deixe desligado, a não ser que o jogo trave sem ele.");
    Tip(ID_TROPHY, L"Mostra os avisos de troféu com som. Desligado no perfil Máximo FPS.");
    Tip(ID_CONSOLE, L"Mostra a janela de console do motor. Desligado, a saída vai para um .log na pasta bin\\_Logs (botão Abrir logs).");
    Tip(ID_BINDLESS, L"--bindless: amostra texturas por arrays bindless em vez de enumerar por draw. Pode reduzir o custo de CPU por draw, mas é experimental: teste e desligue se aparecer textura errada.");
    Tip(ID_TESS, L"--tessellation: desenha os patches de tesselação (por padrão são pulados). Pode travar.");
    Tip(ID_PLAYGO, L"--playgo-hack: usa o stub de PlayGo do emulador. Só ligue se o jogo travar carregando.");

    y = 582;
    Group(L"Avançado", 16, y, 828, 118);
    Label(L"Alto-falante do controle", 32, y + 22, 150); Combo(ID_VOL, 186, y + 22, 96);
    Label(L"Vibração", 304, y + 22, 70); Combo(ID_VIB, 374, y + 22, 90);
    Label(L"Modo X3D (Ryzen)", 486, y + 22, 106); Combo(ID_X3D, 594, y + 22, 236);
    Label(L"Arquivo de cheats", 32, y + 52, 150); Edit(ID_PATCH, 186, y + 52, 540); Btn(ID_PATCH_BR, L"Procurar...", 734, y + 52, 96, 24);
    Label(L"Argumentos extras", 32, y + 82, 150); Edit(ID_EXTRA, 186, y + 82, 260);
    Label(L"Variáveis", 464, y + 82, 70); Edit(ID_ENV, 534, y + 82, 296);
    for (int i = 0; i <= 10; i++) { wchar_t b[16]; _snwprintf(b, 16, L"%d%%", i * 10); Add(ID_VOL, b); Add(ID_VIB, b); }
    { wchar_t b[96]; _snwprintf(b, 96, L"Automático (%s)", CpuIsX3d() ? L"X3D detectado: ligado" : L"desligado neste PC"); Add(ID_X3D, b); }
    Add(ID_X3D, L"Ligado"); Add(ID_X3D, L"Desligado");
    Tip(ID_X3D, L"--cpu-placement: em processadores Ryzen X3D, coloca as threads mais pesadas do emulador no CCD com V-Cache, em núcleos próprios e com prioridade. Dá mais FPS nesses processadores. Automático liga só se o processador for X3D.");
    Tip(ID_READBACK, L"--readback-linear-images true: lê de volta as imagens lineares graváveis a cada envio. É a configuração com que este build do UFC 5 foi ajustado; desligar pode dar um pouco de FPS mas pode quebrar gráficos.");
    Tip(ID_VOL, L"Volume do alto-falante do DualSense (USB ou Bluetooth). 50% é o padrão.");
    Tip(ID_PATCH, L"--game-patch: arquivo de cheats/patches no formato ETAHen (.json). Opcional.");
    Tip(ID_EXTRA, L"Qualquer opção a mais do kyty_emulator, por exemplo: --keymap Cross=Space --mic \"Microfone\"");
    Tip(ID_ENV, L"Variáveis de ambiente do emulador, separadas por ponto e vírgula. Ex.: KYTY_SHADER_DISK_CACHE=1");

    y = 712;
    HWND play = Btn(ID_PLAY, L"JOGAR", 16, y, 200, 42); SendMessageW(play, WM_SETFONT, (WPARAM)g_fontPlay, TRUE);
    Btn(ID_AUTO, L"Config. automática", 228, y, 150, 42);
    Btn(ID_KYTY, L"Atalho na área de trabalho", 386, y, 160, 42);
    Btn(ID_LOGS, L"Abrir logs", 554, y, 120, 42);
    Btn(ID_CLEAR, L"Limpar cache de shaders", 682, y, 162, 42);
    Tip(ID_AUTO, L"Lê o seu PC (placa de vídeo, memória de vídeo, processador, monitor) e escolhe as opções de mais FPS para ele.");
    Tip(ID_KYTY, L"Cria o atalho \"EA SPORTS UFC 5\" na área de trabalho, que abre direto no jogo com estas configurações (como um jogo de PC).");
    Tip(ID_CLEAR, L"Manda o cache de shaders e pipelines do UFC 5 para a Lixeira. Só use se aparecerem gráficos quebrados depois de trocar o emulador ou o driver: a primeira luta depois disso vai engasgar enquanto recompila.");

    g_status = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, SC(16), SC(764), SC(828), SC(40), g_wnd, (HMENU)ID_STATUS, g_inst, NULL);
    SendMessageW(g_status, WM_SETFONT, (WPARAM)g_font, TRUE);
}

/* ---------------------------------------------------------------- dialogs */
static BOOL PickFolder(wchar_t *path)
{
    IFileOpenDialog *d = NULL; BOOL ok = FALSE;
    if (FAILED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileOpenDialog, (void **)&d))) return FALSE;
    DWORD o; d->lpVtbl->GetOptions(d, &o); d->lpVtbl->SetOptions(d, o | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    d->lpVtbl->SetTitle(d, L"Escolha a pasta do UFC 5 (PPSA03541)");
    if (path[0]) { IShellItem *si; if (SUCCEEDED(SHCreateItemFromParsingName(path, NULL, &IID_IShellItem, (void **)&si))) { d->lpVtbl->SetFolder(d, si); si->lpVtbl->Release(si); } }
    if (SUCCEEDED(d->lpVtbl->Show(d, g_wnd))) {
        IShellItem *it; if (SUCCEEDED(d->lpVtbl->GetResult(d, &it))) {
            PWSTR p; if (SUCCEEDED(it->lpVtbl->GetDisplayName(it, SIGDN_FILESYSPATH, &p))) { wcsncpy(path, p, MAX_PATH); path[MAX_PATH - 1] = 0; CoTaskMemFree(p); ok = TRUE; }   /* caminho longo: wcsncpy não termina */
            it->lpVtbl->Release(it);
        }
    }
    d->lpVtbl->Release(d); return ok;
}

static BOOL PickFile(wchar_t *path, const wchar_t *filter, const wchar_t *title)
{
    wchar_t buf[MAX_PATH]; wcscpy(buf, path);
    OPENFILENAMEW of = {sizeof(of)}; of.hwndOwner = g_wnd; of.lpstrFilter = filter; of.lpstrFile = buf; of.nMaxFile = MAX_PATH;
    of.lpstrTitle = title; of.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&of)) return FALSE;
    wcscpy(path, buf); return TRUE;
}

/* ---------------------------------------------------------------- launching */
static void AppendArg(wchar_t *cmd, size_t cap, const wchar_t *fmt, ...)
{
    size_t n = wcslen(cmd); va_list ap; va_start(ap, fmt); _vsnwprintf(cmd + n, cap - n, fmt, ap); va_end(ap); cmd[cap - 1] = 0;
}

static void ApplyEnv(void)
{
    wchar_t buf[2048]; wcscpy(buf, S.env);
    wchar_t *ctx = NULL;
    for (wchar_t *t = wcstok(buf, L";\r\n", &ctx); t; t = wcstok(NULL, L";\r\n", &ctx)) {
        while (*t == L' ') t++;
        wchar_t *eq = wcschr(t, L'='); if (!eq || eq == t) continue;
        *eq = 0; SetEnvironmentVariableW(t, eq + 1);
    }
}

static void SetGpuPreference(const wchar_t *exe)
{
    HKEY k;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\DirectX\\UserGpuPreferences", 0, NULL, 0, KEY_SET_VALUE, NULL, &k, NULL) == ERROR_SUCCESS) {
        const wchar_t *v = L"GpuPreference=2;";
        RegSetValueExW(k, exe, 0, REG_SZ, (const BYTE *)v, (DWORD)((wcslen(v) + 1) * sizeof(wchar_t)));
        RegCloseKey(k);
    }
}

static void PowerHigh(void)
{
    GUID *cur = NULL; g_schemeChanged = FALSE;
    if (PowerGetActiveScheme(NULL, &cur) == ERROR_SUCCESS && cur) {
        g_prevScheme = *cur; LocalFree(cur);
        if (memcmp(&g_prevScheme, &GUID_MIN_POWER_SAVINGS, sizeof(GUID)) && PowerSetActiveScheme(NULL, &GUID_MIN_POWER_SAVINGS) == ERROR_SUCCESS)
            g_schemeChanged = TRUE;
    }
}
static void PowerRestore(void) { if (g_schemeChanged) { PowerSetActiveScheme(NULL, &g_prevScheme); g_schemeChanged = FALSE; } }

static BOOL HasDll(const wchar_t *dll) { wchar_t p[MAX_PATH]; return SearchPathW(NULL, dll, NULL, MAX_PATH, p, NULL) > 0; }

/* ---------------------------------------------------------------- frame generation: Lossless Scaling (Steam app 993090)
   The engine's own frame rate stays what it is (about 30 in a fight on a 6-core Ryzen); Lossless Scaling captures the
   game window and shows generated frames in between, so the screen gets 60-144. The port opens it before the game
   (minimized, so it never covers the game), and with framegen=1 presses its hotkey once the game window has been in
   front and drawing for a few seconds. */
static DWORD FindPid(const wchar_t *exe)
{
    DWORD pid = 0; HANDLE sn = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); PROCESSENTRY32W pe = {sizeof(pe)};
    if (sn == INVALID_HANDLE_VALUE) return 0;
    if (Process32FirstW(sn, &pe)) do { if (!_wcsicmp(pe.szExeFile, exe)) { pid = pe.th32ProcessID; break; } } while (Process32NextW(sn, &pe));
    CloseHandle(sn); return pid;
}
typedef struct { DWORD pid; HWND found; } FindCtx;
static BOOL CALLBACK FindTopWnd(HWND h, LPARAM lp)
{
    FindCtx *c = (FindCtx *)lp; DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid == c->pid && IsWindowVisible(h) && GetWindow(h, GW_OWNER) == NULL) { c->found = h; return FALSE; }
    return TRUE;
}
static HWND TopWindowOf(DWORD pid) { FindCtx c = {pid, NULL}; if (pid) EnumWindows(FindTopWnd, (LPARAM)&c); return c.found; }
/* Steam's own folder, then the libraries listed in libraryfolders.vdf */
static BOOL LsExePath(wchar_t *out)
{
    wchar_t steam[MAX_PATH] = L""; DWORD n = sizeof(steam);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_REG_SZ, NULL, steam, &n) != ERROR_SUCCESS) return FALSE;
    for (wchar_t *c = steam; *c; c++) if (*c == L'/') *c = L'\\';
    _snwprintf(out, MAX_PATH, L"%s\\steamapps\\common\\Lossless Scaling\\LosslessScaling.exe", steam); out[MAX_PATH - 1] = 0;
    if (FileExists(out)) return TRUE;
    wchar_t vdf[MAX_PATH]; _snwprintf(vdf, MAX_PATH, L"%s\\steamapps\\libraryfolders.vdf", steam); vdf[MAX_PATH - 1] = 0;
    HANDLE f = CreateFileW(vdf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return FALSE;
    static char b[65536]; DWORD got = 0; ReadFile(f, b, sizeof(b) - 1, &got, NULL); CloseHandle(f); b[got] = 0;
    for (char *p = b; (p = strstr(p, "\"path\"")) != NULL; ) {
        p += 6; char *q = strchr(p, '"'); if (!q) break; q++; char *e = strchr(q, '"'); if (!e) break;
        char path[MAX_PATH]; int k = 0;
        for (char *s = q; s < e && k < MAX_PATH - 1; s++) { if (s[0] == '\\' && s[1] == '\\') s++; path[k++] = *s; }
        path[k] = 0; p = e + 1;
        wchar_t lib[MAX_PATH]; MultiByteToWideChar(CP_UTF8, 0, path, -1, lib, MAX_PATH);
        _snwprintf(out, MAX_PATH, L"%s\\steamapps\\common\\Lossless Scaling\\LosslessScaling.exe", lib); out[MAX_PATH - 1] = 0;
        if (FileExists(out)) return TRUE;
    }
    return FALSE;
}
static BOOL LsInstalled(void) { wchar_t p[MAX_PATH]; return FindPid(L"LosslessScaling.exe") || LsExePath(p); }
/* opens Lossless Scaling if it is not running yet and waits (up to 25 s) for its window, which is then minimized */
static void LsEnsure(void)
{
    g_lsState = 0;
    if (FindPid(L"LosslessScaling.exe")) { g_lsState = 1; return; }
    wchar_t exe[MAX_PATH]; BOOL have = LsExePath(exe);
    SetStatus(C_SUB, L"Abrindo o Lossless Scaling para a geração de quadros...");
    /* through Steam (the app needs it); straight from its folder only if the steam:// link does not work */
    if ((INT_PTR)ShellExecuteW(NULL, L"open", L"steam://rungameid/993090", NULL, NULL, SW_SHOWMINNOACTIVE) <= 32 && have) {
        wchar_t d[MAX_PATH]; DirOf(d, exe); ShellExecuteW(NULL, L"open", exe, NULL, d, SW_SHOWMINNOACTIVE);
    }
    DWORD t0 = GetTickCount(), pid = 0; HWND lw = NULL;
    while (GetTickCount() - t0 < 25000) {
        MSG m; while (PeekMessageW(&m, NULL, 0, 0, PM_REMOVE)) { if (m.message == WM_QUIT) { PostQuitMessage((int)m.wParam); return; } TranslateMessage(&m); DispatchMessageW(&m); }
        if (!pid) pid = FindPid(L"LosslessScaling.exe");
        if (pid && (lw = TopWindowOf(pid)) != NULL) break;
        Sleep(200);
    }
    if (lw) { Sleep(700); ShowWindowAsync(lw, SW_MINIMIZE); }   /* async: never wait on another program's window */
    g_lsState = pid ? 1 : 2;
}
/* "Ctrl+Alt+S", "Shift+F10", ... -> virtual keys; FALSE if it cannot be read */
static BOOL ParseHotkey(const wchar_t *s, WORD *mods, int *nm, WORD *key)
{
    wchar_t b[64]; wcsncpy(b, s, 63); b[63] = 0; *nm = 0; *key = 0; wchar_t *ctx = NULL;
    for (wchar_t *t = wcstok(b, L"+ ", &ctx); t; t = wcstok(NULL, L"+ ", &ctx)) {
        if (!_wcsicmp(t, L"Ctrl") || !_wcsicmp(t, L"Control")) { if (*nm < 4) mods[(*nm)++] = VK_CONTROL; }
        else if (!_wcsicmp(t, L"Alt")) { if (*nm < 4) mods[(*nm)++] = VK_MENU; }
        else if (!_wcsicmp(t, L"Shift")) { if (*nm < 4) mods[(*nm)++] = VK_SHIFT; }
        else if (!_wcsicmp(t, L"Win")) { if (*nm < 4) mods[(*nm)++] = VK_LWIN; }
        else if (wcslen(t) == 1 && iswalnum(t[0])) *key = (WORD)towupper(t[0]);
        else if ((t[0] == L'F' || t[0] == L'f') && _wtoi(t + 1) >= 1 && _wtoi(t + 1) <= 24) *key = (WORD)(VK_F1 + _wtoi(t + 1) - 1);
        else if (!_wcsicmp(t, L"Home")) *key = VK_HOME; else if (!_wcsicmp(t, L"End")) *key = VK_END;
        else if (!_wcsicmp(t, L"Insert") || !_wcsicmp(t, L"Ins")) *key = VK_INSERT; else if (!_wcsicmp(t, L"Delete") || !_wcsicmp(t, L"Del")) *key = VK_DELETE;
        else if (!_wcsicmp(t, L"PageUp")) *key = VK_PRIOR; else if (!_wcsicmp(t, L"PageDown")) *key = VK_NEXT;
        else if (!_wcsicmp(t, L"Pause")) *key = VK_PAUSE; else if (!_wcsicmp(t, L"ScrollLock")) *key = VK_SCROLL;
        else return FALSE;
    }
    return *key != 0;
}
static void KeyInput(INPUT *in, WORD vk, BOOL up)
{
    memset(in, 0, sizeof(*in)); in->type = INPUT_KEYBOARD; in->ki.wVk = vk; in->ki.wScan = (WORD)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    in->ki.dwFlags = (up ? KEYEVENTF_KEYUP : 0) | ((vk == VK_HOME || vk == VK_END || vk == VK_INSERT || vk == VK_DELETE || vk == VK_PRIOR || vk == VK_NEXT || vk == VK_LWIN) ? KEYEVENTF_EXTENDEDKEY : 0);
}
static BOOL SendHotkey(const wchar_t *spec)
{
    WORD mods[4], key; int nm;
    if (!ParseHotkey(spec, mods, &nm, &key)) return FALSE;
    INPUT in[10]; int n = 0;
    for (int i = 0; i < nm; i++) KeyInput(&in[n++], mods[i], FALSE);
    KeyInput(&in[n++], key, FALSE);
    if (SendInput(n, in, sizeof(INPUT)) != (UINT)n) return FALSE;
    Sleep(40); n = 0;
    KeyInput(&in[n++], key, TRUE);
    for (int i = nm - 1; i >= 0; i--) KeyInput(&in[n++], mods[i], TRUE);
    SendInput(n, in, sizeof(INPUT));
    return TRUE;
}
/* puts the game in front: a tap of Alt lets a background process do it (Windows' foreground lock) */
static void BringToFront(HWND w)
{
    if (GetForegroundWindow() == w) return;
    INPUT in[2]; KeyInput(&in[0], VK_MENU, FALSE); KeyInput(&in[1], VK_MENU, TRUE); SendInput(2, in, sizeof(INPUT));
    SetForegroundWindow(w);
}
/* notes for the session log: kept here and written when the game has closed (the engine writes the same file
   through its own handle while it runs, and would write over anything appended meanwhile) */
static char g_notes[2048]; static int g_nnotes;
static void AppendToLog(const char *line)
{
    int n = (int)strlen(line); if (g_nnotes + n >= (int)sizeof(g_notes)) return;
    memcpy(g_notes + g_nnotes, line, n); g_nnotes += n; g_notes[g_nnotes] = 0;
}
/* called with every FPS sample: once the game window has been in front and drawing for ls_delay seconds */
static void LsTick(void)
{
    if (S.framegen != 1 || g_lsSent || !g_gameWnd) return;
    if (g_lsState == 2) {   /* Steam was slow to open it: it came up after the game */
        DWORD pid = FindPid(L"LosslessScaling.exe"); if (!pid) return;
        HWND lw = TopWindowOf(pid); if (lw && !IsIconic(lw)) ShowWindowAsync(lw, SW_MINIMIZE);
        g_lsState = 1;
    }
    if (g_lsState != 1) return;
    int delay = IniInt(L"ls_delay", 8); if (delay < 2) delay = 2;
    if (g_nfps < delay) return;
    if (!FindPid(L"LosslessScaling.exe")) { g_lsSent = TRUE; AppendToLog("\r\n> LSFG: o Lossless Scaling fechou antes do jogo; atalho não enviado"); return; }
    BringToFront(g_gameWnd);
    if (GetForegroundWindow() != g_gameWnd) { if (++g_lsTries > 30) { g_lsSent = TRUE; AppendToLog("\r\n> LSFG: o jogo não ficou na frente; atalho não enviado"); } return; }
    wchar_t hk[64]; IniStr(L"ls_hotkey", hk, 64, L"Ctrl+Alt+S");
    BOOL ok = SendHotkey(hk); g_lsSent = TRUE;
    char l8[160], h8[64]; WideCharToMultiByte(CP_UTF8, 0, hk, -1, h8, 64, NULL, NULL);
    snprintf(l8, sizeof(l8), "\r\n> LSFG: atalho %s %s aos %d s de jogo", h8, ok ? "enviado" : "inválido (veja ls_hotkey no UFC5.ini)", g_nfps);
    AppendToLog(l8);
}

static void PruneLogs(const wchar_t *dir, const wchar_t *mask, int keep)
{   /* keep the newest <keep> files of <mask> (15 launcher logs, 4 engine logs of full diagnostics) */
    wchar_t pat[MAX_PATH]; Join(pat, dir, mask);
    wchar_t names[256][64]; int n = 0; WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do { if (n < 256) { wcsncpy(names[n], fd.cFileName, 63); names[n][63] = 0; n++; } } while (FindNextFileW(h, &fd));
    FindClose(h);
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (wcscmp(names[i], names[j]) > 0) { wchar_t t[64]; wcscpy(t, names[i]); wcscpy(names[i], names[j]); wcscpy(names[j], t); }
    for (int i = 0; i + keep < n; i++) { wchar_t p[MAX_PATH]; Join(p, dir, names[i]); DeleteFileW(p); }
}

static DWORD WINAPI WaitThread(LPVOID p)
{
    HANDLE h = (HANDLE)p; WaitForSingleObject(h, INFINITE);
    DWORD code = 0; GetExitCodeProcess(h, &code);
    PostMessageW(g_wnd, WM_APP_EXITED, code, 0); return 0;
}

static void Play(void)
{
    if (g_proc) { SetStatus(C_SUB, L"O jogo já está aberto."); return; }
    if (g_starting) return;   /* still opening Lossless Scaling for this start */
    UiToSettings();
    if (!ResolveGameDir(S.game)) {
        SetStatus(C_BAD, L"Não achei o jogo. Escolha a pasta do dump do UFC 5 (a que tem eboot.bin, sce_sys e Data).");
        return;
    }
    SetDlgItemTextW(g_wnd, ID_GAME, S.game);
    if (!FileExists(S.emu)) { SetStatus(C_BAD, L"Não achei o motor do jogo (bin\\UFC5-Game.exe). Clique em Procurar... ao lado de Motor do jogo."); return; }
    if (!HasDll(L"vulkan-1.dll")) { SetStatus(C_BAD, L"O Vulkan não está instalado. Atualize o driver da sua placa de vídeo (NVIDIA, AMD ou Intel)."); return; }
    if (!HasDll(L"msvcp140.dll") || !HasDll(L"vcruntime140_1.dll")) {
        if (MessageBoxW(g_wnd, L"O UFC 5 precisa do Microsoft Visual C++ Redistributable (x64), que não está instalado.\n\nAbrir a página de download da Microsoft?", APP_TITLE, MB_YESNO | MB_ICONWARNING) == IDYES)
            ShellExecuteW(g_wnd, L"open", L"https://aka.ms/vs/17/release/vc_redist.x64.exe", NULL, NULL, SW_SHOWNORMAL);
        return;
    }
    if (S.patch[0] && !FileExists(S.patch)) { SetStatus(C_BAD, L"O arquivo de cheats não existe: %s", S.patch); return; }
    SaveSettings();
    CheckCommit();

    wchar_t emuDir[MAX_PATH]; DirOf(emuDir, S.emu);
    int w = RES[S.res].w, h = RES[S.res].h;
    /* fullscreen always at the monitor's size: with a smaller one this engine draws the picture 1:1 in the middle of
       the screen (1280x720 used a quarter of a 1440p monitor), and the size does not change what the game renders */
    if (S.res == 0 || S.full) { DEVMODEW dm = {.dmSize = sizeof(dm)}; EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm); w = dm.dmPelsWidth; h = dm.dmPelsHeight; if (!w) { w = 1920; h = 1080; } }
    wchar_t logs[MAX_PATH]; Join(logs, emuDir, L"_Logs"); CreateDirectoryW(logs, NULL);
    SYSTEMTIME lt; GetLocalTime(&lt);

    wchar_t runExe[MAX_PATH]; wcscpy(runExe, S.emu);
    static wchar_t cmd[8192]; cmd[0] = 0;
    AppendArg(cmd, 8192, L"\"%s\" --game \"%s\"", runExe, S.game);
    AppendArg(cmd, 8192, L" --screen-width %d --screen-height %d", w, h);
    /* the options that left the fight black or recompiled every shader (log of 2026-10-19 04:28) stay off unless expert_options=1 */
    int safeAmd = S.expert && S.amd, safeBl = S.expert && S.bindless, safeTess = S.expert && S.tess;
    if (S.vblank < 0 || S.vblank >= NVBLANK) S.vblank = 0;
    AppendArg(cmd, 8192, L" --present-mode %s --vblank-frequency %d", PRESENT_ARG[S.present], VBLANK[S.vblank]);
    /* full diagnostic (one session): the engine's own log, where KYTY_GPU_BREAKDOWN and KYTY_SYNC_STATS print */
    g_runDiagFull = S.diagFull; g_motorLog[0] = 0;
    if (g_runDiagFull) {
        _snwprintf(g_motorLog, MAX_PATH, L"%s\\UFC5_%04d%02d%02d_%02d%02d%02d_motor.txt", logs, lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
        g_motorLog[MAX_PATH - 1] = 0;
        AppendArg(cmd, 8192, L" --printf-direction File --printf-output-file \"%s\"", g_motorLog);
    }
    AppendArg(cmd, 8192, L" --shader-optimization-type %s", SHADER_ARG[S.shader]);
    AppendArg(cmd, 8192, L" --console-language %d", LANGS[S.lang].code);
    if (S.user[0]) { wchar_t u[64]; int j = 0; for (int i = 0; S.user[i] && j < 63; i++) if (S.user[i] != L'"') u[j++] = S.user[i]; u[j] = 0; AppendArg(cmd, 8192, L" --user-name \"%s\"", u); }
    AppendArg(cmd, 8192, L" --controller-volume %d --controller-vibration %d", S.vol, S.vib);
    AppendArg(cmd, 8192, L" --trophy-notifications %s", S.trophy ? L"true" : L"false");
    if (S.full) AppendArg(cmd, 8192, L" --fullscreen");
    if (safeAmd) AppendArg(cmd, 8192, L" --amd-cpu");
    if (S.cursor) AppendArg(cmd, 8192, L" --hide-cursor");
    if (safeBl) AppendArg(cmd, 8192, L" --bindless");
    if (safeTess) AppendArg(cmd, 8192, L" --tessellation");
    if (S.playgo) AppendArg(cmd, 8192, L" --playgo-hack");
    AppendArg(cmd, 8192, L" --readback-linear-images %s", S.readback ? L"true" : L"false");
    {   /* calibration: stage 0 alternates the parallel preparation, stage 1 the engine's own cores */
        int stage = S.calib ? S.calibStage : -1;
        g_runCfpu = stage == 0 ? S.calibArm : S.cfpu;
        g_runStagePrep = stage == 1 ? S.calibArm : S.stageprep;
        g_runPlace = stage == 2 ? S.calibArm : S.place;
        g_runPgap = stage == 3 ? S.calibArm : S.pgap;
        g_runLowsh = stage == 4 ? S.calibArm : S.lowsh;
        g_runMidprio = stage == 5 ? S.calibArm : S.midprio;
    }
    if (X3dOn() || g_runPlace) AppendArg(cmd, 8192, L" --cpu-placement true");
    if (S.gpu >= 0) AppendArg(cmd, 8192, L" --gpu %d", S.gpu);
    if (S.patch[0]) AppendArg(cmd, 8192, L" --game-patch \"%s\"", S.patch);
    if (S.extra[0]) AppendArg(cmd, 8192, L" %s", S.extra);

    {   /* shader stalls: wait at most 2 ms for a pipeline (draw is skipped meanwhile) and translate on more threads */
        SYSTEM_INFO si2; GetSystemInfo(&si2); int tt = (int)si2.dwNumberOfProcessors / 2 - 1; if (tt < 2) tt = 2; if (tt > 8) tt = 8;
        wchar_t b[16]; _snwprintf(b, 16, L"%d", tt);
        /* the 20 ms pipeline wait is kept: shorter waits skip draws and can leave the fight black */
        SetEnvironmentVariableW(L"KYTY_PIPELINE_WAIT_MS", NULL);
        if (g_runLowsh && tt > 2) _snwprintf(b, 16, L"%d", 2);   /* fewer shader translation threads: more cores left for the fight */
        if (S.shthreads > 0) _snwprintf(b, 16, L"%d", S.shthreads);
        SetEnvironmentVariableW(L"KYTY_TRANSLATION_THREADS", (S.fastsh || S.shthreads > 0) ? b : NULL);
        SetEnvironmentVariableW(L"KYTY_LARGE_PAGES", S.lpages ? L"1" : NULL);
        SetEnvironmentVariableW(L"KYTY_STAGE_PREP_PARALLEL", g_runStagePrep ? L"1" : NULL);
        /* small video cards (or the weak-PC profile): keep the engine under ~85% of the VRAM, so textures are not thrashed */
        int bgv = S.gpu >= 0 ? S.gpu : BestGpu(); uint64_t vr = bgv >= 0 ? g_gpus[bgv].vram >> 20 : 0;
        if (vr && (vr < 7000 || S.profile == 3 || ArcGpu())) { wchar_t vb[24]; _snwprintf(vb, 24, L"%llu", (unsigned long long)(vr * 85 / 100)); SetEnvironmentVariableW(L"KYTY_VRAM_TARGET_MB", vb); }
        else SetEnvironmentVariableW(L"KYTY_VRAM_TARGET_MB", NULL);
        /* the game's threads in the PS5's floating-point mode (MXCSR 0x9fc0: flush-to-zero, denormals-are-zero): the
           engine leaves the host's mode by default, where physics and animation math on tiny numbers takes the slow path */
        SetEnvironmentVariableW(L"KYTY_GUEST_MXCSR", g_runCfpu ? L"1" : NULL);
        /* periodic lines: where the GPU command thread's time goes (only reaches a file with --printf-direction File,
           so only in the full diagnostic), the memory synchronisation cost, and the memory protection cost */
        SetEnvironmentVariableW(L"KYTY_GPU_BREAKDOWN", g_runDiagFull ? L"1" : NULL);
        SetEnvironmentVariableW(L"KYTY_SYNC_STATS", g_runDiagFull ? L"1" : NULL);
        SetEnvironmentVariableW(L"KYTY_PROTECT_STATS", (S.diag || g_runDiagFull) ? L"1" : NULL);
        /* write-protection of the game's memory in 256 KB runs instead of 64 KB (value in pages): fewer protect calls
           and page faults per frame (35-48 thousand faults per second in a fight), tried by calibration stage 4 */
        SetEnvironmentVariableW(L"KYTY_PROTECT_GAP", g_runPgap ? L"64" : NULL);
    }
    ApplyEnv();   /* the user's own variables win */
    if (S.dgpu) SetGpuPreference(runExe);

    STARTUPINFOW si = {sizeof(si)}; PROCESS_INFORMATION pi = {0};
    HANDLE log = INVALID_HANDLE_VALUE; DWORD flags = 0; g_lastLog[0] = 0;
    if (!S.console) {
        PruneLogs(logs, L"UFC5_*.log", 15); PruneLogs(logs, L"UFC5_*_motor.txt", 4);
        _snwprintf(g_lastLog, MAX_PATH, L"%s\\UFC5_%04d%02d%02d_%02d%02d%02d.log", logs, lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
        SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
        log = CreateFileW(g_lastLog, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (log != INVALID_HANDLE_VALUE) {
            char hdr[9000]; int n = WideCharToMultiByte(CP_UTF8, 0, cmd, -1, hdr + 2, sizeof(hdr) - 8, NULL, NULL);
            hdr[0] = '>'; hdr[1] = ' '; DWORD wr; if (n > 0) { hdr[n + 1] = '\r'; hdr[n + 2] = '\n'; WriteFile(log, hdr, n + 3, &wr, NULL); }
            { wchar_t cpu[64], hw[900]; CpuName(cpu, 64); int bg = BestGpu(); MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms); SYSTEM_INFO si; GetSystemInfo(&si);
              _snwprintf(hw, 899, L"> PC: %s | %lu threads | %llu GB RAM | GPU %s (%llu MB) | x3d=%d readback=%d eco=%d fastsh=%d lpages=%d stageprep=%d calib=%d cfpu=%d diag=%d place=%d pgap=%d lowsh=%d midprio=%d calib_stage=%d vblank=%d framegen=%d diagfull=%d\r\n", cpu, si.dwNumberOfProcessors,
                         (unsigned long long)((ms.ullTotalPhys + (512ull << 20)) >> 30), bg >= 0 ? g_gpus[bg].name : L"?", bg >= 0 ? (unsigned long long)(g_gpus[bg].vram >> 20) : 0ull, X3dOn(), S.readback, S.eco, S.fastsh, S.lpages, g_runStagePrep, S.calib, g_runCfpu, S.diag, g_runPlace, g_runPgap, g_runLowsh, g_runMidprio, S.calib ? S.calibStage : -1,
                         VBLANK[S.vblank], S.framegen, g_runDiagFull);
              hw[899] = 0; char h8[2048]; int m = WideCharToMultiByte(CP_UTF8, 0, hw, -1, h8, sizeof(h8), NULL, NULL); if (m > 1) WriteFile(log, h8, m - 1, &wr, NULL); }
            si.dwFlags = STARTF_USESTDHANDLES; si.hStdOutput = log; si.hStdError = log; si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            flags |= CREATE_NO_WINDOW;
        } else g_lastLog[0] = 0;
    } else flags |= CREATE_NEW_CONSOLE;
    if (S.prio) flags |= g_runMidprio ? ABOVE_NORMAL_PRIORITY_CLASS : HIGH_PRIORITY_CLASS;
    /* frame generation: Lossless Scaling is opened first, so its window comes up (and is minimized) before the game's */
    g_lsSent = FALSE; g_lsTries = 0; g_lsState = 0; g_nnotes = 0; g_notes[0] = 0;
    if (S.framegen) { g_starting = TRUE; LsEnsure(); g_starting = FALSE; }
    if (S.power) PowerHigh();

    BOOL ok = CreateProcessW(runExe, cmd, NULL, NULL, log != INVALID_HANDLE_VALUE, flags, NULL, emuDir, &si, &pi);
    DWORD err = GetLastError();
    if (log != INVALID_HANDLE_VALUE) CloseHandle(log);
    if (!ok) { PowerRestore(); SetStatus(C_BAD, L"Não foi possível iniciar o emulador (erro %lu).", err); return; }
    CloseHandle(pi.hThread);
    g_proc = pi.hProcess; g_pid = pi.dwProcessId; g_gameWnd = NULL; g_nfps = 0; g_startTick = GetTickCount();
    SetTimer(g_wnd, TIMER_FPS, 500, NULL); g_ovFps = 0; g_tick = 0; g_ovHidden = FALSE;
    RegisterHotKey(g_wnd, HOTKEY_OVERLAY, MOD_NOREPEAT, VK_F10);
    if (S.eco) {   /* never let Windows run the game in EcoQoS (E-cores, lower clocks) */
        PROCESS_POWER_THROTTLING_STATE pt = {PROCESS_POWER_THROTTLING_CURRENT_VERSION,
            PROCESS_POWER_THROTTLING_EXECUTION_SPEED | PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION, 0};
        SetProcessInformation(pi.hProcess, ProcessPowerThrottling, &pt, sizeof(pt));
    }
    /* the launcher itself steps out of the way while the game runs */
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
    CloseHandle(CreateThread(NULL, 0, WaitThread, g_proc, 0, NULL));
    SetDlgItemTextW(g_wnd, ID_PLAY, L"EM EXECUÇÃO");
    EnableWindow(Ctl(ID_PLAY), FALSE);
    SetStatus(C_GOOD, L"UFC 5 iniciado (%dx%d, %s, vblank %d Hz%s%s%s). O FPS da sessão aparece aqui quando o jogo fechar.",
              w, h, PRESENT_ARG[S.present], VBLANK[S.vblank], (X3dOn() || g_runPlace) ? L", núcleos próprios para o motor" : L"",
              !S.framegen ? L"" : g_lsState == 2 ? L", Lossless Scaling não abriu" : S.framegen == 1 ? L", Lossless Scaling liga em alguns segundos" : L", Lossless Scaling aberto",
              g_runDiagFull ? L", diagnóstico completo" : L"");
    if (S.framegen && g_lsState == 2) {
        char l8[200]; snprintf(l8, sizeof(l8), "\r\n> LSFG: o Lossless Scaling não abriu (%s)", LsInstalled() ? "instalado, mas não respondeu em 25 s" : "não encontrado na Steam");
        AppendToLog(l8);
    }
    if (!g_headless) ShowWindow(g_wnd, SW_MINIMIZE);
}

/* automatic calibration: six engine options are tried one after the other, each against the settings kept so far,
   one fight with and one without, measured on fight FPS only (menus at the 60 FPS cap do not count).
   Stage 0: console floating-point mode (KYTY_GUEST_MXCSR, the PS5's own mode: kept unless 4% slower).
   Stage 1: parallel stage preparation (KYTY_STAGE_PREP_PARALLEL).
   Stage 2: the engine's own cores for its hot threads (--cpu-placement true).
   Stage 3: memory write-protection in 256 KB runs (KYTY_PROTECT_GAP=64 pages).
   Stage 4: 2 shader translation threads instead of half the CPU (more cores for the fight itself).
   Stage 5: "above normal" priority instead of "high" (high can starve the video driver's own threads).
   Stages 1-5 are kept only with a clear win of 4%; a stage that changes nothing on this PC is skipped.
   Returns TRUE when it set the status line itself. */
#define NCALIB 6
static const wchar_t *const CALIB_NAME[NCALIB] = {L"matemática do console", L"preparação paralela", L"núcleos próprios para o motor",
                                                 L"proteção de memória em blocos maiores", L"menos threads de shader", L"prioridade acima do normal"};
static const wchar_t *const CALIB_ON[NCALIB] = {L"ligada", L"ligada", L"ligados", L"ligada", L"ligado", L"ligada"},
                    *const CALIB_OFF[NCALIB] = {L"desligada", L"desligada", L"desligados", L"desligada", L"desligado", L"desligada"};
static BOOL CalibApplies(int stage)
{
    SYSTEM_INFO si; GetSystemInfo(&si);
    if (stage == 2) return !X3dOn();                                        /* X3D CPUs already get the placement */
    if (stage == 4) return (S.fastsh || S.shthreads > 0) && S.shthreads <= 0 && (int)si.dwNumberOfProcessors / 2 - 1 > 2;
    if (stage == 5) return S.prio;
    return TRUE;
}
static void CalibSet(int stage, int v)
{
    if (stage == 0) { S.cfpu = v; ChkSet(ID_CFPU, v); }
    else if (stage == 1) { S.stageprep = v; ChkSet(ID_STAGEPREP, v); }
    else if (stage == 2) S.place = v;
    else if (stage == 3) S.pgap = v;
    else if (stage == 4) S.lowsh = v;
    else S.midprio = v;
}
static void CalibNextStage(void)
{
    S.calibStage++; S.calibArm = 0;
    while (S.calibStage < NCALIB && !CalibApplies(S.calibStage)) S.calibStage++;
    PutInt(L"calib_a_sum", 0); PutInt(L"calib_a_n", 0); PutInt(L"calib_b_sum", 0); PutInt(L"calib_b_n", 0);
    if (S.calibStage >= NCALIB) { S.calib = 0; ChkSet(ID_CALIB, 0); }
}
static BOOL Calibrate(DWORD code, float avg, int measured)
{
    if (!S.calib) return FALSE;
    if (S.calibStage < 0 || S.calibStage >= NCALIB) { S.calib = 0; ChkSet(ID_CALIB, 0); SaveSettings(); return FALSE; }
    wchar_t msg[600]; int stage = S.calibStage, cand = stage == 0 ? g_runCfpu : stage == 1 ? g_runStagePrep : stage == 2 ? g_runPlace : stage == 3 ? g_runPgap : stage == 4 ? g_runLowsh : g_runMidprio;
    if (code != 0 && code != 1 && !cand) return FALSE;   /* a crash without the option on: not the option's fault, keep the error */
    if (code != 0 && code != 1) {                         /* it crashed with the option on: drop it, go on with the next */
        CalibSet(stage, 0);
        _snwprintf(msg, 600, L"Calibração: o jogo fechou com \"%s\" ligada (código %lu). Ela ficou desligada.", CALIB_NAME[stage], code);
        CalibNextStage(); SaveSettings(); PutStr(L"calib_status", msg); SetStatus(C_BAD, L"%s", msg); return TRUE;
    }
    if (avg <= 0 || measured < 30) {
        _snwprintf(msg, 600, L"Calibração: essa partida não conta (menos de 30 segundos de luta medidos). Jogue uma luta inteira.");
        PutStr(L"calib_status", msg); SetStatus(C_SUB, L"%s", msg); return TRUE;
    }
    if (IniInt(L"calib_vblank", 60) != VBLANK[S.vblank]) {   /* the FPS cap changed since the fights of this test: start the test again */
        PutInt(L"calib_a_sum", 0); PutInt(L"calib_a_n", 0); PutInt(L"calib_b_sum", 0); PutInt(L"calib_b_n", 0); PutInt(L"calib_vblank", VBLANK[S.vblank]);
    }
    int a10 = IniInt(L"calib_a_sum", 0), an = IniInt(L"calib_a_n", 0), b10 = IniInt(L"calib_b_sum", 0), bn = IniInt(L"calib_b_n", 0);
    if (cand) { b10 += (int)(avg * 10.f + 0.5f); bn++; } else { a10 += (int)(avg * 10.f + 0.5f); an++; }
    PutInt(L"calib_a_sum", a10); PutInt(L"calib_a_n", an); PutInt(L"calib_b_sum", b10); PutInt(L"calib_b_n", bn);
    S.calibArm = !cand;
    if (an >= 1 && bn >= 1) {
        float A = a10 / 10.f / an, Bv = b10 / 10.f / bn;
        BOOL keep = stage == 0 ? Bv >= A * 0.96f : Bv >= A * 1.04f;
        CalibSet(stage, keep);
        CalibNextStage();
        if (S.calibStage >= NCALIB)
            _snwprintf(msg, 600, L"Calibração concluída (último teste: %.1f FPS sem \"%s\", %.1f com). Ficou: matemática do console %s, preparação paralela %s, núcleos próprios %s, proteção em blocos maiores %s, menos threads de shader %s, prioridade %s.",
                       A, CALIB_NAME[stage], Bv, S.cfpu ? L"ligada" : L"desligada", S.stageprep ? L"ligada" : L"desligada", S.place ? L"ligados" : L"desligados", S.pgap ? L"ligada" : L"desligada",
                       S.lowsh ? L"ligado" : L"desligado", S.midprio ? L"acima do normal" : L"alta");
        else
            _snwprintf(msg, 600, L"Calibração: teste %d de %d pronto (%.1f FPS sem \"%s\", %.1f com: ficou %s). Agora testa \"%s\" em mais 2 lutas.",
                       stage + 1, NCALIB, A, CALIB_NAME[stage], Bv, keep ? CALIB_ON[stage] : CALIB_OFF[stage], CALIB_NAME[S.calibStage]);
    } else
        _snwprintf(msg, 600, L"Calibração (teste %d de %d, \"%s\"): luta %d de 2 medida (%.1f FPS na luta, %s). A próxima: %s.",
                   stage + 1, NCALIB, CALIB_NAME[stage], an + bn, avg, cand ? CALIB_ON[stage] : CALIB_OFF[stage], S.calibArm ? CALIB_ON[stage] : CALIB_OFF[stage]);
    msg[599] = 0; SaveSettings(); PutStr(L"calib_status", msg); SetStatus(C_GOOD, L"%s", msg);
    return TRUE;
}

static void OnExitedUi(DWORD code);
static void OnExited(DWORD code)
{
    OnExitedUi(code);
    if (g_headless) { g_exitCode = code; DestroyWindow(g_wnd); }   /* Cage Launcher shows the result (last_session / calib_status) */
}
static void OnExitedUi(DWORD code)
{
    KillTimer(g_wnd, TIMER_FPS); UnregisterHotKey(g_wnd, HOTKEY_OVERLAY);
    if (g_ov) ShowWindow(g_ov, SW_HIDE);
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    CloseHandle(g_proc); g_proc = NULL; g_gameWnd = NULL; PowerRestore();
    SetDlgItemTextW(g_wnd, ID_PLAY, L"JOGAR"); EnableWindow(Ctl(ID_PLAY), TRUE);
    if (!g_headless) { ShowWindow(g_wnd, SW_RESTORE); SetForegroundWindow(g_wnd); }
    unsigned mins = (GetTickCount() - g_startTick) / 60000;
    float avg, low, best, play = 0; int playSecs = 0; wchar_t fps[260] = L"", perf[400] = L"", how[96];
    BOOL havePlay = GameplayFps(&play, &playSecs);
    _snwprintf(how, 96, L"%s, %d Hz%s, %s", PRESENT_ARG[S.present], VBLANK[S.vblank], S.framegen ? L" + Lossless Scaling" : L"", PROFILE_NAME[S.profile]); how[95] = 0;
    if (FpsStats(&avg, &low, &best)) {
        if (havePlay) _snwprintf(fps, 260, L"%u min, na luta %.0f FPS, média %.0f FPS, 1%% pior %.0f FPS, máximo %.0f FPS (%s)", mins, play, avg, low, best, how);
        else _snwprintf(fps, 260, L"%u min, média %.0f FPS, 1%% pior %.0f FPS, máximo %.0f FPS (%s)", mins, avg, low, best, how);
        fps[259] = 0; PutStr(L"last_session", fps);
        if (g_lastLog[0]) { HANDLE l = CreateFileW(g_lastLog, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (l != INVALID_HANDLE_VALUE) { char b8[700]; int m = WideCharToMultiByte(CP_UTF8, 0, fps, -1, b8 + 13, 680, NULL, NULL); memcpy(b8, "\r\n> SESSION: ", 13);
                DWORD wr; if (m > 1) WriteFile(l, b8, 13 + m - 1, &wr, NULL);
                /* the FPS of every second of the session, to see menus and fights apart when tuning */
                static char tr[8 * 36000 + 32]; int k = sprintf(tr, "\r\n> FPS/s:");
                for (int i = 0; i < g_nfps; i++) k += sprintf(tr + k, " %d", (int)(g_fps[i] + 0.5f));
                WriteFile(l, tr, k, &wr, NULL); CloseHandle(l); } }
    }
    if (g_lastLog[0] && (g_nnotes || g_runDiagFull)) {   /* frame generation notes, and where the engine's own log went */
        HANDLE l = CreateFileW(g_lastLog, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (l != INVALID_HANDLE_VALUE) { DWORD wr;
            if (g_nnotes) WriteFile(l, g_notes, g_nnotes, &wr, NULL);
            if (g_runDiagFull) { char b8[MAX_PATH * 3 + 40]; int m = sprintf(b8, "\r\n> DIAG: log do motor: ");
                                 int u = WideCharToMultiByte(CP_UTF8, 0, PathFindFileNameW(g_motorLog), -1, b8 + m, MAX_PATH * 3, NULL, NULL); if (u > 1) WriteFile(l, b8, m + u - 1, &wr, NULL); }
            CloseHandle(l); }
    }
    BOOL diagFile = g_runDiagFull && FileExists(g_motorLog);
    if (g_runDiagFull) { S.diagFull = 0; ChkSet(ID_DIAGFULL, 0); PutInt(L"diag_full_next", 0); }   /* one session only */
    if ((S.diag || g_runDiagFull) && (PerfSummary(diagFile ? g_motorLog : g_lastLog, perf, 400) || (diagFile && PerfSummary(g_lastLog, perf, 400)))) {
        PutStr(L"last_perf", perf);
        if (g_lastLog[0]) { HANDLE l = CreateFileW(g_lastLog, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (l != INVALID_HANDLE_VALUE) { char b8[1300]; int m = WideCharToMultiByte(CP_UTF8, 0, perf, -1, b8 + 10, 1280, NULL, NULL); memcpy(b8, "\r\n> PERF: ", 10);
                DWORD wr; if (m > 1) WriteFile(l, b8, 10 + m - 1, &wr, NULL); CloseHandle(l); } }
    }
    if (g_runDiagFull) {   /* say where the diagnostic went, also for Cage Launcher (it shows last_session) */
        wchar_t d[400]; _snwprintf(d, 400, L"%s%sDiagnóstico completo salvo: %s e %s (pasta bin\\_Logs). %s", fps, fps[0] ? L". " : L"",
                                    PathFindFileNameW(g_lastLog), diagFile ? PathFindFileNameW(g_motorLog) : L"(o motor não gravou o log dele)", perf);
        d[399] = 0; PutStr(L"last_session", d);
        SetStatus(code == 0 ? C_GOOD : C_BAD, L"%s", d);
        if (code == 0 || code == 1) return;
    }
    if (Calibrate(code, havePlay ? play : -1.f, playSecs)) return;
    if (perf[0] && code == 0) { SetStatus(C_GOOD, L"Sessão: %s. %s", fps, perf); return; }
    if (code == 0) { if (fps[0]) SetStatus(C_GOOD, L"Sessão: %s.", fps); else SetStatus(C_SUB, L"O jogo foi fechado."); }
    else if (g_lastLog[0]) SetStatus(C_BAD, L"O jogo fechou com o código %lu (0x%08lX). %s%sO log está em Abrir logs: %s", code, code, fps, fps[0] ? L". " : L"", PathFindFileNameW(g_lastLog));
    else SetStatus(C_BAD, L"O jogo fechou com o código %lu (0x%08lX).", code, code);
}

static void OpenLogs(void)
{
    UiToSettings(); if (!FileExists(S.emu)) { SetStatus(C_BAD, L"Escolha o motor do jogo primeiro."); return; }
    wchar_t d[MAX_PATH], l[MAX_PATH]; DirOf(d, S.emu); Join(l, d, L"_Logs"); CreateDirectoryW(l, NULL);
    if (g_lastLog[0] && FileExists(g_lastLog)) { wchar_t a[MAX_PATH + 16]; _snwprintf(a, MAX_PATH + 16, L"/select,\"%s\"", g_lastLog); ShellExecuteW(g_wnd, L"open", L"explorer.exe", a, NULL, SW_SHOWNORMAL); }
    else ShellExecuteW(g_wnd, L"open", l, NULL, NULL, SW_SHOWNORMAL);
}

static BOOL MakeShortcut(BOOL quiet)
{
    wchar_t desk[MAX_PATH], lnk[MAX_PATH], exe[MAX_PATH];
    if (!DesktopDir(desk)) return FALSE;
    Join(lnk, desk, L"EA SPORTS UFC 5.lnk"); GetModuleFileNameW(NULL, exe, MAX_PATH);
    IShellLinkW *sl = NULL; IPersistFile *pf = NULL; BOOL ok = FALSE;
    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (void **)&sl))) {
        sl->lpVtbl->SetPath(sl, exe); sl->lpVtbl->SetArguments(sl, L"--play"); sl->lpVtbl->SetWorkingDirectory(sl, g_exeDir);
        sl->lpVtbl->SetIconLocation(sl, exe, 0); sl->lpVtbl->SetDescription(sl, L"EA SPORTS UFC 5 - Cage Launcher");
        if (SUCCEEDED(sl->lpVtbl->QueryInterface(sl, &IID_IPersistFile, (void **)&pf))) { ok = SUCCEEDED(pf->lpVtbl->Save(pf, lnk, TRUE)); pf->lpVtbl->Release(pf); }
        sl->lpVtbl->Release(sl);
    }
    if (!quiet) { if (ok) SetStatus(C_GOOD, L"Atalho \"EA SPORTS UFC 5\" criado na área de trabalho: ele abre direto no jogo."); else SetStatus(C_BAD, L"Não foi possível criar o atalho."); }
    return ok;
}

/* the old layout kept the engine in emulator\ ; once bin\ exists and the old copy was never used, recycle it */
static void TidyOldLayout(void)
{
    wchar_t bin[MAX_PATH], binDir[MAX_PATH], old[MAX_PATH], oldExe[MAX_PATH];
    Join(binDir, g_exeDir, L"bin"); Join(bin, binDir, ENGINE_EXE); Join(old, g_exeDir, L"emulator"); Join(oldExe, old, L"kyty_emulator.exe");
    if (!FileExists(bin) || !FileExists(oldExe)) return;
    /* carry the shader/pipeline caches, saves and logs over to bin\, so nothing is compiled or lost again */
    static const wchar_t *keep[] = {L"_ShaderCache", L"_PipelineCache", L"_SaveData", L"_DownloadData", L"_TempData", L"_Textures", L"_Logs"};
    BOOL ok = TRUE;
    for (int i = 0; i < (int)(sizeof(keep) / sizeof(keep[0])); i++) {
        wchar_t from[MAX_PATH], to[MAX_PATH]; Join(from, old, keep[i]); Join(to, binDir, keep[i]);
        if (!DirExists(from)) continue;
        if (DirExists(to)) {   /* bin already has one (played from bin before): keep the bigger history, the old one goes to the Recycle Bin */
            wchar_t list[MAX_PATH + 2] = {0}; wcscpy(list, to);
            SHFILEOPSTRUCTW op = {0}; op.wFunc = FO_DELETE; op.pFrom = list; op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
            if (SHFileOperationW(&op) != 0) { ok = FALSE; continue; }
        }
        if (!MoveFileExW(from, to, MOVEFILE_WRITE_THROUGH)) ok = FALSE;
    }
    if (!ok) return;          /* something is in use: try again next time, the old engine keeps working meanwhile */
    wchar_t list[MAX_PATH + 2] = {0}; wcscpy(list, old);
    SHFILEOPSTRUCTW op = {0}; op.wFunc = FO_DELETE; op.pFrom = list; op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
    SHFileOperationW(&op);
    if (!_wcsicmp(S.emu, oldExe) || !FileExists(S.emu)) wcscpy(S.emu, bin);
}

/* ---------------------------------------------------------------- large pages (SeLockMemoryPrivilege) */
static BOOL HasLockPagesRight(void)
{
    HANDLE tok; BOOL has = FALSE; LUID l;
    if (!LookupPrivilegeValueW(NULL, L"SeLockMemoryPrivilege", &l)) return FALSE;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tok)) return FALSE;
    DWORD n = 0; GetTokenInformation(tok, TokenPrivileges, NULL, 0, &n);
    TOKEN_PRIVILEGES *tp = (TOKEN_PRIVILEGES *)LocalAlloc(LPTR, n ? n : 1);
    if (tp && GetTokenInformation(tok, TokenPrivileges, tp, n, &n))
        for (DWORD i = 0; i < tp->PrivilegeCount; i++) if (tp->Privileges[i].Luid.LowPart == l.LowPart && tp->Privileges[i].Luid.HighPart == l.HighPart) has = TRUE;
    LocalFree(tp); CloseHandle(tok); return has;
}
/* runs elevated (UFC5.exe --grant-lock-pages): gives the current user the "Lock pages in memory" right */
static int GrantLockPages(void)
{
    HANDLE tok; BYTE buf[256]; DWORD n = 0; int rc = 1;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tok)) return 1;
    if (GetTokenInformation(tok, TokenUser, buf, sizeof(buf), &n)) {
        PSID sid = ((TOKEN_USER *)buf)->User.Sid;
        LSA_OBJECT_ATTRIBUTES oa = {0}; LSA_HANDLE pol;
        if (LsaOpenPolicy(NULL, &oa, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &pol) == 0) {
            wchar_t name[] = L"SeLockMemoryPrivilege"; LSA_UNICODE_STRING r = {(USHORT)(wcslen(name) * 2), (USHORT)sizeof(name), name};
            if (LsaAddAccountRights(pol, sid, &r, 1) == 0) rc = 0;
            LsaClose(pol);
        }
    }
    CloseHandle(tok); return rc;
}
static void EnsureLockPages(void)
{
    if (HasLockPagesRight()) return;
    if (MessageBoxW(g_wnd, L"Large pages precisa da permissão do Windows \"Bloquear páginas na memória\" para a sua conta.\n\n"
                    L"O port vai pedir permissão de administrador uma vez para ligar isso. Depois saia e entre de novo no Windows (ou reinicie) para valer.\n\nContinuar?",
                    APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES) { S.lpages = 0; ChkSet(ID_LPAGES, 0); return; }
    wchar_t exe[MAX_PATH]; GetModuleFileNameW(NULL, exe, MAX_PATH);
    SHELLEXECUTEINFOW ei = {sizeof(ei)}; ei.fMask = SEE_MASK_NOCLOSEPROCESS; ei.hwnd = g_wnd; ei.lpVerb = L"runas"; ei.lpFile = exe; ei.lpParameters = L"--grant-lock-pages"; ei.nShow = SW_HIDE;
    if (!ShellExecuteExW(&ei) || !ei.hProcess) { SetStatus(C_BAD, L"Large pages: a permissão de administrador foi negada."); S.lpages = 0; ChkSet(ID_LPAGES, 0); return; }
    WaitForSingleObject(ei.hProcess, 30000); DWORD code = 1; GetExitCodeProcess(ei.hProcess, &code); CloseHandle(ei.hProcess);
    if (code == 0) SetStatus(C_GOOD, L"Large pages liberado. Saia e entre de novo no Windows (ou reinicie) para o jogo usar.");
    else { SetStatus(C_BAD, L"Não foi possível dar a permissão de large pages (código %lu).", code); S.lpages = 0; ChkSet(ID_LPAGES, 0); }
}

/* runs elevated (UFC5.exe --set-pagefile): fixed page file on C: of 16-32 GB, takes effect after a restart */
static int SetPagefile(void)
{
    HKEY k; int rc = 1;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management", 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &k) == ERROR_SUCCESS) {
        static const wchar_t v[] = L"C:\\pagefile.sys 16384 32768\0";
        if (RegSetValueExW(k, L"PagingFiles", 0, REG_MULTI_SZ, (const BYTE *)v, sizeof(v)) == ERROR_SUCCESS) rc = 0;
        RegCloseKey(k);
    }
    return rc;
}

/* free commit (RAM + page file) before starting: the engine can need 16-24 GB */
static void CheckCommit(void)
{
    MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms);
    unsigned long long freeGb = ms.ullAvailPageFile >> 30, totGb = ms.ullTotalPageFile >> 30;
    if (freeGb >= 20 || S.pfwarned >= 2) return;
    ULARGE_INTEGER fb = {0}; GetDiskFreeSpaceExW(L"C:\\", &fb, NULL, NULL);
    unsigned long long diskGb = fb.QuadPart >> 30;
    wchar_t m[900];
    _snwprintf(m, 900, L"Memória virtual baixa: só %llu GB livres (limite de %llu GB, RAM + arquivo de paginação). O UFC 5 usa de 16 a 24 GB; "
               L"quando falta, o Windows joga parte do jogo para o disco e o FPS cai para 1-3 (o \"1%% pior\" que aparece no FPS na tela).\n\n"
               L"Sim: o port configura sozinho um arquivo de paginação de 16 a 32 GB no C: (pede administrador uma vez; vale depois de REINICIAR o PC; o C: tem %llu GB livres).\n"
               L"Não: abre a janela do Windows para você fazer à mão (Avançado > Memória virtual > Alterar).\n"
               L"Cancelar: não perguntar mais.", freeGb, totGb, diskGb);
    int r = MessageBoxW(g_wnd, m, APP_TITLE, MB_YESNOCANCEL | MB_ICONWARNING);
    if (r == IDCANCEL) { S.pfwarned = 2; SaveSettings(); return; }
    if (r == IDNO) { ShellExecuteW(g_wnd, L"open", L"SystemPropertiesPerformance.exe", NULL, NULL, SW_SHOWNORMAL); return; }
    if (diskGb < 36) { MessageBoxW(g_wnd, L"O C: não tem espaço livre suficiente (precisa de uns 36 GB). Libere espaço ou use a janela do Windows para colocar o arquivo de paginação em outro SSD.", APP_TITLE, MB_OK | MB_ICONWARNING); return; }
    wchar_t exe[MAX_PATH]; GetModuleFileNameW(NULL, exe, MAX_PATH);
    SHELLEXECUTEINFOW ei = {sizeof(ei)}; ei.fMask = SEE_MASK_NOCLOSEPROCESS; ei.hwnd = g_wnd; ei.lpVerb = L"runas"; ei.lpFile = exe; ei.lpParameters = L"--set-pagefile"; ei.nShow = SW_HIDE;
    if (!ShellExecuteExW(&ei) || !ei.hProcess) { SetStatus(C_BAD, L"Memória virtual: a permissão de administrador foi negada."); return; }
    WaitForSingleObject(ei.hProcess, 30000); DWORD code = 1; GetExitCodeProcess(ei.hProcess, &code); CloseHandle(ei.hProcess);
    if (code == 0) { S.pfwarned = 2; SaveSettings();
        MessageBoxW(g_wnd, L"Pronto: arquivo de paginação de 16 a 32 GB configurado no C:.\n\nREINICIE o PC para valer. Até lá o jogo abre com a memória de antes.", APP_TITLE, MB_OK | MB_ICONINFORMATION); }
    else SetStatus(C_BAD, L"Não foi possível configurar a memória virtual (código %lu).", code);
}

/* ---------------------------------------------------------------- live FPS (read from the engine's window title) */
#define OV_KEY RGB(255, 0, 255)
static HFONT g_ovBig, g_ovSmall;
static LRESULT CALLBACK OverlayProc(HWND w, UINT m, WPARAM wp, LPARAM lp)
{
    if (m == WM_NCHITTEST) return HTTRANSPARENT;
    if (m == WM_PAINT) {
        PAINTSTRUCT ps; HDC dc = BeginPaint(w, &ps); RECT rc; GetClientRect(w, &rc);
        HBRUSH key = CreateSolidBrush(OV_KEY); FillRect(dc, &rc, key); DeleteObject(key);
        HBRUSH bg = CreateSolidBrush(C_BG); HPEN pen = CreatePen(PS_SOLID, 1, C_BG);
        HGDIOBJ ob = SelectObject(dc, bg), op = SelectObject(dc, pen);
        RoundRect(dc, 0, 0, rc.right, rc.bottom, SC(12), SC(12)); SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(bg); DeleteObject(pen);
        HBRUSH red = CreateSolidBrush(C_ACCENT); RECT bar = {SC(6), SC(8), SC(9), rc.bottom - SC(8)}; FillRect(dc, &bar, red); DeleteObject(red);
        SetBkMode(dc, TRANSPARENT);
        wchar_t big[32], small[96]; float avg = 0, low = 0, best = 0;
        COLORREF col = g_ovFps >= 50 ? C_GOOD : g_ovFps >= 30 ? C_WARN : C_BAD;
        if (g_ovFps > 0) _snwprintf(big, 32, L"%.0f FPS", g_ovFps); else wcscpy(big, L"-- FPS");
        if (FpsStats(&avg, &low, &best)) _snwprintf(small, 96, L"média %.0f  ·  1%% pior %.0f", avg, low); else wcscpy(small, L"UFC 5  ·  medindo...");
        SelectObject(dc, g_ovBig); SetTextColor(dc, col); RECT t1 = {SC(16), SC(4), rc.right - SC(8), SC(34)}; DrawTextW(dc, big, -1, &t1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        SelectObject(dc, g_ovSmall); SetTextColor(dc, C_TEXT); RECT t2 = {SC(16), SC(32), rc.right - SC(8), rc.bottom - SC(4)}; DrawTextW(dc, small, -1, &t2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        EndPaint(w, &ps); return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}
static void OverlayUpdate(void)
{
    BOOL want = !g_headless && S.overlay && !g_ovHidden && g_proc && g_gameWnd && IsWindow(g_gameWnd) && !IsIconic(g_gameWnd);
    if (!want) { if (g_ov) ShowWindow(g_ov, SW_HIDE); return; }
    if (!g_ov) {
        WNDCLASSW wc = {0}; wc.lpfnWndProc = OverlayProc; wc.hInstance = g_inst; wc.lpszClassName = L"CageLauncherUFC5Fps"; RegisterClassW(&wc);
        g_ovBig = CreateFontW(-SC(24), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"Bahnschrift");
        g_ovSmall = CreateFontW(-SC(12), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"Segoe UI");
        g_ov = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName, L"", WS_POPUP,
                               0, 0, SC(188), SC(56), NULL, NULL, g_inst, NULL);
        SetLayeredWindowAttributes(g_ov, OV_KEY, 225, LWA_COLORKEY | LWA_ALPHA);
    }
    RECT gr; GetWindowRect(g_gameWnd, &gr);
    POINT cp = {0, 0}; ClientToScreen(g_gameWnd, &cp);     /* the top of the picture, below any title bar */
    int x = S.overlay == 2 ? gr.right - SC(188) - SC(14) : cp.x + SC(14), y = cp.y + SC(14);
    SetWindowPos(g_ov, HWND_TOPMOST, x, y, SC(188), SC(56), SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(g_ov, NULL, FALSE);
}

static void SampleFps(void)
{
    if (!g_proc) return;
    if (!g_gameWnd || !IsWindow(g_gameWnd)) {
        g_gameWnd = TopWindowOf(g_pid);
        if (g_gameWnd) {   /* the game window gets the focus (a fullscreen game left behind another window can stay black) */
            SetForegroundWindow(g_gameWnd);
            /* give the game window the UFC 5 icon in the taskbar and Alt+Tab */
            static HICON big, sm;   /* carregados uma vez: a janela do jogo pode ser recriada e cada sessão acha a dela */
            if (!big) big = (HICON)LoadImageW(g_inst, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
            if (!sm) sm = (HICON)LoadImageW(g_inst, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
            if (big) SendMessageTimeoutW(g_gameWnd, WM_SETICON, ICON_BIG, (LPARAM)big, SMTO_ABORTIFHUNG, 200, NULL);
            if (sm) SendMessageTimeoutW(g_gameWnd, WM_SETICON, ICON_SMALL, (LPARAM)sm, SMTO_ABORTIFHUNG, 200, NULL);
        }
    }
    if (!g_gameWnd) return;
    wchar_t t[512]; DWORD_PTR r = 0;
    if (!SendMessageTimeoutW(g_gameWnd, WM_GETTEXT, 512, (LPARAM)t, SMTO_ABORTIFHUNG, 200, &r)) return;
    t[511] = 0; wchar_t *f = wcsstr(t, L"fps: "); if (!f) f = wcsstr(t, L"FPS: "); if (!f) return;
    float v = (float)wcstod(f + 5, NULL);
    if (v > 0.5f && v < 1000.f) { g_ovFps = v; if ((++g_tick & 1) == 0 && g_nfps < (int)(sizeof(g_fps) / sizeof(g_fps[0]))) { g_fps[g_nfps++] = v; LsTick(); } }
}
static int CmpF(const void *a, const void *b) { float x = *(const float *)a, y = *(const float *)b; return x < y ? -1 : x > y; }
/* average and "1% low" of the session, ignoring the first 20 s (loading and first shader compiles) */
static BOOL FpsStats(float *avg, float *low, float *best)
{
    int skip = g_nfps > 60 ? 20 : 0, n = g_nfps - skip; if (n < 5) return FALSE;
    static float tmp[36000]; memcpy(tmp, g_fps + skip, n * sizeof(float)); qsort(tmp, n, sizeof(float), CmpF);
    double sum = 0; for (int i = 0; i < n; i++) sum += tmp[i];
    *avg = (float)(sum / n); *low = tmp[n / 100]; *best = tmp[n - 1]; return TRUE;
}
/* frame rate of the fight itself: the seconds below 57 FPS (menus run at the 60 FPS cap), and of those only the ones
   at 75% or more of the session's fast end (90th percentile): the fighter walkouts/intros and replays run at about
   half the fight's FPS (15-18 against 30-34 on a Ryzen 5 5500) and would make every comparison noisy. Needs 30 s. */
static BOOL GameplayFps(float *fps, int *seconds)
{
    int skip = g_nfps > 60 ? 20 : 0, n = 0; static float g[36000];
    for (int i = skip; i < g_nfps; i++) if (g_fps[i] > 3.f && g_fps[i] < 57.f) g[n++] = g_fps[i];
    *seconds = 0; if (n < 10) return FALSE;
    qsort(g, n, sizeof(float), CmpF);
    float cut = g[(n * 9) / 10] * 0.75f; double sum = 0; int m = 0;
    for (int i = 0; i < n; i++) if (g[i] >= cut) { sum += g[i]; m++; }
    *seconds = m; if (m < 30) return FALSE;
    *fps = (float)(sum / m); return TRUE;
}

/* reads the "GPU breakdown:" lines the engine printed (KYTY_GPU_BREAKDOWN) and says where the GPU command thread's
   time went: waiting for the game (idle), waiting for the video card (blocked) or working (process) */
static int CmpD(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return x < y ? -1 : x > y; }
static BOOL PerfSummary(const wchar_t *path, wchar_t *out, int cap)
{
    out[0] = 0; if (!path || !path[0]) return FALSE;
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return FALSE;
    LARGE_INTEGER sz; GetFileSizeEx(f, &sz); DWORD n = sz.QuadPart > (64 << 20) ? (64 << 20) : (DWORD)sz.QuadPart;
    char *buf = (char *)malloc((size_t)n + 1); DWORD got = 0;
    if (!buf) { CloseHandle(f); return FALSE; }
    if (n && sz.QuadPart > n) { LARGE_INTEGER at; at.QuadPart = sz.QuadPart - n; SetFilePointerEx(f, at, NULL, FILE_BEGIN); }
    ReadFile(f, buf, n, &got, NULL); CloseHandle(f); buf[got] = 0;
    /* per line: interval, idle, blocked, waits/s, process, draw packets; t = end of the interval since the first line */
    enum { NB = 4096 }; static double L[NB][6], T[NB]; int k = 0; double t = 0;
    for (char *p = buf; (p = strstr(p, "GPU breakdown: ")) != NULL && k < NB; p += 15) {
        float secs, i, b, w, c, pr, sub, loop, inner, draw = -1;
        int m = sscanf(p, "GPU breakdown: %f s: idle %f%% blocked %f%% (%f waits/s) commands %f%% process %f%% = submission %f%% + loop %f%% + inner commands %f%% + packets: draw %f%%",
                       &secs, &i, &b, &w, &c, &pr, &sub, &loop, &inner, &draw);
        if (m < 6) continue;
        t += secs; T[k] = t; L[k][0] = secs; L[k][1] = i; L[k][2] = b; L[k][3] = w; L[k][4] = pr; L[k][5] = m >= 10 ? draw : -1; k++;
    }
    free(buf);
    if (k < 2) return FALSE;
    /* keep the intervals of the fight itself (FPS band of GameplayFps), lined up with the per-second FPS samples */
    static int use[NB]; int nu = 0;
    { static float g[36000]; int ng = 0; for (int i = 0; i < g_nfps; i++) if (g_fps[i] > 3.f && g_fps[i] < 57.f) g[ng++] = g_fps[i];
      if (ng >= 10) { qsort(g, ng, sizeof(float), CmpF); float cut = g[(ng * 9) / 10] * 0.75f;
          for (int j = 0; j < k; j++) { int a0 = (int)(T[j] - L[j][0]), a1 = (int)T[j]; double sum = 0; int c = 0;
              for (int q = a0 < 0 ? 0 : a0; q < a1 && q < g_nfps; q++) { sum += g_fps[q]; c++; }
              if (c && sum / c >= cut && sum / c < 57.0) use[nu++] = j; } } }
    BOOL fight = nu >= 3; if (!fight) { nu = 0; for (int j = 0; j < k; j++) use[nu++] = j; }
    static double v[NB]; double med[6];
    for (int c = 1; c < 6; c++) { int nv = 0; for (int j = 0; j < nu; j++) if (L[use[j]][c] >= 0) v[nv++] = L[use[j]][c];
                                  if (!nv) { med[c] = -1; continue; } qsort(v, nv, sizeof(double), CmpD); med[c] = v[nv / 2]; }
    double mi = med[1], mb = med[2], mw = med[3], mp = med[4], md = med[5];
    const wchar_t *why = mb >= 35.0 ? L"a placa de vídeo / sincronização com a GPU" :
                         mp >= 70.0 ? L"a thread de comandos do emulador (processador)" :
                         mi >= 40.0 ? L"a lógica do próprio jogo (processador)" : L"misto (processador e GPU)";
    wchar_t dr[48] = L""; if (md >= 0) _snwprintf(dr, 48, L" (desenhos %.0f%%)", md);
    _snwprintf(out, cap, L"Diagnóstico (%d medições%s): thread da GPU %.0f%% trabalhando%s, %.0f%% esperando a placa (%.0f esperas/s), %.0f%% esperando o jogo. Gargalo: %s.",
               nu, fight ? L" na luta" : L"", mp, dr, mb, mw, mi, why);
    out[cap - 1] = 0; return TRUE;
}

static void ClearCache(void)
{
    UiToSettings(); if (!FileExists(S.emu)) { SetStatus(C_BAD, L"Escolha o motor do jogo primeiro."); return; }
    if (g_proc) { SetStatus(C_BAD, L"Feche o jogo antes de limpar o cache."); return; }
    if (MessageBoxW(g_wnd, L"Mandar o cache de shaders e pipelines do UFC 5 para a Lixeira?\n\nSó vale a pena se aparecerem gráficos quebrados depois de atualizar o emulador ou o driver. As primeiras lutas depois disso vão engasgar enquanto tudo recompila.",
                    APP_TITLE, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return;
    wchar_t d[MAX_PATH], a[MAX_PATH], b[MAX_PATH]; DirOf(d, S.emu);
    Join(a, d, L"_ShaderCache\\" TITLE_ID); Join(b, d, L"_PipelineCache\\" TITLE_ID L".bin");
    wchar_t list[2 * MAX_PATH + 4] = {0}; size_t n = 0;
    if (DirExists(a)) { wcscpy(list, a); n = wcslen(a) + 1; }
    if (FileExists(b)) { wcscpy(list + n, b); n += wcslen(b) + 1; }
    list[n] = 0;
    if (!n) { SetStatus(C_SUB, L"Não há cache do UFC 5 para limpar."); return; }
    SHFILEOPSTRUCTW op = {0}; op.hwnd = g_wnd; op.wFunc = FO_DELETE; op.pFrom = list; op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
    if (SHFileOperationW(&op) == 0 && !op.fAnyOperationsAborted) SetStatus(C_GOOD, L"Cache do UFC 5 enviado para a Lixeira.");
    else SetStatus(C_BAD, L"Não foi possível limpar o cache (algum arquivo está em uso?).");
}

/* ---------------------------------------------------------------- drawing */
static void DrawButton(DRAWITEMSTRUCT *d)
{   /* JOGAR: vermelho sólido (hover mais claro, pressionado mais escuro, desabilitado cinza escuro);
       os outros: contorno de 1px e texto branco, borda branca em hover ou com o foco */
    BOOL play = d->CtlID == ID_PLAY, down = d->itemState & ODS_SELECTED, dis = d->itemState & ODS_DISABLED, focus = d->itemState & ODS_FOCUS;
    BOOL hot = g_hotCtl == d->hwndItem; RECT r = d->rcItem;
    FillC(d->hDC, r, C_BG);
    if (play) FillC(d->hDC, r, dis ? C_OFF : down ? C_ACCENT2 : hot ? C_ACCHOT : C_ACCENT);
    else { if (down) FillC(d->hDC, r, C_ROWSEL); FrameC(d->hDC, r, hot || focus ? C_TEXT : C_LINEHOT); }
    wchar_t t[64]; GetWindowTextW(d->hwndItem, t, 64); if (play) CharUpperW(t);
    SetBkMode(d->hDC, TRANSPARENT); SetTextColor(d->hDC, dis ? C_SUB : RGB(255, 255, 255));
    HGDIOBJ of = SelectObject(d->hDC, play ? g_fontPlay : g_fontBtn); SetTextCharacterExtra(d->hDC, play ? SC(3) : 0);
    DrawTextW(d->hDC, t, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SetTextCharacterExtra(d->hDC, 0); SelectObject(d->hDC, of);
}
/* item da lista aberta do dropdown: selecionado em cinza com barra vermelha, sem o azul do sistema */
static void DrawComboItem(DRAWITEMSTRUCT *d)
{
    if ((d->itemState & ODS_COMBOBOXEDIT) || d->itemID == (UINT)-1) { InvalidateRect(d->hwndItem, NULL, FALSE); return; }   /* a face é de ComboProc */
    BOOL sel = (d->itemState & ODS_SELECTED) != 0; RECT r = d->rcItem;
    FillC(d->hDC, r, sel ? C_ROWSEL : C_EDIT);
    if (sel) { RECT b = {r.left, r.top, r.left + SC(2), r.bottom}; FillC(d->hDC, b, C_ACCENT); }
    wchar_t t[320] = L""; if (SendMessageW(d->hwndItem, CB_GETLBTEXTLEN, d->itemID, 0) < 320) SendMessageW(d->hwndItem, CB_GETLBTEXT, d->itemID, (LPARAM)t);
    HGDIOBJ of = SelectObject(d->hDC, (HFONT)SendMessageW(d->hwndItem, WM_GETFONT, 0, 0)); SetBkMode(d->hDC, TRANSPARENT); SetTextColor(d->hDC, C_TEXT);
    RECT tr = {r.left + SC(8), r.top, r.right - SC(4), r.bottom}; DrawTextW(d->hDC, t, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    SelectObject(d->hDC, of);
}
/* checkbox desenhada como switch: ligado = trilho vermelho e bolinha branca, desligado = trilho cinza escuro e bolinha cinza */
static LRESULT DrawSwitch(NMCUSTOMDRAW *cd)
{
    if (cd->dwDrawStage != CDDS_PREPAINT) return CDRF_DODEFAULT;
    HWND h = cd->hdr.hwndFrom; HDC dc = cd->hdc; RECT r = cd->rc;
    BOOL on = SendMessageW(h, BM_GETCHECK, 0, 0) == BST_CHECKED, dis = !IsWindowEnabled(h), hot = (cd->uItemState & CDIS_HOT) != 0;
    FillC(dc, r, C_BG);
    int tw = SC(26), th = SC(14), ty = (r.top + r.bottom - th) / 2;
    RECT tr = {r.left, ty, r.left + tw, ty + th};
    if (on && !dis) FillC(dc, tr, hot ? C_ACCHOT : C_ACCENT);
    else { FillC(dc, tr, C_OFF); FrameC(dc, tr, hot && !dis ? C_LINEHOT : C_LINE); }
    int k = SC(8), kx = on ? tr.right - SC(3) - k : tr.left + SC(3), ky = (tr.top + tr.bottom - k) / 2;
    RECT kr = {kx, ky, kx + k, ky + k}; FillC(dc, kr, dis ? C_LINEHOT : on ? RGB(255, 255, 255) : RGB(100, 100, 108));
    wchar_t t[128]; GetWindowTextW(h, t, 128);
    HGDIOBJ of = SelectObject(dc, (HFONT)SendMessageW(h, WM_GETFONT, 0, 0)); SetBkMode(dc, TRANSPARENT); SetTextColor(dc, dis ? C_SUB : C_TEXT);
    RECT xr = {tr.right + SC(8), r.top, r.right, r.bottom}; DrawTextW(dc, t, -1, &xr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    if ((cd->uItemState & CDIS_FOCUS) && !(SendMessageW(h, WM_QUERYUISTATE, 0, 0) & UISF_HIDEFOCUS)) {   /* foco pelo teclado */
        RECT fr = {tr.left - SC(2), tr.top - SC(2), tr.right + SC(2), tr.bottom + SC(2)}; FrameC(dc, fr, C_TEXT);
    }
    SelectObject(dc, of);
    return CDRF_SKIPDEFAULT;
}
/* ---------------------------------------------------------------- window procedure */
static LRESULT CALLBACK WndProc(HWND w, UINT m, WPARAM wp, LPARAM lp)
{
    switch (m) {
    case WM_CREATE: g_wnd = w; return 0;
    case WM_ERASEBKGND: { RECT r; GetClientRect(w, &r); FillRect((HDC)wp, &r, g_brBg); return 1; }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(w, &ps);
        if (g_header) { HDC m2 = CreateCompatibleDC(dc); HGDIOBJ o = SelectObject(m2, g_header); BitBlt(dc, 0, 0, g_headerW, g_headerH, m2, 0, 0, SRCCOPY); SelectObject(m2, o); DeleteDC(m2); }
        PaintGroups(dc);
        EndPaint(w, &ps); return 0;
    }
    case WM_CTLCOLORSTATIC: case WM_CTLCOLORBTN: {
        HDC dc = (HDC)wp; SetBkColor(dc, C_BG);
        SetTextColor(dc, (HWND)lp == g_status ? g_statusColor : C_SUB);   /* rótulos em cinza; as checkboxes se desenham em DrawSwitch */
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLOREDIT: case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wp; SetBkColor(dc, C_EDIT); SetTextColor(dc, C_TEXT); return (LRESULT)g_brEdit;
    }
    case WM_DRAWITEM: if (((DRAWITEMSTRUCT *)lp)->CtlType == ODT_COMBOBOX) DrawComboItem((DRAWITEMSTRUCT *)lp); else DrawButton((DRAWITEMSTRUCT *)lp); return TRUE;
    case WM_MEASUREITEM: {   /* altura da face e dos itens dos dropdowns (owner-draw) */
        MEASUREITEMSTRUCT *mi = (MEASUREITEMSTRUCT *)lp;
        if (mi->CtlType == ODT_COMBOBOX) { mi->itemHeight = mi->itemID == (UINT)-1 ? SC(17) : SC(20); return TRUE; }
        break;
    }
    case WM_NOTIFY: {   /* checkboxes desenhadas como switch */
        NMHDR *nh = (NMHDR *)lp; wchar_t cls[16];
        if (nh->code == NM_CUSTOMDRAW && nh->hwndFrom != g_tip && GetClassNameW(nh->hwndFrom, cls, 16) && !_wcsicmp(cls, L"Button")
            && (GetWindowLongPtrW(nh->hwndFrom, GWL_STYLE) & BS_TYPEMASK) == BS_AUTOCHECKBOX) return DrawSwitch((NMCUSTOMDRAW *)lp);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp), code = HIWORD(wp);
        if (g_applying) return 0;
        switch (id) {
        case ID_PLAY: Play(); return 0;
        case ID_AUTO: UiToSettings(); AutoConfig(TRUE); SettingsToUi(); SaveSettings(); return 0;
        case ID_KYTY: MakeShortcut(FALSE); return 0;
        case ID_LPAGES: if (code == BN_CLICKED) { UiToSettings(); if (S.lpages) EnsureLockPages(); SaveSettings(); } return 0;
        case ID_CALIB:
            if (code == BN_CLICKED) { UiToSettings();
                if (S.calib) { S.calibArm = 0; S.calibStage = 0; PutInt(L"calib_a_sum", 0); PutInt(L"calib_a_n", 0); PutInt(L"calib_b_sum", 0); PutInt(L"calib_b_n", 0);
                               SetStatus(C_GOOD, L"Calibração ligada: jogue até 12 lutas (pelo menos 30 segundos de luta cada). O port testa 6 ajustes do motor, um de cada vez (uma luta com, outra sem), e fica com o que der mais FPS."); }
                SaveSettings(); }
            return 0;
        case ID_LOGS: OpenLogs(); return 0;
        case ID_CLEAR: ClearCache(); return 0;
        case ID_GAME_BR: {
            wchar_t p[MAX_PATH]; GetDlgItemTextW(w, ID_GAME, p, MAX_PATH);
            if (PickFolder(p)) {
                if (!ResolveGameDir(p)) SetStatus(C_BAD, L"Essa pasta não tem o jogo (falta eboot.bin ou sce_sys\\param.json).");
                else { SetDlgItemTextW(w, ID_GAME, p); UiToSettings(); SaveSettings(); SetStatus(C_GOOD, L"Jogo encontrado: %s", p); RefreshGameInfo(); }
            }
            return 0;
        }
        case ID_EMU_BR: {
            wchar_t p[MAX_PATH]; GetDlgItemTextW(w, ID_EMU, p, MAX_PATH);
            if (PickFile(p, L"Motor do jogo (*.exe)\0*.exe\0", L"Escolha o motor do jogo (UFC5-Game.exe ou kyty_emulator.exe)")) { SetDlgItemTextW(w, ID_EMU, p); UiToSettings(); SaveSettings(); }
            return 0;
        }
        case ID_PATCH_BR: {
            wchar_t p[MAX_PATH]; GetDlgItemTextW(w, ID_PATCH, p, MAX_PATH);
            if (PickFile(p, L"Cheats ETAHen (*.json)\0*.json\0Todos os arquivos\0*.*\0", L"Escolha o arquivo de cheats")) { SetDlgItemTextW(w, ID_PATCH, p); UiToSettings(); SaveSettings(); }
            return 0;
        }
        case ID_PROFILE:
            if (code == CBN_SELCHANGE) { UiToSettings(); ApplyProfile(S.profile); SettingsToUi(); SaveSettings(); }
            return 0;
        case ID_PRESENT: case ID_VBLANK: case ID_SHADER:
            if (code == CBN_SELCHANGE) { UiToSettings(); S.profile = 4; CbSet(ID_PROFILE, 4); SaveSettings();
                if (id == ID_VBLANK && S.vblank > 0)
                    SetStatus(C_SUB, L"%d Hz: os quadros saem em passos de %.1f ms e os menus passam de 60 FPS. Na primeira luta, olhe o relógio do round: se andar mais rápido que o normal, volte para 60 Hz.",
                              VBLANK[S.vblank], 1000.0 / VBLANK[S.vblank]); }
            return 0;
        case ID_FRAMEGEN:
            if (code == CBN_SELCHANGE) { UiToSettings(); SaveSettings();
                if (S.framegen && !LsInstalled()) SetStatus(C_BAD, L"Não achei o Lossless Scaling da Steam neste PC. Instale/abra ele uma vez pela Steam; o port abre sozinho depois.");
                else if (S.framegen) SetStatus(C_GOOD, L"Geração de quadros: no Lossless Scaling escolha Frame Generation LSFG (a mais nova), modo Adaptive com alvo 144 (ou 120) ou Fixed X4. %s",
                                               S.framegen == 1 ? L"O port abre o LS e aperta Ctrl+Alt+S quando o jogo estiver na tela (deixe o Auto Scale do LS desligado)." : L"O port só abre o LS: o seu perfil com Auto Scale liga."); }
            return 0;
        case ID_DIAGFULL:
            if (code == BN_CLICKED) { UiToSettings(); SaveSettings();
                if (S.diagFull) SetStatus(C_GOOD, L"Diagnóstico completo na próxima partida: jogue pelo menos 1 minuto de luta e depois mande os 2 arquivos novos da pasta _Logs (botão Abrir logs)."); }
            return 0;
        case ID_FULL: case ID_PRIO: case ID_DGPU: case ID_POWER: case ID_CURSOR: case ID_TROPHY: case ID_BINDLESS: case ID_TESS:
            if (code == BN_CLICKED) { UiToSettings(); S.profile = 4; CbSet(ID_PROFILE, 4); SaveSettings(); }
            return 0;
        default:
            if (code == CBN_SELCHANGE || code == BN_CLICKED) { UiToSettings(); SaveSettings(); }
            return 0;
        }
    }
    case WM_APP_EXITED: OnExited((DWORD)wp); return 0;
    case WM_TIMER: if (wp == TIMER_FPS) { SampleFps(); OverlayUpdate(); } return 0;
    case WM_HOTKEY: if (wp == HOTKEY_OVERLAY) { g_ovHidden = !g_ovHidden; OverlayUpdate(); } return 0;
    case WM_CLOSE:
        if (g_proc && MessageBoxW(w, L"O jogo ainda está aberto. Fechar só o launcher? (o jogo continua rodando)", APP_TITLE, MB_YESNO | MB_ICONQUESTION) != IDYES) return 0;
        UiToSettings(); SaveSettings(); PowerRestore(); DestroyWindow(w); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmdline, int show)
{
    (void)prev;
    g_inst = inst;
    if (cmdline && wcsstr(cmdline, L"--grant-lock-pages")) return GrantLockPages();
    if (cmdline && wcsstr(cmdline, L"--set-pagefile")) return SetPagefile();
    if (cmdline && wcsstr(cmdline, L"--auto-config")) {
        GetModuleFileNameW(NULL, g_exeDir, MAX_PATH); wchar_t *e = wcsrchr(g_exeDir, L'\\'); if (e) *e = 0;
        Join(g_ini, g_exeDir, L"UFC5.ini");
        return HeadlessAutoConfig(wcsstr(cmdline, L"--force") != NULL);
    }
    g_headless = cmdline && wcsstr(cmdline, L"--headless") != NULL;
    HANDLE single = CreateMutexW(NULL, TRUE, L"CAGE_LAUNCHER_UFC5");
    if (GetLastError() == ERROR_ALREADY_EXISTS) { HWND o = FindWindowW(L"CageLauncherUFC5", NULL); if (o && !g_headless) { ShowWindow(o, SW_RESTORE); SetForegroundWindow(o); } return 0; }
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX ic = {sizeof(ic), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES}; InitCommonControlsEx(&ic);
    GdiplusStartupInput gi = {1, NULL, FALSE, FALSE}; GdiplusStartup(&g_gdip, &gi, NULL);

    GetModuleFileNameW(NULL, g_exeDir, MAX_PATH); wchar_t *s = wcsrchr(g_exeDir, L'\\'); if (s) *s = 0;
    Join(g_ini, g_exeDir, L"UFC5.ini");

    HDC sdc = GetDC(NULL); g_dpi = GetDeviceCaps(sdc, LOGPIXELSY); ReleaseDC(NULL, sdc);
    g_font = CreateFontW(-SC(13), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_fontBold = CreateFontW(-SC(13), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Bahnschrift");
    g_fontTitle = CreateFontW(-SC(30), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Bahnschrift");
    g_fontPlay = CreateFontW(-SC(20), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Bahnschrift");
    g_fontLabel = CreateFontW(-SC(13), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Bahnschrift");
    g_fontBtn = CreateFontW(-SC(13), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Bahnschrift SemiCondensed");
    g_brBg = CreateSolidBrush(C_BG); g_brEdit = CreateSolidBrush(C_EDIT);

    LoadSettings();
    TidyOldLayout();
    DetectGpus();
    FindDefaults();
    if (S.gpu >= g_ngpus) S.gpu = -1;
    if (S.lang < 0 || S.lang >= NLANGS) S.lang = 0;
    if (S.res < 0 || S.res >= NRES) S.res = 0;
    if (S.present < 0 || S.present > 2) S.present = 0;
    if (S.vblank < 0 || S.vblank >= NVBLANK) S.vblank = 0;
    if (S.framegen < 0 || S.framegen > 2) S.framegen = 0;
    if (S.shader < 0 || S.shader > 2) S.shader = 0;
    if (S.profile < 0 || S.profile > 4) S.profile = 0;
    if (S.vol < 0 || S.vol > 100) S.vol = 50;
    if (S.vib < 0 || S.vib > 100) S.vib = 100;
    if (S.x3d < 0 || S.x3d > 2) S.x3d = 0;
    if (S.overlay < 0 || S.overlay > 2) S.overlay = 1;
    BOOL first = !S.configured;
    if (first) AutoConfig(FALSE);
    { wchar_t sig[400]; HwSig(sig, 400); PutStr(L"hw_sig", sig); }

    WNDCLASSEXW wc = {sizeof(wc)}; wc.lpfnWndProc = WndProc; wc.hInstance = inst; wc.lpszClassName = L"CageLauncherUFC5";
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW); wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(1)); wc.hbrBackground = g_brBg;
    RegisterClassExW(&wc);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r = {0, 0, SC(860), SC(812)}; AdjustWindowRect(&r, style, FALSE);
    int ww = r.right - r.left, wh = r.bottom - r.top;
    RECT wa; SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
    HWND w = CreateWindowExW(0, wc.lpszClassName, APP_TITLE, style, wa.left + ((wa.right - wa.left) - ww) / 2, wa.top + max(0, ((wa.bottom - wa.top) - wh) / 2), ww, wh, NULL, NULL, inst, NULL);
    BOOL dark = TRUE; DwmSetWindowAttribute(w, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));
    COLORREF cap = C_BG; DwmSetWindowAttribute(w, 35 /* DWMWA_CAPTION_COLOR */, &cap, sizeof(cap));
    BuildUi();
    SettingsToUi();
    RefreshGameInfo();
    if (first) {
        SaveSettings();
        if (!g_headless) MakeShortcut(TRUE);
        wchar_t cpu[64]; CpuName(cpu, 64); int bg = BestGpu();
        SetStatus(C_GOOD, L"Primeira vez: configurado para %s + %s (%s, perfil %s) e atalho EA SPORTS UFC 5 criado na área de trabalho.", bg >= 0 ? g_gpus[bg].name : L"sua GPU", cpu, TIER_NAME[g_tier], PROFILE_NAME[S.profile]);
    } else { wchar_t last[200]; IniStr(L"last_session", last, 200, L"");
             if (last[0]) SetStatus(C_SUB, L"Pronto para jogar. Última sessão: %s.", last); else SetStatus(C_SUB, L"Pronto para jogar."); }
    if (!S.game[0]) SetStatus(C_BAD, L"Não achei o UFC 5 na área de trabalho. Clique em Procurar... e escolha a pasta do dump.");
    else if (!S.emu[0]) SetStatus(C_BAD, L"Não achei o motor do jogo (bin\\UFC5-Game.exe). Clique em Procurar... ao lado de Motor do jogo.");

    /* --play: start straight away (for shortcuts / Cage Launcher) */
    BOOL autoplay = cmdline && wcsstr(cmdline, L"--play");
    if (g_headless) {
        AllowSetForegroundWindow(ASFW_ANY);
        Play();
        if (!g_proc) {   /* it could not start: say why, Cage Launcher shows the code */
            wchar_t t[1024]; GetWindowTextW(g_status, t, 1024);
            MessageBoxW(NULL, t[0] ? t : L"Não foi possível iniciar o UFC 5.", APP_TITLE, MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
            return 3;
        }
    } else {
        ShowWindow(w, show); UpdateWindow(w);
        if (autoplay) Play();
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) { if (!IsDialogMessageW(w, &msg)) { TranslateMessage(&msg); DispatchMessageW(&msg); } }
    CloseHandle(single);
    return (int)g_exitCode;
}
