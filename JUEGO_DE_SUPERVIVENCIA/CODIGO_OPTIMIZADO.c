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

// Constantes de tiempo
#define SEGUNDO_REAL_A_JUEGO 12
#define DURACION_DIA 120000
#define DURACION_MITAD_DIA 60000
#define CICLO_CLIMA 30000
#define HORA_INICIO 7

// Tipos de clima
#define CLIMA_SOLEADO 0
#define CLIMA_LLUVIOSO 1
#define CLIMA_TORMENTA 2
#define CLIMA_LLUVIA 1

// Constantes de recursos
#define NUM_ARBOLES_INICIAL 10
#define NUM_ROCAS_INICIAL 5
#define NUM_ARBUSTOS_INICIAL 5
#define UNIDADES_POR_ARBOL 5
#define UNIDADES_POR_ROCA 3
#define UNIDADES_POR_ARBUSTO 2
#define TIEMPO_REAPARICION_ARBOLES 50000
#define TIEMPO_REAPARICION_ROCAS 75000
#define TIEMPO_REAPARICION_FRUTAS 30000

// Constantes de amenazas
#define NUM_ANIMALES_INICIAL 10
#define TIEMPO_REAPARICION_ANIMAL 30000
#define PROB_ATAQUE_DIA 25
#define PROB_ATAQUE_NOCHE 50
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
#define TIEMPO_DAÑO_CRITICO 30000

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

typedef struct
{
    int x, y;
    int tipo;
    int durabilidad;
    bool activa;
    Uint32 tiempo_creacion;
} Construccion;

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
    int tipo;
    Uint32 ultimo_ataque, ultimo_movimiento;
} Animal;

typedef struct
{
    int x, y, tipo, activo;
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
    Mix_Chunk *sonido_lucha, *sonido_construccion, *sonido_clima,
        *sonido_fuego, *sonido_victoria, *sonido_gameover;
    int clima_actual;
    bool es_de_dia;
    Uint32 tiempo_inicio_dia;
} estadojuego;

// Variables globales
Jugador jugador;
Animal animales[MAX_ANIMALES];
Recurso recursos[MAX_RECURSOS];
int mapa[TAM_MAPA][TAM_MAPA];
int recursos_recolectados[5] = {0};

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

SDL_Color color_agua = {64, 224, 208, 255};
SDL_Color color_arena = {238, 214, 175, 255};
SDL_Color color_pasto = {34, 139, 34, 255};

// Funciones
void actualizar_clima(estadojuego *estado);
void actualizar_ciclo_dia_noche(estadojuego *estado);
void manejar_construccion(estadojuego *estado, int tipo_construccion);
void generar_recursos(void);
void generar_amenazas(void);
void renderizar_juego(estadojuego *estado);
void actualizar_construcciones(void);
void actualizar_stats_jugador(estadojuego *estado);
void renderizar_interfaz(estadojuego *estado);
void manejar_entrada(SDL_Event *evento, int *ejecutando, estadojuego *estado);
int inicializar_juego(estadojuego *estado);
void limpiar_juego(estadojuego *estado);
void renderizar_texto(estadojuego *estado, const char *texto, int x, int y, SDL_Color color);
void manejar_combate(Animal *animal, estadojuego *estado);
void actualizar_animales(estadojuego *estado);
void crear_espada(void);
void crear_armadura(void);
void reiniciar_partida(bool nuevo_juego, estadojuego *estado);
void inicializar_jugador(void);
void inicializar_mapa(void);
void mostrar_pantalla_victoria(estadojuego *estado, int *ejecutando);
void guardar_recursos_recolectados(void);
void reiniciar_contador_recursos(void);

// Implementación de funciones
// Manejo de como se actualiza el clima en el juego
void actualizar_clima(estadojuego *estado)
{
    // Variable estática para recordar el clima anterior (inicializada en -1).
    static int clima_anterior = -1;

    // Variable estática para manejar el canal de sonido de lluvia.
    static int canal_lluvia = -1;

    // Obtiene el tiempo actual en milisegundos desde el inicio del programa.
    Uint32 tiempo_actual = SDL_GetTicks();

    // Verifica si ha pasado el tiempo definido por CICLO_CLIMA desde el inicio del día.
    if (tiempo_actual - estado->tiempo_inicio_dia > CICLO_CLIMA)
    {
        // Genera un nuevo estado de clima aleatorio entre 0 y 2.
        int nuevo_clima = rand() % 3;

        // Evita que el clima pase directamente de lluvioso a tormenta.
        if (estado->clima_actual == CLIMA_LLUVIOSO && nuevo_clima == CLIMA_TORMENTA)
        {
            nuevo_clima = CLIMA_LLUVIOSO;
        }

        // Si el clima actual es tormenta y cambia a soleado o lluvioso, detiene el sonido de lluvia.
        if (estado->clima_actual == CLIMA_TORMENTA &&
            (nuevo_clima == CLIMA_SOLEADO || nuevo_clima == CLIMA_LLUVIOSO))
        {
            Mix_HaltChannel(canal_lluvia); // Detiene el canal de sonido de lluvia.
            canal_lluvia = -1;             // Resetea el canal de lluvia.
        }

        // Actualiza el clima actual en la estructura del juego.
        estado->clima_actual = nuevo_clima;

        // Reinicia el tiempo de inicio del día al tiempo actual.
        estado->tiempo_inicio_dia = tiempo_actual;

        // Manejo de sonidos según el nuevo estado del clima.
        if (estado->clima_actual == CLIMA_SOLEADO)
        {
            // Si el clima es soleado y hay un sonido activo, lo detiene.
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
            // Si el clima es lluvioso o tormentoso y no hay sonido, inicia el sonido de lluvia.
            canal_lluvia = Mix_PlayChannel(-1, estado->sonido_clima, -1);
        }

        // Actualiza el clima anterior al clima actual.
        clima_anterior = estado->clima_actual;
    }
}

