/* 
 * "$Id: util.h,v 1.2 2005/03/17 18:46:20 hofmann Exp $"
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
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE *
pexecvp(const char *file, char *const argv[], pid_t *pid, char *type);

#ifdef __cplusplus
}
#endif

#endif
