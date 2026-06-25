#define SDL_MAIN_USE_CALLBACKS 1

#define SNAKE_GAME_SIZE 750.0
#define SNAKE_SIZE 25.0
#define BAR_DEPTH 250.0
#define TICK_DELTA 125

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>

typedef enum {
    SNAKE_DIR_UP,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
    SNAKE_DIR_RIGHT
} SnakeDirection;

class SnakeData {
public:
    typedef struct {
        SDL_FRect rect;
        SnakeDirection dir;
    }Snake;

    std::vector<Snake> snake;
    int snakeScore;
    SDL_FRect food{};
    bool isFoodEaten;

    SnakeData() {
        snakeScore = 0;
        food.x = SNAKE_SIZE * 3;
        food.y = SNAKE_SIZE * 3;
        food.w = SNAKE_SIZE;
        food.h = SNAKE_SIZE;
        isFoodEaten = false;
        snake.push_back(head);
    }

    static void HandleMovement(SnakeData *snakeData) {
        switch (snakeData->snake.front().dir) {
            case SNAKE_DIR_UP:
                snakeData->snake.front().rect.y -= SNAKE_SIZE;
                break;
            case SNAKE_DIR_DOWN:
                snakeData->snake.front().rect.y += SNAKE_SIZE;
                break;
            case SNAKE_DIR_LEFT:
                snakeData->snake.front().rect.x -= SNAKE_SIZE;
                break;
            case SNAKE_DIR_RIGHT:
                snakeData->snake.front().rect.x += SNAKE_SIZE;
                break;
            default:
                break;
        }
        if (snakeData->snake.front().rect.x >= SNAKE_GAME_SIZE) {
            snakeData->snake.front().rect.x = 0;
        }
        else if (snakeData->snake.front().rect.y >= SNAKE_GAME_SIZE) {
            snakeData->snake.front().rect.y = 0;
        }
        else if (snakeData->snake.front().rect.x < 0) {
            snakeData->snake.front().rect.x = SNAKE_GAME_SIZE - SNAKE_SIZE;
        }
        else if (snakeData->snake.front().rect.y < 0) {
            snakeData->snake.front().rect.y = SNAKE_GAME_SIZE - SNAKE_SIZE;
        }
    }

    static void HandleTailMovement(SnakeData *snakeData) {
        snakeData->snakeScore = snakeData->snake.size();
        if (snakeData->isFoodEaten) {
            snakeData->isFoodEaten = false;
            snakeData->snakeScore--;
        }
        for (unsigned int i = snakeData->snakeScore - 1; i > 0; i--) {
            snakeData->snake.at(i).dir = snakeData->snake.at(i - 1).dir;
            snakeData->snake.at(i).rect.x = snakeData->snake.at(i - 1).rect.x;
            snakeData->snake.at(i).rect.y = snakeData->snake.at(i - 1).rect.y;
        }
    }

    static void InsertTail(SnakeData *snakeData) {
        const SDL_FRect newRect = snakeData->snake.back().rect;
        SnakeDirection newDir = snakeData->snake.back().dir;
        Snake newTail{
        .rect = newRect,
        .dir = newDir};
        snakeData->snake.push_back(newTail);
    }

    static void HandleFoodEaten(SnakeData *snakeData, const float gameSize, const float snakeSize) {
        if (SDL_RectsEqualFloat(&snakeData->snake.front().rect, &snakeData->food)) {
            snakeData->food.x = RandomCoord(gameSize, snakeSize);
            snakeData->food.y = RandomCoord(gameSize, snakeSize);
            snakeData->isFoodEaten = true;
            InsertTail(snakeData);
        };
    }
private:
    Snake head{
        .rect = {
            .x = SNAKE_GAME_SIZE/2,
            .y = SNAKE_GAME_SIZE/2,
            .w = SNAKE_SIZE,
            .h = SNAKE_SIZE},
    .dir = SNAKE_DIR_RIGHT};

    static float RandomCoord(const float gameSize, const float snakeSize) {
        return SDL_rand(gameSize/snakeSize) * snakeSize;
    }
};

class Assets {
public:
    SDL_Texture *tiles;
    TTF_Font *font;
    SDL_Texture *textTexture;

    void RenderText(SDL_Renderer *renderer, const std::string& text, float x, float y, float w, float h) {
        SDL_Surface *textSurfaceBuf = TTF_RenderText_Solid(font, text.c_str(),
        0, SDL_Color{255, 255, 0, 255});
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurfaceBuf);
        SDL_DestroySurface(textSurfaceBuf);

