/*******************************************************************
 * @file Players.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Source file of the players class module.
 *******************************************************************/
#include "Players.h"
#define MODULE(x) (((x) > 0)?(x):(-(x)))

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
		case GOALIE:
			save(gameData);
			break;
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

void Players::shooterReposition(inGameData_t &data)  //capaz esta plagada de errores de punteros...
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
	idealSetPoint.rotation = calculateRotation(idealSetPoint.coord, {data.ballPosition.x,data.ballPosition.z});
	this->setSetpoint(idealSetPoint);
}

void Players::secondShooterReposition(inGameData_t& data)  //capaz esta plagada de errores de punteros...
{
	vector<Vector2> midPoints;
	for (auto bot1 : data.oppTeamPositions)   //obtengo los puntos medios entre enemigos
	{
		for (auto bot2 : data.oppTeamPositions)
		{
			float distanceBetweenEnemies = distanceOfCoords({bot1.x,bot1.z}, {bot2.x,bot2.z});
			if(distanceBetweenEnemies > 0.2)
			{
				Vector2 midPoint = proportionalPosition({ bot1.x, bot1.z },
					{ bot2.x, bot2.z }, 0.5);
				if(MODULE((data.oppGoal.x - midPoint.x)) > 0.2)
				{
					midPoints.push_back(midPoint);
				}
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

	while (!midPoints.empty())  //mira todos los puntos medios entre robots
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
	idealSetPoint.coord ={ idealMidpoint.x, idealMidpoint.y };
	idealSetPoint.rotation = calculateRotation(idealSetPoint.coord, {data.ballPosition.x,data.ballPosition.z});
	this->setSetpoint(idealSetPoint);
}

/**
 * @brief 
 * 
 * @param gameData 
 */
void Players::save(inGameData_t &gameData)
{
	//float deltaX = position.x - gameData.ballPosition.x;
	//float alpha = (gameData.myGoal.x < 0) ? (calculateRotation({ 0,0 },
	//	 { gameData.ballVelocity.x, gameData.ballVelocity.z }) - 90) :
	//	 (270 - calculateRotation({ 0,0 }, { gameData.ballVelocity.x,gameData.ballVelocity.z }));

	float prop = (distanceOfCoords(gameData.myGoal, {gameData.ballPosition.x, gameData.ballPosition.z}) < 2) ? 0.5 : 0.2;
	Vector2 destination = proportionalPosition(gameData.myGoal,
		{ gameData.ballPosition.x, gameData.ballPosition.z }, prop );
	//destination.y = gameData.ballPosition.z - (tan(alpha) * (deltaX));
	Vector2 ballVel = {gameData.ballVelocity.x , gameData.ballVelocity.z};
	if(MODULE(ballVel.x) > 0.1)
	{
		float shootingPower = gameData.myGoal.x / ballVel.x;
		if(shootingPower > 0 && MODULE(ballVel.y * shootingPower) < 0.3)
			destination.y = ballVel.y * shootingPower;
	}

	float alpha = calculateRotation({position.x, position.z}, {gameData.ballPosition.x, gameData.ballPosition.z});
	cout << "GOALIE: " << destination.x << " , " << destination.y << endl;
   	setSetpoint({ destination,(alpha - 180)});
}