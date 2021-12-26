/*
 * set_conf.h - Copied from rigctl_parse.h
 *
 */

#ifndef SET_CONF_H
#define SET_CONF_H

#include <stdio.h>
#include <hamlib/rig.h>

#define RIGCTL_PARSE_END 1
#define RIGCTL_PARSE_ERROR 2


/*
 * Prototypes
 */

int set_conf(RIG *my_rig, char *conf_parms);

#endif  /* SET_CONF_H */