        const SDL_FRect textRect{
            .x = x,
            .y = y,
            .w = w,
            .h = h};
        SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
    }

    SDL_FRect RenderMenu(SDL_Renderer *renderer, const std::string& text, float x, float y, float w, float h, bool selected) {
        SDL_Color color;
        if (selected) {
            color = SDL_Color{255, 255, 255, 255};
        }
        else {
            color = SDL_Color{255, 255, 0, 255};
        }
        SDL_Surface *textSurfaceBuf = TTF_RenderText_Solid(font, text.c_str(),
        0, color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurfaceBuf);
        SDL_DestroySurface(textSurfaceBuf);

        const SDL_FRect textRect{
            .x = x,
            .y = y,
            .w = w,
            .h = h};
        SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
        return textRect;
    }

    static void RenderSolid(SDL_Renderer *renderer, SDL_Texture *texture, float x, float y, float w, float h) {
        const SDL_FRect destRect{
            .x = x,
            .y = y,
            .w = w,
            .h = h
        };
        SDL_RenderTexture(renderer, texture, nullptr, &destRect);
    }

    static void DestroyAssets(Assets *assets) {
        SDL_DestroyTexture(assets->tiles);
        SDL_DestroyTexture(assets->textTexture);
    }
};

class SnakeGame {
    public:
    SDL_Window *window;
    SDL_Renderer *renderer;
    Assets *assets;
    bool isRunning = true;
    bool isGameOver = false;
    bool isPaused = false;
    int menuSelection;
    SnakeData *snakeData;
    Uint32 lastFrameTime = 0;

    static void HandleKeyPress(SnakeGame *state, const SDL_Keycode keyCode) {
        const SnakeDirection prevDir = state->snakeData->snake.front().dir;
        switch (keyCode) {
            case SDL_SCANCODE_ESCAPE:
                state->isPaused = !state->isPaused;
                state->lastFrameTime = SDL_GetTicks();
                SDL_Log("Menu opened!");
                break;
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_W:
                if (prevDir != SNAKE_DIR_DOWN) {
                    state->snakeData->snake.front().dir = SNAKE_DIR_UP;
                    SDL_Log("Set dir to UP");
                }
                if (state->isPaused) {
                    state->menuSelection--;
                }
                break;
            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_S:
                if (prevDir != SNAKE_DIR_UP) {
                    state->snakeData->snake.front().dir = SNAKE_DIR_DOWN;
                    SDL_Log("Set dir to DOWN");
                }
                if (state->isPaused) {
                    state->menuSelection++;
                }
                break;
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_A:
                if (prevDir != SNAKE_DIR_RIGHT) {
                    state->snakeData->snake.front().dir = SNAKE_DIR_LEFT;
                    SDL_Log("Set dir to LEFT");
                }
                break;
            case SDL_SCANCODE_RIGHT:
            case SDL_SCANCODE_D:
                if (prevDir != SNAKE_DIR_LEFT) {
                    state->snakeData->snake.front().dir = SNAKE_DIR_RIGHT;
                    SDL_Log("Set dir to RIGHT");
                }
                break;
            case SDL_SCANCODE_R:
                state->isGameOver = true;
                SDL_Log("Reset called!");
                break;
            case SDL_SCANCODE_RETURN:
                if (state->isPaused) {
                    switch (state->menuSelection) {
                        case 0:
                            state->isPaused = false;
                            break;
                        case 1:
                            state->isRunning = false;
                            break;
                        default:
                            break;
                    }
                }
            default:
                break;
        }
    }

    static void HandleCollision(SnakeGame *state) {
        for (int i = 1; i < state->snakeData->snake.size(); i++) {
            SDL_RectsEqualFloat(&state->snakeData->snake.front().rect,
                &state->snakeData->snake.at(i).rect) ? state->isGameOver = true : false;
        }
    }

    static void ResetGame(SnakeGame *state) {
        state->isGameOver = false;
        state->snakeData->isFoodEaten = false;
        state->snakeData->snake.erase(state->snakeData->snake.begin() + 1, state->snakeData->snake.end());
        state->snakeData->snake.front().rect.x = state->snakeData->snake.front().rect.y = SNAKE_GAME_SIZE/2;
        state->snakeData->snakeScore = 0;
        state->snakeData->food.x = state->snakeData->food.y = SNAKE_SIZE * 3;
        state->snakeData->snake.front().dir = SNAKE_DIR_RIGHT;
    }
};

void Rendering(SnakeGame *state) {
    SDL_SetRenderDrawColor(state->renderer, 15, 15, 15, 255);
    SDL_RenderClear(state->renderer);
    Assets::RenderSolid(state->renderer, state->assets->tiles, 0, 0, 750, 750);

    SDL_SetRenderDrawColor(state->renderer, 120, 0, 00, 255);
    SDL_RenderFillRect(state->renderer, &state->snakeData->food);

    SDL_SetRenderDrawColor(state->renderer, 120, 120, 0, 255);
    SDL_RenderFillRect(state->renderer, &state->snakeData->snake.front().rect);
    SDL_SetRenderDrawColor(state->renderer, 0, 60, 0, 255);
    for (int i = 1; i < state->snakeData->snake.size(); i++) {
        SDL_RenderFillRect(state->renderer, &state->snakeData->snake.at(i).rect);
    }
    std::string text;
    text = "Current score: " + std::to_string(state->snakeData->snakeScore);
    state->assets->RenderText(state->renderer, text, 10, SNAKE_GAME_SIZE + 10.0f, 300.0f, 50.0f);
    SDL_RenderPresent(state->renderer);
}

