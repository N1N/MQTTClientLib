#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "mqttlib.h"
#include "common.h"
#include "nx_api.h"
#include "nx_dns.h"

//#include "comms_thread.h"


NX_STATUS init_resources(mqtt_config_params_h *mqtt_context,nx_packet *nx_pkt)
{
	
#ifdef INT_RCV
    void (*tcp_recieve_notify)(NX_TCP_SOCKET *socket_ptr) = &recieve_callback;
    nx_tcp_socket_receive_notify(&mqtt_context->my_socket,tcp_recieve_notify);
    tx_mutex_create(&mqtt_context->read_mutex,"ReadMutex",0);
#endif
		return NX_STATUS_SUCCESS;

}

NX_STATUS NXI_init_lib(mqtt_config_params_h *mqtt_context,nx_packet *nx_pkt)
{
	
	NX_STATUS        status = NX_STATUS_SUCCESS;
    int packet_length;
    uint16_t keepalive = 20; // Seconds
    uint16_t msg_id = 0;

    /* PATCH0001 */
    char ip[10] = {0};
    CHAR *name_ptr;
    ULONG ip_address;
    ULONG network_mask;
    ULONG mtu_size;
    ULONG physical_address_msw;
    ULONG physical_address_lsw;

	//mqtt_context->nx_pkt=nx_pkt;
	mqtt_context->mqttstateinfo = NOT_INITIALIZED;
	// /* PATCH0001 */
	//mqtt_context->nx_pkt->dhcp_enable = 1;
	/* ToDo : If not required, optimize the code by removing the nx_pkt 
	parameter passing to each function, Use mqtt_context->nx_pkt instead */
	mqtt_context_unv = mqtt_context;

	mqtt_context_unv->nx_pkt->actual_bind_port = mqtt_context_unv->nx_pkt->socket_bind_port;

	    /* Initialize the NetX system.  */
    nx_system_initialize();

	status = init_resources(mqtt_context,nx_pkt);
	/* Check for error.  */
    if (status)
            return status;
		
	/* Initialise nx packet pool, icmp, arp, tcp etc */
    status = init_stack(mqtt_context,nx_pkt);
    /* Check for error.  */
    if (status)
            return status;
	
	status = init_socket(mqtt_context, mqtt_context->nx_pkt->server_ip_address, mqtt_context->nx_pkt->server_bind_port);

	/* PATCH0001 */
	   status = nx_ip_interface_info_get(&mqtt_context->nx_pkt->ip_0, 0, &name_ptr, &ip_address,
	          &network_mask,
	          &mtu_size, &mqtt_context->nx_pkt->phsyical_address_msw,
	          &mqtt_context->nx_pkt->physical_address_lsw);

            debug_view("board MAC address is - \n");
	       sprintf(log_str, "%02x:%02x:%02x:%02x:%02x:%02x", ((mqtt_context->nx_pkt->phsyical_address_msw >> 8) & 0xFF), ((mqtt_context->nx_pkt->phsyical_address_msw) & 0xFF), ((mqtt_context->nx_pkt->physical_address_lsw >> 24) & 0xFF), ((mqtt_context->nx_pkt->physical_address_lsw >> 16) & 0xFF), ((mqtt_context->nx_pkt->physical_address_lsw >> 8) & 0xFF), ((mqtt_context->nx_pkt->physical_address_lsw) & 0xFF));
	        debug_view(log_str);

	return status;
}
#define DNS_SERVER_ADDRESS IP_ADDRESS(8, 8, 8, 8)

