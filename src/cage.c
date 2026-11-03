/*
 * CAGE LAUNCHER 2.7
 * One launcher for the PC ports of EA SPORTS UFC, UFC 2, UFC 3, UFC 4 and UFC 5.
 * Game list on the left, GAME / VIDEO / GAMEPLAY options and PLAY on the right.
 *  - UFC 1-4: options are written to %LOCALAPPDATA%\UFCnNative\sacredfire.ini (the ports' own file),
 *    the port is started as games\UFCn\UFCn.exe --play.
 *  - UFC 5: options are written to games\UFC5\UFC5.ini, the port is started as UFC5.exe --play.
 * The whole window is drawn by hand (no themed controls) except the two text fields.
 * Win32 + GDI/GDI+, no dependencies beyond Windows itself. GPL-2.0-or-later.
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
#include <dwmapi.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <tlhelp32.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define APP_VER L"2.7"
#define WM_APP_EXITED (WM_APP + 1)
#define WM_APP_FROZEN (WM_APP + 2)
#define WM_APP_AUTOCFG (WM_APP + 3)

/* ---------------------------------------------------------------- GDI+ flat API (subset) */
typedef struct { UINT32 GdiplusVersion; void *DebugEventCallback; BOOL SuppressBackgroundThread; BOOL SuppressExternalCodecs; } GdiplusStartupInput;
typedef void GpImage; typedef void GpGraphics; typedef void GpBrush; typedef void GpPen; typedef void GpPath;
int WINAPI GdiplusStartup(ULONG_PTR *, const GdiplusStartupInput *, void *);
int WINAPI GdipCreateBitmapFromStream(IStream *, GpImage **);
int WINAPI GdipLoadImageFromFile(const WCHAR *, GpImage **);
int WINAPI GdipGetImageWidth(GpImage *, UINT *);
int WINAPI GdipGetImageHeight(GpImage *, UINT *);
int WINAPI GdipCreateFromHDC(HDC, GpGraphics **);
int WINAPI GdipSetInterpolationMode(GpGraphics *, int);
int WINAPI GdipSetSmoothingMode(GpGraphics *, int);
int WINAPI GdipSetPixelOffsetMode(GpGraphics *, int);
int WINAPI GdipDrawImageRectRectI(GpGraphics *, GpImage *, INT, INT, INT, INT, INT, INT, INT, INT, int, void *, void *, void *);
int WINAPI GdipDrawImageRectI(GpGraphics *, GpImage *, INT, INT, INT, INT);
int WINAPI GdipCreateSolidFill(UINT32, GpBrush **);
int WINAPI GdipCreateLineBrushI(const POINT *, const POINT *, UINT32, UINT32, int, GpBrush **);
int WINAPI GdipFillRectangleI(GpGraphics *, GpBrush *, INT, INT, INT, INT);
int WINAPI GdipDeleteBrush(GpBrush *);
int WINAPI GdipCreatePen1(UINT32, float, int, GpPen **);
int WINAPI GdipCreatePen2(GpBrush *, float, int, GpPen **);
int WINAPI GdipDeletePen(GpPen *);
int WINAPI GdipCreatePath(int, GpPath **);
int WINAPI GdipDeletePath(GpPath *);
int WINAPI GdipAddPathArcI(GpPath *, INT, INT, INT, INT, float, float);
int WINAPI GdipAddPathLineI(GpPath *, INT, INT, INT, INT);
int WINAPI GdipClosePathFigure(GpPath *);
int WINAPI GdipFillPath(GpGraphics *, GpBrush *, GpPath *);
int WINAPI GdipDrawPath(GpGraphics *, GpPen *, GpPath *);
int WINAPI GdipFillEllipseI(GpGraphics *, GpBrush *, INT, INT, INT, INT);
int WINAPI GdipDrawLineI(GpGraphics *, GpPen *, INT, INT, INT, INT);
int WINAPI GdipDeleteGraphics(GpGraphics *);
int WINAPI GdipDisposeImage(GpImage *);

/* ---------------------------------------------------------------- look: preto, branco e vermelho UFC */
#define A(c, a) (((UINT32)(a) << 24) | ((UINT32)GetRValue(c) << 16) | ((UINT32)GetGValue(c) << 8) | (UINT32)GetBValue(c))
#define C_BG      RGB(10, 10, 12)     /* fundo da janela */
#define C_SIDE    RGB(17, 17, 19)     /* superfície: sidebar */
#define C_FIELD   RGB(17, 17, 19)     /* superfície: campos e dropdowns */
#define C_FIELDB  RGB(38, 38, 43)     /* borda de 1px e divisores */
#define C_FIELDH  RGB(58, 58, 66)     /* borda em hover */
#define C_ROWHOT  RGB(23, 23, 26)     /* linha de jogo em hover */
#define C_ROWSEL  RGB(28, 28, 32)     /* linha de jogo selecionada */
#define C_OFF     RGB(40, 40, 46)     /* trilho do switch desligado, JOGAR desabilitado */
#define C_TEXT    RGB(242, 242, 242)
#define C_SUB     RGB(138, 138, 147)  /* texto secundário */
#define C_ACCENT  RGB(210, 10, 10)    /* vermelho UFC */
#define C_ACCHOT  RGB(229, 37, 42)    /* acento em hover */
#define C_GOOD    RGB(76, 195, 138)   /* bom / rodando */
#define C_ORANGE  RGB(240, 160, 32)   /* aviso (âmbar) */
#define C_BAD     RGB(255, 107, 94)   /* erro: coral, diferente do acento */

#define BASE_W 1180
#define BASE_H 704
#define SIDE_W 322
#define X0 356
#define X1 1148

/* ---------------------------------------------------------------- strings (PT-BR / EN) */
enum { S_READY, S_PICK, S_NOPORT, S_RUNNING, S_GAME, S_VIDEO, S_GAMEPLAY, S_BROWSE, S_PORT, S_CLICKCHANGE, S_ADV, S_LOGS, S_PLAY, S_STOP,
       S_ABOUT, S_RES, S_WIN, S_FPS, S_PRESENT, S_UPSCALE, S_SHARP, S_LANG, S_VOLUME, S_PLAYER, S_FULL, S_NEO, S_SKIP, S_LOWLAT, S_ULTRA,
       S_NATIVE, S_PS5PORT, S_PROFILE, S_SHADERS, S_OVERLAY, S_X3D, S_VIB, S_FASTSH, S_ECO, S_PRIO, S_LPAGES, S_BINDLESS, S_PICKGAME,
       S_PICKPORT, S_NOTFOUND, S_STARTING, S_CLOSED, S_CLOSEDERR, S_CANTSTART, S_ULTRAON, S_CHOOSEFIRST, S_SESSION, S_NOLOGS, S_SHTHREADS, S_SPKVOL, S_CALIB, S_STAGEPREP, S_FRAMEGEN, S_RES5, S_COUNT };
static const wchar_t *TXT[2][S_COUNT] = {
  { L"Pronto para jogar", L"Escolha a pasta do jogo", L"Port não encontrado", L"Em execução", L"JOGO", L"VÍDEO", L"JOGABILIDADE", L"Procurar",
    L"Port", L"clique para trocar", L"Configurações avançadas", L"Logs", L"JOGAR", L"PARAR",
    L"Sobre", L"Resolução do jogo", L"Tamanho da janela", L"Limite de FPS", L"Modo de apresentação", L"Upscaling", L"Nitidez",
    L"Idioma do jogo", L"Volume", L"Nome do jogador", L"Tela cheia", L"Modo PS4 Pro", L"Pular vídeos de introdução", L"Baixa latência (co-op)", L"Ultra desempenho (PC fraco)",
    L"Port nativo para PC", L"Port para PC (PS5)", L"Perfil de desempenho", L"Shaders", L"FPS na tela", L"Modo X3D (Ryzen)", L"Vibração do controle",
    L"Compilar shaders rápido", L"Sem economia de energia", L"Prioridade alta", L"Large pages", L"Texturas bindless (exp.)", L"Escolha a pasta do jogo %s",
    L"Escolha a pasta do port do %s (a que tem o %s)", L"%s não foi encontrado nessa pasta.", L"Iniciando o %s...", L"O jogo foi fechado.",
    L"O jogo fechou com um erro (código %lu). Abra os logs para ver os detalhes.", L"Não foi possível iniciar o %s (erro %lu).",
    L"Ultra desempenho ligado: 720p, modo PS4, 30 FPS, FSR 1.0.", L"Escolha a pasta do jogo primeiro.", L"Última sessão: %s.", L"Ainda não há logs: jogue uma vez primeiro.", L"Threads de compilação", L"Alto-falante do controle", L"Calibrar FPS sozinho", L"Preparação paralela (exp.)", L"Geração de quadros", L"Resolução (modo janela)" },
  { L"Ready to play", L"Choose your game folder", L"Port not found", L"Running", L"GAME", L"VIDEO", L"GAMEPLAY", L"Browse",
    L"Port", L"click to change", L"Advanced settings", L"Logs", L"PLAY", L"STOP",
    L"About", L"Game resolution", L"Window size", L"Frame rate limit", L"Present mode", L"Upscaling", L"Sharpening",
    L"Game language", L"Volume", L"Player name", L"Fullscreen", L"PS4 Pro mode", L"Skip intro videos", L"Low input latency (co-op)", L"Ultra performance (weak PCs)",
    L"Native PC port", L"PC port (PS5)", L"Performance profile", L"Shaders", L"FPS on screen", L"X3D mode (Ryzen)", L"Controller vibration",
    L"Fast shader compiling", L"No Windows power saving", L"High priority", L"Large pages", L"Bindless textures (exp.)", L"Choose the %s game folder",
    L"Choose the folder of the %s port (the one with %s)", L"%s was not found in that folder.", L"Starting %s...", L"The game closed.",
    L"The game closed with an error (code %lu). Open the logs for details.", L"Could not start %s (error %lu).",
    L"Ultra performance is on: 720p, PS4 mode, 30 FPS, FSR 1.0.", L"Choose the game folder first.", L"Last session: %s.", L"No logs yet: play once first.", L"Shader compile threads", L"Controller speaker", L"Self-calibrate FPS", L"Parallel preparation (exp.)", L"Frame generation", L"Resolution (windowed)" },
};
static int g_lang = 0;   /* 0 = PT-BR, 1 = EN */
#define T(k) TXT[g_lang][k]

/* ---------------------------------------------------------------- games */
#define NGAMES 5
typedef struct { const wchar_t *id, *name, *exe, *data; BOOL ps5; } Game;
static const Game G[NGAMES] = {
    {L"UFC1", L"UFC 1", L"UFC1.exe", L"UFC1Native", FALSE},
    {L"UFC2", L"UFC 2", L"UFC2.exe", L"UFC2Native", FALSE},
    {L"UFC3", L"UFC 3", L"UFC3.exe", L"UFC3Native", FALSE},
    {L"UFC4", L"UFC 4", L"UFC4.exe", L"UFC4Native", FALSE},
    {L"UFC5", L"UFC 5", L"UFC5.exe", NULL, TRUE},
};

/* ---------------------------------------------------------------- option tables */
typedef struct { const wchar_t *pt, *en; int value; } Opt;
#define O(pt, en, v) {pt, en, v}
static const Opt RES4[] = {O(L"1280x720", L"1280x720", 0), O(L"1920x1080", L"1920x1080", 1), O(L"2560x1440 (2K, experimental)", L"2560x1440 (2K, experimental)", 2), O(L"3840x2160 (4K, experimental)", L"3840x2160 (4K, experimental)", 3)};
static const Opt WIN4[] = {O(L"Igual ao jogo", L"Same as the game", 0), O(L"1920x1080 (Full HD)", L"1920x1080 (Full HD)", 1), O(L"2560x1440 (2K)", L"2560x1440 (2K)", 2), O(L"3840x2160 (4K)", L"3840x2160 (4K)", 3)};
static const Opt FPS4[] = {O(L"30 FPS (PS4)", L"30 FPS (PS4)", 30), O(L"60 FPS", L"60 FPS", 60), O(L"90 FPS", L"90 FPS", 90), O(L"120 FPS", L"120 FPS", 120), O(L"144 FPS", L"144 FPS", 144), O(L"240 FPS", L"240 FPS", 240), O(L"Sem limite", L"Unlimited", 0)};
static const Opt PRES4[] = {O(L"Mailbox", L"Mailbox", 0), O(L"VSync (FIFO)", L"VSync (FIFO)", 1), O(L"Imediato (sem limite, pode rasgar)", L"Immediate (uncapped, may tear)", 2)};
static const Opt UPS4[] = {O(L"Desligado (bilinear)", L"Off (bilinear)", 0), O(L"AMD FSR 1.0", L"AMD FSR 1.0", 1)};
static const Opt SHP4[] = {O(L"Desligada", L"Off", 0), O(L"Baixa", L"Low", 1), O(L"Média", L"Medium", 2), O(L"Alta", L"High", 3)};
static const Opt LANG4[] = {O(L"Inglês (EUA)", L"English (US)", 1), O(L"Francês", L"French", 2), O(L"Alemão", L"German", 4), O(L"Italiano", L"Italian", 5),
    O(L"Espanhol (Espanha)", L"Spanish (Spain)", 3), O(L"Espanhol (América Latina)", L"Spanish (Latin America)", 20), O(L"Português (Brasil)", L"Portuguese (Brazil)", 17),
    O(L"Português (Portugal)", L"Portuguese (Portugal)", 7), O(L"Polonês", L"Polish", 16), O(L"Russo", L"Russian", 8), O(L"Japonês", L"Japanese", 0), O(L"Chinês (Tradicional)", L"Chinese (Traditional)", 10)};
