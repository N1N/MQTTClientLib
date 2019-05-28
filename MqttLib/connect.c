#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "mqttlib.h"
#include "common.h"
#include "nx_api.h"

NX_STATUS create_socket(mqtt_config_params_h* mqtt_context,void * dis)
{
		NX_STATUS status = NX_STATUS_SUCCESS;
     /* Create a socket.  */
        status =  nx_tcp_socket_create(&mqtt_context->nx_pkt->ip_0, &mqtt_context->my_socket, (CHAR *) "Echo PubClient Socket",
                                    NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 512,
                                    NX_NULL, dis);
        if(status){
			strcpy(log_str," nx_tcp_socket_create Failed \n");
			debug_view(log_str);
			mqtt_context->nx_api_errorcode = status;
			return NX_SOCKET_CREATE_FAILED;
		}
        else
            return status;
}

NX_STATUS socket_bind(mqtt_config_params_h* mqtt_context,UINT port)
{
	NX_STATUS status = NX_STATUS_SUCCESS;
    /* Bind the port to the TCP socket  */
    status =  nx_tcp_client_socket_bind(&mqtt_context->my_socket, port, NX_WAIT_FOREVER);

    if(status){
        strcpy(log_str," nx_tcp_client_socket_bind Failed \n");
		debug_view(log_str);
		mqtt_context->nx_api_errorcode = status;
		return NX_SOCKET_BIND_FAILED;
	}
        else
            return status;
}

NX_STATUS socket_unbind(mqtt_config_params_h* mqtt_context)
{
    NX_STATUS status = NX_STATUS_SUCCESS;
    /* Bind the port to the TCP socket  */
    status =  nx_tcp_client_socket_unbind(&mqtt_context->my_socket);

    if(status){
        strcpy(log_str," nx_tcp_client_socket_unbind Failed \n");
        debug_view(log_str);
        mqtt_context->nx_api_errorcode = status;
        return NX_SOCKET_BIND_FAILED;
    }
        else
            return status;
}



NX_STATUS client_connect(mqtt_config_params_h* mqtt_context,ULONG server_ip,UINT server_port)
{
    int i = 0, j = 0;
    char index[2];
    char ip[10] = {0};
    ULONG actual_bind_port;
    char port[6] = {0};
	NX_STATUS status = NX_STATUS_SUCCESS;
    strcpy(log_str,"Connecting to Preferred server - ");
    for(i = 0 ; i < 4 ; i++)
               if(server_ip == mqttserver[i].server_ip)
                   strcat(log_str,mqttserver[i].url);
    if(i>4){
        sprintf(ip,"%d",mqttserver[i].server_ip);
        strcat(log_str,ip);
    }

    strcat(log_str," on client port ");
    sprintf(port,"%lu",mqtt_context->nx_pkt->socket_bind_port);
    strcat(log_str,port);
    strcat(log_str,"\n");
    strcat(log_str,"\n");
    debug_view(log_str);

    actual_bind_port = mqtt_context->nx_pkt->socket_bind_port;
    status =  nx_tcp_client_socket_connect(&mqtt_context->my_socket, server_ip, server_port, NX_WAIT_FOREVER);
    if(status){
        strcpy(log_str,"Not able to connect to Preferred server - ");
        for(i = 0 ; i < 4 ; i++)
            if(server_ip == mqttserver[i].server_ip)
                strcat(log_str,mqttserver[i].url);
            debug_view(log_str);

            strcpy(log_str,"reconnecting ... \n");
            debug_view(log_str);

            status = reconnect(mqtt_context);
            if(status){
                status = reconnect_default(mqtt_context);
                if(status)
                    debug_view(" Connection failed. Please reboot the board!!");
            }

	}

    else{
        debug_view("Connection successful \n ");
    }

    return status;
}


