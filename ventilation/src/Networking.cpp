#include "external/ITM_Wrapper.h"
#include "Networking.h"

#include <cstring>

static ITM_Wrapper output;

/*	NetworkingStorage is a class that holds every instance
 * 	of "Networking". For this project it isn't really necessary
 * 	but if there were multiple instances of "Networking", this
 * 	class is helpful for mass polling for MQTT traffic.
 *
 *	The MQTT client also requires a callback when subscribing to
 *	some topic. Problem is that the callback cannot point to
 *	a class member function and cannot capture anything, so we
 *	cannot use said callback itself to check which client a
 *	message should go to. This class takes that message and forwards
 *	the message to whoever is subscribed to the topic */
class NetworkingStorage
{
public:
	static void add(Networking* networking)
	{
		//	Don't save instances past the maximum
		if(count >= Networking::maxInstances)
			return;

		instances[count] = networking;
		count++;
	}

	static void poll(unsigned ms)
	{
		//	Poll each client
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
		//	Loop through each instance
		for(unsigned i = 0; i < count; i++)
		{
			//	Loop through each topic of the instance
			for(unsigned t = 0; t < instances[i]->topicCount; t++)
			{
				//	Is this message targeted to this topic of this instance
				if(	strcmp(data->topicName->cstring,
					instances[i]->topics[t].first))
				{
					//	Call the message callback if one is set
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
