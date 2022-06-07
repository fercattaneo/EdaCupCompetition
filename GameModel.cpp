/*******************************************************************
 * @file GameModel.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief In this file are the definitions of the methods of GameModel class.
 *******************************************************************/

#include "GameModel.h"
#include "data.h"

 /**
  * @brief constructor of GameModel class
  *
  * @param mqttClient pointer to the MQTTClient, allows GameModel to read and send messages
  * @param myTeam ally team's name
  */
GameModel::GameModel(MQTTClient2& mqttClient, string myTeam)
{
	this->mqttClient = &mqttClient;
	teamID = myTeam;
	oppTeamID = (myTeam == "1") ? "2" : "1";
	Vector2 arco1 = { -4.5f, 0.0f };
	Vector2 arco2 = { 4.5f, 0.0f };
	dataPassing.myGoal = (myTeam == "1") ? arco1 : arco2;
	dataPassing.oppGoal = (myTeam == "1") ? arco2 : arco1;
	dataPassing.ballPosition = { 0,0,0 };
	dataPassing.ballVelocity = { 0,0,0 };
	dataPassing.availableRobots = 6;
	gameState = START;

	float PIDparameters[6] = { 4, 0, 8, 0, 0, 100 };
	vector<char> payloadPID(6 * sizeof(float));
	memcpy(&payloadPID[0], PIDparameters, 6 * sizeof(float));

	for (auto bot : team)
	{
		this->mqttClient->publish("robot" + myTeam + "." + to_string(bot->robotID) +
			"/pid/parameters/set", payloadPID);
	}
}

/**
 *	@brief GameModel Destructor. clears data members
 *
 */
GameModel::~GameModel()
{
	team.clear();
	dataPassing.oppTeamPositions.clear();
}

/**
 * @brief this function recives a message from MQTTclient and analyzes the topic
 *
 * @param topic: string declaration of messagge's topic
 * @param payload: data of the topic
 *
 */
void GameModel::onMessage(string topic, vector<char> payload)
{
	if (!topic.compare("ball/motion/state")) // compare returns 0 if are equal
	{
		float ball[12];
		memcpy(ball, &payload[0], payload.size());
		dataPassing.ballPosition = { ball[0], ball[1], ball[2] };
		dataPassing.ballVelocity = { ball[3], ball[4], ball[5] };

		updateGameConditions(dataPassing);

		while (!messagesToSend.empty()) // sends all messages appended to message vector
		{
			MQTTMessage actualMessage = messagesToSend.back();
			mqttClient->publish(actualMessage.topic, actualMessage.payload);
			messagesToSend.pop_back();
		}
	}
	else
	{
		assignMessagePayload(topic, payload);
	}
}

/**
 * @brief initializes players from the ally team
 *
 */
void GameModel::start()
{
	for (int playerNumber = 0; playerNumber < team.size(); playerNumber++)
	{
		team[playerNumber]->start(playerNumber);
		team[playerNumber]->fieldRol = playerNumber;
	}
	dataPassing.availableRobots = 6;
	dataPassing.teamPositions.resize(6); 		//non empty value for vectors
	dataPassing.teamFuturePositions.resize(6);
	dataPassing.oppTeamPositions.resize(6);
	dataPassing.oppTeamFuturePos.resize(6);
	dataPassing.oppTeamSpeed.resize(6);
}


/**
 * @brief subscribes to topics of interest
 */
void GameModel::suscribeToGameTopics()
{
	mqttClient->subscribe("ball/motion/state");
	mqttClient->subscribe("edacup/preKickOff");
	mqttClient->subscribe("edacup/kickOff");
	mqttClient->subscribe("edacup/preFreeKick");
	mqttClient->subscribe("edacup/freeKick");
	mqttClient->subscribe("edacup/prePenaltyKick");
	mqttClient->subscribe("edacup/penaltyKick");
	mqttClient->subscribe("edacup/pause");
	mqttClient->subscribe("edacup/continue");
	mqttClient->subscribe("edacup/removeRobot");
	mqttClient->subscribe("edacup/addRobot");
}

