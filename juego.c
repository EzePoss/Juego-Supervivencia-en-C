#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

// Constantes del juego
#define ANCHO_VENTANA 1000
#define ALTO_VENTANA 750
#define TAM_CASILLA 28
#define TAM_MAPA 27
#define VEL_JUEGO 2
#define MAX_ANIMALES 3
#define MAX_MONEDAS 10
#define RETARDO_MOV_ANIMAL 500

// Constantes de jugador y combate
#define VIDA_BASE 50
#define VIDA_ARMADURA 70
#define ATAQUE_BASE 10
#define ATAQUE_ESPADA 15
#define ATAQUE_COMPLETO 20
#define VIDA_ANIMAL 40
#define ATAQUE_ANIMAL 5
#define NIVEL_CRITICO_BASE 6
#define NIVEL_CRITICO_ARMADURA 10
#define NIVEL_RECUPERACION_BASE 18
#define NIVEL_RECUPERACION_ARMADURA 30

typedef struct
{
    int x, y, salud, hambre, sed, energia;
    int inventario[5];
    int tiene_espada, tiene_armadura, golpes_recibidos;
    int vida_maxima, ataque_actual;
    int monedas_recogidas;
} Jugador;

typedef struct
{
    int x, y, vida, activo, danio_causado;
    Uint32 ultimo_ataque, ultimo_movimiento;
} Animal;

typedef struct
{
    int x, y, activa;
} Moneda;

typedef struct
{
    SDL_Window *ventana;
    SDL_Renderer *renderizador;
    SDL_Texture *textura_arbol, *textura_roca, *textura_amenaza, *textura_personaje,
        *textura_personaje_roca, *textura_personaje_completo, *textura_carne, *textura_oro;
    TTF_Font *fuente;
    Mix_Music *musica;
    Mix_Chunk *sonido_lucha, *sonido_gameover, *sonido_moneda;
} EstadoJuego;

// Variables globales
int mapa[TAM_MAPA][TAM_MAPA], experiencia = 0;
Uint32 ultima_actualizacion_vida = 0;
Jugador jugador;
Animal animales[MAX_ANIMALES];
Moneda moneda_actual = {0};
SDL_Color color_agua = {64, 224, 208, 255};
SDL_Color color_arena = {238, 214, 175, 255};
SDL_Color color_pasto = {34, 139, 34, 255};
SDL_Color color_blanco = {255, 255, 255, 255};

// Forma de la isla (matriz original mantenida para compatibilidad)
const int forma_isla[27][27] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

/* Actualiza las estadísticas del jugador basadas en su equipamiento */
void actualizar_stats_jugador(void)
{
    jugador.vida_maxima = jugador.tiene_armadura ? VIDA_ARMADURA : VIDA_BASE;
    jugador.ataque_actual = jugador.tiene_espada ? (jugador.tiene_armadura ? ATAQUE_COMPLETO : ATAQUE_ESPADA) : ATAQUE_BASE;
    jugador.hambre = jugador.hambre > VIDA_BASE ? VIDA_BASE : jugador.hambre;
    jugador.sed = jugador.sed > VIDA_BASE ? VIDA_BASE : jugador.sed;
    jugador.salud = jugador.salud > jugador.vida_maxima ? jugador.vida_maxima : jugador.salud;
}

/* Genera una nueva moneda en una posición válida del mapa */
void generar_moneda(void)
{
    if (jugador.monedas_recogidas >= MAX_MONEDAS)
        return;
    do
    {
        moneda_actual.x = rand() % TAM_MAPA;
        moneda_actual.y = rand() % TAM_MAPA;
    } while (!forma_isla[moneda_actual.y][moneda_actual.x] || mapa[moneda_actual.y][moneda_actual.x] != 2);
    moneda_actual.activa = 1;
}

/* Calcula la distancia Manhattan entre dos puntos */
int distancia_manhattan(int x1, int x2, int y1, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2);
}

