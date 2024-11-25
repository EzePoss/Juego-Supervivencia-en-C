// Librerías estándar
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

// Constantes generales del juego
#define ANCHO_VENTANA 1250
#define ALTO_VENTANA 750
#define TAM_CASILLA 28
#define TAM_MAPA 27
#define VEL_JUEGO 2
#define MAX_ANIMALES 10
#define MAX_RECURSOS 15
#define RETARDO_MOV_ANIMAL 500
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Constantes de tiempo
#define SEGUNDO_REAL_A_JUEGO 12  // 1 segundo real = 12 segundos de juego
#define DURACION_DIA 120000      // 2 minutos en milisegundos
#define DURACION_MITAD_DIA 60000 // 1 minuto en milisegundos
#define CICLO_CLIMA 30000        // 30 segundos
#define HORA_INICIO 7

// Tipos de clima
#define CLIMA_SOLEADO 0
#define CLIMA_LLUVIOSO 1
#define CLIMA_TORMENTA 2

// Constantes de recursos
#define NUM_ARBOLES_INICIAL 10
#define NUM_ROCAS_INICIAL 5
#define NUM_ARBUSTOS_INICIAL 5
#define UNIDADES_POR_ARBOL 5
#define UNIDADES_POR_ROCA 3
#define UNIDADES_POR_ARBUSTO 2
#define TIEMPO_REAPARICION_ARBOLES 50000 // 50 segundos reales
#define TIEMPO_REAPARICION_ROCAS 75000   // 75 segundos reales
#define TIEMPO_REAPARICION_FRUTAS 30000  // 30 segundos reales

// Constantes de amenazas
#define NUM_ANIMALES_INICIAL 10
#define TIEMPO_REAPARICION_ANIMAL 30000 // 30 segundos reales
#define PROB_ATAQUE_DIA 25              // 25% por minuto
#define PROB_ATAQUE_NOCHE 50            // 50% por minuto
#define DAÑO_ANIMAL 20
#define RADIO_DETECCION_ANIMAL 5
#define VELOCIDAD_ANIMAL 1

// Constantes del jugador
#define VIDA_BASE 100
#define VIDA_ARMADURA 150
#define MAX_SALUD 100
#define MAX_ENERGIA 100
#define MAX_HAMBRE 100
#define MAX_SED 100
#define ATAQUE_BASE 10
#define ATAQUE_ESPADA 20
#define ATAQUE_COMPLETO 30
#define VIDA_ANIMAL 50
#define DAÑO_ESPADA 15

// Constantes de estados críticos
#define NIVEL_CRITICO_BASE 20
#define NIVEL_CRITICO_ARMADURA 30
#define NIVEL_RECUPERACION_BASE 80
#define NIVEL_RECUPERACION_ARMADURA 90
#define DAÑO_CRITICO 5
#define TIEMPO_DAÑO_CRITICO 30000 // 30 segundos reales

// Constantes de recuperación
#define RECUPERACION_FRUTA_SALUD 10
#define RECUPERACION_CARNE_SALUD 20
#define RECUPERACION_REFUGIO 5
#define TIEMPO_RECUPERACION_REFUGIO 5000
#define RECUPERACION_ENERGIA_FRUTA 10
#define RECUPERACION_ENERGIA_REFUGIO 10
#define RECUPERACION_HAMBRE_FRUTA 15
#define RECUPERACION_CARNE_HAMBRE 30
#define RECUPERACION_SED_RIO 30

// Constantes de costos
#define COSTO_ENERGIA_RECOLECCION 5
#define COSTO_ENERGIA_CONSTRUCCION 15
#define COSTO_ENERGIA_MOVIMIENTO 0
#define TIEMPO_PERDIDA_ENERGIA_MOVIMIENTO 20000
#define PERDIDA_HAMBRE 5
#define TIEMPO_PERDIDA_HAMBRE 10000
#define PERDIDA_SED 5
#define TIEMPO_PERDIDA_SED 10000

// Constantes de construcción
#define TIPO_REFUGIO 1
#define TIPO_FOGATA 2
#define TIPO_CULTIVO 3
#define TIPO_EMBARCACION 4

#define MADERA_REFUGIO 10
#define TIEMPO_CONSTRUCCION_REFUGIO 10000
#define MADERA_FOGATA 5
#define ROCA_FOGATA 1
#define TIEMPO_CONSTRUCCION_FOGATA 5000

// Constantes de crafteo
#define COSTO_ESPADA_MADERA 5
#define COSTO_ESPADA_PIEDRA 10
#define COSTO_ARMADURA_MADERA 8
#define COSTO_ARMADURA_PIEDRA 15
#define TIEMPO_CONSTRUCCION_ESPADA 10000
#define TIEMPO_CONSTRUCCION_ARMADURA 15000

// Constantes de embarcación
#define MADERA_EMBARCACION 50
#define ROCA_EMBARCACION 20
#define TIEMPO_CONSTRUCCION_EMBARCACION 30000
#define MIN_STATS_VICTORIA 50
#define CLIMA_LLUVIA 1 // O el número que uses para representar la lluvia
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
    int x, y, salud, hambre, sed, energia;
    int inventario[10];
    int tiene_espada, tiene_armadura, tiene_refugio;
    int vida_maxima, ataque_actual;
    int nivel_crafteo;
    Construccion construcciones[5];
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
        *textura_refugio, *textura_fogata, *textura_cultivo,
        *textura_fruta, *textura_semilla, *textura_barco, *textura_carne;
    TTF_Font *fuente;
    Mix_Music *musica_dia, *musica_noche;
    Mix_Chunk *sonido_lucha, *sonido_construccion, *sonido_clima, *sonido_fuego, *sonido_victoria, *sonido_gameover;
    int clima_actual;
    bool es_de_dia;
    Uint32 tiempo_inicio_dia;
} EstadoJuego;

// Variables globales
Jugador jugador;
Animal animales[MAX_ANIMALES];
Recurso recursos[MAX_RECURSOS];
int mapa[TAM_MAPA][TAM_MAPA];
int recursos_recolectados[5] = {0, 0, 0, 0, 0};

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
void manejar_construccion(EstadoJuego *estado, int tipo_construccion);
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
void manejar_combate(Animal *animal, EstadoJuego *estado);
void actualizar_animales(EstadoJuego *estado);
void crear_espada(void);
void crear_armadura(void);
void reiniciar_partida(bool nuevo_juego, EstadoJuego *estado);
void inicializar_jugador(void);
void inicializar_mapa(void);
void mostrar_pantalla_victoria(EstadoJuego *estado, int *ejecutando);
void guardar_recursos_recolectados(void);
void reiniciar_contador_recursos(void);
// Implementación de las funciones principales

void actualizar_clima(EstadoJuego *estado)
{
    static int clima_anterior = -1;
    static int canal_lluvia = -1;
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - estado->tiempo_inicio_dia > CICLO_CLIMA)
    {
        int nuevo_clima = rand() % 3;

        // Evitar cambios bruscos
        if (estado->clima_actual == CLIMA_LLUVIOSO && nuevo_clima == CLIMA_TORMENTA)
        {
            nuevo_clima = CLIMA_LLUVIOSO;
        }

        // Detener sonidos anteriores
        if (estado->clima_actual == CLIMA_TORMENTA &&
            (nuevo_clima == CLIMA_SOLEADO || nuevo_clima == CLIMA_LLUVIOSO))
        {
            Mix_HaltChannel(canal_lluvia);
            canal_lluvia = -1;
        }

        estado->clima_actual = nuevo_clima;
        estado->tiempo_inicio_dia = tiempo_actual;

        // Manejar sonidos según el clima
        if (estado->clima_actual == CLIMA_SOLEADO)
        {
            if (canal_lluvia != -1)
            {
                Mix_HaltChannel(canal_lluvia);
                canal_lluvia = -1;
            }
        }
        else if ((estado->clima_actual == CLIMA_LLUVIOSO ||
                  estado->clima_actual == CLIMA_TORMENTA) &&
                 canal_lluvia == -1 && estado->sonido_clima)
        {
            canal_lluvia = Mix_PlayChannel(-1, estado->sonido_clima, -1);
        }

        clima_anterior = estado->clima_actual;
    }
}
void reiniciar_partida(bool nuevo_juego, EstadoJuego *estado)
{
    if (nuevo_juego)
    {
        // Reiniciar todo desde cero
        inicializar_jugador();
        inicializar_mapa();

        // Limpiar todos los recursos y animales existentes
        for (int i = 0; i < MAX_RECURSOS; i++)
        {
            recursos[i].activo = 0;
        }
        for (int i = 0; i < MAX_ANIMALES; i++)
        {
            animales[i].activo = 0;
        }

        // Generar nuevos recursos
        void generar_recursos(void);

        // Establecer tiempo inicial (7:00 AM)
        estado->tiempo_inicio_dia = SDL_GetTicks();
        estado->es_de_dia = true;
        estado->clima_actual = CLIMA_SOLEADO;
    }
    else
    {
        // Mantener el inventario existente
        int inventario_temp[10];
        memcpy(inventario_temp, jugador.inventario, sizeof(jugador.inventario));
        inicializar_jugador();
        memcpy(jugador.inventario, inventario_temp, sizeof(jugador.inventario));

        // Mantener el mismo mapa pero regenerar recursos
        void generar_recursos(void);
    }

    if (estado->musica_dia)
    {
        Mix_HaltMusic();
        Mix_PlayMusic(estado->musica_dia, -1);
    }
}

