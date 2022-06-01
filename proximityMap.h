#ifndef HEAT
#define HEAT

#include <iostream>
#include <algorithm>
#include <array>
#include <vector>
#include <list>

#include "Players.h"

using namespace std;

#define partition 0.1 //tamano de cada casilla, unidad en metros
#define rows 60       // rows = 6 / particion
#define cols 90       // cols = 9 / particion
#define acc  10       //  accomodation = 1 / partition

typedef struct 
{
    char type = 0; //0 nothing, 1 ally, 2 ball, 3 enemy
    char value;//aca podriamos poner la velocidad del bot o hacia donde va. Pd: nigger
    int x;
    int y;

}BotInfo;

struct sNode
{
    bool beenVisited;                       
   
    float globalGoal;
    float localGoal;
    sNode *parent;

    vector<sNode*> neighbours;
    BotInfo botInfo;
};

class proximityMap
{
    public:

        void start();
        void update(vector<Players *> allyTeam);
        void update(vector<Players *> allyTeam, Vector2 ball);
        void update(vector<Players *> allyTeam, Vector2 ball, vector<Players *> enemyTeam);
        
        void solveAStar(Vector2 initPos, Vector2 endPos);

        /*testing functs*/
        void printhm();
        void printParen();
        void shortest(Vector2 initPost, Vector2 endPos);

    private:

        array<array<sNode,rows>,cols> pMap;

        void restart();
};

#endif