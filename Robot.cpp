/*******************************************************************
 * @file Robot.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Source file of the Robot class module. Setters and getters
 * of robots ingame values.
 *******************************************************************/
#include "Robot.h"

Robot::~Robot()
{
}

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
void Robot::setSetpoint(setPoint_t setpoint)
{
    this->setpoint = setpoint;
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

setPoint_t Robot::getSetPoint()
{
    return setpoint; //TODO :Cambiar esto, no podemos actualizarlo con la posición  
    /*setpoint.coord = {position.x, position.z};
    setpoint.rotation = rotation.y;
	return (setpoint);*/
}

/**
 * @brief: enables robot for playing
 */
void Robot::toEnablePlayer(void)
{
    enablePlayer = true;
}

/**
 * @brief: dissables robot for playing
 */
void Robot::dissablePlayer(void)
{
    enablePlayer = false;
}