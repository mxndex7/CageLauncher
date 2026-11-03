# UFC 2 native port 0.1.0 (test)

## 0.1.0-test7

- **Create Fighter crash fixed** (code 23 when confirming a new fighter): an image copy
  reached past the texture it wrote into and the graphics card stopped. Every image copy is
  now checked against both textures; the same protection is in UFC 3 and UFC 4.
- **Community patches** (by Lepexa and Antigravity), all off by default: 60+ FPS, resolutions,
  crowd off, shadows off, motion blur off, skinning method and more. See README-Windows.md
  ("Patches da comunidade") to turn one on.
- Better crash reports: a GPU loss now names the texture involved.

## 0.1.0-test6

- **Auto settings:** on the first start Cage Launcher reads your PC (graphics card and its memory,
  processor threads, RAM, monitor) and picks this game's settings. The new **Auto settings**
  button does it again any time. FSR stays off (native port: not needed); PS4 Pro mode only on
  strong NVIDIA / AMD cards.

## 0.1.0-test5

- **Louder, clearer sound.** The game mixes 7.1; on headphones and stereo speakers the port now
  folds it to 2 channels itself, without the attenuation that made voices and commentary quiet.
  New **Volume** setting (50% to 300%) in the launcher and Cage Launcher, with a limiter against
  clipping.
- **Steam Deck (Linux):** a Deck profile, on by default there: 1280x720 with FSR 1.0 to the
  800p screen, 2 frames queued (fewer stutters with the CPU and GPU at full clocks), 60 FPS cap.
  `STEAM_DECK_PROFILE=0` in the .conf turns it off.
- **ReShade** works (Vulkan): see README-Windows.md.
- **Better crash reports:** a crash inside the graphics driver (the "code 3221225477" error) now
  writes where it happened to the log, and slow scenes log which part of the frame costs the time.
- Since the last version: the Career FPS collapse (UFC 3: crash code 3 after loading your
  fighter) is fixed in all three games.

[Português](#português)

The first test build of EA SPORTS UFC 2 (PS4 `CUSA01968`, update 1.13) on the native port that
runs UFC 3 and UFC 4. It is built on the UFC 4 port 0.2.0, with its controller, save and frame
rate fixes.

- UFC 2 is recognized by the launcher (`UFC2.exe`, data in `%LOCALAPPDATA%\UFC2Native`).
- The system functions UFC 2 uses and the others did not: fibers (libSceFiber), the module list,
  two online queries.
- Windows and Linux packages; also included in Cage Launcher.

This is a test build: send the newest log (`Open logs folder`) from every run.

---

## Português

A primeira versão de teste do EA SPORTS UFC 2 (PS4 `CUSA01968`, update 1.13) no port nativo que
roda o UFC 3 e o UFC 4. Ela é feita sobre o port do UFC 4 0.2.0, com as correções de controles,
saves e FPS dele.

- O UFC 2 é reconhecido pelo launcher (`UFC2.exe`, dados em `%LOCALAPPDATA%\UFC2Native`).
- As funções do sistema que o UFC 2 usa e os outros não: fibers (libSceFiber), a lista de
  módulos, duas consultas online.
- Pacotes para Windows e Linux; também incluído no Cage Launcher.

É uma versão de teste: mande o log mais novo (`Open logs folder`) de cada vez que jogar.
