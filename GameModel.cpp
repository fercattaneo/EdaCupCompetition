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
	gameState = START;

	float PIDparameters[6] = { 4, 0, 8, 0, 0, 0.005 };
	vector<char> payloadPID(6 * sizeof(float));
	memcpy(&payloadPID[0], PIDparameters, 6 * sizeof(float));

	for (auto bot : team)
	{
		cout << "robot" + myTeam + "." + to_string(bot->robotID) + "/pid/parameters/set" << endl;
		this->mqttClient->publish("robot" + myTeam + "." + to_string(bot->robotID) + "/pid/parameters/set", payloadPID);
	}
}

GameModel::~GameModel()
{
	team.clear();
	dataPassing.oppTeamPositions.clear();
}

/**
 * @brief this function recives a message from MQTTclient and analyzes the topic
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
	dataPassing.teamFuturePositions.resize(6);
	dataPassing.oppTeamPositions.resize(6);
	dataPassing.oppTeamFuturePos.resize(6);
	dataPassing.oppTeamSpeed.resize(6);
}

void GameModel::updateGameConditions(inGameData_t& dataPassing)
{
	switch (gameState)
	{
	case START:
		initialPositions();//start de los robots
		gameState = CONTINUE;
		break;
	case PRE_KICKOFF:  //El equipo (1 o 2) está por realizar un saque inicial.
		// ordenar el equipo
		break;
	case KICKOFF: //El equipo (1 o 2) debe realizar el saque inicial.
		//realizar el saque y poner en CONTINUE
		break;
	case PRE_FREEKICK: //El equipo (1 o 2) está por realizar un tiro libre.
		// ordenar el equipo
		break;
	case FREEKICK: //El equipo (1 o 2) debe realizar el tiro libre.
		// probar si sirve poner el continue, sino cagamo'
		break;
	case PRE_PENALTY: // El equipo (1 o 2) está por realizar un tiro penal
		// ordenar el equipo
		break;
	case PENALTY: //El equipo (1 o 2) debe realizar el tiro penal.
		//fnucion de penal
		break;
	case PAUSE: //El juego se detuvo
		//no hace nada ¿? > se actualiza el game state con los mensajes nomas
		break;
	case CONTINUE: //El juego se reanuda.
		update(dataPassing); //esta es la de movimientos/pases/tiros...
		break;
	case REMOVE_ROBOT: //El equipo (1 o 2) debe retirar un robot.
		// seria un team.pop_back() / sacar un enemigo y resize de vector/es del dataPassing
		break;
	case ADD_ROBOT: //El equipo (1 o 2) puede incorporar un robot.
		// seria un team.push_back() / agregar un enemigo y resize de vector/es del dataPassing, ver lo del start...
		break;
	case ENDED_GAME:
		//no se si hay un mensaje de esto
		break;
	case GOAL:
		//analizarlo y festejo?
		break;
	}
}


/**
 * @brief updates status of robots
 *
 */
void GameModel::update(inGameData_t& dataPassing)
{
	if (isBallInCourt()) {
		updateFuturePos(dataPassing);
		analyzePosession();
		if (posession == FREE_BALL)
		{
			searchFreeBall(); //TESTING
		}
		else
		{
			for (auto player : team)
			{
				if (player->robotID == robotWithBall && posession == MY_TEAM) 	//tiene la pelota
				{
					Vector2 shootGoal = analyzeShoot(*player);
					cout << "ShootGoal: " << shootGoal.x << ", " << shootGoal.y << endl;
					if (shootGoal.x != 0)
						shoot(player, dataPassing.oppGoal);
					else
					{
						int indexForPass = analyzePass(*player);
						if (indexForPass != 7) // " 7 como valor de no hay pase "
						{
							Vector3 passingPlace3D = team[indexForPass]->getPosition();
							Vector2 passingPlace = { passingPlace3D.x, passingPlace3D.z };
							team[indexForPass]->setSetpoint({calculateRotation(
								{dataPassing.ballPosition.x, dataPassing.ballPosition.z},
								passingPlace)});  //ver la rotacion del receptor
							setDribbler(to_string(indexForPass),true);
							shoot(player, passingPlace);
						}
						else
						{
							cout << "No pude dar pase" << endl;
							//dribbleTo();  //queda para hacerla
						}
					}
				}
				else    //no tiene la pelota
				{
					player->update(dataPassing);
				}

				// TESTING
				/*if (player->robotID != 2)
				{
					player->update(dataPassing);
				}
				else
				{
					shoot(player, dataPassing.oppGoal);
				}
				*/

			}
		}
		updatePositions();
	}
}

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
	}
}

