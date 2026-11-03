# UFC 1 native port — Windows (experimental)

Port nativo do **EA SPORTS UFC** (2014, PlayStation 4, `CUSA00222`, update **1.08**), feito a partir
do port nativo do UFC 4 / UFC 3 (https://github.com/GNSB/UFC-3-PS4-PORT-LINUX-WINDOWS,
GPL-2.0-or-later). **Não contém arquivos do jogo**: você precisa do seu próprio dump.
O que mudou nesta versão: `RELEASE-NOTES.md`.

## Requisitos

- Windows 10 (1903+) ou 11, 64 bits; processador com AVX (Intel ou AMD); placa de vídeo com
  Vulkan 1.3 (NVIDIA, AMD ou Intel Arc).
- A pasta do jogo **extraída** com o `eboot.bin` descriptografado e o update 1.08
  (`eboot.bin`, `sce_sys`, `sce_module`, `databigs`…). Também serve o jogo base com a pasta do
  update ao lado (`CUSA00222` + `CUSA00222-patch`) ou a pasta que contém a pasta do jogo.

## Como jogar

1. Extraia esta pasta em qualquer lugar (fora da pasta do jogo e fora do OneDrive), ou use o
   launcher PORTHEON.
2. Abra **`UFC1.exe`**, clique em **Browse...**, escolha a pasta do UFC 1 e aperte **Play**.
3. Dados, saves e logs ficam em `%LOCALAPPDATA%\UFC1Native`.
4. Dois jogadores: conecte dois controles; cada controle é um jogador.
5. Se algo der errado: **Open logs folder** e mande o `.log` mais novo. Para quedas de FPS,
   marque **Frame statistics** antes.

## Estado

Primeiras versões de teste. Online não funciona.

Velocidade: como no PS4, as lutas rodam a 30 FPS e os menus a 60. O jogo conta o tempo da luta
por frame, então o port segue o ritmo do próprio jogo (antes, forçado a 60, a luta ficava
acelerada). Por isso o limite de FPS acima de 60 não muda nada no UFC 1.

## Volume

O som baixo foi corrigido: o jogo mistura em 7.1 e, em fone ou caixa estéreo, o port agora faz a
mistura para 2 canais sem abaixar vozes e narração. Se ainda quiser mais alto (ou mais baixo), use
**Volume** no launcher (50% a 300%; um limitador evita estourar o som).

## ReShade

O port usa Vulkan e funciona com o ReShade (https://reshade.me):

1. Rode o instalador do ReShade e escolha o executável **`bin\sacredfire-game.exe`** dentro da
   pasta do port (`UFC1.exe` é só o launcher; no PORTHEON: `games\UFC1\bin\sacredfire-game.exe`).
2. Em "rendering API" escolha **Vulkan**, e depois os efeitos que quiser.
3. Abra o jogo pelo launcher como sempre; **Home** abre o menu do ReShade.

Dicas: use efeitos de cor e nitidez (LumaSharpen, Vibrance, LUT…). Efeitos que dependem de
profundidade (MXAO, DOF) podem não achar o buffer certo. Se o jogo não abrir com o ReShade,
desinstale-o pelo mesmo instalador.

Este projeto não é afiliado nem endossado pela Electronic Arts, Sony Interactive Entertainment
ou UFC. GPL-2.0-or-later; licenças de terceiros em `licenses\`.
