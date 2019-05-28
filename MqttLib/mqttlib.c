#include <string.h>
#include <mqttlib.h>
#include "common.h"

int mqtt_connect(mqtt_config_params_h* mqtt_context){
	#define PROTONAMELEN 4
	uint16_t clientidlen,usernamelen,passwordlen,payloadlen = 0;
	uint16_t willtopiclen = 0;
	uint16_t willmsglen = 0;
	uint8_t connect_flags = 0x00;
	uint8_t * var_ptr, * fix_ptr;
	uint8_t * connect_packet, * index;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
	uint16_t connect_packet_size = 0;
	
	/* variable_header[2+PROTONAMELEN+1+1+2]
	   2 bytes - Protocol Name Length
	   PROTONAMELEN Bytes - Bytes to hold Protocol Name
	   1 byte - Protocol Version Number
	   1 byte - Connect flags
	   2 bytes - Keep alive  
	*/
	
	uint8_t variable_header[2+PROTONAMELEN+1+1+2]; 
	
	if(strlen(mqtt_context->clientid) > 0)
		clientidlen = (uint16_t)strlen(mqtt_context->clientid);
	else
		clientidlen = 0;
	
	if(strlen(mqtt_context->username) > 0){
		usernamelen = (uint16_t)strlen(mqtt_context->username);
		payloadlen = (uint16_t)(payloadlen + (usernamelen + 2));
		connect_flags |= MQTT_USERNAME_FLAG;
	}
	else
		usernamelen = 0;
	
	if(strlen(mqtt_context->password) > 0){
		passwordlen = (uint16_t)strlen(mqtt_context->password);
		payloadlen = (uint16_t)(payloadlen + (passwordlen + 2));
		connect_flags |= MQTT_PASSWORD_FLAG;
	}
	else
		passwordlen = 0;
	
	if(mqtt_context->clean_session) {
		connect_flags |= MQTT_CLEAN_SESSION;
	}
	
	payloadlen = (uint16_t)(payloadlen + clientidlen + 2);
	
	if(strlen(mqtt_context->willtopic) > 0){
	    willtopiclen = (uint16_t)strlen(mqtt_context->willtopic);
	}
	else{
	    debug_view("will topic not provided\n Will functionality disabled \n");
	    mqtt_context->willflag = 0;
	}

	if(strlen(mqtt_context->willmsg) > 0){
	    willmsglen = (uint16_t)strlen(mqtt_context->willmsg);
	}
	else{
            debug_view("will message not provided \n");
	}


	if(mqtt_context->willflag){
        payloadlen = (uint16_t)(payloadlen + willtopiclen + 2);
        payloadlen = (uint16_t)(payloadlen + willmsglen + 2);
        connect_flags |= MQTT_WILL_FLAG;
        connect_flags |= MQTT_WILL_RETAIN;
        if(mqtt_context->will_qos == 1)
            connect_flags |= MQTT_WILL_QOS1;
        else if(mqtt_context->will_qos == 2)
            connect_flags |= MQTT_WILL_QOS2;
        else
            connect_flags |= 0;
	}
	
	/* Fixed Header frame */
	
	remaining_length = (uint16_t)(sizeof(variable_header)+payloadlen);
	fix_ptr = frame_fixed_header(remaining_length,0,0,0,MQTT_PKT_TYPE_CONNECT,&fixed_header_len);
	
	
	/* Variable Header frame */
	
	memset(variable_header, 0, sizeof(variable_header));
	var_ptr = variable_header;
	/* Protocol name length */
	*var_ptr++ = 0x00;
	*var_ptr++ = 0x04;
	/* Protocol name */
	memcpy(var_ptr,"MQTT",4);
	var_ptr+=4;
	/* MQTT Protocol name */
	*var_ptr++ = MQTT_PROTO_VERSION;

	/* Connect Flags - 
		1b		1b		1b			2b		1b		1b		   1b
	username|password|willretain|willQOS|willflag|cleansession| x  
	*/
	*var_ptr++ = connect_flags;
	
	/* MQTT Keep alive MSB */
	*var_ptr++ = (uint8_t)(mqtt_context->alive>>8);
	/* MQTT Keep alive LSB */
	*var_ptr = (uint8_t) (mqtt_context->alive&0xFF);
	
	
	/* Frame Payload (All are optional fields) */
	
	uint8_t payload[payloadlen];
	uint8_t * payload_ptr = payload;
	
	*payload_ptr++ = (uint8_t) (clientidlen>>8);
	*payload_ptr++ = (uint8_t) (clientidlen&0xFF);
	/* Copy clientid */
	memcpy(payload_ptr,mqtt_context->clientid,clientidlen);
	payload_ptr += clientidlen;
	/* Copy clientid */
	if(mqtt_context->willflag){

	    if(willtopiclen > 0){
			/* 2 bytes - will topic length */
	        *payload_ptr++ = (uint8_t)(willtopiclen>>8);
	        *payload_ptr++ = (uint8_t)(willtopiclen&0xFF);
			/* Copy will topic */
	        memcpy(payload_ptr, mqtt_context->willtopic, willtopiclen);
	        payload_ptr += willtopiclen;
	    }
	    if(willmsglen > 0){
					/* 2 bytes - will msg length */
	                *payload_ptr++ = (uint8_t)(willmsglen>>8);
	                *payload_ptr++ = (uint8_t)(willmsglen&0xFF);
					/* Copy will msg */
	                memcpy(payload_ptr, mqtt_context->willmsg, willmsglen);
	                payload_ptr += willmsglen;
	            }
	}
	
	if(usernamelen) {
		/* 2 bytes - username length */
		*payload_ptr++ = (uint8_t)(usernamelen>>8);
		*payload_ptr++ = (uint8_t)(usernamelen&0xFF);

		/* Copy username */
		memcpy(payload_ptr, mqtt_context->username, usernamelen);
		payload_ptr += usernamelen;
	}

	if(passwordlen) {
		/* 2 bytes - password length */
		*payload_ptr++ = (uint8_t) (passwordlen>>8);
		*payload_ptr++ = (uint8_t) (passwordlen&0xFF);
		/* Copy password */
		memcpy(payload_ptr, mqtt_context->password, passwordlen);
		payload_ptr += passwordlen;
	}
	
	/* Frame Full Connect packet */
	
	connect_packet_size = (uint8_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	connect_packet = (uint8_t *)malloc(connect_packet_size);
	index = connect_packet;
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);

	/* Copy Payload */
	memcpy(index,payload,payloadlen);
	
	#if 0
            char val[10] = {0};
            uint8_t in = 0;

			debug_view("connect packet \n");
			for(in = 0; in < connect_packet_size;in++)
			{
			    sprintf(val,"%x",*(connect_packet+in));
			    debug_view(val);
                debug_view("\n");
			}
	#endif
	

	if(mqtt_context->send(&mqtt_context->my_socket, connect_packet, connect_packet_size) != connect_packet_size) {
		free(fix_ptr);
		free(connect_packet);
		return NX_STATUS_FAILURE;
	}
	
		free(fix_ptr);
		free(connect_packet);
		
		return mqtt_context->connect_status;
}