// Funcion que permite actualizar el horario en el juego (dia o noche)
void actualizar_ciclo_dia_noche(estadojuego *estado)
{
    // Obtiene el tiempo actual en milisegundos desde el inicio del programa.
    Uint32 tiempo_actual = SDL_GetTicks();

    // Guarda el estado anterior del ciclo (si era de día o de noche).
    bool era_de_dia = estado->es_de_dia;

    // Determina si es de día o de noche basado en la duración de la mitad del día.
    estado->es_de_dia = ((tiempo_actual / DURACION_MITAD_DIA) % 2) == 0;

    // Si hubo un cambio en el ciclo (de día a noche o viceversa).
    if (era_de_dia != estado->es_de_dia)
    {
        // Cambia la música a la música del día
        if (estado->es_de_dia)
        {
            if (estado->musica_dia)
                Mix_PlayMusic(estado->musica_dia, -1);

            // Variables para contar árboles y rocas en el mapa.
            int arboles = 0, rocas = 0;

            // Recorre el mapa para contar el número actual de árboles (4) y rocas (3).
            for (int y = 0; y < TAM_MAPA; y++)
                for (int x = 0; x < TAM_MAPA; x++)
                {
                    if (mapa[y][x] == 4)
                        arboles++;
                    if (mapa[y][x] == 3)
                        rocas++;
                }

            // Regenera árboles hasta alcanzar el número inicial (NUM_ARBOLES_INICIAL).
            while (arboles < NUM_ARBOLES_INICIAL)
            {
                int x = rand() % TAM_MAPA, y = rand() % TAM_MAPA;

                // Solo coloca un árbol en áreas válidas (parte de la isla y sin otros recursos).
                if (forma_isla[y][x] && mapa[y][x] == 2)
                {
                    mapa[y][x] = 4;
                    arboles++;
                }
            }

            // Regenera rocas hasta alcanzar el número inicial (NUM_ROCAS_INICIAL).
            while (rocas < NUM_ROCAS_INICIAL)
            {
                int x = rand() % TAM_MAPA, y = rand() % TAM_MAPA;

                // Solo coloca una roca en áreas válidas (parte de la isla y sin otros recursos).
                if (forma_isla[y][x] && mapa[y][x] == 2)
                {
                    mapa[y][x] = 3;
                    rocas++;
                }
            }
        }
        // Cambia la música a la música de la noche
        else if (estado->musica_noche)
        {
            Mix_PlayMusic(estado->musica_noche, -1);
        }
    }
}

// Funcion para la construccion de todos los elementos del juego, refugio, fogata, barco y cultivo
void manejar_construccion(estadojuego *estado, int tipo_construccion)
{
    // Variables estáticas para controlar el progreso de la construcción
    static Uint32 tiempo_inicio_construccion = 0;
    static bool construccion_en_progreso = false;
    static int tipo_actual = -1;
    static int canal_construccion = -1;
    static int canal_fuego = -1;
    static int posicion_x = -1;
    static int posicion_y = -1;

    // Variables para los recursos necesarios y tiempo requerido
    Uint32 tiempo_actual = SDL_GetTicks();
    int madera_necesaria = 0;
    int piedra_necesaria = 0;
    int tiempo_requerido = 0;
    bool requisitos_especiales = true;

    // Maneja el progreso y finalización de una construcción en curso
    if (construccion_en_progreso)
    {
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

        if (tiempo_actual - tiempo_inicio_construccion >= tiempo_requerido)
        {
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

            // Actualiza la información de la construcción completada
            Construccion *nueva_construccion = &jugador.construcciones[0]; // Única construcción activa en la posición actual
            nueva_construccion->tipo = tipo_actual;
            nueva_construccion->x = posicion_x;
            nueva_construccion->y = posicion_y;
            nueva_construccion->activa = true;
            nueva_construccion->durabilidad = 100;
            nueva_construccion->tiempo_creacion = tiempo_actual;

            // Realiza acciones específicas según el tipo de construcción
            switch (tipo_actual)
            {
            case TIPO_REFUGIO:
                jugador.tiene_refugio = 1;
                break;
            case TIPO_CULTIVO:
                jugador.inventario[4]--;
                break;
            case TIPO_FOGATA:
                if (estado->sonido_fuego)
                {
                    canal_fuego = Mix_PlayChannel(-1, estado->sonido_fuego, -1);
                }
                break;
            }

            // Reinicia las variables para una nueva construcción
            construccion_en_progreso = false;
            tipo_actual = -1;
            tiempo_inicio_construccion = 0;
            posicion_x = -1;
            posicion_y = -1;
            return;
        }
        else if (tipo_construccion == -1)
            return;

        return;
    }

    // Si no hay construcción en progreso, verifica si se solicita una nueva construcción
    if (tipo_construccion == -1)
        return;

    // Establece los recursos necesarios y las condiciones para el tipo de construcción solicitado
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
        requisitos_especiales = (jugador.inventario[4] >= 1);
        break;
    default:
        return;
    }

    // Verifica si el jugador cumple con los recursos necesarios y las condiciones especiales
    if (jugador.inventario[0] < madera_necesaria ||
        jugador.inventario[1] < piedra_necesaria ||
        jugador.energia < COSTO_ENERGIA_CONSTRUCCION ||
        !requisitos_especiales)
    {
        return;
    }

    // Configura las variables para iniciar la construcción
    construccion_en_progreso = true;
    tipo_actual = tipo_construccion;
    tiempo_inicio_construccion = tiempo_actual;
    posicion_x = jugador.x;
    posicion_y = jugador.y;

    // Reduce los recursos necesarios del jugador
    jugador.inventario[0] -= madera_necesaria;
    jugador.inventario[1] -= piedra_necesaria;
    jugador.energia = jugador.energia > COSTO_ENERGIA_CONSTRUCCION ? jugador.energia - COSTO_ENERGIA_CONSTRUCCION : 0;

    // Reproduce el sonido de construcción si está disponible
    if (estado->sonido_construccion)
    {
        canal_construccion = Mix_PlayChannel(-1, estado->sonido_construccion, -1);
    }
}