/**
 * @brief analyzes the topic and delivers the payload to the desired place ¿?
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
 * @return posicion futura de la pelota
 * @param ballPosition: posicion de la pelota
 * @param ballVelocity: velocidad de la pelota
 * calculo de tiro oblicuo, relacion en Y
 * Y = Yo + Vy * time + 1/2 * Ay * time^2
 * hallar tiempo de caida para aproximar X,Z
 * time = (-Vy +- sqrt(Vy*Vy - 4*Yo*1/2*Ay))/(2*1/2*Ay)
 * tomo el time positivo xq es el valido
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
	// fijarse si esrtoy cerca de la pelota
	// prender el dribbler
	// si estoy en la rotacion correcta pego
	// si no estoy cerca de la pelota me acerco
	static int flag = 0;
	Vector3 pos = player->getPosition();
	Vector2 playerPosition = { pos.x, pos.z };
	Vector2 ballLocation = { dataPassing.ballPosition.x, dataPassing.ballPosition.z };
	float rotation = calculateRotation({ dataPassing.ballPosition.x,
										dataPassing.ballPosition.z }, playerPosition);

	voltageKickerChipper(to_string(player->robotID + 1));

	if (isCloseTo(playerPosition, ballLocation, 0.1f) && flag == 5) //radio mas chico	
	{
		rotation = calculateRotation(objectivePosition, playerPosition);
		float botRealRot = player->getRotation().y;
		rotation += (rotation < 0) ? 360 : 0;
		botRealRot += (botRealRot < 0) ? 360 : 0;

		if (MODULE(botRealRot - rotation) < 10) // ya termino de rotar
		{
			setDribbler(to_string(player->robotID + 1), false);

			float distance = distanceOfCoords(playerPosition, objectivePosition);
			float power;

			if (objectivePosition.x == dataPassing.oppGoal.x &&
				objectivePosition.y == dataPassing.oppGoal.y)		// Shoot goal
				power = 0.8;
			else				// pass 
				power = distance / 10;		

			setKicker(to_string(player->robotID + 1), power);
			flag = 0;
		}
	}
	else if (isCloseTo(playerPosition, ballLocation, 0.11f) && flag < 5) //Estoy cerca de la pelota
	{
		setDribbler(to_string(player->robotID + 1), true);
		flag++;
	}

	setSetpoint({ ballLocation, rotation }, player->robotID);
}

/**
 * @brief
 *
 * @param oppTeamPositions
 * @param objective
 * @param teamPosition
 * @return true if pass is possible
 * @return false if not
 */
bool GameModel::checkForInterception(vector<Vector3>& oppTeamPositions, Vector2 objective, Vector2 teamPosition)
{
	for (int bot = 0; bot < 6; bot++)
	{
		Vector2 botPosition = { oppTeamPositions[bot].x, oppTeamPositions[bot].z };
		if (betweenTwoLines(teamPosition, objective, botPosition, 0.20f))  	// me fijo si algun robot esta
		{															// en el pasillo donde voy a hacer el pase
			return false;
		}
	}
	return true;
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
	float dist = distanceOfCoords({ posInCourt.x, posInCourt.z }, setpoint.coord);
	//tiene que ir de 1 a 0.5 en funcion de 0 a 10 ¿?
	float propSetPoint;
	if (dist < 1)
		propSetPoint = 0.8;
	else if (dist < 3)
		propSetPoint = 0.6;
	else if (dist < 3.5)
		propSetPoint = 0.5;
	else
		propSetPoint = 0.3;
	setpoint.coord = proportionalPosition({ posInCourt.x, posInCourt.z }, setpoint.coord, propSetPoint);
	checkForCollision({ posInCourt.x,posInCourt.z }, setpoint);

	MQTTMessage setpointMessage = { "robot" + teamID + "." + to_string(robotID + 1) + "/pid/setpoint/set",
								   getArrayFromSetPoint(setpoint) };
	messagesToSend.push_back(setpointMessage);
}

