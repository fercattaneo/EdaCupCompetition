/*******************************************************************
 * @file GameModelUpdate.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief In this file are the definitions of the methods of GameModel class.
 *******************************************************************/

#include "GameModel.h"
#include "data.h"



/**
 * @brief analyzes the topic and delivers the payload to the desired place ¿?
 *
 * @param topic: string declaration of messagge's topic
 * @param payload: data of the topic
 */
void GameModel::assignMessagePayload(string topic, vector<char>& payload)
{
	if (topic.compare(0, 6, "edacup") == 0) // game state messagge
	{
		if (topic.compare(7, 10, "preKickOff") == 0)
		{
			gameState = PRE_KICKOFF;
		}
		else if (topic.compare(7, 7, "kickOff") == 0)
		{
			gameState = KICKOFF;
		}
		else if (topic.compare(7, 11, "preFreeKick") == 0)
		{
			gameState = PRE_FREEKICK;
		}
		else if (topic.compare(7, 8, "freeKick") == 0)
		{
			gameState = FREEKICK;
		}
		else if (topic.compare(7, 14, "prePenaltyKick") == 0)
		{
			gameState = PRE_PENALTY;
		}
		else if (topic.compare(7, 11, "penaltyKick") == 0)
		{
			gameState = PENALTY;
		}
		else if (topic.compare(7, 5, "pause") == 0)
		{
			gameState = PAUSE;
		}
		else if (topic.compare(7, 8, "continue") == 0)
		{
			gameState = CONTINUE;
		}
		else if (topic.compare(7, 11, "removeRobot") == 0)
		{
			removePlayer();
		}
		else if (topic.compare(7, 8, "addRobot") == 0)
		{
			addPlayer();
		}
		else
		{
			cout << "FAILURE: INVALID GAMESTATE MESSAGE RECIVED" << endl;
		}

		if (topic.compare(7, 5, "pause") != 0 && topic.compare(7, 8, "continue") != 0)
		{
			msjteam = payload[0];
		}
	}
	else if (topic.compare(5, 1, teamID) == 0) // the robot belongs to the team
	{
		if (topic.compare(9, 6, "motion") == 0)
		{
			int robotIndex = topic[7] - '0';

			float payloadToFloat[12];
			memcpy(payloadToFloat, &payload[0], payload.size());

			team[robotIndex - 1]->setPosition({ payloadToFloat[0], payloadToFloat[1], 
												payloadToFloat[2] });
			team[robotIndex - 1]->setSpeed({ payloadToFloat[3], payloadToFloat[4], 
												payloadToFloat[5] });
			team[robotIndex - 1]->setRotation({ payloadToFloat[6], payloadToFloat[7], 
												payloadToFloat[8] });
			team[robotIndex - 1]->setAngularSpeed({ payloadToFloat[9], payloadToFloat[10], 
												payloadToFloat[11] });

			dataPassing.teamPositions[robotIndex - 1].x = payloadToFloat[0];
			dataPassing.teamPositions[robotIndex - 1].y = payloadToFloat[1];
			dataPassing.teamPositions[robotIndex - 1].z = payloadToFloat[2];
		}
		else if (topic.compare(9, 11, "power/state") == 0)
		{
			int robotIndex = topic[7] - '0';

			float payloadToFloat[3];
			memcpy(payloadToFloat, &payload[0], std::min(payload.size(), sizeof(payloadToFloat)));

			team[robotIndex - 1]->setPowerLevels({ payloadToFloat[0], payloadToFloat[1], 	
												payloadToFloat[2] });
		}
	}
	else if (topic.compare(5, 1, oppTeamID) == 0) // the robot belongs to opposite team
	{
		if (topic.compare(9, 6, "motion") == 0)
		{
			int robotIndex = topic[7] - '0';

			float payloadToFloat[12];
			memcpy(payloadToFloat, &payload[0], payload.size());

			dataPassing.oppTeamPositions[robotIndex - 1].x = payloadToFloat[0];
			dataPassing.oppTeamPositions[robotIndex - 1].y = payloadToFloat[1];
			dataPassing.oppTeamPositions[robotIndex - 1].z = payloadToFloat[2];
			dataPassing.oppTeamSpeed[robotIndex - 1].x = payloadToFloat[3];
			dataPassing.oppTeamSpeed[robotIndex - 1].y = payloadToFloat[4];
			dataPassing.oppTeamSpeed[robotIndex - 1].z = payloadToFloat[5];
		}
	}
	else // error in the messagge recived or topic processing
	{
		cout << "FAILURE: INVALID MAIN TOPIC MESSAGE RECIVED" << endl;
	}
}

