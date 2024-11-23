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
#define ANCHO_VENTANA 1200
#define ALTO_VENTANA 750
#define TAM_CASILLA 28
#define TAM_MAPA 27
#define VEL_JUEGO 2
#define MAX_ANIMALES 5
#define MAX_RECURSOS 15
#define RETARDO_MOV_ANIMAL 500
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
// Constantes de tiempo
#define DURACION_DIA 60000 // 1 minuto en milisegundos
#define CICLO_CLIMA 30000  // 30 segundos

// Tipos de clima
#define CLIMA_SOLEADO 0
#define CLIMA_LLUVIOSO 1
#define CLIMA_TORMENTA 2

// Constantes de jugador y combate
#define VIDA_BASE 100
#define VIDA_ARMADURA 150
#define ATAQUE_BASE 10
#define ATAQUE_ESPADA 20
#define ATAQUE_COMPLETO 30
#define VIDA_ANIMAL 50
#define ATAQUE_ANIMAL 8
#define NIVEL_CRITICO_BASE 20
#define NIVEL_CRITICO_ARMADURA 30
#define NIVEL_RECUPERACION_BASE 80
#define NIVEL_RECUPERACION_ARMADURA 90
#define RADIO_VISION_ANTORCHA 3
#define RADIO_DETECCION_ANIMAL 5
#define DURACION_ANTORCHA (DURACION_DIA / 4) // La antorcha dura la mitad de la noche
#define COSTO_ANTORCHA 3
#define COSTO_ESPADA_MADERA 5
#define COSTO_ESPADA_PIEDRA 3
#define COSTO_ARMADURA_MADERA 8
#define COSTO_ARMADURA_PIEDRA 5
#define DAÑO_ESPADA 15
#define VELOCIDAD_ANIMAL 1
// Tipos de construcciones
#define TIPO_REFUGIO 1
#define TIPO_FOGATA 2
#define TIPO_CULTIVO 3

typedef struct
{
    int x, y;
    int tipo;
    int durabilidad;
    bool activa;
    Uint32 tiempo_creacion;
} Construccion;

// Modificar la estructura del jugador
typedef struct
{
    int x, y, salud, hambre, sed, energia, temperatura;
    int inventario[10];
    int tiene_espada, tiene_armadura, tiene_refugio, tiene_antorcha;
    int vida_maxima, ataque_actual;
    int nivel_crafteo;
    Construccion construcciones[5];
    Uint32 tiempo_inicio_antorcha;
} Jugador;

typedef struct
{
    int x, y, vida, activo, danio_causado;
    int tipo; // 0:lobo, 1:oso, 2:jabalí
    Uint32 ultimo_ataque, ultimo_movimiento;
} Animal;

typedef struct
{
    int x, y, tipo, activo; // tipos: 0:madera, 1:piedra, 2:fruta, 3:semillas
} Recurso;

typedef struct
{
    SDL_Window *ventana;
    SDL_Renderer *renderizador;
    SDL_Texture *textura_arbol, *textura_roca, *textura_animal_1, *textura_animal_2,
        *textura_personaje, *textura_personaje_equipado, *textura_personaje_espada,
        *textura_personaje_antorcha, *textura_refugio, *textura_fogata, *textura_cultivo,
        *textura_fruta, *textura_semilla;
    TTF_Font *fuente;
    Mix_Music *musica_dia, *musica_noche;
    Mix_Chunk *sonido_lucha, *sonido_construccion, *sonido_clima;
    int clima_actual;
    bool es_de_dia;
    Uint32 tiempo_inicio_dia;
} EstadoJuego;

// Variables globales
Jugador jugador;
Animal animales[MAX_ANIMALES];
Recurso recursos[MAX_RECURSOS];
int mapa[TAM_MAPA][TAM_MAPA];
int temperatura_ambiente = 20;
SDL_Color color_agua = {64, 224, 208, 255};
SDL_Color color_arena = {238, 214, 175, 255};
SDL_Color color_pasto = {34, 139, 34, 255};
SDL_Color color_blanco = {255, 255, 255, 255};

// Matriz de la isla (la misma que en tu código original)
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

// Prototipos de funciones
void actualizar_clima(EstadoJuego *estado);
void actualizar_ciclo_dia_noche(EstadoJuego *estado);
void actualizar_temperatura_jugador(void);
void manejar_construccion(int tipo_construccion);
void generar_recursos(void);
void generar_amenazas(void);
void renderizar_juego(EstadoJuego *estado);
void actualizar_construcciones(void);
void actualizar_stats_jugador(EstadoJuego *estado);
void renderizar_interfaz(EstadoJuego *estado);
void manejar_entrada(SDL_Event *evento, int *ejecutando, EstadoJuego *estado);
int inicializar_juego(EstadoJuego *estado);
void limpiar_juego(EstadoJuego *estado);
void renderizar_texto(EstadoJuego *estado, const char *texto, int x, int y, SDL_Color color);
void manejar_antorcha(EstadoJuego *estado);
void manejar_combate(Animal *animal, EstadoJuego *estado);
void actualizar_animales(EstadoJuego *estado);
void crear_espada(void);
void crear_armadura(void);
// Implementación de las funciones principales

void actualizar_clima(EstadoJuego *estado)
{
    static int clima_anterior = -1;
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - estado->tiempo_inicio_dia > CICLO_CLIMA)
    {
        int nuevo_clima = rand() % 3;

        // Evitar cambios bruscos
        if (estado->clima_actual == CLIMA_LLUVIOSO && nuevo_clima == CLIMA_TORMENTA)
        {
            nuevo_clima = CLIMA_LLUVIOSO;
        }

        if (estado->clima_actual == CLIMA_TORMENTA &&
            (nuevo_clima == CLIMA_SOLEADO || nuevo_clima == CLIMA_LLUVIOSO))
        {
            Mix_HaltChannel(-1);
        }

        estado->clima_actual = nuevo_clima;
        estado->tiempo_inicio_dia = tiempo_actual;

        switch (estado->clima_actual)
        {
        case CLIMA_SOLEADO:
            temperatura_ambiente = 25 + (rand() % 5);
            break;
        case CLIMA_LLUVIOSO:
            temperatura_ambiente = 15 + (rand() % 5);
            break;
        case CLIMA_TORMENTA:
            temperatura_ambiente = 10 + (rand() % 5);
            if (estado->sonido_clima)
            {
                Mix_HaltChannel(-1);
                if (Mix_PlayChannel(-1, estado->sonido_clima, 0) == -1)
                {
                    printf("Error al reproducir sonido de clima: %s\n", Mix_GetError());
                }
            }
            break;
        }

        clima_anterior = estado->clima_actual;
    }
}