static const Opt VOL4[] = {O(L"50%", L"50%", 50), O(L"75%", L"75%", 75), O(L"100%", L"100%", 100), O(L"125%", L"125%", 125), O(L"150%", L"150%", 150), O(L"200%", L"200%", 200), O(L"300%", L"300%", 300)};
/* UFC 5 (indices exactly as UFC5.exe stores them) */
static const Opt RES5[] = {O(L"Nativa do monitor", L"Monitor native", 0), O(L"1280x720", L"1280x720", 1), O(L"1600x900", L"1600x900", 2), O(L"1920x1080 (Full HD)", L"1920x1080 (Full HD)", 3), O(L"2560x1440 (2K)", L"2560x1440 (2K)", 4), O(L"3840x2160 (4K)", L"3840x2160 (4K)", 5)};
static const Opt PRES5[] = {O(L"Imediato (sem VSync, máx. FPS)", L"Immediate (no VSync, max FPS)", 0), O(L"Mailbox (sem tearing)", L"Mailbox (no tearing)", 1), O(L"VSync (Fifo, mais estável)", L"VSync (Fifo, most stable)", 2)};
static const Opt PROF5[] = {O(L"Máximo FPS", L"Maximum FPS", 0), O(L"Equilibrado", L"Balanced", 1), O(L"Qualidade / estabilidade", L"Quality / stability", 2), O(L"PC fraco (leve)", L"Weak PC (light)", 3), O(L"Personalizado", L"Custom", 4)};
static const Opt OVL5[] = {O(L"Desligado", L"Off", 0), O(L"Canto superior esquerdo", L"Top left", 1), O(L"Canto superior direito", L"Top right", 2)};
static const Opt X3D5[] = {O(L"Automático", L"Automatic", 0), O(L"Ligado (só Ryzen X3D)", L"On (Ryzen X3D only)", 1), O(L"Desligado", L"Off", 2)};
static const Opt LANG5[] = {O(L"Português (Brasil)", L"Portuguese (Brazil)", 0), O(L"Português (Portugal)", L"Portuguese (Portugal)", 1), O(L"Inglês (EUA)", L"English (US)", 2),
    O(L"Inglês (Reino Unido)", L"English (UK)", 3), O(L"Espanhol", L"Spanish", 4), O(L"Espanhol (América Latina)", L"Spanish (Latin America)", 5), O(L"Francês", L"French", 6),
    O(L"Alemão", L"German", 7), O(L"Italiano", L"Italian", 8), O(L"Holandês", L"Dutch", 9), O(L"Polonês", L"Polish", 10), O(L"Russo", L"Russian", 11), O(L"Turco", L"Turkish", 12),
    O(L"Japonês", L"Japanese", 13), O(L"Coreano", L"Korean", 14), O(L"Chinês (Simplificado)", L"Chinese (Simplified)", 15), O(L"Chinês (Tradicional)", L"Chinese (Traditional)", 16)};
/* the engine's virtual vblank (UFC5.exe: 0 = 60, 1 = 120, 2 = 144 Hz): frames are flipped on its ticks, so 120/144 let menus pass 60 FPS
   and a fight frame that just missed a tick wait less for the next one */
static const Opt FPS5[] = {O(L"60 FPS (padrão do PS5)", L"60 FPS (PS5 default)", 0), O(L"120 FPS (experimental)", L"120 FPS (experimental)", 1),
                           O(L"144 FPS (experimental)", L"144 FPS (experimental)", 2)};
/* frame generation through Lossless Scaling (Steam): 120/144 on screen from the engine's ~30 in a fight */
static const Opt FG5[] = {O(L"Desligada", L"Off", 0), O(L"Lossless (liga sozinho)", L"Lossless (turns itself on)", 1),
                          O(L"Lossless (só abrir)", L"Lossless (open only)", 2)};
static const Opt SHT5[] = {O(L"Automático", L"Automatic", 0), O(L"2 threads", L"2 threads", 2), O(L"4 threads", L"4 threads", 4), O(L"6 threads", L"6 threads", 6), O(L"8 threads", L"8 threads", 8)};
static const Opt CAL5[] = {O(L"Desligada", L"Off", 0), O(L"Ligada: mede até 12 lutas e escolhe", L"On: measures up to 12 fights and picks", 1)};
static const Opt VIB5[] = {O(L"0%", L"0%", 0), O(L"25%", L"25%", 25), O(L"50%", L"50%", 50), O(L"75%", L"75%", 75), O(L"100%", L"100%", 100)};
#define N(a) (int)(sizeof(a) / sizeof(a[0]))

/* a "field" of the right panel: a dropdown or a switch bound to one key of the port's settings */
typedef struct { int label; const wchar_t *key; const Opt *opts; int nopts; int def; BOOL perf; } Field;   /* perf: UFC 5 key that makes the profile "Custom" */
static const Field VID4[6] = {{S_RES, L"resolution", RES4, N(RES4), 1}, {S_WIN, L"display", WIN4, N(WIN4), 0}, {S_FPS, L"fps_limit", FPS4, N(FPS4), 60},
                              {S_PRESENT, L"present_mode", PRES4, N(PRES4), 0}, {S_UPSCALE, L"upscaler", UPS4, N(UPS4), 0}, {S_SHARP, L"sharpness", SHP4, N(SHP4), 2}};
static const Field VID5[9] = {{S_RES5, L"resolution", RES5, N(RES5), 0}, {S_PRESENT, L"present", PRES5, N(PRES5), 0, TRUE}, {S_FPS, L"vblank", FPS5, N(FPS5), 0, TRUE},
                              {S_PROFILE, L"profile", PROF5, N(PROF5), 0}, {S_FRAMEGEN, L"framegen", FG5, N(FG5), 0}, {S_SHTHREADS, L"shader_threads", SHT5, N(SHT5), 0},
                              {S_OVERLAY, L"fps_overlay", OVL5, N(OVL5), 1}, {S_CALIB, L"calibrate", CAL5, N(CAL5), 0}, {S_VIB, L"controller_vibration", VIB5, N(VIB5), 100}};
static const Field GP4[2] = {{S_LANG, L"language", LANG4, N(LANG4), 17}, {S_VOLUME, L"volume", VOL4, N(VOL4), 100}};
static const Field GP5[2] = {{S_LANG, L"language", LANG5, N(LANG5), 0}, {S_SPKVOL, L"controller_volume", VIB5, N(VIB5), 50}};
static const Field SW4[5] = {{S_FULL, L"fullscreen", NULL, 0, 1}, {S_NEO, L"neo", NULL, 0, 0}, {S_SKIP, L"skip_movies", NULL, 0, 1}, {S_LOWLAT, L"low_latency", NULL, 0, 0}, {S_ULTRA, L"ultra", NULL, 0, 0}};
static const Field SW5[6] = {{S_FULL, L"fullscreen", NULL, 0, 1, TRUE}, {S_FASTSH, L"fast_shaders", NULL, 0, 1}, {S_ECO, L"no_power_throttling", NULL, 0, 1},
                             {S_PRIO, L"high_priority", NULL, 0, 1, TRUE}, {S_LPAGES, L"large_pages", NULL, 0, 0}, {S_STAGEPREP, L"stage_prep_parallel", NULL, 0, 0}};

/* ---------------------------------------------------------------- state */
static HINSTANCE g_inst;
static HWND g_wnd, g_editGame, g_editPlayer;
static int g_dpi = 96, g_sel = 4, g_hot = -1;
static HFONT g_fTitle, g_fCard, g_fText, g_fSmall, g_fSection, g_fPlay, g_fBtn, g_fFoot, g_fLabel, g_fTag, g_fLogo, g_fLogoSub;
static HBRUSH g_brField;
static wchar_t g_exeDir[MAX_PATH], g_pIni[MAX_PATH], g_msg[512];
static COLORREF g_msgColor = C_SUB;
static ULONG_PTR g_gdip;
static GpImage *g_imgLogo;
static HBITMAP g_bgCache; static int g_bgW, g_bgH;
static HANDLE g_job, g_proc; static int g_procGame = -1;
static wchar_t g_ver[NGAMES][48];

#define SC(x) MulDiv((x), g_dpi, 96)

/* hit regions, rebuilt on every paint */
enum { H_CARD = 0, H_DROP = 10, H_SWITCH = 30, H_BROWSE = 50, H_PORT, H_ADV, H_LOGS, H_PLAY, H_ABOUT, H_EN, H_PT };
typedef struct { RECT r; int id; } Hit;
static Hit g_hits[64]; static int g_nhits;
static void AddHit(int l, int t, int r, int b, int id) { if (g_nhits < 64) { SetRect(&g_hits[g_nhits].r, l, t, r, b); g_hits[g_nhits++].id = id; } }
static int HitAt(int x, int y) { POINT p = {x, y}; for (int i = g_nhits - 1; i >= 0; i--) if (PtInRect(&g_hits[i].r, p)) return g_hits[i].id; return -1; }

/* ---------------------------------------------------------------- small helpers */
static BOOL FileExists(const wchar_t *p) { DWORD a = GetFileAttributesW(p); return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY); }
static BOOL DirExists(const wchar_t *p) { DWORD a = GetFileAttributesW(p); return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY); }
static void Join(wchar_t *out, const wchar_t *a, const wchar_t *b) { _snwprintf(out, MAX_PATH, L"%s\\%s", a, b); out[MAX_PATH - 1] = 0; }
static void StripSlash(wchar_t *p) { size_t n = wcslen(p); while (n > 3 && (p[n - 1] == L'\\' || p[n - 1] == L'/')) p[--n] = 0; }
static void Msg(COLORREF c, const wchar_t *fmt, ...)
{
    va_list ap; va_start(ap, fmt); _vsnwprintf(g_msg, 512, fmt, ap); va_end(ap); g_msg[511] = 0; g_msgColor = c;
    if (g_wnd) InvalidateRect(g_wnd, NULL, FALSE);
}

static void PortDir(int g, wchar_t *out)
{
    wchar_t k[32]; _snwprintf(k, 32, L"port_%s", G[g].id);
    GetPrivateProfileStringW(L"CAGE", k, L"", out, MAX_PATH, g_pIni);
    if (out[0] && DirExists(out)) return;
    wchar_t t[MAX_PATH]; Join(t, g_exeDir, L"games"); Join(out, t, G[g].id);
}
static void PortExe(int g, wchar_t *out) { wchar_t d[MAX_PATH]; PortDir(g, d); Join(out, d, G[g].exe); }

/* ---------------------------------------------------------------- settings files */
/* UFC 1-4: %LOCALAPPDATA%\UFCnNative\sacredfire.ini - UTF-8 with BOM, CRLF, key=value, unknown keys kept */
#define MAXL 128
typedef struct { wchar_t k[48], v[MAX_PATH]; } KV;
static KV g_kv[MAXL]; static int g_nkv, g_kvGame = -1;

