#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "mqttlib.h"
#include "common.h"
#include "nx_api.h"


void mqtt_fsm(mqtt_config_params_h *mqtt_context,int pktid,const uint8_t * buffer)
{
	uint16_t msg_id = 0;
	char msgid[10] = {0};
	switch(mqtt_context->mqttstateinfo)
	{
		case NOT_INITIALIZED:
			if(pktid == SOCKET_CREATED)
				mqtt_context->mqttstateinfo = SOCKET_CREATED;
		break;
		case SOCKET_CREATED:
				if(pktid == BIND_SUCCESS)
				mqtt_context->mqttstateinfo = BIND_SUCCESS;
		break;
		case BIND_SUCCESS:
			if(pktid == SOC_CONNECT_SUCCESS)
				mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
		break;
        case UN_BIND_SUCCESS:
                if(pktid == BIND_SUCCESS)
                mqtt_context->mqttstateinfo = BIND_SUCCESS;
        break;
		case SOC_CONNECT_SUCCESS:
				
				if(pktid == SOC_DISCONNECTED)
				mqtt_context->mqttstateinfo = SOC_DISCONNECTED;
				
				if(pktid == MQTT_PKT_TYPE_CONNECT){
				mqtt_context->mqttstateinfo = MQTT_CONNECTING;
			    //Recieve the CONNACK
			    mqtt_context->receive(mqtt_context,1,packet_buffer);
				}
			
		break;
		case SOC_DISCONNECTED:
				if(pktid == BIND_SUCCESS)
				mqtt_context->mqttstateinfo = BIND_SUCCESS;
                if(pktid == UN_BIND_SUCCESS)
                mqtt_context->mqttstateinfo = UN_BIND_SUCCESS;
		break;
		case MQTT_CONNECTING:
				if(pktid == MQTT_PKT_TYPE_CONNACK){
				    switch((buffer[3]))
				            {
				            case MQTT_CONNECTION_ACCEPTED:
				                mqtt_context->mqttstateinfo = MQTT_CONNECTED;
                                mqtt_context->connect_status = NX_STATUS_SUCCESS;
				                break;
				            case MQTT_CONNECT_REJ_INV_PROTO:
				                mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
				                mqtt_context->connect_status = MQTT_CON_REQ_FAIL_INV_PROTO;
				                break;
				            case MQTT_CONNECT_REJ_INV_ID:
                                mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
                                mqtt_context->connect_status = MQTT_CON_REQ_FAIL_INV_ID;
				                break;
				            case MQTT_CONNECT_REJ_SERVER_ANAVAIL:
                                mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
                                mqtt_context->connect_status = MQTT_CON_REQ_FAIL_SERVER_UNAVAIL;

				                break;
				            case MQTT_CONNECT_REJ_BAD_UN_PWD:
                                mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
                                mqtt_context->connect_status = MQTT_CON_REQ_FAIL_BAD_UN_PWD;
                                break;
				            case MQTT_CONNECT_REJ_NOT_AUTH:
                                mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
                                mqtt_context->connect_status = MQTT_CON_REQ_FAIL_NOT_AUTH_INV_UN_PWD;
                                break;
				            default:
                                mqtt_context->mqttstateinfo = SOC_CONNECT_SUCCESS;
                                mqtt_context->connect_status = MQTT_CON_REQ_FAIL_UNKNOWN;
                                break;
				            }
				}

		break;
		case MQTT_CONNECTED:
				switch(pktid)
				{
					case MQTT_PKT_TYPE_PUBLISH:
					/* Log for Publish request : QOS0(1/1),QOS1(1/2),QOS2(1/4) */
		                //Recieve the MQTT_PKT_TYPE_PUBACK for QOS1 or MQTT_PKT_TYPE_PUBREC for QOS2
					    debug_view("PUBLISH\n");
#if 0
					    if(MQTTParseMessageQos(buffer) == MQTT_QOS1 || MQTTParseMessageQos(buffer) == MQTT_QOS2){
		                    // Recive has to return atleast 2 bytes as per protocol
					        if((mqtt_context->recieve(mqtt_context,1,packet_buffer)) <= 1){
					            mqtt_context->seq--;
					            mqtt_context->dup = 1;
                            debug_view("publishackrcv <- retry pubqos -> \n");
					        mqtt_publish_with_qos(mqtt_context, mqtt_context->topic, mqtt_context->msg, mqtt_context->retain, mqtt_context->qos, NULL);
                            debug_view("publishackrcv <- retry pubqos <- \n");
					        }
					        mqtt_context->dup = 0;
					    }
#endif
					break;
					case MQTT_PKT_TYPE_PUBACK:
                        debug_view("PUBACK received \n");
					break;
					case MQTT_PKT_TYPE_PUBREC:
					/* Log for PUBLISH RECIEPT reply from the server : QOS2(2/4) */
					/* Send Publish Release reply to server */
                        debug_view("PUBREC received \n");
#if 0
                        msg_id = buffer[2] << 8;
                        msg_id |= buffer[3];
                        sprintf(msgid,"%lu",msg_id);
                        debug_log("msg_id :");
                        debug_log(msgid);
                        debug_log("\n");
#endif
#if 0
                        mqtt_pubrel(mqtt_context,mqtt_context->seq - 1);
#endif
					break;
					case MQTT_PKT_TYPE_PUBREL:
					/* Log for PUBLISH RELEASE request from the client to server : QOS2(3/4) */
					    //Recieve the MQTT_PKT_TYPE_PUBCOMP for QOS1
                        debug_view("PUBREL \n");
#if 0
					    if((mqtt_context->recieve(mqtt_context,1,packet_buffer)) <= 1){
					        mqtt_context->dup = 1;
	                        mqtt_pubrel(mqtt_context,mqtt_context->seq - 1);
					    }
                        mqtt_context->dup = 0;
#endif
					    break;
					case MQTT_PKT_TYPE_PUBCOMP:
                        debug_view("PUBCOMP received \n");
#if 0
                        msg_id = buffer[2] << 8;
                          msg_id |= buffer[3];
                          sprintf(msgid,"%lu",msg_id);
                          debug_log("msg_id :");
                          debug_log(msgid);
                          debug_log("\n");
#endif
					break;
					case MQTT_PKT_TYPE_SUBSCRIBE:
                        if(MQTTParseMessageQos(buffer) == MQTT_QOS1)
                        mqtt_context->receive(mqtt_context,1,packet_buffer);
					/* Log for Subscribe request */
					break;
					case MQTT_PKT_TYPE_SUBACK:
					/* ToDo : Set event or semaphore to complete subscribe request */
					break;
					case MQTT_PKT_TYPE_UNSUBSCRIBE:
					/* Log for UnSubscribe request */
                        if(MQTTParseMessageQos(buffer) == MQTT_QOS1)
                        mqtt_context->receive(mqtt_context,1,packet_buffer);
					break;
					case MQTT_PKT_TYPE_UNSUBACK:
					/* ToDo : Set event or semaphore to complete UnSubscribe request */
					break;
					case MQTT_PKT_TYPE_PINGREQ:
					/* Log for PING request */
					break;
					case MQTT_PKT_TYPE_PINGRESP:
					/* Log for PING response */
					break;
					case MQTT_PKT_TYPE_DISCONNECT:
						mqtt_context->mqttstateinfo = MQTT_DISCONNECTED;
					break;
#if 1
					case SOC_DISCONNECTED:
					    mqtt_context->mqttstateinfo = SOC_DISCONNECTED;
					    break;
#endif
				}
		break;
		case MQTT_DISCONNECTED:
				if(pktid == MQTT_PKT_TYPE_CONNECT)
					mqtt_context->mqttstateinfo = MQTT_CONNECTING;
		break;
		
	}
}
