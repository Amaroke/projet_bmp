#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdbool.h>
#include "bmp.h"

void transformation_grey(unsigned char tab[3])
{
    unsigned char pixel_gris = (unsigned char)(tab[2] * .299 + tab[1] * .587 + tab[0] * .114);
    memset(tab, pixel_gris, 3);
}

void transformation_red(unsigned char tab[3])
{
    tab[0] = 0;
    tab[1] = 0;
}

void transformation_green(unsigned char tab[3])
{
    tab[0] = 0;
    tab[2] = 0;
}

void transformation_blue(unsigned char tab[3])
{
    tab[1] = 0;
    tab[2] = 0;
}

int transition(int pipe1, int pipe2, char *destination, int offset, char color)
{
    close(pipe2);
    // On ouvre les fichiers.
    int dest = open(destination, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (dest == -1)
    {
        perror("Error open dst gray");
        return EXIT_FAILURE;
    }
    // On utilise pas de lseek puisqu'on change de fichier et qu'on a donc fermé et réouvert les fichiers.
    char temp_char;
    // On recopie l'entête.
    for (int i = 0; i < offset; ++i)
    {
        if (read(pipe1, &temp_char, sizeof(char)) != sizeof(char))
        {
            perror("Error read");
            return EXIT_FAILURE;
        }
        if (write(dest, &temp_char, sizeof(char)) != sizeof(char))
        {
            perror("Error write");
            return EXIT_FAILURE;
        }
    }

    unsigned char pixel_R;
    unsigned char pixel_G;
    unsigned char pixel_B;
    while (read(pipe1, &pixel_R, sizeof(unsigned char)) > 0 && read(pipe1, &pixel_G, sizeof(unsigned char)) > 0 && read(pipe1, &pixel_B, sizeof(unsigned char)) > 0)
    {
        // On récupère les trois pixels et on les traitent.
        unsigned char pixels[3];
        pixels[0] = pixel_R;
        pixels[1] = pixel_G;
        pixels[2] = pixel_B;
        switch (color)
        {
        case 'N':
            transformation_grey(pixels);
            break;
        case 'R':
            transformation_red(pixels);
            break;
        case 'G':
            transformation_green(pixels);
            break;
        case 'B':
            transformation_blue(pixels);
            break;
        default:
            perror("Error transformation");
            return EXIT_FAILURE;
            break;
        }
        if (write(dest, pixels, sizeof(unsigned char) * 3) != 3)
        {
            perror("Error write grey");
            return EXIT_FAILURE;
        }
    }
    close(pipe1);

    // On ferme les fichiers.
    close(dest);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int tube1 = atoi(argv[1]); // p[0]
    int tube2 = atoi(argv[2]); // p[1]
    int offset = atoi(argv[3]);
    char name_destination[1024];
    switch (argv[4][0])
    {
    case 'N':
        sprintf(name_destination, "%s", argv[5]);
        sprintf(memchr(name_destination, '.', 1024), "_gris.bmp");
        break;
    case 'R':
        sprintf(name_destination, "%s", argv[5]);
        sprintf(memchr(name_destination, '.', 1024), "_red.bmp");
        break;
    case 'G':
        sprintf(name_destination, "%s", argv[5]);
        sprintf(memchr(name_destination, '.', 1024), "_green.bmp");
        break;
    case 'B':
        sprintf(name_destination, "%s", argv[5]);
        sprintf(memchr(name_destination, '.', 1024), "_blue.bmp");
        break;
    default:
        perror("Error transformation");
        return EXIT_FAILURE;
        break;
    }
    if (name_destination != NULL)
    {
        transition(tube1, tube2, name_destination, offset, argv[4][0]);
    }
    else
    {
        perror("Error name");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}