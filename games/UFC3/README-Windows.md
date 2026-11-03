# UFC 3 native port — Windows

Port nativo do **EA SPORTS UFC 3** (PlayStation 4, `CUSA06534`, update **1.14**), baseado no port
do GNSB (https://github.com/GNSB/UFC-3-PS4-PORT-LINUX-WINDOWS, GPL-2.0-or-later, versão 0.1.6)
com as correções feitas para o UFC 4. **Não contém arquivos do jogo**: você precisa do seu
próprio dump. O que mudou nesta versão: `RELEASE-NOTES.md`.

## Requisitos

- Windows 10 (1903+) ou 11, 64 bits; processador com AVX (Intel ou AMD); placa de vídeo com
  Vulkan 1.3 (NVIDIA, AMD ou Intel Arc).
- A pasta do jogo **extraída** com o `eboot.bin` descriptografado e o update 1.14 (`eboot.bin`,
  `sce_sys`, `sce_module`, `databigs`…), ou o jogo base com `CUSA06534-patch` ao lado.

## Como jogar

1. Extraia esta pasta em qualquer lugar (fora da pasta do jogo e fora do OneDrive), ou use o
   Cage Launcher.
2. Abra **`UFC3.exe`**, clique em **Browse...**, escolha a pasta do UFC 3 e aperte **Play**.
3. Dados, saves e logs ficam em `%LOCALAPPDATA%\UFC3Native` (os mesmos das versões anteriores).
4. Dois jogadores: conecte dois controles (também dois controles iguais e pelo Parsec); cada
   controle é um jogador. O menu do jogo (**Insert** ou **L3+R3**) mostra qual é qual.
5. PC fraco: marque **Ultra performance**. Menos atraso nos controles: **Low input latency**
   (vem ligado).
6. Se algo der errado: **Open logs folder** e mande o `.log` mais novo.

## Volume

O som baixo foi corrigido: o jogo mistura em 7.1 e, em fone ou caixa estéreo, o port agora faz a
mistura para 2 canais sem abaixar vozes e narração. Se ainda quiser mais alto (ou mais baixo), use
**Volume** no launcher (50% a 300%; um limitador evita estourar o som).

## ReShade

O port usa Vulkan e funciona com o ReShade (https://reshade.me):

1. Rode o instalador do ReShade e escolha o executável **`bin\sacredfire-game.exe`** dentro da
   pasta do port (`UFC3.exe` é só o launcher; no Cage Launcher: `games\UFC3\bin\sacredfire-game.exe`).
2. Em "rendering API" escolha **Vulkan**, e depois os efeitos que quiser.
3. Abra o jogo pelo launcher como sempre; **Home** abre o menu do ReShade.

Dicas: use efeitos de cor e nitidez (LumaSharpen, Vibrance, LUT…). Efeitos que dependem de
profundidade (MXAO, DOF) podem não achar o buffer certo. Se o jogo não abrir com o ReShade,
desinstale-o pelo mesmo instalador.

## Patches da comunidade (opcionais)

`patches\UFC3-community.xml` traz patches da comunidade (Lepexa, Antigravity): 60+ FPS, correção
do chão do octógono, resoluções, sem torcida (+5-10 FPS), sem sombras, sem motion blur, método de
skinning. **Todos vêm desligados.** Para ligar um: abra o arquivo no Bloco de Notas, ache o patch
pelo `Name` com a versão do seu jogo (`AppVer`) e troque `isEnabled="false"` por `isEnabled="true"`.
Na próxima vez que o jogo abrir, o log mostra `Patches: external ...`. Alguns têm efeitos
colaterais (veja o `Note`, por exemplo lutadores caindo pelo octógono): desligue se der problema.

Este projeto não é afiliado nem endossado pela Electronic Arts, Sony Interactive Entertainment
ou UFC. GPL-2.0-or-later; licenças de terceiros em `licenses\`.
