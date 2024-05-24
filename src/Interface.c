#include "Interface.h"

void Creation_Point(FILE *fichier, float latitude, float longitude, char *name, bool fin)
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
    if (fin)
    {
        fprintf(fichier, "\t \t}\n");
    }
    else
    {
        fprintf(fichier, "\t \t},\n");
    }
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
    fprintf(fichier, "\t \t \t \t \t\"color\": \"#130E0A\",\n");
    fprintf(fichier, "\t \t \t \t \t\"weight\": 6,\n");
    fprintf(fichier, "\t \t \t \t \t\"showLabel\": null\n");
    fprintf(fichier, "\t \t \t \t},\n");
    fprintf(fichier, "\t \t \t \t\"name\": \"Path\"\n");
    fprintf(fichier, "\t \t \t}\n");
    fprintf(fichier, "\t \t},\n");
}

int Creation_geojson(Path *path, char *fichierCoord, int *tabSubToGraph, PathMatrix *matrix)
{
    FILE *coor = fopen(fichierCoord, "r");
    if (coor == NULL)
    {
        printf("Problème dans l'ouverture du fichier de coordonées\n");
        return EXIT_FAILURE;
    }

    int nb_noeuds = 0;
    fscanf(coor, "%d", &nb_noeuds);

    float *lati = calloc(nb_noeuds, sizeof(float));
    float *longi = calloc(nb_noeuds, sizeof(float));
    for (int i = 0; i < nb_noeuds; i++)
    {
        fscanf(coor, "%f %f", &lati[i], &longi[i]);
    }
    fclose(coor);

    FILE *fichier = fopen("test_geojson.geojson", "w");
    if (fichier == NULL)
    {
        printf("Erreur avec la création du fichier");
        free(lati);
        free(longi);
        return EXIT_FAILURE;
    }

    fprintf(fichier, "{\n");
    fprintf(fichier, "\t\"type\": \"FeatureCollection\",\n");
    fprintf(fichier, "\t\"features\": [\n");
    // initialisation du i en dehors de la boucle pour pouvoir le free après
    ListIntIter *i = ListIntIter_create(path->list);

    for (; ListIntIter_isValid(i) && i->current->next != i->sentinel; ListIntIter_next(i)) // on est dans le path entre i et i->next
    {
        // initialisation du j en dehors de la boucle pour pouvoir le free après, c'est le path à l'intersection de i et j (i et j sont des noeuds).
        ListIntIter *j = ListIntIter_create(matrix->matrix[i->current->value][i->current->next->value]->list);
        for (; ListIntIter_isValid(j) && j->current->next != j->sentinel; ListIntIter_next(j)) // on est dans le path entre j  et j->next
        {
            int next = j->current->next->value;
            Creation_LineString(fichier, lati[j->current->value], longi[j->current->value], lati[next], longi[next]);
        }
        ListIntIter_destroy(j);
        bool fin = false;
        Creation_Point(fichier, lati[tabSubToGraph[i->current->value]], longi[tabSubToGraph[i->current->value]], "Intersection", fin);
    }
    float longitude = longi[tabSubToGraph[path->list->sentinel.next->value]];
    float latitude = lati[tabSubToGraph[path->list->sentinel.next->value]];
    char *name = calloc(10, sizeof(char));
    strcpy(name, "Start/End");
    Creation_Point(fichier, latitude, longitude, name, 1);
    fprintf(fichier, "\t]\n");
    fprintf(fichier, "}\n");
    fclose(fichier);
    ListIntIter_destroy(i);
    free(lati);
    free(longi);
    free(name);
    return EXIT_SUCCESS;
}