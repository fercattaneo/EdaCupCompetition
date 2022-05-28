#include "Goalie.h"

using namespace std;

Goalie::Goalie()
{
}

Goalie::~Goalie()
{
}

void Goalie::update(inGameData_t * gameData)
{
	save(*(gameData->ballPosition), *(gameData->ballVelocity));
}


//TESTING
/***/
 void Goalie::save(Vector3 ballPosition, Vector3 ballVelocity)
 {
    float deltaX = position.x - ballPosition.x;
    
    //si la norma es mayor a tanto, no calculo

    float alpha = myGoal.x < 0 ? (oppAttacker.rotation - 90) : (270 - oppAttacker.rotation);


    Vector2 destination = proportionalPosition(myGoal, {ballPosition.x, ballPosition.z} ,(0.5 / 9) );
    destination.y = ballPosition.z - tan(alpha) * (deltaX);
 }