void Menu(SnakeGame *state) {
    SDL_SetRenderDrawColor(state->renderer, 10, 10, 10, 255);
    SDL_RenderClear(state->renderer);
    std::string text;
    bool playSelected = false, quitSelected = false;
    if (state->menuSelection == 0) {
        playSelected = true;
    }
    else if (state->menuSelection == 1) {
        quitSelected = true;
    }
    else if (state->menuSelection > 1) {
        state->menuSelection = 0;
    }
    else if (state->menuSelection < 0) {
        state->menuSelection = 1;
    }
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_FPoint mousePos{
    .x = mouseX,
    .y = mouseY};
    text = "Play";
    SDL_FRect playRect = state->assets->RenderMenu(state->renderer, text, SNAKE_GAME_SIZE/2 - 50, SNAKE_GAME_SIZE/2 - 25,
        100.0f, 50.0f, playSelected);
    if (SDL_PointInRectFloat(&mousePos, &playRect)) {
        state->menuSelection = 0;
    }
    text = "Quit";
    SDL_FRect quitRect = state->assets->RenderMenu(state->renderer, text, SNAKE_GAME_SIZE/2 - 50, SNAKE_GAME_SIZE/2 + 25,
        100.0f, 50.0f, quitSelected);
    if (SDL_PointInRectFloat(&mousePos, &quitRect)) {
        state->menuSelection = 1;
    }

    SDL_RenderPresent(state->renderer);
}

SDL_AppResult SDL_AppIterate(void* AppState) {
    auto* state = static_cast<SnakeGame*>(AppState);
    auto now = SDL_GetTicks();

    if (!state->isPaused) {
        // Game logic
        while ((now - state->lastFrameTime) > TICK_DELTA)
        {
            SnakeData::HandleTailMovement(state->snakeData);
            SnakeData::HandleMovement(state->snakeData);
            SnakeGame::HandleCollision(state);
            if (state->isGameOver) {
                SnakeGame::ResetGame(state);
            }
            SnakeData::HandleFoodEaten(state->snakeData, SNAKE_GAME_SIZE, SNAKE_SIZE);
            state->lastFrameTime += TICK_DELTA;
        }

        // Rendering
        Rendering(state);
    }
    else {
        // Menu Screen
        Menu(state);
    }

    return state->isRunning ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppInit(void** AppState, int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    };

    auto *state = new SnakeGame;
    SDL_CreateWindowAndRenderer("Snake Game", SNAKE_GAME_SIZE, SNAKE_GAME_SIZE + 75,
        SDL_WINDOW_KEYBOARD_GRABBED, &state->window, &state->renderer);
    if (!state->window || !state->renderer) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(state->window);
        return SDL_APP_FAILURE;
    }

    // Assets
    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize SDL_ttf: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Surface *tileSurface = SDL_LoadBMP("./assets/images/tiles.bmp");
    if (!tileSurface) {
        SDL_Log("Tile loading failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state->assets = new Assets;
    state->assets->tiles = SDL_CreateTextureFromSurface(state->renderer, tileSurface);
    SDL_DestroySurface(tileSurface);
    state->assets->font = TTF_OpenFont("./assets/fonts/asimov-font/Asimov-MwEn.otf", 16.0);
    if (!state->assets->font) {
        SDL_Log("Couldn't open font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Surface *textSurfaceBuf = TTF_RenderText_Solid(state->assets->font, "Calculating...",
        0, SDL_Color{255, 255, 0, 255});
    state->assets->textTexture = SDL_CreateTextureFromSurface(state->renderer, textSurfaceBuf);
    SDL_DestroySurface(textSurfaceBuf);

    // Initialize game logic
    state->isRunning = true;
    state->snakeData = new SnakeData;
    state->menuSelection = 0;
    state->isPaused = true;
    state->lastFrameTime = SDL_GetTicks();

    *AppState = state;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* AppState, const SDL_Event* Event) {
    const auto state = static_cast<SnakeGame*>(AppState);

    switch (Event->type) {
        case SDL_EVENT_QUIT:
            state->isRunning = false;
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            SDL_Log("Window closed requested");
            state->isRunning = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            SnakeGame::HandleKeyPress(state, Event->key.scancode);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            break;
        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* AppState, SDL_AppResult Result) {
    SDL_Log("App Quit");

    if (AppState) {
        auto* state = static_cast<SnakeGame*>(AppState);
        Assets::DestroyAssets(state->assets);
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        delete state->assets;
        delete state->snakeData;
        delete state;
    }
}