// verificar si hay suficientes árboles, rocas, frutas y semillas. Si faltan los regenera
void generar_recursos(void)
{
    static Uint32 ultimo_tiempo_generacion = 0; // Última vez que se generaron recursos
    static int veces_regeneradas = 0;           // Número de veces que se regeneraron frutas o semillas
    Uint32 tiempo_actual = SDL_GetTicks();
    int tiempo_minimo = TIEMPO_REAPARICION_ARBOLES < TIEMPO_REAPARICION_ROCAS ? TIEMPO_REAPARICION_ARBOLES : TIEMPO_REAPARICION_ROCAS;
    tiempo_minimo = tiempo_minimo < TIEMPO_REAPARICION_FRUTAS ? tiempo_minimo : TIEMPO_REAPARICION_FRUTAS;

    if (tiempo_actual - ultimo_tiempo_generacion < tiempo_minimo)
        return;

    int arboles = 0, rocas = 0, frutas = 0, semillas = 0;

    // Cuenta los árboles y rocas presentes en el mapa
    for (int y = 0; y < TAM_MAPA; y++)
        for (int x = 0; x < TAM_MAPA; x++)
        {
            if (mapa[y][x] == 4) // Árboles
                arboles++;
            if (mapa[y][x] == 3) // Rocas
                rocas++;
        }

    // Cuenta las frutas y semillas activas en el arreglo de recursos
    for (int i = 0; i < MAX_RECURSOS; i++)
    {
        if (recursos[i].activo)
        {
            if (recursos[i].tipo == 2) // Frutas
                frutas++;
            if (recursos[i].tipo == 3) // Semillas
                semillas++;
        }
    }

    // Regenera árboles si son menos de 20 y ha pasado suficiente tiempo
    if (arboles < 20 && tiempo_actual - ultimo_tiempo_generacion >= TIEMPO_REAPARICION_ARBOLES)
    {
        for (int i = 0; i < 20 - arboles; i++)
        {
            // Busca un lugar adecuado en el mapa para colocar un árbol
            for (int intentos = 0; intentos < 100; intentos++)
            {
                int x = rand() % TAM_MAPA, y = rand() % TAM_MAPA;
                if (forma_isla[y][x] && mapa[y][x] == 2)
                {
                    mapa[y][x] = 4; // Coloca un árbol
                    arboles++;
                    break;
                }
            }
        }
    }

    // Regenera rocas si son menos de 10 y ha pasado suficiente tiempo
    if (rocas < 10 && tiempo_actual - ultimo_tiempo_generacion >= TIEMPO_REAPARICION_ROCAS)
    {
        for (int i = 0; i < 10 - rocas; i++)
        {
            // Busca un lugar adecuado en el mapa para colocar una roca
            for (int intentos = 0; intentos < 100; intentos++)
            {
                int x = rand() % TAM_MAPA, y = rand() % TAM_MAPA;
                if (forma_isla[y][x] && mapa[y][x] == 2)
                {
                    mapa[y][x] = 3; // Coloca una roca
                    rocas++;
                    break;
                }
            }
        }
    }

    // Regenera frutas y semillas si no se han regenerado más de dos veces y ha pasado suficiente tiempo
    if (veces_regeneradas < 2 && tiempo_actual - ultimo_tiempo_generacion >= TIEMPO_REAPARICION_FRUTAS)
    {
        // Genera frutas si hay menos de 5
        if (frutas < 5)
        {
            for (int i = 0; i < MAX_RECURSOS && frutas < 5; i++)
            {
                if (!recursos[i].activo)
                {
                    // Busca un lugar adecuado en el mapa para colocar una fruta
                    for (int intentos = 0; intentos < 100; intentos++)
                    {
                        int x = rand() % TAM_MAPA, y = rand() % TAM_MAPA;
                        if (forma_isla[y][x] && mapa[y][x] == 2)
                        {
                            recursos[i].x = x;
                            recursos[i].y = y;
                            recursos[i].tipo = 2; // Tipo fruta
                            recursos[i].activo = 1;
                            frutas++;
                            break;
                        }
                    }
                }
            }
        }

        // Genera semillas si hay menos de 5
        if (semillas < 5)
        {
            for (int i = 0; i < MAX_RECURSOS && semillas < 5; i++)
            {
                if (!recursos[i].activo)
                {
                    // Busca un lugar adecuado en el mapa para colocar una semilla
                    for (int intentos = 0; intentos < 100; intentos++)
                    {
                        int x = rand() % TAM_MAPA, y = rand() % TAM_MAPA;
                        if (forma_isla[y][x] && mapa[y][x] == 2)
                        {
                            recursos[i].x = x;
                            recursos[i].y = y;
                            recursos[i].tipo = 3; // Tipo semilla
                            recursos[i].activo = 1;
                            semillas++;
                            break;
                        }
                    }
                }
            }
        }

        if (frutas > 0 || semillas > 0)
            veces_regeneradas++;
    }

    // Actualiza el tiempo de la última generación de recursos
    ultimo_tiempo_generacion = tiempo_actual;
}

