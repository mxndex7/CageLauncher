@echo off
rem UFC 1: o motor roda no modo estavel (gravacao dos comandos e preparo dos desenhos na thread de comandos da GPU).
rem Com as threads de gravacao a luta congelava: um envio para a GPU ficava preso enquanto a thread de comandos esperava por ele.
rem O shader da luta 0xb61e59a3b11b03ad agora e corrigido (user\shader\patch) e nao e mais pulado.
set BB_VK_RECORD_THREAD=0
set BB_DRAW_PIPE=0
start "" "%~dp0UFC1.exe" --play