void actualizar_ciclo_dia_noche(EstadoJuego *estado)
{
    Uint32 tiempo_actual = SDL_GetTicks();
    bool era_de_dia = estado->es_de_dia;

    estado->es_de_dia = ((tiempo_actual / DURACION_DIA) % 2) == 0;

    if (era_de_dia != estado->es_de_dia)
    {
        if (estado->es_de_dia)
        {
            if (estado->musica_dia)
                Mix_PlayMusic(estado->musica_dia, -1);
            temperatura_ambiente += 5;
        }
        else
        {
            if (estado->musica_noche)
                Mix_PlayMusic(estado->musica_noche, -1);
            temperatura_ambiente -= 5;
        }
    }
}

void actualizar_construcciones(void)
{
    Uint32 tiempo_actual = SDL_GetTicks();
    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa)
        {
            // Reducir durabilidad con el tiempo
            if (tiempo_actual - jugador.construcciones[i].tiempo_creacion > 10000)
            { // cada 10 segundos
                jugador.construcciones[i].durabilidad--;
                jugador.construcciones[i].tiempo_creacion = tiempo_actual;
            }

            // Manejar cultivos
            if (jugador.construcciones[i].tipo == TIPO_CULTIVO)
            {
                if (tiempo_actual - jugador.construcciones[i].tiempo_creacion > 30000)
                {                                             // 30 segundos para crecer
                    jugador.inventario[3]++;                  // Añadir fruta
                    jugador.construcciones[i].activa = false; // El cultivo se consume
                }
            }

            // Destruir construcciones sin durabilidad
            if (jugador.construcciones[i].durabilidad <= 0)
            {
                jugador.construcciones[i].activa = false;
                if (jugador.construcciones[i].tipo == TIPO_REFUGIO)
                {
                    jugador.tiene_refugio = 0;
                }
            }
        }
    }
}
void generar_amenazas(void)
{
    static Uint32 ultimo_tiempo_amenaza = 0;
    Uint32 tiempo_actual = SDL_GetTicks();

    // Generar amenaza cada 30 segundos
    if (tiempo_actual - ultimo_tiempo_amenaza >= 30000)
    {
        for (int i = 0; i < MAX_ANIMALES; i++)
        {
            if (!animales[i].activo)
            {
                // Generar en los bordes del mapa
                int lado = rand() % 4;
                switch (lado)
                {
                case 0: // Arriba
                    animales[i].x = rand() % TAM_MAPA;
                    animales[i].y = 1;
                    break;
                case 1: // Derecha
                    animales[i].x = TAM_MAPA - 2;
                    animales[i].y = rand() % TAM_MAPA;
                    break;
                case 2: // Abajo
                    animales[i].x = rand() % TAM_MAPA;
                    animales[i].y = TAM_MAPA - 2;
                    break;
                case 3: // Izquierda
                    animales[i].x = 1;
                    animales[i].y = rand() % TAM_MAPA;
                    break;
                }

                if (forma_isla[animales[i].y][animales[i].x])
                {
                    animales[i].activo = 1;
                    animales[i].vida = VIDA_ANIMAL;
                    animales[i].tipo = rand() % 2;
                    animales[i].ultimo_ataque = tiempo_actual;
                    animales[i].ultimo_movimiento = tiempo_actual;
                }
            }
        }
        ultimo_tiempo_amenaza = tiempo_actual;
    }
}
void renderizar_texto(EstadoJuego *estado, const char *texto, int x, int y, SDL_Color color)
{
    SDL_Surface *superficie = TTF_RenderText_Solid(estado->fuente, texto, color);
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

void renderizar_interfaz(EstadoJuego *estado)
{
    char texto[100];
    int y_offset = 10;
    int x_controles = 10;              // Panel izquierdo para controles
    int x_stats = ANCHO_VENTANA - 200; // Panel derecho para estadísticas

    // Colores para la interfaz
    SDL_Color color_titulo = {255, 215, 0, 255};   // Dorado
    SDL_Color color_stats = {0, 255, 0, 255};      // Verde brillante
    SDL_Color color_bajo = {255, 50, 50, 255};     // Rojo suave
    SDL_Color color_normal = {200, 200, 200, 255}; // Gris claro
    SDL_Color color_alto = {100, 255, 255, 255};   // Cyan brillante
    SDL_Color color_especial = {255, 128, 0, 255}; // Naranja

    // Panel izquierdo semi-transparente (Controles)
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 40, 200);
    SDL_Rect panelIzquierdo = {0, 0, 250, ALTO_VENTANA};
    SDL_RenderFillRect(estado->renderizador, &panelIzquierdo);

    // Panel derecho semi-transparente (Estadísticas)
    SDL_Rect panelDerecho = {ANCHO_VENTANA - 200, 0, 200, ALTO_VENTANA};
    SDL_RenderFillRect(estado->renderizador, &panelDerecho);

    // === CONTROLES (PANEL IZQUIERDO) ===
    renderizar_texto(estado, "=== CONTROLES ===", x_controles, y_offset, color_titulo);
    y_offset += 40;

    renderizar_texto(estado, "Movimiento:", x_controles, y_offset, color_especial);
    renderizar_texto(estado, "W - Arriba", x_controles + 10, y_offset += 25, color_normal);
    renderizar_texto(estado, "S - Abajo", x_controles + 10, y_offset += 25, color_normal);
    renderizar_texto(estado, "A - Izquierda", x_controles + 10, y_offset += 25, color_normal);
    renderizar_texto(estado, "D - Derecha", x_controles + 10, y_offset += 25, color_normal);

    y_offset += 20;
    renderizar_texto(estado, "Acciones:", x_controles, y_offset, color_especial);
    renderizar_texto(estado, "E - Recolectar", x_controles + 10, y_offset += 25, color_normal);
    renderizar_texto(estado, "ESPACIO - Comer", x_controles + 10, y_offset += 25, color_normal);

    y_offset += 20;
    renderizar_texto(estado, "=== CONSTRUCCIONES ===", x_controles, y_offset, color_titulo);
    y_offset += 30;

    renderizar_texto(estado, "Refugio (R):", x_controles, y_offset, color_especial);
    renderizar_texto(estado, "- 10 madera", x_controles + 10, y_offset += 20, color_normal);
    renderizar_texto(estado, "- 5 piedra", x_controles + 10, y_offset += 20, color_normal);

    y_offset += 10;
    renderizar_texto(estado, "Fogata (F):", x_controles, y_offset, color_especial);
    renderizar_texto(estado, "- 5 madera", x_controles + 10, y_offset += 20, color_normal);

    y_offset += 10;
    renderizar_texto(estado, "Cultivo (C):", x_controles, y_offset, color_especial);
    renderizar_texto(estado, "- 3 madera", x_controles + 10, y_offset += 20, color_normal);
    renderizar_texto(estado, "- 1 semilla", x_controles + 10, y_offset += 20, color_normal);

    // === CRAFTEO ===
    y_offset += 20;
    renderizar_texto(estado, "=== CRAFTEO ===", x_controles, y_offset, color_titulo);
    y_offset += 30;

    renderizar_texto(estado, "Antorcha (T):", x_controles, y_offset, color_especial);
    renderizar_texto(estado, "- 3 madera", x_controles + 10, y_offset += 20, color_normal);

    renderizar_texto(estado, "Espada (1):", x_controles, y_offset += 25, color_especial);
    renderizar_texto(estado, "- 5 madera", x_controles + 10, y_offset += 20, color_normal);
    renderizar_texto(estado, "- 3 piedra", x_controles + 10, y_offset += 20, color_normal);

    renderizar_texto(estado, "Armadura (2):", x_controles, y_offset += 25, color_especial);
    renderizar_texto(estado, "- 8 madera", x_controles + 10, y_offset += 20, color_normal);
    renderizar_texto(estado, "- 5 piedra", x_controles + 10, y_offset += 20, color_normal);
    renderizar_texto(estado, "(Requiere espada)", x_controles + 10, y_offset += 20, color_normal);

    // === ESTADÍSTICAS (PANEL DERECHO) ===
    y_offset = 10;
    renderizar_texto(estado, "=== ESTADÍSTICAS ===", x_stats, y_offset, color_titulo);
    y_offset += 30;

    // Barras de estado
    SDL_Rect barra;
    int ancho_barra = 150;
    int alto_barra = 15;

    // Barra de Salud
    sprintf(texto, "Salud: %d/%d", jugador.salud, jugador.vida_maxima);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.salud * ancho_barra) / jugador.vida_maxima, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 255, 0, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Barra de Energía
    y_offset += 45;
    sprintf(texto, "Energía: %d/100", jugador.energia < 0 ? 0 : jugador.energia);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20,
                       ((jugador.energia < 0 ? 0 : jugador.energia) * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 255, 255, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Barra de Hambre
    y_offset += 45;
    sprintf(texto, "Hambre: %d/100", jugador.hambre);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.hambre * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 139, 69, 19, 255); // Marrón
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Barra de Sed
    y_offset += 45;
    sprintf(texto, "Sed: %d/100", jugador.sed);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.sed * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 0, 191, 255, 255); // Azul claro
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Temperatura
    y_offset += 45;
    SDL_Color color_temp = jugador.temperatura < 10 ? color_bajo : jugador.temperatura > 35 ? color_alto
                                                                                            : color_normal;
    sprintf(texto, "Temperatura: %d°C %s", jugador.temperatura,
            jugador.temperatura < 10 ? "[-]" : jugador.temperatura > 35 ? "[+]"
                                                                        : "[=]");
    renderizar_texto(estado, texto, x_stats, y_offset, color_temp);

    // === INVENTARIO ===
    y_offset += 45;
    renderizar_texto(estado, "=== INVENTARIO ===", x_stats, y_offset, color_titulo);
    y_offset += 30;

    sprintf(texto, "[*] Madera: %d", jugador.inventario[0]);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);

    sprintf(texto, "[#] Piedra: %d", jugador.inventario[1]);
    renderizar_texto(estado, texto, x_stats, y_offset += 25, color_normal);

    sprintf(texto, "[M] Carne: %d", jugador.inventario[2]);
    renderizar_texto(estado, texto, x_stats, y_offset += 25, color_normal);

    sprintf(texto, "[O] Fruta: %d", jugador.inventario[3]);
    renderizar_texto(estado, texto, x_stats, y_offset += 25, color_normal);

    sprintf(texto, "[.] Semillas: %d", jugador.inventario[4]);
    renderizar_texto(estado, texto, x_stats, y_offset += 25, color_normal);

    // === AMBIENTE ===
    y_offset += 40;
    renderizar_texto(estado, "=== AMBIENTE ===", x_stats, y_offset, color_titulo);
    y_offset += 30;

    sprintf(texto, "Clima: %s %s",
            estado->clima_actual == CLIMA_SOLEADO ? "Soleado" : estado->clima_actual == CLIMA_LLUVIOSO ? "Lluvioso"
                                                                                                       : "Tormenta",
            estado->clima_actual == CLIMA_SOLEADO ? "[O]" : estado->clima_actual == CLIMA_LLUVIOSO ? "[~]"
                                                                                                   : "[!]");
    renderizar_texto(estado, texto, x_stats, y_offset, color_especial);

    sprintf(texto, "Hora: %s %s", estado->es_de_dia ? "Día" : "Noche",
            estado->es_de_dia ? "[D]" : "[N]");
    renderizar_texto(estado, texto, x_stats, y_offset += 25, color_especial);
}

