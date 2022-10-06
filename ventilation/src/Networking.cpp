#include "external/ITM_Wrapper.h"
#include "Networking.h"

#include <cstring>

static ITM_Wrapper output;

class NetworkingStorage
{
public:
	static void add(Networking* networking)
	{
		if(count >= Networking::maxInstances)
			return;

		instances[count] = networking;
		count++;
	}

	static void poll()
	{
		for(unsigned i = 0; i < count; i++)
		{
			int rc = MQTTYield(&instances[i]->client, 100);
			if(rc != 0)
				output.print("[", i, "] Return code from yield is ", rc);
		}
	}

	static void send(MessageData* data)
	{
		for(unsigned i = 0; i < count; i++)
		{
			for(unsigned t = 0; t < instances[i]->topicCount; t++)
			{
				if(	strcmp(data->topicName->cstring,
					instances[i]->topics[t].first))
				{
					if(instances[i]->topics[t].second)
					{
						std::string msg((const char*)data->message->payload, data->message->payloadlen);
						instances[i]->topics[t].second(msg);
					}
				}
			}
		}
	}

private:
	static std::array <Networking*, Networking::maxInstances> instances;
	static unsigned count;
};

std::array <Networking*, Networking::maxInstances> NetworkingStorage::instances;
unsigned NetworkingStorage::count = 0;

void messageArrived(MessageData* data)
{
    static ITM_Wrapper output;
    output.print("RECEIVED MESSAGE");
	NetworkingStorage::send(data);
}

Networking::Networking(const char* ssid, const char* pass, const char* broker)
{
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	NetworkInit(&network, ssid, pass);

	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	int rc = 0;
	const unsigned brokerPort = 1883;

	rc = NetworkConnect(&network, (char*)broker, brokerPort);
	if(rc != 0)
	{
		output.print("Return code from network connect is ", rc);
		return;
	}

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = (char*)"ventilation";

	rc = MQTTConnect(&client, &connectData);
	if(rc != 0)
	{
		output.print("Return code from MQTT connect is ", rc);
		return;
	}

	output.print("MQTT Connected");
	NetworkingStorage::add(this);
}

void Networking::close()
{
	NetworkDisconnect(&network);
}

bool Networking::subscribe(const char* topic, const MessageCallback& cb)
{
	int rc = MQTTSubscribe(&client, topic, QOS2, messageArrived);

	if(rc != 0)
	{
		output.print("Return code from MQTT subscribe is ", rc);
		return false;
	}

	if(topicCount >= maxTopics)
	{
		output.print("Max subscriptions reached");
		return false;
	}

	topics[topicCount].first = topic;
	topics[topicCount].second = cb;

	topicCount++;
	output.print("Subscribed to ", topic);

	return true;
}

void Networking::poll()
{
	NetworkingStorage::poll();
}
