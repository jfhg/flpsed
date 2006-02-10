// 
// "$Id: GsWidget.H,v 1.4 2005/01/27 21:07:09 hofmann Exp $"
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

#include "PostscriptDSC.H"

class GsWidget : public Fl_Widget {
private:
  PostscriptDSC *dsc;
  int gs_win;
  Atom atoms[5];
  pid_t gs_pid;
  int display_x, display_y;
  int in_fd, reload_needed;
  int zoom_percent;
  int initial_width, initial_height;
  int feeding;

protected:
  int page;
  Fl_Offscreen offscreen;
  int xdpi, ydpi;
  int paper_x, paper_y;

  void draw();

public:
  GsWidget(int X,int Y,int W, int H);
    
  ~GsWidget();
  
  int open_file(char *f);

  int open_file(int fd);

  int load();

  int load_page(int p);

  int reload();

  int next();

  int prev();

  int handleX11(int ev);

  int zoom(int p);

  int get_page();

  int get_pages();

private:
  void setProps();
	
  void exec_gs();

  void kill_gs();

  int fd_copy(int to, int from, size_t len);

  bool gs_active();
};
#endif