void actualizar_ciclo_dia_noche(EstadoJuego *estado)
{
    Uint32 tiempo_actual = SDL_GetTicks();
    bool era_de_dia = estado->es_de_dia;

    // Actualizar ciclo día/noche
    estado->es_de_dia = ((tiempo_actual / DURACION_MITAD_DIA) % 2) == 0;

    if (era_de_dia != estado->es_de_dia)
    {
        if (estado->es_de_dia)
        {
            // Amanecer
            if (estado->musica_dia)
                Mix_PlayMusic(estado->musica_dia, -1);

            // Regenerar recursos
            int arboles = 0, rocas = 0;

            // Contar recursos actuales
            for (int y = 0; y < TAM_MAPA; y++)
            {
                for (int x = 0; x < TAM_MAPA; x++)
                {
                    if (mapa[y][x] == 4)
                        arboles++;
                    if (mapa[y][x] == 3)
                        rocas++;
                }
            }

            // Generar árboles faltantes
            while (arboles < NUM_ARBOLES_INICIAL)
            {
                int x, y;
                do
                {
                    x = rand() % TAM_MAPA;
                    y = rand() % TAM_MAPA;
                } while (!forma_isla[y][x] || mapa[y][x] != 2);

                mapa[y][x] = 4;
                arboles++;
            }

            // Generar rocas faltantes
            while (rocas < NUM_ROCAS_INICIAL)
            {
                int x, y;
                do
                {
                    x = rand() % TAM_MAPA;
                    y = rand() % TAM_MAPA;
                } while (!forma_isla[y][x] || mapa[y][x] != 2);

                mapa[y][x] = 3;
                rocas++;
            }
        }
        else
        {
            // Anochecer
            if (estado->musica_noche)
                Mix_PlayMusic(estado->musica_noche, -1);
        }
    }
}

