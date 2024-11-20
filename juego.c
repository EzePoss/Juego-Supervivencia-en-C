#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TILE_SIZE 32
#define MAP_SIZE 20
#define GAME_SPEED 2

#define NIVEL_CRITICO 0
#define NIVEL_PELIGRO 15
#define NIVEL_RECUPERACION 60

typedef struct
{
    int x, y;
    int salud;
    int hambre;
    int sed;
    int energia;
    int inventario[5];
    int tiene_espada;
    int tiene_armadura;
    int golpes_recibidos;
} Jugador;

typedef struct
{
    int x, y;
    int vida;
    int activo;
    int daño_causado;
    int ultimo_ataque;
} Animal;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *arbolTexture;
    SDL_Texture *rocaTexture;
    SDL_Texture *amenazaTexture;
    SDL_Texture *personajeTexture;
    SDL_Texture *personajeRocaTexture;
    SDL_Texture *personajeCompletoTexture;
    SDL_Texture *carneTexture;
    TTF_Font *font;
} GameState;

int mapa[MAP_SIZE][MAP_SIZE];
Jugador jugador;
Animal animal_actual = {0, 0, 100, 0, 0, 0};
int experiencia = 0;
int combate_activo = 0;
Uint32 ultimo_update_vida = 0;

const int islaShape[MAP_SIZE][MAP_SIZE] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}};

SDL_Color colorAgua = {64, 224, 208, 255};
SDL_Color colorArena = {238, 214, 175, 255};
SDL_Color colorPasto = {34, 139, 34, 255};
SDL_Color colorBlanco = {255, 255, 255, 255};

