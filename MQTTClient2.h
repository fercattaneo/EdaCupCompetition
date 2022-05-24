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

#ifndef _MQTTCLIENT_H
#define _MQTTCLIENT_H

#include <string>
#include <vector>

#include <mosquitto.h>

using namespace std;

typedef struct
{
    string topic;
    vector<char> payload;
} MQTTMessage;

/**
 * @brief ...
 *
 */
class MQTTListener
{
public:
    virtual void onMessage(string topic, vector<char> payload) = 0;
};

/**
 * @brief Manages an MQTT client connection.
 *
 */
class MQTTClient2
{
public:
    MQTTClient2();
    ~MQTTClient2();

    bool connect(std::string clientId, std::string host, int port, std::string username, std::string password);
    bool isConnected();
    void disconnect();

    bool publish(std::string topic, std::vector<char> &payload);

    bool subscribe(std::string topic);
    bool unsubscribe(std::string topic);

    void setListener(MQTTListener *listener);
    void run();

private:
    struct mosquitto *mosquittoInstance;
    MQTTListener *listener;

    friend void onMQTTMessage(struct mosquitto *mosquittoClient,
                              void *context,
                              const struct mosquitto_message *message);
};

#endif
