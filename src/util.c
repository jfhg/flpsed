/* 
 * "$Id: util.c,v 1.1 2005/02/28 17:56:51 hofmann Exp $"
 *
 * flpsed program.
 *
 * Copyright 2005 by Johannes Hofmann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

FILE *
pexecvp(const char *file, char *const argv[], pid_t *pid, char *type) {
  FILE *iop;
  int   pdes[2];

  if (pipe(pdes) < 0) {
    return NULL;
  }
	
  *pid = vfork();

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
    exit(127);
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