/**
 * @brief determines the current state of the game
 *
 * @param dataPassing pointer to the information that the team may need
 */
void GameModel::updateGameConditions(inGameData_t& dataPassing)
{
	switch (gameState)
	{
	case START:
		initialPositions();
		gameState = PRE_KICKOFF;
		break;
	case PRE_KICKOFF:
		initialPositions();
		break;
	case KICKOFF: 
		kickonce(*team[dataPassing.availableRobots - 1]);
	case FREEKICK: 
		if (checkPlayingBall())
		{
			gameState = CONTINUE;
		}
		break;
	case PRE_FREEKICK:
		freekickPositions(); 
		break;
	case PRE_PENALTY:
		penaltyPositions();
		break;
	case PENALTY:
		if (checkPlayingBall())
		{
			shoot(team[dataPassing.availableRobots - 1], { dataPassing.oppGoal.x, (1 * 0.45f) }, 1);
			if (MODULE(dataPassing.ballVelocity.x) > 0.5)
			{
				gameState = CONTINUE;
			}
		}
		else
			gameState = CONTINUE;
		break;
	case PAUSE:
		pausePositions();
		break;
	case CONTINUE:
		update(dataPassing);
		break;
	case REMOVE_ROBOT:
		break;
	case ADD_ROBOT:
		break;
	}
}

/**
 * @brief updates bots
 *
 * @param dataPassing pointer to the information that the bots may need
 */
void GameModel::update(inGameData_t& dataPassing)
{
	if (isInCourt(dataPassing.ballPosition)) {
		updateFuturePos(dataPassing);
		analyzePosession();
		if (posession == FREE_BALL)
		{
			int baller = searchFreeBall(); //TESTING
			for (auto player : team)
			{
				if (player->robotID != baller)
					player->update(dataPassing, 2);
			}
		}
		else
		{
			if (posession == MY_TEAM)
			{
				int nonUpdatingPlayer = 7;
				Vector2 shootGoal = analyzeShoot(*team[robotWithBall]);
				if (shootGoal.x != 0)
					shoot(team[robotWithBall], dataPassing.oppGoal, 1);
				else
				{
					int indexForPass = analyzePass(*team[robotWithBall]);
					if (indexForPass != 7) // 7 as a non valid pass
					{
						Vector3 passingPlace3D = dataPassing.teamFuturePositions[indexForPass];
						Vector2 passingPlace = { passingPlace3D.x, passingPlace3D.z };
						team[indexForPass]->setSetpoint({ passingPlace,calculateRotation(
							{dataPassing.ballPosition.x, dataPassing.ballPosition.z},
							passingPlace) });
						setDribbler(to_string(indexForPass), true);
						shoot(team[robotWithBall], passingPlace, 1);
						nonUpdatingPlayer = indexForPass;
					}
					else
					{
						shoot(team[robotWithBall],dataPassing.oppGoal, 0);
					}
				}
				for (auto player : team)
				{
					if(player->robotID != nonUpdatingPlayer)
						player->update(dataPassing, posession);
				}
			}
			else
			{
				for (auto player : team)
				{
					player->update(dataPassing, posession);
				}
			}
		}
		updatePositions();
	}
}

/**
 * @brief gets the positions the bots may be 
 *
 * @param datapassing pointer to the information to update
 */
