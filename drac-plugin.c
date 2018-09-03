/*
 *
 * dovecot plugin for DRAC authentication
 *
 * Copyright (C) 2011 DesigNET, INC.
 *
 * $Id: drac-plugin.c,v 1.1.1.1 2011/09/09 08:04:34 usuda Exp $
 *
 * based:
 *   http://dovecot.org/patches/1.1/drac.c
 *
 * Hacked for IPv6 and dovecot-2.2 by Anton Lundin <glance@acc.umu.se>
 *
 */
#include "lib.h"
#include "net.h"
#include "ioloop.h"
#include "mail-user.h"
#include "mail-storage-private.h"
#include <stdlib.h>
#include <string.h>

/* default parameters */
#define DRAC_TIMEOUT_SECS	(60)
#define DRAC_HOST               "127.0.0.1"


/* libdrac function */
int dracauth(const char *, unsigned long, char **);
int dracauth6(const char *host, const unsigned char *userip6, char **errmsg);

const char *drac_plugin_version = DOVECOT_ABI_VERSION;

static struct timeout *to_drac = NULL;
static const char *drachost = NULL; /* dracd host */
static struct ip_addr ip; /* remote ip address */
static unsigned long dractout = 0; /* drac timeout secs */

static void drac_timeout(void *context ATTR_UNUSED)
{
    char *err;
    struct ip_addr ipv4 = ip;

    if (IPADDR_IS_V4(&ipv4) || (net_ipv6_mapped_ipv4_convert(&ip, &ipv4) == 0)) {
        i_info("drac_timeout() IP is ipv4(%s)",net_ip2addr(&ipv4));
        if (dracauth(drachost, (unsigned long) ipv4.u.ip4.s_addr, &err) != 0) {
            i_error("%s: dracauth() failed: %s", __FUNCTION__, err);
        }
    } else if(IPADDR_IS_V6(&ip)) {
        i_info("drac_timeout() IP is ipv6(%s)",net_ip2addr(&ip));
        if (dracauth6(drachost, (const unsigned char *) &ip.u.ip6.s6_addr, &err) != 0) {
            i_error("%s: dracauth6() failed: %s", __FUNCTION__, err);
        }
    } else {
        i_error("drac_timeout() ip adress unrecognized, canceling myself.");
        timeout_remove(&to_drac);
    }
}

static void drac_mail_user_created(struct mail_user *user)
{
    const char *dractout_str;
    char *ep;

    if (user->conn.remote_ip == NULL) {
        i_debug("%s Not a remote login", __FUNCTION__);
        return;
    }

    /* check address family */
    if(IPADDR_IS_V4(user->conn.remote_ip) || IPADDR_IS_V6(user->conn.remote_ip)) {
        /* get remote IP address... uum... */
        memcpy(&ip, user->conn.remote_ip, sizeof(ip));

        /* get DRAC server name */
        drachost = mail_user_plugin_getenv(user, "dracdserver");
        if(drachost == NULL) {
            drachost = DRAC_HOST;
        }

        /* get timeout secs */
        dractout_str = mail_user_plugin_getenv(user, "dracdtimeout");
        if(dractout_str == NULL) {
            dractout = DRAC_TIMEOUT_SECS;
        } else {
            dractout = strtoul(dractout_str, &ep, 10);
            /* bad format -> use default value */
            if(ep != NULL && *ep != '\0') {
                i_warning("%s: bad dracdtimeout (%s). using default %d",
                          __FUNCTION__, dractout_str, DRAC_TIMEOUT_SECS);
                dractout = DRAC_TIMEOUT_SECS;
            }
        }
        i_debug("%s: dracdserver=%s, timeout=%ldsecs", __FUNCTION__,
               drachost, dractout);

        /* connect to DRAC server */
        drac_timeout(NULL);
#undef timeout_add
#define timeout_add(msecs, callback, context) \
        timeout_add(msecs, __FILE__, __LINE__, callback, context)
        to_drac = timeout_add(1000*dractout, drac_timeout, NULL);
    } else {
        i_error("%s: Only IPv4 and IPv6 addresses are supported", __FUNCTION__);
    }
}

static struct mail_storage_hooks drac_mail_storage_hooks = {
    .mail_user_created = drac_mail_user_created,
};

void drac_plugin_init(struct module *module)
{
    mail_storage_hooks_add(module, &drac_mail_storage_hooks);
    i_debug("%s called", __FUNCTION__);
}

void drac_plugin_deinit(void)
{
    if(to_drac != NULL) {
        timeout_remove(&to_drac);
        to_drac = NULL;
    }
    mail_storage_hooks_remove(&drac_mail_storage_hooks);
    i_debug("%s called", __FUNCTION__);
}
