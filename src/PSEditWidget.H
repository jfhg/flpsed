// 
// "$Id: PSEditWidget.H,v 1.7 2004/10/12 20:52:23 hofmann Exp $"
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

class PSText;

class PSEditWidget : public GsWidget {
private:
  PSText **text;
  int max_pages;
  int cur_size;
  int show_tags;

protected:
  int loaded;
  int mod;
  void clear_text();
  void draw();

public: 
  PSText *cur_text;

  PSEditWidget(int X,int Y,int W, int H);  
  int next();
  void new_text(int x1, int y1, const char *s, int p);
  void new_text(int x1, int y1, const char *s);
  int set_cur_text(int x1, int y1);
  void append_text(const char *s);
  void move(int x1, int y1);
  void rm_char();
  int ps_to_display_x(int x1);
  int ps_to_display_y(int y1);
  int ps_x(int x1);
  int ps_y(int y1);
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
};


class PSText {
  int x, y;
  char *s;
  char *tag;
  PSText *next;
  PSEditWidget *gsew;
  
public:
  Fl_Color c;
  int size;
  
  PSText(PSEditWidget *g, int x1, int y1, const char *s1, int size1);
  ~PSText();
  void append_text(const char*s1);
  void rm_char();
  void move(int x1, int y1);
  void append(PSText *g);
  PSText *get_match(int x1, int y1);
  char *get_text();
  char *get_tag();
  int set_tag(const char *t);
  int get_size();
  PSText *get_next();
  Fl_Color get_color();
  int get_x();
  int get_y();
  void draw(int off_x,int off_y);
};

#endif