/**
 * @brief prepares to shoot and shoots
 *
 * @param player pointer to the player willing to shoot
 * @param objectivePosition coordinate to shoot
 * @param kickChip 1 for kicker, 0 for chipper
 */
void GameModel::shoot(Players* player, Vector2 objectivePosition, bool kickChip)
{
	static int flag = 0;
	Vector3 pos = player->getPosition();
	Vector2 playerPosition = { pos.x, pos.z };
	Vector2 ballLocation = { dataPassing.ballPosition.x, dataPassing.ballPosition.z };
	float rotation = calculateRotation(playerPosition, { dataPassing.ballPosition.x,
										dataPassing.ballPosition.z });

	voltageKickerChipper(to_string(player->robotID + 1));

	if (isCloseTo(playerPosition, ballLocation, 0.1f) && flag == 5)
	{
		rotation = calculateRotation(playerPosition, objectivePosition);
		float botRealRot = player->getRotation().y;
		rotation += (rotation < 0) ? 360 : 0;
		botRealRot += (botRealRot < 0) ? 360 : 0;

		if (MODULE(botRealRot - rotation) < 5.0)
		{
			setDribbler(to_string(player->robotID + 1), false);

			float distance = distanceOfCoords(playerPosition, objectivePosition);
			float power;

			if ((objectivePosition.x == dataPassing.oppGoal.x) || distance >= 4.0f)	// Shoot to goal
				power = 0.8;
			else
			{
				power = distance / 5;
				power = (power <= 0.3) ? 0.3 : power;
			}				// pass 
			if (kickChip)
				setKicker(to_string(player->robotID + 1), power);
			else
				setChipper(to_string(player->robotID + 1), power);
			flag = 0;
		}
	}
	else if (isCloseTo(playerPosition, ballLocation, 0.11f) && flag < 5)
	{
		setDribbler(to_string(player->robotID + 1), true);
		flag++;
	}

	setSetpoint({ ballLocation, rotation }, player->robotID);
}

/**
 * @brief checks if an enemy player is between the player willing to make pass and the objective
 *
 * @param oppPositions
 * @param objective
 * @param Position
 *
 * @return true if pass is possible
 * @return false if not
 */
bool GameModel::checkForInterception(vector<Vector3>& oppPositions,
	Vector2 objective, Vector2 Position)
{
	for (int bot = 0; bot < 6; bot++)
	{
		Vector2 botPosition = { oppPositions[bot].x, oppPositions[bot].z };
		if (betweenTwoLines(Position, objective, botPosition, 0.20f))  	// near enemies
		{
			return false;
		}
	}
	return true;
}

/**
 * @brief
 *
 * @param setpoint
 */
void GameModel::checkForCollision(Vector2 actualPos, setPoint_t& setpoint, int robotID)
{
	float proxTime = 0.2;

	for (int bot = 0; bot < 6; bot++)  // Check collision with opp
	{
		Vector3 positionOpp = dataPassing.oppTeamFuturePos[bot];
		if (betweenTwoLines(actualPos, setpoint.coord, { positionOpp.x, positionOpp.z }, 0.24f))
		{
			Vector3 vel = dataPassing.oppTeamSpeed[bot];
			float signX;
			float signZ;
			if (MODULE(vel.x) < 0.05)
				signX = 1;
			else
				signX = (vel.x / MODULE(vel.x));
			if (MODULE(vel.z) < 0.05)
				signZ = 1;
			else
				signZ = (vel.z / MODULE(vel.z));
			setpoint.coord = { dataPassing.oppTeamFuturePos[bot].x - (signX * 0.2f),
				dataPassing.oppTeamFuturePos[bot].z - (signZ * 0.2f) };
			break;
		}
	}

	for (int bot = 0; bot < 6; bot++)	// Check collision with 
	{
		Vector3 robotPosition = dataPassing.teamPositions[bot];
		if (betweenTwoLines(actualPos, setpoint.coord, { robotPosition.x, robotPosition.z }, 0.24f))
		{
			Vector3 vel = dataPassing.teamFuturePositions[bot];
			float signX;
			float signZ;
			if (MODULE(vel.x) < 0.05)
				signX = 1;
			else
				signX = (vel.x / MODULE(vel.x));
			if (MODULE(vel.z) < 0.05)
				signZ = 1;
			else
				signZ = (vel.z / MODULE(vel.z));
			setpoint.coord = { dataPassing.oppTeamFuturePos[bot].x - (signX * 0.2f),
				dataPassing.oppTeamFuturePos[bot].z - (signZ * 0.2f) };
			setSetpoint({ { dataPassing.oppTeamFuturePos[bot].x + (signX * 0.2f),
				dataPassing.oppTeamFuturePos[bot].z + (signZ * 0.2f)}, 0 }, bot);
			break;
		}
	}
}

