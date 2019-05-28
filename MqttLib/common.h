#ifndef __COMMON__
#define __COMMON_

#include "nx_api.h"
#include "mqttlib.h"

//#define INT_RCV 1

uint8_t print_buff[15];
char read_buff[10];

char log_str[100];

void user_thread_entry   (void);
extern  void nx_ether_driver_eth1(NX_IP_DRIVER*);

typedef enum mqttstatus{
	NX_STATUS_SUCCESS = 0,
    NX_STATUS_FAILURE = -1,
	NX_PACKET_POOL_CREATE_FAILURE = 1,
	NX_IP_CREATE_FAILURE,
	NX_IP_FRAGMENT_ENABLE_FAILURE,
	NX_IP_GATEWAY_ADDRESS_SET_FAILURE,
	NX_ARP_ENABLE_FAILURE,
	NX_TCP_ENABLE_FAILURE,
	NX_UDP_ENABLE_FAILURE,
	NX_DHCP_START_FAILURE,
	NX_ICMP_ENABLE_FAILURE,
	NX_IP_STATUS_CHECK_FAILURE,
	NX_IP_ADDRESS_GET_FAILURE,
	NX_DNS_CREATE_FAILURE,
	NX_DSN_SERVER_ADD_FAILURE,
	NX_DNS_HOST_BY_NAME_GET_FAILURE,
    NX_SOCKET_CREATE_FAILED,
    NX_SOCKET_BIND_FAILED,
    NX_SOCKET_UNBIND_FAILED,
    NX_SOCKET_CONNECT_FAILED,
	NX_SOCKET_RECEIVE_ALLOC_FAILED,
	NX_SOCKET_RECEIVE_FAILED,
	NX_DATA_RETRIEVE_FAILED,
	NX_SOCKET_SEND_ALLOC_FAILED,
	NX_DATA_APPEND_FAILED,
	NX_SOCKET_SEND_FAILED,
	MQTT_CON_REQ_FAIL_INV_PROTO,
    MQTT_CON_REQ_FAIL_INV_ID,
    MQTT_CON_REQ_FAIL_SERVER_UNAVAIL,
    MQTT_CON_REQ_FAIL_BAD_UN_PWD,
    MQTT_CON_REQ_FAIL_NOT_AUTH_INV_UN_PWD,
    MQTT_CON_REQ_FAIL_UNKNOWN,
	MQTT_DISCONNECT_REQUEST_FAILED
}NX_STATUS;

mqtt_config_params_h * mqtt_context_unv;

void disconnect_cli(NX_TCP_SOCKET * socket_echo1);
int handle_disconnection(Network *net);
int manage_data(Network * network);
ULONG read_packet(void *mqtt_context,int timeout,uint8_t * buffer);
int send_packet(void *mqtt_context, const void* buf, unsigned int count);
void receive_callback(NX_TCP_SOCKET *socket_ptr);


NX_STATUS init_resources(mqtt_config_params_h *mqtt_context,nx_packet *nx_pkt);
NX_STATUS NXI_init_lib(mqtt_config_params_h *mqtt_context,nx_packet *nx_pkt);
NX_STATUS init_stack(mqtt_config_params_h *mqtt_context,nx_packet * nxi_pkt );
NX_STATUS init_socket(mqtt_config_params_h* mqtt_context, ULONG serverip /* const char* hostname */, UINT port);
NX_STATUS create_socket(mqtt_config_params_h* mqtt_context,void * dis);
NX_STATUS socket_bind(mqtt_config_params_h* mqtt_context,UINT port);
NX_STATUS client_connect(mqtt_config_params_h* mqtt_context,ULONG server_ip,UINT server_port);


void mqtt_fsm(mqtt_config_params_h *mqtt_context,int pktid,const uint8_t * buffer);
NX_STATUS mqtt_connect_start(mqtt_config_params_h *mqtt_context);
NX_STATUS reconnect(mqtt_config_params_h *mqtt_context);
NX_STATUS reconnect_default(mqtt_config_params_h *mqtt_context);
NX_STATUS dns_get_ip(mqtt_config_params_h* mqtt_context);

void debug_view(char *log);

#endif
