#ifndef NETWORKING_H_
#define NETWORKING_H_

#include "external/mqtt/MQTT_lpc1549.h"
#include "external/mqtt/MQTTClient.h"

#include <functional>
#include <string>
#include <array>

typedef std::function <void(const std::string& data)> MessageCallback;

class Networking
{
public:
	Networking(const char* ssid, const char* pass, const char* broker);
	void close();

	friend class NetworkingStorage;

	const static unsigned maxTopics = 5;
	const static unsigned maxInstances = 5;

	bool subscribe(const char* topic, const MessageCallback& cb);
	static void poll();

private:
	Network network;

	unsigned char sendbuf[256];
	unsigned char readbuf[256];

protected:
	std::array <std::pair <const char*, MessageCallback>, maxTopics> topics;
	unsigned topicCount = 0;

	MQTTClient client;
};

#endif /* NETWORKING_H_ */
