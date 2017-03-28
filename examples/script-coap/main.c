/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       CoAP example server application (using nanocoap)
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @}
 */

#include <stdio.h>

#include "nanocoap.h"
#include "nanocoap_sock.h"
#include "jerryscript.h"

#include "thread.h"

#include "xtimer.h"

char jsstack[THREAD_STACKSIZE_MAIN];

#define COAP_INBUF_SIZE (256U)

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/* import "ifconfig" shell command, used for printing addresses */
extern int _netif_config(int argc, char **argv);

/* execute every period T script stored in static string S */
/* both T and S are resources on which GET and SET are possible via CoAP  */

void *jsthread_handler(void *arg)
{
    jerry_char_t script[50];
    *script = '\0';
    strcat((char *)script, "print('in js thread');");
    size_t script_size = strlen((char *) script);
    while(1) {
    /* puts("I'm in the \"js-thread\" now"); */
    jerry_run_simple(script, script_size, JERRY_INIT_EMPTY);
    xtimer_sleep(30);
    }
    return NULL;
}


int main(void)
{
    puts("RIOT nanocoap example application");

    /* nanocoap_server uses gnrc sock which uses gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _netif_config(0, NULL);

    /* launch javascript thread */
    thread_create(jsstack, sizeof(jsstack),
                    THREAD_PRIORITY_MAIN - 1,
                    THREAD_CREATE_STACKTEST,
                    jsthread_handler,
                    NULL, "js thread");

    /* initialize nanocoap server instance */
    uint8_t buf[COAP_INBUF_SIZE];
    sock_udp_ep_t local = { .port=COAP_PORT, .family=AF_INET6 };
    puts("I'm in the \"main-thread\" now");
    nanocoap_server(&local, buf, sizeof(buf));

    /* should be never reached */
    return 0;
}