void GameModel::updateFuturePos(inGameData_t& dataPassing)
{
	float timeInFuture = 0.2;   // 2 update cycles
	for (int i = 0; i < 6; i++)
	{
		dataPassing.teamFuturePositions[i].x = dataPassing.teamFuturePositions[i].x
			+ (dataPassing.teamFuturePositions[i].x * timeInFuture);
		dataPassing.teamFuturePositions[i].z = dataPassing.teamFuturePositions[i].z
			+ (dataPassing.teamFuturePositions[i].z * timeInFuture);
		dataPassing.oppTeamFuturePos[i].x = dataPassing.oppTeamPositions[i].x
			+ (dataPassing.oppTeamSpeed[i].x * timeInFuture);
		dataPassing.oppTeamFuturePos[i].z = dataPassing.oppTeamPositions[i].z
			+ (dataPassing.oppTeamSpeed[i].z * timeInFuture);

		if (!isInCourt(dataPassing.oppTeamPositions[i]))
			dataPassing.oppTeamPositions[i].y = 100;
	}
}

/**
 * @brief updates the position of the players
 *
 */
void GameModel::updatePositions()
{
	for (auto Robot : team)
	{
		setPoint_t destination = Robot->getSetPoint();
		if (destination.coord.x != 20 && destination.coord.y != 20 && destination.rotation != 20)
		{
			setSetpoint(destination, Robot->robotID);
		}
	}
}

/**
 * @brief adds a robot to the field
 *
 * @param bot pointer to the robot added
 */
void GameModel::addPlayer()
{
	if (msjteam == (teamID[0] - '0'))
	{
		if (dataPassing.availableRobots < 6)
		{
			team[dataPassing.availableRobots]->enablePlayer();
			dataPassing.availableRobots++;
		}
	}
}

/**
 * @brief removes a robot from the field
 *
 * @param bot pointer to the robot added
 */
void GameModel::removePlayer()
{
	if (msjteam == (teamID[0] - '0'))
	{
		if (dataPassing.availableRobots > 0)
		{
			dataPassing.availableRobots--;
			team[dataPassing.availableRobots]->dissablePlayer();
		}
	}
}

/**
 * @brief adds a setpoint from RobotID to messagesToSend
 *
 * @param setpoint position to set in the robot
 * @param robotID number of robot to change the setpoint
 */
void GameModel::setSetpoint(setPoint_t setpoint, int robotID)
{
	if (team[robotID]->getInField())
	{
		Vector3 posInCourt = dataPassing.teamPositions[robotID];
		float dist = distanceOfCoords({ posInCourt.x, posInCourt.z }, setpoint.coord);
		float propSetPoint;
		if (dist < 1)
			propSetPoint = 0.65;
		else if (dist < 3)
			propSetPoint = 0.5;
		else if (dist < 3.5)
			propSetPoint = 0.3;
		else
			propSetPoint = 0.2;
		setpoint.coord = proportionalPosition({ posInCourt.x, posInCourt.z },
			setpoint.coord, propSetPoint);
		checkForCollision({ posInCourt.x,posInCourt.z }, setpoint, robotID);

		if (isInCourt({ setpoint.coord.x, 0, setpoint.coord.y })) //valid position
		{
			if (!((MODULE(dataPassing.oppGoal.x - dataPassing.ballPosition.x) <= 1.1f) &&
				(dataPassing.ballPosition.z > -1.1f) && (dataPassing.ballPosition.z < 1.1f) &&
				(MODULE(dataPassing.oppGoal.x - setpoint.coord.x) <= 1.1f) &&
				(setpoint.coord.y > -1.1f) && (setpoint.coord.y < 1.1f))) //enemies area
			{
				MQTTMessage setpointMessage = { "robot" + teamID + "." + to_string(robotID + 1) +
					"/pid/setpoint/set", getArrayFromSetPoint(setpoint) };
				messagesToSend.push_back(setpointMessage);
			}
		}
	}
	else
	{
		int sign = (dataPassing.myGoal.x > 0 ? 1 : -1);
		setpoint.coord = { (sign * ((robotID * 0.4f) + 1)),-3.5f };
		setpoint.rotation = 180.0f;

		Vector3 posInCourt = dataPassing.teamPositions[robotID];
		float dist = distanceOfCoords({ posInCourt.x, posInCourt.z }, setpoint.coord);

		float propSetPoint;
		if (dist < 1)
			propSetPoint = 0.7;
		else if (dist < 3)
			propSetPoint = 0.5;
		else if (dist < 3.5)
			propSetPoint = 0.3;
		else
			propSetPoint = 0.2;

		setpoint.coord = proportionalPosition({ posInCourt.x, posInCourt.z }, setpoint.coord,
			propSetPoint);
		checkForCollision({ posInCourt.x,posInCourt.z }, setpoint, robotID);

		MQTTMessage setpointMessage = { "robot" + teamID + "." + to_string(robotID + 1) +
			"/pid/setpoint/set", getArrayFromSetPoint(setpoint) };
		messagesToSend.push_back(setpointMessage);
	}
}

