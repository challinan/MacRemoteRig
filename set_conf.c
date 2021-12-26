/*
 * rigctl_parse.c - (C) Stephane Fillod 2000-2011
 *                  (C) Nate Bargmann 2003,2006,2008,2010,2011,2012,2013
 *                  (C) Terry Embry 2008-2009
 *                  (C) The Hamlib Group 2002,2006,2007,2008,2009,2010,2011
 *
 * This program tests/controls a radio using Hamlib.
 * It takes commands in interactive mode as well as
 * from command line options.
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else                             /* !defined(HAVE_HISTORY_H) */
extern void add_history();
extern int write_history();
extern int read_history();
#  endif                            /* defined(HAVE_READLINE_HISTORY_H) */
/* no history */
#endif                              /* HAVE_READLINE_HISTORY */

#include "set_conf.h"

#define STR1(S) #S
#define STR(S) STR1(S)

#define MAXNAMSIZ 32
#define MAXNBOPT 100    /* max number of different options */
#define MAXARGSZ 511

int set_conf(RIG *my_rig, char *conf_parms)
{
    char *p, *n;

    p = conf_parms;
    printf("set_conf: Entered - conf_parms = %s\n", p);

    while (p && *p != '\0')
    {
        int ret;

        /* FIXME: left hand value of = cannot be null */
        char *q = strchr(p, '=');

        if (!q)
        {
            fprintf(stderr, "set_conf: bailing out here 1\n");
            return(-RIG_EINVAL);
        }

        *q++ = '\0';
        n = strchr(q, ',');

        if (n)
        {
            *n++ = '\0';
        }

        fprintf(stderr, "set_conf: calling rig_set_conf w/ p = %s, %s\n", p, q);
        ret = rig_set_conf(my_rig, rig_token_lookup(my_rig, p), q);

        if (ret != RIG_OK)
        {
            fprintf(stderr, "set_conf: bailing out here 2\n");
            return(ret);
        }

        p = n;
    }

    return(RIG_OK);
}
