#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "mqttlib.h"
#include "common.h"
#include "nx_api.h"


NX_STATUS mqtt_connect_start(mqtt_config_params_h *mqtt_context)
{
	NX_STATUS        status = NX_STATUS_SUCCESS;
	ULONG bytes = 0;

	/* PATCH0001 */
	char mac_id[15] = {0};
    char full_mac_id[20] = {0};

	/* Set keep alive for mqtt client */
    mqtt_set_keepalive(mqtt_context, mqtt_context->alive);

	/* Init callbacks */
    mqtt_context->send = &send_packet;
    mqtt_context->receive = &read_packet;

    sprintf(mac_id,"%lu",mqtt_context->nx_pkt->phsyical_address_msw);
    strcpy(full_mac_id,mac_id);
    sprintf(mac_id,"%lu",mqtt_context->nx_pkt->physical_address_lsw);
    strcat(full_mac_id,mac_id);

    debug_view("\n Client Id : ");
    debug_view(full_mac_id);
    debug_view("\n");
    mqtt_init(mqtt_context,full_mac_id);
    mqtt_set_auth(mqtt_context, mqtt_context->username,mqtt_context->password);

	// >>>>> CONNECT
	status = mqtt_connect(mqtt_context);
	
	return status;
}



