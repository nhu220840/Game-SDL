#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "logic.h"
#include "globals.h"
#include "stages.h"
#include "init.h"
#include "utils.h"

// --- KHAI BÁO CÁC HÀM TĨNH (STATIC) CHỈ DÙNG TRONG FILE NÀY ---
static void doPlayer(void);
static void doFighters(void);
static void doEnemies(void);
static void doBullets(void);
static void doStarfield(void);
static void doBackground(void);
static void spawnEnemies(void);
static void clipPlayer(void);
static void doExplosions(void);
static void doDebris(void);
static void addDebris(Entity* e);
static void addExplosions(int x, int y, int num);
static int bulletHitFighter(Entity* b);
static void fireAlienBullet(Entity* e);
static void fireBullet(void);
static void doPointsPods(void);
static void addPointsPod(int x, int y);
static int highscoreComparator(const void* a, const void* b);

// --- ĐỊNH NGHĨA CÁC HÀM ---

void logic(void) {
    doBackground();
    doStarfield();
    doPlayer();
    doEnemies();
    doFighters();
    doBullets();
    doExplosions();
    doDebris();
    doPointsPods();
    spawnEnemies();
    clipPlayer();

    // Nếu người chơi chết, chờ một lúc rồi chuyển sang màn hình điểm cao
    if (player == NULL && --stageResetTimer <= 0) {
        addHighscore(stage.score);
        initHighscores();
    }
}

static void doBackground(void) {
    if (--backgroundX < -SCREEN_WIDTH) {
        backgroundX = 0;
    }
}

static void doStarfield(void) {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x -= stars[i].speed;
        if (stars[i].x < 0) {
            stars[i].x = SCREEN_WIDTH + stars[i].x;
        }
    }
}

static void doPlayer(void) {
    if (player != NULL) {
        player->dx = 0;
        player->dy = 0;

        if (player->reload > 0) {
            player->reload--;
        }

        if (app.keyboard[SDL_SCANCODE_UP]) player->dy = -PLAYER_SPEED;
        if (app.keyboard[SDL_SCANCODE_DOWN]) player->dy = PLAYER_SPEED;
        if (app.keyboard[SDL_SCANCODE_LEFT]) player->dx = -PLAYER_SPEED;
        if (app.keyboard[SDL_SCANCODE_RIGHT]) player->dx = PLAYER_SPEED;
        
        if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload <= 0) {
            fireBullet();
        }

        // Áp dụng di chuyển sau khi đã tính toán
        player->x += player->dx;
        player->y += player->dy;
    }
}

static void clipPlayer(void) {
    if (player != NULL) {
        if (player->x < 0) player->x = 0;
        if (player->y < 0) player->y = 0;
        if (player->x > SCREEN_WIDTH - player->w) player->x = SCREEN_WIDTH - player->w;
        if (player->y > SCREEN_HEIGHT - player->h) player->y = SCREEN_HEIGHT - player->h;
    }
}

static void fireBullet(void) {
    Entity* bullet = (Entity*)malloc(sizeof(Entity));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = player->x;
    bullet->y = player->y;
    bullet->dx = PLAYER_BULLET_SPEED;
    bullet->health = 1;
    bullet->texture = bulletTexture;
    bullet->side = SIDE_PLAYER;
    SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);

    bullet->x += (player->w / 2) - (bullet->w / 2);
    bullet->y += (player->h / 2) - (bullet->h / 2);
    player->reload = 8;
    playSound(SND_PLAYER_FIRE, CH_PLAYER);
}

static void doBullets(void) {
    Entity *b, *prev = &stage.bulletHead;
    for (b = stage.bulletHead.next; b != NULL; b = b->next) {
        b->x += b->dx;
        b->y += b->dy;

        if (bulletHitFighter(b) || b->x < -b->w || b->x > SCREEN_WIDTH || b->y < -b->h || b->y > SCREEN_HEIGHT) {
            if (b == stage.bulletTail) stage.bulletTail = prev;
            prev->next = b->next;
            free(b);
            b = prev;
        }
        prev = b;
    }
}

static int bulletHitFighter(Entity* b) {
    for (Entity* e = stage.fighterHead.next; e != NULL; e = e->next) {
        if (e->health > 0 && e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h)) {
            b->health = 0;
            e->health = 0;

            if (e == player) {
                playSound(SND_PLAYER_DIE, CH_PLAYER);
            } else {
                addPointsPod(e->x + e->w / 2, e->y + e->h / 2);
                playSound(SND_ALIEN_DIE, CH_ANY);
            }
            return 1;
        }
    }
    return 0;
}