static void SfPath(int g, wchar_t *out) { wchar_t la[MAX_PATH] = L""; GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); wchar_t d[MAX_PATH]; Join(d, la, G[g].data); Join(out, d, L"sacredfire.ini"); }
static void SfLoad(int g)
{
    g_nkv = 0; g_kvGame = g;
    wchar_t p[MAX_PATH]; SfPath(g, p);
    HANDLE f = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD sz = GetFileSize(f, NULL); if (sz > 256 * 1024) sz = 256 * 1024;
    char *b = (char *)HeapAlloc(GetProcessHeap(), 0, sz + 1); DWORD n = 0; ReadFile(f, b, sz, &n, NULL); CloseHandle(f); b[n] = 0;
    char *s = b; if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) s += 3;
    while (*s && g_nkv < MAXL) {
        char *e = s; while (*e && *e != '\n') e++;
        char save = *e; *e = 0; char *cr = strchr(s, '\r'); if (cr) *cr = 0;
        char *eq = strchr(s, '=');
        if (eq && eq != s) { *eq = 0; MultiByteToWideChar(CP_UTF8, 0, s, -1, g_kv[g_nkv].k, 48); MultiByteToWideChar(CP_UTF8, 0, eq + 1, -1, g_kv[g_nkv].v, MAX_PATH); g_nkv++; }
        *e = save; s = *e ? e + 1 : e;
    }
    HeapFree(GetProcessHeap(), 0, b);
}
static void SfSave(void)
{
    if (g_kvGame < 0) return;
    wchar_t p[MAX_PATH], d[MAX_PATH]; SfPath(g_kvGame, p); wcscpy(d, p); *wcsrchr(d, L'\\') = 0; CreateDirectoryW(d, NULL);
    HANDLE f = CreateFileW(p, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD w; WriteFile(f, "\xEF\xBB\xBF", 3, &w, NULL);
    for (int i = 0; i < g_nkv; i++) {
        wchar_t line[MAX_PATH + 64]; _snwprintf(line, MAX_PATH + 64, L"%s=%s\r\n", g_kv[i].k, g_kv[i].v);
        char u[(MAX_PATH + 64) * 3]; int n = WideCharToMultiByte(CP_UTF8, 0, line, -1, u, sizeof(u), NULL, NULL);
        if (n > 1) WriteFile(f, u, n - 1, &w, NULL);
    }
    CloseHandle(f);
}
static KV *SfFind(const wchar_t *k) { for (int i = 0; i < g_nkv; i++) if (!wcscmp(g_kv[i].k, k)) return &g_kv[i]; return NULL; }

/* UFC 5: games\UFC5\UFC5.ini, section [UFC5] */
static void K5Ini(wchar_t *out) { wchar_t d[MAX_PATH]; PortDir(4, d); Join(out, d, L"UFC5.ini"); }

/* one access layer for both games */
static void GetStr(int g, const wchar_t *k, wchar_t *out, int n, const wchar_t *def)
{
    if (G[g].ps5) { wchar_t ini[MAX_PATH]; K5Ini(ini); GetPrivateProfileStringW(L"UFC5", k, def, out, n, ini); return; }
    if (g_kvGame != g) SfLoad(g);
    KV *kv = SfFind(k); wcsncpy(out, kv ? kv->v : def, n); out[n - 1] = 0;
}
static int GetInt(int g, const wchar_t *k, int def) { wchar_t b[32], d[16]; _snwprintf(d, 16, L"%d", def); GetStr(g, k, b, 32, d); return b[0] ? _wtoi(b) : def; }
static void PutStr(int g, const wchar_t *k, const wchar_t *v)
{
    if (G[g].ps5) { wchar_t ini[MAX_PATH]; K5Ini(ini); WritePrivateProfileStringW(L"UFC5", k, v, ini); return; }
    if (g_kvGame != g) SfLoad(g);
    KV *kv = SfFind(k);
    if (!kv && g_nkv < MAXL) { kv = &g_kv[g_nkv++]; wcsncpy(kv->k, k, 47); kv->k[47] = 0; }
    if (kv) { wcsncpy(kv->v, v, MAX_PATH - 1); kv->v[MAX_PATH - 1] = 0; }
    SfSave();
}
static void PutInt(int g, const wchar_t *k, int v) { wchar_t b[16]; _snwprintf(b, 16, L"%d", v); PutStr(g, k, b); }

/* UFC 5 profiles, the same as UFC5.exe's: only performance keys change */
static void ApplyProfile5(int p)
{
    PutInt(4, L"profile", p);
    if (p > 3) return;
    static const int present[4] = {0, 1, 2, 0}, prio[4] = {1, 1, 0, 1}, power[4] = {1, 1, 0, 1}, trophy[4] = {0, 1, 1, 0};
    if (p == 3) { PutInt(4, L"resolution", 3); PutInt(4, L"fast_shaders", 1); PutInt(4, L"no_power_throttling", 1); }
    PutInt(4, L"present", present[p]); PutInt(4, L"vblank", 0); PutInt(4, L"shader", 0); PutInt(4, L"fullscreen", 1);
    PutInt(4, L"high_priority", prio[p]); PutInt(4, L"dedicated_gpu", 1); PutInt(4, L"power_plan", power[p]); PutInt(4, L"hide_cursor", 1);
    PutInt(4, L"trophies", trophy[p]); PutInt(4, L"bindless", 0); PutInt(4, L"tessellation", 0);
}

/* ---------------------------------------------------------------- game folder */
static BOOL HasEboot(const wchar_t *d) { wchar_t p[MAX_PATH]; Join(p, d, L"eboot.bin"); return FileExists(p); }
static BOOL GameOk(const wchar_t *d)
{
    if (!d[0] || !DirExists(d)) return FALSE;
    if (HasEboot(d)) return TRUE;
    wchar_t pat[MAX_PATH], c[MAX_PATH]; Join(pat, d, L"*"); WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd); BOOL ok = FALSE;
    if (h == INVALID_HANDLE_VALUE) return FALSE;
    do { if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') { Join(c, d, fd.cFileName); if (HasEboot(c)) { ok = TRUE; break; } } } while (FindNextFileW(h, &fd));
    FindClose(h); return ok;
}
static BOOL PortOk(int g) { wchar_t e[MAX_PATH]; PortExe(g, e); return FileExists(e); }
static int Status(int g)   /* S_READY, S_PICK, S_NOPORT, S_RUNNING */
{
    if (g_proc && g_procGame == g) return S_RUNNING;
    if (!PortOk(g)) return S_NOPORT;
    wchar_t p[MAX_PATH]; GetStr(g, L"game", p, MAX_PATH, L"");
    if (G[g].ps5 && !p[0]) {   /* the UFC 5 port finds Desktop\UFC 5 by itself */
        PWSTR dk = NULL; if (SHGetKnownFolderPath(&FOLDERID_Desktop, 0, NULL, &dk) == S_OK) { Join(p, dk, L"UFC 5"); CoTaskMemFree(dk); }
    }
    return GameOk(p) ? S_READY : S_PICK;
}

static void ReadVersion(int g)
{
    wchar_t d[MAX_PATH], p[MAX_PATH]; PortDir(g, d); Join(p, d, L"VERSION"); g_ver[g][0] = 0;
    HANDLE f = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    char b[64]; DWORD n = 0; ReadFile(f, b, 63, &n, NULL); CloseHandle(f); b[n] = 0;
    char *e = b; while (*e && *e != '\r' && *e != '\n') e++; *e = 0;
    MultiByteToWideChar(CP_UTF8, 0, b, -1, g_ver[g], 48);
}

/* ---------------------------------------------------------------- images */
static GpImage *LoadPngRes(int id)
{
    HRSRC r = FindResourceW(g_inst, MAKEINTRESOURCEW(id), (LPCWSTR)RT_RCDATA); if (!r) return NULL;
    DWORD sz = SizeofResource(g_inst, r); void *p = LockResource(LoadResource(g_inst, r)); if (!p) return NULL;
    IStream *s = SHCreateMemStream((const BYTE *)p, sz); if (!s) return NULL;
    GpImage *img = NULL; GdipCreateBitmapFromStream(s, &img); s->lpVtbl->Release(s); return img;
}
static void BuildBg(int w, int h)
{
    if (g_bgCache && g_bgW == w && g_bgH == h) return;
    if (g_bgCache) DeleteObject(g_bgCache);
    HDC wdc = GetDC(g_wnd), dc = CreateCompatibleDC(wdc);
    g_bgCache = CreateCompatibleBitmap(wdc, w, h); g_bgW = w; g_bgH = h;
    HGDIOBJ o = SelectObject(dc, g_bgCache);
    HBRUSH b = CreateSolidBrush(C_BG); RECT r = {0, 0, w, h}; FillRect(dc, &r, b); DeleteObject(b);
    b = CreateSolidBrush(C_SIDE); RECT s = {0, 0, SC(SIDE_W), h}; FillRect(dc, &s, b); DeleteObject(b);          /* sidebar sólida */
    b = CreateSolidBrush(C_FIELDB); RECT d = {SC(SIDE_W), 0, SC(SIDE_W) + 1, h}; FillRect(dc, &d, b); DeleteObject(b);   /* divisor de 1px */
    GpGraphics *g = NULL; GdipCreateFromHDC(dc, &g);
    if (g) {
        GdipSetInterpolationMode(g, 7); GdipSetSmoothingMode(g, 4);
        if (g_imgLogo) GdipDrawImageRectI(g, g_imgLogo, SC((SIDE_W - 232) / 2), SC(10), SC(232), SC(232));
        GdipDeleteGraphics(g);
    }
    SelectObject(dc, o); DeleteDC(dc); ReleaseDC(g_wnd, wdc);
}

/* ---------------------------------------------------------------- drawing primitives (GDI+, anti-aliased) */
static GpPath *RoundPath(int x, int y, int w, int h, int r)
{
    GpPath *p = NULL; GdipCreatePath(0, &p); int d = r * 2;
    if (r <= 0) { GdipAddPathLineI(p, x, y, x + w, y); GdipAddPathLineI(p, x + w, y, x + w, y + h); GdipAddPathLineI(p, x + w, y + h, x, y + h); }
    else {
        GdipAddPathArcI(p, x, y, d, d, 180, 90); GdipAddPathArcI(p, x + w - d, y, d, d, 270, 90);
        GdipAddPathArcI(p, x + w - d, y + h - d, d, d, 0, 90); GdipAddPathArcI(p, x, y + h - d, d, d, 90, 90);
    }
    GdipClosePathFigure(p); return p;
}
static void Box(GpGraphics *g, RECT r, int rad, UINT32 fill, UINT32 border, float bw)
{
    GpPath *p = RoundPath(r.left, r.top, r.right - r.left - 1, r.bottom - r.top - 1, rad);
    if (fill) { GpBrush *b = NULL; GdipCreateSolidFill(fill, &b); GdipFillPath(g, b, p); GdipDeleteBrush(b); }
    if (border) { GpPen *pen = NULL; GdipCreatePen1(border, bw, 2, &pen); GdipDrawPath(g, pen, p); GdipDeletePen(pen); }
    GdipDeletePath(p);
}
static void Fill(GpGraphics *g, int x, int y, int w, int h, COLORREF c)
{ GpBrush *b = NULL; GdipCreateSolidFill(A(c, 255), &b); GdipFillRectangleI(g, b, x, y, w, h); GdipDeleteBrush(b); }
static void Text(HDC dc, HFONT f, COLORREF c, int x, int y, const wchar_t *s)
{ SelectObject(dc, f); SetTextColor(dc, c); TextOutW(dc, x, y, s, (int)wcslen(s)); }
static void TextR(HDC dc, HFONT f, COLORREF c, RECT r, const wchar_t *s, UINT fl)
{ SelectObject(dc, f); SetTextColor(dc, c); DrawTextW(dc, s, -1, &r, fl | DT_SINGLELINE | DT_NOPREFIX); }
static int TextW(HDC dc, HFONT f, const wchar_t *s) { SelectObject(dc, f); SIZE z; GetTextExtentPoint32W(dc, s, (int)wcslen(s), &z); return z.cx; }
/* cópia em caixa alta (títulos, seções e botões em Bahnschrift) */
static const wchar_t *Caps(wchar_t *out, int n, const wchar_t *s) { wcsncpy(out, s, n - 1); out[n - 1] = 0; CharUpperW(out); return out; }

static void Section(HDC dc, GpGraphics *g, int y, const wchar_t *t)
{   /* traço vermelho curto, rótulo em caixa alta e linha fina até a borda direita */
    wchar_t u[64]; Caps(u, 64, t);
    Fill(g, SC(X0), SC(y + 7), SC(14), SC(2), C_ACCENT);
    SetTextCharacterExtra(dc, SC(2)); int w = TextW(dc, g_fSection, u);
    Text(dc, g_fSection, C_TEXT, SC(X0 + 24), SC(y), u); SetTextCharacterExtra(dc, 0);
    Fill(g, SC(X0 + 24) + w + SC(12), SC(y + 8), SC(X1) - (SC(X0 + 24) + w + SC(12)), 1, C_FIELDB);
}

/* ---------------------------------------------------------------- fields */
static const Field *Vid(int g) { return G[g].ps5 ? VID5 : VID4; }
static const Field *Gp(int g) { return G[g].ps5 ? GP5 : GP4; }
static const Field *Sw(int g, int *n) { *n = G[g].ps5 ? N(SW5) : N(SW4); return G[g].ps5 ? SW5 : SW4; }
static int OptIndex(const Field *f, int v) { for (int i = 0; i < f->nopts; i++) if (f->opts[i].value == v) return i; return -1; }
static const wchar_t *OptName(const Opt *o) { return g_lang ? o->en : o->pt; }

static void Dropdown(HDC dc, GpGraphics *g, int x, int y, int w, const Field *f, int id)
{
    Text(dc, g_fLabel, C_SUB, SC(x), SC(y), T(f->label));
    RECT r = {SC(x), SC(y + 20), SC(x + w), SC(y + 56)};
    BOOL hot = g_hot == id;
    Box(g, r, SC(2), A(C_FIELD, 255), A(hot ? C_FIELDH : C_FIELDB, 255), 1.0f);
    int v = GetInt(g_sel, f->key, f->def), i = OptIndex(f, v);
    wchar_t custom[32]; const wchar_t *name = i >= 0 ? OptName(&f->opts[i]) : (_snwprintf(custom, 32, L"%d", v), custom);
    RECT tr = {r.left + SC(14), r.top, r.right - SC(36), r.bottom}; TextR(dc, g_fText, C_TEXT, tr, name, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
    GpPen *pen = NULL; GdipCreatePen1(A(hot ? C_TEXT : C_SUB, 255), 1.5f * g_dpi / 96, 2, &pen);   /* chevron fino */
    int cx = r.right - SC(18), cy = (r.top + r.bottom) / 2;
    GdipDrawLineI(g, pen, cx - SC(4), cy - SC(2), cx, cy + SC(2)); GdipDrawLineI(g, pen, cx, cy + SC(2), cx + SC(4), cy - SC(2)); GdipDeletePen(pen);
    AddHit(r.left, r.top, r.right, r.bottom, id);
}

static void Switch(HDC dc, GpGraphics *g, int x, int y, const Field *f, int id)
{   /* ligado: trilho vermelho e bolinha branca; desligado: trilho cinza escuro e bolinha cinza */
    BOOL on = GetInt(g_sel, f->key, f->def) != 0, hot = g_hot == id;
    RECT r = {SC(x), SC(y + 3), SC(x + 36), SC(y + 21)};
    if (on) Box(g, r, SC(2), A(hot ? C_ACCHOT : C_ACCENT, 255), 0, 0);
    else Box(g, r, SC(2), A(C_OFF, 255), A(hot ? C_FIELDH : C_FIELDB, 255), 1.0f);
    int k = SC(12), ky = (r.top + r.bottom - k) / 2, kx = on ? r.right - SC(3) - k : r.left + SC(3);
    RECT kr = {kx, ky, kx + k, ky + k}; Box(g, kr, SC(1), on ? 0xFFFFFFFF : A(hot ? C_SUB : RGB(100, 100, 108), 255), 0, 0);
    Text(dc, g_fText, C_TEXT, SC(x + 48), SC(y + 2), T(f->label));
    AddHit(r.left, SC(y), SC(x + 48) + TextW(dc, g_fText, T(f->label)), SC(y + 24), id);
}

static void Button(HDC dc, GpGraphics *g, RECT r, const wchar_t *t, int id)
{   /* secundário: contorno de 1px, texto branco, borda branca em hover */
    BOOL hot = g_hot == id; wchar_t u[64];
    Box(g, r, SC(2), 0, A(hot ? C_TEXT : C_FIELDH, 255), 1.0f);
    SetTextCharacterExtra(dc, SC(1)); TextR(dc, g_fBtn, C_TEXT, r, Caps(u, 64, t), DT_CENTER | DT_VCENTER); SetTextCharacterExtra(dc, 0);
    AddHit(r.left, r.top, r.right, r.bottom, id);
}

static void PlayButton(HDC dc, GpGraphics *g, RECT r)
{
    BOOL hot = g_hot == H_PLAY, can = Status(g_sel) == S_READY || g_proc;
    const wchar_t *t = g_proc ? T(S_STOP) : T(S_PLAY); COLORREF tc;
    if (g_proc) { Box(g, r, SC(2), A(hot ? C_ROWSEL : C_BG, 255), A(C_TEXT, 255), 1.0f); tc = C_TEXT; }   /* PARAR: contorno branco, nunca vermelho sólido */
    else if (can) { Box(g, r, SC(2), A(hot ? C_ACCHOT : C_ACCENT, 255), 0, 0); tc = RGB(255, 255, 255); }
    else { Box(g, r, SC(2), A(C_OFF, 255), 0, 0); tc = C_SUB; }
    SetTextCharacterExtra(dc, SC(3));
    if (g_proc) {   /* ícone de parar (quadrado) antes do texto */
        int tw = TextW(dc, g_fPlay, t), q = SC(10), x = (r.left + r.right - tw - q - SC(14)) / 2;
        Fill(g, x, (r.top + r.bottom - q) / 2, q, q, C_TEXT);
        RECT tr = {x + q + SC(14), r.top, r.right, r.bottom}; TextR(dc, g_fPlay, tc, tr, t, DT_LEFT | DT_VCENTER);
    } else TextR(dc, g_fPlay, tc, r, t, DT_CENTER | DT_VCENTER);
    SetTextCharacterExtra(dc, 0);
    AddHit(r.left, r.top, r.right, r.bottom, H_PLAY);
}

static COLORREF StatusColor(int s) { return s == S_READY ? C_TEXT : s == S_RUNNING ? C_GOOD : s == S_PICK ? C_ORANGE : C_BAD; }

static void Card(HDC dc, GpGraphics *g, int i, int y)
{   /* linha plana: selecionada = barra vermelha de 3px + fundo mais claro; hover = fundo mais claro */
    RECT r = {0, SC(y), SC(SIDE_W), SC(y + 62)};
    BOOL sel = i == g_sel, hot = g_hot == H_CARD + i;
    if (sel || hot) Fill(g, r.left, r.top, r.right - r.left, r.bottom - r.top, sel ? C_ROWSEL : C_ROWHOT);
    if (sel) Fill(g, r.left, r.top, SC(3), r.bottom - r.top, C_ACCENT);
    Text(dc, g_fCard, C_TEXT, SC(28), r.top + SC(10), G[i].name);
    int st = Status(i);
    Fill(g, SC(29), r.top + SC(41), SC(6), SC(6), StatusColor(st));
    Text(dc, g_fSmall, C_SUB, SC(42), r.top + SC(35), T(st));
    if (G[i].ps5 && st != S_NOPORT) {   /* tag "NOVO" do port mais recente: texto pequeno vermelho */
        const wchar_t *tag = g_lang ? L"NEW" : L"NOVO";
        SetTextCharacterExtra(dc, SC(1)); Text(dc, g_fTag, C_ACCENT, SC(SIDE_W - 24) - TextW(dc, g_fTag, tag), r.top + SC(15), tag); SetTextCharacterExtra(dc, 0);
    }
    AddHit(r.left, r.top, r.right, r.bottom, H_CARD + i);
}

static void Paint(HDC wdc)
{
    RECT rc; GetClientRect(g_wnd, &rc);
    HDC dc = CreateCompatibleDC(wdc); HBITMAP bmp = CreateCompatibleBitmap(wdc, rc.right, rc.bottom); HGDIOBJ ob = SelectObject(dc, bmp);
    BuildBg(rc.right, rc.bottom);
    { HDC m = CreateCompatibleDC(wdc); HGDIOBJ o = SelectObject(m, g_bgCache); BitBlt(dc, 0, 0, rc.right, rc.bottom, m, 0, 0, SRCCOPY); SelectObject(m, o); DeleteDC(m); }
    SetBkMode(dc, TRANSPARENT); g_nhits = 0;
    GpGraphics *g = NULL; GdipCreateFromHDC(dc, &g); GdipSetSmoothingMode(g, 4); GdipSetPixelOffsetMode(g, 4);

    /* sidebar: cards and footer */
    for (int i = 0; i < NGAMES; i++) Card(dc, g, i, 252 + i * 70);
    int fy = BASE_H - 44;
    Text(dc, g_fFoot, g_hot == H_ABOUT ? C_TEXT : C_SUB, SC(30), SC(fy), T(S_ABOUT)); AddHit(SC(24), SC(fy - 4), SC(90), SC(fy + 22), H_ABOUT);
    Text(dc, g_fFoot, C_SUB, SC(150), SC(fy), L"v" APP_VER);
    Text(dc, g_fFoot, g_lang == 1 ? C_TEXT : C_SUB, SC(222), SC(fy), L"EN"); AddHit(SC(216), SC(fy - 4), SC(246), SC(fy + 22), H_EN);
    Text(dc, g_fFoot, g_lang == 0 ? C_TEXT : C_SUB, SC(258), SC(fy), L"PT-BR"); AddHit(SC(252), SC(fy - 4), SC(306), SC(fy + 22), H_PT);

    /* title, subtitle and status pill */
    int gsel = g_sel, st = Status(gsel);
    Text(dc, g_fTitle, C_TEXT, SC(X0), SC(18), G[gsel].name);
    wchar_t sub[128]; _snwprintf(sub, 128, L"%s - %s", G[gsel].ps5 ? T(S_PS5PORT) : T(S_NATIVE), g_ver[gsel][0] ? g_ver[gsel] : L"?");
    Text(dc, g_fText, C_SUB, SC(X0 + 2), SC(84), sub);
    { int tw = TextW(dc, g_fText, T(st)); RECT t = {SC(X1) - tw, SC(30), SC(X1), SC(64)};
      COLORREF c = StatusColor(st); Fill(g, t.left - SC(18), (t.top + t.bottom) / 2 - SC(4), SC(8), SC(8), c);
      TextR(dc, g_fText, c, t, T(st), DT_LEFT | DT_VCENTER); }

    /* GAME */
    Section(dc, g, 114, T(S_GAME));
    RECT fr = {SC(X0), SC(138), SC(X1 - 186), SC(174)}; Box(g, fr, SC(2), A(C_FIELD, 255), A(GetFocus() == g_editGame ? C_ACCENT : C_FIELDB, 255), 1.0f);
    MoveWindow(g_editGame, fr.left + SC(16), fr.top + SC(9), fr.right - fr.left - SC(28), SC(20), FALSE);
    RECT br = {SC(X1 - 172), SC(138), SC(X1), SC(174)}; Button(dc, g, br, T(S_BROWSE), H_BROWSE);
    wchar_t pd[MAX_PATH], line[MAX_PATH + 64]; PortDir(gsel, pd);
    _snwprintf(line, MAX_PATH + 64, L"%s: %s   -   %s", T(S_PORT), pd, T(S_CLICKCHANGE));
    RECT pr = {SC(X0), SC(182), SC(X1), SC(202)}; TextR(dc, g_fSmall, g_hot == H_PORT ? C_TEXT : C_SUB, pr, line, DT_LEFT | DT_VCENTER | DT_PATH_ELLIPSIS);
    AddHit(pr.left, pr.top, pr.left + TextW(dc, g_fSmall, line), pr.bottom, H_PORT);

    /* VIDEO: 3 rows x 2 columns */
    Section(dc, g, 214, T(S_VIDEO));
    const Field *v = Vid(gsel); int nv = G[gsel].ps5 ? 9 : 6, cols = G[gsel].ps5 ? 3 : 2, gap = G[gsel].ps5 ? 20 : 28, cw = (X1 - X0 - gap * (cols - 1)) / cols;
    for (int i = 0; i < nv; i++) Dropdown(dc, g, X0 + (i % cols) * (cw + gap), 236 + (i / cols) * 64, cw, &v[i], H_DROP + i);

    /* GAMEPLAY */
    Section(dc, g, 432, T(S_GAMEPLAY));
    const Field *gp = Gp(gsel);
    Dropdown(dc, g, X0, 454, 300, &gp[0], H_DROP + 9);
    Dropdown(dc, g, X0 + 316, 454, 156, &gp[1], H_DROP + 10);
    Text(dc, g_fText, C_SUB, SC(X0 + 488), SC(454), T(S_PLAYER));
    RECT nr = {SC(X0 + 488), SC(474), SC(X1), SC(510)}; Box(g, nr, SC(2), A(C_FIELD, 255), A(GetFocus() == g_editPlayer ? C_ACCENT : C_FIELDB, 255), 1.0f);
    MoveWindow(g_editPlayer, nr.left + SC(16), nr.top + SC(9), nr.right - nr.left - SC(28), SC(20), FALSE);
    int ns; const Field *sw = Sw(gsel, &ns);
    static const int sx[3] = {X0, X0 + 266, X0 + 530};
    for (int i = 0; i < ns; i++) Switch(dc, g, sx[i % 3], 528 + (i / 3) * 38, &sw[i], H_SWITCH + i);

    /* buttons */
    RECT ab = {SC(X0), SC(612), SC(X0 + 270), SC(656)}; Button(dc, g, ab, T(S_ADV), H_ADV);
    RECT lb = {SC(X0 + 284), SC(612), SC(X0 + 404), SC(656)}; Button(dc, g, lb, T(S_LOGS), H_LOGS);
    RECT pb = {SC(X1 - 300), SC(600), SC(X1), SC(660)}; PlayButton(dc, g, pb);
    if (g_msg[0]) { RECT mr = {SC(X0), SC(668), SC(X1), SC(696)}; TextR(dc, g_fSmall, g_msgColor, mr, g_msg, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS); }

    GdipDeleteGraphics(g);
    BitBlt(wdc, 0, 0, rc.right, rc.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, ob); DeleteObject(bmp); DeleteDC(dc);
}

/* ---------------------------------------------------------------- selection and actions */
static void LoadEdits(void)
{
    wchar_t b[MAX_PATH];
    GetStr(g_sel, L"game", b, MAX_PATH, L""); SetWindowTextW(g_editGame, b);
    GetStr(g_sel, G[g_sel].ps5 ? L"user" : L"player", b, MAX_PATH, G[g_sel].ps5 ? L"Lutador" : L"Player 1"); SetWindowTextW(g_editPlayer, b);
}
static void SaveEdits(void)
{
    wchar_t b[MAX_PATH], cur[MAX_PATH];
    GetWindowTextW(g_editGame, b, MAX_PATH); StripSlash(b); GetStr(g_sel, L"game", cur, MAX_PATH, L"");
    if (wcscmp(b, cur)) PutStr(g_sel, L"game", b);
    const wchar_t *pk = G[g_sel].ps5 ? L"user" : L"player";
    GetWindowTextW(g_editPlayer, b, 64); GetStr(g_sel, pk, cur, MAX_PATH, L"");
    if (wcscmp(b, cur)) PutStr(g_sel, pk, b);
}
static void Select(int g)
{
    if (g < 0 || g >= NGAMES || g == g_sel) return;
    SaveEdits(); g_sel = g; g_kvGame = -1; g_msg[0] = 0;
    WritePrivateProfileStringW(L"CAGE", L"selected_game", G[g].id, g_pIni);
    LoadEdits();
    if (G[g].ps5) { wchar_t last[400]; GetStr(4, GetInt(4, L"calibrate", 0) ? L"calib_status" : L"last_session", last, 400, L"");
                    if (last[0]) Msg(C_SUB, GetInt(4, L"calibrate", 0) ? L"%s" : T(S_SESSION), last); }
    InvalidateRect(g_wnd, NULL, FALSE);
}

static BOOL PickFolder(wchar_t *path, const wchar_t *title)
{
    IFileOpenDialog *d = NULL; BOOL ok = FALSE;
    if (FAILED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileOpenDialog, (void **)&d))) return FALSE;
    DWORD o; d->lpVtbl->GetOptions(d, &o); d->lpVtbl->SetOptions(d, o | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM); d->lpVtbl->SetTitle(d, title);
    if (path[0]) { IShellItem *si; if (SUCCEEDED(SHCreateItemFromParsingName(path, NULL, &IID_IShellItem, (void **)&si))) { d->lpVtbl->SetFolder(d, si); si->lpVtbl->Release(si); } }
    if (SUCCEEDED(d->lpVtbl->Show(d, g_wnd))) {
        IShellItem *it; if (SUCCEEDED(d->lpVtbl->GetResult(d, &it))) {
            PWSTR p; if (SUCCEEDED(it->lpVtbl->GetDisplayName(it, SIGDN_FILESYSPATH, &p))) { wcsncpy(path, p, MAX_PATH); CoTaskMemFree(p); ok = TRUE; }
            it->lpVtbl->Release(it);
        }
    }
    d->lpVtbl->Release(d); return ok;
}

