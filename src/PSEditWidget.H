// 
// "$Id: PSEditWidget.H,v 1.9 2004/10/23 19:57:14 hofmann Exp $"
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

#ifndef PSEditWidget_H
#define PSEditWidget_H

#include "GsWidget.H"
#include "PSEditModel.H"

class PSText;

class PSEditWidget : public GsWidget {
private:
  int cur_size;
  int show_tags;

protected:
  PSEditModel *model;
  int loaded;
  int mod;
  void clear_text();
  void draw();

public: 
  PSEditWidget(int X,int Y,int W, int H);  
  int next();
  void new_text(int x1, int y1, const char *s, int p);
  void new_text(int x1, int y1, const char *s);
  int set_cur_text(int x1, int y1);
  void append_text(const char *s);
  void move(int x1, int y1, int last_x, int last_y);
  void rm_char();
  int reload();
  void set_cur_size(int s);
  void set_size(int s);
  int get_size();
  int get_max_pages();
  char *get_tag();
  int set_tag(const char *t);
  int get_show_tags();
  void set_show_tags(int s);
  PSText * get_text(int p);
  int modified();
  int file_loaded();
  int replace_tag(char* tag, char* text);
};

#endif