NX_STATUS dns_get_ip(mqtt_config_params_h* mqtt_context){

    UINT   status = NX_STATUS_SUCCESS;
    ULONG  host_ip_address;
    UINT   i = 0;
    char stringit[20];
    int a, b, c, d;

    /********************************************************************************/
    /* Type A */
    /* Send A type DNS Query to its DNS server and get the IPv4 address. */

    /********************************************************************************/

     /* Look up an IPv4 address over IPv4. */
     debug_view("nx_dns_host_by_name_get \n");

     status = nx_dns_host_by_name_get(&mqtt_context->nx_pkt->client_dns, (UCHAR *)mqtt_context->nx_pkt->server_url,
    &mqtt_context->nx_pkt->server_ip_address, 400);

     /* Check for DNS query error. */
     if (status){
                    mqtt_context->nx_api_errorcode = status;
                    return NX_DNS_HOST_BY_NAME_GET_FAILURE;
     }
     else
     {
         debug_view( "------------------------------------------------------\r\r\n");
         debug_view(" Server IP address : ");
         a = (mqtt_context->nx_pkt->server_ip_address & (0xff << 24)) >> 24;
         b = (mqtt_context->nx_pkt->server_ip_address & (0xff << 16)) >> 16;
         c = (mqtt_context->nx_pkt->server_ip_address & (0xff << 8)) >> 8;
         d = mqtt_context->nx_pkt->server_ip_address & 0xff;
         sprintf(stringit,"%d",a);
         strcpy(log_str,stringit);
         strcat(log_str,".");
         sprintf(stringit,"%d",b);
         strcat(log_str,stringit);
         strcat(log_str,".");
         sprintf(stringit,"%d",c);
         strcat(log_str,stringit);
         strcat(log_str,".");
         sprintf(stringit,"%d",d);
         strcat(log_str,stringit);
         strcat(log_str,"\n");
         debug_view(log_str);
         return NX_STATUS_SUCCESS;
     }

}
//#define TEST_WEB_SITE   "iot.eclipse.org"
//#define TEST_IP_ADDRESS IP_ADDRESS(88, 221, 87, 128)
NX_STATUS init_dns(mqtt_config_params_h* mqtt_context){

UINT   status = NX_STATUS_SUCCESS;

     /* Create a DNS instance for the Client. Note this function will create
     the DNS Client packet pool for creating DNS message packets intended
     for querying its DNS server. */
        debug_view("nx_dns_create \n");

     status = nx_dns_create(&mqtt_context->nx_pkt->client_dns, &mqtt_context->nx_pkt->ip_0, (UCHAR *)"DNS Client");

     /* Check for DNS create error. */

     if(status){
     mqtt_context->nx_api_errorcode = status;
     return NX_DNS_CREATE_FAILURE;
     }

#if 0
#ifdef NX_DNS_CACHE_ENABLE
     /* Initialize the cache. */

     status = nx_dns_cache_initialize(&client_dns, local_cache, LOCAL_CACHE_SIZE);
     /* Check for DNS cache error. */
     if (NX_SUCCESS != status)
     {
         while(1);
     }
#endif
#endif

     /* Is the DNS client configured for the host application to create the pecket
    pool? */
#if 0
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
     /* Yes, use the packet pool created above which has appropriate payload size
     for DNS messages. */
     status = nx_dns_packet_pool_set(&client_dns, &client_pool);

     /* Check for set DNS packet pool error. */
     if (NX_SUCCESS != status)
     {
         while(1);
     }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */
#endif

     debug_view("nx_dns_server_add \n");
     /* Add an IPv4 server address to the Client list. */
     status = nx_dns_server_add(&mqtt_context->nx_pkt->client_dns, DNS_SERVER_ADDRESS);

     /* Check for DNS add server error. */
     if (status){
               mqtt_context->nx_api_errorcode = status;
               return NX_DSN_SERVER_ADD_FAILURE;
           }

     return NX_STATUS_SUCCESS;

}