/**
 * @brief sets a player to loook for the ball
 *
 * @return index of player searching the ball
 */
int GameModel::searchFreeBall()
{
	setPoint_t destination;
	if (dataPassing.ballPosition.y > 0.1)
		destination.coord = getProxPosBall2D(dataPassing.ballPosition, dataPassing.ballVelocity);
	else
	{
		float proxTime = 0.2;
		Vector2 futurePosition;
		futurePosition.x = dataPassing.ballPosition.x + (dataPassing.ballVelocity.x * proxTime);
		futurePosition.y = dataPassing.ballPosition.z + (dataPassing.ballVelocity.z * proxTime);
		destination.coord = futurePosition;
	}

	float maxCloseness = 9.0;
	int index = 3;
	for (auto ally : team)
	{
		Vector3 pos = ally->getPosition();
		float closeness = distanceOfCoords({ pos.x, pos.z },
			{ destination.coord.x, destination.coord.y });
		if (closeness < maxCloseness)
		{
			maxCloseness = closeness;
			index = ally->robotID;
		}
	}
	destination.rotation = calculateRotation({ destination.coord.x, destination.coord.y },
		{ dataPassing.ballPosition.x, dataPassing.ballPosition.z });
	setSetpoint(destination, index);
	return index;
}

/**
 * @brief
 *
 *
 */
void GameModel::analyzePosession()
{
	static int cycles = 0;
	posession = FREE_BALL;
	for (auto enemy : dataPassing.oppTeamPositions)
	{
		bool nearness = isCloseTo({ enemy.x, enemy.z },
			{ dataPassing.ballPosition.x, dataPassing.ballPosition.z }, 0.12);
		if (nearness)
		{
			posession = OPP_TEAM;

			// if (MODULE(dataPassing.ballVelocity.x) < 0.05 && 
			// MODULE(dataPassing.ballVelocity.z) < 0.05)
			// {
			//  	cycles++;
			//  	if (cycles > 15)
			// 	{
			//  		posession = FREE_BALL;
			//  		cycles = 0;
			//  	}
			// }
			break;
		}
	}
	for (auto ally : team)
	{
		Vector3 posOfAlly = ally->getPosition();
		if (isCloseTo({ posOfAlly.x, posOfAlly.z }, { dataPassing.ballPosition.x,
													 dataPassing.ballPosition.z }, 0.12))
		{
			posession = MY_TEAM;
			robotWithBall = ally->robotID;
			break;
		}
	}
}

/**
 * @brief
 *
 *
 */
bool GameModel::isInCourt(Vector3 param)
{
	if (param.x > LENGTH_OF_COURT_X / 2 ||
		param.x < -LENGTH_OF_COURT_X / 2 ||
		param.z > LENGTH_OF_COURT_Z / 2 ||
		param.z < -LENGTH_OF_COURT_Z / 2)
		return false;
	else
		return true;
}

/**
 * @brief
 *
 *
 */
void GameModel::initialPositions()
{
	float sign = (dataPassing.myGoal.x) / (MODULE(dataPassing.myGoal.x));
	float rot = calculateRotation(dataPassing.myGoal, { 0,0 });
	setSetpoint({ {(dataPassing.myGoal.x), 0}, rot }, 0); //goalie
	if (msjteam == (teamID[0] - '0'))
	{
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.8f)),-1},rot }, 1); //def 1
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.8f)),1}, rot }, 2); //def 2
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 2.5f)), 0}, rot }, 3); //midf
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 3.5f)), -1}, rot }, 4); //att 1
		setSetpoint({ {(0 - (sign * 0.25f)), 0}, calculateRotation({0,0},
			dataPassing.myGoal) }, 5);  //att 2	
	}
	else
	{
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.8f)), -0.3},rot }, 1); //def 1
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.8f)), 0.3}, rot }, 2); //def 2
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 2.5f)), 0}, rot }, 3); //midf
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 3.8f)), -0.5}, rot }, 4); //att 1
		setSetpoint({ {(dataPassing.myGoal.x - (sign * 3.8f)), 0.5}, rot }, 5);  //att 2	
	}
}