static void Browse(void)
{
    wchar_t p[MAX_PATH], t[160]; GetWindowTextW(g_editGame, p, MAX_PATH); _snwprintf(t, 160, T(S_PICKGAME), G[g_sel].name);
    if (!PickFolder(p, t)) return;
    StripSlash(p); SetWindowTextW(g_editGame, p); PutStr(g_sel, L"game", p);
    Msg(GameOk(p) ? C_GOOD : C_ORANGE, L"%s", GameOk(p) ? T(S_READY) : T(S_PICK));
}
static void ChangePort(void)
{
    wchar_t p[MAX_PATH], t[200]; PortDir(g_sel, p); _snwprintf(t, 200, T(S_PICKPORT), G[g_sel].name, G[g_sel].exe);
    if (!PickFolder(p, t)) return;
    wchar_t e[MAX_PATH]; Join(e, p, G[g_sel].exe);
    if (!FileExists(e)) { Msg(C_BAD, T(S_NOTFOUND), G[g_sel].exe); return; }
    wchar_t k[32]; _snwprintf(k, 32, L"port_%s", G[g_sel].id); WritePrivateProfileStringW(L"CAGE", k, p, g_pIni);
    ReadVersion(g_sel); g_kvGame = -1; LoadEdits(); InvalidateRect(g_wnd, NULL, FALSE);
}