NX_STATUS dhcp_get_ip(mqtt_config_params_h *mqtt_context,NX_IP *ip_ptr)
{
    NX_STATUS status = NX_STATUS_SUCCESS;
    ULONG actual_status;
    ULONG ip_address = 0;
    ULONG network_mask = 0;
    ULONG data_size = 0;
    NX_PACKET *response_ptr;
    int a, b, c, d;
    char ip[5] = {0};


    /* Wait for the link to come up. */
     do
     {

     /* Get the link status. */
     status = nx_ip_status_check(ip_ptr, NX_IP_LINK_ENABLED,&actual_status, 100);

     } while (status != NX_SUCCESS);
     /* Create a DHCP instance. */
      status = nx_dhcp_create(&mqtt_context->nx_pkt->my_dhcp, ip_ptr, "My DHCP");

      /* Check for DHCP create error. */
      if (status){
          mqtt_context->nx_api_errorcode = status;
          return NX_UDP_ENABLE_FAILURE;
      }

      /* Start DHCP. */
     status = nx_dhcp_start(&mqtt_context->nx_pkt->my_dhcp);
     /* Check for DHCP create error. */
     if (status){
         mqtt_context->nx_api_errorcode = status;
         return NX_DHCP_START_FAILURE;
     }
      /* Wait for IP address to be resolved through DHCP. */
      status = nx_ip_status_check(ip_ptr, NX_IP_ADDRESS_RESOLVED,
      (ULONG *) &status, 100000);

      /* Check to see if we have a valid IP address. */
      if (status)
      {
          mqtt_context->nx_api_errorcode = status;
          return NX_IP_STATUS_CHECK_FAILURE;
      }

      status = nx_ip_address_get(ip_ptr,&mqtt_context->nx_pkt->board_ip_address,&mqtt_context->nx_pkt->network_mask);

      if (status)
      {
          mqtt_context->nx_api_errorcode = status;
          return NX_IP_ADDRESS_GET_FAILURE;
      }

        a = (mqtt_context->nx_pkt->board_ip_address & (0xff << 24)) >> 24;
        b = (mqtt_context->nx_pkt->board_ip_address & (0xff << 16)) >> 16;
        c = (mqtt_context->nx_pkt->board_ip_address & (0xff << 8)) >> 8;
        d = mqtt_context->nx_pkt->board_ip_address & 0xff;

        strcpy(log_str,"DHCP Obtained IP - ");
        sprintf(ip,"%d",a);
        strcat(log_str,ip);
        strcat(log_str,".");
        sprintf(ip,"%d",b);
        strcat(log_str,ip);
        strcat(log_str,".");
        sprintf(ip,"%d",c);
        strcat(log_str,ip);
        strcat(log_str,".");
        sprintf(ip,"%d",d);
        strcat(log_str,ip);
        strcat(log_str,"\n");
        debug_view(log_str);

       /* Yes, a valid IP address is now on lease… All NetX
            services are available. */

      return NX_STATUS_SUCCESS;
}

