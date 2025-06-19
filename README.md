# SDL2 2D Shooter Game

A simple 2D side-scrolling space shooter built with **C++** and the **SDL2** library. This project is configured to compile and run on Linux systems.

---

## Controls

* **Arrow Keys**: Move the player's ship.
* **Left Ctrl**: Fire weapon.
* **Return (Enter)**: Confirm name entry on the high score screen.

---

## Building on Linux

### 1. Prerequisites

First, clone the repository to your local machine:

```bash
git clone https://github.com/nhu220840/shooting-game.git
cd shooting-game
```

### 2. Dependencies

You will need `build-essential` and the development libraries for SDL2.
On Debian-based systems (like Ubuntu), install them with:

```bash
sudo apt-get install build-essential libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

### 3. Build Instructions

Compile the game by running the `make` command in the project's root directory:

```bash
make
```

Run the game after compiling:

```bash
./shooter_game
```

**(Optional)**: Create a distributable package by running the `package.sh` script.
This will create a `.tar.gz` archive containing the game, assets, and all required libraries:

```bash
bash package.sh
```

---

## License

This project is licensed under the **MIT License**.
See the `LICENSE` file for more details.

> **Note:** Audio assets are subject to their own licenses. Please see the `README.txt` files in the `assets/` subdirectories for detailed credits.

---

## Contact

* **Do Nguyen Gia Nhu** – [gianhuw.work@gmail.com](mailto:gianhuw.work@gmail.com)
* **Project Link:** [https://github.com/nhu220840/shooting-game.git](https://github.com/nhu220840/shooting-game.git)

---
