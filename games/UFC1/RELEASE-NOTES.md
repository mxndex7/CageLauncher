# UFC 1 native port 0.1.0 (test)

First package of EA SPORTS UFC (2014, PS4 CUSA00222, update 1.08), also in PORTHEON 1.7.

- Fights run at the right speed. UFC 1 counts fight time per frame at its own 30 FPS; the port
  forced 60 FPS with a fast virtual vblank, so a fight ran about twice as fast. The port now follows
  the game's own frame rate on a 60 Hz vblank, as on the console (menus 60 FPS, fights 30).
- Same port base as UFC 2/3/4 1.6: volume control and 7.1-to-stereo mix, crash reports, GPU fixes.
- Online does not work.
