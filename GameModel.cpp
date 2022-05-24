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
	this->teamID = myTeam;
	oppTeamID = (myTeam == "1") ? "2" : "1";
	Vector2 arco1 = { 4.5f, 0.0f };
	Vector2 arco2 = { -4.5f, 0.0f };
	arcoTeam = (myTeam == "1") ? arco1 : arco2;
	arcoOpposite = (myTeam == "1") ? arco2 : arco1;
	for (int i = 0; i < 12; i++)
	{
		ball[i] = 0;
	}
	ball[0] = 1.5;

	float PIDparameters[6] = { 4, 0, 8, 0, 0, 0.005 };
	vector<char> payloadPID(6 * sizeof(float));
	memcpy(&payloadPID[0], PIDparameters, 6 * sizeof(float));

	for (auto bot : team)
	{
		this->mqttClient->publish("robot" + myTeam + "." + bot->robotID + "/pid/parameters/set", payloadPID);
	}

	Goalie player1(arcoTeam);
	addPlayer(&player1);
	Defense player2(arcoTeam);
	addPlayer(&player2);
	Defense player3(arcoTeam);
	addPlayer(&player3);
	Midfielder player4(arcoTeam);
	addPlayer(&player4);
	Midfielder player5(arcoTeam);
	addPlayer(&player5);
	Shooter player6(arcoTeam);
	addPlayer(&player6);

	Robot enemy1;
	oppTeam.push_back(&enemy1);
	Robot enemy2;
	oppTeam.push_back(&enemy2);
	Robot enemy3;
	oppTeam.push_back(&enemy3);
	Robot enemy4;
	oppTeam.push_back(&enemy4);
	Robot enemy5;
	oppTeam.push_back(&enemy5);
	Robot enemy6;
	oppTeam.push_back(&enemy6);
}

GameModel::~GameModel()
{
	team.clear();
	oppTeam.clear();
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
		team[playerNumber]->start(to_string(playerNumber + 1));
	}
}

/**
 * @brief initializes players of the team
 *
 */