void actualizar_construcciones(void)
{
    Uint32 tiempo_actual = SDL_GetTicks();

    // Para cada construcción activa
    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa)
        {
            // Reducir durabilidad con el tiempo (cada 10 segundos)
            if (tiempo_actual - jugador.construcciones[i].tiempo_creacion > 10000)
            {
                jugador.construcciones[i].durabilidad--;
                jugador.construcciones[i].tiempo_creacion = tiempo_actual;
            }

            // Manejar cultivos
            if (jugador.construcciones[i].tipo == TIPO_CULTIVO)
            {
                // Generar fruta y semilla cada 30 segundos
                if (tiempo_actual - jugador.construcciones[i].tiempo_creacion >= 30000)
                {
                    // Buscar espacio para la fruta
                    for (int r = 0; r < MAX_RECURSOS; r++)
                    {
                        if (!recursos[r].activo)
                        {
                            // Colocar fruta en la posición del cultivo
                            recursos[r].x = jugador.construcciones[i].x;
                            recursos[r].y = jugador.construcciones[i].y;
                            recursos[r].tipo = 2; // Fruta
                            recursos[r].activo = 1;
                            break;
                        }
                    }

                    // Buscar espacio para la semilla en tiles adyacentes
                    int direcciones[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
                    bool semilla_colocada = false;

                    // Intentar colocar en una dirección aleatoria
                    int dir_inicial = rand() % 4;
                    for (int d = 0; d < 4 && !semilla_colocada; d++)
                    {
                        int dir = (dir_inicial + d) % 4;
                        int nuevo_x = jugador.construcciones[i].x + direcciones[dir][0];
                        int nuevo_y = jugador.construcciones[i].y + direcciones[dir][1];

                        if (nuevo_x >= 0 && nuevo_x < TAM_MAPA &&
                            nuevo_y >= 0 && nuevo_y < TAM_MAPA &&
                            forma_isla[nuevo_y][nuevo_x] &&
                            mapa[nuevo_y][nuevo_x] == 2)
                        {

                            // Verificar si el espacio está libre
                            bool espacio_ocupado = false;
                            for (int r = 0; r < MAX_RECURSOS; r++)
                            {
                                if (recursos[r].activo &&
                                    recursos[r].x == nuevo_x &&
                                    recursos[r].y == nuevo_y)
                                {
                                    espacio_ocupado = true;
                                    break;
                                }
                            }

                            if (!espacio_ocupado)
                            {
                                for (int r = 0; r < MAX_RECURSOS; r++)
                                {
                                    if (!recursos[r].activo)
                                    {
                                        recursos[r].x = nuevo_x;
                                        recursos[r].y = nuevo_y;
                                        recursos[r].tipo = 3; // Semilla
                                        recursos[r].activo = 1;
                                        semilla_colocada = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // Resetear el tiempo para el siguiente ciclo
                    jugador.construcciones[i].tiempo_creacion = tiempo_actual;
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
    int y_offset = 15;
    int x_controles = 10;
    int x_stats = ANCHO_VENTANA - 230;

    SDL_Color color_titulo = {255, 215, 0, 255};   // Dorado
    SDL_Color color_stats = {0, 255, 0, 255};      // Verde brillante
    SDL_Color color_bajo = {255, 50, 50, 255};     // Rojo suave
    SDL_Color color_normal = {200, 200, 200, 255}; // Gris claro
    SDL_Color color_alto = {100, 255, 255, 255};   // Cyan brillante
    SDL_Color color_especial = {255, 128, 0, 255}; // Naranja

    // Panel izquierdo
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 40, 200);
    SDL_Rect panelIzquierdo = {0, 0, 250, ALTO_VENTANA};
    SDL_RenderFillRect(estado->renderizador, &panelIzquierdo);

    // Panel derecho
    SDL_Rect panelDerecho = {ANCHO_VENTANA - 250, 0, 250, ALTO_VENTANA};
    SDL_RenderFillRect(estado->renderizador, &panelDerecho);

    // === CONSTRUCCIONES ===
    renderizar_texto(estado, "=== CONSTRUCCIONES ===", x_controles, y_offset, color_titulo);
    y_offset += 35;

    renderizar_texto(estado, "Refugio (R):", x_controles, y_offset, color_especial);
    y_offset += 25;
    renderizar_texto(estado, "- 10 madera", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "Tiempo: 10s", x_controles + 10, y_offset, color_normal);

    y_offset += 30;
    renderizar_texto(estado, "Fogata (F):", x_controles, y_offset, color_especial);
    y_offset += 25;
    renderizar_texto(estado, "- 5 madera", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "- 1 piedra", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "Tiempo: 5s", x_controles + 10, y_offset, color_normal);

    y_offset += 30;
    renderizar_texto(estado, "Cultivo (C):", x_controles, y_offset, color_especial);
    y_offset += 25;
    renderizar_texto(estado, "- 3 madera", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "- 1 semilla", x_controles + 10, y_offset, color_normal);

    y_offset += 30;
    renderizar_texto(estado, "Barco (B):", x_controles, y_offset, color_especial);
    y_offset += 25;
    renderizar_texto(estado, "- 50 madera", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "- 20 piedra", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "Tiempo: 30s", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "(Solo en arena)", x_controles + 10, y_offset, color_normal);

    // === CRAFTEO ===
    y_offset += 35;
    renderizar_texto(estado, "=== CRAFTEO ===", x_controles, y_offset, color_titulo);
    y_offset += 35;

    renderizar_texto(estado, "Espada (1):", x_controles, y_offset, color_especial);
    y_offset += 25;
    renderizar_texto(estado, "- 10 roca", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "- 5 madera", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "Tiempo: 10s", x_controles + 10, y_offset, color_normal);

    y_offset += 30;
    renderizar_texto(estado, "Armadura (2):", x_controles, y_offset, color_especial);
    y_offset += 25;
    renderizar_texto(estado, "- 15 roca", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "- 8 madera", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "Tiempo: 15s", x_controles + 10, y_offset, color_normal);
    y_offset += 20;
    renderizar_texto(estado, "(Requiere espada)", x_controles + 10, y_offset, color_normal);

    // === ESTADISTICAS ===
    y_offset = 15;
    renderizar_texto(estado, "=== ESTADISTICAS ===", x_stats, y_offset, color_titulo);
    y_offset += 35;

    // Barras de estado
    SDL_Rect barra;
    int ancho_barra = 150;
    int alto_barra = 15;

    // Salud
    sprintf(texto, "Salud: %d/100", jugador.salud);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.salud * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 255, 0, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Energia
    y_offset += 50;
    sprintf(texto, "Energia: %d/100", jugador.energia);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.energia * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 255, 255, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Hambre
    y_offset += 50;
    sprintf(texto, "Hambre: %d/100", jugador.hambre);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.hambre * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 139, 69, 19, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    // Sed
    y_offset += 50;
    sprintf(texto, "Sed: %d/100", jugador.sed);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.sed * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 0, 191, 255, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    // === AMBIENTE ===
    y_offset += 45;
    renderizar_texto(estado, "=== AMBIENTE ===", x_stats, y_offset, color_titulo);
    y_offset += 35;

    sprintf(texto, "Clima: %s",
            estado->clima_actual == CLIMA_SOLEADO ? "Soleado" : estado->clima_actual == CLIMA_LLUVIOSO ? "Lluvioso"
                                                                                                       : "Tormenta");
    renderizar_texto(estado, texto, x_stats, y_offset, color_especial);

    y_offset += 25;
    sprintf(texto, "Hora: %s", estado->es_de_dia ? "Dia" : "Noche");
    renderizar_texto(estado, texto, x_stats, y_offset, color_especial);

    // === INVENTARIO ===
    y_offset += 50;
    renderizar_texto(estado, "=== INVENTARIO ===", x_stats, y_offset, color_titulo);
    y_offset += 35;

    sprintf(texto, "Madera: %d", jugador.inventario[0]);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);

    y_offset += 25;
    sprintf(texto, "Piedra: %d", jugador.inventario[1]);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);

    y_offset += 25;
    sprintf(texto, "Carne: %d", jugador.inventario[2]);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);

    y_offset += 25;
    sprintf(texto, "Fruta: %d", jugador.inventario[3]);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);

    y_offset += 25;
    sprintf(texto, "Semillas: %d", jugador.inventario[4]);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);

    // === CONTROLES ===
    y_offset += 45;
    renderizar_texto(estado, "=== CONTROLES ===", x_stats, y_offset, color_titulo);
    y_offset += 35;

    renderizar_texto(estado, "W - Arriba", x_stats, y_offset, color_normal);
    y_offset += 25;
    renderizar_texto(estado, "S - Abajo", x_stats, y_offset, color_normal);
    y_offset += 25;
    renderizar_texto(estado, "A - Izquierda", x_stats, y_offset, color_normal);
    y_offset += 25;
    renderizar_texto(estado, "D - Derecha", x_stats, y_offset, color_normal);
    y_offset += 25;
    renderizar_texto(estado, "E - Recolectar", x_stats, y_offset, color_normal);
    y_offset += 25;
    renderizar_texto(estado, "ESPACIO - Comer/Beber", x_stats, y_offset, color_normal);
    y_offset += 25;
    renderizar_texto(estado, "ENTER - Atacar", x_stats, y_offset, color_normal);
}

void renderizar_juego(EstadoJuego *estado)
{
    // Limpiar pantalla
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 255);
    SDL_RenderClear(estado->renderizador);

    // Calcular dimensiones del mapa
    int ancho_interfaces = 250;
    int x_inicio_mapa = ancho_interfaces;
    int ancho_mapa = ANCHO_VENTANA - (2 * ancho_interfaces);
    int alto_mapa = ALTO_VENTANA;

    // Calcular escala para ajustar el mapa al espacio disponible
    float escala_x = (float)ancho_mapa / (TAM_MAPA * TAM_CASILLA);
    float escala_y = (float)alto_mapa / (TAM_MAPA * TAM_CASILLA);
    float escala_mapa = MIN(escala_x, escala_y);

    // Calcular offset para centrar el mapa
    int offset_x = x_inicio_mapa + (ancho_mapa - (TAM_MAPA * TAM_CASILLA * escala_mapa)) / 2;
    int offset_y = (alto_mapa - (TAM_MAPA * TAM_CASILLA * escala_mapa)) / 2;

    // Renderizar mapa base
    for (int y = 0; y < TAM_MAPA; y++)
    {
        for (int x = 0; x < TAM_MAPA; x++)
        {
            SDL_Rect destRect = {
                offset_x + (int)(x * TAM_CASILLA * escala_mapa),
                offset_y + (int)(y * TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa)};

            SDL_Color color_terreno = forma_isla[y][x] == 0 ? color_agua : mapa[y][x] == 1 ? color_arena
                                                                                           : color_pasto;
            SDL_SetRenderDrawColor(estado->renderizador, color_terreno.r, color_terreno.g, color_terreno.b, color_terreno.a);
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
                offset_x + (int)(recursos[i].x * TAM_CASILLA * escala_mapa),
                offset_y + (int)(recursos[i].y * TAM_CASILLA * escala_mapa),
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
            case 4: // Carne
                SDL_RenderCopy(estado->renderizador, estado->textura_carne, NULL, &destRect);
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
                offset_x + (int)(jugador.construcciones[i].x * TAM_CASILLA * escala_mapa),
                offset_y + (int)(jugador.construcciones[i].y * TAM_CASILLA * escala_mapa),
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
            case TIPO_EMBARCACION:
                SDL_RenderCopy(estado->renderizador, estado->textura_barco, NULL, &destRect);
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
                offset_x + (int)(animales[i].x * TAM_CASILLA * escala_mapa),
                offset_y + (int)(animales[i].y * TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa),
                (int)ceil(TAM_CASILLA * escala_mapa)};

            SDL_Rect barraVida = {
                offset_x + (int)(animales[i].x * TAM_CASILLA * escala_mapa),
                offset_y + (int)(animales[i].y * TAM_CASILLA * escala_mapa - 5),
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
        offset_x + (int)(jugador.x * TAM_CASILLA * escala_mapa),
        offset_y + (int)(jugador.y * TAM_CASILLA * escala_mapa),
        (int)ceil(TAM_CASILLA * escala_mapa),
        (int)ceil(TAM_CASILLA * escala_mapa)};

    SDL_Texture *textura_actual = estado->textura_personaje;
    if (jugador.tiene_armadura)
        textura_actual = estado->textura_personaje_equipado;
    else if (jugador.tiene_espada)
        textura_actual = estado->textura_personaje_espada;

    SDL_RenderCopy(estado->renderizador, textura_actual, NULL, &jugadorRect);

    // Efectos de iluminación y clima
    if (!estado->es_de_dia)
    {
        SDL_SetRenderDrawBlendMode(estado->renderizador, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 20, 100);
        SDL_Rect pantalla = {x_inicio_mapa, 0, ancho_mapa, ALTO_VENTANA};
        SDL_RenderFillRect(estado->renderizador, &pantalla);
    }

    // Renderizar lluvia
    if (estado->clima_actual == CLIMA_LLUVIOSO || estado->clima_actual == CLIMA_TORMENTA)
    {
        SDL_SetRenderDrawBlendMode(estado->renderizador, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 20, 50);
        SDL_Rect pantalla = {x_inicio_mapa, 0, ancho_mapa, ALTO_VENTANA};
        SDL_RenderFillRect(estado->renderizador, &pantalla);

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

    // Renderizar interfaz
    renderizar_interfaz(estado);
    SDL_RenderPresent(estado->renderizador);
}
void mostrar_pantalla_victoria(EstadoJuego *estado, int *ejecutando)
{
    // Detener música actual y reproducir sonido de victoria
    Mix_HaltMusic();
    if (estado->sonido_victoria)
    {
        Mix_PlayChannel(-1, estado->sonido_victoria, 0);
    }

    // Limpiar pantalla
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 255);
    SDL_RenderClear(estado->renderizador);

    // Colores para el texto
    SDL_Color color_dorado = {255, 215, 0, 255};   // Color dorado
    SDL_Color color_blanco = {255, 255, 255, 255}; // Color blanco

    // Renderizar textos de victoria centrados
    renderizar_texto(estado, "¡FELICITACIONES!", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 - 100, color_dorado);
    renderizar_texto(estado, "¡ESCAPASTE DE LA ISLA!", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 - 50, color_dorado);
    renderizar_texto(estado, "ESPACIO - Nueva partida", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 50, color_blanco);
    renderizar_texto(estado, "ESC - Salir", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 80, color_blanco);

    // Actualizar pantalla
    SDL_RenderPresent(estado->renderizador);

    // Esperar input del usuario
    SDL_Event evento;
    bool esperando_input = true;
    while (esperando_input)
    {
        while (SDL_PollEvent(&evento))
        {
            if (evento.type == SDL_QUIT)
            {
                esperando_input = false;
                *ejecutando = 0;
            }
            else if (evento.type == SDL_KEYDOWN)
            {
                if (evento.key.keysym.sym == SDLK_SPACE)
                {
                    reiniciar_partida(true, estado);
                    esperando_input = false;
                }
                else if (evento.key.keysym.sym == SDLK_ESCAPE)
                {
                    esperando_input = false;
                    *ejecutando = 0;
                }
            }
        }
        SDL_Delay(16); // Controlar la frecuencia de actualización
    }
}
void manejar_entrada(SDL_Event *evento, int *ejecutando, EstadoJuego *estado)
{
    static Uint32 ultima_accion = 0;
    const Uint32 DELAY_ACCION = 200;

    // Primero verificamos si hay construcciones en progreso
    manejar_construccion(estado, -1); // Pasamos -1 para indicar que solo queremos verificar el progreso

    while (SDL_PollEvent(evento))
    {
        if (evento->type == SDL_QUIT)
        {
            guardar_recursos_recolectados();
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
            case SDLK_w: // Movimiento hacia arriba
                if (jugador.y > 0 && forma_isla[jugador.y - 1][jugador.x])
                    jugador.y--;
                break;
            case SDLK_s: // Movimiento hacia abajo
                if (jugador.y < TAM_MAPA - 1 && forma_isla[jugador.y + 1][jugador.x])
                    jugador.y++;
                break;
            case SDLK_a: // Movimiento hacia la izquierda
                if (jugador.x > 0 && forma_isla[jugador.y][jugador.x - 1])
                    jugador.x--;
                break;
            case SDLK_d: // Movimiento hacia la derecha
                if (jugador.x < TAM_MAPA - 1 && forma_isla[jugador.y][jugador.x + 1])
                    jugador.x++;
                break;

            case SDLK_e: // Recolectar recursos
                // Primero revisamos si hay frutas o carne para recolectar
                bool hay_comida = false;
                for (int i = 0; i < MAX_RECURSOS; i++)
                {
                    if (recursos[i].activo && recursos[i].x == jugador.x && recursos[i].y == jugador.y)
                    {
                        if (recursos[i].tipo == 2 || recursos[i].tipo == 4)
                        { // Si es fruta o carne
                            hay_comida = true;
                            if (recursos[i].tipo == 2)
                            { // Fruta
                                jugador.inventario[3]++;
                                jugador.inventario[4]++;
                                recursos_recolectados[2]++; // Contador de frutas
                            }
                            else
                            { // Carne
                                jugador.inventario[2]++;
                                recursos_recolectados[3]++; // Contador de carne
                            }
                            recursos[i].activo = 0;
                            break;
                        }
                    }
                }

                // Si no hay comida para recolectar, verificar energía para otros recursos
                if (!hay_comida)
                {
                    if (jugador.energia >= COSTO_ENERGIA_RECOLECCION)
                    {
                        // Recolectar otros recursos (semillas, árboles, rocas)
                        for (int i = 0; i < MAX_RECURSOS; i++)
                        {
                            if (recursos[i].activo && recursos[i].x == jugador.x && recursos[i].y == jugador.y)
                            {
                                if (recursos[i].tipo == 3)
                                { // Semilla
                                    jugador.inventario[4]++;
                                    recursos_recolectados[4]++; // Contador de semillas
                                    recursos[i].activo = 0;
                                    jugador.energia = MAX(0, jugador.energia - COSTO_ENERGIA_RECOLECCION);
                                    break;
                                }
                            }
                        }

                        // Recolectar árboles y rocas
                        if (mapa[jugador.y][jugador.x] == 4)
                        { // Árbol
                            jugador.inventario[0] += UNIDADES_POR_ARBOL;
                            recursos_recolectados[0] += UNIDADES_POR_ARBOL; // Contador de madera
                            mapa[jugador.y][jugador.x] = 2;
                            jugador.energia = MAX(0, jugador.energia - COSTO_ENERGIA_RECOLECCION);
                        }
                        else if (mapa[jugador.y][jugador.x] == 3)
                        { // Roca
                            jugador.inventario[1] += UNIDADES_POR_ROCA;
                            recursos_recolectados[1] += UNIDADES_POR_ROCA; // Contador de roca
                            mapa[jugador.y][jugador.x] = 2;
                            jugador.energia = MAX(0, jugador.energia - COSTO_ENERGIA_RECOLECCION);
                        }
                    }
                    else
                    {
                        printf("No tienes suficiente energia para recolectar este recurso.\n");
                        printf("Solo puedes recolectar frutas y carne cuando tienes poca energia.\n");
                    }
                }
                break;

            case SDLK_SPACE:                         // Comer o beber según el terreno
                if (mapa[jugador.y][jugador.x] == 1) // Si está en arena
                {
                    // Verificar si hay agua cerca
                    bool agua_cerca = false;
                    if (jugador.y > 0 && !forma_isla[jugador.y - 1][jugador.x])
                        agua_cerca = true;
                    if (jugador.y < TAM_MAPA - 1 && !forma_isla[jugador.y + 1][jugador.x])
                        agua_cerca = true;
                    if (jugador.x > 0 && !forma_isla[jugador.y][jugador.x - 1])
                        agua_cerca = true;
                    if (jugador.x < TAM_MAPA - 1 && !forma_isla[jugador.y][jugador.x + 1])
                        agua_cerca = true;

                    if (agua_cerca)
                    {
                        jugador.sed = MIN(100, jugador.sed + RECUPERACION_SED_RIO);
                    }
                }
                else if (mapa[jugador.y][jugador.x] == 2) // Si está en pasto
                {
                    if (jugador.inventario[3] > 0) // Si tiene fruta
                    {
                        jugador.hambre = MIN(100, jugador.hambre + RECUPERACION_HAMBRE_FRUTA);
                        jugador.salud = MIN(jugador.vida_maxima, jugador.salud + RECUPERACION_FRUTA_SALUD);
                        jugador.energia = MIN(100, jugador.energia + 20);
                        jugador.inventario[3]--;
                    }
                    else if (jugador.inventario[2] > 0) // Si tiene carne
                    {
                        jugador.hambre = MIN(100, jugador.hambre + RECUPERACION_CARNE_HAMBRE);
                        jugador.salud = MIN(jugador.vida_maxima, jugador.salud + RECUPERACION_CARNE_SALUD);
                        jugador.energia = MIN(100, jugador.energia + 20);
                        jugador.inventario[2]--;
                    }
                }
                break;

            case SDLK_r: // Construir refugio
                manejar_construccion(estado, TIPO_REFUGIO);
                break;

            case SDLK_f: // Construir fogata
                manejar_construccion(estado, TIPO_FOGATA);
                break;

            case SDLK_c: // Construir cultivo
                manejar_construccion(estado, TIPO_CULTIVO);
                break;

            case SDLK_b: // Construir embarcación
                if (mapa[jugador.y][jugador.x] == 1)
                {
                    manejar_construccion(estado, TIPO_EMBARCACION);
                }
                break;

            case SDLK_1:
                crear_espada();
                break;

            case SDLK_2:
                crear_armadura();
                break;
            }

            // Verificar victoria después del movimiento
            for (int i = 0; i < 5; i++)
            {
                if (jugador.construcciones[i].activa &&
                    jugador.construcciones[i].tipo == TIPO_EMBARCACION &&
                    jugador.x == jugador.construcciones[i].x &&
                    jugador.y == jugador.construcciones[i].y)
                {
                    guardar_recursos_recolectados();
                    mostrar_pantalla_victoria(estado, ejecutando);
                    return;
                }
            }

            if (x_anterior != jugador.x || y_anterior != jugador.y)
            {
                jugador.energia = MAX(0, jugador.energia - COSTO_ENERGIA_MOVIMIENTO);
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

    // Actualizar ciclo día/noche
    void actualizar_ciclo_dia_noche(EstadoJuego * estado);
}

void manejar_construccion(EstadoJuego *estado, int tipo_construccion)
{
    static Uint32 tiempo_inicio_construccion = 0;
    static bool construccion_en_progreso = false;
    static int tipo_actual = -1;
    static int espacio_actual = -1;
    static int canal_construccion = -1;
    static int canal_fuego = -1;
    static int posicion_x = -1;
    static int posicion_y = -1;

    Uint32 tiempo_actual = SDL_GetTicks();
    int madera_necesaria = 0;
    int piedra_necesaria = 0;
    int tiempo_requerido = 0;
    bool requisitos_especiales = true;

    // Si hay una construcción en progreso, verificar si se completó
    if (construccion_en_progreso)
    {
        // Determinar el tiempo requerido según el tipo
        switch (tipo_actual)
        {
        case TIPO_REFUGIO:
            tiempo_requerido = TIEMPO_CONSTRUCCION_REFUGIO;
            break;
        case TIPO_FOGATA:
            tiempo_requerido = TIEMPO_CONSTRUCCION_FOGATA;
            break;
        case TIPO_EMBARCACION:
            tiempo_requerido = TIEMPO_CONSTRUCCION_EMBARCACION;
            break;
        case TIPO_CULTIVO:
            tiempo_requerido = 5000;
            break;
        }

        // Verificar si se completó el tiempo de construcción
        if (tiempo_actual - tiempo_inicio_construccion >= tiempo_requerido)
        {
            // Detener sonidos
            if (canal_construccion != -1)
            {
                Mix_HaltChannel(canal_construccion);
                canal_construccion = -1;
            }
            if (canal_fuego != -1)
            {
                Mix_HaltChannel(canal_fuego);
                canal_fuego = -1;
            }

            // Completar construcción
            Construccion *nueva_construccion = &jugador.construcciones[espacio_actual];
            nueva_construccion->tipo = tipo_actual;
            nueva_construccion->x = posicion_x;
            nueva_construccion->y = posicion_y;
            nueva_construccion->activa = true;
            nueva_construccion->durabilidad = 100;
            nueva_construccion->tiempo_creacion = tiempo_actual;

            // Efectos específicos según el tipo de construcción
            switch (tipo_actual)
            {
            case TIPO_REFUGIO:
                jugador.tiene_refugio = 1;
                printf("¡Has construido un refugio!\n");
                break;
            case TIPO_CULTIVO:
                jugador.inventario[4]--; // Consumir semilla
                printf("¡Has construido un cultivo! Generará frutas y semillas cada 30 segundos.\n");
                break;
            case TIPO_EMBARCACION:
                printf("¡Has construido la embarcación! ¡Puedes escapar de la isla!\n");
                break;
            case TIPO_FOGATA:
                if (estado->sonido_fuego)
                {
                    canal_fuego = Mix_PlayChannel(-1, estado->sonido_fuego, -1);
                }
                printf("¡Has construido una fogata!\n");
                break;
            }

            construccion_en_progreso = false;
            tipo_actual = -1;
            espacio_actual = -1;
            tiempo_inicio_construccion = 0;
            posicion_x = -1;
            posicion_y = -1;

            return;
        }
        else if (tipo_construccion == -1)
        {
            int progreso = ((tiempo_actual - tiempo_inicio_construccion) * 100) / tiempo_requerido;
            printf("Construcción en progreso: %d%%\n", progreso);
            return;
        }
        return;
    }

    if (tipo_construccion == -1)
        return;

    switch (tipo_construccion)
    {
    case TIPO_REFUGIO:
        madera_necesaria = MADERA_REFUGIO;
        tiempo_requerido = TIEMPO_CONSTRUCCION_REFUGIO;
        break;
    case TIPO_FOGATA:
        madera_necesaria = MADERA_FOGATA;
        piedra_necesaria = ROCA_FOGATA;
        tiempo_requerido = TIEMPO_CONSTRUCCION_FOGATA;
        break;
    case TIPO_EMBARCACION:
        madera_necesaria = MADERA_EMBARCACION;
        piedra_necesaria = ROCA_EMBARCACION;
        tiempo_requerido = TIEMPO_CONSTRUCCION_EMBARCACION;
        requisitos_especiales = (jugador.salud >= MIN_STATS_VICTORIA &&
                                 jugador.hambre >= MIN_STATS_VICTORIA &&
                                 jugador.sed >= MIN_STATS_VICTORIA);
        break;
    case TIPO_CULTIVO:
        madera_necesaria = 3;
        tiempo_requerido = 5000;
        requisitos_especiales = (jugador.inventario[4] >= 1); // Verificar si tiene semillas
        break;
    default:
        printf("Tipo de construcción inválido.\n");
        return;
    }

    if (jugador.inventario[0] < madera_necesaria ||
        jugador.inventario[1] < piedra_necesaria ||
        jugador.energia < COSTO_ENERGIA_CONSTRUCCION ||
        !requisitos_especiales)
    {
        if (tipo_construccion == TIPO_CULTIVO && jugador.inventario[4] < 1)
        {
            printf("No tienes semillas para cultivar.\n");
        }
        else
        {
            printf("No hay suficientes recursos, energía o no se cumplen los requisitos.\n");
        }
        return;
    }

    // Buscar espacio disponible para la construcción
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
        printf("No hay espacio para más construcciones.\n");
        return;
    }

    // Iniciar nueva construcción
    construccion_en_progreso = true;
    tipo_actual = tipo_construccion;
    espacio_actual = espacio_encontrado;
    tiempo_inicio_construccion = tiempo_actual;

    // Guardar posición actual del jugador para la construcción
    posicion_x = jugador.x;
    posicion_y = jugador.y;

    // Consumir recursos
    jugador.inventario[0] -= madera_necesaria;
    jugador.inventario[1] -= piedra_necesaria;
    jugador.energia = MAX(0, jugador.energia - COSTO_ENERGIA_CONSTRUCCION);

    // Reproducir sonidos
    if (estado->sonido_construccion)
    {
        canal_construccion = Mix_PlayChannel(-1, estado->sonido_construccion, -1);
    }

    printf("Comenzando construcción...\n");
}

void generar_recursos(void)
{
    static Uint32 ultimo_tiempo_generacion = 0;
    static int veces_regeneradas = 0; // Contador para frutas y semillas
    Uint32 tiempo_actual = SDL_GetTicks();
    int frutas_generadas = 0;   // Declaración añadida
    int semillas_generadas = 0; // Declaración añadida

    // Verificar si ha pasado suficiente tiempo desde la última generación
    Uint32 tiempo_minimo = MIN(TIEMPO_REAPARICION_ARBOLES,
                               MIN(TIEMPO_REAPARICION_ROCAS, TIEMPO_REAPARICION_FRUTAS));

    if (tiempo_actual - ultimo_tiempo_generacion < tiempo_minimo)
    {
        return;
    }

    // Contar recursos actuales
    int arboles = 0, rocas = 0, frutas = 0, semillas = 0;
    for (int y = 0; y < TAM_MAPA; y++)
    {
        for (int x = 0; x < TAM_MAPA; x++)
        {
            if (mapa[y][x] == 4)
                arboles++;
            if (mapa[y][x] == 3)
                rocas++;
        }
    }

    // Contar frutas y semillas existentes
    for (int i = 0; i < MAX_RECURSOS; i++)
    {
        if (recursos[i].activo)
        {
            if (recursos[i].tipo == 2)
                frutas++;
            if (recursos[i].tipo == 3)
                semillas++;
        }
    }

    // Generar árboles si es necesario y si estamos al amanecer
    if (arboles < 20 &&
        tiempo_actual - ultimo_tiempo_generacion >= TIEMPO_REAPARICION_ARBOLES)
    {
        int arboles_faltantes = 20 - arboles;
        for (int i = 0; i < arboles_faltantes; i++)
        {
            int x, y;
            int intentos = 0;
            const int MAX_INTENTOS = 100;

            do
            {
                x = rand() % TAM_MAPA;
                y = rand() % TAM_MAPA;
                intentos++;
                if (intentos >= MAX_INTENTOS)
                    break;
            } while (!forma_isla[y][x] || mapa[y][x] != 2);

            if (intentos < MAX_INTENTOS)
            {
                mapa[y][x] = 4;
                arboles++;
            }
        }
    }

    // Generar rocas si es necesario y si estamos al amanecer
    if (rocas < 10 &&
        tiempo_actual - ultimo_tiempo_generacion >= TIEMPO_REAPARICION_ROCAS)
    {
        int rocas_faltantes = 10 - rocas;
        for (int i = 0; i < rocas_faltantes; i++)
        {
            int x, y;
            int intentos = 0;
            const int MAX_INTENTOS = 100;

            do
            {
                x = rand() % TAM_MAPA;
                y = rand() % TAM_MAPA;
                intentos++;
                if (intentos >= MAX_INTENTOS)
                    break;
            } while (!forma_isla[y][x] || mapa[y][x] != 2);

            if (intentos < MAX_INTENTOS)
            {
                mapa[y][x] = 3;
                rocas++;
            }
        }
    }

    // Generar frutas y semillas solo las primeras 2 veces
    if (veces_regeneradas < 2 &&
        tiempo_actual - ultimo_tiempo_generacion >= TIEMPO_REAPARICION_FRUTAS)
    {
        // Generar frutas si hay menos de 5
        if (frutas < 5)
        {
            for (int i = 0; i < MAX_RECURSOS && frutas_generadas < (5 - frutas); i++)
            {
                if (!recursos[i].activo)
                {
                    int x, y;
                    int intentos = 0;
                    const int MAX_INTENTOS = 100;

                    do
                    {
                        x = rand() % TAM_MAPA;
                        y = rand() % TAM_MAPA;
                        intentos++;
                        if (intentos >= MAX_INTENTOS)
                            break;
                    } while (!forma_isla[y][x] || mapa[y][x] != 2);

                    if (intentos < MAX_INTENTOS)
                    {
                        recursos[i].x = x;
                        recursos[i].y = y;
                        recursos[i].tipo = 2; // Fruta
                        recursos[i].activo = 1;
                        frutas_generadas++;
                    }
                }
            }
        }

        // Generar semillas si hay menos de 5
        if (semillas < 5)
        {
            for (int i = 0; i < MAX_RECURSOS && semillas_generadas < (5 - semillas); i++)
            {
                if (!recursos[i].activo)
                {
                    int x, y;
                    int intentos = 0;
                    const int MAX_INTENTOS = 100;

                    do
                    {
                        x = rand() % TAM_MAPA;
                        y = rand() % TAM_MAPA;
                        intentos++;
                        if (intentos >= MAX_INTENTOS)
                            break;
                    } while (!forma_isla[y][x] || mapa[y][x] != 2);

                    if (intentos < MAX_INTENTOS)
                    {
                        recursos[i].x = x;
                        recursos[i].y = y;
                        recursos[i].tipo = 3; // Semilla
                        recursos[i].activo = 1;
                        semillas_generadas++;
                    }
                }
            }
        }

        if (frutas_generadas > 0 || semillas_generadas > 0)
        {
            veces_regeneradas++;
        }
    }

    ultimo_tiempo_generacion = tiempo_actual;
}

void actualizar_stats_jugador(EstadoJuego *estado)
{
    static Uint32 ultimo_tiempo = 0;
    static Uint32 ultimo_tiempo_daño_critico = 0;
    static Uint32 ultimo_tiempo_hambre = 0;
    static Uint32 ultimo_tiempo_sed = 0;
    static Uint32 ultimo_tiempo_recuperacion = 0;
    static Uint32 ultimo_tiempo_energia = 0;
    static Uint32 ultimo_tiempo_daño_noche = 0;
    static Uint32 ultimo_tiempo_daño_lluvia = 0;
    static Uint32 ultimo_tiempo_energia_noche = 0;

    Uint32 tiempo_actual = SDL_GetTicks();

    // Actualizar hambre cada 10 segundos
    if (tiempo_actual - ultimo_tiempo_hambre >= 10000)
    {
        if (jugador.hambre > 0)
        {
            jugador.hambre = MAX(0, jugador.hambre - 5);
        }
        ultimo_tiempo_hambre = tiempo_actual;
    }

    // Actualizar sed cada 10 segundos
    if (tiempo_actual - ultimo_tiempo_sed >= 10000)
    {
        if (jugador.sed > 0)
        {
            jugador.sed = MAX(0, jugador.sed - 5);
        }
        ultimo_tiempo_sed = tiempo_actual;
    }

    // Daño crítico por hambre o sed cada 5 segundos
    if ((jugador.hambre <= 20 || jugador.sed <= 20) &&
        tiempo_actual - ultimo_tiempo_daño_critico >= 5000)
    {
        jugador.salud = MAX(0, jugador.salud - 5);
        ultimo_tiempo_daño_critico = tiempo_actual;
    }

    // Verificar si está en refugio
    bool en_refugio = false;
    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa &&
            jugador.construcciones[i].tipo == TIPO_REFUGIO &&
            jugador.x == jugador.construcciones[i].x &&
            jugador.y == jugador.construcciones[i].y)
        {
            en_refugio = true;
            break;
        }
    }

    // Recuperación en refugio (día y noche)
    if (en_refugio)
    {
        if (tiempo_actual - ultimo_tiempo_recuperacion >= 5000)
        {
            if (jugador.energia >= 20 && jugador.sed > 20 && jugador.hambre > 20)
            {
                jugador.salud = MIN(jugador.vida_maxima, jugador.salud + 10); // Cambiado de 5 a 10
                jugador.energia = MIN(100, jugador.energia + 10);
            }
            ultimo_tiempo_recuperacion = tiempo_actual;
        }

        // Recuperación extra de energía en refugio
        if (tiempo_actual - ultimo_tiempo_energia >= 5000)
        {
            jugador.energia = MIN(100, jugador.energia + 20);
            ultimo_tiempo_energia = tiempo_actual;
        }
    }
    else
    {
        // Si NO está en refugio, aplicar daños

        // Daño por noche
        if (!estado->es_de_dia &&
            tiempo_actual - ultimo_tiempo_daño_noche >= 2000)
        {
            jugador.salud = MAX(0, jugador.salud - 2);
            ultimo_tiempo_daño_noche = tiempo_actual;
        }

        // Daño por lluvia (solo si no está en refugio)
        if (estado->clima_actual == CLIMA_LLUVIA &&
            tiempo_actual - ultimo_tiempo_daño_lluvia >= 3000)
        {
            if (estado->es_de_dia)
            {
                // Daño por lluvia durante el día
                jugador.salud = MAX(0, jugador.salud - 2);
            }
            else
            {
                // Daño por lluvia durante la noche
                jugador.salud = MAX(0, jugador.salud - 4);
            }
            ultimo_tiempo_daño_lluvia = tiempo_actual;
        }

        // Consumo de energía durante la noche (modificado)
        if (!estado->es_de_dia &&
            tiempo_actual - ultimo_tiempo_energia_noche >= 3000) // Cambiado a 3000 (3 segundos)
        {
            jugador.energia = MAX(0, jugador.energia - 4); // Se mantiene el daño de 4
            ultimo_tiempo_energia_noche = tiempo_actual;
        }
    }

    // Asegurarnos de que las estadísticas nunca excedan sus límites máximos
    jugador.sed = MIN(100, jugador.sed);
    jugador.hambre = MIN(100, jugador.hambre);
    jugador.energia = MIN(100, jugador.energia);
    jugador.salud = MIN(jugador.vida_maxima, jugador.salud);
}

// Agregar esta nueva función
void guardar_recursos_recolectados(void)
{
    FILE *archivo = fopen("recursos_recolectados.txt", "w");
    if (archivo == NULL)
    {
        printf("Error al crear el archivo de recursos\n");
        return;
    }

    fprintf(archivo, "RECURSOS RECOLECTADOS EN TU ULTIMO JUEGO:\n");
    fprintf(archivo, "%d de madera\n", recursos_recolectados[0]);
    fprintf(archivo, "%d de roca\n", recursos_recolectados[1]);
    fprintf(archivo, "%d frutas\n", recursos_recolectados[2]);
    fprintf(archivo, "%d carnes\n", recursos_recolectados[3]);
    fprintf(archivo, "%d semillas\n", recursos_recolectados[4]);

    fclose(archivo);
}

// Agregar esta función para reiniciar el contador
void reiniciar_contador_recursos(void)
{
    for (int i = 0; i < 5; i++)
    {
        recursos_recolectados[i] = 0;
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
    // Para texturas con ruta relativa desde JUEGO_DE_SUPERVIVENCIA
    estado->textura_arbol = IMG_LoadTexture(estado->renderizador, "../recursos/arbol.png");
    estado->textura_roca = IMG_LoadTexture(estado->renderizador, "../recursos/roca.png");
    estado->textura_animal_1 = IMG_LoadTexture(estado->renderizador, "../recursos/animal_1.png");
    estado->textura_animal_2 = IMG_LoadTexture(estado->renderizador, "../recursos/animal_2.png");
    estado->textura_personaje = IMG_LoadTexture(estado->renderizador, "../recursos/personaje.png");
    estado->textura_personaje_equipado = IMG_LoadTexture(estado->renderizador, "../recursos/personaje_equipado.png");
    estado->textura_refugio = IMG_LoadTexture(estado->renderizador, "../recursos/refugio.png");
    estado->textura_fogata = IMG_LoadTexture(estado->renderizador, "../recursos/fogata.png");
    estado->textura_cultivo = IMG_LoadTexture(estado->renderizador, "../recursos/cultivo.png");
    estado->textura_fruta = IMG_LoadTexture(estado->renderizador, "../recursos/fruta.png");
    estado->textura_semilla = IMG_LoadTexture(estado->renderizador, "../recursos/semilla.png");
    estado->textura_personaje_espada = IMG_LoadTexture(estado->renderizador, "../recursos/personaje_espada.png");
    estado->textura_barco = IMG_LoadTexture(estado->renderizador, "../recursos/barco.png");
    estado->textura_carne = IMG_LoadTexture(estado->renderizador, "../recursos/carne.png");

    // Cargar fuente
    estado->fuente = TTF_OpenFont("../recursos/8514oem.ttf", 24);
    if (!estado->fuente)
        return 0;

    // Cargar audio
    estado->musica_dia = Mix_LoadMUS("../recursos/musica_dia.mp3");
    estado->musica_noche = Mix_LoadMUS("../recursos/musica_noche.mp3");
    estado->sonido_lucha = Mix_LoadWAV("../recursos/lucha.mp3");
    estado->sonido_construccion = Mix_LoadWAV("../recursos/construccion.mp3");
    estado->sonido_clima = Mix_LoadWAV("../recursos/clima.mp3");
    estado->sonido_fuego = Mix_LoadWAV("../recursos/fuego.mp3");
    estado->sonido_victoria = Mix_LoadWAV("../recursos/victoria.mp3");
    estado->sonido_gameover = Mix_LoadWAV("../recursos/gameover.mp3");

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
    if (estado->sonido_fuego)
        Mix_FreeChunk(estado->sonido_fuego);
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
    if (estado->textura_barco)
        SDL_DestroyTexture(estado->textura_barco);
    if (estado->textura_carne)
        SDL_DestroyTexture(estado->textura_carne);
    if (estado->sonido_victoria)
        Mix_FreeChunk(estado->sonido_victoria);
    if (estado->sonido_gameover)
        Mix_FreeChunk(estado->sonido_gameover);

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

void actualizar_animales(EstadoJuego *estado)
{
    static Uint32 ultimo_movimiento = 0;
    static Uint32 ultimo_tiempo_spawn = 0;
    Uint32 tiempo_actual = SDL_GetTicks();

    // Contar animales activos
    int animales_activos = 0;
    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (animales[i].activo)
            animales_activos++;
    }

    // Generar nuevos animales si es necesario
    if (animales_activos < NUM_ANIMALES_INICIAL &&
        tiempo_actual - ultimo_tiempo_spawn >= TIEMPO_REAPARICION_ANIMAL)
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
                    animales_activos++;
                    if (animales_activos >= NUM_ANIMALES_INICIAL)
                        break;
                }
            }
        }
        ultimo_tiempo_spawn = tiempo_actual;
    }

    // Actualizar comportamiento de animales
    if (tiempo_actual - ultimo_movimiento < RETARDO_MOV_ANIMAL)
        return;
    ultimo_movimiento = tiempo_actual;

    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (!animales[i].activo)
            continue;

        // Calcular probabilidad de ataque según el momento del día
        int prob_ataque = estado->es_de_dia ? PROB_ATAQUE_DIA : PROB_ATAQUE_NOCHE;
        if (jugador.tiene_refugio)
            prob_ataque = prob_ataque / 2;

        // Calcular distancia al jugador
        int dx = jugador.x - animales[i].x;
        int dy = jugador.y - animales[i].y;
        int distancia = (int)sqrt(dx * dx + dy * dy);

        if (distancia <= RADIO_DETECCION_ANIMAL && (rand() % 100 < prob_ataque))
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

            // Si está junto al jugador o su refugio, atacar
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
                if (animales[i].y > 0 && forma_isla[animales[i].y - 1][animales[i].x])
                    animales[i].y--;
                break;
            case 1: // Abajo
                if (animales[i].y < TAM_MAPA - 1 && forma_isla[animales[i].y + 1][animales[i].x])
                    animales[i].y++;
                break;
            case 2: // Izquierda
                if (animales[i].x > 0 && forma_isla[animales[i].y][animales[i].x - 1])
                    animales[i].x--;
                break;
            case 3: // Derecha
                if (animales[i].x < TAM_MAPA - 1 && forma_isla[animales[i].y][animales[i].x + 1])
                    animales[i].x++;
                break;
            }
        }
    }
}
void manejar_combate(Animal *animal, EstadoJuego *estado)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    Uint32 tiempo_actual = SDL_GetTicks();
    static bool ataque_presionado = false;

    // Verificar si el jugador está en un refugio
    bool en_refugio = false;
    bool tiene_fogata_cerca = false;

    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa)
        {
            // Verificar si está en refugio
            if (jugador.construcciones[i].tipo == TIPO_REFUGIO &&
                jugador.x == jugador.construcciones[i].x &&
                jugador.y == jugador.construcciones[i].y)
            {
                en_refugio = true;
            }

            // Verificar si hay fogata cerca
            if (jugador.construcciones[i].tipo == TIPO_FOGATA &&
                abs(jugador.construcciones[i].x - jugador.x) <= 2 &&
                abs(jugador.construcciones[i].y - jugador.y) <= 2)
            {
                tiene_fogata_cerca = true;
            }
        }
    }

    // Si está en refugio y tiene fogata cerca, no permitir el ataque
    if (en_refugio && tiene_fogata_cerca)
    {
        return;
    }

    // Verificar el ataque del animal (cada segundo)
    if (tiempo_actual - animal->ultimo_ataque >= 1000)
    {
        animal->ultimo_ataque = tiempo_actual;

        // El animal ataca al jugador
        if (!jugador.tiene_armadura)
            jugador.salud -= DAÑO_ANIMAL;
        else
            jugador.salud -= DAÑO_ANIMAL / 2;
    }

    // Ataque del jugador
    if (keys[SDL_SCANCODE_RETURN]) // Si ENTER está presionado
    {
        if (!ataque_presionado) // Solo si no estaba presionado antes
        {
            ataque_presionado = true;

            // Aplicar daño al animal
            if (jugador.tiene_espada)
                animal->vida -= DAÑO_ESPADA;
            else
                animal->vida -= ATAQUE_BASE;

            // Reproducir sonido de lucha
            Mix_PlayChannel(-1, estado->sonido_lucha, 0);

            // Consumir energía por luchar (si es necesario)
            if (jugador.energia >= COSTO_ENERGIA_CONSTRUCCION)
            {
                jugador.energia = MAX(0, jugador.energia - COSTO_ENERGIA_CONSTRUCCION);
            }
        }
    }
    else
    {
        ataque_presionado = false; // Resetear el estado cuando se suelta la tecla
    }

    // Verificar si el animal muere
    if (animal->vida <= 0)
    {
        // Crear un nuevo recurso de carne en la posición del animal
        for (int i = 0; i < MAX_RECURSOS; i++)
        {
            if (!recursos[i].activo)
            {
                recursos[i].x = animal->x;
                recursos[i].y = animal->y;
                recursos[i].tipo = 4; // Nuevo tipo para carne
                recursos[i].activo = 1;
                break;
            }
        }
        animal->activo = 0;
    }
}
// Nuevas funciones para crafteo
void crear_espada(void)
{
    if (jugador.tiene_espada)
    {
        printf("Ya tienes una espada equipada.\n");
        return;
    }

    if (jugador.energia < COSTO_ENERGIA_CONSTRUCCION)
    {
        printf("No tienes suficiente energía para crear una espada.\n");
        return;
    }

    if (jugador.inventario[0] >= COSTO_ESPADA_MADERA &&
        jugador.inventario[1] >= COSTO_ESPADA_PIEDRA)
    {
        jugador.inventario[0] -= COSTO_ESPADA_MADERA;
        jugador.inventario[1] -= COSTO_ESPADA_PIEDRA;
        jugador.tiene_espada = 1;
        jugador.ataque_actual = ATAQUE_ESPADA;
        jugador.energia -= COSTO_ENERGIA_CONSTRUCCION;
        printf("¡Has creado una espada!\n");
    }
    else
    {
        printf("No tienes suficientes recursos para crear una espada.\n");
        printf("Necesitas: %d madera y %d piedra.\n", COSTO_ESPADA_MADERA, COSTO_ESPADA_PIEDRA);
    }
}

