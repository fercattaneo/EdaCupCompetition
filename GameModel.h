/*******************************************************************
 * @file GameModel.h
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Header file of the GameModel class module.
 *******************************************************************/
#ifndef _GAMEMODEL_H
#define _GAMEMODEL_H

#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include "MQTTClient2.h"
#include "data.h"

#include "Team/Players.h"
#include "Team/Goalie.h"
#include "Team/Defense.h"
#include "Team/Midfielder.h"
#include "Team/Shooter.h"

using namespace std;

#define LENGTH_OF_COURT_X 9f // meters
#define LENGTH_OF_COURT_Z 6f

#define EARTH__GRAVITY 9.80665f

enum GameState
{
    START,
    PRE_KICKOFF,
    KICKOFF,
    IN_GAME,
    PRE_FREEKICK,
    FREEKICK,
    PRE_PENALTY,
    PENALTY,
    PAUSE,
    CONTINUE,
    REMOVE_ROBOT,
    ADD_ROBOT,
    ENDED_GAME,
    GOAL
};

enum BallPosession
{
    MY_TEAM,
    OPP_TEAM,
    FREE_BALL
};

class GameModel : public MQTTListener
{
public:
    GameModel(MQTTClient2 &mqttClient, string myTeam);
    ~GameModel();

    void onMessage(string topic, vector<char> payload);

    void start(void);
    void suscribeToGameTopics();
    void setDisplay(string path, string robotID);
    void addPlayer(Players *bot);
    void removePlayer(Players *bot);

private:
    MQTTClient2 *mqttClient;
    vector<MQTTMessage> messagesToSend;

    GameState gameState;
    BallPosession attackingTeam;
    string teamID;
    string oppTeamID;

    float ball[12];
    vector<Players *> team;
    vector<Robot *> oppTeam;
    Vector2 arcoTeam;
    Vector2 arcoOpposite;

    void update();
    void updatePositions();
    void verifyPossesion();
    void assignMessagePayload(string topic, vector<char> &payload);
    void robotMessagesToSend(Robot* robot);

    void setSetpoint(setPoint_t setpoint, string robotID);

    void shootToGoal(Players *player);

    void voltageKickerChipper(string robotID);
    void setDribbler(string robotID);
    void setChipper(string robotID);
    void setKicker(string robotID);

    coord_t getProxPosBall2D(Vector3 ballPosition, Vector3 ballVelocity);
    bool isBallStill(void);

    string getTeamID();
};

#endif //_GAMEMODEL_H