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

	if (isCloseTo({position.x, position.z}, nearBall, 0.15f))
	{
		if (isCloseTo({position.x, position.z}, ballPosition, 0.1f))
			return {100, 100, 100}; // reference value designed for shooting command
		else
			return goToBall(objectivePosition, ballPosition, 1.0f);
	}
	else
	{
		setPoint_t result = goToBall(objectivePosition, ballPosition, 1.05f);
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


void Players::shooterReposition (inGameData_t data)  //capaz esta plagada de errores de punteros...
{
	vector<Vector2> midPoints;
	for(auto bot1 : (data.oppTeam))   //obtengo los puntos medios entre enemigos
	{
		for(auto bot2 : (data.oppTeam))
		{
			if(bot2->robotID > bot1->robotID)
			{
				Vector3 bot1pos = bot1->getPosition();
				Vector3 bot2pos = bot2->getPosition();
				Vector2 midPoint = proportionalPosition({bot1pos.x, bot1pos.z},
					{bot2pos.x, bot2pos.z}, 0.5);
				midPoints.push_back(midPoint);
			}
		}
	}

	float maxDist = 0;
	Vector2 idealMidpoint = {0,0};
	float minDistance = 9.0;
	
	while(midPoints.empty())  //mira todos los puntos medios entre robots
	{
		Vector2 point = midPoints.back();
		for(auto bot : data.oppTeam)  //mira distancias entre enemigo y pnto medio
		{
			Vector3 botPos = bot->getPosition();
			float distBotGoal = botPos.x - data.oppGoal.x;  //solo enemigos en el area contraria desde el pto penal
			bool valid = (distBotGoal > -3 && distBotGoal < 3) ? true : false ;
			if(valid)
			{
				float distance = distanceOfCoords(point, {botPos.x,botPos.z});
				if(distance < minDistance)   //selecciona al punto medio mas "comodo"
				{
					minDistance = distance;
					idealMidpoint = point;
				}			
			}
		}
		midPoints.pop_back();
	}
	Vector3 idealSetPoint = {idealMidpoint.x, 0, idealMidpoint.y};
	this->setPosition(idealSetPoint);
}

void Players::secondShooterReposition (inGameData_t &data)  //capaz esta plagada de errores de punteros...
{
	vector<Vector2> midPoints;
	vector <Robot *> nearGoalEnemies;
	for(auto enemy : (data.oppTeam))
	{
		Vector3 enemyPos = enemy->getPosition();
		float distEnemyGoal = enemyPos.x - data.oppGoal.x;  //solo enemigos "cerca" del arco contrario
		if(distEnemyGoal < 2.5 && distEnemyGoal > -2.5)
		{
			nearGoalEnemies.push_back(enemy);
		}
	}
	for(auto bot1 : nearGoalEnemies)   //obtengo los puntos medios entre enemigos
	{
		Vector3 bot1pos = bot1->getPosition();
		for(auto bot2 : nearGoalEnemies)
		{
			if(bot2->robotID > bot1->robotID)
			{
				Vector3 bot2pos = bot2->getPosition();
				Vector2 midPoint = proportionalPosition({bot1pos.x, bot1pos.z},
					{bot2pos.x, bot2pos.z}, 0.5);
				midPoints.push_back(midPoint);
			}
		}
		Vector2 firstCorner = {data.oppGoal.x, 3}; //calculo con una esquina
		Vector2 firstCornerMidPoint = proportionalPosition({bot1pos.x, bot1pos.z}, firstCorner, 0.5);
		midPoints.push_back(firstCornerMidPoint);
		Vector2 secondCorner = {data.oppGoal.x, -3}; //calculo con otra esquina
		Vector2 secondCornerMidPoint = proportionalPosition({bot1pos.x, bot1pos.z}, secondCorner, 0.5);
		midPoints.push_back(secondCornerMidPoint);
	}

	float maxDist = 0;
	Vector2 idealMidpoint = {0,0};
	float minDistance = 9.0;
	
	while(midPoints.empty())  //mira todos los puntos medios entre robots
	{
		Vector2 point = midPoints.back();
		for(auto bot : nearGoalEnemies)  //mira distancias entre enemigo y pnto medio
		{
			Vector3 botPos = bot->getPosition();
			float distance = distanceOfCoords(point, {botPos.x,botPos.z});
			if(distance < minDistance)   //selecciona al punto medio mas "comodo"
			{
				minDistance = distance;
				idealMidpoint = point;
			}			
		}

		float distance = distanceOfCoords(point, {data.oppGoal.x,3});
		if(distance < minDistance)   //selecciona al punto medio mas "comodo"
		{
			minDistance = distance;
			idealMidpoint = point;
		}	

		distance = distanceOfCoords(point, {data.oppGoal.x,-3});
		if(distance < minDistance)   //selecciona al punto medio mas "comodo"
		{
			minDistance = distance;
			idealMidpoint = point;
		}	

		midPoints.pop_back();
	}
	Vector3 idealSetPoint = {idealMidpoint.x, 0, idealMidpoint.y};
	this->setPosition(idealSetPoint);
}


/**
 * @brief 
 * 
 * @param objectivePlayer 
 * @param gameData 
 */
void Players::pass(Players objectivePlayer, inGameData_t &gameData)
{
	Vector2 ballPosition = {gameData.ballPosition.x, gameData.ballPosition.z};
	Vector2 objectivePlayerPosition = {objectivePlayer.getPosition().x, objectivePlayer.getPosition().z};
	
	// fijasrse si hay jugadores en el medio, devuelvo false o la tiro por arriba?
	if(!checkForInterception(gameData.oppTeam, objectivePlayerPosition))
	{
		kickBallLogic(objectivePlayerPosition, ballPosition);
	}
}

/**
 * @brief checks if a pass is possible
 * 
 * @param oppTeam 
 * @return true if possible
 * @return false if not
 */
bool Players::checkForInterception(vector<Robot*> &oppTeam, Vector2 objective)
{
	bool possible = true;
	Vector2 myPosition = {position.x, position.z};

	for (auto bot : oppTeam)
	{
		Vector2 botPosition = {bot->getPosition().x, bot->getPosition().z};
		if (!betweenTwoLines(myPosition, objective, botPosition, 0.20f))  	// me fijo si algun robot esta
		{															// en el pasillo donde voy a hacer el pase
			possible = false;
		}
	}

	return possible;
}