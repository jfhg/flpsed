// 
// "$Id: GsWidget.H,v 1.3 2004/11/10 18:49:08 hofmann Exp $"
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

#ifndef GsWidget_H
#define GsWidget_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>


class GsWidget : public Fl_Widget {
  int gs_win;
  Atom atoms[5];
  pid_t gs_pid;
  int display_x, display_y;
  int in_fd, reload_needed;

protected:
  int page;
  Fl_Offscreen offscreen;
  int xdpi, ydpi;
  int paper_x, paper_y;

  void draw();

public:
  GsWidget(int X,int Y,int W, int H);
    
  ~GsWidget();
   
  int load(char *f);

  int load(int fd);

  int reload();

  int next();

  int handleX11(int ev);

  int get_page();

private:
  void setProps();

  void kill_gs();

  bool gs_active();
};
#endif