void actualizar_stats_jugador(estadojuego *estado)
{
    static Uint32 ultimo_tiempo_daño_critico = 0;
    static Uint32 ultimo_tiempo_hambre = 0;
    static Uint32 ultimo_tiempo_sed = 0;
    static Uint32 ultimo_tiempo_recuperacion = 0;
    static Uint32 ultimo_tiempo_energia = 0;
    static Uint32 ultimo_tiempo_daño_noche = 0;
    static Uint32 ultimo_tiempo_daño_lluvia = 0;
    static Uint32 ultimo_tiempo_energia_noche = 0;
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - ultimo_tiempo_hambre >= 10000)
    {
        jugador.hambre = jugador.hambre > 0 ? jugador.hambre - 5 : 0;
        ultimo_tiempo_hambre = tiempo_actual;
    }
    if (tiempo_actual - ultimo_tiempo_sed >= 10000)
    {
        jugador.sed = jugador.sed > 0 ? jugador.sed - 5 : 0;
        ultimo_tiempo_sed = tiempo_actual;
    }

    if ((jugador.hambre <= 20 || jugador.sed <= 20) &&
        tiempo_actual - ultimo_tiempo_daño_critico >= 5000)
    {
        jugador.salud = jugador.salud > 5 ? jugador.salud - 5 : 0;
        ultimo_tiempo_daño_critico = tiempo_actual;
    }
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

    if (en_refugio)
    {
        if (tiempo_actual - ultimo_tiempo_recuperacion >= 5000 &&
            jugador.energia >= 20 && jugador.sed > 20 && jugador.hambre > 20)
        {
            jugador.salud = jugador.salud + 10 < jugador.vida_maxima ? jugador.salud + 10 : jugador.vida_maxima;
            jugador.energia = jugador.energia + 10 < 100 ? jugador.energia + 10 : 100;
            ultimo_tiempo_recuperacion = tiempo_actual;
        }

        if (tiempo_actual - ultimo_tiempo_energia >= 5000)
        {
            jugador.energia = jugador.energia + 20 < 100 ? jugador.energia + 20 : 100;
            ultimo_tiempo_energia = tiempo_actual;
        }
    }
    else
    {
        if (!estado->es_de_dia && tiempo_actual - ultimo_tiempo_daño_noche >= 2000)
        {
            jugador.salud = jugador.salud > 2 ? jugador.salud - 2 : 0;
            ultimo_tiempo_daño_noche = tiempo_actual;
        }
        if (estado->clima_actual == CLIMA_LLUVIA && tiempo_actual - ultimo_tiempo_daño_lluvia >= 3000)
        {
            jugador.salud = jugador.salud > (estado->es_de_dia ? 2 : 4) ? jugador.salud - (estado->es_de_dia ? 2 : 4) : 0;
            ultimo_tiempo_daño_lluvia = tiempo_actual;
        }
        if (!estado->es_de_dia && tiempo_actual - ultimo_tiempo_energia_noche >= 3000)
        {
            jugador.energia = jugador.energia > 4 ? jugador.energia - 4 : 0;
            ultimo_tiempo_energia_noche = tiempo_actual;
        }
    }

    // asegurar límites de estadísticas
    jugador.sed = jugador.sed > 100 ? 100 : jugador.sed;
    jugador.hambre = jugador.hambre > 100 ? 100 : jugador.hambre;
    jugador.energia = jugador.energia > 100 ? 100 : jugador.energia;
    jugador.salud = jugador.salud > jugador.vida_maxima ? jugador.vida_maxima : jugador.salud;
}
// Cada 30 segundos, los cultivos activos generan frutas
void actualizar_construcciones(void)
{
    Uint32 tiempo_actual = SDL_GetTicks();

    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa)
        {

            if (jugador.construcciones[i].tipo == TIPO_CULTIVO &&
                tiempo_actual - jugador.construcciones[i].tiempo_creacion >= 30000)
            {

                for (int r = 0; r < MAX_RECURSOS; r++)
                {
                    if (!recursos[r].activo)
                    {
                        recursos[r].x = jugador.construcciones[i].x;
                        recursos[r].y = jugador.construcciones[i].y;
                        recursos[r].tipo = 2;
                        recursos[r].activo = 1;
                        break;
                    }
                }

                jugador.construcciones[i].tiempo_creacion = tiempo_actual;
            }
        }
    }
}

// funcion que se utiliza para la generacion de amenzas en el juego
void generar_amenazas(void)
{
    static Uint32 ultimo_tiempo_amenaza = 0;
    Uint32 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - ultimo_tiempo_amenaza >= 30000)
    {
        for (int i = 0; i < MAX_ANIMALES; i++)
        {
            if (!animales[i].activo)
            {
                int lado = rand() % 4;
                switch (lado)
                {
                case 0:
                    animales[i].x = rand() % TAM_MAPA;
                    animales[i].y = 1;
                    break;
                case 1:
                    animales[i].x = TAM_MAPA - 2;
                    animales[i].y = rand() % TAM_MAPA;
                    break;
                case 2:
                    animales[i].x = rand() % TAM_MAPA;
                    animales[i].y = TAM_MAPA - 2;
                    break;
                case 3:
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
                    break;
                }
            }
        }
        ultimo_tiempo_amenaza = tiempo_actual;
    }
}

// Renderiza texto en pantalla
void renderizar_texto(estadojuego *estado, const char *texto, int x, int y, SDL_Color color)
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

