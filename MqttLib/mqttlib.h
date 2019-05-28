#ifndef __LIBMQTTLIB__
#define __LIBMQTTLIB__

#include <stdint.h>
#include "nx_api.h"
#include "nx_dhcp.h"
#include "nx_dns.h"

#define RCVBUFSIZE 1024
uint8_t packet_buffer[RCVBUFSIZE];

#ifndef MQTT_CONF_USERNAME_LENGTH
	#define MQTT_CONF_USERNAME_LENGTH 13 // Recommended by MQTT Specification (12 + '\0')
#endif

#ifndef MQTT_CONF_PASSWORD_LENGTH
	#define MQTT_CONF_PASSWORD_LENGTH 13 // Recommended by MQTT Specification (12 + '\0')
#endif

#define MQTT_QOS0 0
#define MQTT_QOS1 1
#define MQTT_QOS2 2

#define MQTT_PROTO_VERSION 4

#define MQTT_PKT_TYPE_CONNECT       1
#define MQTT_PKT_TYPE_CONNACK       2
#define MQTT_PKT_TYPE_PUBLISH       3
#define MQTT_PKT_TYPE_PUBACK        4
#define MQTT_PKT_TYPE_PUBREC        5
#define MQTT_PKT_TYPE_PUBREL        6
#define MQTT_PKT_TYPE_PUBCOMP       7
#define MQTT_PKT_TYPE_SUBSCRIBE     8
#define MQTT_PKT_TYPE_SUBACK        9
#define MQTT_PKT_TYPE_UNSUBSCRIBE  10
#define MQTT_PKT_TYPE_UNSUBACK     11
#define MQTT_PKT_TYPE_PINGREQ      12
#define MQTT_PKT_TYPE_PINGRESP     13
#define MQTT_PKT_TYPE_DISCONNECT   14


#define MQTT_DUP_FLAG     1<<3
#define MQTT_QOS0_FLAG    0<<1
#define MQTT_QOS1_FLAG    1<<1
#define MQTT_QOS2_FLAG    2<<1

#define MQTT_RETAIN_FLAG  1

#define MQTT_CLEAN_SESSION  1<<1
#define MQTT_WILL_FLAG      1<<2
#define MQTT_WILL_QOS1      1<<3
#define MQTT_WILL_QOS2      1<<4
#define MQTT_WILL_RETAIN    1<<5
#define MQTT_PASSWORD_FLAG  1<<6
#define MQTT_USERNAME_FLAG  1<<7

#define MQTT_CONNECTION_ACCEPTED 0x00
#define MQTT_CONNECT_REJ_INV_PROTO 0x01
#define MQTT_CONNECT_REJ_INV_ID 0x02
#define MQTT_CONNECT_REJ_SERVER_ANAVAIL 0x03
#define MQTT_CONNECT_REJ_BAD_UN_PWD 0x04
#define MQTT_CONNECT_REJ_NOT_AUTH 0x05


#define MQTTParseMessageType(buffer) ((*buffer & 0xF0 ) >> 4)


#define MQTTParseMessageDuplicate(buffer) ( *buffer & 0x08 )


#define MQTTParseMessageQos(buffer) ( (*buffer & 0x06) >> 1 )


#define MQTTParseMessageRetain(buffer) ( *buffer & 0x01 )



uint8_t mqtt_get_rem_lenbytes(const uint8_t* buffer);

uint16_t mqtt_get_remaining_length(const uint8_t* buffer);

uint16_t mqtt_get_msg_id(const uint8_t* buffer);

uint16_t mqtt_get_pub_topic(const uint8_t* buffer, uint8_t* topic);

uint16_t mqtt_get_pub_topic_ptr(const uint8_t* buffer, const uint8_t **topic_ptr);

uint16_t mqtt_get_publish_msg(const uint8_t* buffer, uint8_t* msg);

uint16_t mqtt_get_publish_msg_ptr(const uint8_t* buffer, const uint8_t **msg_ptr);

uint8_t * frame_fixed_header(uint16_t rem_length,uint8_t retain,uint8_t qos,uint8_t dup,uint8_t type,uint8_t * fixed_header_len);

struct server_list{
    char *url;
    ULONG server_ip;
    UINT server_port;
};

static struct server_list mqttserver[4] = {{"iot.eclipse.org",IP_ADDRESS(198,41,30,241),1883},
                                  {"mqtt-dashboard.com",IP_ADDRESS(53,96,106,56),1883},
                                  {"m12.cloudmqtt.com",IP_ADDRESS(52,23,211,239),14471},
                                  {"test.mosquitto.org",IP_ADDRESS(85,119,83,194),1883}
                                 };

