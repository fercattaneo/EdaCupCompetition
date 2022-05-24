/*******************************************************************
 * @file Robot.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Source file of the Robot class module. Setters and getters
 * of robots ingame values.
 *******************************************************************/
#include "Robot.h"

/* SETTERS */
void Robot::setPosition(Vector3 pos)
{
    position = pos;
}

void Robot::setRotation(Vector3 newRot)
{
    rotation = newRot;
}

void Robot::setSpeed(Vector3 newSpeed)
{
    speed = newSpeed;
}

void Robot::setAngularSpeed(Vector3 newAngSpeed)
{
    angularSpeed = newAngSpeed;
}

void Robot::setPowerLevels(Vector3 powerValues)
{
    powerLevel = powerValues;
}

void Robot::setDribbler(float volts)
{
    dribbler = volts;
}

void Robot::setKicker(float value)
{
    kicker = value;
}

void Robot::setChipper(float value)
{
    chipper = value;
}

void Robot::updateSetPoint()
{
    setpointInCourt.coord = {position.x,position.z};
    setpointInCourt.rotation = rotation.y;
}

/* GETTERS */
Vector3 Robot::getPosition()
{
    return position;
}

Vector3 Robot::getRotation()
{
    return rotation;
}

Vector3 Robot::getSpeed()
{
    return speed;
}

Vector3 Robot::getAngularSpeed()
{
    return angularSpeed;
}

float Robot::getKickerCharge()
{
    return powerLevel.z;
}

float Robot::getDribbler()
{
    return dribbler;
}
float Robot::getKicker()
{
    return kicker;
}
float Robot::getChipper()
{
    return chipper;
}

setPoint_t Robot::getRobotSetPoint()
{
    return setpointInCourt;
}