// Renderiza la interfaz del juego
void renderizar_interfaz(estadojuego *estado)
{
    char texto[100];
    int y_offset = 15;
    int x_controles = 10;
    int x_stats = ANCHO_VENTANA - 230;
    SDL_Color color_titulo = {255, 215, 0, 255};
    SDL_Color color_stats = {0, 255, 0, 255};
    SDL_Color color_bajo = {255, 50, 50, 255};
    SDL_Color color_normal = {200, 200, 200, 255};
    SDL_Color color_alto = {100, 255, 255, 255};
    SDL_Color color_especial = {255, 128, 0, 255};

    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 40, 200);
    SDL_Rect panelIzquierdo = {0, 0, 250, ALTO_VENTANA};
    SDL_Rect panelDerecho = {ANCHO_VENTANA - 250, 0, 250, ALTO_VENTANA};
    SDL_RenderFillRect(estado->renderizador, &panelIzquierdo);
    SDL_RenderFillRect(estado->renderizador, &panelDerecho);

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

    y_offset = 15;
    renderizar_texto(estado, "=== ESTADISTICAS ===", x_stats, y_offset, color_titulo);
    y_offset += 35;

    SDL_Rect barra;
    int ancho_barra = 150;
    int alto_barra = 15;

    sprintf(texto, "Salud: %d/100", jugador.salud);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.salud * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 255, 0, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    y_offset += 50;
    sprintf(texto, "Energia: %d/100", jugador.energia);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.energia * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 255, 255, 0, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

    y_offset += 50;
    sprintf(texto, "Hambre: %d/100", jugador.hambre);
    renderizar_texto(estado, texto, x_stats, y_offset, color_normal);
    barra = (SDL_Rect){x_stats, y_offset + 20, (jugador.hambre * ancho_barra) / 100, alto_barra};
    SDL_SetRenderDrawColor(estado->renderizador, 139, 69, 19, 255);
    SDL_RenderFillRect(estado->renderizador, &barra);

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

    sprintf(texto, "Clima: %s", estado->clima_actual == CLIMA_SOLEADO ? "Soleado" : estado->clima_actual == CLIMA_LLUVIOSO ? "Lluvioso"
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

// Maneja la entrada del usuario
void manejar_entrada(SDL_Event *evento, int *ejecutando, estadojuego *estado)
{
    static Uint32 ultima_accion = 0;
    const Uint32 DELAY_ACCION = 200;

    manejar_construccion(estado, -1);

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
            case SDLK_e:
                for (int i = 0; i < MAX_RECURSOS; i++)
                {
                    if (recursos[i].activo && recursos[i].x == jugador.x && recursos[i].y == jugador.y)
                    {
                        if (recursos[i].tipo == 2 || recursos[i].tipo == 4)
                        {
                            if (recursos[i].tipo == 2)
                            {
                                jugador.inventario[3]++;
                                jugador.inventario[4]++;
                                recursos_recolectados[2]++;
                            }
                            else
                            {
                                jugador.inventario[2]++;
                                recursos_recolectados[3]++;
                            }
                            recursos[i].activo = 0;
                            break;
                        }
                    }
                }

                if (jugador.energia >= COSTO_ENERGIA_RECOLECCION)
                {
                    for (int i = 0; i < MAX_RECURSOS; i++)
                    {
                        if (recursos[i].activo && recursos[i].x == jugador.x && recursos[i].y == jugador.y)
                        {
                            if (recursos[i].tipo == 3)
                            {
                                jugador.inventario[4]++;
                                recursos_recolectados[4]++;
                                recursos[i].activo = 0;
                                jugador.energia = jugador.energia > COSTO_ENERGIA_RECOLECCION ? jugador.energia - COSTO_ENERGIA_RECOLECCION : 0;
                                break;
                            }
                        }
                    }

                    if (mapa[jugador.y][jugador.x] == 4)
                    {
                        jugador.inventario[0] += UNIDADES_POR_ARBOL;
                        recursos_recolectados[0] += UNIDADES_POR_ARBOL;
                        mapa[jugador.y][jugador.x] = 2;
                        jugador.energia = jugador.energia > COSTO_ENERGIA_RECOLECCION ? jugador.energia - COSTO_ENERGIA_RECOLECCION : 0;
                    }
                    else if (mapa[jugador.y][jugador.x] == 3)
                    {
                        jugador.inventario[1] += UNIDADES_POR_ROCA;
                        recursos_recolectados[1] += UNIDADES_POR_ROCA;
                        mapa[jugador.y][jugador.x] = 2;
                        jugador.energia = jugador.energia > COSTO_ENERGIA_RECOLECCION ? jugador.energia - COSTO_ENERGIA_RECOLECCION : 0;
                    }
                }
                break;

            case SDLK_SPACE:
                if (mapa[jugador.y][jugador.x] == 1)
                {
                    bool agua_cerca = false;
                    if (jugador.y > 0 && !forma_isla[jugador.y - 1][jugador.x] ||
                        jugador.y < TAM_MAPA - 1 && !forma_isla[jugador.y + 1][jugador.x] ||
                        jugador.x > 0 && !forma_isla[jugador.y][jugador.x - 1] ||
                        jugador.x < TAM_MAPA - 1 && !forma_isla[jugador.y][jugador.x + 1])
                    {
                        jugador.sed = jugador.sed + RECUPERACION_SED_RIO < 100 ? jugador.sed + RECUPERACION_SED_RIO : 100;
                    }
                }
                else if (mapa[jugador.y][jugador.x] == 2)
                {
                    if (jugador.inventario[3] > 0)
                    {
                        jugador.hambre = jugador.hambre + RECUPERACION_HAMBRE_FRUTA < 100 ? jugador.hambre + RECUPERACION_HAMBRE_FRUTA : 100;
                        jugador.salud = jugador.salud + RECUPERACION_FRUTA_SALUD < jugador.vida_maxima ? jugador.salud + RECUPERACION_FRUTA_SALUD : jugador.vida_maxima;
                        jugador.energia = jugador.energia + 20 < 100 ? jugador.energia + 20 : 100;
                        jugador.inventario[3]--;
                    }
                    else if (jugador.inventario[2] > 0)
                    {
                        jugador.hambre = jugador.hambre + RECUPERACION_CARNE_HAMBRE < 100 ? jugador.hambre + RECUPERACION_CARNE_HAMBRE : 100;
                        jugador.salud = jugador.salud + RECUPERACION_CARNE_SALUD < jugador.vida_maxima ? jugador.salud + RECUPERACION_CARNE_SALUD : jugador.vida_maxima;
                        jugador.energia = jugador.energia + 20 < 100 ? jugador.energia + 20 : 100;
                        jugador.inventario[2]--;
                    }
                }
                break;

            case SDLK_r:
                manejar_construccion(estado, TIPO_REFUGIO);
                break;
            case SDLK_f:
                manejar_construccion(estado, TIPO_FOGATA);
                break;
            case SDLK_c:
                manejar_construccion(estado, TIPO_CULTIVO);
                break;
            case SDLK_b:
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
                jugador.energia = jugador.energia > COSTO_ENERGIA_MOVIMIENTO ? jugador.energia - COSTO_ENERGIA_MOVIMIENTO : 0;
            }

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

// Renderiza el juego completo
void renderizar_juego(estadojuego *estado)
{
    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 255);
    SDL_RenderClear(estado->renderizador);

    int ancho_interfaces = 250;
    int x_inicio_mapa = ancho_interfaces;
    int ancho_mapa = ANCHO_VENTANA - (2 * ancho_interfaces);
    int alto_mapa = ALTO_VENTANA;
    float escala_x = (float)ancho_mapa / (TAM_MAPA * TAM_CASILLA);
    float escala_y = (float)alto_mapa / (TAM_MAPA * TAM_CASILLA);
    float escala_mapa = escala_x < escala_y ? escala_x : escala_y;
    int offset_x = x_inicio_mapa + (ancho_mapa - (TAM_MAPA * TAM_CASILLA * escala_mapa)) / 2;
    int offset_y = (alto_mapa - (TAM_MAPA * TAM_CASILLA * escala_mapa)) / 2;

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

            SDL_SetRenderDrawColor(estado->renderizador,
                                   color_terreno.r,
                                   color_terreno.g,
                                   color_terreno.b,
                                   color_terreno.a);
            SDL_RenderFillRect(estado->renderizador, &destRect);

            if (mapa[y][x] == 3)
                SDL_RenderCopy(estado->renderizador, estado->textura_roca, NULL, &destRect);
            else if (mapa[y][x] == 4)
                SDL_RenderCopy(estado->renderizador, estado->textura_arbol, NULL, &destRect);
        }
    }

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
            case 2:
                SDL_RenderCopy(estado->renderizador, estado->textura_fruta, NULL, &destRect);
                break;
            case 3:
                SDL_RenderCopy(estado->renderizador, estado->textura_semilla, NULL, &destRect);
                break;
            case 4:
                SDL_RenderCopy(estado->renderizador, estado->textura_carne, NULL, &destRect);
                break;
            }
        }
    }

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

    if (!estado->es_de_dia)
    {
        SDL_SetRenderDrawBlendMode(estado->renderizador, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 20, 100);
        SDL_Rect pantalla = {x_inicio_mapa, 0, ancho_mapa, ALTO_VENTANA};
        SDL_RenderFillRect(estado->renderizador, &pantalla);
    }

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

    renderizar_interfaz(estado);
    SDL_RenderPresent(estado->renderizador);
}

