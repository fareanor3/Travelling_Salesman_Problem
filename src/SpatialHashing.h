#pragma once
#include "Settings.h"

#include "Graph.h"
#include "ShortestPath.h"

typedef struct
{
    float latitude;
    float longitude;
} Point;

Point *CreatePoint(float latitude, float longitude);
Point *destroyPoint(Point *point);

/// @brief
/// @param filename
/// @param code_postal
/// @return les coordonnées GPS du code postal
Point *NearestPointFromCSV(char *filename, int code_postal);

/// @brief Utilise Haversine formula pour calculer la distance entre deux points
/// @param point Du Code postal
/// @param filename Le fichier txt des points du graphe
/// @return l'id du node du graph le plus proche du point
int CoordToGraphIndex(Point *point, char *filename);

float Haversine(Point *point1, Point *point2);