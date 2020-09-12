#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#include <raylib.h>

struct Entity {
    int position_x;
    int position_y;
};

struct EnemyList {
    struct Entity     enemy;
    struct EnemyList* next;
};

static struct {
    enum {
        GAME_INIT,
        GAME_RUNING,
        GAME_OVER,
    } state;
    int speed;
    int score;
} game = {
    .state = GAME_INIT,
    .speed = 3,
    .score = 0
};

static int screen_width;
static int screen_height;

static const Color text_color       = { 0,   0,   0,   255 };
static const Color player_color     = { 0,   0,   0,   255 };
static const Color enemy_color      = { 255, 0,   0,   255 };
static const Color background_color = { 255, 255, 255, 255 };

static const int wall_width = 30;

static const int entity_width  = 30;
static const int entity_height = 30;

static const int enemy_spawn_chance = 10;

static struct Entity player;
static int           player_direction;

static struct EnemyList* enemy_list;
static struct EnemyList* enemy_list_end;

static inline void* memAlloc(size_t size) {
    void* res = malloc(size);
    if (res == NULL) {
        fprintf(stderr, "Error: malloc fail\n");
        exit(EXIT_FAILURE);
    }
    memset(res, 0, size);
    return res;
}

static inline void initEnemyList(void) {
    enemy_list = memAlloc(sizeof(struct EnemyList));
    enemy_list_end = enemy_list;
}

static inline void appendEnemyList(struct Entity enemy) {
    enemy_list_end -> enemy = enemy;
    enemy_list_end -> next = memAlloc(sizeof(struct EnemyList));
    enemy_list_end = enemy_list_end -> next;
}

static inline void initPlayer(void) {
    player.position_y = screen_height - (entity_height + 10);
    player.position_x = screen_width / 3;
    player_direction = 1;
}

static inline void initGame(void) {
    game.speed = 3;
    game.score = 0;
    game.state = GAME_INIT;
}

static void resetEnemyList(void) {
    struct EnemyList* enemies = enemy_list;
    while (enemies != NULL) {
        struct EnemyList* old = enemies;
        free(enemies);
        enemies = old -> next;
    }
    initEnemyList();
}

static void drawWalls(void) {
    DrawRectangle(
        0,
        0,
        wall_width,
        screen_height,
        enemy_color
    );
    DrawRectangle(
        screen_width - wall_width,
        0,
        wall_width,
        screen_height,
        enemy_color
    );
}

static void drawEnemy(struct Entity enemy) {
    DrawRectangle(
        enemy.position_x,
        enemy.position_y,
        entity_width,
        entity_height,
        enemy_color
    );
}

static void drawEnemies(void) {
    struct EnemyList* enemies = enemy_list;
    while (enemies != NULL) {
        drawEnemy(enemies -> enemy);
        enemies = enemies -> next;
    }
}

static void spownEnemies(void) {
    if ((rand() % enemy_spawn_chance) == 0) {
        int position_x = 
            ((rand() + wall_width) % (screen_width - (2 * wall_width)));
        appendEnemyList(
            (struct Entity) {
                .position_y = -entity_height,
                .position_x = position_x
            }
        );
    }
}

static void moveEnemies(void) {
    struct EnemyList* enemies = enemy_list;
    while (enemies != NULL) {
        enemies -> enemy.position_y += (game.speed + 3);
        enemies = enemies -> next;
    }
}

static void removeEnemies(void) {
    while (enemy_list -> enemy.position_y > screen_height) {
        struct EnemyList* tmp = enemy_list;
        enemy_list = enemy_list -> next;
        free(tmp);
    }
}

static void collision(void) {
    if (player.position_x + entity_width > screen_width - wall_width ||
        player.position_x < wall_width) {
        game.state = GAME_OVER;
    }
    
    struct EnemyList* enemies = enemy_list;
    while (enemies -> enemy.position_y + entity_height > player.position_y) {
        if (enemies -> enemy.position_x > player.position_x &&
            enemies -> enemy.position_x < entity_width + player.position_x &&
            !(enemies -> enemy.position_y > entity_height + player.position_y)
        ) {
            game.state = GAME_OVER;
        }
        enemies = enemies -> next;
    }
}

static void drawPlayer(void) {
    DrawRectangle(
        player.position_x,
        player.position_y,
        entity_width,
        entity_height,
        player_color
    );
}

static void movePlayer(void) {
    player.position_x += game.speed * player_direction;
}

static void drawScore(void) {
    char buf[250];
    sprintf(buf, "Score: %d", game.score);
    DrawText(buf, 40, 20, 20, text_color);
}

static void gameInput(void) {
    if (IsKeyPressed(' ')) {
        switch (game.state) {
        case GAME_INIT:
            game.state = GAME_RUNING;
            break;
        case GAME_RUNING:
            player_direction = -player_direction;
            game.speed++;
            break;
        case GAME_OVER:
            resetEnemyList();
            initPlayer();
            initGame();
            break;
        }
    }
}

static void drawGame() {
    ClearBackground(background_color);
    switch (game.state) {
    case GAME_INIT:
        DrawText(
            "Red is death Press space to start",
            (screen_width / 2),
            (screen_height / 2),
            20, 
            text_color
        );
        break;
    case GAME_OVER:
        DrawText(
            "Game Over Press space to restart",
            (screen_width / 2),
            (screen_height / 2),
            20, 
            text_color
        );
        /* Falls through. */
    case GAME_RUNING:
        drawPlayer();
        drawEnemies();
        drawWalls();
        drawScore();
        break;
    }
}

static void updateGame(void) {
    switch (game.state) {
    case GAME_RUNING:
        game.score += ((game.speed / 20) + 1);
        spownEnemies();
        moveEnemies();
        removeEnemies();
        movePlayer();
        collision();
    case GAME_INIT:
    case GAME_OVER:
        break;
    }
}

int main(void) {
    InitWindow(screen_width, screen_height, "Game!!!");
    ToggleFullscreen();

    SetTargetFPS(60);

    screen_width  = GetScreenWidth();
    screen_height = GetScreenHeight();

    srand(time(0)); 
    initEnemyList();
    initPlayer();

    while (!WindowShouldClose()) {
        BeginDrawing();

        updateGame();
        drawGame();
        gameInput();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