void renderizar_juego(EstadoJuego *estado)
{
    // Limpiar pantalla
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 255);
    SDL_RenderClear(estado->renderizador);

    // Calcular el área del mapa (entre las interfaces)
    int ancho_interfaces = 250;
    int x_inicio_mapa = ancho_interfaces;
    int ancho_mapa = ANCHO_VENTANA - (2 * ancho_interfaces);
    float escala_mapa = (float)ancho_mapa / (TAM_MAPA * TAM_CASILLA);

    // Renderizar mapa base
    for (int y = 0; y < TAM_MAPA; y++)
    {
        for (int x = 0; x < TAM_MAPA; x++)
        {
            SDL_Rect destRect = {
                x_inicio_mapa + (int)(x * TAM_CASILLA * escala_mapa),
                (int)(y * TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa)};

            SDL_Color color_terreno = forma_isla[y][x] == 0 ? color_agua : mapa[y][x] == 1 ? color_arena
                                                                                           : color_pasto;

            SDL_SetRenderDrawColor(estado->renderizador,
                                   color_terreno.r,
                                   color_terreno.g,
                                   color_terreno.b,
                                   color_terreno.a);
            SDL_RenderFillRect(estado->renderizador, &destRect);

            if (mapa[y][x] == 3) // Roca
                SDL_RenderCopy(estado->renderizador, estado->textura_roca, NULL, &destRect);
            else if (mapa[y][x] == 4) // Árbol
                SDL_RenderCopy(estado->renderizador, estado->textura_arbol, NULL, &destRect);
        }
    }

    // Renderizar recursos
    for (int i = 0; i < MAX_RECURSOS; i++)
    {
        if (recursos[i].activo)
        {
            SDL_Rect destRect = {
                x_inicio_mapa + (int)(recursos[i].x * TAM_CASILLA * escala_mapa),
                (int)(recursos[i].y * TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa)};

            switch (recursos[i].tipo)
            {
            case 2: // Fruta
                SDL_RenderCopy(estado->renderizador, estado->textura_fruta, NULL, &destRect);
                break;
            case 3: // Semilla
                SDL_RenderCopy(estado->renderizador, estado->textura_semilla, NULL, &destRect);
                break;
            }
        }
    }

    // Renderizar construcciones
    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa)
        {
            SDL_Rect destRect = {
                x_inicio_mapa + (int)(jugador.construcciones[i].x * TAM_CASILLA * escala_mapa),
                (int)(jugador.construcciones[i].y * TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa)};

            switch (jugador.construcciones[i].tipo)
            {
            case TIPO_REFUGIO:
                SDL_RenderCopy(estado->renderizador, estado->textura_refugio, NULL, &destRect);
                break;
            case TIPO_FOGATA:
                SDL_RenderCopy(estado->renderizador, estado->textura_fogata, NULL, &destRect);
                break;
            case TIPO_CULTIVO:
                SDL_RenderCopy(estado->renderizador, estado->textura_cultivo, NULL, &destRect);
                break;
            }
        }
    }

    // Renderizar animales
    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (animales[i].activo)
        {
            SDL_Rect destRect = {
                x_inicio_mapa + (int)(animales[i].x * TAM_CASILLA * escala_mapa),
                (int)(animales[i].y * TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa)};

            SDL_Rect barraVida = {
                x_inicio_mapa + (int)(animales[i].x * TAM_CASILLA * escala_mapa),
                (int)(animales[i].y * TAM_CASILLA * escala_mapa - 5),
                (int)((animales[i].vida * TAM_CASILLA * escala_mapa) / VIDA_ANIMAL),
                3};
            SDL_SetRenderDrawColor(estado->renderizador, 255, 0, 0, 255);
            SDL_RenderFillRect(estado->renderizador, &barraVida);

            SDL_RenderCopy(estado->renderizador,
                           animales[i].tipo == 0 ? estado->textura_animal_1 : estado->textura_animal_2,
                           NULL, &destRect);
        }
    }

    // Renderizar jugador
    SDL_Rect jugadorRect = {
        x_inicio_mapa + (int)(jugador.x * TAM_CASILLA * escala_mapa),
        (int)(jugador.y * TAM_CASILLA * escala_mapa),
        (int)ceil(TAM_CASILLA * escala_mapa),
        (int)ceil(TAM_CASILLA * escala_mapa)};

    SDL_Texture *textura_actual = estado->textura_personaje;
    if (jugador.tiene_antorcha)
        textura_actual = estado->textura_personaje_antorcha;
    else if (jugador.tiene_armadura)
        textura_actual = estado->textura_personaje_equipado;
    else if (jugador.tiene_espada)
        textura_actual = estado->textura_personaje_espada;

    SDL_RenderCopy(estado->renderizador, textura_actual, NULL, &jugadorRect);

    // Efectos climáticos y de iluminación
    if (!estado->es_de_dia)
    {
        // Aplicar oscuridad de noche a todo el mapa
        SDL_SetRenderDrawBlendMode(estado->renderizador, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 20, 230);
        SDL_Rect pantalla = {x_inicio_mapa, 0, ancho_mapa, ALTO_VENTANA};
        SDL_RenderFillRect(estado->renderizador, &pantalla);

        // Si tiene antorcha, iluminar el área de visión
        if (jugador.tiene_antorcha)
        {
            for (int y = -RADIO_VISION_ANTORCHA; y <= RADIO_VISION_ANTORCHA; y++)
            {
                for (int x = -RADIO_VISION_ANTORCHA; x <= RADIO_VISION_ANTORCHA; x++)
                {
                    int tile_x = jugador.x + x;
                    int tile_y = jugador.y + y;

                    if (tile_x >= 0 && tile_x < TAM_MAPA && tile_y >= 0 && tile_y < TAM_MAPA)
                    {
                        float distancia = sqrt(x * x + y * y);
                        if (distancia <= RADIO_VISION_ANTORCHA)
                        {
                            SDL_Rect tileRect = {
                                x_inicio_mapa + (int)(tile_x * TAM_CASILLA * escala_mapa),
                                (int)(tile_y * TAM_CASILLA * escala_mapa),
                                (int)ceil(TAM_CASILLA * escala_mapa),
                                (int)ceil(TAM_CASILLA * escala_mapa)};

                            // Limpiar la oscuridad de la noche
                            SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 230);
                            SDL_RenderFillRect(estado->renderizador, &tileRect);

                            // Aplicar el efecto de día lluvioso (ligeramente oscuro)
                            SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 20, 100);
                            SDL_RenderFillRect(estado->renderizador, &tileRect);
                        }
                    }
                }
            }
        }
    }

    // Efecto de lluvia
    if (estado->clima_actual == CLIMA_LLUVIOSO || estado->clima_actual == CLIMA_TORMENTA)
    {
        // Si es de día, oscurecer ligeramente
        if (estado->es_de_dia)
        {
            SDL_SetRenderDrawBlendMode(estado->renderizador, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 20, 100);
            SDL_Rect pantalla = {x_inicio_mapa, 0, ancho_mapa, ALTO_VENTANA};
            SDL_RenderFillRect(estado->renderizador, &pantalla);
        }

        // Renderizar la lluvia
        int num_gotas = estado->clima_actual == CLIMA_TORMENTA ? 200 : 100;
        static int *gotas = NULL;

        if (!gotas)
        {
            gotas = (int *)malloc(num_gotas * 2 * sizeof(int));
            for (int i = 0; i < num_gotas; i++)
            {
                gotas[i * 2] = x_inicio_mapa + (rand() % ancho_mapa);
                gotas[i * 2 + 1] = rand() % ALTO_VENTANA;
            }
        }

        SDL_SetRenderDrawColor(estado->renderizador, 200, 200, 255,
                               estado->clima_actual == CLIMA_TORMENTA ? 255 : 180);

        for (int i = 0; i < num_gotas; i++)
        {
            int idx = i * 2;
            if (gotas[idx] >= x_inicio_mapa && gotas[idx] <= x_inicio_mapa + ancho_mapa)
            {
                SDL_RenderDrawLine(estado->renderizador,
                                   gotas[idx], gotas[idx + 1],
                                   gotas[idx] + (estado->clima_actual == CLIMA_TORMENTA ? 2 : 1),
                                   gotas[idx + 1] + (estado->clima_actual == CLIMA_TORMENTA ? 5 : 3));
            }

            gotas[idx + 1] += (estado->clima_actual == CLIMA_TORMENTA ? 15 : 10);
            gotas[idx] += (estado->clima_actual == CLIMA_TORMENTA ? 3 : 2);

            if (gotas[idx + 1] > ALTO_VENTANA)
            {
                gotas[idx + 1] = 0;
                gotas[idx] = x_inicio_mapa + (rand() % ancho_mapa);
            }
            if (gotas[idx] > x_inicio_mapa + ancho_mapa)
            {
                gotas[idx] = x_inicio_mapa;
            }
        }
    }

    // Renderizar interfaz y presentar
    renderizar_interfaz(estado);
    SDL_RenderPresent(estado->renderizador);
}
void manejar_entrada(SDL_Event *evento, int *ejecutando, EstadoJuego *estado)
{
    static Uint32 ultima_accion = 0;
    const Uint32 DELAY_ACCION = 200;

    while (SDL_PollEvent(evento))
    {
        if (evento->type == SDL_QUIT)
        {
            *ejecutando = 0;
            return;
        }

        if (evento->type == SDL_KEYDOWN)
        {
            Uint32 tiempo_actual = SDL_GetTicks();
            if (tiempo_actual - ultima_accion < DELAY_ACCION)
                continue;
            ultima_accion = tiempo_actual;

            int x_anterior = jugador.x;
            int y_anterior = jugador.y;

            switch (evento->key.keysym.sym)
            {
            case SDLK_w:
                if (jugador.y > 0 && forma_isla[jugador.y - 1][jugador.x])
                    jugador.y--;
                break;
            case SDLK_s:
                if (jugador.y < TAM_MAPA - 1 && forma_isla[jugador.y + 1][jugador.x])
                    jugador.y++;
                break;
            case SDLK_a:
                if (jugador.x > 0 && forma_isla[jugador.y][jugador.x - 1])
                    jugador.x--;
                break;
            case SDLK_d:
                if (jugador.x < TAM_MAPA - 1 && forma_isla[jugador.y][jugador.x + 1])
                    jugador.x++;
                break;
            case SDLK_e: // Recolectar
                // Recolectar recursos
                for (int i = 0; i < MAX_RECURSOS; i++)
                {
                    if (recursos[i].activo &&
                        recursos[i].x == jugador.x &&
                        recursos[i].y == jugador.y)
                    {
                        switch (recursos[i].tipo)
                        {
                        case 2:                      // Fruta
                            jugador.inventario[3]++; // Aumenta contador de fruta
                            break;
                        case 3:                      // Semilla
                            jugador.inventario[4]++; // Aumenta contador de semillas
                            break;
                        }
                        recursos[i].activo = 0;
                        jugador.energia = MAX(0, jugador.energia - 5);
                        break;
                    }
                }

                // Recolectar árbol o roca
                if (mapa[jugador.y][jugador.x] == 4)
                {
                    jugador.inventario[0] += 3; // Madera
                    mapa[jugador.y][jugador.x] = 2;
                    jugador.energia = MAX(0, jugador.energia - 10);
                }
                else if (mapa[jugador.y][jugador.x] == 3)
                {
                    jugador.inventario[1] += 2; // Piedra
                    mapa[jugador.y][jugador.x] = 2;
                    jugador.energia = MAX(0, jugador.energia - 10);
                }
                break;
            case SDLK_r:
                manejar_construccion(TIPO_REFUGIO);
                if (estado->sonido_construccion)
                    Mix_PlayChannel(-1, estado->sonido_construccion, 0);
                break;
            case SDLK_f:
                manejar_construccion(TIPO_FOGATA);
                if (estado->sonido_construccion)
                    Mix_PlayChannel(-1, estado->sonido_construccion, 0);
                break;
            case SDLK_c:
                manejar_construccion(TIPO_CULTIVO);
                if (estado->sonido_construccion)
                    Mix_PlayChannel(-1, estado->sonido_construccion, 0);
                break;
            case SDLK_t: // Crear antorcha
                if (!jugador.tiene_antorcha &&
                    jugador.inventario[0] >= COSTO_ANTORCHA &&
                    !estado->es_de_dia)
                {
                    jugador.tiene_antorcha = 1;
                    jugador.inventario[0] -= COSTO_ANTORCHA;
                    jugador.tiempo_inicio_antorcha = SDL_GetTicks();
                }
                break;
            case SDLK_1: // Crear espada
                crear_espada();
                break;
            case SDLK_2: // Crear armadura
                crear_armadura();
                break;
            case SDLK_SPACE:                   // Comer
                if (jugador.inventario[3] > 0) // Fruta
                {
                    jugador.hambre = MIN(100, jugador.hambre + 15);
                    jugador.inventario[3]--;
                }
                else if (jugador.inventario[2] > 0) // Carne
                {
                    jugador.hambre = MIN(100, jugador.hambre + 30);
                    jugador.inventario[2]--;
                }
                break;
            }

            // Actualizar energía al moverse
            if (x_anterior != jugador.x || y_anterior != jugador.y)
            {
                jugador.energia = MAX(0, jugador.energia - 2);
            }

            // Verificar combate con animales
            for (int i = 0; i < MAX_ANIMALES; i++)
            {
                if (animales[i].activo)
                {
                    int dx = abs(jugador.x - animales[i].x);
                    int dy = abs(jugador.y - animales[i].y);
                    if (dx <= 1 && dy <= 1)
                    {
                        manejar_combate(&animales[i], estado);
                    }
                }
            }
        }
    }
}

