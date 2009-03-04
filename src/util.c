/* 
 * Copyright 2007-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

FILE *
pexecvp(const char *file, char *const argv[], pid_t *pid, const char *type) {
	FILE *iop;
	int   pdes[2];

	if (pipe(pdes) < 0) {
		return NULL;
	}

	*pid = fork();

	if (*pid == -1) {
		perror("vfork");
		close(pdes[0]);
		close(pdes[1]);
		return NULL;
	} else if (*pid == 0) {
		/* child */

		if (*type == 'r') {
			close(pdes[0]);
			if (pdes[1] != STDOUT_FILENO) {
				dup2(pdes[1], STDOUT_FILENO);
				close(pdes[1]);
			}
		} else {
			close(pdes[1]);
			if (pdes[0] != STDIN_FILENO) {
				dup2(pdes[0], STDIN_FILENO);
				close(pdes[0]);
			}
		}

		execvp(file, argv);
		_exit(127);
	} else {
		/* parent */ 
		if (*type == 'r') {
			iop = fdopen(pdes[0], "r");
			close(pdes[1]);
		} else {
			iop = fdopen(pdes[1], "w");
			close(pdes[0]);
		}
		return iop;
	}
}