/* Mueve un animal hacia el jugador siguiendo la ruta más directa */
void mover_animal_hacia_jugador(Animal *animal)
{
    if (!animal->activo || SDL_GetTicks() - animal->ultimo_movimiento < RETARDO_MOV_ANIMAL)
        return;

    int dx = animal->x < jugador.x ? 1 : (animal->x > jugador.x ? -1 : 0);
    int dy = animal->y < jugador.y ? 1 : (animal->y > jugador.y ? -1 : 0);

    if (abs(jugador.x - animal->x) > abs(jugador.y - animal->y))
    {
        if (dx && forma_isla[animal->y][animal->x + dx])
            animal->x += dx;
        else if (dy && forma_isla[animal->y + dy][animal->x])
            animal->y += dy;
    }
    else
    {
        if (dy && forma_isla[animal->y + dy][animal->x])
            animal->y += dy;
        else if (dx && forma_isla[animal->y][animal->x + dx])
            animal->x += dx;
    }
    animal->ultimo_movimiento = SDL_GetTicks();
}

/* Gestiona el combate entre el jugador y los animales */
void manejar_combate(EstadoJuego *estado)
{
    if (!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F])
        return;

    static Uint32 ultimo_ataque = 0;
    Uint32 tiempo_actual = SDL_GetTicks();
    if (tiempo_actual - ultimo_ataque < 300)
        return;

    ultimo_ataque = tiempo_actual;
    int golpe = 0;

    // Buscar animales en el mapa cerca del jugador
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int new_x = jugador.x + dx;
            int new_y = jugador.y + dy;

            // Verificar límites del mapa
            if (new_x < 0 || new_x >= TAM_MAPA || new_y < 0 || new_y >= TAM_MAPA)
                continue;

            // Verificar si hay un animal estático en esta posición
            if (mapa[new_y][new_x] == 5)
            {
                golpe = 1;

                // Buscar un slot libre para activar el animal
                for (int i = 0; i < MAX_ANIMALES; i++)
                {
                    if (!animales[i].activo)
                    {
                        // Activar el animal con vida completa y luego aplicar el daño
                        animales[i].x = new_x;
                        animales[i].y = new_y;
                        animales[i].vida = VIDA_ANIMAL; // Vida completa inicial
                        animales[i].activo = 1;
                        animales[i].danio_causado = 0;
                        animales[i].ultimo_ataque = SDL_GetTicks();
                        animales[i].ultimo_movimiento = SDL_GetTicks();

                        // Aplicar el daño después de activarlo
                        animales[i].vida -= jugador.ataque_actual;
                        mapa[new_y][new_x] = 2; // Limpiar la casilla del mapa

                        if (animales[i].vida <= 0)
                        {
                            mapa[new_y][new_x] = 6; // Convertir en carne
                            animales[i].activo = 0;
                            experiencia += 20;
                        }
                        break;
                    }
                }
            }
        }
    }

    // Verificar animales activos (los que persiguen)
    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (!animales[i].activo)
            continue;

        if (distancia_manhattan(jugador.x, animales[i].x, jugador.y, animales[i].y) <= 1)
        {
            golpe = 1;
            animales[i].vida -= jugador.ataque_actual;

            if (animales[i].vida <= 0)
            {
                mapa[animales[i].y][animales[i].x] = 6; // Convertir en carne
                animales[i].activo = 0;
                experiencia += 20;
            }
        }
    }

    if (golpe && estado->sonido_lucha)
        Mix_PlayChannel(-1, estado->sonido_lucha, 0);
}

/* Maneja los ataques de los animales al jugador */
void manejar_ataques_animales(void)
{
    Uint32 tiempo_actual = SDL_GetTicks();
    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (!animales[i].activo)
            continue;

        // Asegurarnos de que el animal persiga al jugador
        if (animales[i].vida > 0)
        {
            mover_animal_hacia_jugador(&animales[i]);

            // Verificar si el animal está lo suficientemente cerca para atacar
            if (tiempo_actual - animales[i].ultimo_ataque > 1000 &&
                distancia_manhattan(jugador.x, animales[i].x, jugador.y, animales[i].y) <= 1)
            {
                jugador.salud -= ATAQUE_ANIMAL;
                animales[i].ultimo_ataque = tiempo_actual;
                jugador.golpes_recibidos++;
            }
        }
    }
}

