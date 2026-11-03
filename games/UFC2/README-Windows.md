# UFC 2 native port — Windows (experimental)

Port nativo do **EA SPORTS UFC 2** (PlayStation 4, `CUSA01968`, update **1.13**), feito a partir
do port nativo do UFC 4 / UFC 3 (https://github.com/GNSB/UFC-3-PS4-PORT-LINUX-WINDOWS,
GPL-2.0-or-later). **Não contém arquivos do jogo**: você precisa do seu próprio dump.
O que mudou nesta versão: `RELEASE-NOTES.md`.

## Requisitos

- Windows 10 (1903+) ou 11, 64 bits; processador com AVX (Intel ou AMD); placa de vídeo com
  Vulkan 1.3 (NVIDIA, AMD ou Intel Arc).
- A pasta do jogo **extraída** com o `eboot.bin` descriptografado e o update 1.13
  (`eboot.bin`, `sce_sys`, `sce_module`, `databigs`…). Também serve o jogo base com a pasta do
  update ao lado (`CUSA01968` + `CUSA01968-patch`) ou a pasta que contém a pasta do jogo.

## Como jogar

1. Extraia esta pasta em qualquer lugar (fora da pasta do jogo e fora do OneDrive), ou use o
   launcher PORTHEON.
2. Abra **`UFC2.exe`**, clique em **Browse...**, escolha a pasta do UFC 2 e aperte **Play**.
3. Dados, saves e logs ficam em `%LOCALAPPDATA%\UFC2Native`.
4. Dois jogadores: conecte dois controles; cada controle é um jogador.
5. Se algo der errado: **Open logs folder** e mande o `.log` mais novo. Para quedas de FPS,
   marque **Frame statistics** antes.

## Estado

Primeiras versões de teste: o UFC 2 está sendo trazido para o port agora. Online não funciona.

## Volume

O som baixo foi corrigido: o jogo mistura em 7.1 e, em fone ou caixa estéreo, o port agora faz a
mistura para 2 canais sem abaixar vozes e narração. Se ainda quiser mais alto (ou mais baixo), use
**Volume** no launcher (50% a 300%; um limitador evita estourar o som).

## ReShade

O port usa Vulkan e funciona com o ReShade (https://reshade.me):

1. Rode o instalador do ReShade e escolha o executável **`bin\sacredfire-game.exe`** dentro da
   pasta do port (`UFC2.exe` é só o launcher; no PORTHEON: `games\UFC2\bin\sacredfire-game.exe`).
2. Em "rendering API" escolha **Vulkan**, e depois os efeitos que quiser.
3. Abra o jogo pelo launcher como sempre; **Home** abre o menu do ReShade.

Dicas: use efeitos de cor e nitidez (LumaSharpen, Vibrance, LUT…). Efeitos que dependem de
profundidade (MXAO, DOF) podem não achar o buffer certo. Se o jogo não abrir com o ReShade,
desinstale-o pelo mesmo instalador.

## Patches da comunidade (opcionais)

`patches\UFC2-community.xml` traz patches da comunidade (Lepexa, Antigravity): 60+ FPS, correção
do chão do octógono, resoluções, sem torcida (+5-10 FPS), sem sombras, sem motion blur, método de
skinning. **Todos vêm desligados.** Para ligar um: abra o arquivo no Bloco de Notas, ache o patch
pelo `Name` com a versão do seu jogo (`AppVer`) e troque `isEnabled="false"` por `isEnabled="true"`.
Na próxima vez que o jogo abrir, o log mostra `Patches: external ...`. Alguns têm efeitos
colaterais (veja o `Note`, por exemplo lutadores caindo pelo octógono): desligue se der problema.

Este projeto não é afiliado nem endossado pela Electronic Arts, Sony Interactive Entertainment
ou UFC. GPL-2.0-or-later; licenças de terceiros em `licenses\`.