static void doFighters(void) {
    Entity *e, *other;

    // --- LOGIC MỚI: VÒNG LẶP KIỂM TRA VA CHẠM GIỮA CÁC TÀU ---
    // Vòng lặp này chỉ để phát hiện va chạm và đánh dấu health = 0
    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        // Bỏ qua nếu tàu đã bị phá hủy trong các lần kiểm tra trước
        if (e->health == 0) continue;

        for (other = e->next; other != NULL; other = other->next) {
            // Bỏ qua nếu tàu kia đã bị phá hủy
            if (other->health == 0) continue;

            // Chỉ kiểm tra va chạm giữa 2 phe khác nhau
            if (e->side != other->side) {
                if (collision(e->x, e->y, e->w, e->h, other->x, other->y, other->w, other->h)) {
                    // Khi va chạm, đánh dấu cả hai bị phá hủy
                    e->health = 0;
                    other->health = 0;
                }
            }
        }
    }

    // --- VÒNG LẶP CẬP NHẬT VỊ TRÍ VÀ DỌN DẸP ---
    // Vòng lặp này cập nhật vị trí và xóa các tàu có health = 0
    Entity* prev = &stage.fighterHead;
    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        // Cập nhật vị trí của các thực thể (player đã được cập nhật trong doPlayer)
        if (e != player) {
            e->x += e->dx;
            e->y += e->dy;
        }
        
        // Kẻ thù bay ra khỏi màn hình bên trái cũng bị xóa
        if (e != player && e->x < -e->w) {
            e->health = 0;
        }
        
        // Xử lý các fighter có health = 0 (bị trúng đạn hoặc va chạm)
        if (e->health == 0) {
            if (e == player) {
                player = NULL;
                playSound(SND_PLAYER_DIE, CH_PLAYER); // Chơi âm thanh nổ của player
            } else {
                // Âm thanh nổ của enemy đã được xử lý khi trúng đạn,
                // nhưng va chạm trực tiếp cũng sẽ kích hoạt hiệu ứng nổ ở đây.
                playSound(SND_ALIEN_DIE, CH_ANY);
            }

            addExplosions(e->x, e->y, 32);
            addDebris(e);

            if (e == stage.fighterTail) {
                stage.fighterTail = prev;
            }
            prev->next = e->next;
            free(e);
            e = prev;
        }
        prev = e;
    }
}

static void doEnemies(void) {
    for (Entity* e = stage.fighterHead.next; e != NULL; e = e->next) {
        if (e != player) {
            if (e->y < 0 || e->y > SCREEN_HEIGHT - e->h) e->dy = -e->dy;
            
            if (player != NULL && --(e->reload) <= 0) {
                fireAlienBullet(e);
            }
        }
    }
}

static void fireAlienBullet(Entity* e) {
    Entity* bullet = (Entity*)malloc(sizeof(Entity));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = e->x;
    bullet->y = e->y;
    bullet->health = 1;
    bullet->texture = alienBulletTexture;
    bullet->side = SIDE_ALIEN;
    SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);

    bullet->x += (e->w / 2) - (bullet->w / 2);
    bullet->y += (e->h / 2) - (bullet->h / 2);

    calcSlope(player->x + (player->w / 2), player->y + (player->h / 2), e->x, e->y, &bullet->dx, &bullet->dy);
    bullet->dx *= ALIEN_BULLET_SPEED;
    bullet->dy *= ALIEN_BULLET_SPEED;
    
    e->reload = (rand() % (FPS * 2)) + (FPS / 2); // Thêm thời gian chờ tối thiểu
    playSound(SND_ALIEN_FIRE, CH_ALIEN_FIRE);
}

static void spawnEnemies(void) {
    if (--enemySpawnTimer <= 0) {
        Entity* enemy = (Entity*)malloc(sizeof(Entity));
        memset(enemy, 0, sizeof(Entity));
        stage.fighterTail->next = enemy;
        stage.fighterTail = enemy;

        enemy->x = SCREEN_WIDTH;
        enemy->y = rand() % (SCREEN_HEIGHT - 100);
        enemy->texture = enemyTexture;
        SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);
        enemy->dx = -((float)(2 + (rand() % 4)));
        enemy->dy = (float)((rand() % 101) - 50) / 100.0f;
        enemy->side = SIDE_ALIEN;
        enemy->health = 1;
        enemy->reload = FPS * (1 + (rand() % 3));
        enemySpawnTimer = 30 + (rand() % 60);
    }
}