/* Actualiza el estado del jugador (salud, hambre, sed) */
void actualizar_estado_jugador(void)
{
    Uint32 tiempo_actual = SDL_GetTicks();
    if (tiempo_actual - ultima_actualizacion_vida < 1000)
        return;
    ultima_actualizacion_vida = tiempo_actual;

    int nivel_critico = jugador.tiene_armadura ? NIVEL_CRITICO_ARMADURA : NIVEL_CRITICO_BASE;
    int nivel_recuperacion = jugador.tiene_armadura ? NIVEL_RECUPERACION_ARMADURA : NIVEL_RECUPERACION_BASE;

    if (jugador.hambre > nivel_recuperacion && jugador.sed > nivel_recuperacion && jugador.salud < jugador.vida_maxima)
    {
        jugador.salud++;
    }
    if ((jugador.hambre <= nivel_critico || jugador.sed <= nivel_critico) && jugador.salud > 0)
    {
        jugador.salud--;
    }
}

/* Renderiza texto en la pantalla */
void renderizar_texto(EstadoJuego *estado, const char *texto, int x, int y)
{
    SDL_Surface *superficie = TTF_RenderText_Solid(estado->fuente, texto, color_blanco);
    if (superficie)
    {
        SDL_Texture *textura = SDL_CreateTextureFromSurface(estado->renderizador, superficie);
        if (textura)
        {
            SDL_Rect destRect = {x, y, superficie->w, superficie->h};
            SDL_RenderCopy(estado->renderizador, textura, NULL, &destRect);
            SDL_DestroyTexture(textura);
        }
        SDL_FreeSurface(superficie);
    }
}

/* Renderiza una casilla en la pantalla */
void renderizar_casilla(SDL_Renderer *renderizador, int x, int y, SDL_Color color)
{
    SDL_Rect casilla = {x * TAM_CASILLA, y * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
    SDL_SetRenderDrawColor(renderizador, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderizador, &casilla);
}

/* Renderiza la pantalla de victoria */
void renderizar_victoria(EstadoJuego *estado)
{
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 128);
    SDL_RenderFillRect(estado->renderizador, &(SDL_Rect){0, 0, ANCHO_VENTANA, ALTO_VENTANA});
    renderizar_texto(estado, "¡GANASTE!", ANCHO_VENTANA / 2 - 100, ALTO_VENTANA / 2 - 30);
    renderizar_texto(estado, "¡Felicitaciones! Has recolectado todas las monedas", ANCHO_VENTANA / 2 - 250, ALTO_VENTANA / 2 + 10);
    renderizar_texto(estado, "Presiona R para reiniciar o Q para salir", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 50);
}

/* Renderiza la pantalla de game over */
void renderizar_game_over(EstadoJuego *estado)
{
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 128);
    SDL_RenderFillRect(estado->renderizador, &(SDL_Rect){0, 0, ANCHO_VENTANA, ALTO_VENTANA});
    renderizar_texto(estado, "¡GAME OVER!", ANCHO_VENTANA / 2 - 100, ALTO_VENTANA / 2 - 30);
    renderizar_texto(estado, "Presiona R para reiniciar o Q para salir", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 10);
}

