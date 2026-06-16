#define SDL_MAIN_USE_CALLBACKS 1

#define SNAKE_GAME_SIZE 750.0
#define SNAKE_SIZE 25.0
#define BAR_DEPTH 250.0
#define TICK_DELTA 125

#include "SDL3/SDL.h"
#include <SDL3/SDL_main.h>
#include <vector>

typedef enum {
    SNAKE_DIR_UP,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
    SNAKE_DIR_RIGHT
} SnakeDirection;

typedef struct SnakeData {
    std::vector<SDL_FRect> snake;
    SDL_FRect food;
    SnakeDirection dir = SNAKE_DIR_RIGHT;
    bool isFoodEaten = false;
} SnakeData;

typedef struct GameState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool isRunning = true;
    bool isGameOver = false;
    SnakeData *snakeData;
    Uint32 lastFrameTime;
} GameState;

float randomCoord() {
    constexpr int gameSize = static_cast<int>(SNAKE_GAME_SIZE);
    constexpr int snakeSize = static_cast<int>(SNAKE_SIZE);
    return SDL_rand(gameSize/snakeSize) * snakeSize;
}

void handleMovement(SnakeData *snakeData) {
    switch (snakeData->dir) {
        case SNAKE_DIR_UP:
            snakeData->snake.front().y -= SNAKE_SIZE;
            break;
        case SNAKE_DIR_DOWN:
            snakeData->snake.front().y += SNAKE_SIZE;
            break;
        case SNAKE_DIR_LEFT:
            snakeData->snake.front().x -= SNAKE_SIZE;
            break;
        case SNAKE_DIR_RIGHT:
            snakeData->snake.front().x += SNAKE_SIZE;
            break;
        default:
            break;
    }
    if (snakeData->snake.front().x >= SNAKE_GAME_SIZE) {
        snakeData->snake.front().x = 0;
    }
    else if (snakeData->snake.front().y >= SNAKE_GAME_SIZE) {
        snakeData->snake.front().y = 0;
    }
    else if (snakeData->snake.front().x < 0) {
        snakeData->snake.front().x = SNAKE_GAME_SIZE - SNAKE_SIZE;
    }
    else if (snakeData->snake.front().y < 0) {
        snakeData->snake.front().y = SNAKE_GAME_SIZE - SNAKE_SIZE;
    }
}

void handleTailMovement(SnakeData *snakeData) {
    unsigned int vectorSize = snakeData->snake.size();
    if (snakeData->isFoodEaten) {
        snakeData->isFoodEaten = false;
        vectorSize--;
    }
    for (unsigned int i = vectorSize - 1; i > 0; i--) {
        snakeData->snake.at(i).y = snakeData->snake.at(i - 1).y;
        snakeData->snake.at(i).x = snakeData->snake.at(i - 1).x;
    }
}

void handleKeyPress(GameState *state, SDL_Keycode keyCode) {
    SnakeDirection prevDir = state->snakeData->dir;
    switch (keyCode) {
        case SDL_SCANCODE_ESCAPE:
            state->isRunning = false;
            break;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
            if (prevDir != SNAKE_DIR_DOWN) {
                state->snakeData->dir = SNAKE_DIR_UP;
                SDL_Log("Set dir to UP");
            }
            break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
            if (prevDir != SNAKE_DIR_UP) {
                state->snakeData->dir = SNAKE_DIR_DOWN;
                SDL_Log("Set dir to DOWN");
            }
            break;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
            if (prevDir != SNAKE_DIR_RIGHT) {
                state->snakeData->dir = SNAKE_DIR_LEFT;
                SDL_Log("Set dir to LEFT");
            }
            break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
            if (prevDir != SNAKE_DIR_LEFT) {
                state->snakeData->dir = SNAKE_DIR_RIGHT;
                SDL_Log("Set dir to RIGHT");
            }
            break;
        case SDL_SCANCODE_R:
            state->isGameOver = true;
            SDL_Log("Reset called!");
            break;
        default:
            break;
    }
}

