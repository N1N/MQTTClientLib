#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "mqttlib.h"
#include "common.h"
#include "nx_api.h"

#ifdef INT_RCV
void recieve_callback(NX_TCP_SOCKET *socket_ptr)
{

    /* ToDo : Callback when multiple sockets are used */

}
#endif


ULONG read_packet(void *mqtt_context_ptr,int timeout,uint8_t * buffer)
{

    mqtt_config_params_h *mqtt_context;
    char val[5] = {0};
    UINT status = 0;
    //NX_PACKET * packet_ptr;
    ULONG bytes = 0;
    mqtt_context = (mqtt_config_params_h *)mqtt_context_ptr;
    /* Allocate a packet.  */

    status =  nx_tcp_socket_receive(&mqtt_context->my_socket, &mqtt_context->rcv_packet_ptr,2000);

	if (status){
			strcpy(log_str,"Read : nx_tcp_socket_receive failed \n");
			debug_view(log_str);
		    //debug_view("receive ");
		    //sprintf(val,"%du",status);
		    //debug_view(val);
            //debug_view("dump \n");
		    nx_packet_release(mqtt_context->rcv_packet_ptr);
			mqtt_context->read_write_error = NX_SOCKET_RECEIVE_FAILED;
			return NX_STATUS_FAILURE;
	 }

    status = nx_packet_data_retrieve(mqtt_context->rcv_packet_ptr,buffer,&bytes);
	  
	if (status){
			strcpy(log_str,"Read : nx_packet_data_retrieve failed \n");
			debug_view(log_str);
		    nx_packet_release(mqtt_context->rcv_packet_ptr);
			mqtt_context->read_write_error = NX_DATA_RETRIEVE_FAILED;
			return NX_STATUS_FAILURE;
	 }
    status = nx_packet_release(mqtt_context->rcv_packet_ptr);

	 mqtt_fsm(mqtt_context,MQTTParseMessageType(buffer),buffer);
     mqtt_context->read_write_error = NX_STATUS_SUCCESS;

	 return bytes;


}


int send_packet(void *mqtt_context_ptr, const void* buf, unsigned int count)
{
    UINT status = 0;
    char val[5] = {0};
    mqtt_config_params_h *mqtt_context;
    mqtt_context = (mqtt_config_params_h *)mqtt_context_ptr;


    /* Allocate a packet.  */
    status =  nx_packet_allocate(&mqtt_context->nx_pkt->pool_0, &mqtt_context->send_packet_ptr, NX_TCP_PACKET, 500);

    /* Check for error.  */
	if (status){
			strcpy(log_str,"Send : nx_packet_allocate failed \n");
			debug_view(log_str);
			mqtt_context->read_write_error = NX_SOCKET_RECEIVE_ALLOC_FAILED;
			return NX_STATUS_FAILURE;
	}


     status = nx_packet_data_append(mqtt_context->send_packet_ptr, (void *)buf , count,&mqtt_context->nx_pkt->pool_0,
                             NX_WAIT_FOREVER);
	if (status){
			strcpy(log_str,"Send : nx_packet_data_append failed \n");
			debug_view(log_str);
			mqtt_context->read_write_error = NX_DATA_APPEND_FAILED;
			return NX_STATUS_FAILURE;
	}
						 
     status =  nx_tcp_socket_send(&mqtt_context->my_socket, mqtt_context->send_packet_ptr, NX_WAIT_FOREVER);

     /* Determine if the status is valid.  */
     if (status)
     {
        status = nx_packet_release(mqtt_context->send_packet_ptr);
		//strcpy(log_str,"Send : nx_tcp_socket_send failed \n");
		//debug_view(log_str);

		mqtt_context->read_write_error = NX_SOCKET_SEND_FAILED;
		
		nx_packet_release(mqtt_context->send_packet_ptr);
         /* queue the packet if needed  */
		return NX_STATUS_FAILURE;
		
     }

 	 mqtt_fsm(mqtt_context,MQTTParseMessageType((uint8_t *)buf),buf);

	 return (int)count;
}

void debug_view(char *log)
{
#if 1
    if(mqtt_context_unv->debug)
        mqtt_context_unv->debug(log);
#endif
}

