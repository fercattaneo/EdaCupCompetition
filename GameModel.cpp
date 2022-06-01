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
  * @param mqttClient
  * @param myTeam
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

	float PIDparameters[6] = { 4, 0, 8, 0, 0, 0.005 };
	vector<char> payloadPID(6 * sizeof(float));
	memcpy(&payloadPID[0], PIDparameters, 6 * sizeof(float));

	for (auto bot : team)
	{
		cout << "robot" + myTeam + "." + to_string(bot->robotID) + "/pid/parameters/set" << endl;
		this->mqttClient->publish("robot" + myTeam + "." + to_string(bot->robotID) + "/pid/parameters/set", payloadPID);
	}

	// vector<Vector3> positions;
	// for (auto bot : team)
	// {
	// 	positions.push_back(bot->getPosition()); //pusheo 6 posiciones al vector
	// }
	// positions = dataPassing.teamPositions;

}

GameModel::~GameModel()
{
	team.clear();
	dataPassing.oppTeamPositions.clear();
}

/**
 * @brief this function recives a message from MQTTclient and analizes the topic
 *
 * @param topic: string declaration of messagge´s topic
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

		update(dataPassing);

		while (!messagesToSend.empty()) // sends all messages appended to message vector
		{
			MQTTMessage actualMessage = messagesToSend.back();
			mqttClient->publish(actualMessage.topic, actualMessage.payload);
			messagesToSend.pop_back();
		}
	}
	else
		assignMessagePayload(topic, payload);
}

/**
 * @brief initializes players of the team
 *
 */
void GameModel::start()
{
	cout << "GameModel START" << endl;

	for (int playerNumber = 0; playerNumber < team.size(); playerNumber++)
	{
		team[playerNumber]->start(playerNumber);
		team[playerNumber]->fieldRol = playerNumber;
	}
	dataPassing.teamPositions.resize(6); 		//le damos un tamaño al vector para que no sea vacío
	dataPassing.oppTeamPositions.resize(6);
}

/**
 * @brief updates status of robots
 *
 */
void GameModel::update(inGameData_t& dataPassing)
{
	for (auto player : team)
	{
		player->update(dataPassing);
	}
	updatePositions();
}

/**
 * @brief analizes the topic and delivers the payload to the desired place ¿?
 *
 * @param topic: string declaration of messagge´s topic
 * @param payload: data of the topic
 */
void GameModel::assignMessagePayload(string topic, vector<char>& payload)
{
	if (topic.compare(0, 6, "edacup") == 0) // game state messagge
	{
		cout << "EDACUP MESSAGGE" << endl;
		if (topic.compare(7, 10, "preKickOff") == 0)
			gameState = PRE_KICKOFF;
		else if (topic.compare(7, 7, "kickOff") == 0)
			gameState = KICKOFF;
		else if (topic.compare(7, 11, "preFreeKick") == 0)
			gameState = PRE_FREEKICK;
		else if (topic.compare(7, 8, "freeKick") == 0)
			gameState = FREEKICK;
		else if (topic.compare(7, 14, "prePenaltyKick") == 0)
			gameState = PRE_PENALTY;
		else if (topic.compare(7, 11, "penaltyKick") == 0)
			gameState = PENALTY;
		else if (topic.compare(7, 5, "pause") == 0)
			gameState = PAUSE;
		else if (topic.compare(7, 8, "continue") == 0)
			gameState = CONTINUE;
		else if (topic.compare(7, 11, "removeRobot") == 0)
			gameState = REMOVE_ROBOT;
		else if (topic.compare(7, 8, "addRobot") == 0)
			gameState = ADD_ROBOT;
		else
			cout << "FAILURE: INVALID GAMESTATE MESSAGE RECIVED" << endl;
	}
	else if (topic.compare(5, 1, teamID) == 0) // the robot belongs to the team
	{
		if (topic.compare(9, 6, "motion") == 0)
		{
			int robotIndex = topic[7] - '0';

			float payloadToFloat[12];
			memcpy(payloadToFloat, &payload[0], payload.size());

			team[robotIndex - 1]->setPosition({ payloadToFloat[0], payloadToFloat[1], payloadToFloat[2] });
			team[robotIndex - 1]->setSpeed({ payloadToFloat[3], payloadToFloat[4], payloadToFloat[5] });
			team[robotIndex - 1]->setRotation({ payloadToFloat[6], payloadToFloat[7], payloadToFloat[8] });
			team[robotIndex - 1]->setAngularSpeed({ payloadToFloat[9], payloadToFloat[10], payloadToFloat[11] });

			dataPassing.teamPositions[robotIndex - 1].x = payloadToFloat[0];
			dataPassing.teamPositions[robotIndex - 1].y = payloadToFloat[1];
			dataPassing.teamPositions[robotIndex - 1].z = payloadToFloat[2];
		}
		else if (topic.compare(9, 11, "power/state") == 0)
		{
			int robotIndex = topic[7] - '0';

			float payloadToFloat[3];
			memcpy(payloadToFloat, &payload[0], std::min(payload.size(), sizeof(payloadToFloat)));

			team[robotIndex - 1]->setPowerLevels({ payloadToFloat[0], payloadToFloat[1], payloadToFloat[2] });
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

		}
	}
	else // error in the messagge recived or topic processing
	{
		cout << "FAILURE: INVALID MAIN TOPIC MESSAGE RECIVED" << endl;
	}
}