static DWORD WINAPI WaitThread(LPVOID p)
{
    HANDLE h = (HANDLE)p; WaitForSingleObject(h, INFINITE);
    DWORD code = 0; GetExitCodeProcess(h, &code); PostMessageW(g_wnd, WM_APP_EXITED, code, 0); return 0;
}

/* ---------------------------------------------------------------- UFC 1-4: freeze catcher
   The SacredFire engine prints "GPU wait stuck" when its GPU command thread has waited more than 2 s for the graphics
   card. The catcher then saves the call stack of every thread of sacredfire-game.exe beside the log
   (<log>.stacks.txt): which thread waits for which, the information a freeze needs to be fixed. */
typedef struct { uint64_t base, size; wchar_t name[64]; } ModInfo;
typedef HRESULT (WINAPI *GetThreadDescriptionFn)(HANDLE, PWSTR *);
static volatile LONG g_watchStop, g_watchSession;
static wchar_t g_frozenFile[MAX_PATH];

static void Out(FILE *f, const wchar_t *fmt, ...)
{
    wchar_t w[1200]; va_list ap; va_start(ap, fmt); _vsnwprintf(w, 1200, fmt, ap); va_end(ap); w[1199] = 0;
    char u[3600]; int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, u, sizeof(u), NULL, NULL); if (n > 1) fwrite(u, 1, (size_t)n - 1, f);
}
static int ModCmp(const void *a, const void *b) { uint64_t x = ((const ModInfo *)a)->base, y = ((const ModInfo *)b)->base; return x < y ? -1 : x > y; }
static int ModAt(const ModInfo *m, int n, uint64_t a)
{
    int lo = 0, hi = n - 1, found = -1;
    while (lo <= hi) { int mid = (lo + hi) / 2; if (m[mid].base <= a) { found = mid; lo = mid + 1; } else hi = mid - 1; }
    return found >= 0 && a < m[found].base + m[found].size ? found : -1;
}
static void Where(wchar_t *out, int cap, uint64_t a, const ModInfo *m, int n)
{
    int i = ModAt(m, n, a);
    if (i >= 0) _snwprintf(out, cap, L"%s+0x%llx", m[i].name, (unsigned long long)(a - m[i].base));
    else _snwprintf(out, cap, L"0x%llx", (unsigned long long)a);
    out[cap - 1] = 0;
}
/* the 8 bytes before a value found on a stack end with a call instruction: it is a return address */
static BOOL CallBefore(const unsigned char *b)
{
#define BB(k) b[8 - (k)]
    if (BB(5) == 0xE8) return TRUE;                                                   /* call rel32 */
    if (BB(6) == 0xFF && BB(5) == 0x15) return TRUE;                                  /* call [rip+disp32] */
    if (BB(2) == 0xFF && (BB(1) & 0xF8) == 0xD0) return TRUE;                         /* call reg */
    if ((BB(3) & 0xF0) == 0x40 && BB(2) == 0xFF && (BB(1) & 0xF8) == 0xD0) return TRUE; /* call r8-r15 */
    if (BB(2) == 0xFF && (BB(1) & 0xF8) == 0x10 && (BB(1) & 7) != 4 && (BB(1) & 7) != 5) return TRUE;   /* call [reg] */
    if (BB(3) == 0xFF && (BB(2) & 0xF8) == 0x50) return TRUE;                         /* call [reg+disp8] */
    if (BB(4) == 0xFF && BB(3) == 0x54) return TRUE;                                  /* call [reg+index+disp8] */
    if (BB(6) == 0xFF && (BB(5) & 0xF8) == 0x90) return TRUE;                         /* call [reg+disp32] */
    if ((BB(4) & 0xF0) == 0x40 && BB(3) == 0xFF && (BB(2) & 0xF8) == 0x50) return TRUE;
    if ((BB(7) & 0xF0) == 0x40 && BB(6) == 0xFF && (BB(5) & 0xF8) == 0x90) return TRUE;
    if ((BB(3) & 0xF0) == 0x40 && BB(2) == 0xFF && (BB(1) & 0xF8) == 0x10) return TRUE;
    return FALSE;
#undef BB
}
static DWORD FindProcess(const wchar_t *exe)
{
    DWORD pid = 0; HANDLE sn = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); PROCESSENTRY32W pe = {sizeof(pe)};
    if (sn == INVALID_HANDLE_VALUE) return 0;
    if (Process32FirstW(sn, &pe)) do { if (!_wcsicmp(pe.szExeFile, exe)) pid = pe.th32ProcessID; } while (Process32NextW(sn, &pe));
    CloseHandle(sn); return pid;
}
static BOOL SaveStacks(DWORD pid, const wchar_t *path, const char *why)
{
    HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hp) return FALSE;
    static ModInfo mods[1024]; int nm = 0;
    HANDLE ms = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (ms != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me = {sizeof(me)};
        if (Module32FirstW(ms, &me)) do {
            if (nm < 1024) { mods[nm].base = (uint64_t)(uintptr_t)me.modBaseAddr; mods[nm].size = me.modBaseSize; wcsncpy(mods[nm].name, me.szModule, 63); mods[nm].name[63] = 0; nm++; }
        } while (Module32NextW(ms, &me));
        CloseHandle(ms);
    }
    qsort(mods, nm, sizeof(ModInfo), ModCmp);
    FILE *f = _wfopen(path, L"wb"); if (!f) { CloseHandle(hp); return FALSE; }
    SYSTEMTIME t; GetLocalTime(&t);
    Out(f, L"Cage Launcher %s freeze catcher, %04d-%02d-%02d %02d:%02d:%02d: the threads of sacredfire-game.exe (pid %lu)\r\n", APP_VER, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, pid);
    { wchar_t w[300]; MultiByteToWideChar(CP_UTF8, 0, why, -1, w, 300); w[299] = 0; Out(f, L"Trigger: %s\r\n\r\nModules:\r\n", w); }
    for (int i = 0; i < nm; i++) Out(f, L"  %016llx %08llx %s\r\n", (unsigned long long)mods[i].base, (unsigned long long)mods[i].size, mods[i].name);
    GetThreadDescriptionFn gtd = (GetThreadDescriptionFn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetThreadDescription");
    static unsigned char stack[128 * 1024];
    HANDLE ts = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); THREADENTRY32 te = {sizeof(te)}; int nt = 0;
    if (ts != INVALID_HANDLE_VALUE && Thread32First(ts, &te)) do {
        if (te.th32OwnerProcessID != pid) continue;
        nt++;
        HANDLE ht = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_LIMITED_INFORMATION, FALSE, te.th32ThreadID);
        if (!ht) { Out(f, L"\r\nthread %lu: cannot open it (error %lu)\r\n", te.th32ThreadID, GetLastError()); continue; }
        wchar_t tname[128] = L"";
        if (gtd) { PWSTR d = NULL; if (SUCCEEDED(gtd(ht, &d)) && d) { wcsncpy(tname, d, 127); tname[127] = 0; LocalFree(d); } }
        CONTEXT ctx; memset(&ctx, 0, sizeof(ctx)); ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
        SIZE_T have = 0; BOOL got = FALSE;
        if (SuspendThread(ht) != (DWORD)-1) {
            got = GetThreadContext(ht, &ctx);
            if (got) while (have < sizeof(stack)) {   /* the stack from rsp up, page by page, until an unreadable one */
                uint64_t at = ctx.Rsp + have; SIZE_T want = 4096 - (SIZE_T)(at & 4095), n = 0;
                if (want > sizeof(stack) - have) want = sizeof(stack) - have;
                if (!ReadProcessMemory(hp, (LPCVOID)(uintptr_t)at, stack + have, want, &n) || !n) break;
                have += n;
            }
            ResumeThread(ht);
        }
        wchar_t w1[200];
        Out(f, L"\r\nthread %lu \"%s\" base priority %ld\r\n", te.th32ThreadID, tname, te.tpBasePri);
        if (!got) { Out(f, L"  (no context)\r\n"); CloseHandle(ht); continue; }
        Where(w1, 200, ctx.Rip, mods, nm);
        Out(f, L"  rip %s   rsp %llx rbp %llx rax %llx rcx %llx rdx %llx r8 %llx\r\n", w1, (unsigned long long)ctx.Rsp, (unsigned long long)ctx.Rbp,
            (unsigned long long)ctx.Rax, (unsigned long long)ctx.Rcx, (unsigned long long)ctx.Rdx, (unsigned long long)ctx.R8);
        uint64_t bp = ctx.Rbp;   /* frame pointer chain (the engine keeps frame pointers) */
        for (int d = 0; d < 40 && bp; d++) {
            uint64_t fr[2]; SIZE_T n = 0;
            if (!ReadProcessMemory(hp, (LPCVOID)(uintptr_t)bp, fr, sizeof(fr), &n) || n != sizeof(fr) || fr[0] <= bp) break;
            Where(w1, 200, fr[1], mods, nm); Out(f, L"  fp#%-2d %s\r\n", d, w1); bp = fr[0];
        }
        int shown = 0;           /* return addresses found on the stack */
        for (SIZE_T o = 0; o + 8 <= have && shown < 72; o += 8) {
            uint64_t v; memcpy(&v, stack + o, 8);
            int mi = ModAt(mods, nm, v); if (mi < 0 || v < mods[mi].base + 8) continue;
            unsigned char code[8]; SIZE_T n = 0;
            if (!ReadProcessMemory(hp, (LPCVOID)(uintptr_t)(v - 8), code, 8, &n) || n != 8 || !CallBefore(code)) continue;
            Out(f, L"  [rsp+%05llx] %s+0x%llx\r\n", (unsigned long long)o, mods[mi].name, (unsigned long long)(v - mods[mi].base)); shown++;
        }
        CloseHandle(ht);
    } while (Thread32Next(ts, &te));
    if (ts != INVALID_HANDLE_VALUE) CloseHandle(ts);
    Out(f, L"\r\n%d threads\r\n", nt);
    fclose(f); CloseHandle(hp); return nt > 0;
}
typedef struct { int game; LONG session; FILETIME start; } WatchArgs;
static DWORD WINAPI FreezeWatch(LPVOID p)
{
    WatchArgs a = *(WatchArgs *)p; free(p);
    wchar_t la[MAX_PATH] = L"", dir[MAX_PATH], logs[MAX_PATH], log[MAX_PATH] = L"";
    GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); Join(dir, la, G[a.game].data); Join(logs, dir, L"logs");
    ULARGE_INTEGER s0; s0.LowPart = a.start.dwLowDateTime; s0.HighPart = a.start.dwHighDateTime; s0.QuadPart -= 30ull * 10000000ull;
    LONGLONG offset = 0; DWORD stuckAt = 0; char carry[64] = ""; char why[200] = "";
    static char buf[1 << 20];
    while (!g_watchStop && g_watchSession == a.session) {
        Sleep(1000);
        if (!log[0]) {   /* the log this launch writes: the newest one created after the start */
            wchar_t pat[MAX_PATH]; Join(pat, logs, L"*.log"); WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
            ULARGE_INTEGER best = {0};
            if (h != INVALID_HANDLE_VALUE) {
                do { ULARGE_INTEGER c; c.LowPart = fd.ftCreationTime.dwLowDateTime; c.HighPart = fd.ftCreationTime.dwHighDateTime;
                     if (c.QuadPart >= s0.QuadPart && c.QuadPart > best.QuadPart) { best = c; Join(log, logs, fd.cFileName); } } while (FindNextFileW(h, &fd));
                FindClose(h);
            }
            if (!log[0]) continue;
        }
        HANDLE f = CreateFileW(log, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
        if (f == INVALID_HANDLE_VALUE) continue;
        for (;;) {
            LARGE_INTEGER at; at.QuadPart = offset; SetFilePointerEx(f, at, NULL, FILE_BEGIN);
            size_t keep = strlen(carry); memcpy(buf, carry, keep);
            DWORD got = 0; if (!ReadFile(f, buf + keep, (DWORD)(sizeof(buf) - keep - 1), &got, NULL) || !got) break;
            offset += got; buf[keep + got] = 0;
            for (size_t i = keep; i < keep + got; i++) if (!buf[i]) buf[i] = ' ';   /* stray NUL bytes would end the search early */
            char *hit = strstr(buf, "GPU wait stuck");
            if (hit && !stuckAt) { stuckAt = GetTickCount(); char *e = strchr(hit, '\n'); size_t n = e ? (size_t)(e - hit) : strlen(hit); if (n > 190) n = 190; memcpy(why, hit, n); why[n] = 0; if (n && why[n - 1] == '\r') why[n - 1] = 0; }
            size_t tot = keep + got, c = tot > 40 ? 40 : tot; memcpy(carry, buf + tot - c, c); carry[c] = 0;
        }
        CloseHandle(f);
        if (stuckAt && GetTickCount() - stuckAt >= 5000) {   /* 5 s later: the engine's own wait report is in the log too */
            DWORD pid = FindProcess(L"sacredfire-game.exe");
            wchar_t out[MAX_PATH]; wcscpy(out, log); wchar_t *dot = wcsrchr(out, L'.'); if (dot) *dot = 0; wcsncat(out, L".stacks.txt", MAX_PATH - wcslen(out) - 1);
            BOOL ok = pid && SaveStacks(pid, out, why);
            if (ok) { wcscpy(g_frozenFile, out); PostMessageW(g_wnd, WM_APP_FROZEN, (WPARAM)a.game, 0); }
            break;
        }
    }
    return 0;
}
/* ---------------------------------------------------------------- this PC (for the automatic settings of UFC 1-4)
   The video card through Vulkan (name, vendor, memory, and whether the CPU can see all of its memory: Resizable BAR,
   which Intel Arc cards need to run well), processor threads, RAM and the monitor. */
