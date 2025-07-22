#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_TYPE_LENGTH 12
#define SENSOR_DESCONECTED -1.0000

float generate_measurement(const char *type);
float calculate_distance(int x1, int y1, int x2, int y2);
void validate_arguments(int argc, char **argv);

#endif // SENSOR_UTILS_H
