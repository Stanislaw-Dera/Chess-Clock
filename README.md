# Chess Clock (Arduino Nano)

A chess clock I designed and built on my own as a learning project.

**Videos of the clock in action:** [Google Drive](https://drive.google.com/drive/folders/1sQYMvmA-D-meGYdj8IVYb6a-Tlf4nF57?usp=sharing)

## Features

- Two six-digit displays (HH.MM.SS), handling games lasting several hours.
- Four potentiometers used for setting hours (0–7), minutes, seconds and increment (0–60 s).
- Increment added to a player's time after each move.
- Two modes, chosen with a toggle switch: count down (classic chess clock) or count up (stopwatch).
- Illuminated arcade button for each player; the backlight shows whose move it is.
- The game state control button: ready, pause, reset.
- When time runs out, the display blinks and shows a message.

Libraries: [TM1637TinyDisplay](https://github.com/jasonacox/TM1637TinyDisplay), [ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead).