/* Renderiza el juego completo */
void renderizar_juego(EstadoJuego *estado)
{
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 255);
    SDL_RenderClear(estado->renderizador);

    // Renderizar mapa base
    for (int y = 0; y < TAM_MAPA; y++)
    {
        for (int x = 0; x < TAM_MAPA; x++)
        {
            SDL_Rect destRect = {x * TAM_CASILLA, y * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
            renderizar_casilla(estado->renderizador, x, y, forma_isla[y][x] == 0 ? color_agua : mapa[y][x] == 1 ? color_arena
                                                                                                                : color_pasto);

            switch (mapa[y][x])
            {
            case 3:
                SDL_RenderCopy(estado->renderizador, estado->textura_roca, NULL, &destRect);
                break;
            case 4:
                SDL_RenderCopy(estado->renderizador, estado->textura_arbol, NULL, &destRect);
                break;
            case 5:
                SDL_RenderCopy(estado->renderizador, estado->textura_amenaza, NULL, &destRect);
                break;
            case 6:
                SDL_RenderCopy(estado->renderizador, estado->textura_carne, NULL, &destRect);
                break;
            }
        }
    }

    // Renderizar moneda y animales
    if (moneda_actual.activa)
    {
        SDL_RenderCopy(estado->renderizador, estado->textura_oro, NULL,
                       &(SDL_Rect){moneda_actual.x * TAM_CASILLA, moneda_actual.y * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA});
    }

    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (animales[i].activo)
        {
            SDL_SetRenderDrawColor(estado->renderizador, 255, 0, 0, 255);
            SDL_RenderFillRect(estado->renderizador, &(SDL_Rect){
                                                         animales[i].x * TAM_CASILLA,
                                                         animales[i].y * TAM_CASILLA - 10,
                                                         (animales[i].vida * TAM_CASILLA) / VIDA_ANIMAL,
                                                         5});
            SDL_RenderCopy(estado->renderizador, estado->textura_amenaza, NULL,
                           &(SDL_Rect){animales[i].x * TAM_CASILLA, animales[i].y * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA});
        }
    }

    // Renderizar jugador
    SDL_RenderCopy(estado->renderizador,
                   jugador.tiene_espada && jugador.tiene_armadura ? estado->textura_personaje_completo : jugador.tiene_espada ? estado->textura_personaje_roca
                                                                                                                              : estado->textura_personaje,
                   NULL, &(SDL_Rect){jugador.x * TAM_CASILLA, jugador.y * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA});

    // Renderizar UI
    char texto_stats[32];
    int ancho_barra = 150, alto_barra = 20, inicio_x = ANCHO_VENTANA - 200, inicio_y = 30;
    sprintf(texto_stats, "SALUD: %d/%d", jugador.salud, jugador.vida_maxima);
    renderizar_texto(estado, texto_stats, inicio_x, inicio_y - 20);
    SDL_SetRenderDrawColor(estado->renderizador, 255, 0, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &(SDL_Rect){inicio_x, inicio_y, jugador.salud * ancho_barra / jugador.vida_maxima, alto_barra});

    sprintf(texto_stats, "HAMBRE: %d/%d", jugador.hambre, jugador.vida_maxima);
    renderizar_texto(estado, texto_stats, inicio_x, inicio_y + 30);
    SDL_SetRenderDrawColor(estado->renderizador, 255, 255, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &(SDL_Rect){inicio_x, inicio_y + 50, jugador.hambre * ancho_barra / jugador.vida_maxima, alto_barra});

    sprintf(texto_stats, "SED: %d/%d", jugador.sed, jugador.vida_maxima);
    renderizar_texto(estado, texto_stats, inicio_x, inicio_y + 80);
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 255, 255);
    SDL_RenderFillRect(estado->renderizador, &(SDL_Rect){inicio_x, inicio_y + 100, jugador.sed * ancho_barra / jugador.vida_maxima, alto_barra});

    sprintf(texto_stats, "MONEDAS: %d/%d", jugador.monedas_recogidas, MAX_MONEDAS);
    renderizar_texto(estado, texto_stats, inicio_x, inicio_y + 130);

    static bool sonido_game_over_reproducido = false;
    if (jugador.salud <= 0)
    {
        Mix_HaltMusic();
        if (!sonido_game_over_reproducido && estado->sonido_gameover)
        {
            Mix_PlayChannel(-1, estado->sonido_gameover, 0);
            sonido_game_over_reproducido = true;
        }
        renderizar_game_over(estado);
    }
    else if (jugador.monedas_recogidas >= MAX_MONEDAS)
    {
        renderizar_victoria(estado);
    }
    else
    {
        sonido_game_over_reproducido = false;
    }

    SDL_RenderPresent(estado->renderizador);
}