static void addExplosions(int x, int y, int num) {
    for (int i = 0; i < num; i++) {
        Explosion* e = (Explosion*)malloc(sizeof(Explosion));
        memset(e, 0, sizeof(Explosion));
        stage.explosionTail->next = e;
        stage.explosionTail = e;

        e->x = x + (rand() % 32) - 16;
        e->y = y + (rand() % 32) - 16;
        e->dx = (float)((rand() % 10) - 5) / 10.0f;
        e->dy = (float)((rand() % 10) - 5) / 10.0f;
        
        switch (rand() % 4) {
            case 0: e->r = 255; break;
            case 1: e->r = 255; e->g = 128; break;
            case 2: e->r = 255; e->g = 255; break;
            default: e->r = 255; e->g = 255; e->b = 255; break;
        }
        e->a = rand() % FPS * 3;
    }
}

static void doExplosions(void) {
    Explosion* e, * prev = &stage.explosionHead;
    for (e = stage.explosionHead.next; e != NULL; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;
        if (--e->a <= 0) {
            if (e == stage.explosionTail) stage.explosionTail = prev;
            prev->next = e->next;
            free(e);
            e = prev;
        }
        prev = e;
    }
}

static void addDebris(Entity* e) {
    int x, y, w = e->w / 2, h = e->h / 2;
    for (y = 0; y <= h; y += h) {
        for (x = 0; x <= w; x += w) {
            Debris* d = (Debris*)malloc(sizeof(Debris));
            memset(d, 0, sizeof(Debris));
            stage.debrisTail->next = d;
            stage.debrisTail = d;
            d->x = e->x + e->w / 2;
            d->y = e->y + e->h / 2;
            d->dx = (float)((rand() % 5) - (rand() % 5));
            d->dy = -((float)(5 + (rand() % 12)));
            d->life = FPS * 2;
            d->texture = e->texture;
            d->rect.x = x;
            d->rect.y = y;
            d->rect.w = w;
            d->rect.h = h;
        }
    }
}

static void doDebris(void) {
    Debris* d, * prev = &stage.debrisHead;
    for (d = stage.debrisHead.next; d != NULL; d = d->next) {
        d->x += d->dx;
        d->y += d->dy;
        d->dy += 0.5;
        if (--d->life <= 0) {
            if (d == stage.debrisTail) stage.debrisTail = prev;
            prev->next = d->next;
            free(d);
            d = prev;
        }
        prev = d;
    }
}

static void addPointsPod(int x, int y) {
    Entity* e = (Entity*)malloc(sizeof(Entity));
    memset(e, 0, sizeof(Entity));
    stage.pointsTail->next = e;
    stage.pointsTail = e;
    e->x = x;
    e->y = y;
    e->dx = -((float)(rand() % 5));
    e->dy = (float)((rand() % 5) - (rand() % 5));
    e->health = FPS * 10;
    e->texture = pointsTexture;
    SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
    e->x -= e->w / 2;
    e->y -= e->h / 2;
}

static void doPointsPods(void) {
    Entity* e, * prev = &stage.pointsHead;
    for (e = stage.pointsHead.next; e != NULL; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;
        if (e->x < 0 || e->x + e->w > SCREEN_WIDTH) e->dx = -e->dx;
        if (e->y < 0 || e->y + e->h > SCREEN_HEIGHT) e->dy = -e->dy;
        if (player != NULL && collision(e->x, e->y, e->w, e->h, player->x, player->y, player->w, player->h)) {
            e->health = 0;
            stage.score++;
            highscore = fmax(stage.score, highscore);
            playSound(SND_POINTS, CH_POINTS);
        }
        if (--e->health <= 0) {
            if (e == stage.pointsTail) stage.pointsTail = prev;
            prev->next = e->next;
            free(e);
            e = prev;
        }
        prev = e;
    }
}

static int highscoreComparator(const void* a, const void* b) {
    return ((Highscore*)b)->score - ((Highscore*)a)->score;
}

void addHighscore(int score) {
    Highscore newHighscores[NUM_HIGHSCORES + 1];
    for (int i = 0; i < NUM_HIGHSCORES; i++) {
        newHighscores[i] = highscores.highscore[i];
        newHighscores[i].recent = 0;
    }
    newHighscores[NUM_HIGHSCORES].score = score;
    newHighscores[NUM_HIGHSCORES].recent = 1;

    qsort(newHighscores, NUM_HIGHSCORES + 1, sizeof(Highscore), highscoreComparator);

    newHighscoreFlag = false;
    for (int i = 0; i < NUM_HIGHSCORES; i++) {
        highscores.highscore[i] = newHighscores[i];
        if (highscores.highscore[i].recent) newHighscoreFlag = true;
    }
    
    // Sao chép tên và điểm vào biến newHighscore để nhập tên
    if (newHighscoreFlag) {
        newHighscore.score = score;
        newHighscore.name[0] = '\0';
    }
}