// Maneja el combate entre el jugador y los animales
void manejar_combate(Animal *animal, estadojuego *estado)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    Uint32 tiempo_actual = SDL_GetTicks();
    static bool ataque_presionado = false;
    bool en_refugio = false;
    bool tiene_fogata_cerca = false;

    // Verifica si el jugador está en un refugio o cerca de una fogata
    for (int i = 0; i < 5; i++)
    {
        if (jugador.construcciones[i].activa)
        {
            if (jugador.construcciones[i].tipo == TIPO_REFUGIO &&
                jugador.x == jugador.construcciones[i].x &&
                jugador.y == jugador.construcciones[i].y)
            {
                en_refugio = true; // El jugador está en un refugio
            }
            if (jugador.construcciones[i].tipo == TIPO_FOGATA &&
                abs(jugador.construcciones[i].x - jugador.x) <= 2 &&
                abs(jugador.construcciones[i].y - jugador.y) <= 2)
            {
                tiene_fogata_cerca = true; // Hay una fogata cerca del jugador
            }
        }
    }

    // Si el jugador está en un refugio y tiene una fogata cerca, se ignora el combate
    if (en_refugio && tiene_fogata_cerca)
        return;

    // Verifica si el animal puede atacar al jugador
    if (tiempo_actual - animal->ultimo_ataque >= 1000)
    {
        animal->ultimo_ataque = tiempo_actual;
        jugador.salud -= jugador.tiene_armadura ? DAÑO_ANIMAL / 2 : DAÑO_ANIMAL; // Calcula el daño recibido
    }

    // Maneja el ataque del jugador
    if (keys[SDL_SCANCODE_RETURN] && !ataque_presionado)
    {
        ataque_presionado = true;                                         // Marca el ataque como presionado
        animal->vida -= jugador.tiene_espada ? DAÑO_ESPADA : ATAQUE_BASE; // Aplica el daño al animal
        Mix_PlayChannel(-1, estado->sonido_lucha, 0);                     // Reproduce el sonido de ataque

        // Reduce la energía del jugador por atacar
        if (jugador.energia >= COSTO_ENERGIA_CONSTRUCCION)
        {
            jugador.energia = jugador.energia > COSTO_ENERGIA_CONSTRUCCION ? jugador.energia - COSTO_ENERGIA_CONSTRUCCION : 0;
        }
    }
    else
    {
        ataque_presionado = false; // Permite atacar nuevamente
    }

    // Si el animal muere, genera un recurso en su posición
    if (animal->vida <= 0)
    {
        for (int i = 0; i < MAX_RECURSOS; i++)
        {
            if (!recursos[i].activo)
            {
                recursos[i].x = animal->x; // Asigna la posición del recurso al lugar donde murió el animal
                recursos[i].y = animal->y;
                recursos[i].tipo = 4; // Tipo de recurso (por ejemplo, carne)
                recursos[i].activo = 1;
                break;
            }
        }
        animal->activo = 0; // Desactiva el animal
    }
}

