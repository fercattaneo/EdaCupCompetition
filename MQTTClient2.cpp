/**
 * @file MQTTClient.cpp
 * @author Marc S. Ressl
 * @brief Simple MQTT client with callback
 * @version 2.0
 *
 * @copyright Copyright (c) 2022
 *
 * @cite libmosquitto documentation: https://mosquitto.org/api/files/mosquitto-h.html
 *
 */

#include <cstring>
#include <iostream>

static void onMQTTMessage(struct mosquitto *mosquittoClient,
                          void *context,
                          const struct mosquitto_message *message);

#include "MQTTClient2.h"

using namespace std;

// Awful but necessary global variable:
static bool isMosquittoInitialized = false;

/**
 * @brief MQTT message callback.
 *
 */
static void onMQTTMessage(struct mosquitto *mosquittoClient,
                          void *context,
                          const struct mosquitto_message *message)
{
    MQTTClient2 *mqttClient2 = (MQTTClient2 *)context;

    if (mqttClient2->listener == NULL)
        return;

    vector<char> payload;
    payload.resize(message->payloadlen);
    memcpy(payload.data(), message->payload, message->payloadlen);

    mqttClient2->listener->onMessage(message->topic, payload);
}

/**
 * @brief Construct a new MQTTClient::MQTTClient object
 *
 */
MQTTClient2::MQTTClient2()
{
    if (!isMosquittoInitialized)
    {
        mosquitto_lib_init();

        isMosquittoInitialized = true;
    }

    mosquittoInstance = NULL;
}

/**
 * @brief Destroy the MQTTClient::MQTTClient object
 *
 */
MQTTClient2::~MQTTClient2()
{
    disconnect();
}

/**
 * @brief Connects to an MQTT server without encryption.
 *
 * @param clientId MQTT client identifier
 * @param host Host to connect to (IP address or domain name)
 * @param port TCP port of MQTT server
 * @param username MQTT username
 * @param password MQTT password
 * @return true Success
 * @return false Failure
 */
bool MQTTClient2::connect(string clientId, string host, int port, string username, string password)
{
    disconnect();

    const bool cleanSession = true;

    mosquittoInstance = mosquitto_new(clientId.c_str(), cleanSession, this);

    if (mosquittoInstance == NULL)
        return false;

    mosquitto_message_callback_set(mosquittoInstance, onMQTTMessage);

    int errorCode;

    mosquitto_username_pw_set(mosquittoInstance,
                              username.c_str(),
                              password.c_str());

    const int keepalive = 60;

    errorCode = mosquitto_connect(mosquittoInstance,
                                  host.c_str(),
                                  port,
                                  keepalive);

    if (errorCode != MOSQ_ERR_SUCCESS)
    {
        disconnect();

        return false;
    }

    return true;
}

/**
 * @brief Connection up?
 *
 * @return true Connection is up
 * @return false Connection is down
 */
bool MQTTClient2::isConnected()
{
    return mosquittoInstance != NULL;
}

/**
 * @brief Disconnects from the MQTT server.
 *
 */
void MQTTClient2::disconnect()
{
    if (!mosquittoInstance)
        return;

    mosquitto_disconnect(mosquittoInstance);

    mosquitto_destroy(mosquittoInstance);

    mosquittoInstance = 0;
}

/**
 * @brief Publishes an MQTT message on the server.
 *
 * @param topic The MQTT topic
 * @param payload The data to be sent
 * @return true Message sent
 * @return false Message not sent
 */
bool MQTTClient2::publish(string topic, vector<char> &payload)
{
    if (!mosquittoInstance)
        return false;

    const int qos = 0;
    const bool retain = false;

    int errorCode = mosquitto_publish(mosquittoInstance,
                                      NULL,
                                      topic.c_str(),
                                      (int)payload.size(),
                                      payload.data(),
                                      qos,
                                      retain);

    if (errorCode == MOSQ_ERR_NO_CONN)
        disconnect();

    return (errorCode == MOSQ_ERR_SUCCESS);
}

/**
 * @brief Sends an MQTT subscription request.
 *
 * @param topic The MQTT topic
 * @return true Call successful
 * @return false Call failed
 */
bool MQTTClient2::subscribe(string topic)
{
    if (!mosquittoInstance)
        return false;

    const int qos = 0;

    int errorCode = mosquitto_subscribe(mosquittoInstance,
                                        NULL,
                                        topic.c_str(),
                                        qos);

    if (errorCode == MOSQ_ERR_NO_CONN)
        disconnect();

    return (errorCode == MOSQ_ERR_SUCCESS);
}

/**
 * @brief Send an MQTT unsubscription request. Should match a previous subscription request.
 *
 * @param topic The MQTT topic
 * @return true Call successfull
 * @return false Call failed
 */
bool MQTTClient2::unsubscribe(string topic)
{
    if (!mosquittoInstance)
        return false;

    int errorCode = mosquitto_unsubscribe(mosquittoInstance,
                                          NULL,
                                          topic.c_str());

    if (errorCode == MOSQ_ERR_NO_CONN)
        disconnect();

    return (errorCode == MOSQ_ERR_SUCCESS);
}

/**
 * @brief Sets the MQTT message listener.
 *
 * @param listener The MQTT message listener
 */
void MQTTClient2::setListener(MQTTListener *listener)
{
    this->listener = listener;
}

/**
 * @brief Runs the MQTT message loop.
 *
 */
void MQTTClient2::run()
{
    if (mosquittoInstance)
    {
        const int timeout = 0;
        const int maxPackets = 1;

        int errorCode = mosquitto_loop_forever(mosquittoInstance, timeout, maxPackets);

        if ((errorCode == MOSQ_ERR_NO_CONN) ||
            (errorCode == MOSQ_ERR_CONN_LOST))
            disconnect();
    }
}