void manejar_construccion(int tipo_construccion)
{
    // Verificar recursos necesarios
    int madera_necesaria = 0, piedra_necesaria = 0;
    int costo_energia = 0;

    switch (tipo_construccion)
    {
    case TIPO_REFUGIO:
        madera_necesaria = 10;
        piedra_necesaria = 5;
        costo_energia = 20;
        break;
    case TIPO_FOGATA:
        madera_necesaria = 5;
        costo_energia = 10;
        piedra_necesaria = 0;
        break;
    case TIPO_CULTIVO:
        madera_necesaria = 3;
        costo_energia = 15;
        piedra_necesaria = 0;
        if (jugador.inventario[4] < 1)
            return;
        break;
    default:
        return;
    }

    // Verificar si tiene suficientes recursos y energía
    if (jugador.inventario[0] < madera_necesaria ||
        jugador.inventario[1] < piedra_necesaria ||
        jugador.energia < costo_energia)
    {
        printf("No hay suficientes recursos o energía\n");
        return;
    }

    // Verificar si hay espacio para una nueva construcción
    int espacio_encontrado = -1;
    for (int i = 0; i < 5; i++)
    {
        if (!jugador.construcciones[i].activa)
        {
            espacio_encontrado = i;
            break;
        }
    }

    if (espacio_encontrado == -1)
    {
        printf("No hay espacio para más construcciones\n");
        return;
    }

    // Realizar la construcción
    Construccion *nueva_construccion = &jugador.construcciones[espacio_encontrado];
    nueva_construccion->tipo = tipo_construccion;
    nueva_construccion->x = jugador.x;
    nueva_construccion->y = jugador.y;
    nueva_construccion->activa = true;
    nueva_construccion->durabilidad = 100;
    nueva_construccion->tiempo_creacion = SDL_GetTicks();

    // Consumir recursos
    jugador.inventario[0] -= madera_necesaria;
    jugador.inventario[1] -= piedra_necesaria;
    jugador.energia = MAX(0, jugador.energia - costo_energia);

    if (tipo_construccion == TIPO_CULTIVO)
    {
        jugador.inventario[4]--; // Consumir semilla
    }
    else if (tipo_construccion == TIPO_REFUGIO)
    {
        jugador.tiene_refugio = 1;
    }

    printf("Construcción realizada con éxito\n");
}