void crear_armadura(void)
{
    if (!jugador.tiene_espada)
    {
        printf("Necesitas tener una espada para crear una armadura.\n");
        return;
    }

    if (jugador.tiene_armadura)
    {
        printf("Ya tienes una armadura equipada.\n");
        return;
    }

    if (jugador.energia < COSTO_ENERGIA_CONSTRUCCION)
    {
        printf("No tienes suficiente energía para crear una armadura.\n");
        return;
    }

    if (jugador.inventario[0] >= COSTO_ARMADURA_MADERA &&
        jugador.inventario[1] >= COSTO_ARMADURA_PIEDRA)
    {
        jugador.inventario[0] -= COSTO_ARMADURA_MADERA;
        jugador.inventario[1] -= COSTO_ARMADURA_PIEDRA;
        jugador.tiene_armadura = 1;
        jugador.vida_maxima = VIDA_ARMADURA;
        jugador.energia -= COSTO_ENERGIA_CONSTRUCCION;
    }
    else
    {
        printf("No tienes suficientes recursos para crear una armadura.\n");
        printf("Necesitas: %d madera y %d piedra.\n", COSTO_ARMADURA_MADERA, COSTO_ARMADURA_PIEDRA);
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
    // Semilla para generación aleatoria
    srand(time(NULL));

    // Variables principales del juego
    EstadoJuego estado;
    SDL_Event evento;
    int ejecutando = 1;
    Uint32 ultimo_tiempo_recursos = 0;
    const int MS_POR_FRAME = 16; // Tiempo por frame (aproximadamente 60 FPS)
    bool pausa = false;

    // Inicialización del juego
    if (!inicializar_juego(&estado))
    {
        printf("Error al inicializar el juego\n");
        return 1;
    }

    // Inicialización de otros elementos del juego
    inicializar_jugador();
    inicializar_mapa();
    generar_recursos();

    // Iniciar música de día si está disponible
    if (estado.musica_dia)
    {
        Mix_PlayMusic(estado.musica_dia, -1);
    }

    // Bucle principal del juego
    while (ejecutando)
    {
        Uint32 tiempo_inicio_frame = SDL_GetTicks();

        // Manejo de entrada del usuario
        manejar_entrada(&evento, &ejecutando, &estado);

        // Detectar tecla ESC para pausar o continuar
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_ESCAPE])
        {
            pausa = !pausa; // Alterna entre pausa y juego
            SDL_Delay(200); // Evitar múltiples cambios rápidos
        }

        if (!pausa && jugador.salud > 0) // Solo actualizar si no está en pausa
        {
            // Actualizaciones del juego
            actualizar_ciclo_dia_noche(&estado);
            actualizar_clima(&estado);
            actualizar_construcciones();
            actualizar_animales(&estado);
            actualizar_stats_jugador(&estado);

            // Generar nuevos recursos cada 10 segundos
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

            // Penalización si no hay refugio durante la noche
            if (!jugador.tiene_refugio && !estado.es_de_dia && rand() % 100 < 10)
            {
                jugador.energia = MAX(0, jugador.energia - 1);
            }
        }

        // Renderizar la escena del juego
        renderizar_juego(&estado);

        // Renderizar superposición de pausa si está activa
        if (pausa)
        {
            SDL_SetRenderDrawBlendMode(estado.renderizador, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(estado.renderizador, 0, 0, 0, 128); // Fondo semi-transparente
            SDL_Rect overlay = {0, 0, ANCHO_VENTANA, ALTO_VENTANA};
            SDL_RenderFillRect(estado.renderizador, &overlay);

            SDL_Color color_pausa = {255, 255, 255, 255};
            renderizar_texto(&estado, "PAUSA", ANCHO_VENTANA / 2 - 50, ALTO_VENTANA / 2 - 30, color_pausa);
            renderizar_texto(&estado, "Presiona ESC para continuar", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 10, color_pausa);
            SDL_RenderPresent(estado.renderizador);
        }

        // Controlar el tiempo de cada frame
        Uint32 duracion_frame = SDL_GetTicks() - tiempo_inicio_frame;
        if (duracion_frame < MS_POR_FRAME)
        {
            SDL_Delay(MS_POR_FRAME - duracion_frame);
        }

        // Comprobación de fin de juego
        if (jugador.salud <= 0)
        {
            // Detener música y reproducir sonido de game over
            Mix_HaltMusic();
            if (estado.sonido_gameover)
            {
                Mix_PlayChannel(-1, estado.sonido_gameover, 0);
            }

            SDL_SetRenderDrawColor(estado.renderizador, 0, 0, 0, 255);
            SDL_RenderClear(estado.renderizador);

            SDL_Color color_rojo = {255, 0, 0, 255};
            SDL_Color color_blanco = {255, 255, 255, 255};

            renderizar_texto(&estado, "GAME OVER", ANCHO_VENTANA / 2 - 100, ALTO_VENTANA / 2 - 50, color_rojo);
            renderizar_texto(&estado, "ENTER - Volver a intentar", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 10, color_blanco);
            renderizar_texto(&estado, "ESPACIO - Nueva partida", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 40, color_blanco);
            renderizar_texto(&estado, "ESC - Salir", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 70, color_blanco);

            SDL_RenderPresent(estado.renderizador);

            // Manejo de entrada en pantalla de GAME OVER
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
                        switch (evento.key.keysym.sym)
                        {
                        case SDLK_RETURN:
                            reiniciar_partida(false, &estado); // Volver a intentar sin reiniciar contadores
                            esperando_input = false;
                            break;
                        case SDLK_SPACE:
                            guardar_recursos_recolectados();  // Guardar recursos antes de reiniciar
                            reiniciar_contador_recursos();    // Reiniciar contadores
                            reiniciar_partida(true, &estado); // Nueva partida
                            esperando_input = false;
                            break;
                        case SDLK_ESCAPE:
                            guardar_recursos_recolectados();
                            esperando_input = false;
                            ejecutando = 0;
                            break;
                        }
                    }
                }
                SDL_Delay(16); // Evitar alta carga de CPU durante la espera
            }
        }
    }

    // Liberar recursos al finalizar el juego
    limpiar_juego(&estado);
    return 0;
}