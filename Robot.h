/*******************************************************************
 * @file Robot.h
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Header file of the Robot class module.
 *******************************************************************/
#ifndef ROBOT_H
#define ROBOT_H

#include "raylib.h"
#include <string>

using namespace std;

typedef struct // court coordenates
{
    float x;
    float z;
} coord_t;

typedef struct // robot position and rotation
{
    Vector2 coord;
    float rotation;
} setPoint_t;

class Robot
{
public:
    ~Robot();

    string teamID;
    int robotID;

    Vector3 getPosition();
    Vector3 getRotation();
    Vector3 getSpeed();
    Vector3 getAngularSpeed();
    float getKickerCharge();
    setPoint_t getSetPoint(void);

    void setPosition(Vector3 newPos);
    void setRotation(Vector3 newRot);
    void setSpeed(Vector3 newSpeed);
    void setAngularSpeed(Vector3 newAngSpeed);
    void setPowerLevels(Vector3 powerValues);
    void setSetpoint(setPoint_t setpoint);

protected:
    Vector3 position;
    Vector3 rotation;
    Vector3 speed;
    Vector3 angularSpeed;
    Vector3 powerLevel; // x for consume, y for battery level, z for kicker charge
    setPoint_t setpoint;
};

#endif