void generar_recursos(void)
{
    int recursos_generados = 0;
    while (recursos_generados < MAX_RECURSOS)
    {
        int x = rand() % TAM_MAPA;
        int y = rand() % TAM_MAPA;

        // Verificar si la posición es válida
        if (forma_isla[y][x] && mapa[y][x] == 2)
        { // Solo en tierra y pasto
            recursos[recursos_generados].x = x;
            recursos[recursos_generados].y = y;
            recursos[recursos_generados].tipo = rand() % 4; // 0:madera, 1:piedra, 2:fruta, 3:semillas
            recursos[recursos_generados].activo = 1;
            recursos_generados++;
        }
    }
}
void actualizar_stats_jugador(EstadoJuego *estado)
{
    static Uint32 ultimo_tiempo_actualizacion = 0;
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - ultimo_tiempo_actualizacion >= 1000)
    {
        // Reducir hambre y sed gradualmente
        jugador.hambre = MAX(0, jugador.hambre - 1);
        jugador.sed = MAX(0, jugador.sed - 1);

        // Efectos del clima
        if (estado->clima_actual == CLIMA_TORMENTA)
        {
            if (!jugador.tiene_refugio)
            {
                jugador.energia = MAX(0, jugador.energia - 2);
                if (rand() % 5 == 0)
                {
                    jugador.salud = MAX(0, jugador.salud - 1);
                }
            }
        }

        // Regenerar sed durante la lluvia si no está en refugio
        if ((estado->clima_actual == CLIMA_LLUVIOSO || estado->clima_actual == CLIMA_TORMENTA) && !jugador.tiene_refugio)
        {
            jugador.sed = MIN(100, jugador.sed + 2);
        }

        // Regenerar energía si está descansando en refugio
        if (jugador.tiene_refugio)
        {
            for (int i = 0; i < 5; i++)
            {
                if (jugador.construcciones[i].activa &&
                    jugador.construcciones[i].tipo == TIPO_REFUGIO &&
                    jugador.x == jugador.construcciones[i].x &&
                    jugador.y == jugador.construcciones[i].y)
                {
                    jugador.energia = MIN(100, jugador.energia + 5);
                    break;
                }
            }
        }

        // Efectos de hambre y sed
        if (jugador.hambre <= 20 || jugador.sed <= 20)
        {
            jugador.salud = MAX(0, jugador.salud - 1);
        }

        // Regeneración de salud
        if (jugador.hambre >= 50 && jugador.sed >= 50 &&
            jugador.energia >= 50 && jugador.salud < jugador.vida_maxima)
        {
            jugador.salud = MIN(jugador.vida_maxima, jugador.salud + 1);
        }

        ultimo_tiempo_actualizacion = tiempo_actual;
    }
}
void actualizar_temperatura_jugador(void)
{
    int temp_objetivo = temperatura_ambiente;

    // Modificadores
    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa &&
            jugador.construcciones[i].tipo == TIPO_FOGATA &&
            abs(jugador.x - jugador.construcciones[i].x) <= 2 &&
            abs(jugador.y - jugador.construcciones[i].y) <= 2)
        {
            temp_objetivo += 10;
            break;
        }
    }

    if (jugador.tiene_refugio &&
        mapa[jugador.y][jugador.x] == TIPO_REFUGIO)
        temp_objetivo += 5;

    if (jugador.temperatura < temp_objetivo)
        jugador.temperatura++;
    else if (jugador.temperatura > temp_objetivo)
        jugador.temperatura--;

    if (jugador.temperatura < 10)
    {
        jugador.energia = MAX(0, jugador.energia - 1);
        if (jugador.energia <= 0)
            jugador.salud = MAX(0, jugador.salud - 1);
    }
    else if (jugador.temperatura > 35)
    {
        jugador.sed = MAX(0, jugador.sed - 2);
        if (jugador.sed <= 0)
            jugador.salud = MAX(0, jugador.salud - 1);
    }
}
int inicializar_juego(EstadoJuego *estado)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        return 0;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        return 0;
    if (TTF_Init() == -1)
        return 0;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        return 0;

    estado->ventana = SDL_CreateWindow("Survival Island",
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       ANCHO_VENTANA,
                                       ALTO_VENTANA,
                                       SDL_WINDOW_SHOWN);
    if (!estado->ventana)
        return 0;

    estado->renderizador = SDL_CreateRenderer(estado->ventana, -1,
                                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!estado->renderizador)
        return 0;

    // Cargar texturas
    estado->textura_arbol = IMG_LoadTexture(estado->renderizador, "arbol.png");
    estado->textura_roca = IMG_LoadTexture(estado->renderizador, "roca.png");
    estado->textura_animal_1 = IMG_LoadTexture(estado->renderizador, "animal_1.png");
    estado->textura_animal_2 = IMG_LoadTexture(estado->renderizador, "animal_2.png");
    estado->textura_personaje = IMG_LoadTexture(estado->renderizador, "personaje.png");
    estado->textura_personaje_equipado = IMG_LoadTexture(estado->renderizador, "personaje_equipado.png");
    estado->textura_refugio = IMG_LoadTexture(estado->renderizador, "refugio.png");
    estado->textura_fogata = IMG_LoadTexture(estado->renderizador, "fogata.png");
    estado->textura_cultivo = IMG_LoadTexture(estado->renderizador, "cultivo.png");
    estado->textura_fruta = IMG_LoadTexture(estado->renderizador, "fruta.png");
    estado->textura_semilla = IMG_LoadTexture(estado->renderizador, "semilla.png");
    estado->textura_personaje_espada = IMG_LoadTexture(estado->renderizador, "personaje_espada.png");
    estado->textura_personaje_antorcha = IMG_LoadTexture(estado->renderizador, "personaje_antorcha.png");

    // Cargar fuente
    estado->fuente = TTF_OpenFont("8514oem.ttf", 24);
    if (!estado->fuente)
        return 0;

    // Cargar audio
    estado->musica_dia = Mix_LoadMUS("musica_dia.mp3");
    estado->musica_noche = Mix_LoadMUS("musica_noche.mp3");
    estado->sonido_lucha = Mix_LoadWAV("lucha.mp3");
    estado->sonido_construccion = Mix_LoadWAV("construccion.mp3");
    estado->sonido_clima = Mix_LoadWAV("clima.mp3");

    // Inicializar variables de juego
    estado->clima_actual = CLIMA_SOLEADO;
    estado->es_de_dia = true;
    estado->tiempo_inicio_dia = SDL_GetTicks();

    return 1;
}

