#ifndef DEFS_H
#define DEFS_H

// -- Kích thước màn hình --
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// -- Tốc độ di chuyển --
#define PLAYER_SPEED 4
#define PLAYER_BULLET_SPEED 16
#define ALIEN_BULLET_SPEED 8

// -- Giới hạn --
#define MAX_KEYBOARD_KEYS 350
#define MAX_STARS 500
#define MAX_SND_CHANNELS 8
#define MAX_LINE_LENGTH 1024
#define MAX_NAME_LENGTH 32
#define MAX_SCORE_NAME_LENGTH 16
#define NUM_HIGHSCORES 8

// -- Hằng số game --
#define SIDE_PLAYER 0
#define SIDE_ALIEN 1
#define FPS 60

// -- Đồ họa --
#define GLYPH_WIDTH 18
#define GLYPH_HEIGHT 28

// -- Enum cho kênh âm thanh --
enum {
    CH_ANY = -1,
    CH_PLAYER,
    CH_ALIEN_FIRE,
    CH_POINTS
};

// -- Enum cho các tệp âm thanh --
enum {
    SND_PLAYER_FIRE,
    SND_ALIEN_FIRE,
    SND_PLAYER_DIE,
    SND_ALIEN_DIE,
    SND_MAX,
    SND_POINTS
};

// -- Enum cho căn lề văn bản --
enum {
    TEXT_LEFT,
    TEXT_CENTER,
    TEXT_RIGHT
};

#endif // DEFS_H