// 
// "$Id: PSEditModel.H,v 1.8 2004/10/26 16:41:40 hofmann Exp $"
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

#ifndef PSEditModel_H
#define PSEditModel_H

#include "PSEditText.H"

class PSEditModel {
  PSEditText **text;
  int max_pages;
  int page;
  PSEditText *cur_text;  
  float xdpi, ydpi;
  int paper_x, paper_y;


public:
  PSEditModel(int x1, int y1, float dx, float dy);
  ~PSEditModel();

  void clear();
  void new_text(int x1, int y1, const char *s, int size, int p);
  void append_text(const char *s);
  void move(int x1, int y1);
  void rm_char();
  void set_size(int s);
  int get_size();
  void set_page(int p);
  int get_page();
  char *get_tag();
  int set_tag(const char *t);
  int get_max_pages();
  int set_cur_text(int x1, int y1, int p);
  int is_cur_text(PSEditText *t);
  PSEditText *get_text(int p);
  PSEditText *get_cur_text();
  int replace_tag(char* tag, char* text);
  int dump_tags();
  int ps_to_display_x(int x1);
  int ps_to_display_y(int y1);
  int ps_x(int x1);
  int ps_y(int y1);
  int PSEditModel::load(FILE *fp);  
  int save(FILE *sfp, int tmp_fd);

};

#endif