/**
 * @brief prepares and uploads images
 *
 * @param path to the image file
 * @param robotID number of robot to upload image
 */
void GameModel::setDisplay(string path, string robotID)
{
	Image displayImage = LoadImage(path.c_str());
	ImageFormat(&displayImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8);

	const int dataSize = 16 * 16 * 3;
	vector<char> payload(dataSize);
	memcpy(payload.data(), displayImage.data, dataSize);

	MQTTMessage setDisplayMessage = { robotID + "/display/lcd/set", payload };
	messagesToSend.push_back(setDisplayMessage);

	UnloadImage(displayImage);
}

/**
 * @brief charges the robot capacitor
 *
 * @param robotID number of robot to charge voltage
 */
void GameModel::voltageKickerChipper(string robotID)
{
	float voltage = 200.0f;
	vector<char> payload = getDataFromFloat(voltage);

	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/kicker/chargeVoltage/set",
							 payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief sets the robot dribbler
 *
 * @param robotID number of robot to set the dribbler
 */
void GameModel::setDribbler(string robotID, bool onOff)
{
	float voltage = onOff ? 0.5f : 0.0f;
	vector<char> payload = getDataFromFloat(voltage);

	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/dribbler/current/set", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief sets the robot kicker
 *
 * @param robotID number of robot to kick
 */
void GameModel::setKicker(string robotID, float power)
{
	vector<char> payload = getDataFromFloat(power);

	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/kicker/kick/cmd", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief sets the robot chipper
 *
 * @param robotID number of robot to set the chipper
 */
void GameModel::setChipper(string robotID, float power)
{
	vector<char> payload = getDataFromFloat(power);
	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/kicker/chip/cmd", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief adds a robot to the team
 *
 * @param bot pointer to the robot added
 */
void GameModel::addRobot(Players* bot)
{
	team.push_back(bot);
}

string GameModel::getTeamID()
{
	return teamID;
}

/**
 * @brief gets the position of where the ball is going to land
 * @return posicion futura de la pelota
 * @param ballPosition: posicion de la pelota
 * @param ballVelocity: velocidad de la pelota
 * Y relation in oblicuous trajectory
 * Y = Yo + Vy * time + 1/2 * Ay * time^2
 * aproximate X,Z for falling time
 * time = (-Vy +- sqrt(Vy*Vy - 4*Yo*1/2*Ay))/(2*1/2*Ay) (time > 0)
 */
Vector2 GameModel::getProxPosBall2D(Vector3 ballPosition, Vector3 ballVelocity)
{
	Vector2 proxPos;
	float discriminante = (ballVelocity.y * ballVelocity.y) - (2 * ballPosition.y * EARTH__GRAVITY);
	if (discriminante >= 0)
	{
		discriminante = sqrt(discriminante);
		float fallingTime = (-ballVelocity.y + discriminante) / (EARTH__GRAVITY);
		if (fallingTime < 0) // negative time
			fallingTime = (-ballVelocity.y - discriminante) / (EARTH__GRAVITY);

		proxPos.x = ballPosition.x + (ballVelocity.x * fallingTime);
		proxPos.y = ballPosition.z + (ballVelocity.z * fallingTime);
	}
	else
	{
		proxPos.x = ballPosition.x + (ballVelocity.x * 0.1); // forced aproximation
		proxPos.y = ballPosition.z + (ballVelocity.z * 0.1);
	}
	return proxPos;
}