/**
 * @brief
 *
 * @param setpoint
 */
void GameModel::checkForCollision(Vector2 actualPos, setPoint_t& setpoint)
{
	float proxTime = 0.2;
	Vector2 futurePosition = { 0, 0 };

	for (int bot = 0; bot < 6; bot++)
	{
		Vector3 positionOpp = dataPassing.oppTeamPositions[bot];
		if (betweenTwoLines(actualPos, setpoint.coord, { positionOpp.x, positionOpp.z }, 0.24f))
		{
			setpoint.coord = { dataPassing.oppTeamFuturePos[bot].x , dataPassing.oppTeamFuturePos[bot].z };
			break;
		}
	}
	for (int bot = 0; bot < 6; bot++)
	{
		Vector3 robotPosition = dataPassing.teamPositions[bot];
		if (betweenTwoLines(actualPos, setpoint.coord, { robotPosition.x, robotPosition.z }, 0.24f))
		{
			futurePosition.x = (robotPosition.x + (dataPassing.ballVelocity.x * proxTime) -
				(dataPassing.ballVelocity.x / MODULE(dataPassing.ballVelocity.x)) * 0.2);
			futurePosition.y = (robotPosition.z + (dataPassing.ballVelocity.z * proxTime) -
				(dataPassing.ballVelocity.z / MODULE(dataPassing.ballVelocity.z)) * 0.2);
			setpoint.coord = { futurePosition.x, futurePosition.y };
			break;
		}
	}
}


//		PARA DESPUES, CUANDO TENGAMOS "TODO ARMADO"
// funcion 
void GameModel::searchFreeBall()
{
	setPoint_t destination;
	if (dataPassing.ballPosition.y > 0.1) //si la altura es mayor a ALGO analizo funcion de caida
		destination.coord = getProxPosBall2D(dataPassing.ballPosition, dataPassing.ballVelocity);
	else  //sino analizo por pases por el piso
	{
		float proxTime = 0.2; //calculo de la posicion en 0.2 segundos
		Vector2 futurePosition;
		futurePosition.x = dataPassing.ballPosition.x + (dataPassing.ballVelocity.x * proxTime);
		futurePosition.y = dataPassing.ballPosition.z + (dataPassing.ballVelocity.z * proxTime);
		destination.coord = futurePosition;
	}
	//con el punto objetivo miro a nuestro robot mas cercano y seteo que vaya a esa posicion 
	float maxCloseness = 9.0;
	int index;
	for (auto ally : team)
	{
		Vector3 pos = ally->getPosition();
		float closeness = distanceOfCoords({ pos.x, pos.z }, { destination.coord.x, destination.coord.y });
		if (closeness < maxCloseness)
		{
			maxCloseness = closeness;
			index = ally->robotID;
		}
	}
	destination.rotation = calculateRotation({ dataPassing.ballPosition.x, dataPassing.ballPosition.z }, 
		{ destination.coord.x, destination.coord.y });
	setSetpoint(destination, index);
}

