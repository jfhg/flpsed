// 
// "$Id: PSEditText.H,v 1.2 2004/10/21 20:12:36 hofmann Exp $"
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
class PSEditText;

class PSEditText {
  int x, y;
  char *s;
  char *tag;
  PSEditText *next;
  
public:
  int c;
  int size;
  
  PSEditText(int x1, int y1, const char *s1, int size1);
  ~PSEditText();
  void append_text(const char*s1);
  void rm_char();
  void move(int x1, int y1);
  void append(PSEditText *g);
  PSEditText *get_match(int x1, int y1);
  char *get_text();
  char *get_tag();
  int set_tag(const char *t);
  int get_size();
  PSEditText *get_next();
  int get_color();
  int get_x();
  int get_y();
};

#endif
