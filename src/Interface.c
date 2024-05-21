#include "Interface.h"

void Creation_Point(FILE *fichier, float latitude, float longitude, char *name)
{
    fprintf(fichier, "\t \t{\n");
    fprintf(fichier, "\t \t \t\"type\": \"Feature\",\n");
    fprintf(fichier, "\t \t \t\"geometry\": {\n");
    fprintf(fichier, "\t \t \t \t\"type\": \"Point\",\n");
    fprintf(fichier, "\t \t \t \t\"coordinates\": [%f, %f ]\n", latitude, longitude);
    fprintf(fichier, "\t \t \t},\n");
    fprintf(fichier, "\t \t \t\"properties\": {\n");
    fprintf(fichier, "\t \t \t \t\"_umap_options\": {\n");
    fprintf(fichier, "\t \t \t \t \t\"color\": \"#2196F3\",\n");
    fprintf(fichier, "\t \t \t \t \t\"weight\": 6,\n");
    fprintf(fichier, "\t \t \t \t \t\"iconClass\": \"Drop\",\n");
    fprintf(fichier, "\t \t \t \t \t\"showLabel\": null\n");
    fprintf(fichier, "\t \t \t \t},\n");
    fprintf(fichier, "\t \t \t \t\"name\": \"%s\"\n", name);
    fprintf(fichier, "\t \t \t}\n");
    fprintf(fichier, "\t \t},\n");
}

void Creation_LineString(FILE *fichier, float latitude_1, float longitude_1, float latitude_2, float longitude_2)
{
    fprintf(fichier, "\t \t{\n");
    fprintf(fichier, "\t \t \t\"type\": \"Feature\",\n");
    fprintf(fichier, "\t \t \t\"geometry\": {\n");
    fprintf(fichier, "\t \t \t \t\"type\": \"LineString\",\n");
    fprintf(fichier, "\t \t \t \t\"coordinates\": [ \n");
    fprintf(fichier, "\t \t \t \t \t[%f, %f],\n", latitude_1, longitude_1);
    fprintf(fichier, "\t \t \t \t \t[%f, %f]\n", latitude_2, longitude_2);
    fprintf(fichier, "\t \t \t \t \t]\n");
    fprintf(fichier, "\t \t \t},\n");
    fprintf(fichier, "\t \t \t\"properties\": {\n");
    fprintf(fichier, "\t \t \t \t\"_umap_options\": {\n");
    fprintf(fichier, "\t \t \t \t \t\"color\": \"#9C27B0\",\n");
    fprintf(fichier, "\t \t \t \t \t\"weight\": 6,\n");
    fprintf(fichier, "\t \t \t \t \t\"showLabel\": null\n");
    fprintf(fichier, "\t \t \t \t},\n");
    fprintf(fichier, "\t \t \t \t\"name\": \"Path\"\n");
    fprintf(fichier, "\t \t \t}\n");
    fprintf(fichier, "\t \t},\n");
}

int Creation_geojson(Path *path, int size)
{
    int start = path->list->sentinel.next->value;
    int end = path->list->sentinel.prev->value;
    // Création de 2 tableaux afin d'avoir accès aux coordonnéees
    float *lati = calloc(size, sizeof(float));
    float *longi = calloc(size, sizeof(float));

    FILE *coor = fopen("../TPF_Donnees/Data/laval_inter.txt", "r");
    if (coor == NULL)
    {
        printf("Problème dans l'ouverture du fichier de coordonées\n");
        return EXIT_FAILURE;
    }
    int nb_noeuds = 0;
    fscanf(coor, "%d", &nb_noeuds);
    for (int i = 0; i < size; i++)
    {
        fscanf(coor, "%f %f", &lati[i], &longi[i]);
    }
    // Pour les différents tests
    float latitude_1 = 0, latitude_2 = 0, longitude_1 = 0, longitude_2 = 0;
    // Ouverture du fichier + vérification qu'il est bien ouvert
    FILE *fichier = fopen("test_geojson.geojson", "w");
    if (fichier == NULL)
    {
        printf("Erreur avec la création du fichier");
        free(lati);
        free(longi);
        return EXIT_FAILURE;
    }

    // On écrit dans le fichier
    fprintf(fichier, "{\n");
    fprintf(fichier, "\t\"type\": \"FeatureCollection\",\n");
    fprintf(fichier, "\t\"features\": [\n");
    int nb_iter = 0;
    // ça crée en boucle des lignes entre 2 points
    for (ListIntNode *i = path->list->sentinel.next; i->value != end; i = i->next)
    {
        // Pour récupérer les coordonées du point actuel et du suivant
        // (Pour pouvoir tracer la ligne)
        latitude_1 = lati[i->value];
        longitude_1 = longi[i->value];
        latitude_2 = lati[i->next->value];
        longitude_2 = longi[i->next->value];

        // Le tracé de la ligne entre le point actuel et le suivant
        Creation_LineString(fichier, latitude_1, longitude_1, latitude_2, longitude_2);
        nb_iter++;
    }
    printf("%d\n", nb_iter);
    // Le tracé du point de début grâce à la fonction ;-)
    latitude_1 = lati[start];
    longitude_1 = longi[start];
    char *name = calloc(10, sizeof(char));
    strcpy(name, "Start");
    Creation_Point(fichier, latitude_1, longitude_1, name);

    // Le dernier point, afin qu'il n'est pas de virgule pour dire que c'est le dernier
    latitude_1 = lati[end];
    longitude_1 = longi[end];
    fprintf(fichier, "\t \t{\n");
    fprintf(fichier, "\t \t \t\"type\": \"Feature\",\n");
    fprintf(fichier, "\t \t \t\"geometry\": {\n");
    fprintf(fichier, "\t \t \t \t\"type\": \"Point\",\n");
    fprintf(fichier, "\t \t \t \t\"coordinates\": [%f, %f ]\n", latitude_1, longitude_1);
    fprintf(fichier, "\t \t \t},\n");
    fprintf(fichier, "\t \t \t\"properties\": {\n");
    fprintf(fichier, "\t \t \t \t\"_umap_options\": {\n");
    fprintf(fichier, "\t \t \t \t \t\"color\": \"#C91B0E\",\n");
    fprintf(fichier, "\t \t \t \t \t\"weight\": 6,\n");
    fprintf(fichier, "\t \t \t \t \t\"iconClass\": \"Drop\",\n");
    fprintf(fichier, "\t \t \t \t \t\"showLabel\": null\n");
    fprintf(fichier, "\t \t \t \t},\n");
    fprintf(fichier, "\t \t \t \t\"name\": \"End\"\n");
    fprintf(fichier, "\t \t \t}\n");
    // Juste ici :-)
    fprintf(fichier, "\t \t}\n");

    // La fermeture du fichier après toutes les features
    fprintf(fichier, "\t]\n");
    fprintf(fichier, "}\n");
    free(name);
    free(lati);
    free(longi);
    fclose(fichier);
    return EXIT_SUCCESS;
}