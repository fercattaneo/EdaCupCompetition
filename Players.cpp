/*******************************************************************
 * @file Players.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Source file of the players class module.
 *******************************************************************/
#include "Players.h"
#include "raymath.h"

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
			defendGoal(gameData, 0.5);
			break;
		case DEFENSE2:
			defendGoal(gameData, -0.5);
			break;
		case MIDFIELDER:
			midfielderReposition(gameData);
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
 * @param ballPosition: actual ball's position ~ point of reference
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
			float distance = distanceOfCoords(point, { data.oppTeamPositions[bot].x,data.oppTeamPositions[bot].z });
			if (distance < minDistance)   //selecciona al punto medio mas "comodo"
			{
				minDistance = distance;
			}
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
 * @brief Arquerea el arquero
 * 
 * @param gameData 
 */
void Players::save(inGameData_t &gameData)
{ //prop starts as the proportional between the goal and the ball position
	
	Vector2 ballVel = Vector2Normalize({gameData.ballVelocity.x , gameData.ballVelocity.z});
	Vector2 destination;

	float possibleYAxis = 0.0f;
	if(ballVel.x != 0)
		possibleYAxis = (ballVel.y / ballVel.x)  * (gameData.myGoal.x - gameData.ballPosition.x)+ gameData.ballPosition.z;

	if(ballVel.x/gameData.myGoal.x > 0.0 && MODULE(possibleYAxis) < 0.5f) //if the ball is in a possible direction that could go to the goal
	{
		destination = { (1.0f / 9.0f) * distanceOfCoords({gameData.ballPosition.x, gameData.ballPosition.z}, gameData.myGoal), possibleYAxis };
		destination.x += gameData.myGoal.x;
		// setSetpoint({ destination,(0)});
		// float alpha = calculateRotation({position.x, position.z}, {gameData.ballPosition.x, 
		// 														   gameData.ballPosition.z});
		// setSetpoint({ destination,(alpha)});
	}
	else
	{ 
		float prop = (distanceOfCoords(gameData.myGoal, {gameData.ballPosition.x, gameData.ballPosition.z}) < 2) ? 0.5 : 0.2;

		destination = proportionalPosition(gameData.myGoal,
													{gameData.ballPosition.x, 
													gameData.ballPosition.z}, 
													prop );
			
		// Vector2 ballVel = {gameData.ballVelocity.x , gameData.ballVelocity.z};
		/*if(MODULE(ballVel.x) > 0.33)
		{
			float shootingPower = gameData.myGoal.x / ballVel.x;
			if(shootingPower > 0 && MODULE(ballVel.y * shootingPower) < 0.3)
				destination.y = ballVel.y * shootingPower;
		}*/

		//TESTING
		if(MODULE(gameData.myGoal.x - gameData.ballPosition.x) < 0.8 &&
			MODULE(gameData.myGoal.x -  gameData.ballPosition.x) < 1.5)
		{
			destination = {gameData.ballPosition.x, gameData.ballPosition.z};
		}
		//END TESTING
	}

	float alpha = calculateRotation({gameData.ballPosition.x, gameData.ballPosition.z},
									{position.x, position.z});
	setSetpoint({ destination, alpha});

}

/////////////
// TESTING //
/////////////

/**
* @brief: sets a court position for the defense
* @param data: information from the court
* @param goalZpoint: location in z axis for defensive reference to protect
* Builds an fictional line between the defensive point and the ball
* and selects a position in the line to go to.
*/
void Players::defendGoal(inGameData_t &data, float goalZpoint)
{
	setPoint_t destination;
	float lengthProp = 0.6;
	
	if(MODULE(data.myGoal.x - data.ballPosition.x) > 4.5)
		lengthProp = 0.35;
		
	float angleProp = 2 - ( MODULE(data.myGoal.x - data.ballPosition.x) / 6);
	if(angleProp < 1.0)
		angleProp = 1.0;
	if(MODULE(data.myGoal.x - data.ballPosition.x) < 2)
		goalZpoint /= angleProp;
		
	destination.coord = proportionalPosition({data.myGoal.x, (data.myGoal.y - goalZpoint)},
		{data.ballPosition.x, data.ballPosition.z}, lengthProp);//va al pto medio entre la pelota  y el pto del arco
	if(distanceOfCoords({data.myGoal.x, data.myGoal.y}, {data.ballPosition.x, data.ballPosition.z}) < 1)
	{
		destination.coord = proportionalPosition({data.myGoal.x, goalZpoint},  //va hacia la pelota
			{data.ballPosition.x, data.ballPosition.z}, 1);
		//TODO: set dribbler
	}
	destination.rotation = calculateRotation({ position.x, position.z },
		{ data.ballPosition.x, data.ballPosition.z });
	setSetpoint(destination);
}

/*Midfielder: Entre los puntos de penal, el "mejor punto posible"
* Cuando los otros tienen la pelota : que sea un defensor más
* Hacemos una proporcional para qye se mueva en z según la pelota, en un rango de x - 1.5, 1.5
* buscamos los pasillos posibles con saltos de 10 o 5 cm
* Si no encuentra pasillo para dar pase, podria devolver false para que los defensores se pasen la pelota
*/
void Players::midfielderReposition(inGameData_t &data)
{
	float distance = 100;
	int botToAnalyse = 3;

	for (int bot = 1; bot < 6; bot++)
	{
		if (bot != 3)
		{
			float botDistance = 0;
			botDistance = distanceOfCoords({ data.ballPosition.x, data.ballPosition.z }, 
									       { data.teamPositions[bot].x,data.teamPositions[bot].y });
						
			if(botDistance < distance)
			{
				distance = botDistance;
				botToAnalyse = bot;
			}
		}
	}

	// empezar a analizar los pasillos con saltos de 10 cm
	// tenemos que preguntar tmb si no hay robot en un radio de 1 metro o algo así
	float proportionalX = (1.5 / MODULE(data.oppGoal.x)) * data.ballPosition.x;
	for (auto enemyBot : data.oppTeamPositions)
	{
		for (int pasillo = 0; pasillo < (LENGTH_OF_COURT_Z / 0.1f); pasillo++)
		{
			Vector2 teamPosition = { data.teamPositions[botToAnalyse].x,data.teamPositions[botToAnalyse].z };
			float positionZ = (pasillo % 2) ? (pasillo * LENGTH_OF_COURT_Z / 0.1f) : 
											  - (pasillo * LENGTH_OF_COURT_Z / 0.1f);
			Vector2 possibleCoords = { proportionalX,  positionZ};  
			if (!betweenTwoLines(teamPosition, possibleCoords, {enemyBot.x, enemyBot.z}, 1.0f)) // TODO: debugear, no funciona betweenT
			{
				float rotation = calculateRotation(teamPosition, possibleCoords);
				setSetpoint({ possibleCoords, rotation });
				
				return;
			}
		}
	}
}