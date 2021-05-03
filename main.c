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

bool file_exists(char *nom)
{
    struct stat s;
    if (stat(nom, &s) == -1) // On vérifie l'existence du fichier.
    {
        perror("Error stat");
        return false;
    }
    return S_ISREG(s.st_mode); // On return vrai seulement si c'est un fichier.
}

int reading_header(char *nom, bmp_t *bitmap)
{
    int fichier = open(nom, O_RDONLY); // On ouvre le fichier à recopier.
    if (fichier == -1)
    {
        perror("Error open src");
        return EXIT_FAILURE;
    }
    if (read(fichier, bitmap, sizeof(bmp_t)) != sizeof(bmp_t)) // On lit le fichier source
    {
        perror("Error read src");
        return EXIT_FAILURE;
    }
    printf("%s une bitmap !\n"
           "La taille du fichier est : %i.\n"
           "La résolution de l'image est %ix%i.\n"
           "%s compressé !\n",
           bitmap->file_type[0] == 'B' && bitmap->file_type[1] == 'M' ? "C'est" : "Ce n'est pas",
           bitmap->file_size,
           bitmap->width, bitmap->height,
           bitmap->compression ? "C'est" : "Ce n'est pas");

    close(fichier); // On ferme le fichier.
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int p[2];
    int p_red[2];
    int p_green[2];
    int p_blue[2];
    bmp_t bitmap;
    if (pipe(p) == -1 || pipe(p_red) == -1 || pipe(p_green) == -1 || pipe(p_blue) == -1) // On ouvre crée les pipe.
    {
        fprintf(stderr, "Erreur lors de l'ouverture du tube.\n)");
        return EXIT_FAILURE;
    }
    if (argc != 3)
    {
        fprintf(stderr, "Il faut fournir deux arguments, le fichier d'origine et celui de destination !\n"); 
        return EXIT_FAILURE;
    }
    if (file_exists(argv[1]))
    {
        printf("Le fichier existe !\n");
        if (reading_header(argv[1], &bitmap) == EXIT_FAILURE)
        {
            return EXIT_FAILURE;
        }
    }
    else
    {
        perror("Error no file");
        return EXIT_FAILURE;
    }
    pid_t ret = fork();
    if (ret == -1)
    {
        perror("Création impossible !");
        return EXIT_FAILURE;
    }
    else if (ret == 0)
    {
        // On est dans le fils gris
        char offset[10];
        sprintf(offset, "%i", bitmap.bitmap_offset);
        char nomTube0[10];
        sprintf(nomTube0, "%i", p[0]);
        char nomTube1[10];
        sprintf(nomTube1, "%i", p[1]);
        sleep(2);
        execl("./covering", "covering", nomTube0, nomTube1, offset, "N", argv[2], NULL);
        perror("Error execl");
        return EXIT_FAILURE;
    }
    else
    {
        pid_t ret2 = fork();
        if (ret2 == -1)
        {
            perror("Création impossible !");
            return EXIT_FAILURE;
        }
        else if (ret2 == 0)
        {
            // On est dans le fils rouge
            char offset[10];
            sprintf(offset, "%i", bitmap.bitmap_offset);
            char nomTube0[10];
            sprintf(nomTube0, "%i", p_red[0]);
            char nomTube1[10];
            sprintf(nomTube1, "%i", p_red[1]);
            sleep(2);
            execl("./covering", "covering", nomTube0, nomTube1, offset, "R", argv[2], NULL);
            perror("Error execl");
            return EXIT_FAILURE;
        }
        else
        {
            pid_t ret3 = fork();
            if (ret3 == -1)
            {
                perror("Création impossible !");
                return EXIT_FAILURE;
            }
            else if (ret3 == 0)
            {
                // On est dans le fils vert
                char offset[10];
                sprintf(offset, "%i", bitmap.bitmap_offset);
                char nomTube0[10];
                sprintf(nomTube0, "%i", p_green[0]);
                char nomTube1[10];
                sprintf(nomTube1, "%i", p_green[1]);
                sleep(2);
                execl("./covering", "covering", nomTube0, nomTube1, offset, "G", argv[2], NULL);
                perror("Error execl");
                return EXIT_FAILURE;
            }
            else
            {
                pid_t ret4 = fork();
                if (ret4 == -1)
                {
                    perror("Création impossible !");
                    return EXIT_FAILURE;
                }
                else if (ret4 == 0)
                {
                    // On est dans le fils bleu
                    char offset[10];
                    sprintf(offset, "%i", bitmap.bitmap_offset);
                    char nomTube0[10];
                    sprintf(nomTube0, "%i", p_blue[0]);
                    char nomTube1[10];
                    sprintf(nomTube1, "%i", p_blue[1]);
                    sleep(2);
                    execl("./covering", "covering", nomTube0, nomTube1, offset, "B", argv[2], NULL);
                    perror("Error execl");
                    return EXIT_FAILURE;
                }
                else
                {
                    close(p[0]);
                    close(p_red[0]);
                    close(p_green[0]);
                    close(p_blue[0]);
                    // On est dans le père
                    if (bitmap.bits_per_pixel == 24)
                    {
                        // On ouvre le fichier à recopier.
                        int fichier = open(argv[1], O_RDONLY);
                        if (fichier == -1)
                        {
                            perror("Error open src");
                            return EXIT_FAILURE;
                        }

                        // On ouvre/crée le fichier cible du recopiage.
                        int dst = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                        if (dst == -1)
                        {
                            perror("Error open dst");
                            return EXIT_FAILURE;
                        }
                        // On effectue le recopiage.
                        char temp_char;
                        while (read(fichier, &temp_char, sizeof(char)) > 0)
                        {
                            if (write(dst, &temp_char, sizeof(char)) == -1 || write(p[1], &temp_char, sizeof(char)) == -1 || write(p_red[1], &temp_char, sizeof(char)) == -1 || write(p_green[1], &temp_char, sizeof(char)) == -1 || write(p_blue[1], &temp_char, sizeof(char)) == -1)
                            {
                                perror("Error write pipe");
                                return EXIT_FAILURE;
                            }
                        }
                        close(p[1]);
                        close(p_red[1]);
                        close(p_green[1]);
                        close(p_blue[1]);

                        // On ferme les fichiers.
                        close(fichier);
                        close(dst);
                        return EXIT_SUCCESS;
                    }
                    else
                    {
                        printf("Il n'y a pas 24 bits par pixel, traitement impossible.");
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}