void GameModel::update()
{
	for (auto player : team)
	{
		//player->update();
	}
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
		memcpy(ball, &payload[0], payload.size());
		verifyPossesion();

		update();

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

void GameModel::verifyPossesion()
{
	int multipleCloseness = 0;
	for (auto bot : team)
	{
		Vector3 botCoord = bot->getPosition();
		Vector2 XZcoord = { botCoord.x,botCoord.z };
		if (isCloseTo(XZcoord, { ball[0],ball[2] }, 0.15))
		{
			attackingTeam = MY_TEAM;
			multipleCloseness++;
		}
	}
	for (auto bot : oppTeam)
	{
		Vector3 botCoord = bot->getPosition();
		Vector2 XZcoord = { botCoord.x,botCoord.z };
		if (isCloseTo(XZcoord, { ball[0],ball[2] }, 0.15))
		{
			attackingTeam = OPP_TEAM;
			multipleCloseness--;
		}
	}
	if (multipleCloseness == 0)
		attackingTeam = FREE_BALL;
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

			oppTeam[robotIndex - 1]->setPosition({ payloadToFloat[0], payloadToFloat[1], payloadToFloat[2] });
			oppTeam[robotIndex - 1]->setSpeed({ payloadToFloat[3], payloadToFloat[4], payloadToFloat[5] });
			oppTeam[robotIndex - 1]->setRotation({ payloadToFloat[6], payloadToFloat[7], payloadToFloat[8] });
			oppTeam[robotIndex - 1]->setAngularSpeed({ payloadToFloat[9], payloadToFloat[10], payloadToFloat[11] });
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
coord_t GameModel::getProxPosBall2D(Vector3 ballPosition, Vector3 ballVelocity)
{

	coord_t proxPos;
	float discriminante = (ballVelocity.y * ballVelocity.y) - (2 * ballPosition.y * EARTH__GRAVITY);
	if (discriminante >= 0)
	{
		discriminante = sqrt(discriminante);
		float fallingTime = (-ballVelocity.y + discriminante) / (EARTH__GRAVITY);
		if (fallingTime < 0) // negative time
			fallingTime = (-ballVelocity.y - discriminante) / (EARTH__GRAVITY);

		proxPos.x = ballPosition.x + (ballVelocity.x * fallingTime);
		proxPos.z = ballPosition.z + (ballVelocity.z * fallingTime);
	}
	else // raiz imaginaria
	{
		cout << "Invalid ball position algebra..." << endl;
		proxPos.x = ballPosition.x + (ballVelocity.x * 0.1); // forced aproximation
		proxPos.z = ballPosition.z + (ballVelocity.z * 0.1);
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
		if (attackingTeam == MY_TEAM)
		{
			teamRobot->updateAttack();
		}
		else if (attackingTeam == OPP_TEAM)
		{
			teamRobot->updateDefense();
		}
		else
		{
			//teamRobot->updateFreeBall();
		}
		/*setPoint_t destination = teamRobot->goToBall(arcoOpposite, {ball[0], ball[2]}, 1.05f);
		MQTTMessage setpointMessage = {"robot" + teamRobot->teamID + "." + teamRobot->robotID +
										   "/pid/setpoint/set",
									   getArrayFromSetPoint(destination)};
		messagesToSend.push_back(setpointMessage);*/
		robotMessagesToSend(teamRobot);
	}
}


void GameModel::robotMessagesToSend(Robot* robot)
{
	setPoint_t proxDestination = robot->getRobotSetPoint();
	float kickerCharge = robot->getKickerCharge();
	float dribblerCommand = robot->getDribbler();
	float kickerCommand = robot->getKicker();
	float chipperCommand = robot->getChipper();

	string ID = "robot" + teamID + "." + robot->robotID;

	MQTTMessage message1 = { ID + "/pid/setpoint/set", getArrayFromSetPoint(proxDestination) };
	messagesToSend.push_back(message1);
	MQTTMessage message2 = { ID + "/kicker/kick/cmd", getDataFromFloat(kickerCommand) };
	messagesToSend.push_back(message2);
	MQTTMessage message3 = { ID + "/kicker/chip/cmd", getDataFromFloat(chipperCommand) };
	messagesToSend.push_back(message3);
	MQTTMessage message4 = { ID + "/dribbler/voltage/set", getDataFromFloat(dribblerCommand) };
	messagesToSend.push_back(message4);
	MQTTMessage message5 = { ID + "/kicker/chargeVoltage/set", getDataFromFloat(kickerCharge) };
	messagesToSend.push_back(message5);
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
 * @brief gets the robot to the ball location and, if positioned
 * kikcs the ball
 *
 * @param player
 */
void GameModel::shootToGoal(Players* player)
{
	setPoint_t placeInCourt = player->kickBallLogic(arcoOpposite, { ball[0], ball[2] });
	setPoint_t kickValue = { 100, 100, 100 };

	if ((placeInCourt.coord.x == kickValue.coord.x) &&
		(placeInCourt.coord.y == kickValue.coord.y) &&
		(placeInCourt.rotation == kickValue.rotation)) // comparacion de igualdad de setpoints
	{
		if (player->getKickerCharge() >= 160)
			setKicker(player->robotID);
		else
			voltageKickerChipper(player->robotID); // este orden por el pop_back del vector
	}
	else // mover hasta el setpoint indicado
		setSetpoint(placeInCourt, player->robotID);
}

/**
 * @brief adds a setpoint from RobotID to messagesToSend
 *
 * @param setpoint position to set in the robot
 * @param robotID number of robot to change the setpoint
 */
void GameModel::setSetpoint(setPoint_t setpoint, string robotID)
{
	MQTTMessage setpointMessage = { "robot" + teamID + "." + robotID + "/pid/setpoint/set",
								   getArrayFromSetPoint(setpoint) };
	messagesToSend.push_back(setpointMessage);
}