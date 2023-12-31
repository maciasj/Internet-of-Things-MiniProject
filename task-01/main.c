#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "ztimer.h"
#include "shell.h"

#include "mutex.h"

#include "lpsxxx.h"
#include "lpsxxx_params.h"

#include "lsm303dlhc.h"
#include "lsm303dlhc_params.h"

//#include "net/gcoap.h"


static lpsxxx_t lpsxxx;

static lsm303dlhc_t lsm303dlhc;

static mutex_t lsm_lock = MUTEX_INIT_LOCKED;
static mutex_t lps_lock = MUTEX_INIT_LOCKED;

/* stack memory allocated for the lsm303dlhc thread */
static char lsm303dlhc_stack[THREAD_STACKSIZE_MAIN];
static char lpsxxx_stack[THREAD_STACKSIZE_MAIN];
const char *server_address = "coap://13.51.195.117:1111";  // Replace with your server's address

/*
void send_acceleAndMagnet_to_coap_server(int accX, int accY, int accZ, int magX, int magY, int magZ )
{
    // Allocate buffer and coap_pkt_t for the request
    uint8_t coap_buffer[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t coap_pkt;

    // Initialize the request
    gcoap_req_init(&coap_pkt, coap_buffer, CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_PUT);

    // Set the request path
    const char *resource_path = "/temperaturePressure";  // Replace with your specific resource path
    char full_uri[128];  // Adjust the size based on your requirements
    snprintf(full_uri, sizeof(full_uri), "%s%s", server_address, resource_path);

    coap_pkt.payload = gcoap_request(&coap_pkt, COAP_METHOD_POST, full_uri);

    // Add CoAP options (if needed)
    coap_opt_add_format(coap_pkt.payload, COAP_FORMAT_JSON );

    // Complete PDU metadata
    size_t metadata_len = coap_opt_finish(coap_pkt.payload);
    int data_length = snprintf(NULL, 0, "%d%d%d%d%d%d", accX, accY, accZ, magX, magY, magZ);

    // Write the accelerometer and magnetometer payload
    memcpy(coap_pkt.payload, accX,  accY,  accZ,  magX,  magY,  magZ);

    // Calculate the total length (metadata + payload)
    size_t pdu_len = metadata_len + data_length;

    // Set the destination endpoint 
    sock_udp_ep_t remote = {.family = AF_INET6, .port = 1111, .netif = path};

    // Send the CoAP request
    gcoap_req_send(&coap_pkt, pdu_len, &remote, NULL);
}

void send_tempAndPressure_to_coap_server(int temperature, int pressure)
{
    static const coap_endpoint_path_t path = {2, {"temperature", "riot"}};
    // Allocate buffer and coap_pkt_t for the request
    uint8_t coap_buffer[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t coap_pkt;

    // Initialize the request
    gcoap_req_init(&coap_pkt, coap_buffer, CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST);

    // Set the request path
    coap_pkt.payload = gcoap_request(&coap_pkt, COAP_METHOD_POST, path);

    // Add CoAP options (if needed)
    coap_opt_add_format(coap_pkt.payload, COAP_FORMAT_JSON );

    // Complete PDU metadata
    size_t metadata_len = coap_opt_finish(coap_pkt.payload);
    int data_length = snprintf(NULL, 0, "%d%d", temperature, pressure);

    // Write the accelerometer and magnetometer payload
    memcpy(coap_pkt.payload, temperature, pressure);

    // Calculate the total length (metadata + payload)
    size_t pdu_len = metadata_len + data_length;

    // Set the destination endpoint 
    sock_udp_ep_t remote = {.family = AF_INET6, .port = 1111, .netif = SOCK_ADDR_ANY_NETIF};

    // Send the CoAP request
    gcoap_req_send(&coap_pkt, pdu_len, &remote, NULL);
}
*/



