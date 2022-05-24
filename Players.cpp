/*******************************************************************
 * @file Players.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Source file of the players class module.
 *******************************************************************/
#include "Players.h"

using namespace std;

Players::Players()
{
}

Players::~Players()
{
}

/*
 * @brief: assigns an identification number to the robot for simplifying the messagge recognition
 * @param playerNumber: string containing the indentification
 */
void Players::start(string playerNumber)
{
	// depende de la entrega
	robotID = playerNumber.c_str();
}

/**
 * @brief calculates a court setpoint for the player
 *
 * @param oppositeGoal: opposites team goal ~ where to shoot
 * @param ballPosition: actual ball�s position ~ point of reference
 * @param proportional: proportional value for position calculation
 *
 * @return destination: position in court and players rotation for shooting
 */
setPoint_t Players::goToBall(Vector2 oppositeGoal, Vector2 ballPosition, float proportional)
{
	setPoint_t destination;

	destination.coord = proportionalPosition(oppositeGoal, ballPosition, proportional);
	destination.rotation = calculateRotation(oppositeGoal, ballPosition);

	return destination;
}

/**
 * @brief non blocking logic for player reposition and shooting.
 * If the player is too far, it aproaches to the ball, if its
 * quite near it gets even closer, and if its in the right
 * position, it shoots.
 *
 * @param oppositeGoal: opposites team goal ~ where to shoot
 * @param ballPosition: actual ball�s position ~ point of reference
 *
 * @return result: position in court and players rotation for shooting
 */
setPoint_t Players::kickBallLogic(Vector2 oppositeGoal, Vector2 ballPosition)
{
	Vector2 nearBall = proportionalPosition(oppositeGoal, ballPosition, 1.05f);

	if (isCloseTo({position.x, position.z}, nearBall, 0.15f))
	{
		if (isCloseTo({position.x, position.z}, ballPosition, 0.1f))
			return {100, 100, 100}; // reference value designed for shooting command
		else
			return goToBall(oppositeGoal, ballPosition, 1.0f);
	}
	else
	{
		setPoint_t result = goToBall(oppositeGoal, ballPosition, 1.05f);
		if (sameLine({position.x, position.z}, result.coord, ballPosition))
		{
			result.coord.x += 0.3;
			if (((position.z * ballPosition.y) < 0) && (position.z > 0))
				result.coord.y += 0.3;
			else if (((position.z * ballPosition.y) < 0) && (position.z < 0))
				result.coord.y -= 0.3;
			else if (((position.z * ballPosition.y) > 0) && (position.z > 0))
				result.coord.y -= 0.3;
			else
				result.coord.y += 0.3;
		}
		return result;
	}
}

/*
 * brief: enables robot for playing
 */
void Players::toEnablePlayer(void)
{
	enablePlayer = true;
}

/*
 * brief: dissables robot for playing
 */
void Players::dissablePlayer(void)
{
	enablePlayer = false;
}
