/*******************************************************************
 * @file data.h
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Header file of the data module. Module prepared for
 * multiple data types and usefull functions calculus.
 *******************************************************************/
#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <vector>
#include "Robot.h"
#include <cstring>
#include <cmath>
#include <assert.h>

#define MODULE(x) (((x) > 0)?(x):(-(x)))

std::vector<char> getDataFromFloat(float data);
std::vector<char> getArrayFromSetPoint(setPoint_t setpoint);
float getFloat(vector<char> vec);
Vector2 proportionalPosition(Vector2 originPos, Vector2 finalPos, float proportion);
float calculateRotation(Vector2 originPos, Vector2 finalPos);
bool isCloseTo(Vector2 originCoord, Vector2 destinationCoord, float nearRange);
bool sameLine(Vector2 originPos, Vector2 finalPos, Vector2 mediumPos);
bool betweenTwoLines(Vector2 originPos, Vector2 finalPos, Vector2 point, float displacement);
float distanceOfCoords(Vector2 firstCoord, Vector2 secondCoord);

#endif //  DATA_H