void render_text(GameState *game, const char *text, int x, int y)
{
    SDL_Surface *surface = TTF_RenderText_Solid(game->font, text, colorBlanco);
    if (!surface)
        return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    if (!texture)
    {
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(game->renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void render_tile(SDL_Renderer *renderer, int x, int y, SDL_Color color)
{
    SDL_Rect tile = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &tile);
}

void render_game_over(GameState *game)
{
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 128);
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(game->renderer, &overlay);
    render_text(game, "¡GAME OVER!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 30);
    render_text(game, "Presiona R para reiniciar o Q para salir", WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 + 10);
}

void actualizar_texturas_jugador(void)
{
    if (jugador.tiene_espada && jugador.tiene_armadura)
    {
        jugador.tiene_armadura = 1;
        jugador.tiene_espada = 1;
    }
    else if (jugador.tiene_espada)
    {
        jugador.tiene_armadura = 0;
        jugador.tiene_espada = 1;
    }
    else
    {
        jugador.tiene_armadura = 0;
        jugador.tiene_espada = 0;
    }
}

void aplicar_daño_jugador(float daño)
{
    jugador.golpes_recibidos++;

    if (jugador.tiene_armadura && jugador.tiene_espada)
    {
        if (jugador.golpes_recibidos >= 3)
        {
            jugador.tiene_armadura = 0;
            jugador.golpes_recibidos = 0;
        }
    }
    else if (jugador.tiene_espada)
    {
        if (jugador.golpes_recibidos >= 2)
        {
            jugador.salud = 0;
        }
    }
    else
    {
        jugador.salud -= daño;
        if (jugador.golpes_recibidos >= 3)
        {
            jugador.salud = 0;
        }
    }
    actualizar_texturas_jugador();
}

int distancia_manhattan(int x1, int y1, int x2, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2);
}

void actualizar_estado_jugador(void)
{
    Uint32 tiempo_actual = SDL_GetTicks();
    if (tiempo_actual - ultimo_update_vida < 1000)
        return;

    ultimo_update_vida = tiempo_actual;

    if (jugador.hambre > NIVEL_RECUPERACION && jugador.sed > NIVEL_RECUPERACION)
    {
        if (jugador.salud < 20)
            jugador.salud++;
    }

    if (jugador.hambre <= NIVEL_CRITICO || jugador.sed <= NIVEL_CRITICO)
    {
        jugador.salud = (jugador.salud > 0) ? jugador.salud - 2 : 0;
    }
    else if (jugador.hambre <= NIVEL_PELIGRO && jugador.sed <= NIVEL_PELIGRO)
    {
        jugador.salud = (jugador.salud > 0) ? jugador.salud - 1 : 0;
    }
    else if (jugador.hambre <= NIVEL_PELIGRO || jugador.sed <= NIVEL_PELIGRO)
    {
        if (rand() % 2 == 0)
        {
            jugador.salud = (jugador.salud > 0) ? jugador.salud - 1 : 0;
        }
    }
}

void init_mapa(void)
{
    for (int i = 0; i < MAP_SIZE; i++)
    {
        for (int j = 0; j < MAP_SIZE; j++)
        {
            if (islaShape[i][j] == 0)
            {
                mapa[i][j] = 0;
            }
            else
            {
                int es_borde = 0;
                if (i > 0 && islaShape[i - 1][j] == 0)
                    es_borde = 1;
                if (i < MAP_SIZE - 1 && islaShape[i + 1][j] == 0)
                    es_borde = 1;
                if (j > 0 && islaShape[i][j - 1] == 0)
                    es_borde = 1;
                if (j < MAP_SIZE - 1 && islaShape[i][j + 1] == 0)
                    es_borde = 1;

                if (es_borde)
                {
                    mapa[i][j] = 1;
                }
                else
                {
                    int r = rand() % 100;
                    if (r < 20)
                        mapa[i][j] = 4;
                    else if (r < 30)
                        mapa[i][j] = 3;
                    else if (r < 35)
                        mapa[i][j] = 5;
                    else
                        mapa[i][j] = 2;
                }
            }
        }
    }
}

void reiniciar_juego(Jugador *jugador)
{
    jugador->x = MAP_SIZE / 2;
    jugador->y = MAP_SIZE / 2;
    jugador->salud = 20;
    jugador->hambre = 20;
    jugador->sed = 20;
    jugador->energia = 100;
    jugador->tiene_espada = 0;
    jugador->tiene_armadura = 0;
    jugador->golpes_recibidos = 0;
    experiencia = 0;
    for (int i = 0; i < 5; i++)
    {
        jugador->inventario[i] = 0;
    }
    animal_actual.activo = 0;
    animal_actual.daño_causado = 0;
    animal_actual.ultimo_ataque = 0;
    combate_activo = 0;
}

int init_game(GameState *game)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL error: %s\n", SDL_GetError());
        return 0;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("SDL_image error: %s\n", IMG_GetError());
        return 0;
    }

    if (TTF_Init() == -1)
    {
        printf("TTF error: %s\n", TTF_GetError());
        return 0;
    }

    game->window = SDL_CreateWindow("Isla Survival",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!game->window)
        return 0;

    game->renderer = SDL_CreateRenderer(game->window, -1,
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer)
        return 0;

    game->font = TTF_OpenFont("8514oem.ttf", 24);
    if (!game->font)
    {
        printf("Font error: %s\n", TTF_GetError());
        return 0;
    }

    game->arbolTexture = IMG_LoadTexture(game->renderer, "arbol.png");
    game->rocaTexture = IMG_LoadTexture(game->renderer, "roca.png");
    game->amenazaTexture = IMG_LoadTexture(game->renderer, "animalmalo.png");
    game->personajeTexture = IMG_LoadTexture(game->renderer, "personaje.png");
    game->personajeRocaTexture = IMG_LoadTexture(game->renderer, "personajeroca.png");
    game->personajeCompletoTexture = IMG_LoadTexture(game->renderer, "personajecompleto.png");
    game->carneTexture = IMG_LoadTexture(game->renderer, "carne.png");

    if (!game->arbolTexture || !game->rocaTexture || !game->amenazaTexture ||
        !game->personajeTexture || !game->personajeRocaTexture ||
        !game->personajeCompletoTexture || !game->carneTexture)
    {
        printf("Texture error: %s\n", IMG_GetError());
        return 0;
    }

    return 1;
}