typedef struct { wchar_t name[256]; uint32_t vendor, type; uint64_t vram, bar; } PcGpu;
static PcGpu g_gpu; static BOOL g_haveGpu; static int g_threads, g_ramGb, g_monW, g_monH, g_monHz;
typedef void *(WINAPI *PFN_GIPA_)(void *, const char *);
typedef struct { int sType; const void *pNext; const char *app; uint32_t appVer; const char *eng; uint32_t engVer; uint32_t api; } VkAppInfo__;
typedef struct { int sType; const void *pNext; uint32_t flags; const VkAppInfo__ *app; uint32_t nl; const char *const *l; uint32_t ne; const char *const *e; } VkInstInfo__;
static void DetectPc(void)
{
    SYSTEM_INFO si; GetSystemInfo(&si); g_threads = (int)si.dwNumberOfProcessors;
    MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms); g_ramGb = (int)((ms.ullTotalPhys + (512ull << 20)) >> 30);
    DEVMODEW dm = {.dmSize = sizeof(dm)}; EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm);
    g_monW = dm.dmPelsWidth ? (int)dm.dmPelsWidth : 1920; g_monH = dm.dmPelsHeight ? (int)dm.dmPelsHeight : 1080; g_monHz = dm.dmDisplayFrequency > 1 ? (int)dm.dmDisplayFrequency : 60;
    g_haveGpu = FALSE;
    HMODULE vk = LoadLibraryW(L"vulkan-1.dll"); if (!vk) return;
    PFN_GIPA_ gipa = (PFN_GIPA_)(void *)GetProcAddress(vk, "vkGetInstanceProcAddr");
    int (WINAPI *create)(const VkInstInfo__ *, const void *, void **) = gipa ? (int (WINAPI *)(const VkInstInfo__ *, const void *, void **))gipa(NULL, "vkCreateInstance") : NULL;
    VkAppInfo__ ai = {0, NULL, "CageLauncher", 1, "none", 1, (1u << 22) | (1u << 12)}; VkInstInfo__ ii = {1, NULL, 0, &ai, 0, NULL, 0, NULL}; void *inst = NULL;
    if (!create || create(&ii, NULL, &inst) != 0 || !inst) { FreeLibrary(vk); return; }
    int (WINAPI *enumDev)(void *, uint32_t *, void **) = (int (WINAPI *)(void *, uint32_t *, void **))gipa(inst, "vkEnumeratePhysicalDevices");
    void (WINAPI *props)(void *, void *) = (void (WINAPI *)(void *, void *))gipa(inst, "vkGetPhysicalDeviceProperties");
    void (WINAPI *mem)(void *, void *) = (void (WINAPI *)(void *, void *))gipa(inst, "vkGetPhysicalDeviceMemoryProperties");
    void (WINAPI *destroy)(void *, const void *) = (void (WINAPI *)(void *, const void *))gipa(inst, "vkDestroyInstance");
    uint32_t n = 8; void *devs[8]; uint64_t bestScore = 0;
    if (enumDev && props && mem && enumDev(inst, &n, devs) >= 0) {
        static unsigned char buf[8192];
        for (uint32_t i = 0; i < n && i < 8; i++) {
            PcGpu c = {0}; memset(buf, 0, sizeof(buf)); props(devs[i], buf);
            c.vendor = *(uint32_t *)(buf + 8); c.type = *(uint32_t *)(buf + 16); MultiByteToWideChar(CP_UTF8, 0, (char *)buf + 20, -1, c.name, 256);
            memset(buf, 0, sizeof(buf)); mem(devs[i], buf);
            uint32_t nt = *(uint32_t *)buf, nh = *(uint32_t *)(buf + 260);
            for (uint32_t h = 0; h < nh && h < 16; h++) { uint64_t size = *(uint64_t *)(buf + 264 + h * 16); uint32_t fl = *(uint32_t *)(buf + 272 + h * 16); if ((fl & 1) && size > c.vram) c.vram = size; }
            for (uint32_t t = 0; t < nt && t < 32; t++) {   /* device-local + host-visible memory: its heap is the BAR window */
                uint32_t fl = *(uint32_t *)(buf + 4 + t * 8), hi = *(uint32_t *)(buf + 8 + t * 8);
                if ((fl & 3) == 3 && hi < 16) { uint64_t hs = *(uint64_t *)(buf + 264 + hi * 16); if (hs > c.bar) c.bar = hs; }
            }
            uint64_t score = c.vram + (c.type == 2 ? (1ull << 40) : 0);
            if (!g_haveGpu || score > bestScore) { g_gpu = c; bestScore = score; g_haveGpu = TRUE; }
        }
    }
    if (destroy) destroy(inst, NULL);
    FreeLibrary(vk);
}
static BOOL GpuIsArc(void) { return g_haveGpu && g_gpu.vendor == 0x8086 && g_gpu.type == 2; }
/* Resizable BAR on: the CPU-visible window of video memory is (nearly) all of it, not the old 256 MB */
static BOOL GpuHasRebar(void) { return g_haveGpu && g_gpu.bar > (1ull << 30); }
static int PcTier(void)   /* 2 strong, 1 weak, 0 very weak: the same rule as UFC5.exe */
{
    int vmb = g_haveGpu ? (int)(g_gpu.vram >> 20) : 0;
    if (g_threads <= 4 || (vmb && vmb < 3800) || g_ramGb < 12) return 0;
    if (g_threads < 12 || (vmb && vmb < 7500) || g_ramGb < 16) return 1;
    return 2;
}
static void PcSig(wchar_t *out, int n)
{
    _snwprintf(out, n, L"%d|%d|%s|%llu|%dx%d@%d", g_threads, g_ramGb, g_haveGpu ? g_gpu.name : L"-", g_haveGpu ? (unsigned long long)(g_gpu.vram >> 20) : 0ull, g_monW, g_monH, g_monHz);
    out[n - 1] = 0;
}
/* the best settings of UFC 1-4 for this PC (what the ports' notes say holds on these games):
   - FPS: UFC 1, 2 and 4 count game time per frame, so 60 (UFC 1 fights stay at its own 30); UFC 3 runs fights at any
     rate, so a strong PC on a 120/144 Hz monitor gets 120/144;
   - resolution: 1080p; very weak PCs 720p with FSR 1.0 to the monitor and sharpening, "ultra" below 4 threads/3 GB;
   - PS4 Pro mode only on strong NVIDIA/AMD cards (UFC 2-4; UFC 1 has none);
   - Intel Arc: VSync (Fifo), the present mode its driver always has, and never PS4 Pro mode. */
static void AutoConfig4(int g)
{
    int t = PcTier(); BOOL arc = GpuIsArc(), nvamd = g_haveGpu && (g_gpu.vendor == 0x10DE || g_gpu.vendor == 0x1002);
    int vmb = g_haveGpu ? (int)(g_gpu.vram >> 20) : 0;
    int fps = 60;
    if (g == 2 && t == 2 && g_monHz >= 120) fps = g_monHz >= 144 ? 144 : 120;
    PutInt(g, L"fullscreen", 1); PutInt(g, L"skip_movies", 1); PutInt(g, L"low_latency", 0);
    PutInt(g, L"fps_limit", fps); PutInt(g, L"present_mode", arc ? 1 : 0);
    if (t == 0) {
        PutInt(g, L"resolution", 0); PutInt(g, L"display", g_monH >= 1080 ? 1 : 0); PutInt(g, L"upscaler", 1); PutInt(g, L"sharpness", 2);
        PutInt(g, L"ultra", (g_threads <= 4 || (vmb && vmb < 3000)) ? 1 : 0); PutInt(g, L"neo", 0);
    } else {
        PutInt(g, L"resolution", 1); PutInt(g, L"display", 0); PutInt(g, L"upscaler", 0); PutInt(g, L"sharpness", 0); PutInt(g, L"ultra", 0);
        PutInt(g, L"neo", (g != 0 && t == 2 && nvamd && vmb >= 7500) ? 1 : 0);
    }
}
/* on the first start, and again when the PC changes (other card, more RAM, other monitor): every installed UFC 1-4 port
   gets the settings above. A port that was already set up by hand before this version keeps its settings. */
static int AutoConfigAll(BOOL force)
{
    wchar_t sig[400]; PcSig(sig, 400); int done = 0;
    for (int g = 0; g < 4; g++) {
        if (!PortOk(g)) continue;
        wchar_t k[32], old[400]; _snwprintf(k, 32, L"auto_%s", G[g].id);
        GetPrivateProfileStringW(L"CAGE", k, L"", old, 400, g_pIni);
        wchar_t sf[MAX_PATH]; SfPath(g, sf);
        BOOL had = FileExists(sf);
        if (!force && !wcscmp(old, sig)) continue;
        if (force || !had || old[0]) { g_kvGame = -1; AutoConfig4(g); done++; }
        WritePrivateProfileStringW(L"CAGE", k, sig, g_pIni);
    }
    g_kvGame = -1;
    return done;
}
static void PcSummary(wchar_t *out, int n)
{
    static const wchar_t *const TN[2][3] = {{L"PC muito fraco", L"PC fraco", L"PC forte"}, {L"very weak PC", L"weak PC", L"strong PC"}};
    _snwprintf(out, n, L"%s: %s (%llu GB), %d threads, %d GB RAM, %dx%d %d Hz%s", TN[g_lang][PcTier()], g_haveGpu ? g_gpu.name : L"GPU?",
               g_haveGpu ? (unsigned long long)((g_gpu.vram + (512ull << 20)) >> 30) : 0ull, g_threads, g_ramGb, g_monW, g_monH, g_monHz,
               GpuIsArc() ? (g_lang ? L", Intel Arc mode" : L", modo Intel Arc") : L"");
    out[n - 1] = 0;
}

/* UFC 5 configured for this PC as soon as Cage Launcher opens: UFC5.exe --auto-config (no window) reads the processor,
   RAM and video card, and only changes UFC5.ini the first time or when that hardware changed (exit code 1) */
static HANDLE g_autoProc; static int g_autoDone4;
static DWORD WINAPI AutoCfgThread(void *p)
{
    wchar_t exe[MAX_PATH], dir[MAX_PATH], cmd[MAX_PATH + 48]; PortExe(4, exe); PortDir(4, dir);
    if (!FileExists(exe)) { PostMessageW(g_wnd, WM_APP_AUTOCFG, 0, 0); return 0; }
    _snwprintf(cmd, MAX_PATH + 48, p ? L"\"%s\" --auto-config --force" : L"\"%s\" --auto-config", exe);
    STARTUPINFOW si = {sizeof(si)}; si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE; PROCESS_INFORMATION pi = {0};
    if (!CreateProcessW(exe, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS, NULL, dir, &si, &pi)) { PostMessageW(g_wnd, WM_APP_AUTOCFG, 0, 0); return 0; }
    CloseHandle(pi.hThread); g_autoProc = pi.hProcess;
    DWORD code = 0;
    if (WaitForSingleObject(pi.hProcess, 30000) == WAIT_OBJECT_0) GetExitCodeProcess(pi.hProcess, &code);
    PostMessageW(g_wnd, WM_APP_AUTOCFG, code, 0);
    return 0;
}