/* Maneja la entrada del usuario */
void manejar_entrada(SDL_Event *evento, int *ejecutando, int *reiniciar, EstadoJuego *estado)
{
    static Uint32 ultima_tecla = 0;
    const Uint32 retardo_tecla = 200;

    while (SDL_PollEvent(evento))
    {
        if (evento->type == SDL_QUIT)
        {
            *ejecutando = 0;
            return;
        }
        if (evento->type != SDL_KEYDOWN)
            continue;

        if (jugador.salud <= 0 || jugador.monedas_recogidas >= MAX_MONEDAS)
        {
            if (evento->key.keysym.sym == SDLK_r)
                *reiniciar = 1;
            else if (evento->key.keysym.sym == SDLK_q)
                *ejecutando = 0;
            return;
        }

        Uint32 tiempo_actual = SDL_GetTicks();
        if (tiempo_actual - ultima_tecla < retardo_tecla)
            continue;
        ultima_tecla = tiempo_actual;

        int x_anterior = jugador.x, y_anterior = jugador.y;

        switch (evento->key.keysym.sym)
        {
        case SDLK_w:
            if (jugador.y > 0 && forma_isla[jugador.y - 1][jugador.x] && mapa[jugador.y - 1][jugador.x] != 4)
                jugador.y--;
            break;
        case SDLK_s:
            if (jugador.y < TAM_MAPA - 1 && forma_isla[jugador.y + 1][jugador.x] && mapa[jugador.y + 1][jugador.x] != 4)
                jugador.y++;
            break;
        case SDLK_a:
            if (jugador.x > 0 && forma_isla[jugador.y][jugador.x - 1] && mapa[jugador.y][jugador.x - 1] != 4)
                jugador.x--;
            break;
        case SDLK_d:
            if (jugador.x < TAM_MAPA - 1 && forma_isla[jugador.y][jugador.x + 1] && mapa[jugador.y][jugador.x + 1] != 4)
                jugador.x++;
            break;
        case SDLK_q:
            *ejecutando = 0;
            break;
        case SDLK_e:
            if (mapa[jugador.y][jugador.x] <= 1)
            {
                jugador.sed = (jugador.sed + 5 > VIDA_BASE) ? VIDA_BASE : jugador.sed + 5;
            }
            else
            {
                switch (mapa[jugador.y][jugador.x])
                {
                case 3:
                    if (!jugador.tiene_espada)
                    {
                        jugador.tiene_espada = 1;
                        actualizar_stats_jugador();
                    }
                    else if (!jugador.tiene_armadura)
                    {
                        jugador.tiene_armadura = 1;
                        actualizar_stats_jugador();
                    }
                    mapa[jugador.y][jugador.x] = 2;
                    break;
                case 4:
                    jugador.inventario[0]++;
                    experiencia += 10;
                    mapa[jugador.y][jugador.x] = 2;
                    break;
                case 6:
                    jugador.hambre = (jugador.hambre + 5 > VIDA_BASE) ? VIDA_BASE : jugador.hambre + 5;
                    mapa[jugador.y][jugador.x] = 2;
                    break;
                }
            }
            break;
        }
    }

    // Activar animales cercanos
    for (int y = jugador.y - 5; y <= jugador.y + 5; y++)
    {
        for (int x = jugador.x - 5; x <= jugador.x + 5; x++)
        {
            if (y >= 0 && y < TAM_MAPA && x >= 0 && x < TAM_MAPA && mapa[y][x] == 5)
            {
                for (int i = 0; i < MAX_ANIMALES; i++)
                {
                    if (!animales[i].activo)
                    {
                        animales[i] = (Animal){x, y, VIDA_ANIMAL, 1, 0, SDL_GetTicks(), SDL_GetTicks()};
                        mapa[y][x] = 2;
                        break;
                    }
                }
            }
        }
    }

    // Reactivar animales cercanos
    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (animales[i].x > 0 && animales[i].y > 0 && !animales[i].activo &&
            distancia_manhattan(jugador.x, animales[i].x, jugador.y, animales[i].y) <= 5)
        {
            animales[i].activo = 1;
            animales[i].vida = VIDA_ANIMAL;
            mapa[animales[i].y][animales[i].x] = 2;
        }
    }

    // Verificar recolección de monedas
    if (moneda_actual.activa && jugador.x == moneda_actual.x && jugador.y == moneda_actual.y)
    {
        jugador.monedas_recogidas++;
        moneda_actual.activa = 0;
        if (estado->sonido_moneda)
            Mix_PlayChannel(-1, estado->sonido_moneda, 0);
        if (jugador.monedas_recogidas < MAX_MONEDAS)
            generar_moneda();
    }
}

