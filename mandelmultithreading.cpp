#include <stdio.h>
#include <time.h>
#include <pthread.h>


/** Nombre de threads */
#define NB_THREADS 2

/* Taille de l'image de sortie */
const unsigned int OUTPUT_WIDTH = 1920;
const unsigned int OUTPUT_HEIGHT = 1080;

/* Point de départ de la fenêtre de dessin dans le plan réel / complexe de mandelbrot */
const double START_X = -0.75;
const double START_Y = 0.0;
const double ZOOM = 0.8;

/* ----- Ne rien modifier aprés cette ligne ----- */

/** Tableaux de couleurs (142 couleurs, tirées d'un nuancier trouvé sur le web) */
const unsigned int COLOR_TABLE[] = {
    0xf7df, 0xff5a, 0x07ff, 0x7ffa, 0xf7ff, 0xf7bb, 0xff38, 0xff59, 0x001f, 0x895c,
    0xa145, 0xddd0, 0x5cf4, 0x7fe0, 0xd343, 0xfbea, 0x64bd, 0xffdb, 0xd8a7, 0x07ff,
    0x0011, 0x0451, 0xbc21, 0xad55, 0x0320, 0xbdad, 0x8811, 0x5345, 0xfc60, 0x9999,
    0x8800, 0xecaf, 0x8df1, 0x49f1, 0x2a69, 0x067a, 0x901a, 0xf8b2, 0x05ff, 0x6b4d,
    0x1c9f, 0xd48e, 0xb104, 0xffde, 0x2444, 0xf81f, 0xdefb, 0xffdf, 0xfea0, 0xdd24,
    0x8410, 0x0400, 0xafe5, 0xf7fe, 0xfb56, 0xcaeb, 0x4810, 0xfffe, 0xf731, 0xe73f,
    0xff9e, 0x7fe0, 0xffd9, 0xaedc, 0xf410, 0xe7ff, 0xffda, 0xd69a, 0x9772, 0xfdb8,
    0xfd0f, 0x2595, 0x867f, 0x839f, 0x7453, 0xb63b, 0xfffc, 0x07e0, 0x3666, 0xff9c,
    0xf81f, 0x8000, 0x6675, 0x0019, 0xbaba, 0x939b, 0x3d8e, 0x7b5d, 0x07d3, 0x4e99,
    0xc0b0, 0x18ce, 0xf7ff, 0xff3c, 0xff36, 0xfef5, 0x0010, 0xffbc, 0x8400, 0x6c64,
    0xfd20, 0xfa20, 0xdb9a, 0xef55, 0x9fd3, 0xaf7d, 0xdb92, 0xff7a, 0xfed7, 0xcc27,
    0xfe19, 0xdd1b, 0xb71c, 0x8010, 0xf800, 0xbc71, 0x435c, 0x8a22, 0xfc0e, 0xf52c,
    0x2c4a, 0xffbd, 0xa285, 0xc618, 0x867d, 0x6ad9, 0x7412, 0xffdf, 0x07ef, 0x4416,
    0xd5b1, 0x0410, 0xddfb, 0xfb08, 0x471a, 0xec1d, 0xd112, 0xf6f6, 0xffff, 0xf7be,
    0xffe0, 0x9e66, 0x0000
};

/** Nombre de couleurs dans le tableau */
const unsigned int MAX_ITERATION = sizeof(COLOR_TABLE) / sizeof(COLOR_TABLE[0]);


/** Structure avec les info de calcul */
typedef struct {
    unsigned int x0;
    unsigned int y0;
    unsigned int x1;
    unsigned int y1;
} MandelbrotThreadArgs_t;

/** Buffer pour l'image de sortie entiére (format RGB565) */
unsigned int pixels[OUTPUT_HEIGHT][OUTPUT_WIDTH];


/**
 * Calcul une partie de la fractale de mandelbrot.
 */