void GameModel::analyzePosession()
{
	posession = FREE_BALL;
	for (auto enemy : team)
	{
		Vector3 posOfEnemy = enemy->getPosition();
		bool nearness = isCloseTo({ posOfEnemy.x, posOfEnemy.z },
			{ dataPassing.ballPosition.x, dataPassing.ballPosition.z }, 0.12);
		if (nearness)  //la tiene un enemigo
		{
			posession = OPP_TEAM;
			break;
		}
	}
	for (auto ally : team)
	{
		Vector3 posOfAlly = ally->getPosition();
		if (isCloseTo({ posOfAlly.x, posOfAlly.z }, { dataPassing.ballPosition.x,
													 dataPassing.ballPosition.z }, 0.12))
		{
			posession = MY_TEAM; //un aliado esta cerca
			robotWithBall = ally->robotID;
			break;
		}
	}
}

/**
 * @brief Dribbles foward in direction to the oppGoal dogging any enemy that appears
 *
 *
 */
void GameModel::dribbleTo(Players& player)
{
	Vector3 position = player.getPosition();
	float rotation = calculateRotation({ position.x, position.z }, { dataPassing.ballPosition.x, dataPassing.ballPosition.z });

	setDribbler(to_string(player.robotID + 1), true);

	//setPoint_t destination = { , rotation };
	//player->setSetpoint();
}

bool GameModel::isBallInCourt()
{
	if (dataPassing.ballPosition.x > LENGTH_OF_COURT_X / 2 ||
		dataPassing.ballPosition.x < -LENGTH_OF_COURT_X / 2 ||
		dataPassing.ballPosition.z > LENGTH_OF_COURT_Z / 2 ||
		dataPassing.ballPosition.z < -LENGTH_OF_COURT_Z / 2)
		return false;
	else
		return true;
}

void GameModel::initialPositions()
{
	float sign = (dataPassing.myGoal.x) / (MODULE(dataPassing.myGoal.x));
	float rot = calculateRotation(dataPassing.myGoal, { 0,0 });
	for (auto ally : team)
	{
		switch (ally->robotID)
		{
		case 0:ally->setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.0f)), 0}, rot }); break; //goalie
		case 1:	ally->setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.8f)),-1},rot }); break; //def 1
		case 2:	ally->setSetpoint({ {(dataPassing.myGoal.x - (sign * 1.8f)),1}, rot }); break; //def 2
		case 3:	ally->setSetpoint({ {(dataPassing.myGoal.x - (sign * 2.5f)), 0}, rot }); break; //midf
		case 4:	ally->setSetpoint({ {(dataPassing.myGoal.x - (sign * 3.5f)), -1}, rot }); break; //att 1
		case 5:	ally->setSetpoint({ {(dataPassing.myGoal.x - (sign * 3.5f)), 1}, rot }); break;  //att 2
		default: ally->setSetpoint({ {0, 0}, rot }); break;
		}
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
	int flag;

	for (int i = 5; i >= 0; i--)
	{
		flag = 0;
		if (i != player.robotID)
		{
			Vector3 ally = dataPassing.teamFuturePositions[i];
			for (auto enemyBot : dataPassing.oppTeamPositions)
			{
				if (betweenTwoLines({ pos.x,pos.y }, { ally.x,ally.y }, { enemyBot.x, enemyBot.z }, 0.1f))
				{
					flag = 1; //no se puede con este i
				}
			}
			if (!flag)
				return i;
		}
	}

	return 7;
}

Vector2 GameModel::analyzeShoot(Players& player)
{
	Vector2 objectivePosition = player.openZPlace(0.01, dataPassing,
		{ dataPassing.oppGoal.x, dataPassing.oppGoal.y - 0.45f },
		{ dataPassing.oppGoal.x, dataPassing.oppGoal.y + 0.45f },
		{ dataPassing.ballPosition.x, dataPassing.ballPosition.x }); //devuelve la pos libre para el tiro
	if (objectivePosition.y == 0)
		objectivePosition = { 0,0 }; //condicion del NO TIRO
	Vector3 baller = player.getPosition();
	if (distanceOfCoords({ baller.x, baller.z }, dataPassing.oppGoal) < 1.7)
		objectivePosition = dataPassing.oppGoal;

	return objectivePosition;
}