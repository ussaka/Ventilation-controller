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

	static void poll(unsigned ms)
	{
		for(unsigned i = 0; i < count; i++)
		{
			//	TODO if there are multiple instances, divide ms by the count
			int rc = MQTTYield(&instances[i]->client, ms);
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

void Networking::publish(const char* topic, const std::string& data)
{
	MQTTMessage message;
	char payload[256];

	message.qos = QOS1;
	message.retained = 0;
	message.payload = payload;
	sprintf(payload, "%s", data.c_str());
	message.payloadlen = strlen(payload);

	int rc = MQTTPublish(&client, topic, &message);
	if(rc != 0)
	{
		output.print("Return code from MQTT publish is ", rc);
		return;
	}
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

void Networking::poll(unsigned ms)
{
	NetworkingStorage::poll(ms);
}