void handle_combat(SDL_Event *event)
{
    if (!animal_actual.activo)
        return;

    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_f)
    {
        int dist = distancia_manhattan(jugador.x, jugador.y, animal_actual.x, animal_actual.y);
        if (dist <= 1)
        {
            float daño_base = jugador.tiene_espada ? 20.0f : 10.0f;

            if (dist == 0)
                daño_base *= 1.5f;

            animal_actual.vida -= daño_base;

            if (animal_actual.vida <= 0)
            {
                mapa[animal_actual.y][animal_actual.x] = 6;
                animal_actual.activo = 0;
                combate_activo = 0;
                experiencia += 20;
                return;
            }
        }
    }

    Uint32 tiempo_actual = SDL_GetTicks();
    if (tiempo_actual - animal_actual.ultimo_ataque > 1000)
    {
        int dist = distancia_manhattan(jugador.x, jugador.y, animal_actual.x, animal_actual.y);
        if (dist <= 1)
        {
            float daño_base = dist == 0 ? 1.5f : 1.0f;
            aplicar_daño_jugador(daño_base);
            animal_actual.ultimo_ataque = tiempo_actual;
        }
    }
}

void render_game(GameState *game)
{
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    for (int y = 0; y < MAP_SIZE; y++)
    {
        for (int x = 0; x < MAP_SIZE; x++)
        {
            SDL_Rect destRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

            if (islaShape[y][x] == 0)
            {
                render_tile(game->renderer, x, y, colorAgua);
            }
            else if (mapa[y][x] == 1)
            {
                render_tile(game->renderer, x, y, colorArena);
            }
            else
            {
                render_tile(game->renderer, x, y, colorPasto);

                switch (mapa[y][x])
                {
                case 3: // Roca
                    SDL_RenderCopy(game->renderer, game->rocaTexture, NULL, &destRect);
                    break;
                case 4: // Árbol
                    SDL_RenderCopy(game->renderer, game->arbolTexture, NULL, &destRect);
                    break;
                case 5: // Animal
                    SDL_RenderCopy(game->renderer, game->amenazaTexture, NULL, &destRect);
                    break;
                case 6: // Carne
                    SDL_RenderCopy(game->renderer, game->carneTexture, NULL, &destRect);
                    break;
                }
            }
        }
    }

    SDL_Rect playerRect = {jugador.x * TILE_SIZE, jugador.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_Texture *currentTexture;

    if (jugador.tiene_espada && jugador.tiene_armadura)
    {
        currentTexture = game->personajeCompletoTexture;
    }
    else if (jugador.tiene_espada)
    {
        currentTexture = game->personajeRocaTexture;
    }
    else
    {
        currentTexture = game->personajeTexture;
    }

    SDL_RenderCopy(game->renderer, currentTexture, NULL, &playerRect);

    int barWidth = 100;
    int barHeight = 20;
    int startX = 650;
    int startY = 30;
    int textY = startY - 20;
    int spacing = 50;

    char statsText[32];
    sprintf(statsText, "SALUD: %d/20", jugador.salud);
    render_text(game, statsText, startX, textY);
    SDL_Rect healthBar = {startX, startY, (jugador.salud * barWidth) / 20, barHeight};
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(game->renderer, &healthBar);

    sprintf(statsText, "HAMBRE: %d/20", jugador.hambre);
    render_text(game, statsText, startX, textY + spacing);
    SDL_Rect hungerBar = {startX, startY + spacing, (jugador.hambre * barWidth) / 20, barHeight};
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(game->renderer, &hungerBar);

    sprintf(statsText, "SED: %d/20", jugador.sed);
    render_text(game, statsText, startX, textY + spacing * 2);
    SDL_Rect thirstBar = {startX, startY + spacing * 2, (jugador.sed * barWidth) / 20, barHeight};
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(game->renderer, &thirstBar);

    if (combate_activo && animal_actual.activo)
    {
        SDL_Rect animalHealthBar = {
            animal_actual.x * TILE_SIZE,
            animal_actual.y * TILE_SIZE - 10,
            (animal_actual.vida * TILE_SIZE) / 100,
            5};
        SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(game->renderer, &animalHealthBar);
    }

    if (jugador.salud <= 0)
    {
        render_game_over(game);
    }

    SDL_RenderPresent(game->renderer);
}

void handle_input(SDL_Event *event, int *running, int *restart)
{
    while (SDL_PollEvent(event))
    {
        if (event->type == SDL_QUIT)
        {
            *running = 0;
        }
        else if (event->type == SDL_KEYDOWN)
        {
            if (jugador.salud <= 0)
            {
                if (event->key.keysym.sym == SDLK_r)
                {
                    *restart = 1;
                }
                else if (event->key.keysym.sym == SDLK_q)
                {
                    *running = 0;
                }
                return;
            }

            switch (event->key.keysym.sym)
            {
            case SDLK_w:
                if (jugador.y > 0 && islaShape[jugador.y - 1][jugador.x])
                    jugador.y--;
                break;
            case SDLK_s:
                if (jugador.y < MAP_SIZE - 1 && islaShape[jugador.y + 1][jugador.x])
                    jugador.y++;
                break;
            case SDLK_a:
                if (jugador.x > 0 && islaShape[jugador.y][jugador.x - 1])
                    jugador.x--;
                break;
            case SDLK_d:
                if (jugador.x < MAP_SIZE - 1 && islaShape[jugador.y][jugador.x + 1])
                    jugador.x++;
                break;
            case SDLK_q:
                *running = 0;
                break;
            case SDLK_e:
                if (mapa[jugador.y][jugador.x] == 1)
                {
                    jugador.sed = (jugador.sed + 5 > 20) ? 20 : jugador.sed + 5;
                }
                else
                {
                    switch (mapa[jugador.y][jugador.x])
                    {
                    case 0:
                        jugador.sed = (jugador.sed + 5 > 20) ? 20 : jugador.sed + 5;
                        break;
                    case 3:
                        if (!jugador.tiene_espada)
                        {
                            jugador.tiene_espada = 1;
                        }
                        else if (!jugador.tiene_armadura)
                        {
                            jugador.tiene_armadura = 1;
                        }
                        mapa[jugador.y][jugador.x] = 2;
                        break;
                    case 4:
                        jugador.inventario[0]++;
                        experiencia += 10;
                        mapa[jugador.y][jugador.x] = 2;
                        break;
                    case 6:
                        jugador.hambre = (jugador.hambre + 5 > 20) ? 20 : jugador.hambre + 5;
                        mapa[jugador.y][jugador.x] = 2;
                        break;
                    }
                }
                break;
            case SDLK_f:
                handle_combat(event);
                break;
            }

            if (animal_actual.activo)
            {
                int dist = distancia_manhattan(jugador.x, jugador.y, animal_actual.x, animal_actual.y);
                if (dist <= 1)
                {
                    combate_activo = 1;
                }
            }

            if (mapa[jugador.y][jugador.x] == 5 && !animal_actual.activo)
            {
                combate_activo = 1;
                animal_actual.x = jugador.x;
                animal_actual.y = jugador.y;
                animal_actual.vida = 100;
                animal_actual.activo = 1;
                animal_actual.daño_causado = 0;
                animal_actual.ultimo_ataque = SDL_GetTicks();
            }
        }
    }
}

void cleanup(GameState *game)
{
    SDL_DestroyTexture(game->arbolTexture);
    SDL_DestroyTexture(game->rocaTexture);
    SDL_DestroyTexture(game->amenazaTexture);
    SDL_DestroyTexture(game->personajeTexture);
    SDL_DestroyTexture(game->personajeRocaTexture);
    SDL_DestroyTexture(game->personajeCompletoTexture);
    SDL_DestroyTexture(game->carneTexture);
    TTF_CloseFont(game->font);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    GameState game;
    SDL_Event event;
    int running = 1;

    if (!init_game(&game))
    {
        return 1;
    }

    while (running)
    {
        int restart = 0;
        init_mapa();
        reiniciar_juego(&jugador);

        while (running && !restart)
        {
            handle_input(&event, &running, &restart);
            actualizar_estado_jugador();
            render_game(&game);
            SDL_Delay(16);

            if (jugador.salud > 0)
            {
                if (rand() % (100 * GAME_SPEED) == 0)
                {
                    jugador.sed = (jugador.sed > 0) ? jugador.sed - 1 : 0;
                    jugador.hambre = (jugador.hambre > 0) ? jugador.hambre - 1 : 0;
                }
            }
        }
    }

    cleanup(&game);
    return 0;
}