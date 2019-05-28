# MQTTClientLib
MQTT Client Library in C

This library handles all MQTT Subscribe and Publish requests.

Using the wrapper API's provided by the library, application can be written on top of this which can act as either Publisher Or Subscriber Or both.

API's Provided by the Library,

1)NX_STATUS NXI_init_lib(mqtt_broker_handle_t *broker,nx_packet *nx_pkt);
Syntax:
	NX_STATUS NXI_init_lib(mqtt_broker_handle_t *,nx_packet *);
Parameters:
  mqtt_broker_handle_t * : pointer to mqtt_broker_handle_t structure
  nx_packet * : pointer to nx_packet structure
Return:
  NX_STATUS values.
  NX_STATUS_SUCCESS if success or other corresponding values if it fails.
Summary :
  This function does the following operations,
  Initializes library and HAL.
  Creates packet pool,ip instance, enables ip fragmentation table, sets gateway address, enables ARP, TCP, ICMP (Hardware dependent Initialization).
  Creates TCP Socket, Binds and Connects to the TCP socket.
  Returns with NX_STATUS value.
  Every information will be stored in mqtt_broker_handle_t structure pointer passed as parameter.

2)	NX_STATUS mqtt_connect_start(mqtt_broker_handle_t *broker);
Syntax:
	NX_STATUS mqtt_connect_start(mqtt_broker_handle_t *);
Parameters:
  mqtt_broker_handle_t * : pointer to mqtt_broker_handle_t structure
Return:
  NX_STATUS values.
  NX_STATUS_SUCCESS if success or other corresponding values if it fails.
Summary :
  This function does the following operations,
  Sets keep alive, username and password.
  Sends MQTT connect request.
  Returns NX_STATUS value.
  If connected properly, then ACK will be recieved from the server.

3)	int mqtt_publish(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t retain);
Syntax:
  int mqtt_publish(mqtt_broker_handle_t* , const char* , const char* , uint8_t);
Parameters:
mqtt_broker_handle_t * : pointer to mqtt_broker_handle_t structure
const char * : pointer holding topic name.
Const char * : pointer holding the message to be published with respect to topic.
uint8_t : 0 - retain
          1 - Do not retain 
Return:
  NX_STATUS values.
  NX_STATUS_SUCCESS if success or other corresponding values if it fails.
Summary :
  Sends publish request with provided topic and message information.
  Returns NX_STATUS value.

4)	int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t retain, uint8_t qos, uint16_t* message_id);
Syntax:
  int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* , const char* , uint8_t , uint8_t , uint16_t*);
Parameters:
  mqtt_broker_handle_t * : pointer to mqtt_broker_handle_t structure
  const char * : pointer holding topic name.
  Const char * : pointer holding the message to be published with respect to topic.
  uint8_t : 0 - retain
            1 - Do not retain 
  uint8_t : 1 - QOS level 1
            2 - QOS level 2
  uint16_t* : pointer holding message id.
Return:
  NX_STATUS values.
  NX_STATUS_SUCCESS if success or other corresponding values if it fails.
Summary :
  Sends publish request with provided topic and message information.
  Returns NX_STATUS value.
  Acknowledge will be sent from the server based on the QOS level selected.

4.3.5	int mqtt_subscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id);
Syntax:
  mqtt_subscribe(mqtt_broker_handle_t* , const char* , uint16_t*);
Parameters:
  mqtt_broker_handle_t * : pointer to mqtt_broker_handle_t structure
  const char * : pointer holding topic name.
  uint16_t* : pointer holding message id.
Return:
  NX_STATUS values.
  NX_STATUS_SUCCESS if success or other corresponding values if it fails.
Summary :
  Sends subscribe request with provided topic and message information.
  Returns NX_STATUS value.
  Acknowledge will be sent from the server.
  Note : If any data is sent from the publisher over the subscribed topic, data will be avilable over the TCP socket. It can be     retrieved using TCP read socket API.

6) int mqtt_unsubscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id);
Syntax:
  int mqtt_unsubscribe(mqtt_broker_handle_t*, const char* , uint16_t*);
Parameters:
  mqtt_broker_handle_t * : pointer to mqtt_broker_handle_t structure
  const char * : pointer holding topic name.
  uint16_t* : pointer holding message id.
Return:
  NX_STATUS values.
  NX_STATUS_SUCCESS if success or other corresponding values if it fails.
Summary :
  Sends unsubscribe request with provided topic and message information.
  Returns NX_STATUS value.