typedef struct{
NX_PACKET_POOL pool_0;
UCHAR          packet_pool_area[(1536 + 50 + sizeof(NX_PACKET)) * 50] __attribute__ ((aligned(4)));
NX_IP          ip_0;
UCHAR          ip_memory_area[2048]                                   __attribute__ ((aligned(4)));
UCHAR          arp_memory_area[1024]                                  __attribute__ ((aligned(4)));
NX_DHCP my_dhcp;
ULONG gateway_address;
ULONG server_ip_address;
ULONG board_ip_address;
ULONG network_mask;
ULONG ping_ip_address;
ULONG actual_bind_port;
ULONG socket_bind_port;
ULONG server_bind_port;
NX_DNS                  client_dns;
int dhcp_enable;
/* PATCH0001 */
ULONG physical_address_lsw;
ULONG phsyical_address_msw;
char server_url[50];
}nx_packet;

typedef enum mqttstateval{
    NOT_INITIALIZED = -1,
    SOCKET_CREATED = 50,
    BIND_SUCCESS,
    UN_BIND_SUCCESS,
    SOC_CONNECT_SUCCESS,
    SOC_DISCONNECTED,
    MQTT_CONNECTING,
    MQTT_CONNECTED,
    MQTT_DISCONNECTED
}mqttstates;


typedef struct {
    NX_TCP_SOCKET my_socket; //Modified
    NX_PACKET * send_packet_ptr;
    NX_PACKET * rcv_packet_ptr;
	int (*send)(void *mqtt_context, const void* buf, unsigned int count);
	ULONG (*receive)(void *mqtt_context,int timeout,uint8_t * buffer);//Added
	void (*debug)(char log[]);
	// Connection info
	char clientid[50];
	// Auth fields
	char username[MQTT_CONF_USERNAME_LENGTH];
	char password[MQTT_CONF_PASSWORD_LENGTH];
	// Will topic
    uint8_t willflag;
	uint8_t will_retain;
	uint8_t will_qos;
	uint8_t clean_session;
	uint8_t dup;
	// Management fields
	uint16_t seq;
	uint16_t alive;
	char * pubtopic;
	char * subtopic[10];
	char willtopic[50];
	char * msg;
	char willmsg[50];
    uint8_t qos;
	uint8_t retain;
	nx_packet *nx_pkt;
	mqttstates mqttstateinfo;
	uint8_t connect_status;
	int nx_api_errorcode;
	int read_write_error;
	int mqtt_context_init_done;
	int mqtt_context_init_exit;
	int socket_disconnected;
	int rcvfsm;
	int sendfsm;
} mqtt_config_params_h;

typedef mqtt_config_params_h Network;

void mqtt_init(mqtt_config_params_h* mqtt_context, const char* clientid);

void mqtt_set_auth(mqtt_config_params_h* mqtt_context, const char* username, const char* password);

void mqtt_set_keepalive(mqtt_config_params_h* mqtt_context, uint16_t alive);

int mqtt_connect(mqtt_config_params_h* mqtt_context);

int mqtt_disconnect(mqtt_config_params_h* mqtt_context);

int mqtt_publish(mqtt_config_params_h* mqtt_context, const char* topic, const char* message, uint8_t retain, uint8_t qos, uint16_t* message_id);

int mqtt_publish_data(mqtt_config_params_h* mqtt_context, const char* topic, const char* msg, uint8_t retain, uint8_t qos, uint16_t* message_id);

int mqtt_publish_with_qos(mqtt_config_params_h* mqtt_context, const char* topic, const char* msg, uint8_t retain, uint8_t qos, uint16_t* message_id);

int mqtt_puback(mqtt_config_params_h* mqtt_context, uint16_t message_id);

int mqtt_rec(mqtt_config_params_h* mqtt_context, uint16_t message_id);

int mqtt_rel(mqtt_config_params_h* mqtt_context, uint16_t message_id);

int mqtt_pubcomp(mqtt_config_params_h* mqtt_context, uint16_t message_id);

int mqtt_subscribe(mqtt_config_params_h* mqtt_context, const char* topic, uint16_t* message_id);

int mqtt_unsubscribe(mqtt_config_params_h* mqtt_context, const char* topic, uint16_t* message_id);

int mqtt_ping(mqtt_config_params_h* mqtt_context);

int mqtt_receive_subscription(mqtt_config_params_h* mqtt_context,int timeout,char * pkt_buffer,char **topic,char **msg_ptr);

void pkt_dump(uint8_t *buf,uint16_t len);

#endif

//__LIBMQTTLIB__