static void *lsm303dlhc_thread(void *arg)
{
    (void)arg;

    while (1) {
        /* Acquire the mutex here */

        mutex_lock(&lsm_lock);

        /* Read the accelerometer/magnetometer values here */
        lsm303dlhc_3d_data_t mag_value;
        lsm303dlhc_3d_data_t acc_value;
        lsm303dlhc_read_acc(&lsm303dlhc, &acc_value);
        printf("Accelerometer x: %i y: %i z: %i\n",
               acc_value.x_axis, acc_value.y_axis, acc_value.z_axis);
        lsm303dlhc_read_mag(&lsm303dlhc, &mag_value);
        printf("Magnetometer x: %i y: %i z: %i\n",
               mag_value.x_axis, mag_value.y_axis, mag_value.z_axis);
        
        //send_acceleAndMagnet_to_coap_server(acc_value.x_axis, acc_value.y_axis, acc_value.z_axis, mag_value.x_axis, mag_value.y_axis, mag_value.z_axis);

        /* Release the mutex here */
        mutex_unlock(&lsm_lock);


        ztimer_sleep(ZTIMER_MSEC, 15000);
    }

    return 0;
}

static void _lsm303dlhc_usage(char *cmd)
{
    printf("usage: %s <start|stop>\n", cmd);
}

static int lsm303dlhc_handler(int argc, char *argv[])
{
    
    if (argc < 2) {
        _lsm303dlhc_usage(argv[0]);
        return -1;
    }

    if (!strcmp(argv[1], "start")) {
        mutex_unlock(&lsm_lock);
    }
    else if (!strcmp(argv[1], "stop")) {
        mutex_trylock(&lsm_lock);
    }
    else {
        _lsm303dlhc_usage(argv[0]);
        return -1;
    }

    return 0;
}

static void _lpsxxx_usage(char *cmd)
{
    printf("usage: %s <start|stop>\n", cmd);
}

static int lpsxxx_handler(int argc, char *argv[])
{
    if (argc < 2) {
        _lpsxxx_usage(argv[0]);
        return -1;
    }

     if (!strcmp(argv[1], "start")) {
        mutex_unlock(&lps_lock);
        }
    
    else if (!strcmp(argv[1], "stop")) {
        mutex_trylock(&lps_lock);
    }
    else {
        _lpsxxx_usage(argv[0]);
        return -1;
    }
    return 0;
}

static void *lpsxxx_thread(void *arg)
{
    (void)arg;

    while (1) {
        /* Read temperature and pressure here */
        int16_t temp = 0;
        lpsxxx_read_temp(&lpsxxx, &temp);
        printf("Temperature: %i.%u°C\n", (temp / 100), (temp % 100));

        uint16_t pres = 0;
        lpsxxx_read_pres(&lpsxxx, &pres);
        printf("Pressure: %uhPa\n", pres);
        // Send temperature and pressure data to CoAP server
       // send_tempAndPressure_to_coap_server(temp, pres); 
        ztimer_sleep(ZTIMER_MSEC, 30000);
    }

    return NULL;
}
static const shell_command_t commands[] = {
    /* lsm303dlhc shell command handler */
    { "lsm", "start/stop reading accelerometer values", lsm303dlhc_handler },

    /* Add the lps331ap command description here */

    { "lps", "read the lps331ap values", lpsxxx_handler },
    { NULL, NULL, NULL}
};

int main(void)
{
    lpsxxx_init(&lpsxxx, &lpsxxx_params[0]);
    lsm303dlhc_init(&lsm303dlhc, &lsm303dlhc_params[0]);

    thread_create(lsm303dlhc_stack, sizeof(lsm303dlhc_stack), THREAD_PRIORITY_MAIN - 1,
                  0, lsm303dlhc_thread, NULL, "lsm303dlhc");

    thread_create(lpsxxx_stack, sizeof(lpsxxx_stack), THREAD_PRIORITY_MAIN - 1,
                  0, lpsxxx_thread, NULL, "lpsxxx");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
