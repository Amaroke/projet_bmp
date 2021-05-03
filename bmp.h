#ifndef BMP_H
#define BMP_H

// On utilise __attribute__((__packed__)) pour que ce soit de la bonne taille, autrement file_size s'écrit directement sur "le segment de mémoire suivant" laissant "un trou" de deux octets.

typedef struct __attribute__((__packed__)) bmp_t
{
    char file_type[2];    /*!< Identification du format. */
    int file_size;        /*!< Taille du fichier. */
    int reserved;         /*!< Champ réserve. */
    int bitmap_offset;    /*!< Offset de l'image. */
    int header_size;      /*!< Taille de l'entête (en octets). */
    int width;            /*!< Largeur de l'image (en pixels). */
    int height;           /*!< Hauteur de l'image (en pixels). */
    short planes;         /*!< Nombre de plans utilisés. */
    short bits_per_pixel; /*!< Nombre de bits par pixel. */
    int compression;      /*!< Présence ou non de compression. */
} bmp_t;

#endif