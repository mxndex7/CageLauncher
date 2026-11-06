# Changelog

## 1.0

Primeira versão do Cage Launcher: um launcher para os ports para PC do EA SPORTS UFC, UFC 2, UFC 3,
UFC 4 e UFC 5.

- **Visual escuro e minimalista**, em preto, branco e vermelho: lista de jogos à esquerda, opções
  de JOGO, VÍDEO e JOGABILIDADE à direita. O logo é desenhado com a proporção do arquivo.
- **Melhores configurações para cada PC, em todos os UFC.** Na primeira vez, e quando o hardware
  muda, o Cage Launcher lê o seu PC e configura:
  - no UFC 1 a 4: resolução, limite de FPS, apresentação, modo PS4 Pro, FSR e ultra desempenho;
  - no UFC 5: o perfil (pelo `UFC5.exe --auto-config`).
  Os jogos que você já configurou à mão ficam como estão. "Configurações avançadas > Melhores
  configurações para este PC" refaz isso quando você quiser.
- **Modo Intel Arc**, automático:
  - aviso se o Resizable BAR estiver desligado;
  - VSync (Fifo) e modo PS4 Pro desligado no UFC 1–4;
  - limite de VRAM no UFC 5.
- **UFC 5:**
  - limite de FPS 60/120/144;
  - geração de quadros com o Lossless Scaling;
  - tela cheia na resolução do monitor;
  - diagnóstico completo de uma partida;
  - menu de Configurações avançadas;
  - detecção de Intel Arc e aviso de ReBAR.
