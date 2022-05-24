/*******************************************************************
 * @file data.cpp
 * @date 20/05/2020
 * EDA - 22.08
 * Group 7: Cattaneo, Diaz Excoffon, Diaz Guzman, Michelotti, Wickham
 * @brief Source file of the data module. Module prepared for
 * multiple data types and usefull functions calculus.
 *******************************************************************/
#include "data.h"

using namespace std;

vector<char> getArrayFromSetPoint(setPoint_t setpoint)
{
	vector<char> payload(12);

	*((float *)&payload[0]) = setpoint.coord.x;
	*((float *)&payload[4]) = setpoint.coord.y;
	*((float *)&payload[8]) = setpoint.rotation;

	return payload;
}

/**
 * @brief gets a float from a char vector
 * @param vec: sequence of bytes
 * @return float
 */
float getFloat(vector<char> vec)
{
	float value = 0.0;

	// Extraído de: https://www.cplusplus.com/reference/cassert/assert/
	assert(vec.size() == sizeof(value));

	memcpy(&value, &vec[0], min(vec.size(), sizeof(float)));
	return value;
}

/**
 * @brief Get the Data From Float object
 *
 * @param data
 * @return vector<char>
 */
vector<char> getDataFromFloat(float data)
{
	std::vector<char> payload(sizeof(float));

	memcpy(payload.data(), &data, sizeof(float));

	return payload;
}

/**
 * @brief: calculates the coordinate in reference from other 2 and a proportional value
 * @param: originPos - origin position of object
 * @param: finalPos - final position of reference
 * @param: proportion - proportional position [0 = origin ~~ 1 = final]
 * @return: coordinate calculated
 */
Vector2 proportionalPosition(Vector2 originPos, Vector2 finalPos, float proportion)
{
	Vector2 destination;

	destination.x = (finalPos.x - originPos.x) * proportion + originPos.x;
	destination.y = (finalPos.y - originPos.y) * proportion + originPos.y;

	return destination;
}

/*
 * @brief: calculates the rotation between 2 coordinates
 *
 * @param: originPos - origin position of object
 * @param: finalPos - final position of reference
 *
 * @return: angle in eulerian degrees
 */
float calculateRotation(Vector2 originPos, Vector2 finalPos)
{
	float deltaX = finalPos.x - originPos.x;
	float deltaZ = finalPos.y - originPos.y;

	if (deltaX == 0 && deltaZ == 0)
	{
		// cout << "Same Position delivered" << endl;
		return 180;
	}
	if (deltaZ == 0)
	{
		// cout << "Invalid Angle, aprox to -90°" << endl;
		return 270;
	}
	if (deltaX == 0)
	{
		// cout << "Invalid Angle, aprox to 180°" << endl;
		return 180;
	}

	float angle = std::atan(deltaX / deltaZ);
	if (deltaZ > 0)
	{
		angle -= PI; // radians degrees
	}
	angle = angle * (180 / PI); // grados sexagecimal degrees

	return angle;
}

/**
 * @brief Recives 2 coordinates and if the origin coord is close to destination returns true
 *
 * @param originPos - origin position of object
 * @param finalPos - final position of reference
 * @param nearRange is the area to consider close
 *
 * @return true if its considered near, if not, false
 */
bool isCloseTo(Vector2 originCoord, Vector2 destinationCoord, float nearRange)
{
	if ((originCoord.x) <= (destinationCoord.x + nearRange) &&
		(originCoord.x) >= (destinationCoord.x - nearRange) &&
		(originCoord.y) <= (destinationCoord.y + nearRange) &&
		(originCoord.y) >= (destinationCoord.y - nearRange))
		return true;
	else
		return false;
}

bool sameLine(Vector2 originPos, Vector2 finalPos, Vector2 mediumPos)
{
	float deltaX = finalPos.x - originPos.x;
	float deltaY = finalPos.y - originPos.y;
	if (deltaX == 0) // linea vertical
	{
		if ((mediumPos.x - originPos.x) == 0) // pertenece a la linea
		{
			return true;
		}
	}
	float pendiente = deltaY / deltaX;

	float deltaX2 = mediumPos.x - originPos.x;
	float deltaY2 = mediumPos.y - originPos.y;
	if ((deltaX2 == 0) && ((finalPos.x - mediumPos.x) <= 0))
	{
		return true;
	}
	float pendiente2 = deltaY2 / deltaX2;
	if ((pendiente2 >= (pendiente - 0.05)) && (pendiente2 <= (pendiente + 0.05)))
	{
		return true;
	}
	return false;
}

/**
 * @brief calculates a court setpoint for the player
 *
 * @param destPos: destination of reference
 * @param firstPos: point of reference
 * @param proportional: proportional value for position calculation
 *
 * @return destination: position in court and players rotation
 */
setPoint_t getSetPoint(Vector2 destPos, Vector2 firstPos, float proportional)
{
	setPoint_t destination;

	destination.coord = proportionalPosition(destPos, firstPos, proportional);
	destination.rotation = calculateRotation(destPos, firstPos);

	return destination;
}