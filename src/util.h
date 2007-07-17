//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

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