uint8_t * frame_fixed_header(uint16_t rem_length,uint8_t retain,uint8_t qos,uint8_t dup,uint8_t type,uint8_t * fixed_header_len){

uint8_t * ptr;
uint8_t length = 0;

if(rem_length > 127)
length = 2;
else
length = 1;

struct fixed_header{
uint8_t retain:1;
uint8_t qos:2;
uint8_t dup:1;
uint8_t type:4;
uint8_t remain_length[length];
};

struct fixed_header header;

ptr = (uint8_t *)malloc(sizeof(header));

header.retain = retain;
header.qos = qos;
header.dup = dup;
header.type = type;

if(rem_length > 127){
header.remain_length[0] = rem_length%128;
header.remain_length[1] = header.remain_length[1] | 0x80;
header.remain_length[2] = (uint8_t) (rem_length / 128);
}
else
header.remain_length[0] = (uint8_t) rem_length;

memcpy(ptr,(uint8_t *)&header,sizeof(header));

*fixed_header_len = (uint8_t) sizeof(header);

return ptr;
}

int mqtt_publish_data(mqtt_config_params_h* mqtt_context, const char* topic, const char* msg, uint8_t retain, uint8_t qos, uint16_t* message_id) {
    int status = 0;

     do{
         debug_view("dopub\n");
            status =  mqtt_publish(mqtt_context, topic, msg, retain, qos, message_id);
            if(mqtt_context->mqttstateinfo <= SOC_DISCONNECTED){
                return status;
            }
       }while(mqtt_context->dup);

    return status;
}