NX_STATUS reconnect(mqtt_config_params_h *mqtt_context){

    NX_STATUS status = NX_STATUS_SUCCESS;
    int i = 0, j = 0;
    char index[2];
    char port[6] = {0};
    char ip_address[20] = {0};


    for(i = 0 ; i< 10; i++){

    mqtt_context->nx_pkt->socket_bind_port +=1;

    if(mqtt_context->nx_pkt->socket_bind_port > (mqtt_context->nx_pkt->actual_bind_port + 30))
    {
        mqtt_context->nx_pkt->socket_bind_port = mqtt_context->nx_pkt->actual_bind_port;
    }

    status = socket_unbind(&mqtt_context->my_socket);
     if(status)
      return status;
     else mqtt_fsm(mqtt_context,UN_BIND_SUCCESS,NULL);

     status =  socket_bind(&mqtt_context->my_socket, mqtt_context->nx_pkt->socket_bind_port);
     if(status)
      return status;
     else
         mqtt_fsm(mqtt_context,BIND_SUCCESS,NULL);
    //status =  client_connect(mqtt_context,mqtt_context->nx_pkt->server_ip_address, mqtt_context->nx_pkt->server_bind_port);
     debug_view("Connecting to server - ");
     debug_view(mqtt_context->nx_pkt->server_url);
     debug_view(" on client port ");
     sprintf(port,"%lu",mqtt_context->nx_pkt->socket_bind_port);
     debug_view(port);
     debug_view("\n");

     status = nx_tcp_client_socket_connect(&mqtt_context->my_socket,mqtt_context->nx_pkt->server_ip_address, mqtt_context->nx_pkt->server_bind_port, NX_WAIT_FOREVER);
     if(status)
     {
         strcpy(log_str,"Connecting to preferred server failed on client port :  ");
         sprintf(port,"%lu",mqtt_context->nx_pkt->socket_bind_port);
         strcat(log_str,port);
         debug_view(log_str);
     }
     else{
         strcpy(log_str,"Successfully connected to ");
         //sprintf(ip_address,"%lu",mqtt_context->nx_pkt->server_ip_address);
         strcat(log_str,mqtt_context->nx_pkt->server_url);
         strcat(log_str," on client port ");
         sprintf(port,"%lu",mqtt_context->nx_pkt->socket_bind_port);
         strcat(log_str,port);
         strcat(log_str,"\n");
         debug_view(log_str);
         mqtt_fsm(mqtt_context,SOC_CONNECT_SUCCESS,NULL);
         return NX_STATUS_SUCCESS;
     }

    }

    mqtt_context->nx_pkt->socket_bind_port = mqtt_context->nx_pkt->actual_bind_port;

    debug_view(" reconnect Failed.. \n");
    mqtt_context->nx_api_errorcode = status;
    return NX_SOCKET_CONNECT_FAILED;
}

NX_STATUS reconnect_default(mqtt_config_params_h *mqtt_context){

    int i = 0, j= 0;
    ULONG actual_bind_port;
    NX_STATUS status = NX_STATUS_SUCCESS;
    actual_bind_port = mqtt_context->nx_pkt->socket_bind_port;

     debug_view("Not able to connect to Preferred server - ");
     debug_view(mqtt_context->nx_pkt->server_url);

     debug_view(" Trying to connect different servers \n");

     for(i = 0; i < 4; i++){

                //mqtt_context->nx_pkt->server_ip_address = mqttserver[i].server_ip;
                strcpy(mqtt_context->nx_pkt->server_url,mqttserver[i].url);
                debug_view(" dns_get_ip reconnect_default \n");
                dns_get_ip(mqtt_context);
                //mqtt_context->nx_pkt->server_bind_port = mqttserver[i].server_port;
                mqtt_context->nx_pkt->socket_bind_port = actual_bind_port;
                mqtt_context->nx_pkt->server_bind_port = mqttserver[i].server_port;

                status = reconnect(mqtt_context);
                if(status == NX_STATUS_SUCCESS)
                    break;
      }
      if( i > 3){
            strcpy(log_str," reconnect Failed.. Please reboot the board!! \n");
            debug_view(log_str);
            mqtt_context->nx_api_errorcode = status;
            return NX_SOCKET_CONNECT_FAILED;
            }
      return NX_STATUS_SUCCESS;
}

void disconnect_cli(NX_TCP_SOCKET * socket_echo1){
    NX_STATUS status = 0;
    strcpy(log_str," Server closed the connection \n");
    debug_view(log_str);
    mqtt_fsm(mqtt_context_unv,SOC_DISCONNECTED,NULL);
    return;
}

