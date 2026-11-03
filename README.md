# Cage Launcher 1.0

**Cage Launcher** é um launcher para jogar no PC os ports de **EA SPORTS UFC 1, 2, 3, 4 (PS4) e UFC 5
(PS5)**. Ele acha o jogo, lê o seu PC e escolhe as melhores configurações para cada UFC sozinho.

> **Nenhum arquivo de jogo está incluído.** Você precisa do seu próprio dump de cada jogo.
> Este projeto não é afiliado nem endossado pela Electronic Arts, pela Sony Interactive
> Entertainment ou pelo UFC.

[English below](#english)

---

## Downloads

| Arquivo | Para quem |
|---|---|
| `CageLauncher-1.0-Windows-x64.zip` | Windows 10 (1903+) e 11, 64 bits. Inclui o modo Intel Arc. |
| `CageLauncher-1.0-Linux-x64.tar.gz` | Linux e Steam Deck, pelo Wine ou pelo Proton. É o mesmo pacote com o `cage.sh`. |

**Placas Intel Arc:** não precisa de download separado. O Cage Launcher reconhece a placa Arc e liga o
modo Intel Arc sozinho (veja [Intel Arc](#intel-arc)).

## Jogos

| Jogo | Plataforma original | Dump necessário | Motor do port |
|---|---|---|---|
| EA SPORTS UFC (2014) | PS4 | `CUSA00222`, update 1.08 | port nativo (SacredFire / bbport + shadPS4) |
| EA SPORTS UFC 2 | PS4 | `CUSA01968`, update 1.13 | port nativo |
| EA SPORTS UFC 3 | PS4 | `CUSA06534`, update 1.14 | port nativo (base: GNSB 0.1.6) |
| EA SPORTS UFC 4 | PS4 | `CUSA14209` ou `CUSA14204`, update 28.00 | port nativo |
| EA SPORTS UFC 5 | PS5 | `PPSA03541` (testado com a v01.031) | KytyPS5 (emulador de PS5, build para o UFC 5) |

Os jogos de PS4 precisam do `eboot.bin` descriptografado e da pasta do update ao lado do jogo base,
ou já juntos. O launcher de cada port explica o que estiver faltando.

## O que o Cage Launcher faz sozinho

Na primeira vez que abre, e de novo quando o PC muda (outra placa de vídeo, mais RAM, outro
monitor), o Cage Launcher lê o seu PC:
- a placa de vídeo pelo Vulkan (fabricante, memória e Resizable BAR);
- os threads do processador;
- a RAM;
- a resolução e a frequência do monitor.

Com isso ele escolhe as configurações de cada jogo:

| | PC forte (12+ threads, 7,5+ GB de VRAM, 16+ GB de RAM) | PC fraco | PC muito fraco (até 4 threads, menos de 4 GB de VRAM ou menos de 12 GB de RAM) |
|---|---|---|---|
| UFC 1, 2, 4 | 1080p, 60 FPS, Mailbox | 1080p, 60 FPS | 720p com FSR 1.0 e nitidez, 60 FPS; "ultra desempenho" abaixo de 4 threads ou 3 GB |
| UFC 3 | 1080p, até 120/144 FPS com monitor de 120/144 Hz | 1080p, 60 FPS | igual aos outros |
| Modo PS4 Pro (UFC 2, 3, 4) | ligado em NVIDIA/AMD com 7,5 GB ou mais | desligado | desligado |
| UFC 5 | perfil Máximo FPS, tela cheia na resolução do monitor | perfil PC fraco (limite de VRAM, 1080p) | 720p, menos threads de shader |

Por que esses números:
- UFC 1, 2 e 4 contam o tempo do jogo por quadro, então ficam em 60 FPS. No UFC 1, as lutas ficam
  nos 30 FPS do próprio jogo.
- O UFC 3 roda a luta em qualquer FPS.

Se você já tinha configurado um jogo à mão, ele fica como está. Para refazer, use
**Configurações avançadas > Melhores configurações para este PC**.

### Intel Arc

O Cage Launcher detecta Intel Arc (A e B) e aplica:
- **Aviso de Resizable BAR:** se o ReBAR estiver desligado, aparece um aviso para ligar
  "Re-Size BAR" e "Above 4G Decoding" na BIOS. O Arc perde muito desempenho sem ele.
- **UFC 1 a 4:** apresentação VSync (Fifo), o modo que o driver da Intel sempre suporta, e modo PS4
  Pro desligado.
- **UFC 5:** limite de memória de vídeo em 85% da placa, porque o driver lida pior com a VRAM cheia.

Também ajuda usar o driver Intel mais recente.

### UFC 5 (PS5)

- **Limite de FPS 60, 120 ou 144** (experimental). Os menus passam de 60. Se o relógio do round
  correr rápido, volte para 60.
- **Geração de quadros com o Lossless Scaling** (Steam, pago e separado): 120/144 FPS na tela. O
  Cage Launcher abre o Lossless Scaling e aperta Ctrl+Alt+S sozinho.
- **Calibração automática:** 12 lutas, 6 ajustes do motor, e fica com o que der mais FPS.
- **Diagnóstico completo** de uma partida, e FPS na tela (F10).

Seja realista: o UFC 5 roda num emulador de PS5 e o processador é o limite. Num Ryzen 5 5500 +
RTX 4060, a luta fica perto de 30 FPS. Detalhes em `games/UFC5/LEIA-ME.txt`.

## Como usar — Windows

1. Extraia o `.zip` numa pasta fora do OneDrive, por exemplo `C:\Jogos\CageLauncher`.
2. Abra o `CageLauncher.exe`.
3. Escolha o jogo na esquerda e, em **Procurar**, a pasta do dump. Aperte **JOGAR**.

Requisitos:
- processador com AVX;
- placa com Vulkan 1.3 (NVIDIA, AMD ou Intel Arc), com o driver atualizado;
- para o UFC 5, o Microsoft Visual C++ Redistributable x64 (o port oferece o link) e 16 GB de RAM com
  arquivo de paginação de 16 a 32 GB.

Saves, configurações e logs:
- UFC 1–4: `%LOCALAPPDATA%\UFC<n>Native`;
- UFC 5: `games\UFC5\bin`;
- Cage Launcher: `%LOCALAPPDATA%\CageLauncher\cage.ini`.

## Como usar — Linux e Steam Deck

```bash
tar xf CageLauncher-1.0-Linux-x64.tar.gz
cd CageLauncher-1.0
bash cage.sh             # abre o Cage Launcher
bash cage.sh --install   # coloca o Cage Launcher no menu de aplicativos
```

- O `cage.sh` usa o **umu-launcher (Proton)** se ele estiver instalado. Se não, usa o **Wine 9
  ou mais novo**. O prefixo fica em `~/.local/share/CageLauncher/prefix`.
- As pastas do Linux aparecem no Cage Launcher como o disco **Z:**, por exemplo
  `Z:\home\voce\Jogos\UFC 4`.
- **Steam Deck:** em Modo Desktop, adicione o `CageLauncher.exe` como jogo de fora da Steam. Em
  Propriedades > Compatibilidade, escolha GE-Proton ou Proton Experimental. Depois jogue no Modo
  de Jogo.
- **UFC 3 nativo para Linux:** o projeto original do GNSB tem um build Linux de verdade (sem Wine):
  https://github.com/GNSB/UFC-3-PS4-PORT-LINUX-WINDOWS
- **Estado:** o launcher funciona no Wine. Os ports do UFC 1–4 usam só Vulkan e devem funcionar no
  Proton, mas isso ainda não foi testado com os jogos.
- **UFC 5 no Linux é experimental:** o emulador usa APIs de memória do Windows que o Wine 9 não tem
  por completo, e no Wine 9 ele fecha logo ao abrir. Teste com o Proton mais novo (GE-Proton) e
  mande o log.
- **Geração de quadros no Linux:** o Lossless Scaling é só para Windows. No Linux existe o lsfg-vk,
  que usa a sua cópia do Lossless Scaling.

## Problemas comuns

- **"Vulkan não está instalado":** atualize o driver da placa.
- **Gráficos quebrados depois de trocar o driver:** limpe o cache de shaders na janela do port
  (Configurações avançadas > Todas as opções).
- **FPS caindo para 1–3 no UFC 5:** falta memória virtual. Use 16 GB de RAM e um arquivo de
  paginação de 16 a 32 GB.
- **Intel Arc lento:** ligue o Resizable BAR na BIOS.
- **Logs:** use o botão **Logs** no Cage Launcher e mande o mais novo junto com o relatório.

## Código-fonte e licenças

- `CageLauncher.exe` (`src/`) e o launcher do UFC 5 (`games/UFC5/src/`) estão em C/Win32, sob
  GPL-2.0-or-later. Para compilar, rode o `build.sh` com o llvm-mingw.
- Os ports do UFC 1–4 são GPL-2.0-or-later. Eles se baseiam em:
  - [SacredFire](https://github.com/T3CHNOLOG1C/SacredFire);
  - [bbport](https://github.com/deadinside28/bloodborne_pc);
  - [shadPS4](https://github.com/shadps4-emu/shadPS4);
  - o [port do UFC 3 do GNSB](https://github.com/GNSB/UFC-3-PS4-PORT-LINUX-WINDOWS).
  As bibliotecas de cada port estão em `games/UFC<n>/licenses`.
- UFC 5: [KytyPS5](https://github.com/KytyPS5/KytyPS5), GPL-2.0, no build do
  [roshanarunk/KytyPS5-UFC5-builds](https://github.com/roshanarunk/KytyPS5-UFC5-builds). A oferta
  de código-fonte do autor está em `games/UFC5/bin/README-motor-KytyPS5.txt`.
- Patches da comunidade (todos vêm desligados): Lepexa e Antigravity. Correções de AMD e modo Pro:
  @luscamovi. Cabelo e pele: mancelli.
- Lossless Scaling é um produto da THS na Steam. O Cage Launcher só abre o programa e aperta o atalho
  dele; ele não vem incluído.

EA SPORTS UFC, PlayStation e as marcas relacionadas pertencem aos seus donos. Elas aparecem aqui
só para identificar os jogos.

---

## English

**Cage Launcher** is a launcher for the PC ports of **EA SPORTS UFC 1, 2, 3, 4 (PS4) and UFC 5 (PS5)**.
No game files are included: you need your own dump of each game.

- **Best settings for every PC:** on the first start, and whenever the hardware changes, Cage Launcher
  reads the GPU (through Vulkan), CPU threads, RAM and monitor. It then picks resolution, FPS cap,
  present mode, PS4 Pro mode and FSR for UFC 1–4, and the UFC 5 profile. Games you already set up by
  hand are left alone. To redo it: *Advanced settings > Best settings for this PC*.
- **Intel Arc:** detected automatically.
  - Without Resizable BAR you get a warning to turn on "Re-Size BAR" and "Above 4G Decoding" in the
    BIOS.
  - UFC 1–4 use FIFO presentation and no PS4 Pro mode.
  - UFC 5 gets a VRAM budget of 85% of the card.
- **UFC 5:**
  - 60/120/144 FPS cap (experimental);
  - frame generation through Lossless Scaling, sold separately on Steam;
  - fullscreen at the monitor's resolution;
  - automatic calibration and a one-session full diagnostic.
  The fight is CPU-bound in the emulator: about 30 FPS on a Ryzen 5 5500.
- **Windows:** extract the zip, run `CageLauncher.exe`, pick a game, Browse to your dump, PLAY.
- **Linux / Steam Deck:** run `bash cage.sh`. It uses umu-launcher (Proton) if installed,
  otherwise Wine 9 or newer.
  - Linux folders show up as drive Z:.
  - UFC 1–4 should work under Proton, but this is untested with the games.
  - UFC 5 under Wine is experimental: the emulator needs Windows memory APIs that Wine 9 lacks.
  - For a native UFC 3 build, see GNSB's repository.

Licenses: GPL-2.0-or-later (Cage Launcher, the UFC 1–4 ports), GPL-2.0 (KytyPS5). Third-party licenses
are in each `licenses` folder.