int mqtt_publish(mqtt_config_params_h* mqtt_context, const char* topic, const char* message, uint8_t retain, uint8_t qos, uint16_t* message_id){
	
	    uint16_t topiclen = (uint16_t) strlen(topic);
		uint16_t msglen = (uint16_t) strlen(message);
		uint8_t msgid_size = 0;
		uint8_t fixed_header_len = 0;
		uint16_t remaining_length = 0;
		uint8_t * var_ptr, * fix_ptr;
		uint8_t * publish_packet, * index;
		int status = 0;
		uint16_t publish_packet_size = 0;
		uint8_t payload[msglen];
		uint16_t payloadlen = 0;
		
	if(qos == 1 || qos == 2) {
	   /* 2 bytes for QoS */
		msgid_size = 2;
	}
	
	uint8_t variable_header[2+topiclen+msgid_size]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	payloadlen = msglen;
	remaining_length = (uint16_t) (sizeof(variable_header)+payloadlen);
	
	/* Fixed Header frame */
	
	fix_ptr = frame_fixed_header(remaining_length,retain,qos,mqtt_context->dup,MQTT_PKT_TYPE_PUBLISH,&fixed_header_len);
	
	/* Variable Header frame */
	
	var_ptr = variable_header;
	*var_ptr++ = (uint8_t)(topiclen >> 8);
	*var_ptr++ = (uint8_t)(topiclen & 0xFF);
	
	memcpy(var_ptr,topic,topiclen);
	var_ptr+=topiclen;
	
	if(msgid_size){
		*var_ptr++ = (uint8_t) (mqtt_context->seq >> 8);
		*var_ptr++ = (uint8_t) (mqtt_context->seq & 0xFF);
		mqtt_context->seq++;
		if(message_id) {
			*message_id = mqtt_context->seq;
		}
	}
	
	/* Frame Payload */
	memcpy(payload,message,msglen);
	
	publish_packet_size = (uint16_t) (fixed_header_len + sizeof(variable_header) + payloadlen);
	publish_packet = (uint8_t *)malloc(publish_packet_size);
	index = publish_packet;
	
	/* Frame Full publish packet */
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	memcpy(index,payload,payloadlen);

	//pkt_dump(publish_packet,publish_packet_size);

	if(mqtt_context->send(&mqtt_context->my_socket, publish_packet, publish_packet_size) != publish_packet_size) {
		free(fix_ptr);
		free(publish_packet);
		return NX_STATUS_FAILURE;
	}
		free(fix_ptr);
		free(publish_packet);
		
	if(qos == 1){
    debug_view("mqtt_publish_with_qos get ack for qos 1 \n");
	if((mqtt_context->receive(mqtt_context,2000,packet_buffer)) <= 1){
	 mqtt_context->seq--;
	 mqtt_context->dup = 1;
	    debug_view("mqtt_publish_with_qos <- duplicate req  \n");
	 return NX_STATUS_SUCCESS;
	  }
	}

    if(qos == 2){
    debug_view("mqtt_publish_with_qos get ack for qos 2 \n");
    if((mqtt_context->receive(mqtt_context,2000,packet_buffer)) <= 1){
     mqtt_context->seq--;
     mqtt_context->dup = 1;
        debug_view("mqtt_publish_with_qos <- duplicate req  \n");
     return NX_STATUS_SUCCESS;
      }
    else
    {
        mqtt_context->dup = 0;
        do{
            debug_view("dopubrel\n");
            status = mqtt_rel(mqtt_context,(uint16_t)(mqtt_context->seq - 1));
            if(status) return NX_STATUS_FAILURE;

            if((mqtt_context->receive(mqtt_context,2000,packet_buffer)) <= 1){
                mqtt_context->seq--;
                mqtt_context->dup = 1;
                   debug_view("mqtt_pubrel duplicate req  \n");
               // return NX_STATUS_SUCCESS;
            }
            else
                mqtt_context->dup = 0;
            if(mqtt_context->mqttstateinfo <= SOC_DISCONNECTED){
                return status;
            }
        }while(mqtt_context->dup);
    }

  }
  
  mqtt_context->dup = 0;
  debug_view("mqtt_publish_with_qos <- \n");

	return NX_STATUS_SUCCESS;	
}