void limpiar_juego(EstadoJuego *estado)
{
    // Liberar texturas
    if (estado->textura_arbol)
        SDL_DestroyTexture(estado->textura_arbol);
    if (estado->textura_roca)
        SDL_DestroyTexture(estado->textura_roca);
    if (estado->textura_animal_1)
        SDL_DestroyTexture(estado->textura_animal_1);
    if (estado->textura_animal_2)
        SDL_DestroyTexture(estado->textura_animal_2);
    if (estado->textura_personaje)
        SDL_DestroyTexture(estado->textura_personaje);
    if (estado->textura_personaje_equipado)
        SDL_DestroyTexture(estado->textura_personaje_equipado);
    if (estado->textura_personaje_espada)
        SDL_DestroyTexture(estado->textura_personaje_espada);
    if (estado->textura_personaje_antorcha)
        SDL_DestroyTexture(estado->textura_personaje_antorcha);
    if (estado->textura_refugio)
        SDL_DestroyTexture(estado->textura_refugio);
    if (estado->textura_fogata)
        SDL_DestroyTexture(estado->textura_fogata);
    if (estado->textura_cultivo)
        SDL_DestroyTexture(estado->textura_cultivo);
    if (estado->textura_fruta)
        SDL_DestroyTexture(estado->textura_fruta);
    if (estado->textura_semilla)
        SDL_DestroyTexture(estado->textura_semilla);

    // Liberar recursos de audio
    if (estado->musica_dia)
        Mix_FreeMusic(estado->musica_dia);
    if (estado->musica_noche)
        Mix_FreeMusic(estado->musica_noche);
    if (estado->sonido_lucha)
        Mix_FreeChunk(estado->sonido_lucha);
    if (estado->sonido_construccion)
        Mix_FreeChunk(estado->sonido_construccion);
    if (estado->sonido_clima)
        Mix_FreeChunk(estado->sonido_clima);

    // Liberar fuente
    if (estado->fuente)
        TTF_CloseFont(estado->fuente);

    // Liberar memoria de las gotas de lluvia
    static int *gotas = NULL;
    if (gotas)
    {
        free(gotas);
        gotas = NULL;
    }

    // Liberar renderizador y ventana
    if (estado->renderizador)
        SDL_DestroyRenderer(estado->renderizador);
    if (estado->ventana)
        SDL_DestroyWindow(estado->ventana);

    // Cerrar subsistemas
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
void manejar_antorcha(EstadoJuego *estado)
{
    Uint32 tiempo_actual = SDL_GetTicks();

    // Si tiene antorcha, verificar si se agotó
    if (jugador.tiene_antorcha)
    {
        if (tiempo_actual - jugador.tiempo_inicio_antorcha >= DURACION_ANTORCHA)
        {
            jugador.tiene_antorcha = 0;
            printf("La antorcha se ha agotado\n");
        }
    }

    // Si es de día, quitar la antorcha
    if (estado->es_de_dia && jugador.tiene_antorcha)
    {
        jugador.tiene_antorcha = 0;
        printf("La antorcha se ha apagado por ser de día\n");
    }
}
void actualizar_animales(EstadoJuego *estado)
{
    static Uint32 ultimo_movimiento = 0;
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - ultimo_movimiento < 500) // Actualizar cada 500ms
        return;

    ultimo_movimiento = tiempo_actual;

    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (!animales[i].activo)
            continue;

        // Calcular distancia al jugador
        int dx = jugador.x - animales[i].x;
        int dy = jugador.y - animales[i].y;
        int distancia = (int)sqrt(dx * dx + dy * dy);

        if (distancia <= RADIO_DETECCION_ANIMAL)
        {
            // Perseguir al jugador
            if (dx > 0 && forma_isla[animales[i].y][animales[i].x + 1])
                animales[i].x += VELOCIDAD_ANIMAL;
            else if (dx < 0 && forma_isla[animales[i].y][animales[i].x - 1])
                animales[i].x -= VELOCIDAD_ANIMAL;

            if (dy > 0 && forma_isla[animales[i].y + 1][animales[i].x])
                animales[i].y += VELOCIDAD_ANIMAL;
            else if (dy < 0 && forma_isla[animales[i].y - 1][animales[i].x])
                animales[i].y -= VELOCIDAD_ANIMAL;

            // Si está junto al jugador, atacar
            if (distancia <= 1)
            {
                manejar_combate(&animales[i], estado);
            }
        }
        else
        {
            // Movimiento aleatorio
            int direccion = rand() % 4;
            switch (direccion)
            {
            case 0: // Arriba
                if (forma_isla[animales[i].y - 1][animales[i].x])
                    animales[i].y--;
                break;
            case 1: // Abajo
                if (forma_isla[animales[i].y + 1][animales[i].x])
                    animales[i].y++;
                break;
            case 2: // Izquierda
                if (forma_isla[animales[i].y][animales[i].x - 1])
                    animales[i].x--;
                break;
            case 3: // Derecha
                if (forma_isla[animales[i].y][animales[i].x + 1])
                    animales[i].x++;
                break;
            }
        }
    }
}
void manejar_combate(Animal *animal, EstadoJuego *estado)
{
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - animal->ultimo_ataque < 1000) // Ataque cada segundo
        return;

    animal->ultimo_ataque = tiempo_actual;

    // El animal ataca al jugador
    if (!jugador.tiene_armadura)
        jugador.salud -= ATAQUE_ANIMAL;
    else
        jugador.salud -= ATAQUE_ANIMAL / 2;

    // El jugador ataca al animal
    if (jugador.tiene_espada)
        animal->vida -= DAÑO_ESPADA;
    else
        animal->vida -= ATAQUE_BASE;

    // Reproducir sonido de lucha
    Mix_PlayChannel(-1, estado->sonido_lucha, 0);

    // Verificar si el animal muere
    if (animal->vida <= 0)
    {
        animal->activo = 0;
        jugador.inventario[2] += 2; // Dar carne al jugador
    }

    jugador.energia = MAX(0, jugador.energia - 5); // Consumir energía por luchar
}
// Nuevas funciones para crafteo
void crear_espada(void)
{
    if (jugador.tiene_espada)
        return;

    if (jugador.inventario[0] >= COSTO_ESPADA_MADERA &&
        jugador.inventario[1] >= COSTO_ESPADA_PIEDRA)
    {
        jugador.inventario[0] -= COSTO_ESPADA_MADERA;
        jugador.inventario[1] -= COSTO_ESPADA_PIEDRA;
        jugador.tiene_espada = 1;
        jugador.ataque_actual = ATAQUE_ESPADA;
    }
}

