/*******************************************************************
 * @file Players.h
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Header file of the players class module.
 *******************************************************************/

#ifndef PLAYERS_H
#define PLAYERS_H

#include "Robot.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <assert.h>
#include "data.h"

using namespace std;

// Rol of each player
enum PLAYERS_POSITION // for court rol identification
{
    GOALIE,
    DEFENSE,
    MIDFIELDER,
    SHOOTER
};

class Players : public Robot
{
public:
    Players();
    ~Players();

    void start(string playerNumber);
    void moveMotors();
    setPoint_t goToBall(Vector2 oppositeGoal, Vector2 ballPosition, float proportional);
    setPoint_t kickBallLogic(Vector2 oppositeGoal, Vector2 ballPosition);

    PLAYERS_POSITION fieldRol;
    void toEnablePlayer(void);
    void dissablePlayer(void);

private:
    bool enablePlayer;
};

#endif // PLAYERS_H