static BOOL StartPort(int g, BOOL play)
{
    if (g == 4 && g_autoProc) WaitForSingleObject(g_autoProc, 15000);   /* never start while it is still writing UFC5.ini */
    wchar_t exe[MAX_PATH], dir[MAX_PATH], cmd[MAX_PATH + 32]; PortExe(g, exe); PortDir(g, dir);
    if (!FileExists(exe)) { Msg(C_BAD, T(S_NOTFOUND), G[g].exe); return FALSE; }
    /* UFC 5: UFC5.exe runs hidden (--headless), only to start the engine with the settings chosen here, write the log
       and measure the FPS; no window of its own and no FPS overlay over the game */
    _snwprintf(cmd, MAX_PATH + 32, play ? (G[g].ps5 ? L"\"%s\" --play --headless" : L"\"%s\" --play") : L"\"%s\"", exe);
    if (play && G[g].ps5) AllowSetForegroundWindow(ASFW_ANY);   /* so the game window can take the focus from the launcher */
    STARTUPINFOW si = {sizeof(si)}; PROCESS_INFORMATION pi = {0};
    /* UFC 1: the fight's compute shader 0xb61e59a3b11b03ad is fixed by a corrected SPIR-V in the engine's shader patch
       folder (user\shader\patch), so it is no longer skipped: skipping it left its hit list unwritten for the shaders
       after it. The engine runs on the classic single-threaded GPU path: with the Vulkan recording threads and the draw
       pipe, a fight froze when a submission handed to them never reached the GPU while the GPU command thread waited for
       it inside a readback (logs of 2026-10-19). ufc1_stable_engine=0 in cage.ini turns it off for a test. */
    static const wchar_t *const envK[] = {L"BB_VK_RECORD_THREAD", L"BB_DRAW_PIPE"}, *const envV[] = {L"0", L"0"};
    BOOL stable = g == 0 && GetPrivateProfileIntW(L"CAGE", L"ufc1_stable_engine", 1, g_pIni);
    wchar_t oldV[2][256]; DWORD had[2] = {0, 0};
    if (stable) for (int i = 0; i < 2; i++) { had[i] = GetEnvironmentVariableW(envK[i], oldV[i], 256); SetEnvironmentVariableW(envK[i], envV[i]); }
    /* started as a plain process: inside a job object the UFC 5 engine renders the fight black */
    FILETIME started; GetSystemTimeAsFileTime(&started);
    BOOL okp = CreateProcessW(exe, cmd, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi); DWORD perr = GetLastError();
    if (stable) for (int i = 0; i < 2; i++) SetEnvironmentVariableW(envK[i], had[i] && had[i] < 256 ? oldV[i] : NULL);
    if (!okp) { Msg(C_BAD, T(S_CANTSTART), G[g].exe, perr); return FALSE; }
    if (play) {
        g_proc = pi.hProcess; g_procGame = g;
        CloseHandle(CreateThread(NULL, 0, WaitThread, g_proc, 0, NULL));
        if (!G[g].ps5) {   /* UFC 1-4: catch a freeze of the engine */
            WatchArgs *wa = (WatchArgs *)malloc(sizeof(WatchArgs));
            if (wa) { wa->game = g; wa->start = started; wa->session = InterlockedIncrement(&g_watchSession); InterlockedExchange(&g_watchStop, 0); g_frozenFile[0] = 0;
                      HANDLE th = CreateThread(NULL, 0, FreezeWatch, wa, 0, NULL); if (th) CloseHandle(th); else free(wa); }
        }
        Msg(C_GOOD, T(S_STARTING), G[g].name);
        if (G[g].ps5) ShowWindow(g_wnd, SW_MINIMIZE);   /* out of the way: the fullscreen game must stay in front */
    } else CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
}
static void Play(void)
{
    if (g_proc) {   /* STOP: close the port and every process it started (the game engine) */
        DWORD root = GetProcessId(g_proc), kids[64]; int nk = 0, done = 0;
        kids[nk++] = root;
        while (done < nk) {
            DWORD parent = kids[done++];
            HANDLE sn = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); PROCESSENTRY32W pe = {sizeof(pe)};
            if (sn != INVALID_HANDLE_VALUE) {
                if (Process32FirstW(sn, &pe)) do { if (pe.th32ParentProcessID == parent && nk < 64) kids[nk++] = pe.th32ProcessID; } while (Process32NextW(sn, &pe));
                CloseHandle(sn);
            }
        }
        for (int i = nk - 1; i >= 0; i--) { HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, kids[i]); if (h) { TerminateProcess(h, 1); CloseHandle(h); } }
        return;
    }
    SaveEdits();
    int st = Status(g_sel);
    if (st == S_NOPORT) { Msg(C_BAD, T(S_NOTFOUND), G[g_sel].exe); return; }
    if (st == S_PICK) { Msg(C_ORANGE, L"%s", T(S_CHOOSEFIRST)); return; }
    if (!G[g_sel].ps5 && GetInt(g_sel, L"ultra", 0)) Msg(C_SUB, L"%s", T(S_ULTRAON));
    if (StartPort(g_sel, TRUE)) ShowWindow(g_wnd, SW_MINIMIZE);
}
static void OnExited(DWORD code)
{
    int g = g_procGame; CloseHandle(g_proc); g_proc = NULL; g_procGame = -1;
    InterlockedExchange(&g_watchStop, 1);
    if (g_job) { CloseHandle(g_job); g_job = NULL; }
    ShowWindow(g_wnd, SW_RESTORE); SetForegroundWindow(g_wnd);
    g_kvGame = -1;
    wchar_t last[400] = L"", cal[400] = L""; if (g >= 0 && G[g].ps5) { GetStr(4, L"last_session", last, 400, L""); GetStr(4, L"calib_status", cal, 400, L""); }
    if (g >= 0 && G[g].ps5 && cal[0] && (code == 0 || code == 1) && wcsstr(cal, L"Calibra")) { Msg(C_GOOD, L"%s", cal); if (wcsstr(cal, L"conclu")) PutStr(4, L"calib_status", L""); }
    else if (code == 0 || code == 1) { if (last[0]) Msg(C_GOOD, T(S_SESSION), last); else Msg(C_SUB, L"%s", T(S_CLOSED)); }
    else Msg(C_BAD, T(S_CLOSEDERR), code);
    LoadEdits();
}
static void OpenLogs(void)
{
    wchar_t p[MAX_PATH];
    if (G[g_sel].ps5) { wchar_t d[MAX_PATH]; PortDir(g_sel, d); Join(p, d, L"bin\\_Logs"); if (!DirExists(p)) Join(p, d, L"emulator\\_Logs"); }
    else { wchar_t la[MAX_PATH] = L"", d[MAX_PATH]; GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); Join(d, la, G[g_sel].data); Join(p, d, L"logs"); if (!DirExists(p)) wcscpy(p, d); }
    if (!DirExists(p)) { Msg(C_SUB, L"%s", T(S_NOLOGS)); return; }
    ShellExecuteW(g_wnd, L"open", p, NULL, NULL, SW_SHOWNORMAL);
}
static void About(void)
{
    MessageBoxW(g_wnd, g_lang ?
        L"CAGE LAUNCHER " APP_VER L"\n\nOne launcher for the PC ports of EA SPORTS UFC, UFC 2, UFC 3, UFC 4 and UFC 5.\n\n"
        L"The ports are built on open-source projects: SacredFire / bbport (GNSB), shadPS4, KytyPS5 and the libraries listed in each port's licenses folder. "
        L"GPL-2.0-or-later, source code in the src folder.\n\nNot affiliated with Electronic Arts, the UFC or Sony. You need your own copy of each game." :
        L"CAGE LAUNCHER " APP_VER L"\n\nUm launcher para os ports para PC do EA SPORTS UFC, UFC 2, UFC 3, UFC 4 e UFC 5.\n\n"
        L"Os ports são feitos sobre projetos de código aberto: SacredFire / bbport (GNSB), shadPS4, KytyPS5 e as bibliotecas listadas na pasta licenses de cada port. "
        L"GPL-2.0-or-later, código-fonte na pasta src.\n\nSem afiliação com a Electronic Arts, o UFC ou a Sony. É preciso ter a sua própria cópia de cada jogo.",
        L"CAGE LAUNCHER", MB_OK | MB_ICONINFORMATION);
}

static void DropdownMenu(int idx)
{
    if (idx < 9 && !G[g_sel].ps5 && idx >= 6) return;
    const Field *f = idx < 9 ? &Vid(g_sel)[idx] : &Gp(g_sel)[idx - 9];
    HMENU m = CreatePopupMenu(); int cur = OptIndex(f, GetInt(g_sel, f->key, f->def));
    for (int i = 0; i < f->nopts; i++) AppendMenuW(m, MF_STRING | (i == cur ? MF_CHECKED : 0), 1 + i, OptName(&f->opts[i]));
    RECT r = {0}; for (int i = 0; i < g_nhits; i++) if (g_hits[i].id == H_DROP + idx) r = g_hits[i].r;
    POINT pt = {r.left, r.bottom + SC(2)}; ClientToScreen(g_wnd, &pt);
    int c = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, g_wnd, NULL);
    DestroyMenu(m);
    if (c <= 0) return;
    int val = f->opts[c - 1].value;
    if (G[g_sel].ps5 && !wcscmp(f->key, L"calibrate") && val) {
        PutInt(4, L"calib_arm", 0); PutInt(4, L"calib_stage", 0); PutInt(4, L"calib_a_sum", 0); PutInt(4, L"calib_a_n", 0); PutInt(4, L"calib_b_sum", 0); PutInt(4, L"calib_b_n", 0);
        Msg(C_GOOD, g_lang ? L"Calibration on: play up to 12 fights (30 s of fighting or more each); the port tries 6 engine settings one at a time (one fight with, one without), and keeps what is faster."
                           : L"Calibração ligada: jogue até 12 lutas (30 s de luta ou mais cada). O port testa 6 ajustes do motor, um de cada vez (uma luta com, outra sem), e fica com o que for mais rápido.");
    }
    if (G[g_sel].ps5 && !wcscmp(f->key, L"profile")) ApplyProfile5(val);
    else { PutInt(g_sel, f->key, val); if (G[g_sel].ps5 && f->perf) PutInt(4, L"profile", 4); }
    if (G[g_sel].ps5) PutInt(4, L"configured", 1);
    if (G[g_sel].ps5 && !wcscmp(f->key, L"vblank") && val)
        Msg(C_SUB, g_lang ? L"%d FPS: frames come out in %.1f ms steps and menus pass 60. If the round clock runs fast in a fight, go back to 60."
                          : L"%d FPS: quadros em passos de %.1f ms e menus acima de 60. Se o relógio do round correr rápido na luta, volte para 60.",
            val == 1 ? 120 : 144, val == 1 ? 8.3 : 6.9);
    if (G[g_sel].ps5 && !wcscmp(f->key, L"framegen") && val)
        Msg(C_SUB, val == 1 ? (g_lang ? L"In Lossless Scaling pick LSFG, Adaptive, target 144. The port opens it and presses Ctrl+Alt+S once the game is up."
                                      : L"No Lossless Scaling: LSFG, Adaptive, alvo 144. O port abre o LS e aperta Ctrl+Alt+S com o jogo na tela.")
                            : (g_lang ? L"The port only opens Lossless Scaling: make an LS profile for UFC5-Game.exe with Auto Scale on."
                                      : L"O port só abre o Lossless Scaling: faça nele um perfil do UFC5-Game.exe com Auto Scale."));
    InvalidateRect(g_wnd, NULL, FALSE);
}
/* UFC 5's "Advanced settings": the port's own window (every engine option), a one-session full diagnostic, Lossless Scaling */
static BOOL StartPort(int g, BOOL play);
static DWORD WINAPI AutoCfgThread(void *p);
static void AdvMenu4(void)
{
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING, 1, g_lang ? L"Best settings for this PC (automatic)" : L"Melhores configurações para este PC (automático)");
    AppendMenuW(m, MF_STRING, 2, g_lang ? L"All options (the port's own window)" : L"Todas as opções (janela do port)");
    RECT r = {0}; for (int i = 0; i < g_nhits; i++) if (g_hits[i].id == H_ADV) r = g_hits[i].r;
    POINT pt = {r.left, r.bottom + SC(2)}; ClientToScreen(g_wnd, &pt);
    int c = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, g_wnd, NULL);
    DestroyMenu(m);
    if (c == 1) { DetectPc(); g_kvGame = -1; AutoConfig4(g_sel); g_kvGame = -1; wchar_t sum[400]; PcSummary(sum, 400);
                  Msg(C_GOOD, g_lang ? L"%s set up for this PC (%s)." : L"%s configurado para este PC (%s).", G[g_sel].name, sum); LoadEdits(); }
    else if (c == 2) StartPort(g_sel, FALSE);
    InvalidateRect(g_wnd, NULL, FALSE);
}
static void Ufc5AdvMenu(void)
{
    HMENU m = CreatePopupMenu(); BOOL diag = GetInt(4, L"diag_full_next", 0) != 0;
    AppendMenuW(m, MF_STRING, 4, g_lang ? L"Best settings for this PC (automatic)" : L"Melhores configurações para este PC (automático)");
    AppendMenuW(m, MF_STRING, 1, g_lang ? L"All UFC 5 options (the port's own window)" : L"Todas as opções do UFC 5 (janela do port)");
    AppendMenuW(m, MF_STRING | (diag ? MF_CHECKED : 0), 2, g_lang ? L"Full diagnostic in the next session" : L"Diagnóstico completo na próxima partida");
    AppendMenuW(m, MF_STRING, 3, g_lang ? L"Open Lossless Scaling (frame generation)" : L"Abrir o Lossless Scaling (geração de quadros)");
    RECT r = {0}; for (int i = 0; i < g_nhits; i++) if (g_hits[i].id == H_ADV) r = g_hits[i].r;
    POINT pt = {r.left, r.bottom + SC(2)}; ClientToScreen(g_wnd, &pt);
    int c = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, g_wnd, NULL);
    DestroyMenu(m);
    if (c == 1) StartPort(4, FALSE);
    else if (c == 2) {
        PutInt(4, L"diag_full_next", !diag);
        Msg(C_SUB, !diag ? (g_lang ? L"Full diagnostic on for the next session: play at least 1 minute of a fight, then send the 2 newest files of the logs folder."
                                   : L"Diagnóstico completo na próxima partida: jogue 1 minuto de luta ou mais e mande os 2 arquivos novos da pasta de logs.")
                         : (g_lang ? L"Full diagnostic off." : L"Diagnóstico completo desligado."));
    }
    else if (c == 3) ShellExecuteW(g_wnd, L"open", L"steam://rungameid/993090", NULL, NULL, SW_SHOWNORMAL);
    else if (c == 4) { HANDLE th = CreateThread(NULL, 0, AutoCfgThread, (void *)1, 0, NULL); if (th) CloseHandle(th);
                       Msg(C_SUB, g_lang ? L"Reading this PC..." : L"Lendo este PC..."); }
    InvalidateRect(g_wnd, NULL, FALSE);
}
static void ToggleSwitch(int idx)
{
    int n; const Field *f = &Sw(g_sel, &n)[idx]; if (idx >= n) return;
    int v = !GetInt(g_sel, f->key, f->def); PutInt(g_sel, f->key, v);
    if (G[g_sel].ps5) { if (f->perf) PutInt(4, L"profile", 4); PutInt(4, L"configured", 1); }
    if (G[g_sel].ps5 && !wcscmp(f->key, L"large_pages") && v)
        Msg(C_SUB, g_lang ? L"Large pages: on the first PLAY the port asks for administrator rights once; then sign out of Windows and back in."
                          : L"Large pages: no primeiro JOGAR o port pede permissão de administrador uma vez; depois saia e entre de novo no Windows.");
    if (!G[g_sel].ps5 && !wcscmp(f->key, L"ultra") && v) Msg(C_SUB, L"%s", T(S_ULTRAON));
    InvalidateRect(g_wnd, NULL, FALSE);
}

