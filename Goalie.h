#ifndef GOALIE_H
#define GOALIE_H
#include <iostream>
#include <string>
#include <cstring>

#include "Players.h"

using namespace std;

class Goalie : public Players 
{
public:
    Goalie();
    ~Goalie();
    virtual void update(inGameData_t * gameData);
    void save(Vector3 ballPosition, Vector3 ballVelocity);

    //TESTING

private:
    
};

#endif // GOALIE_H