int mqtt_puback(mqtt_config_params_h* mqtt_context, uint16_t message_id) {
    // Variable header
	uint8_t * fix_ptr;
	uint8_t * puback_packet, * index;
	uint16_t payloadlen = 0;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
	uint16_t puback_packet_size = 0;
	
	uint8_t variable_header[2]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	remaining_length = (uint16_t) (sizeof(variable_header) + payloadlen);
	
	/* Fixed Header frame */
	
	fix_ptr = frame_fixed_header(remaining_length,0,0,0,MQTT_PKT_TYPE_PUBACK,&fixed_header_len);
	
	/* Variable Header frame */
    variable_header[0] = (uint8_t)(message_id>>8);
    variable_header[1] = (uint8_t)(message_id&0xFF);

	/* Frame Payload */
	/* No payload */
	
	puback_packet_size = (uint16_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	puback_packet = (uint8_t *)malloc(puback_packet_size);
	index = puback_packet;
	
	/* Frame Full publish packet */
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	/* No Payload */
	
		if(mqtt_context->send(&mqtt_context->my_socket, puback_packet, puback_packet_size) != puback_packet_size) {
		free(fix_ptr);
		free(puback_packet);
		return NX_STATUS_FAILURE;
		}
		
		free(fix_ptr);
		free(puback_packet);


    return NX_STATUS_SUCCESS;
}


int mqtt_rec(mqtt_config_params_h* mqtt_context, uint16_t message_id) {
    // Variable header
	uint8_t * fix_ptr;
	uint8_t * pubrec_packet, * index;
	uint16_t payloadlen = 0;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
    uint16_t pubrec_packet_size = 0;
	
	uint8_t variable_header[2]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	remaining_length = (uint16_t)(sizeof(variable_header) + payloadlen);
	
	/* Fixed Header frame */
	
	fix_ptr = frame_fixed_header(remaining_length,0,0,0,MQTT_PKT_TYPE_PUBREC,&fixed_header_len);
	
	/* Variable Header frame */
    variable_header[0] = (uint8_t)(message_id>>8);
    variable_header[1] = (uint8_t)(message_id&0xFF);

	/* Frame Payload */
	/* No payload */
	
	pubrec_packet_size = (uint16_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	pubrec_packet = (uint8_t *)malloc(pubrec_packet_size);
	index = pubrec_packet;
	
	/* Frame Full publish packet */
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	/* No Payload */
	
		if(mqtt_context->send(&mqtt_context->my_socket, pubrec_packet, pubrec_packet_size) != pubrec_packet_size) {
		free(fix_ptr);
		free(pubrec_packet);
		return NX_STATUS_FAILURE;
		}
		
		free(fix_ptr);
		free(pubrec_packet);


    return NX_STATUS_SUCCESS;
}


int mqtt_rel(mqtt_config_params_h* mqtt_context, uint16_t message_id) {
	uint8_t * fix_ptr;
	uint8_t * pubrel_packet, * index;
	uint16_t payloadlen = 0;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
    uint16_t pubrel_packet_size = 0;
	
	uint8_t variable_header[2]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	remaining_length = (uint16_t)(sizeof(variable_header) + payloadlen);
	
	/* Fixed Header frame */
	
	fix_ptr = frame_fixed_header(remaining_length,0,MQTT_QOS1,mqtt_context->dup,MQTT_PKT_TYPE_PUBREL,&fixed_header_len);
	
	/* Variable Header frame */
    variable_header[0] = (uint8_t)(message_id>>8);
    variable_header[1] = (uint8_t)(message_id&0xFF);

	/* Frame Payload */
	/* No payload */
	
	pubrel_packet_size = (uint16_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	pubrel_packet = (uint8_t *)malloc(pubrel_packet_size);
	index = pubrel_packet;
	
	/* Frame Full publish packet */
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	/* No Payload */
	//pkt_dump(pubrel_packet,pubrel_packet_size);

		if(mqtt_context->send(&mqtt_context->my_socket, pubrel_packet, pubrel_packet_size) != pubrel_packet_size) {
		free(fix_ptr);
		free(pubrel_packet);
		return NX_STATUS_FAILURE;
		}
		
		free(fix_ptr);
		free(pubrel_packet);


    return NX_STATUS_SUCCESS;
}


int mqtt_pubcomp(mqtt_config_params_h* mqtt_context, uint16_t message_id) {
	uint8_t * fix_ptr;
	uint8_t * pubcomp_packet, * index;
	uint16_t payloadlen = 0;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
    uint16_t pubcomp_packet_size = 0;
	
	uint8_t variable_header[2]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	remaining_length = (uint16_t)(sizeof(variable_header) + payloadlen);
	
	/* Fixed Header frame */
	
	fix_ptr = frame_fixed_header(remaining_length,0,0,0,MQTT_PKT_TYPE_PUBCOMP,&fixed_header_len);
	
	/* Variable Header frame */
    variable_header[0] = (uint8_t)(message_id>>8);
    variable_header[1] = (uint8_t)(message_id&0xFF);

	/* Frame Payload */
	/* No payload */
	
	pubcomp_packet_size = (uint16_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	pubcomp_packet = (uint8_t *)malloc(pubcomp_packet_size);
	index = pubcomp_packet;
	
	/* Frame Full publish packet */
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	/* No Payload */
	
		if(mqtt_context->send(&mqtt_context->my_socket, pubcomp_packet, pubcomp_packet_size) != pubcomp_packet_size) {
		free(fix_ptr);
		free(pubcomp_packet);
		return NX_STATUS_FAILURE;
		}
		
		free(fix_ptr);
		free(pubcomp_packet);


    return NX_STATUS_SUCCESS;
}


int mqtt_subscribe(mqtt_config_params_h* mqtt_context, const char* topic, uint16_t* message_id) {
	uint8_t * fix_ptr;
	uint8_t * subscribe_packet, * index,* payload_ptr;
	uint16_t payloadlen = 0;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
    uint16_t subscribe_packet_size = 0;
	uint16_t topiclen = (uint16_t)strlen(topic);
	uint8_t payload[topiclen+3]; /* topiclen(2 bytes) + topic (topiclen bytes) + QOS (1 byte) */

	uint8_t variable_header[2]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	payloadlen = (uint16_t)(sizeof(payload));
	remaining_length = (uint16_t)(sizeof(variable_header) + payloadlen);
		
	/* Fixed Header frame */
	
	fix_ptr = frame_fixed_header(remaining_length,0,MQTT_QOS1,0,MQTT_PKT_TYPE_SUBSCRIBE,&fixed_header_len);
	
	/* Variable Header frame */
	variable_header[0] = (uint8_t)(mqtt_context->seq>>8);
	variable_header[1] = (uint8_t)(mqtt_context->seq&0xFF);
	if(message_id) { // Returning message id
		*message_id = mqtt_context->seq;
	}
	mqtt_context->seq++;
	
	/* Frame Payload */
	payload_ptr = payload;	
	memset(payload, 0, sizeof(payload));
	
	*payload_ptr++ = (uint8_t)(topiclen>>8);
	*payload_ptr++ = (uint8_t)(topiclen&0xFF);
	
	memcpy(payload_ptr, topic, topiclen);
	payload_ptr+=topiclen;

	memcpy(payload_ptr,&mqtt_context->qos,1);

	/* Frame Full publish packet */
	
	subscribe_packet_size = (uint16_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	subscribe_packet = (uint8_t *)malloc(subscribe_packet_size);
	index = subscribe_packet;
	
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	memcpy(index,payload,payloadlen);

	debug_view("mqtt_subscribe send \n");
	pkt_dump(subscribe_packet,subscribe_packet_size);

	// Send the packet
	if(mqtt_context->send(&mqtt_context->my_socket, subscribe_packet, subscribe_packet_size) != subscribe_packet_size) {
		free(fix_ptr);
		free(subscribe_packet);
		return NX_STATUS_FAILURE;
	}
	
	free(fix_ptr);
	free(subscribe_packet);
	return NX_STATUS_SUCCESS;
}


int mqtt_receive_subscription(mqtt_config_params_h* mqtt_context,int timeout,char * pkt_buffer,char **topic,char **msg_ptr){
	int length = 0;
	char buffer[10] = {0};
	uint16_t type = 0;
here:
    if((int)(mqtt_context->receive(mqtt_context,timeout,(uint8_t *)pkt_buffer)) <= 1){
        debug_view("Did not receive any packet\n Reading again... \n");
                        return NX_STATUS_FAILURE;
    }
    type = MQTTParseMessageType(pkt_buffer);
    debug_view(" packet type \n");
            sprintf(buffer,"%d",type);
            debug_view(buffer);
            debug_view("\n");

   /* Sometimes duplicate requests will be received. So do not return
   * the data to application.Go for Next packet.
    */
    if(type != MQTT_PKT_TYPE_PUBLISH){
        debug_view("Not publish \n");
        goto here;
    }

    length = mqtt_get_pub_topic_ptr((uint8_t *)pkt_buffer,(uint8_t **)topic);

    if(!length){
        debug_view("Not a publish message \n");
    return NX_STATUS_FAILURE;
    }
    mqtt_get_publish_msg_ptr(pkt_buffer,(uint8_t **)msg_ptr);

    return length;
}

int mqtt_unsubscribe(mqtt_config_params_h* mqtt_context, const char* topic, uint16_t* message_id) {
	
	uint8_t * fix_ptr;
	uint8_t * unsub_packet, * index,* payload_ptr;
	uint16_t payloadlen = 0;
    uint8_t fixed_header_len = 0;
    uint16_t remaining_length = 0;
    uint16_t unsub_packet_size = 0;
	uint16_t topiclen = (uint16_t)strlen(topic);
	uint8_t payload[topiclen+2]; /* topiclen(2 bytes) + topic (topiclen bytes)  */
	uint8_t variable_header[2]; 
	memset(variable_header, 0, sizeof(variable_header));
	
	payloadlen = (uint16_t)(sizeof(payload));
	remaining_length = (uint16_t)(sizeof(variable_header) + payloadlen);
	
	/* Fixed Header frame */

	fix_ptr = frame_fixed_header(remaining_length,0,MQTT_QOS1_FLAG,0,MQTT_PKT_TYPE_UNSUBSCRIBE,&fixed_header_len);

		
	/* Variable Header frame */
	variable_header[0] = (uint8_t)(mqtt_context->seq>>8);
	variable_header[1] = (uint8_t)(mqtt_context->seq&0xFF);
	if(message_id) { // Returning message id
		*message_id = mqtt_context->seq;
	}
	mqtt_context->seq++;
	
	/* Frame Payload */
	payload_ptr = payload;	
	memset(payload, 0, sizeof(payload));
	
	*payload_ptr++ = (uint8_t)(topiclen>>8);
	*payload_ptr++ = (uint8_t)(topiclen&0xFF);
	
	memcpy(payload_ptr, topic, topiclen);
	payload_ptr+=topiclen;
	
	/* Frame Full publish packet */
	
	unsub_packet_size = (uint16_t)(fixed_header_len + sizeof(variable_header) + payloadlen);
	unsub_packet = (uint8_t *)malloc(unsub_packet_size);
	index = unsub_packet;
	
	
	/* Copy Fixed header */
	memcpy(index,fix_ptr,fixed_header_len);
	index+=fixed_header_len;
	
	/* Copy Variable header */
	memcpy(index,variable_header,sizeof(variable_header));
	index += sizeof(variable_header);
	
	/* Copy Payload */
	memcpy(index,payload,payloadlen);
	
	
	debug_view("mqtt_unsubscribe send \n");
	// Send the packet
	if(mqtt_context->send(&mqtt_context->my_socket, unsub_packet, unsub_packet_size) != unsub_packet_size) {
		free(fix_ptr);
		free(unsub_packet);
		return NX_STATUS_FAILURE;
	}
	
	free(fix_ptr);
	free(unsub_packet);
	
	return NX_STATUS_SUCCESS;
	
}

int mqtt_disconnect(mqtt_config_params_h* mqtt_context) {

	uint8_t disconnect_packet[2] = {0};
	
	disconnect_packet[0] = (MQTT_PKT_TYPE_DISCONNECT << 4);
	disconnect_packet[1] = 0x00;
	debug_view("mqtt_disconnect send \n");
	if(mqtt_context->send(&mqtt_context->my_socket, disconnect_packet, sizeof(disconnect_packet)) != sizeof(disconnect_packet)) {
		return NX_STATUS_FAILURE;
	}

	return NX_STATUS_SUCCESS;
}

int mqtt_ping(mqtt_config_params_h* mqtt_context) {
	uint8_t ping_packet[2] = {0};
	
	ping_packet[0] = (MQTT_PKT_TYPE_PINGREQ << 4);
	ping_packet[1] = 0x00;
	debug_view("mqtt_ping send \n");
	
	if(mqtt_context->send(&mqtt_context->my_socket, ping_packet, sizeof(ping_packet)) != sizeof(ping_packet)) {
		return NX_STATUS_FAILURE;
	}

	return NX_STATUS_SUCCESS;
}


void mqtt_set_keepalive(mqtt_config_params_h* mqtt_context, uint16_t alive) {
	mqtt_context->alive = alive;
}

void mqtt_set_auth(mqtt_config_params_h* mqtt_context, const char* username, const char* password) {
	if(username && username[0] != '\0')
		strncpy(mqtt_context->username, username, sizeof(mqtt_context->username)-1);
	if(password && password[0] != '\0')
		strncpy(mqtt_context->password, password, sizeof(mqtt_context->password)-1);
}

void mqtt_init(mqtt_config_params_h* mqtt_context, const char* clientid) {

	/* message id sequence */
	mqtt_context->seq = 1; 

	memset(mqtt_context->clientid, 0, sizeof(mqtt_context->clientid));

	if(clientid) {
		strncpy(mqtt_context->clientid, clientid, sizeof(mqtt_context->clientid));
	} else {
		strcpy(mqtt_context->clientid, "iWavecid123");
	}
	
	mqtt_context->clean_session = 1;
}

uint16_t mqtt_get_publish_msg(const uint8_t* buffer, uint8_t* msg) {
	uint16_t msg_len = 0;
	const uint8_t* msg_ptr;
		
	msg_len = mqtt_get_publish_msg_ptr(buffer, &msg_ptr);
	
	if(msg_len != 0 && msg_ptr != NULL) {
		memcpy(msg, msg_ptr, msg_len);
	}
	return msg_len;
}

uint16_t mqtt_get_publish_msg_ptr(const uint8_t* buffer, const uint8_t **msg_ptr) {
	uint16_t length = 0,var_len = 0,offset = 0;
	uint8_t rem_len_bytes = 0,fixed_len = 0;
	if(MQTTParseMessageType(buffer) == MQTT_PKT_TYPE_PUBLISH) {
		// message starts at
		// fixed header length + Topic (UTF encoded) + msg id (if QoS>0)

		rem_len_bytes = (uint8_t)(mqtt_get_rem_lenbytes(buffer));
		fixed_len = (uint8_t)(1 + rem_len_bytes);
		
		/* topic length MSB - LSB */
		var_len = (uint16_t)((*(buffer+1+rem_len_bytes))<<8);
		var_len = (uint16_t)(var_len | (*(buffer+1+rem_len_bytes+1)));
		var_len = (uint16_t)(var_len+2);
		
		offset = (uint16_t)(fixed_len + var_len);
		
		
		if(MQTTParseMessageQos(buffer)) {
			/* 2 bytes - message id */
			offset = (uint16_t)(offset + 2);
		}

		*msg_ptr = (buffer + offset);
		
      	length = (uint16_t)(mqtt_get_remaining_length(buffer) - (offset-(rem_len_bytes+1)));
	} else {
		*msg_ptr = NULL;
	}
	return length;
}

uint16_t mqtt_get_pub_topic(const uint8_t* buffer, uint8_t* topic) {
	uint16_t topic_len = 0;
	const uint8_t* topic_ptr;
	
	topic_len = mqtt_get_pub_topic_ptr(buffer, &topic_ptr);

	if(topic_len != 0 && topic_ptr != NULL) {
		memcpy(topic, topic_ptr, topic_len);
	}
	
	return topic_len;
}


uint16_t mqtt_get_pub_topic_ptr(const uint8_t* buffer, const uint8_t **topic_ptr) {
	uint16_t length = 0;
	uint8_t rem_len_bytes = 0;

	if(MQTTParseMessageType(buffer) == MQTT_PKT_TYPE_PUBLISH) {
		
		rem_len_bytes = (uint8_t)(mqtt_get_rem_lenbytes(buffer));
		/* fixed header length - 1  byte + rem_len_bytes */
		length = (uint16_t)(*(buffer+1+rem_len_bytes)<<8);	// MSB of topic
		length = (uint16_t)(length | *(buffer+1+rem_len_bytes+1));	// LSB of topic
		
		/* topic = fixedHeaderFlag 1B + rem_len_bytes B + topic len 2B */
		*topic_ptr = (buffer + (1 + rem_len_bytes + 2));
	} else {
		*topic_ptr = NULL;
	}
	return length;
}

uint16_t mqtt_get_msg_id(const uint8_t* buffer) {
	uint8_t pkt_type = MQTTParseMessageType(buffer);
	uint8_t qos = MQTTParseMessageQos(buffer);
	uint8_t rem_len_bytes = 0,offset = 0,fixed_len = 0;
	uint16_t id = 0,var_len = 0;
	
	//printf("mqtt_parse_msg_id\n");
	
	if(pkt_type >= MQTT_PKT_TYPE_PUBLISH && pkt_type <= MQTT_PKT_TYPE_UNSUBACK) {
		if(pkt_type == MQTT_PKT_TYPE_PUBLISH) {
			if(qos != 0) {
				// fixed header length + Topic (UTF encoded)
				// = 1 for "flags" byte + rem_len_bytes for length bytes + topic size
				rem_len_bytes = (uint8_t) mqtt_get_rem_lenbytes(buffer);
				fixed_len = (uint8_t)(1 + rem_len_bytes);
				
				/* Topic len MSB/LSB */
				var_len = (uint16_t)((*(buffer+1+rem_len_bytes))<<8);
				var_len = (uint16_t)(var_len | *(buffer+1+rem_len_bytes+1));
				var_len = (uint16_t)(var_len + 2);
				
				offset = (uint8_t)(fixed_len + var_len);

				id = (uint16_t)(*(buffer+offset)<<8);				// id MSB
				id = (uint16_t)(id | *(buffer+offset+1));				// id LSB
			}
		} else {
			// fixed header length
			// 1 for "flags" byte + rem_len_bytes for length bytes
			rem_len_bytes = (uint8_t)(mqtt_get_rem_lenbytes(buffer));
			id =  (uint16_t)(*(buffer+1+rem_len_bytes)<<8);	// id MSB
			id = (uint16_t)(id | *(buffer+1+rem_len_bytes+1));	// id LSB
		}
	}
	return id;
}


uint16_t mqtt_get_remaining_length(const uint8_t* buffer) {
	uint16_t multiplier = 1;
	uint16_t value = 0;
	uint8_t digit;
	
	//printf("mqtt_parse_rem_len\n");
	
	/* skip "flags" byte */
	buffer++;

	do {
		digit = *buffer;
		value = (uint16_t)(value + (digit & 127) * multiplier);
		multiplier =  (uint16_t)(multiplier * 128);
		buffer++;
	} while ((digit & 128) != 0);

	return value;
}


uint8_t mqtt_get_rem_lenbytes(const uint8_t* buffer) {
	uint8_t bytes = 1;
	
	//printf("mqtt_num_rem_len_bytes\n");
	
	if ((buffer[1] & 0x80) == 0x80) {
		bytes++;
		if ((buffer[2] & 0x80) == 0x80) {
			bytes ++;
			if ((buffer[3] & 0x80) == 0x80) {
				bytes ++;
			}
		}
	}
	return bytes;
}

void pkt_dump(uint8_t *buf,uint16_t len)
{
    char val[10] = {0};
    uint8_t in = 0;
    debug_view("packet dump \n");
    for(in = 0; in < len;in++)
    {
    sprintf(val,"%x",*(buf+in));
    debug_view(val);
    debug_view("\n");
    }
}

