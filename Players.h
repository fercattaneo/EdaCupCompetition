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

#define LENGTH_OF_COURT_X 9.0f // meters
#define LENGTH_OF_COURT_Z 6.0f

typedef struct
{
    Vector2 myGoal;                     
    Vector2 oppGoal;               
    Vector3 ballPosition;
    Vector3 ballVelocity;
    vector<Vector3> teamPositions;
    vector<Vector3> teamFuturePositions;
    vector<Vector3> oppTeamPositions;
    vector<Vector3> oppTeamSpeed;
    vector<Vector3> oppTeamFuturePos;
} inGameData_t;

// Role of each player
enum PLAYERS_POSITION // for court role identification
{
    GOALIE,
    DEFENSE,
    DEFENSE2,
    MIDFIELDER,
    SHOOTER,
    SHOOTER2
};

class Players : public Robot
{
public:
    Players();
    ~Players();

    void update(inGameData_t gameData);
    void start(int playerNumber);
    setPoint_t goToBall(Vector2 objectivePosition, Vector2 ballPosition, float proportional);
    setPoint_t kickBallLogic(Vector2 objectivePosition, Vector2 ballPosition);
    Vector2 openZPlace (float dist, inGameData_t &data, Vector2 point0, Vector2 point1, Vector2 vertix);
    
    int fieldRol;
    void toEnablePlayer(void);
    void dissablePlayer(void);

private:
    bool enablePlayer;
    
    // GOALIE
    void save(inGameData_t &gameData);

    // DEFENSE
    void defendGoal(inGameData_t &data, float goalZpoint);
    
    // MIDFIELDER
    void midfielderReposition(inGameData_t& data);
    void defensiveMidfielder (inGameData_t &data);

    // SHOOTERS
    void shooterReposition(inGameData_t &data);
    void secondShooterReposition (inGameData_t &data);

    
};

#endif // PLAYERS_H