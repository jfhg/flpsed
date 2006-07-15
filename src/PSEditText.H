// 
// "$Id: PSEditText.H,v 1.5 2005/06/17 18:20:42 hofmann Exp $"
//
// X11 header file for the Fast Light Tool Kit (FLTK).
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

#ifndef PSEditText_H
#define PSEditText_H

#include <math.h>

class PSEditColor {
 public:
  double r, g, b;

  PSEditColor() {
    r = 0.0;
    g = 0.0;
    b = 0.0;
  }

  PSEditColor(double r1, double g1, double b1) {
    set(r1, g1, b1);
  }

  void set(unsigned char r1, unsigned char g1, unsigned char b1) {
    r = r1 / 256.0;
    g = g1 / 256.0;
    b = b1 / 256.0;
  }

  void set(double r1, double g1, double b1) {
    r = r1;
    g = g1;
    b = b1;
  }

  void set(const PSEditColor *c) {
    r = c->r;
    g = c->g;
    b = c->b;
  }

  unsigned char get_r() {
    return (unsigned char) rint(r * 256.0);
  }

  unsigned char get_g() {
    return (unsigned char) rint(g * 256.0);
  }

  unsigned char get_b() {
    return (unsigned char) rint(b * 256.0);
  }


};

class PSEditText;

class PSEditText {
  int x, y;

  char *s;

  char *tag;

  PSEditText *next;
  
public:

  PSEditColor text_color;

  int size;
  
  PSEditText(int x1, int y1, const char *s1, int size1, PSEditColor *c);

  ~PSEditText();

  void append_text(const char*s1);

  void rm_char();

  void move(int x1, int y1);

  void append(PSEditText *g);

  PSEditText *get_match(int x1, int y1);

  int set_text(const char *t);

  char *get_text();

  int is_empty();

  char *get_tag();

  int set_tag(const char *t);

  int get_size();

  PSEditText *get_next();

  int get_x();

  int get_y();
};

#endif
