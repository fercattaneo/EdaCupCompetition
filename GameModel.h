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
    //Despues de un gol se presiona pause
    START, //saque del centro
    PRE_KICKOFF, //jugadores del lado de cancha correspondiente
    KICKOFF, //saque inical y puedo patear de una al alrco bajar la vel a 1,5 m/s
    PRE_FREEKICK, //Posicion de tiro libre
    FREEKICK, //Tiro libre bajar la vel a 1,5 m/s
    PRE_PENALTY, //Posicion de penal
    PENALTY, //Penal
    PAUSE, //no se juega y hay que bajar la vel a 1,5 m/s y 0,5 mts de la pelota 
    CONTINUE, //nuestro modo de jugar
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
    int msjteam;

    vector<Players *> team;
    void updateGameConditions(inGameData_t& dataPassing);
    void update(inGameData_t &dataPassing);
    void updateFuturePos(inGameData_t& dataPassing);
    void updatePositions();
    void assignMessagePayload(string topic, vector<char> &payload);

    void setSetpoint(setPoint_t setpoint, int robotID);

    void voltageKickerChipper(string robotID);
    void setDribbler(string robotID, bool onOff);
    void setChipper(string robotID);
    void setKicker(string robotID, float power);

    void shoot(Players *player, Vector2 objectivePosition);
    Vector2 getProxPosBall2D(Vector3 ballPosition, Vector3 ballVelocity);
    bool isBallStill(void);
    void searchFreeBall();
    void analyzePosession();
    void checkForCollision(Vector2 actualPos, setPoint_t &setpoint);
    string getTeamID();

    int analyzePass(Players &player);
    Vector2 analyzeShoot(Players &player);
    bool isInCourt(Vector3 param);
    void initialPositions();
    void freekickPositions();
    void penaltyPositions();

    /*TESTING*/
    bool checkForInterception(vector<Vector3> &oppTeam, Vector2 objective, Vector2 teamPosition); 
    void dribbleTo(Players &player);
    bool checkPlayingBall();

};

#endif //_GAMEMODEL_H