void handleCollision(GameState *state) {
    for (int i = 1; i < state->snakeData->snake.size(); i++) {
        bool collide = SDL_RectsEqualFloat(&state->snakeData->snake.front(),
            &state->snakeData->snake.at(i));
        if (collide) {
            state->isGameOver = true;
            SDL_Log("Game over!");
        }
    }
}

void resetGame(GameState *state) {
    state->isGameOver = false;
    state->snakeData->isFoodEaten = false;
    state->snakeData->snake.erase(state->snakeData->snake.begin() + 1, state->snakeData->snake.end());
    state->snakeData->snake.front().x = state->snakeData->snake.front().y = SNAKE_GAME_SIZE/2;
    state->snakeData->food.x = state->snakeData->food.y = SNAKE_SIZE * 3;
    state->snakeData->dir = SNAKE_DIR_RIGHT;
}

void insertTail(SnakeData *snakeData) {
    SDL_FRect rect = snakeData->snake.back();
    snakeData->snake.push_back(rect);
}

SDL_AppResult SDL_AppIterate(void* AppState) {
    auto* state = static_cast<GameState*>(AppState);
    auto now = SDL_GetTicks();

    // Clear screen & Generate info bar
    SDL_SetRenderDrawColor(state->renderer, 15, 15, 15, 255);
    SDL_RenderClear(state->renderer);

    // Game logic
    while ((now - state->lastFrameTime) > TICK_DELTA)
    {
        handleTailMovement(state->snakeData);
        handleMovement(state->snakeData);
        handleCollision(state);
        if (state->isGameOver) {
            resetGame(state);
        }

        if (SDL_RectsEqualFloat(&state->snakeData->snake.front(), &state->snakeData->food)) {
            state->snakeData->food.x = randomCoord();
            state->snakeData->food.y = randomCoord();
            state->snakeData->isFoodEaten = true;
            insertTail(state->snakeData);
        };
        state->lastFrameTime += TICK_DELTA;
    }

    // Rendering
    SDL_SetRenderDrawColor(state->renderer, 120, 0, 00, 255);
    SDL_RenderFillRect(state->renderer, &state->snakeData->food);

    SDL_SetRenderDrawColor(state->renderer, 120, 120, 0, 255);
    SDL_RenderFillRect(state->renderer, &state->snakeData->snake.front());
    SDL_SetRenderDrawColor(state->renderer, 0, 60, 0, 255);
    for (int i = 1; i < state->snakeData->snake.size(); i++) {
        SDL_RenderFillRect(state->renderer, &state->snakeData->snake[i]);
    }

    SDL_RenderPresent(state->renderer);

    return state->isRunning ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppInit(void** AppState, int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    };

    auto *state = new GameState;
    SDL_CreateWindowAndRenderer("Snake Game", SNAKE_GAME_SIZE, SNAKE_GAME_SIZE,
        SDL_WINDOW_KEYBOARD_GRABBED, &state->window, &state->renderer);
    if (!state->window || !state->renderer) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(state->window);
        return SDL_APP_FAILURE;
    }
    state->isRunning = true;

    state->snakeData = new SnakeData;

    SDL_FRect head;
    head.w = head.h = SNAKE_SIZE;
    head.x = head.y = SNAKE_GAME_SIZE/2;
    state->snakeData->snake.push_back(head);
    state->snakeData->food.w = state->snakeData->food.h = SNAKE_SIZE;
    state->snakeData->food.x = state->snakeData->food.y = SNAKE_SIZE * 3;

    state->lastFrameTime = SDL_GetTicks();

    *AppState = state;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* AppState, SDL_Event* Event) {
    auto state = static_cast<GameState*>(AppState);

    switch (Event->type) {
        case SDL_EVENT_QUIT:
            state->isRunning = false;
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            SDL_Log("Window closed requested");
            state->isRunning = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            handleKeyPress(state, Event->key.scancode);
            break;
        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* AppState, SDL_AppResult Result) {
    SDL_Log("App Quit");

    if (AppState) {
        auto* state = static_cast<GameState*>(AppState);
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        delete state->snakeData;
        delete state;
    }
}