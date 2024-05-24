#include "SpatialHashing.h"

Point *CreatePoint(float latitude, float longitude)
{
    Point *point = (Point *)malloc(sizeof(Point));
    point->latitude = latitude;
    point->longitude = longitude;
    return point;
}

Point *destroyPoint(Point *point)
{
    if (point != NULL)
    {
        free(point);
    }
    return NULL;
}

Point *NearestPointFromCSV(char *filename, int code_postal)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Unable to open file");
        return NULL;
    }

    char line[1024];
    int found = 0;
    Point *result = NULL;

    // Skip la première ligne
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file))
    {
        int postal_code = 0;
        float latitude = 0.0f;
        float longitude = 0.0f;
        char *token;

        // Va directement au code postal
        token = strtok(line, ","); // code_commune_INSEE
        token = strtok(NULL, ","); // nom_commune_postal
        token = strtok(NULL, ","); // code_postal

        if (token)
        {
            postal_code = atoi(token);
        }

        if (postal_code == code_postal)
        {
            // Parse latitude and longitude
            for (int i = 0; i < 2; i++)
            {
                token = strtok(NULL, ","); // Skip to latitude field
            }

            if (token)
            {
                latitude = atof(token);
                token = strtok(NULL, ","); // longitude
                if (token)
                {
                    longitude = atof(token);
                }
            }

            // Create and return the point
            result = CreatePoint(longitude, latitude);
            found = 1;
            break;
        }
    }

    fclose(file);
    if (!found)
    {
        printf("Postal code not found.\n");
    }
    return result;
}

float Haversine(Point *point1, Point *point2)
{
    float lat1 = point1->latitude;
    float lon1 = point1->longitude;
    float lat2 = point2->latitude;
    float lon2 = point2->longitude;

    float dLat = (lat2 - lat1) * M_PI / 180.0;
    float dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = (lat1)*M_PI / 180.0;
    lat2 = (lat2)*M_PI / 180.0;

    float a = sin(dLat / 2) * sin(dLat / 2) + sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    float distanceInKilometers = 6371 * c;
    return distanceInKilometers * 1000; // mètres
}

int CoordToGraphIndex(Point *point, char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Unable to open file\n");
        return EXIT_FAILURE;
    }

    int closest_node = -1;
    int NbNodes;
    fscanf(file, "%d", &NbNodes);
    float min_distance = INFINITY;
    for (int i = 0; i < NbNodes; i++)
    {

        float latitude, longitude;
        fscanf(file, "%f %f", &latitude, &longitude);
        Point *node = CreatePoint(latitude, longitude);
        float distance = Haversine(point, node);
        if (distance < min_distance)
        {
            min_distance = distance;
            closest_node = i;
        }
        destroyPoint(node);
    }
    printf("Closest node: %d\n", closest_node);
    printf("Distance: %f\n", min_distance);
    fclose(file);
    return closest_node;
}