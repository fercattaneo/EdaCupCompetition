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

	*((float*)&payload[0]) = setpoint.coord.x;
	*((float*)&payload[4]) = setpoint.coord.y;
	*((float*)&payload[8]) = setpoint.rotation;

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
 * @brief gets vector<char> from float
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
 *
 * @param originPos - origin position of object
 * @param finalPos - final position of reference
 * @param proportion - proportional position [0 = origin ~~ 1 = final]
 *
 * @return coordinate calculated
 */
Vector2 proportionalPosition(Vector2 originPos, Vector2 finalPos, float proportion)
{
	Vector2 destination;

	destination.x = (finalPos.x - originPos.x) * proportion + originPos.x;
	destination.y = (finalPos.y - originPos.y) * proportion + originPos.y;

	return destination;
}

/**
 * @brief: calculates the rotation between 2 coordinates
 *
 * @param: originPos - origin position of object
 * @param: finalPos - final position of reference
 *
 * @return: angle in eulerian degrees
 */
float calculateRotation(Vector2 originPos, Vector2 rotateHere)
{
	float deltaX = rotateHere.x - originPos.x;
	float deltaZ = originPos.y - rotateHere.y;
	float angle = std::atan(MODULE(deltaZ) / MODULE(deltaX));
	angle *= (180 / PI); // grados sexagecimal degrees
	if (deltaZ == 0)
		angle = (deltaX > 0) ? 90 : 270;
	else if (deltaX == 0)
		angle = (deltaZ > 0) ? 0 : 180;
	else
	{
		if (deltaX > 0)
			angle = 90 - ((deltaZ > 0) ? (-angle) : angle);
		else
			angle = 270 + ((deltaZ > 0) ? (-angle) : angle);
	}

	return angle;
}

/**
 * @brief Recives 2 coordinates and if the origin coord is close to destination returns true
 *
 * @param: originPos - origin position of object
 * @param: finalPos - final position of reference
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

/**
 * @brief Checks if a point belongs to the line between two points
 *
 * @param originPos
 * @param finalPos
 * @param mediumPos
 * @return true
 * @return false
 */
bool sameLine(Vector2 originPos, Vector2 finalPos, Vector2 mediumPos)
{
	float deltaX = finalPos.x - originPos.x;
	float deltaY = finalPos.y - originPos.y;
	if (deltaX == 0)
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
 * @brief
 *
 * @param originPos
 * @param finalPos
 * @param point
 *
 * @return true if point is in the corridor
 * @return false
	//   X1         
	//   | \  P
	//   |  \/ (90° ~ recta normal)
	//   |   \
	//   |    X2
	// recta entre X1 Y X2   (M = (y2-y1) / (x2-x1) )
	// normal a la recta q cruze con P  (N = -1/M)   (N (xP-xa) + ya = yP )
	// interseccion de rectas y punto P  (M (x2-xa) + ya = y2 )
	//  N (xP-xa) -yP = M (x2-xa) -y2 --->  xa = (Mx2+xp/M)-y2-yp)/(M+1/M) ---> ya = y2 + M(x2-xa)
	// si la distancia es menor a displacement esta adentro
 */
bool betweenTwoLines(Vector2 originPos, Vector2 finalPos, Vector2 point, float displacement)
{
	if (originPos.x < finalPos.x)
	{
		if ((point.x) > originPos.x && (point.x) < finalPos.x)
		{
			if (originPos.y < finalPos.y)
			{
				if ((point.y) < originPos.y || (point.y) > finalPos.y)
					return false;
			}
			else
			{
				if ((point.y) > originPos.y || (point.y) < finalPos.y)
					return false;
			}
		}
		else
			return false;
	}
	else
	{
		if ((point.x) < originPos.x && (point.x) > finalPos.x)
		{
			if (originPos.y < finalPos.y)
			{
				if ((point.y) < originPos.y || (point.y) > finalPos.y)
					return false;
			}
			else
			{
				if ((point.y) > originPos.y || (point.y) < finalPos.y)
					return false;
			}
		}
		else
			return false;
	}

	float deltaY = (finalPos.y - originPos.y);
	float deltaX = (finalPos.x - originPos.x);

	if (MODULE(deltaY) < 0.05)
	{
		if (MODULE(point.y - originPos.y) < MODULE(displacement))
			return true;
	}
	else if (MODULE(deltaX) < 0.05)
	{
		if (MODULE(point.x - originPos.x) < MODULE(displacement))
			return true;
	}
	else
	{
		float M = (deltaY / deltaX);
		Vector2 intersection;
		intersection.x = (((M * finalPos.x) + (point.x / M) - finalPos.y - point.y) / (M + (1 / M)));
		intersection.y = finalPos.y - (M * (finalPos.x - intersection.x));
		float distPointInter = distanceOfCoords(point, intersection);

		if (distPointInter <= displacement)
			return true;
	}

	return false;
}

/**
 * @brief gets the distance from 2 coordinates
 *
 * @param firstCoord
 * @param secondCoord
 */
float distanceOfCoords(Vector2 firstCoord, Vector2 secondCoord)
{
	float deltaX = firstCoord.x - secondCoord.x;
	float deltaZ = firstCoord.y - secondCoord.y;
	float square = (deltaX * deltaX) + (deltaZ * deltaZ);
	float distance = sqrt(square);

	return distance;
}