static int contar_casillas_validas(void)
{
    int count = 0;
    for (int i = 0; i < TAM_MAPA; i++)
    {
        for (int j = 0; j < TAM_MAPA; j++)
        {
            if (forma_isla[i][j] && mapa[i][j] == 2)
            {
                count++;
            }
        }
    }
    return count;
}

static void colocar_elemento(int tipo, int cantidad)
{
    while (cantidad > 0)
    {
        int y = rand() % TAM_MAPA;
        int x = rand() % TAM_MAPA;

        // Verificar si la posición es válida (es pasto y está dentro de la isla)
        if (forma_isla[y][x] && mapa[y][x] == 2)
        {
            mapa[y][x] = tipo;
            cantidad--;
        }
    }
}

/* Inicializa el mapa del juego */
void inicializar_mapa(void)
{
    // Primero inicializamos todo el mapa base
    for (int i = 0; i < TAM_MAPA; i++)
    {
        for (int j = 0; j < TAM_MAPA; j++)
        {
            if (!forma_isla[i][j])
            {
                mapa[i][j] = 0; // Agua
                continue;
            }
            // Los bordes siempre son arena
            int es_borde = (i > 0 && !forma_isla[i - 1][j]) ||
                           (i < TAM_MAPA - 1 && !forma_isla[i + 1][j]) ||
                           (j > 0 && !forma_isla[i][j - 1]) ||
                           (j < TAM_MAPA - 1 && !forma_isla[i][j + 1]);

            mapa[i][j] = es_borde ? 1 : 2; // 1 = arena, 2 = pasto
        }
    }

    // Colocar elementos específicos
    colocar_elemento(3, 10); // 10 rocas
    colocar_elemento(5, 30); // 30 animales
    colocar_elemento(4, 40); // 40 árboles

    // Activar animales iniciales
    for (int i = 0; i < TAM_MAPA; i++)
    {
        for (int j = 0; j < TAM_MAPA; j++)
        {
            if (mapa[i][j] == 5)
            {
                for (int k = 0; k < MAX_ANIMALES; k++)
                {
                    if (!animales[k].activo)
                    {
                        animales[k] = (Animal){j, i, VIDA_ANIMAL, 1, 0, SDL_GetTicks(), SDL_GetTicks()};
                        mapa[i][j] = 2;
                        break;
                    }
                }
            }
        }
    }
}

/* Reinicia el estado del jugador */
void reiniciar_jugador(Jugador *j)
{
    j->x = j->y = TAM_MAPA / 2;
    j->vida_maxima = VIDA_BASE;
    j->ataque_actual = ATAQUE_BASE;
    j->salud = j->hambre = j->sed = VIDA_BASE;
    j->energia = 100;
    j->tiene_espada = j->tiene_armadura = j->golpes_recibidos = j->monedas_recogidas = 0;
    experiencia = 0;
    memset(j->inventario, 0, sizeof(j->inventario));
    memset(animales, 0, sizeof(Animal) * MAX_ANIMALES);
    moneda_actual = (Moneda){0};
    generar_moneda();
}

