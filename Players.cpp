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

/**
 * @brief: assigns an identification number to the robot for simplifying the messagge recognition
 * @param playerNumber: string containing the indentification
 */
void Players::start(int playerNumber)
{
	robotID = playerNumber;
	setSetpoint({ 20,20,20 });
	if(playerNumber == 5)
	{
		setSetpoint({ -1.5,0,0 });
	}
	//setSetpoint({ -4.5,0,0 });
}

/**
 * @brief
 *
 * @param gameData
 */
void Players::update(inGameData_t gameData)
{
	switch (fieldRol)   //POR AHORA REPOSICIONES NOMAS
	{
	case DEFENSE:
		//shooterReposition(gameData);
		break;
	case DEFENSE2:
		//shooterReposition(gameData);
		break;
	case MIDFIELDER:
		//shooterReposition(gameData);
		break;
	case SHOOTER:
		shooterReposition(gameData);
		break;
	case SHOOTER2:
		secondShooterReposition(gameData);
		break;
	default: break;
	}

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
setPoint_t Players::goToBall(Vector2 objectivePosition, Vector2 ballPosition, float proportional)
{
	setPoint_t destination;

	destination.coord = proportionalPosition(objectivePosition, ballPosition, proportional);
	destination.rotation = calculateRotation(objectivePosition, ballPosition);

	return destination;
}

/**
 * @brief non blocking logic for player reposition and shooting.
 * If the player is too far, it aproaches to the ball, if its
 * quite near it gets even closer, and if its in the right
 * position, it shoots.
 *
 * @param objectivePosition: opposites team goal ~ where to shoot
 * @param ballPosition: actual ball�s position ~ point of reference
 *
 * @return result: position in court and players rotation for shooting
 */
setPoint_t Players::kickBallLogic(Vector2 objectivePosition, Vector2 ballPosition)
{
	Vector2 nearBall = proportionalPosition(objectivePosition, ballPosition, 1.05f);

	if (isCloseTo({ position.x, position.z }, nearBall, 0.15f))
	{
		if (isCloseTo({ position.x, position.z }, ballPosition, 0.1f))
			return { 100, 100, 100 }; // reference value designed for shooting command
		else
			return goToBall(objectivePosition, ballPosition, 1.0f);
	}
	else
	{
		setPoint_t result = goToBall(objectivePosition, ballPosition, 1.05f);
		if (sameLine({ position.x, position.z }, result.coord, ballPosition))
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

/**
 * @brief: enables robot for playing
 */
void Players::toEnablePlayer(void)
{
	enablePlayer = true;
}

/**
 * @brief: dissables robot for playing
 */
void Players::dissablePlayer(void)
{
	enablePlayer = false;
}


//TESTING


void Players::shooterReposition(inGameData_t data)  //capaz esta plagada de errores de punteros...
{
	vector<Vector2> midPoints;
	for (int bot1 = 0; bot1 < 6; bot1++)   //obtengo los puntos medios entre enemigos
	{
		for (int bot2 = bot1+1; bot2 < 6; bot2++)
		{
			Vector2 midPoint = proportionalPosition({ data.oppTeamPositions[bot1].x, data.oppTeamPositions[bot1].z },
				{ data.oppTeamPositions[bot2].x, data.oppTeamPositions[bot2].z }, 0.5);
			midPoints.push_back(midPoint);

		}
	}

	Vector2 auxMidpoint = { 0,0 };
	Vector2 idealMidpoint = { 0,0 };
	float maxDistance = 0.0;   // distacia maxima entre todos los midpoints y todos los robots
	float minDistance = 9.0;   // distancia minima entre midpoint y todos los robots

	while (!midPoints.empty())  //mira todos los puntos medios entre robots
	{
		Vector2 point = midPoints.back();
		for (int bot = 0; bot < 6; bot++)  //mira distancias entre enemigo y pnto medio
		{
			//float distBotGoal = data.oppTeamPositions[bot].x - data.oppGoal.x;  //solo enemigos en el area contraria desde el pto penal
			//bool valid = (distBotGoal > -3 && distBotGoal < 3) ? true : false;
			//if (valid)
			//{
				float distance = distanceOfCoords(point, { data.oppTeamPositions[bot].x,data.oppTeamPositions[bot].z });
				if (distance < minDistance)   //selecciona al punto medio mas "comodo"
				{
					minDistance = distance;
				}
			//}
		}
		if(minDistance > maxDistance)
		{
			maxDistance = minDistance;
			idealMidpoint = point; //marco pos ideal
		}
		minDistance = 9.0;
		midPoints.pop_back();
	}

	setPoint_t idealSetPoint; 
	idealSetPoint.coord ={ idealMidpoint.x, idealMidpoint.y };
	idealSetPoint.rotation = 0;
	this->setSetpoint(idealSetPoint);
}

void Players::secondShooterReposition(inGameData_t& data)  //capaz esta plagada de errores de punteros...
{
	vector<Vector2> midPoints;
	// vector<Vector3> nearGoalEnemies;
	// for (int enemy = 0; enemy < 6; enemy++)
	// {
	// 	float distEnemyGoal = data.oppTeamPositions[enemy].x - data.oppGoal.x;  //solo enemigos "cerca" del arco contrario
	// 	if (distEnemyGoal < 2.5 && distEnemyGoal > -2.5)
	// 	{
	// 		nearGoalEnemies.push_back(data.oppTeamPositions[enemy]);
	// 	}
	// }
	for (auto bot1 : data.oppTeamPositions)   //obtengo los puntos medios entre enemigos
	{
		for (auto bot2 : data.oppTeamPositions)
		{
			float distanceBetweenEnemies = distanceOfCoords({bot1.x,bot1.z}, {bot2.x,bot2.z});
			if(distanceBetweenEnemies > 0.2)
			{
				Vector2 midPoint = proportionalPosition({ bot1.x, bot1.z },
					{ bot2.x, bot2.z }, 0.5);
				midPoints.push_back(midPoint);
			}
		}
		Vector2 firstCorner = { data.oppGoal.x, 3 }; //calculo con una esquina
		Vector2 firstCornerMidPoint = proportionalPosition({ bot1.x, bot1.z}, firstCorner, 0.5);
		midPoints.push_back(firstCornerMidPoint);
		Vector2 secondCorner = { data.oppGoal.x, -3 }; //calculo con otra esquina
		Vector2 secondCornerMidPoint = proportionalPosition({ bot1.x, bot1.z }, secondCorner, 0.5);
		midPoints.push_back(secondCornerMidPoint);
	}

	float maxDist = 0;
	Vector2 idealMidpoint = { 0,0 };
	float minDistance = 9.0;

	while (midPoints.empty())  //mira todos los puntos medios entre robots
	{
		Vector2 point = midPoints.back();
		for (auto bot : data.oppTeamPositions)  //mira distancias entre enemigo y pnto medio
		{
			float distance = distanceOfCoords(point, { bot.x,bot.z });
			if (distance < minDistance)   //selecciona al punto medio mas "comodo"
				minDistance = distance;
		}

		float distance = distanceOfCoords(point, { data.oppGoal.x,3 });
		if (distance < minDistance)   //selecciona al punto medio mas "comodo"
			minDistance = distance;

		distance = distanceOfCoords(point, { data.oppGoal.x,-3 });
		if (distance < minDistance)   //selecciona al punto medio mas "comodo"
			minDistance = distance;

		if(minDistance > maxDist)
		{
			maxDist = minDistance;
			idealMidpoint = point;
		}
		minDistance = 9.0;
		midPoints.pop_back();
	}
	setPoint_t idealSetPoint; 
	cout << "2nd shooter: " << idealMidpoint.x << " , " << idealMidpoint.y << endl;
	idealSetPoint.coord ={ idealMidpoint.x, idealMidpoint.y };
	idealSetPoint.rotation = 0;
	this->setSetpoint(idealSetPoint);
}


// /**
//  * @brief
//  *
//  * @param objectivePlayer
//  * @param gameData
//  */
// void Players::pass(Vector3 objectivePlayer, inGameData_t& gameData)
// {
// 	Vector2 ballPosition = { gameData.ballPosition.x, gameData.ballPosition.z };
// 	Vector2 objectivePlayerPosition = { objectivePlayer.x, objectivePlayer.z };

// 	setPoint_t proxPlaceInCourt;
// 	setPoint_t kickValue = { 100, 100, 100 };

// 	// fijasrse si hay jugadores en el medio, devuelvo false o la tiro por arriba?
// 	if (!checkForInterception(gameData.oppTeamPositions, objectivePlayerPosition))
// 	{
// 		proxPlaceInCourt = kickBallLogic(objectivePlayerPosition, ballPosition);
		
// 		// if ((proxPlaceInCourt.coord.x == kickValue.coord.x) &&
// 		// 	(proxPlaceInCourt.coord.y == kickValue.coord.y) &&
// 		// 	(proxPlaceInCourt.rotation == kickValue.rotation)) 		// comparacion de igualdad de setpoints
// 		// {
// 		// 	if (getKickerCharge() >= 160)
// 		// 		// setKicker(to_string(player->robotID));
// 		// 	// else
// 		// 		// voltageKickerChipper(to_string(player->robotID)); 	// este orden por el pop_back del vector
// 		// }
// 		// else // mover hasta el setpoint indicado
// 		// {
// 		// 	// setpoint = 
// 		// }
// 	}
// }