// Actualiza los estados de los animales
void actualizar_animales(estadojuego *estado)
{
    static Uint32 ultimo_movimiento = 0;
    static Uint32 ultimo_tiempo_spawn = 0;
    Uint32 tiempo_actual = SDL_GetTicks();
    int animales_activos = 0;

    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (animales[i].activo)
            animales_activos++;
    }

    if (animales_activos < NUM_ANIMALES_INICIAL &&
        tiempo_actual - ultimo_tiempo_spawn >= TIEMPO_REAPARICION_ANIMAL)
    {
        for (int i = 0; i < MAX_ANIMALES; i++)
        {
            if (!animales[i].activo)
            {
                int lado = rand() % 4;
                switch (lado)
                {
                case 0:
                    animales[i].x = rand() % TAM_MAPA;
                    animales[i].y = 1;
                    break;
                case 1:
                    animales[i].x = TAM_MAPA - 2;
                    animales[i].y = rand() % TAM_MAPA;
                    break;
                case 2:
                    animales[i].x = rand() % TAM_MAPA;
                    animales[i].y = TAM_MAPA - 2;
                    break;
                case 3:
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

    if (tiempo_actual - ultimo_movimiento < RETARDO_MOV_ANIMAL)
        return;
    ultimo_movimiento = tiempo_actual;

    for (int i = 0; i < MAX_ANIMALES; i++)
    {
        if (!animales[i].activo)
            continue;

        int prob_ataque = estado->es_de_dia ? PROB_ATAQUE_DIA : PROB_ATAQUE_NOCHE;
        if (jugador.tiene_refugio)
            prob_ataque /= 2;

        int dx = jugador.x - animales[i].x;
        int dy = jugador.y - animales[i].y;
        int distancia = (int)sqrt(dx * dx + dy * dy);

        if (distancia <= RADIO_DETECCION_ANIMAL && (rand() % 100 < prob_ataque))
        {
            if (dx > 0 && forma_isla[animales[i].y][animales[i].x + 1])
                animales[i].x += VELOCIDAD_ANIMAL;
            else if (dx < 0 && forma_isla[animales[i].y][animales[i].x - 1])
                animales[i].x -= VELOCIDAD_ANIMAL;

            if (dy > 0 && forma_isla[animales[i].y + 1][animales[i].x])
                animales[i].y += VELOCIDAD_ANIMAL;
            else if (dy < 0 && forma_isla[animales[i].y - 1][animales[i].x])
                animales[i].y -= VELOCIDAD_ANIMAL;

            if (distancia <= 1)
                manejar_combate(&animales[i], estado);
        }
        else
        {
            int direccion = rand() % 4;
            switch (direccion)
            {
            case 0:
                if (animales[i].y > 0 && forma_isla[animales[i].y - 1][animales[i].x])
                    animales[i].y--;
                break;
            case 1:
                if (animales[i].y < TAM_MAPA - 1 && forma_isla[animales[i].y + 1][animales[i].x])
                    animales[i].y++;
                break;
            case 2:
                if (animales[i].x > 0 && forma_isla[animales[i].y][animales[i].x - 1])
                    animales[i].x--;
                break;
            case 3:
                if (animales[i].x < TAM_MAPA - 1 && forma_isla[animales[i].y][animales[i].x + 1])
                    animales[i].x++;
                break;
            }
        }
    }
}

// Crea una espada para el jugador
void crear_espada(void)
{
    if (!jugador.tiene_espada && jugador.energia >= COSTO_ENERGIA_CONSTRUCCION &&
        jugador.inventario[0] >= COSTO_ESPADA_MADERA && jugador.inventario[1] >= COSTO_ESPADA_PIEDRA)
    {
        jugador.inventario[0] -= COSTO_ESPADA_MADERA;
        jugador.inventario[1] -= COSTO_ESPADA_PIEDRA;
        jugador.tiene_espada = 1;
        jugador.ataque_actual = ATAQUE_ESPADA;
        jugador.energia -= COSTO_ENERGIA_CONSTRUCCION;
    }
}

// Crea una armadura para el jugador
void crear_armadura(void)
{
    if (jugador.tiene_espada && !jugador.tiene_armadura &&
        jugador.energia >= COSTO_ENERGIA_CONSTRUCCION &&
        jugador.inventario[0] >= COSTO_ARMADURA_MADERA &&
        jugador.inventario[1] >= COSTO_ARMADURA_PIEDRA)
    {
        jugador.inventario[0] -= COSTO_ARMADURA_MADERA;
        jugador.inventario[1] -= COSTO_ARMADURA_PIEDRA;
        jugador.tiene_armadura = 1;
        jugador.vida_maxima = VIDA_ARMADURA;
        jugador.energia -= COSTO_ENERGIA_CONSTRUCCION;
    }
}

// Reinicia la partida
void reiniciar_partida(bool nuevo_juego, estadojuego *estado)
{
    if (nuevo_juego)
    {
        inicializar_jugador();
        inicializar_mapa();

        for (int i = 0; i < MAX_RECURSOS; i++)
            recursos[i].activo = 0;
        for (int i = 0; i < MAX_ANIMALES; i++)
            animales[i].activo = 0;

        generar_recursos();
        estado->tiempo_inicio_dia = SDL_GetTicks();
        estado->es_de_dia = true;
        estado->clima_actual = CLIMA_SOLEADO;
    }
    else
    {
        int inventario_temp[10];
        memcpy(inventario_temp, jugador.inventario, sizeof(jugador.inventario));
        inicializar_jugador();
        memcpy(jugador.inventario, inventario_temp, sizeof(jugador.inventario));
        generar_recursos();
    }

    if (estado->musica_dia)
    {
        Mix_HaltMusic();
        Mix_PlayMusic(estado->musica_dia, -1);
    }
}

// Inicializa el jugador
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

// Inicializa el mapa del juego
void inicializar_mapa(void)
{
    for (int y = 0; y < TAM_MAPA; y++)
    {
        for (int x = 0; x < TAM_MAPA; x++)
        {
            if (!forma_isla[y][x])
            {
                mapa[y][x] = 0;
            }
            else
            {
                int es_borde = (y > 0 && !forma_isla[y - 1][x]) ||
                               (y < TAM_MAPA - 1 && !forma_isla[y + 1][x]) ||
                               (x > 0 && !forma_isla[y][x - 1]) ||
                               (x < TAM_MAPA - 1 && !forma_isla[y][x + 1]);
                mapa[y][x] = es_borde ? 1 : 2;
            }
        }
    }

    for (int i = 0; i < 10; i++)
    {
        int x, y;
        do
        {
            x = rand() % TAM_MAPA;
            y = rand() % TAM_MAPA;
        } while (!forma_isla[y][x] || mapa[y][x] != 2);
        mapa[y][x] = 3;
    }

    for (int i = 0; i < 20; i++)
    {
        int x, y;
        do
        {
            x = rand() % TAM_MAPA;
            y = rand() % TAM_MAPA;
        } while (!forma_isla[y][x] || mapa[y][x] != 2);
        mapa[y][x] = 4;
    }
}

// Muestra la pantalla de victoria
void mostrar_pantalla_victoria(estadojuego *estado, int *ejecutando)
{
    Mix_HaltMusic();
    if (estado->sonido_victoria)
        Mix_PlayChannel(-1, estado->sonido_victoria, 0);

    SDL_SetRenderDrawColor(estado->renderizador, 0, 0, 0, 255);
    SDL_RenderClear(estado->renderizador);

    SDL_Color color_dorado = {255, 215, 0, 255};
    SDL_Color color_blanco = {255, 255, 255, 255};

    renderizar_texto(estado, "¡FELICITACIONES!", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 - 100, color_dorado);
    renderizar_texto(estado, "¡ESCAPASTE DE LA ISLA!", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 - 50, color_dorado);
    renderizar_texto(estado, "ESPACIO - Nueva partida", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 50, color_blanco);
    renderizar_texto(estado, "ESC - Salir", ANCHO_VENTANA / 2 - 150, ALTO_VENTANA / 2 + 80, color_blanco);

    SDL_RenderPresent(estado->renderizador);

    bool esperando_input = true;
    SDL_Event evento;
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
        SDL_Delay(16);
    }
}

// Guarda los recursos recolectados en un archivo
void guardar_recursos_recolectados(void)
{
    FILE *archivo = fopen("recursos_recolectados.txt", "w");
    if (!archivo)
        return;

    fprintf(archivo, "RECURSOS RECOLECTADOS EN TU ULTIMO JUEGO:\n");
    fprintf(archivo, "%d de madera\n", recursos_recolectados[0]);
    fprintf(archivo, "%d de roca\n", recursos_recolectados[1]);
    fprintf(archivo, "%d frutas\n", recursos_recolectados[2]);
    fprintf(archivo, "%d carnes\n", recursos_recolectados[3]);
    fprintf(archivo, "%d semillas\n", recursos_recolectados[4]);

    fclose(archivo);
}

// Reinicia el contador de recursos
void reiniciar_contador_recursos(void)
{
    memset(recursos_recolectados, 0, sizeof(recursos_recolectados));
}

// Inicializa el juego
int inicializar_juego(estadojuego *estado)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    estado->ventana = SDL_CreateWindow("Isla de Supervivencia - Lab C. Tema 4",
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       ANCHO_VENTANA,
                                       ALTO_VENTANA,
                                       SDL_WINDOW_SHOWN);

    estado->renderizador = SDL_CreateRenderer(estado->ventana, -1,
                                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Cargar texturas
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

    // Cargar audio
    estado->musica_dia = Mix_LoadMUS("../recursos/musica_dia.mp3");
    estado->musica_noche = Mix_LoadMUS("../recursos/musica_noche.mp3");
    estado->sonido_lucha = Mix_LoadWAV("../recursos/lucha.mp3");
    estado->sonido_construccion = Mix_LoadWAV("../recursos/construccion.mp3");
    estado->sonido_clima = Mix_LoadWAV("../recursos/clima.mp3");
    estado->sonido_fuego = Mix_LoadWAV("../recursos/fuego.mp3");
    estado->sonido_victoria = Mix_LoadWAV("../recursos/victoria.mp3");
    estado->sonido_gameover = Mix_LoadWAV("../recursos/gameover.mp3");

    estado->clima_actual = CLIMA_SOLEADO;
    estado->es_de_dia = true;
    estado->tiempo_inicio_dia = SDL_GetTicks();

    return 1;
}

// Limpia los recursos del juego
void limpiar_juego(estadojuego *estado)
{
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
    if (estado->textura_barco)
        SDL_DestroyTexture(estado->textura_barco);
    if (estado->textura_carne)
        SDL_DestroyTexture(estado->textura_carne);
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
    if (estado->sonido_fuego)
        Mix_FreeChunk(estado->sonido_fuego);
    if (estado->sonido_victoria)
        Mix_FreeChunk(estado->sonido_victoria);
    if (estado->sonido_gameover)
        Mix_FreeChunk(estado->sonido_gameover);
    if (estado->fuente)
        TTF_CloseFont(estado->fuente);
    if (estado->renderizador)
        SDL_DestroyRenderer(estado->renderizador);
    if (estado->ventana)
        SDL_DestroyWindow(estado->ventana);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Función principal del juego
int main(int argc, char *argv[])
{
    srand(time(NULL));
    estadojuego estado;
    SDL_Event evento;
    int ejecutando = 1;
    Uint32 ultimo_tiempo_recursos = 0;
    const int MS_POR_FRAME = 16;
    bool pausa = false;

    if (!inicializar_juego(&estado))
        return 1;

    inicializar_jugador();
    inicializar_mapa();
    generar_recursos();

    if (estado.musica_dia)
        Mix_PlayMusic(estado.musica_dia, -1);

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
            actualizar_construcciones();
            actualizar_animales(&estado);
            actualizar_stats_jugador(&estado);

            Uint32 tiempo_actual = SDL_GetTicks();
            if (tiempo_actual - ultimo_tiempo_recursos >= 10000)
            {
                int recursos_activos = 0;
                for (int i = 0; i < MAX_RECURSOS; i++)
                    if (recursos[i].activo)
                        recursos_activos++;

                if (recursos_activos < MAX_RECURSOS / 2)
                    generar_recursos();
                ultimo_tiempo_recursos = tiempo_actual;
            }

            if (!jugador.tiene_refugio && !estado.es_de_dia && rand() % 100 < 10)
                jugador.energia = jugador.energia > 0 ? jugador.energia - 1 : 0;
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
            SDL_Delay(MS_POR_FRAME - duracion_frame);

        if (jugador.salud <= 0)
        {
            Mix_HaltMusic();
            if (estado.sonido_gameover)
                Mix_PlayChannel(-1, estado.sonido_gameover, 0);

            SDL_SetRenderDrawColor(estado.renderizador, 0, 0, 0, 255);
            SDL_RenderClear(estado.renderizador);

            SDL_Color color_rojo = {255, 0, 0, 255};
            SDL_Color color_blanco = {255, 255, 255, 255};

            renderizar_texto(&estado, "GAME OVER", ANCHO_VENTANA / 2 - 100, ALTO_VENTANA / 2 - 50, color_rojo);
            renderizar_texto(&estado, "ENTER - Volver a intentar", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 10, color_blanco);
            renderizar_texto(&estado, "ESPACIO - Nueva partida", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 40, color_blanco);
            renderizar_texto(&estado, "ESC - Salir", ANCHO_VENTANA / 2 - 200, ALTO_VENTANA / 2 + 70, color_blanco);

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
                        switch (evento.key.keysym.sym)
                        {
                        case SDLK_RETURN:
                            reiniciar_partida(false, &estado);
                            esperando_input = false;
                            break;
                        case SDLK_SPACE:
                            guardar_recursos_recolectados();
                            reiniciar_contador_recursos();
                            reiniciar_partida(true, &estado);
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
                SDL_Delay(16);
            }
        }
    }

    limpiar_juego(&estado);
    return 0;
}