/* Inicializa el juego */
int inicializar_juego(EstadoJuego *estado)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ||
        !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ||
        TTF_Init() == -1 ||
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        return 0;

    estado->ventana = SDL_CreateWindow("Isla Survival", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       ANCHO_VENTANA, ALTO_VENTANA, SDL_WINDOW_SHOWN);
    if (!estado->ventana)
        return 0;

    estado->renderizador = SDL_CreateRenderer(estado->ventana, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!estado->renderizador)
        return 0;

    estado->fuente = TTF_OpenFont("8514oem.ttf", 24);
    if (!estado->fuente)
        return 0;

    // Cargar texturas
    estado->textura_arbol = IMG_LoadTexture(estado->renderizador, "arbol.png");
    estado->textura_roca = IMG_LoadTexture(estado->renderizador, "roca.png");
    estado->textura_amenaza = IMG_LoadTexture(estado->renderizador, "animalmalo.png");
    estado->textura_personaje = IMG_LoadTexture(estado->renderizador, "personaje.png");
    estado->textura_personaje_roca = IMG_LoadTexture(estado->renderizador, "personajeroca.png");
    estado->textura_personaje_completo = IMG_LoadTexture(estado->renderizador, "personajecompleto.png");
    estado->textura_carne = IMG_LoadTexture(estado->renderizador, "carne.png");
    estado->textura_oro = IMG_LoadTexture(estado->renderizador, "oro.png");

    // Cargar música y efectos
    estado->musica = Mix_LoadMUS("musica.mp3");
    estado->sonido_lucha = Mix_LoadWAV("lucha.mp3");
    estado->sonido_gameover = Mix_LoadWAV("gameover.mp3");
    estado->sonido_moneda = Mix_LoadWAV("moneda.mp3");

    if (estado->musica)
        Mix_PlayMusic(estado->musica, -1);

    return estado->textura_arbol && estado->textura_roca && estado->textura_amenaza &&
           estado->textura_personaje && estado->textura_personaje_roca &&
           estado->textura_personaje_completo && estado->textura_carne && estado->textura_oro &&
           estado->sonido_lucha && estado->sonido_gameover && estado->sonido_moneda;
}

/* Limpia los recursos del juego */
void limpiar_juego(EstadoJuego *estado)
{
    SDL_DestroyTexture(estado->textura_arbol);
    SDL_DestroyTexture(estado->textura_roca);
    SDL_DestroyTexture(estado->textura_amenaza);
    SDL_DestroyTexture(estado->textura_personaje);
    SDL_DestroyTexture(estado->textura_personaje_roca);
    SDL_DestroyTexture(estado->textura_personaje_completo);
    SDL_DestroyTexture(estado->textura_carne);
    SDL_DestroyTexture(estado->textura_oro);
    Mix_FreeMusic(estado->musica);
    Mix_FreeChunk(estado->sonido_lucha);
    Mix_FreeChunk(estado->sonido_gameover);
    Mix_FreeChunk(estado->sonido_moneda);
    TTF_CloseFont(estado->fuente);
    SDL_DestroyRenderer(estado->renderizador);
    SDL_DestroyWindow(estado->ventana);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    EstadoJuego estado;
    SDL_Event evento;
    int ejecutando = 1;

    if (!inicializar_juego(&estado))
        return 1;

    while (ejecutando)
    {
        int reiniciar = 0;
        inicializar_mapa();
        reiniciar_jugador(&jugador);

        while (ejecutando && !reiniciar)
        {
            manejar_entrada(&evento, &ejecutando, &reiniciar, &estado);
            actualizar_estado_jugador();
            manejar_combate(&estado);
            manejar_ataques_animales();
            renderizar_juego(&estado);
            SDL_Delay(16);

            if (jugador.salud > 0 && rand() % (100 * VEL_JUEGO) == 0)
            {
                if (jugador.sed > 0)
                    jugador.sed--;
                if (jugador.hambre > 0)
                    jugador.hambre--;
            }
        }
    }

    limpiar_juego(&estado);
    return 0;
}