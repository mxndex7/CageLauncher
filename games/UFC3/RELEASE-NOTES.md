# UFC 3 native port 0.1.7 (Cage Launcher)

## 0.1.12

- **Safer texture copies:** every image copy, upload and tiling pass is checked against its
  textures and buffers before it reaches the graphics card (the fix for UFC 2's Create
  Fighter crash, code 23).
- **Community patches** (by Lepexa and Antigravity), all off by default: 60+ FPS, resolutions,
  crowd off, shadows off, motion blur off, skinning method and more. See README-Windows.md
  ("Patches da comunidade") to turn one on.
- Better crash reports: a GPU loss now names the texture involved.

## 0.1.11

- **Auto settings:** on the first start Cage Launcher reads your PC (graphics card and its memory,
  processor threads, RAM, monitor) and picks this game's settings. The new **Auto settings**
  button does it again any time. FSR stays off (native port: not needed); PS4 Pro mode only on
  strong NVIDIA / AMD cards.
- **Online over the Internet (experimental):** Host server asks your router (UPnP) to open its
  ports and shows your public address for your friend's Join server. Without UPnP (or behind
  carrier-grade NAT) the launcher says so: use a VPN, or Parsec for local co-op over the Internet.
  See docs/ufc3/ONLINE-HOSTING.md.

## 0.1.10

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

GNSB's UFC 3 port 0.1.6 with the fixes made for the UFC 4 port, built for Cage Launcher.
Saves, settings and the shader cache stay in `%LOCALAPPDATA%\UFC3Native`.

- **Two or more controllers, two or more players.** Each gamepad is its own PS4 user (two
  identical pads, Parsec's virtual pads); the in-game menu (Insert / L3+R3) shows which is which.
- **Low input latency (co-op)**, on by default: one frame less between a button and the screen.
- **Ultra performance** for weak PCs: 720p, PS4 mode, FSR 1.0 to the screen, no sharpening.
- Fewer frame drops: memory copies next to GPU data no longer wait for the GPU; game threads use
  the PS4's floating-point mode.
- Saves on Windows survive another program (an antivirus) holding them open.
- Intel processors (SSE4a) and Intel Arc graphics fixes.
- Game folders without `sce_module`, or paired by `param.sfo`, are accepted.

---

## Português

O port do UFC 3 do GNSB, versão 0.1.6, com as correções feitas para o port do UFC 4, preparado
para o Cage Launcher. Saves, configurações e cache de shaders continuam em
`%LOCALAPPDATA%\UFC3Native`.

- **Dois ou mais controles, dois ou mais jogadores.** Cada controle é um usuário do PS4 (dois
  controles iguais, controles virtuais do Parsec); o menu do jogo (Insert / L3+R3) mostra qual é
  qual.
- **Baixa latência (co-op)**, ligada por padrão: um quadro a menos entre o botão e a tela.
- **Ultra desempenho** para PC fraco: 720p, modo PS4, FSR 1.0 na tela, sem nitidez.
- Menos quedas de FPS: cópias de memória ao lado de dados da GPU não esperam mais a GPU; as
  threads do jogo usam o modo de ponto flutuante do PS4.
- Saves no Windows resistem a outro programa (antivírus) com o arquivo aberto.
- Correções para processadores Intel (SSE4a) e placas Intel Arc.
- Pastas do jogo sem `sce_module`, ou juntadas pelo `param.sfo`, são aceitas.