/**
 * @brief
 *
 *
 */
void GameModel::penaltyPositions()
{
	Vector2 penPoint;

	if (msjteam == (teamID[0] - '0')) //my  shooting
	{
		penPoint = { dataPassing.oppGoal.x ,0 };
		penPoint.x += (dataPassing.oppGoal.x < 0) ? 6 : -6;
	}
	else //opp  shooting
	{
		penPoint = { dataPassing.myGoal.x ,0 };
		penPoint.x += (dataPassing.myGoal.x < 0) ? 6 : -6;
	}

	float sign = (penPoint.x) / (MODULE(penPoint.x));
	float XPoint = (penPoint.x + sign);
	float rot = calculateRotation(penPoint, dataPassing.oppGoal);
	setSetpoint({ {dataPassing.myGoal.x, 0}, rot }, 0); //goalie
	setSetpoint({ {XPoint, 0.3}, rot }, 1); //def 1
	setSetpoint({ {XPoint,	-0.3}, rot }, 2); //def 2
	setSetpoint({ {XPoint, 0.6}, rot }, 3); //midf
	setSetpoint({ {XPoint, -0.6}, rot }, 4);//att 1
	if (msjteam == (teamID[0] - '0'))
		setSetpoint({ {(penPoint.x + (sign / 3)), 0}, rot }, 5);
	else
		setSetpoint({ {XPoint, 0}, rot }, 5);
}

/**
 * @brief
 *
 *
 */
void GameModel::freekickPositions()
{
	if (msjteam == (teamID[0] - '0')) //my  shooting
	{
		float rot = calculateRotation({ dataPassing.ballPosition.x, dataPassing.ballPosition.z }, dataPassing.oppGoal);
		setPoint_t setpoint;
		setpoint.coord = { dataPassing.ballPosition.x +
			(dataPassing.myGoal.x / MODULE(dataPassing.myGoal.x)) * 0.2f,
			dataPassing.ballPosition.z };
		setpoint.rotation = rot;
		setSetpoint(setpoint, 2);
	}
	else //opp  shooting
	{
		Vector2 barrierPos = proportionalPosition(dataPassing.myGoal,
			{ dataPassing.ballPosition.x, dataPassing.ballPosition.z }, 0.5);
		if (distanceOfCoords(barrierPos, { dataPassing.ballPosition.x, dataPassing.ballPosition.z }) < 0.5)
			barrierPos.x = dataPassing.myGoal.x - (1.5 * dataPassing.myGoal.x / MODULE(dataPassing.myGoal.x));

		float rot = calculateRotation(dataPassing.myGoal, { dataPassing.ballPosition.x, dataPassing.ballPosition.z });

		setSetpoint({ {dataPassing.myGoal.x, 0}, rot }, 0); //goalie

		setSetpoint({ {barrierPos.x, barrierPos.y - 0.5f }, rot }, 1); //def 1
		setSetpoint({ {barrierPos.x, barrierPos.y - 0.25f }, rot }, 2); //def 2
		setSetpoint({ {barrierPos.x, barrierPos.y }, rot }, 3); //midf
		setSetpoint({ {barrierPos.x, barrierPos.y + 0.25f}, rot }, 4);//att 1
		setSetpoint({ {barrierPos.x, barrierPos.y + 0.5f}, rot }, 5);//att 2
	}
}