/* ---------------------------------------------------------------- window */
static void MakeFonts(void)
{
    #define F(h, w, face) CreateFontW(-SC(h), 0, 0, 0, w, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, face)
    g_fTitle = F(46, FW_BOLD, L"Bahnschrift"); g_fCard = F(20, FW_SEMIBOLD, L"Bahnschrift");
    g_fText = F(15, FW_NORMAL, L"Segoe UI"); g_fSmall = F(13, FW_NORMAL, L"Segoe UI"); g_fSection = F(13, FW_SEMIBOLD, L"Bahnschrift");
    g_fPlay = F(24, FW_BOLD, L"Bahnschrift"); g_fBtn = F(14, FW_SEMIBOLD, L"Bahnschrift"); g_fFoot = F(13, FW_NORMAL, L"Bahnschrift");
    g_fLabel = F(13, FW_NORMAL, L"Bahnschrift"); g_fTag = F(11, FW_BOLD, L"Bahnschrift");
    g_fLogo = F(50, FW_BOLD, L"Bahnschrift"); g_fLogoSub = F(13, FW_SEMIBOLD, L"Bahnschrift");
}

static LRESULT CALLBACK WndProc(HWND w, UINT m, WPARAM wp, LPARAM lp)
{
    switch (m) {
    case WM_CREATE: g_wnd = w; return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: { PAINTSTRUCT ps; HDC dc = BeginPaint(w, &ps); Paint(dc); EndPaint(w, &ps); return 0; }
    case WM_CTLCOLOREDIT: { HDC dc = (HDC)wp; SetBkColor(dc, C_FIELD); SetTextColor(dc, C_TEXT); return (LRESULT)g_brField; }
    case WM_MOUSEMOVE: {
        int h = HitAt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        if (h != g_hot) { g_hot = h; InvalidateRect(w, NULL, FALSE); TRACKMOUSEEVENT t = {sizeof(t), TME_LEAVE, w, 0}; TrackMouseEvent(&t); }
        return 0;
    }
    case WM_MOUSELEAVE: if (g_hot != -1) { g_hot = -1; InvalidateRect(w, NULL, FALSE); } return 0;
    case WM_SETCURSOR: if (LOWORD(lp) == HTCLIENT && g_hot >= 0) { SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_HAND)); return TRUE; } break;
    case WM_LBUTTONDOWN: {
        SetFocus(w);
        int h = HitAt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        if (h >= H_CARD && h < H_CARD + NGAMES) Select(h - H_CARD);
        else if (h >= H_DROP && h < H_DROP + 11) DropdownMenu(h - H_DROP);
        else if (h >= H_SWITCH && h < H_SWITCH + 6) ToggleSwitch(h - H_SWITCH);
        else switch (h) {
            case H_BROWSE: Browse(); break;
            case H_PORT: ChangePort(); break;
            case H_ADV: SaveEdits();
                if (g_proc && g_procGame == g_sel) Msg(C_SUB, g_lang ? L"The game is running: close it first." : L"O jogo está aberto: feche o jogo primeiro.");
                else if (G[g_sel].ps5) Ufc5AdvMenu();
                else AdvMenu4();
                InvalidateRect(w, NULL, FALSE); break;
            case H_LOGS: OpenLogs(); break;
            case H_PLAY: Play(); break;
            case H_ABOUT: About(); break;
            case H_EN: case H_PT: g_lang = h == H_EN; WritePrivateProfileStringW(L"CAGE", L"language", g_lang ? L"0" : L"1", g_pIni); g_msg[0] = 0; InvalidateRect(w, NULL, FALSE); break;
        }
        return 0;
    }
    case WM_COMMAND: if (HIWORD(wp) == EN_KILLFOCUS) { SaveEdits(); InvalidateRect(w, NULL, FALSE); } return 0;
    case WM_APP_EXITED: OnExited((DWORD)wp); return 0;
    case WM_APP_AUTOCFG:
        if (wp == 1 || g_autoDone4) {
            wchar_t sum[400]; PcSummary(sum, 400);
            Msg(C_GOOD, g_lang ? L"Best settings picked for this PC (%s%s): %s" : L"Melhores configurações escolhidas para este PC (%s%s): %s",
                wp == 1 ? L"UFC 5" : L"", (wp == 1 && g_autoDone4) ? L", UFC 1-4" : g_autoDone4 ? L"UFC 1-4" : L"", sum);
            g_autoDone4 = 0; g_kvGame = -1; InvalidateRect(w, NULL, FALSE);
        }
        if (GpuIsArc() && !GpuHasRebar())   /* the one thing an Arc card cannot do without */
            Msg(C_ORANGE, g_lang ? L"Intel Arc without Resizable BAR: turn on \"Re-Size BAR\" and \"Above 4G Decoding\" in the BIOS, or the card loses a lot of FPS."
                                 : L"Intel Arc sem Resizable BAR: ligue \"Re-Size BAR\" e \"Above 4G Decoding\" na BIOS, senão a placa perde muito FPS.");
        return 0;
    case WM_APP_FROZEN:   /* the engine stopped getting GPU work done: say so and where the diagnosis went */
        FlashWindow(w, TRUE);
        Msg(C_BAD, g_lang ? L"%s froze (the GPU stopped answering). The diagnosis was saved to the logs folder (%s). Click STOP to close it." :
                            L"O %s travou (a GPU parou de responder). O diagnóstico foi salvo na pasta de logs (%s). Clique em PARAR para fechar.",
            G[wp < NGAMES ? wp : 0].name, PathFindFileNameW(g_frozenFile));
        InvalidateRect(w, NULL, FALSE); return 0;
    case WM_ACTIVATE: if (LOWORD(wp) != WA_INACTIVE) { g_kvGame = -1; for (int i = 0; i < NGAMES; i++) ReadVersion(i); InvalidateRect(w, NULL, FALSE); } break;
    case WM_DPICHANGED: {
        g_dpi = HIWORD(wp); MakeFonts();
        SendMessageW(g_editGame, WM_SETFONT, (WPARAM)g_fText, TRUE); SendMessageW(g_editPlayer, WM_SETFONT, (WPARAM)g_fText, TRUE);
        RECT *r = (RECT *)lp; SetWindowPos(w, NULL, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE); return 0;
    }
    case WM_CLOSE: SaveEdits(); DestroyWindow(w); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}

/* dark context menus (uxtheme ordinals; ignored where unavailable) */
static void DarkMenus(void)
{
    HMODULE ux = LoadLibraryW(L"uxtheme.dll"); if (!ux) return;
    typedef int (WINAPI *SetMode)(int); typedef void (WINAPI *Flush)(void);
    SetMode sm = (SetMode)(void *)GetProcAddress(ux, MAKEINTRESOURCEA(135)); Flush fl = (Flush)(void *)GetProcAddress(ux, MAKEINTRESOURCEA(136));
    if (sm) sm(2); if (fl) fl();
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmdline, int show)
{
    (void)prev; (void)cmdline;
    g_inst = inst;
    HANDLE single = CreateMutexW(NULL, TRUE, L"CAGE_LAUNCHER_Single");
    if (GetLastError() == ERROR_ALREADY_EXISTS) { HWND o = FindWindowW(L"CageLauncher", NULL); if (o) { ShowWindow(o, SW_RESTORE); SetForegroundWindow(o); } return 0; }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX ic = {sizeof(ic), ICC_STANDARD_CLASSES}; InitCommonControlsEx(&ic);
    GdiplusStartupInput gi = {1, NULL, FALSE, FALSE}; GdiplusStartup(&g_gdip, &gi, NULL);
    DarkMenus();

    GetModuleFileNameW(NULL, g_exeDir, MAX_PATH); wchar_t *s = wcsrchr(g_exeDir, L'\\'); if (s) *s = 0;
    { wchar_t la[MAX_PATH] = L"", d[MAX_PATH]; GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); Join(d, la, L"CageLauncher"); CreateDirectoryW(d, NULL); Join(g_pIni, d, L"cage.ini"); }
    g_lang = GetPrivateProfileIntW(L"CAGE", L"language", 1, g_pIni) ? 0 : 1;   /* language=1 is PT-BR */
    wchar_t selId[16]; GetPrivateProfileStringW(L"CAGE", L"selected_game", L"UFC5", selId, 16, g_pIni);
    for (int i = 0; i < NGAMES; i++) { ReadVersion(i); if (!_wcsicmp(selId, G[i].id)) g_sel = i; }

    g_imgLogo = LoadPngRes(101);
    g_brField = CreateSolidBrush(C_FIELD);

    WNDCLASSEXW wc = {sizeof(wc)}; wc.lpfnWndProc = WndProc; wc.hInstance = inst; wc.lpszClassName = L"CageLauncher";
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW); wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(1));
    RegisterClassExW(&wc);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    POINT cur; GetCursorPos(&cur); HMONITOR mon = MonitorFromPoint(cur, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = {sizeof(mi)}; GetMonitorInfoW(mon, &mi);
    UINT dx = 96, dy = 96; HMODULE sh = LoadLibraryW(L"shcore.dll");
    if (sh) { typedef HRESULT (WINAPI *GDFM)(HMONITOR, int, UINT *, UINT *); GDFM f = (GDFM)(void *)GetProcAddress(sh, "GetDpiForMonitor"); if (f) f(mon, 0, &dx, &dy); }
    g_dpi = (int)dx;
    /* fit small screens: never taller than the work area */
    int wah = mi.rcWork.bottom - mi.rcWork.top;
    while (g_dpi > 96 && SC(BASE_H) + SC(60) > wah) g_dpi -= 8;
    MakeFonts();
    RECT r = {0, 0, SC(BASE_W), SC(BASE_H)}; AdjustWindowRectExForDpi(&r, style, FALSE, 0, g_dpi);
    int ww = r.right - r.left, wh = r.bottom - r.top;
    HWND w = CreateWindowExW(0, wc.lpszClassName, L"CAGE LAUNCHER", style,
                             mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - ww) / 2, mi.rcWork.top + max(0, (wah - wh) / 2), ww, wh, NULL, NULL, inst, NULL);
    BOOL dark = TRUE; DwmSetWindowAttribute(w, 20, &dark, sizeof(dark));
    COLORREF cap = C_BG; DwmSetWindowAttribute(w, 35 /* DWMWA_CAPTION_COLOR */, &cap, sizeof(cap));
    g_editGame = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 10, 10, w, (HMENU)300, inst, NULL);
    g_editPlayer = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 10, 10, w, (HMENU)301, inst, NULL);
    SendMessageW(g_editGame, WM_SETFONT, (WPARAM)g_fText, TRUE); SendMessageW(g_editPlayer, WM_SETFONT, (WPARAM)g_fText, TRUE);
    SendMessageW(g_editPlayer, EM_LIMITTEXT, 16, 0);
    LoadEdits();
    if (G[g_sel].ps5) { wchar_t last[200]; GetStr(4, L"last_session", last, 200, L""); if (last[0]) Msg(C_SUB, T(S_SESSION), last); }
    ShowWindow(w, show); UpdateWindow(w);
    DetectPc(); g_autoDone4 = AutoConfigAll(FALSE);
    { HANDLE th = CreateThread(NULL, 0, AutoCfgThread, NULL, 0, NULL); if (th) CloseHandle(th); }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    CloseHandle(single);
    return 0;
}