void* mandelbrot(void* args) {
    MandelbrotThreadArgs_t* params = static_cast<MandelbrotThreadArgs_t *>(args);

    /* Pour chaque pixel en Y */
    for (unsigned int y = params->y0; y < params->y1; ++y) {
        double p_i = (y - OUTPUT_HEIGHT / 2.0) / (0.5 * ZOOM * OUTPUT_HEIGHT) + START_Y;

        /* Pour chaque pixel en X */
        for (unsigned int x = params->x0; x < params->x1; ++x) {
            double p_r = 1.5 * (x - OUTPUT_WIDTH / 2.0) / (0.5 * ZOOM * OUTPUT_WIDTH) + START_X;
            double new_r = 0, new_i = 0, old_r = 0, old_i = 0;
            unsigned int i = 0;

            /* Magie noir mathématique (merci Wikipedia) */
            while ((new_r * new_r + new_i * new_i) < 4.0 && i < MAX_ITERATION) {
                old_r = new_r;
                old_i = new_i;
                new_r = old_r * old_r - old_i * old_i + p_r;
                new_i = 2.0 * old_r * old_i + p_i;
                ++i;
            }

            /* Garde le résultat en mémoire */
            pixels[y][x] = COLOR_TABLE[i];
        }
    }

    /* Fin du thread */
    pthread_exit(NULL);
}


int main() {
    float temps;
    clock_t t1, t2;
    t1 = clock();

#if NB_THREADS == 1
    /* Exemple 1 thread */
    pthread_t thread0;
    MandelbrotThreadArgs_t args0 = {0, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT};
    pthread_create(&thread0, NULL, mandelbrot, &args0);

    pthread_join(thread0, NULL);
    // 0.359s

#elif NB_THREADS == 2
    /* Exemple 2 threads */
    pthread_t thread0;
    MandelbrotThreadArgs_t args0 = {0, 0, OUTPUT_WIDTH / 2, OUTPUT_HEIGHT};
    pthread_create(&thread0, NULL, mandelbrot, &args0);

    pthread_t thread1;
    MandelbrotThreadArgs_t args1 = {OUTPUT_WIDTH / 2, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT};
    pthread_create(&thread1, NULL, mandelbrot, &args1);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);
    // 0.296s

#elif NB_THREADS == 4
    /* Exemple 4 threads */
    pthread_t thread0;
    MandelbrotThreadArgs_t args0 = {0, 0, OUTPUT_WIDTH / 2, OUTPUT_HEIGHT / 2};
    pthread_create(&thread0, NULL, mandelbrot, &args0);

    pthread_t thread1;
    MandelbrotThreadArgs_t args1 = {OUTPUT_WIDTH / 2, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT / 2};
    pthread_create(&thread1, NULL, mandelbrot, &args1);

    pthread_t thread2;
    MandelbrotThreadArgs_t args2 = {0, OUTPUT_HEIGHT / 2, OUTPUT_WIDTH / 2, OUTPUT_HEIGHT};
    pthread_create(&thread2, NULL, mandelbrot, &args2);

    pthread_t thread3;
    MandelbrotThreadArgs_t args3 = {OUTPUT_WIDTH / 2, OUTPUT_HEIGHT / 2, OUTPUT_WIDTH, OUTPUT_HEIGHT};
    pthread_create(&thread3, NULL, mandelbrot, &args3);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    // 0.156s

#else
#error "NB_THREADS est invalide"
#endif

    t2 = clock();
    temps = (float)(t2 - t1) / CLOCKS_PER_SEC;
    printf("temps = %f s\n", temps);

    /* Ouvre le fichier pour l'image de sortie */
    FILE *fp = fopen("output.ppm", "wb");
    if (fp == NULL) {
        puts("Impossible d'ouvrir le ficher de sortie");
        return -2;
    }

    /* Ecrit l'entête du fichier PPM binaire */
    fprintf(fp, "P3\n%d %d\n255\n", OUTPUT_WIDTH, OUTPUT_HEIGHT);

    /* Génere l'image de sortie */
    for (unsigned int y = 0; y < OUTPUT_HEIGHT; ++y) {
        for (unsigned int x = 0; x < OUTPUT_WIDTH; ++x) {
            unsigned int color = pixels[y][x];
            unsigned int red = ((color >> 11u) & 0x1Fu) << 3u;
            unsigned int green = ((color >> 5u) & 0x3Fu) << 2u;
            unsigned int blue = (color & 0x1Fu) << 3u;

            fprintf(fp, "%d %d %d\n", red, green, blue);
        }
    }

    /* Ferme le fichier de sortie */
    fclose(fp);

    /* The END */
    return 0;
}