/**
* @brief checks for the optimal pass
*
* @param player which tries to make a pass
*
* @return index of robot to pass. Returns 7 if no pass is available
*/
int GameModel::analyzePass(Players& player)
{
	Vector3 pos = player.getPosition();
	bool flag;
	int aux = 7;

	for (int i = 5; i > 0; i--)
	{
		flag = true;
		if (i != player.robotID)
		{
			Vector3 ally = dataPassing.teamPositions[i];
			for (auto enemyBot : dataPassing.oppTeamPositions)
			{
				if ((enemyBot.y != 100))
				{
					if ((betweenTwoLines({ pos.x,pos.y }, { ally.x,ally.y },
						{ enemyBot.x, enemyBot.z }, 0.1f)))
					{
						flag = false; //se puede con este i
						aux = i;
						break;
					}
				}
			}
		}
		if (!flag)
			break;
	}

	return aux;
}

/**
 * @brief
 *
 *
 */
Vector2 GameModel::analyzeShoot(Players& player)
{
	Vector2 objectivePosition = player.openZPlace(0.01, dataPassing,
		{ dataPassing.oppGoal.x, dataPassing.oppGoal.y - 0.45f },
		{ dataPassing.oppGoal.x, dataPassing.oppGoal.y + 0.45f },
		{ dataPassing.ballPosition.x, dataPassing.ballPosition.x });
	if (objectivePosition.y == 0)
		objectivePosition = { 0,0 };
	Vector3 baller = player.getPosition();
	if (distanceOfCoords({ baller.x, baller.z }, dataPassing.oppGoal) < 2.7)
		objectivePosition = dataPassing.oppGoal;

	return objectivePosition;
}

/**
 * @brief waits for conditions
 *	if the opposite team is taking a freekick, penalty, or kickoff
 *
 */
bool GameModel::checkPlayingBall()
{
	if (msjteam == (oppTeamID[0] - '0'))
	{
		static float initialTime;
		static Vector2 ballInitialPos;
		float actualTime = GetTime();
		static bool valid = false;
		if (!valid)
		{
			initialTime = GetTime();
			ballInitialPos = { dataPassing.ballPosition.x, dataPassing.ballPosition.z };
			valid = true;
		}
		else
		{
			if ((distanceOfCoords(ballInitialPos, { dataPassing.ballPosition.x, dataPassing.ballPosition.z })
				>= 0.05) || ((actualTime - initialTime) >= 10))
			{
				valid = false;
				return true;
			}
		}
		return false;

	}
	else
		return true;
}

/**
 * @brief for initial kickoff or free kick
 *
 * @param player to kick
 */
void GameModel::kickonce(Players& player)
{
	setDribbler(to_string(player.robotID + 1), false);
	if (msjteam == (teamID[0] - '0'))
	{
		setDribbler(to_string(player.robotID + 1), false);
		shoot(&player, {team[3]->getPosition().x, team[3]->getPosition().z}, true);
		// Vector3 pos = player.getPosition();
		// if(distanceOfCoords({pos.x, pos.z}, {0,0}) <= 0.1)
		// 	setDribbler((to_string(player.robotID + 1)), true);
		// 	int value = analyzePass(player);
		// 	if (value != 7) {
		// 		setDribbler(to_string(player.robotID + 1), false);
		// 		shoot(&player, {team[value]->getPosition().x, team[value]->getPosition().z}, true);
		// 	}
		// else
		// 	setSetpoint({{0,0}, calculateRotation({0,0}, dataPassing.myGoal)}, player.robotID);
	}
}

/**
 * @brief positions for the players when game is paused
 *
 *
 */
void GameModel::pausePositions()
{
	for (auto player : team)
	{
		setDribbler(to_string(player->robotID + 1), false);
		Vector3 bot = player->getPosition();
		if (isCloseTo({ bot.x, bot.z }, { dataPassing.ballPosition.x, dataPassing.ballPosition.z }, 0.65))
		{
			float dist = distanceOfCoords({ bot.x, bot.z },
				{ dataPassing.ballPosition.x, dataPassing.ballPosition.z });
			float sign = dataPassing.ballPosition.x - bot.x;
			sign = sign / MODULE(sign);
			float rot = calculateRotation({ bot.x, bot.z },
				{ dataPassing.ballPosition.x, dataPassing.ballPosition.z });
			setSetpoint({ {bot.x - (sign * (1.0f - dist)), bot.z}, rot }, player->robotID);
		}

		if (!player->getInField())
			setSetpoint({{0,0}, 0}, player->robotID);
	}
}
