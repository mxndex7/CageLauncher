# Changelog

## 2.7 (2026-10-19)

- **Melhores configurações para cada PC, em todos os UFC.** Na primeira vez, e quando o hardware
  muda, o PORTHEON lê o seu PC e configura:
  - no UFC 1 a 4: resolução, limite de FPS, apresentação, modo PS4 Pro, FSR e ultra desempenho;
  - no UFC 5: o perfil (pelo `UFC5.exe --auto-config`).
  Os jogos que você já configurou à mão ficam como estão. "Configurações avançadas > Melhores
  configurações para este PC" refaz isso quando você quiser.
- **Modo Intel Arc**, automático:
  - aviso se o Resizable BAR estiver desligado;
  - VSync (Fifo) e modo PS4 Pro desligado no UFC 1–4;
  - limite de VRAM no UFC 5.
- **Linux e Steam Deck:** `portheon.sh` roda o PORTHEON pelo umu-launcher (Proton) ou pelo Wine 9+,
  e `--install` cria o atalho no menu.
- UFC 5 v1.8.1: detecção de Intel Arc e aviso de ReBAR.

## 2.6

- UFC 5:
  - limite de FPS 60/120/144;
  - geração de quadros com o Lossless Scaling;
  - tela cheia na resolução do monitor (antes, com 1280x720, a imagem ficava pequena no meio da tela);
  - diagnóstico completo de uma partida;
  - menu de Configurações avançadas.
