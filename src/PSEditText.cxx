//
// "$Id: PSEditText.cxx,v 1.3 2005/06/17 18:20:42 hofmann Exp $"
//
// PSEditWidget routines.
//
// Copyright 2004 by Johannes Hofmann
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "PSEditText.H"


PSEditText::PSEditText(int x1, int y1, const char *s1, 
		       int size1, PSEditColor *c) {
  x = x1;
  y = y1;
  s = strdup(s1);
  tag = NULL;
  size = size1;
  text_color.set(c->r, c->g, c->b);
  next = NULL;
}

PSEditText::~PSEditText() {
  if (next) {
    delete(next);
  }
  if (s) {
    free(s);
  }
  if (tag) {
    free(tag);
  }
}
  
void PSEditText::append_text(const char*s1) {
  int len = (s?strlen(s):0) + strlen(s1) + 1;
  char *tmp = (char*) malloc(len);
  
  strncpy(tmp, s?s:"", len);
  strncat(tmp, s1, len - strlen(tmp));
  
  if (s) {
    free(s);
  }
  
  s = tmp;
}

void PSEditText::rm_char() {
  if (s && strlen(s) > 0) {
    s[strlen(s) - 1] = '\0';
  }
}

void PSEditText::move(int x1, int y1) {
  x = x1;
  y = y1;
}

void PSEditText::append(PSEditText *g) {
  PSEditText *p = this;
  while (p->next) {
    p = p->next;
  }
  p->next = g;
}

PSEditText *PSEditText::get_match(int x1, int y1) {
  if (abs(x - x1) < 10 && abs(y - y1) < 10) {
    return this;
  } else if (next) {
    return next->get_match(x1, y1);
  } else {
      return NULL;
  }
}

int PSEditText::set_text(const char *t) {
  if (s) {
    free(s);
  }
  if (t) {
    s = strdup(t);
  } else {
    s = NULL;
  }

  return 0;
}

char *PSEditText::get_text() {
  return s;
}

char *PSEditText::get_tag() {
  return tag;
}

int PSEditText::set_tag(const char *t) {
  if (tag) {
    free(tag);
  }
  if (t) {
    tag = strdup(t);
  } else {
    tag = NULL;
  }

  return 0;
}

int PSEditText::get_size() {
  return size;
}

PSEditText* PSEditText::get_next() {
  return next;
}

int PSEditText::get_x() {
  return x;
}

int PSEditText::get_y() {
  return y;
}
