/* 
 * Copyright (c) 2015-2016, Gregory M. Kurtzer. All rights reserved.
 * 
 * “Singularity” Copyright (c) 2016, The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory (subject to receipt of any
 * required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * 
 * If you have questions about your rights to use or distribute this software,
 * please contact Berkeley Lab's Innovation & Partnerships Office at
 * IPO@lbl.gov.
 * 
 * NOTICE.  This Software was developed under funding from the U.S. Department of
 * Energy and the U.S. Government consequently retains certain rights. As such,
 * the U.S. Government has been granted for itself and others acting on its
 * behalf a paid-up, nonexclusive, irrevocable, worldwide license in the Software
 * to reproduce, distribute copies to the public, prepare derivative works, and
 * perform publicly and display publicly, and to permit other to do so. 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h> 
#include <string.h>
#include <grp.h>


#include "config.h"
#include "file.h"
#include "util.h"



int build_passwd(char *template, char *output) {
    uid_t uid = getuid();

    if ( is_file(template) < 0 ) {
        fprintf(stderr, "ERROR: Template passwd not found: %s\n", template);
        return(-1);
    }

    if ( copy_file(template, output) < 0 ) {
        fprintf(stderr, "ERROR: Could not copy %s to %s: %s\n", template, output, strerror(errno));
        return(-1);
    }

    if ( uid != 0 ) {
        FILE *fd_output;
        uid_t uid = getuid();
        struct passwd *pwent = getpwuid(uid);

        fd_output = fopen(output, "a");
        fprintf(fd_output, "\n%s:x:%d:%d:%s:%s:%s\n", pwent->pw_name, pwent->pw_uid, pwent->pw_gid, pwent->pw_gecos, pwent->pw_dir, pwent->pw_shell);
        fclose(fd_output);
    }

    return(0);
}


int build_group(char *template, char *output) {
    gid_t gid = getgid();

    if ( is_file(template) < 0 ) {
        fprintf(stderr, "ERROR: Template group file not found: %s\n", template);
        return(-1);
    }

    if ( copy_file(template, output) < 0 ) {
        fprintf(stderr, "ERROR: Could not copy %s to %s: %s\n", template, output, strerror(errno));
        return(-1);
    }

    if ( gid != 0 ) {
        FILE *fd_output;
        int groupcount;
        int i;
        int maxgroups = sysconf(_SC_NGROUPS_MAX) + 1;
        uid_t uid = getuid();
        gid_t gids[maxgroups];
        struct passwd *pwent = getpwuid(uid);
        struct group *grent = getgrgid(gid);

        fd_output = fopen(output, "a");
        fprintf(fd_output, "\n%s:x:%d:%s\n", grent->gr_name, grent->gr_gid, pwent->pw_name);

        groupcount = getgroups(maxgroups, gids);

        for (i=0; i<= groupcount; i++) {
            if ( gids[i] >= 500 && gids[i] != grent->gr_gid ) {
                struct group *gr = getgrgid(gids[i]);
                fprintf(fd_output, "%s:x:%d:%s\n", gr->gr_name, gr->gr_gid, pwent->pw_name);
            }
        }

        fclose(fd_output);
    }

    return(0);
}