NX_STATUS init_stack(mqtt_config_params_h *mqtt_context,nx_packet * nxi_pkt )
{
    NX_STATUS status = NX_STATUS_SUCCESS;
    ULONG       actual_status;


    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&nxi_pkt->pool_0, (CHAR *) "NetX Main Packet Pool", (1536 + 32),&nxi_pkt->packet_pool_area, sizeof(nxi_pkt->packet_pool_area));

    /* Check for pool creation error.  */
    if (status){
		mqtt_context->nx_api_errorcode = status;
		strcpy(log_str,"NX_PACKET_POOL_CREATE_FAILURE");
		debug_view(log_str);
		return NX_PACKET_POOL_CREATE_FAILURE;
	}
    if(mqtt_context->nx_pkt->dhcp_enable == 0){    /* Create an IP instance.  */
    //85,119,83,194
    status = nx_ip_create(&nxi_pkt->ip_0, (CHAR *) "NetX IP Instance 0", mqtt_context->nx_pkt->board_ip_address, 0xFFFFFE00UL, &nxi_pkt->pool_0, nx_ether_driver_eth1,
                          &nxi_pkt->ip_memory_area, sizeof(nxi_pkt->ip_memory_area), 1);
    }
    else
    {
        /* Create an IP instance for DHCP  */
        status = nx_ip_create(&nxi_pkt->ip_0, (CHAR *) "NetX IP Instance 0", IP_ADDRESS(0,0,0,0), 0xFFFFFE00UL, &nxi_pkt->pool_0, nx_ether_driver_eth1,
                                 &nxi_pkt->ip_memory_area, sizeof(nxi_pkt->ip_memory_area), 1);

    }

    /* Check for IP create errors.  */
    if (status){
        mqtt_context->nx_api_errorcode = status;
        return NX_IP_CREATE_FAILURE;
    }


    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&nxi_pkt->ip_0);

    /* Check for UDP enable errors.  */
    if (status){
           mqtt_context->nx_api_errorcode = status;
           return NX_UDP_ENABLE_FAILURE;
    }


    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status =  nx_arp_enable(&nxi_pkt->ip_0, &nxi_pkt->arp_memory_area, sizeof(nxi_pkt->arp_memory_area));

    /* Check for ARP enable errors.  */
   if (status){
       mqtt_context->nx_api_errorcode = status;
        strcpy(log_str,"NX_ARP_ENABLE_FAILURE");
        debug_view(log_str);
        return NX_ARP_ENABLE_FAILURE;
   }

    /* Enable TCP traffic.  */
    status =  nx_tcp_enable(&nxi_pkt->ip_0);

    /* Check for TCP enable errors.  */
   if (status){
        mqtt_context->nx_api_errorcode = status;
        strcpy(log_str,"NX_TCP_ENABLE_FAILURE");
        debug_view(log_str);
        return NX_TCP_ENABLE_FAILURE;
   }
    /* Enable ICMP.  */
    status =  nx_icmp_enable(&nxi_pkt->ip_0);

    /* Check for errors.  */
   if (status){
       mqtt_context->nx_api_errorcode = status;
        strcpy(log_str,"NX_ICMP_ENABLE_FAILURE");
        debug_view(log_str);
        return NX_ICMP_ENABLE_FAILURE;
   }

    if(mqtt_context->nx_pkt->dhcp_enable == 1){
    status = dhcp_get_ip(mqtt_context,&nxi_pkt->ip_0);

    /* Check for dhcp_get_ip errors.  */
    if (status){
        mqtt_context->nx_api_errorcode = status;
        return status;
    }
}

   status = nx_ip_fragment_enable(&nxi_pkt->ip_0);

    if (status){
        mqtt_context->nx_api_errorcode = status;
		strcpy(log_str,"NX_IP_FRAGMENT_ENABLE_FAILURE");
		debug_view(log_str);
		return NX_IP_FRAGMENT_ENABLE_FAILURE;
	}

    if(mqtt_context->nx_pkt->dhcp_enable == 0){
    status = nx_ip_gateway_address_set(&nxi_pkt->ip_0,mqtt_context->nx_pkt->gateway_address);

	if (status){
		mqtt_context->nx_api_errorcode = status;
		strcpy(log_str,"NX_IP_GATEWAY_ADDRESS_SET_FAILURE");
		debug_view(log_str);
        return NX_IP_GATEWAY_ADDRESS_SET_FAILURE;
	}
    }

    /* Ensure the IP instance has been initialized.  */
    status =  nx_ip_status_check(&nxi_pkt->ip_0, NX_IP_INITIALIZE_DONE, &actual_status, NX_WAIT_FOREVER);
    /* Check for errors.  */
   if (status){
	   mqtt_context->nx_api_errorcode = status;
		strcpy(log_str,"NX_IP_STATUS_CHECK_FAILURE");
		debug_view(log_str);
		return NX_IP_STATUS_CHECK_FAILURE;
   }

   if(strlen(mqtt_context->nx_pkt->server_url) != 0){
       status = init_dns(mqtt_context);

       if (status){
             mqtt_context->nx_api_errorcode = status;
             strcpy(log_str,"NX_DNS_FAILURE");
             debug_view(log_str);
             return status;
       }

       debug_view("dns_get_ip \n");

       status = dns_get_ip(mqtt_context);
       if (status){
          mqtt_context->nx_api_errorcode = status;
          strcpy(log_str,"NX_DNS_FAILURE");
          debug_view(log_str);
          return status;
       }
   }
   return status;
}


NX_STATUS init_socket(mqtt_config_params_h* mqtt_context, ULONG serverip /* const char* hostname */, UINT port)
{

	NX_STATUS status = NX_STATUS_SUCCESS;
    void (*disconnect_callback)(NX_TCP_SOCKET *) = &disconnect_cli;

     status = create_socket(mqtt_context,disconnect_callback);

     /* Check for error.  */
     if (status){
		strcpy(log_str,"Create socket Failed \n");
		debug_view(log_str);
		return status;
		 }
	 else 
 			mqtt_fsm(mqtt_context,SOCKET_CREATED,NULL);
		status = socket_bind(mqtt_context,mqtt_context->nx_pkt->server_bind_port);

     /* Check for error.  */
     if (status){
		strcpy(log_str," Socket Bind Failed \n");
		debug_view(log_str);
         return status;
	 }
	 else 
 			mqtt_fsm(mqtt_context,BIND_SUCCESS,NULL);
    /* PATCH0001 */
     tx_thread_sleep(100);
    status = client_connect(mqtt_context,serverip,port);

     /* Check for error.  */
     if (status){
		strcpy(log_str," Socket Connect Failed \n");
		debug_view(log_str);
		return status;
	 }
     else 
 			mqtt_fsm(mqtt_context,SOC_CONNECT_SUCCESS,NULL);

     return status;
}
