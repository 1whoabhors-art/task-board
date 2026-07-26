

A lightweight retro-style task manager written in C++ using SFML.



---

## Features

   Create tasks
   Double-click to edit
   Red = incomplete
   Blue = completed
   Automatically saves tasks
   Completed tasks are automatically removed after one week
   Retro Windows inspired interface

---

## Controls

| Action | Input |
|---------|-------|
| Add task | Click the **+** button |
| Complete task | Left click |
| Edit task | Double click |
| Save | Automatic |


## Building

Requirements

- C++17
- SFML 2.5+

Clone the repository

```bash
git clone https://github.com/1whoabhors/task-board.git
```

Compile

```bash
g++ src/main.cpp -o RetroTaskBoard \
-lsfml-graphics \
-lsfml-window \
-lsfml-system


- C++
- SFML
- File I/O
- Event handling
- GUI programming


## License

MIT