void crear_armadura(void)
{
    if (!jugador.tiene_espada || jugador.tiene_armadura)
        return;

    if (jugador.inventario[0] >= COSTO_ARMADURA_MADERA &&
        jugador.inventario[1] >= COSTO_ARMADURA_PIEDRA)
    {
        jugador.inventario[0] -= COSTO_ARMADURA_MADERA;
        jugador.inventario[1] -= COSTO_ARMADURA_PIEDRA;
        jugador.tiene_armadura = 1;
        jugador.vida_maxima = VIDA_ARMADURA;
    }
}
void inicializar_mapa(void)
{
    // Inicializar el mapa base
    for (int y = 0; y < TAM_MAPA; y++)
    {
        for (int x = 0; x < TAM_MAPA; x++)
        {
            if (!forma_isla[y][x])
            {
                mapa[y][x] = 0; // Agua
            }
            else
            {
                // Verificar si es borde de la isla
                int es_borde =
                    (y > 0 && !forma_isla[y - 1][x]) ||
                    (y < TAM_MAPA - 1 && !forma_isla[y + 1][x]) ||
                    (x > 0 && !forma_isla[y][x - 1]) ||
                    (x < TAM_MAPA - 1 && !forma_isla[y][x + 1]);

                mapa[y][x] = es_borde ? 1 : 2; // 1 = arena, 2 = pasto
            }
        }
    }

    // Colocar elementos en el mapa
    int elementos_colocados = 0;

    // Colocar rocas
    for (int i = 0; i < 10; i++)
    {
        int x, y;
        do
        {
            x = rand() % TAM_MAPA;
            y = rand() % TAM_MAPA;
        } while (!forma_isla[y][x] || mapa[y][x] != 2);
        mapa[y][x] = 3; // Roca
    }

    // Colocar árboles
    for (int i = 0; i < 20; i++)
    {
        int x, y;
        do
        {
            x = rand() % TAM_MAPA;
            y = rand() % TAM_MAPA;
        } while (!forma_isla[y][x] || mapa[y][x] != 2);
        mapa[y][x] = 4; // Árbol
    }
}
void inicializar_jugador(void)
{
    jugador.x = TAM_MAPA / 2;
    jugador.y = TAM_MAPA / 2;
    jugador.salud = VIDA_BASE;
    jugador.hambre = VIDA_BASE;
    jugador.sed = VIDA_BASE;
    jugador.energia = 100;
    jugador.temperatura = 20;
    jugador.tiene_espada = 0;
    jugador.tiene_armadura = 0;
    jugador.tiene_refugio = 0;
    jugador.vida_maxima = VIDA_BASE;
    jugador.ataque_actual = ATAQUE_BASE;
    jugador.nivel_crafteo = 1;

    memset(jugador.inventario, 0, sizeof(jugador.inventario));
    memset(jugador.construcciones, 0, sizeof(jugador.construcciones));
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    EstadoJuego estado;
    SDL_Event evento;
    int ejecutando = 1;
    Uint32 ultimo_tiempo_actualizacion = 0;
    Uint32 ultimo_tiempo_recursos = 0;
    const int MS_POR_FRAME = 16;
    bool pausa = false;

    if (!inicializar_juego(&estado))
    {
        printf("Error al inicializar el juego\n");
        return 1;
    }

    inicializar_jugador();
    inicializar_mapa();
    generar_recursos();

    if (estado.musica_dia)
    {
        Mix_PlayMusic(estado.musica_dia, -1);
    }

    while (ejecutando)
    {
        Uint32 tiempo_inicio_frame = SDL_GetTicks();

        manejar_entrada(&evento, &ejecutando, &estado);

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_ESCAPE])
        {
            pausa = !pausa;
            SDL_Delay(200);
        }

        if (!pausa && jugador.salud > 0)
        {
            actualizar_ciclo_dia_noche(&estado);
            actualizar_clima(&estado);
            actualizar_temperatura_jugador();
            actualizar_construcciones();
            actualizar_animales(&estado);
            actualizar_stats_jugador(&estado);
            manejar_antorcha(&estado);

            Uint32 tiempo_actual = SDL_GetTicks();
            if (tiempo_actual - ultimo_tiempo_recursos >= 10000)
            {
                int recursos_activos = 0;
                for (int i = 0; i < MAX_RECURSOS; i++)
                {
                    if (recursos[i].activo)
                        recursos_activos++;
                }

                if (recursos_activos < MAX_RECURSOS / 2)
                {
                    generar_recursos();
                }
                ultimo_tiempo_recursos = tiempo_actual;
            }

            if (estado.es_de_dia)
            {
                if (rand() % 100 < 5 && jugador.sed < VIDA_BASE)
                {
                    jugador.sed++;
                }
            }
            else
            {
                if (!jugador.tiene_refugio && rand() % 100 < 10)
                {
                    jugador.energia = MAX(0, jugador.energia - 1);
                }
            }
        }

        renderizar_juego(&estado);

        if (pausa)
        {
            SDL_SetRenderDrawBlendMode(estado.renderizador, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(estado.renderizador, 0, 0, 0, 128);
            SDL_Rect overlay = {0, 0, ANCHO_VENTANA, ALTO_VENTANA};
            SDL_RenderFillRect(estado.renderizador, &overlay);

            SDL_Color color_pausa = {255, 255, 255, 255};
            renderizar_texto(&estado, "PAUSA", ANCHO_VENTANA / 2 - 50, ALTO_VENTANA / 2 - 30, color_pausa);
            renderizar_texto(&estado, "Presiona ESC para continuar", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 10, color_pausa);
            SDL_RenderPresent(estado.renderizador);
        }

        Uint32 duracion_frame = SDL_GetTicks() - tiempo_inicio_frame;
        if (duracion_frame < MS_POR_FRAME)
        {
            SDL_Delay(MS_POR_FRAME - duracion_frame);
        }

        if (jugador.salud <= 0)
        {
            Mix_HaltMusic();

            SDL_SetRenderDrawColor(estado.renderizador, 0, 0, 0, 255);
            SDL_RenderClear(estado.renderizador);

            SDL_Color color_rojo = {255, 0, 0, 255};
            SDL_Color color_blanco = {255, 255, 255, 255};

            renderizar_texto(&estado, "GAME OVER", ANCHO_VENTANA / 2 - 100, ALTO_VENTANA / 2 - 50, color_rojo);
            renderizar_texto(&estado, "Presiona ENTER para reiniciar", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 10, color_blanco);
            renderizar_texto(&estado, "Presiona ESC para salir", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 50, color_blanco);

            SDL_RenderPresent(estado.renderizador);

            bool esperando_input = true;
            while (esperando_input)
            {
                while (SDL_PollEvent(&evento))
                {
                    if (evento.type == SDL_QUIT)
                    {
                        esperando_input = false;
                        ejecutando = 0;
                    }
                    else if (evento.type == SDL_KEYDOWN)
                    {
                        if (evento.key.keysym.sym == SDLK_RETURN)
                        {
                            inicializar_jugador();
                            inicializar_mapa();
                            generar_recursos();
                            if (estado.musica_dia)
                            {
                                Mix_PlayMusic(estado.musica_dia, -1);
                            }
                            esperando_input = false;
                        }
                        else if (evento.key.keysym.sym == SDLK_ESCAPE)
                        {
                            esperando_input = false;
                            ejecutando = 0;
                        }
                    }
                }
                SDL_Delay(16);
            }
        }
    }

    limpiar_juego(&estado);
    return 0;
}