/**
 * @brief adds a robot to the team
 *
 * @param bot pointer to the robot added
 */
void GameModel::addPlayer(Players* bot)
{
	team.push_back(bot);
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

string GameModel::getTeamID()
{
	return teamID;
}

/**
 * @brief gets the position of where the ball is going to land
 *
 * @param ballPosition
 * @param ballVelocity
 */
Vector2 GameModel::getProxPosBall2D(Vector3 ballPosition, Vector3 ballVelocity)
{
	// calculo de tiro oblicuo, relacion en Y
	//  Y = Yo + Vy * time + 1/2 * Ay * time^2
	// hallar tiempo de caida para aproximar X,Z
	//  time = (-Vy +- sqrt(Vy*Vy - 4*Yo*1/2*Ay))/(2*1/2*Ay)
	// tomo el time positivo xq es el valido
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
	else // raiz imaginaria
	{
		cout << "Invalid ball position algebra..." << endl;
		proxPos.x = ballPosition.x + (ballVelocity.x * 0.1); // forced aproximation
		proxPos.y = ballPosition.z + (ballVelocity.z * 0.1);
	}
	return proxPos;
}

/**
 * @brief updates the position of the players
 *
 */
void GameModel::updatePositions()
{
	for (auto teamRobot : team)
	{
		setPoint_t destination = teamRobot->getSetPoint();
		if (destination.coord.x != 20 && destination.coord.y != 20 && destination.rotation != 20)
		{
			setSetpoint(destination, teamRobot->robotID);
		}
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
	ImageFormat(&displayImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8); // a Robot10.png no le cambia el formato

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
	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/kicker/chargeVoltage/set", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief sets the robot dribbler
 *
 * @param robotID number of robot to set the dribbler
 */
void GameModel::setDribbler(string robotID)
{
	float voltage = 100.0f;
	vector<char> payload = getDataFromFloat(voltage);
	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/dribbler/voltage/set", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief sets the robot kicker
 *
 * @param robotID number of robot to kick
 */
void GameModel::setKicker(string robotID)
{
	float potencia = 0.8;
	vector<char> payload = getDataFromFloat(potencia);
	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/kicker/kick/cmd", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief sets the robot chipper
 *
 * @param robotID number of robot to set the chipper
 */
void GameModel::setChipper(string robotID)
{
	float potencia = 0.8;
	vector<char> payload = getDataFromFloat(potencia);
	MQTTMessage setKicker = { "robot" + teamID + "." + robotID + "/kicker/chip/cmd", payload };
	messagesToSend.push_back(setKicker);
}

/**
 * @brief 
 * 
 * @param player 
 * @param objectivePosition 
 */
void GameModel::shoot(Players* player, Vector2 objectivePosition)
{
	setPoint_t proxPlaceInCourt = player->kickBallLogic(objectivePosition, { dataPassing.ballPosition.x, dataPassing.ballPosition.z });
	setPoint_t kickValue = { 100, 100, 100 };
	Vector2 playerPosition = {player->getPosition().x, player->getPosition().z}; 

	if (!checkForInterception(dataPassing.oppTeamPositions, objectivePosition, playerPosition))
	{
		if ((proxPlaceInCourt.coord.x == kickValue.coord.x) &&
			(proxPlaceInCourt.coord.y == kickValue.coord.y) &&
			(proxPlaceInCourt.rotation == kickValue.rotation)) // comparacion de igualdad de setpoints
		{
			if (player->getKickerCharge() >= 160)
				setKicker(to_string(player->robotID));
			else
				voltageKickerChipper(to_string(player->robotID)); // este orden por el pop_back del vector
		}
		else // mover hasta el setpoint indicado
		{
			Vector3 posInCourt = player->getPosition();
			setSetpoint(proxPlaceInCourt, player->robotID);
		}
	}
	// else
	// {
	// 	//Ver como calcular la fuerza para pareat por arriba
	// }
}

/**
 * @brief 
 * 
 * @param oppTeamPositions 
 * @param objective 
 * @param teamPosition 
 * @return true if possible
 * @return false if not
 */
bool GameModel::checkForInterception(vector<Vector3>& oppTeamPositions, Vector2 objective, Vector2 teamPosition)
{
	bool possible = true;

	for (int bot=0;bot<6;bot++)
	{
		Vector2 botPosition = { oppTeamPositions[bot].x, oppTeamPositions[bot].z };
		if (!betweenTwoLines(teamPosition, objective, botPosition, 0.20f))  	// me fijo si algun robot esta
		{															// en el pasillo donde voy a hacer el pase
			possible = false;
		}
	}

	return possible;
}

/**
 * @brief adds a setpoint from RobotID to messagesToSend
 *
 * @param setpoint position to set in the robot 
 * @param robotID number of robot to change the setpoint
 */
void GameModel::setSetpoint(setPoint_t setpoint, int robotID)
{
	Vector3 posInCourt = dataPassing.teamPositions[robotID];
	float dist = distanceOfCoords({posInCourt.x, posInCourt.z}, setpoint.coord);
	//tiene que ir de 1 a 0.5 en funcion de 0 a 10 ¿?
	float propSetPoint;
	if(dist < 1)
		propSetPoint = 0.8;
	else if(dist < 3)
		propSetPoint = 0.6;
	else 
		propSetPoint = 0.5;
	setpoint.coord = proportionalPosition({ posInCourt.x, posInCourt.z }, setpoint.coord, propSetPoint);
	MQTTMessage setpointMessage = { "robot" + teamID + "." + to_string(robotID + 1) + "/pid/setpoint/set",
								   getArrayFromSetPoint(setpoint) };
	messagesToSend.push_back(setpointMessage);
}


//
//		PARA DESPUES, CUANDO TENGAMOS "TODO ARMADO"
// funcion 
void GameModel::searchFreeBall()
{
	setPoint_t destination;
	if(dataPassing.ballPosition.y > 0.01) //si la altura es mayor a ALGO analizo funcion de caida
		destination.coord = getProxPosBall2D(dataPassing.ballPosition, dataPassing.ballVelocity);
	else  //sino analizo por pases por el piso
	{
		float proxTime = 0.33; //calculo de la posicion en 0.33 segundos
		Vector2 futurePosition;
		futurePosition.x = dataPassing.ballPosition.x + (dataPassing.ballVelocity.x * proxTime);
		futurePosition.y = dataPassing.ballPosition.z + (dataPassing.ballVelocity.z * proxTime);
		destination.coord = futurePosition;
	}
	//con el punto objetivo miro a nuestro robot mas cercano y seteo que vaya a esa posicion 
	float maxCloseness = 9.0;
	int index;
	for(auto ally : team)
	{
		Vector3 pos = ally->getPosition();
		float closeness = distanceOfCoords({pos.x, pos.z},{destination.coord.x, destination.coord.y});
		if(closeness < maxCloseness)
		{
			maxCloseness = closeness;
			index = ally->robotID;
		}
	}
	destination.rotation = calculateRotation ({destination.coord.x, destination.coord.y},
		{dataPassing.ballPosition.x, dataPassing.ballPosition.z});
	setSetpoint(destination, index);
}

void GameModel::analizePosession()
{
	posession = FREE_BALL;
	for(auto ally : team)
	{
		Vector3 posOfAlly = ally->getPosition();
		if(isCloseTo({posOfAlly.x, posOfAlly.z}, {dataPassing.ballPosition.x, dataPassing.ballPosition.z} , 0.15))
		{
			posession = MY_TEAM; //un aliado esta cerca
			robotWithBall = ally->robotID;
			break;
		}
	}
	int nearEnemies = 0;
	for (auto enemy : team)
	{
		Vector3 posOfEnemy = enemy->getPosition();
		bool nearness = isCloseTo({posOfEnemy.x, posOfEnemy.z},
			{dataPassing.ballPosition.x, dataPassing.ballPosition.z} , 0.15);
		if(nearness && (posession == FREE_BALL))  //la tiene un enemigo
		{
			posession = OPP_TEAM;
			break;
		}
		else if(nearness && (posession == MY_TEAM)) //esta en disputa
			nearEnemies++;
	}
	if((posession != FREE_BALL) && nearEnemies == 1)
		posession = DISPUTED_BALL;
	else if((posession != FREE_BALL) && nearEnemies > 1)
		posession = OPP_TEAM;
}