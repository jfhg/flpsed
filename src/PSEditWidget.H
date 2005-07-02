// 
// "$Id: PSEditWidget.H,v 1.5 2004/06/25 17:25:17 hofmann Exp $"
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
  int mark_x, mark_y;
  PSText **text;
  int max_pages;
  int tmp_fd;    
  int cur_size;
  int mod, loaded;

protected:
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
  int load(char *f);
  int reload();
  void to_ps(FILE *f, int p);
  int save(const char* savefile);
  void set_cur_size(int s);
  void set_size(int s);
  int get_size();
  int modified();
  int file_loaded();
};


class PSText {
  int x, y;
  char *s;
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
  void draw(int off_x,int off_y);
  void string_to_ps(FILE *f, char *s); 
  void to_ps(FILE *f);
};

#endif
