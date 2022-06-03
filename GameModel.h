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
#include "Players.h"
#include "data.h"
#include "proximityMap.h"

using namespace std;

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

enum BALL_POSESSION
{
    MY_TEAM,
    OPP_TEAM,
    FREE_BALL,
    DISPUTED_BALL
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

    inGameData_t dataPassing;
    GameState gameState;
    BALL_POSESSION posession;
    int robotWithBall;
    string teamID;
    string oppTeamID;

    vector<Players *> team;

    void update(inGameData_t &dataPassing);
    void updateFuturePos(inGameData_t& dataPassing);
    void updatePositions();
    void assignMessagePayload(string topic, vector<char> &payload);

    void setSetpoint(setPoint_t setpoint, int robotID);

    void voltageKickerChipper(string robotID);
    void setDribbler(string robotID, bool onOff);
    void setChipper(string robotID);
    void setKicker(string robotID, float power);

    Vector2 getProxPosBall2D(Vector3 ballPosition, Vector3 ballVelocity);
    bool isBallStill(void);
    void searchFreeBall();
    void analizePosession();
    void checkForCollision(Vector2 actualPos, setPoint_t &setpoint);
    string getTeamID();

    /*TESTING*/
    void shoot(Players *player, Vector2 objectivePosition);
    bool checkForInterception(vector<Vector3> &oppTeam, Vector2 objective, Vector2 teamPosition);
    void dribbleTo(Players &player);

};

#endif //_GAMEMODEL_H