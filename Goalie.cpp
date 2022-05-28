#include "Goalie.h"

using namespace std;

Goalie::Goalie()
{
}

Goalie::~Goalie()
{
}

void Goalie::update(inGameData_t  &gameData)
{
	save(gameData.ballPosition, gameData.ballVelocity, gameData.myGoal);
}


//TESTING
/***/
 void Goalie::save(Vector3 ballPosition, Vector3 ballVelocity, Vector2 myGoal)
 {
    float deltaX = position.x - ballPosition.x;
    float alpha = myGoal.x < 0 ? (calculateRotation({ 0,0 }, {ballVelocity.x,ballVelocity.z}) - 90) : (270 - calculateRotation({ 0,0 }, { ballVelocity.x,ballVelocity.z }));

    Vector2 destination = proportionalPosition(myGoal, {ballPosition.x, ballPosition.z} ,(0.5 / 9) );
    destination.y = ballPosition.z - tan(alpha) * (deltaX);
 }