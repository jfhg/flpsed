//
// "$Id: GsWidget.cxx,v 1.17 2005/04/19 20:22:23 hofmann Exp $"
//
// GsWidget routines.
//
// Copyright 2004 by Johannes Hofmann.
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
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <FL/fl_draw.H>

#include "GsWidget.H"


void GsWidget::draw() {
  if (!offscreen) {
    offscreen = fl_create_offscreen(w(), h());
    fl_begin_offscreen(offscreen);
    fl_color(FL_WHITE);
    fl_rectf(0, 0, w(), h());
    fl_end_offscreen();

    if (reload_needed) {
      reload();
      reload_needed = 0;
    }
  }
  fl_push_clip(x(), y(), w(), h());
  fl_copy_offscreen(x(), y(), w(), h(), offscreen, 0, 0); 
  fl_pop_clip();
}

void GsWidget::setProps() {
  char data[512];

  if (!offscreen) {
    offscreen = fl_create_offscreen(w(), h());
  }

  atoms[0] = XInternAtom(fl_display,"GHOSTVIEW" , false);
  atoms[1] = XInternAtom(fl_display,"GHOSTVIEW_COLORS" , false);
  atoms[2] = XInternAtom(fl_display,"NEXT" , false);
  atoms[3] = XInternAtom(fl_display,"PAGE" , false);
  atoms[4] = XInternAtom(fl_display,"DONE" , false);
   
  snprintf(data, 512, "%d %d %d %d %d %d %d.0 %d.0",
	   0, 0, 0, 0, paper_x, paper_y, xdpi, ydpi);

  int xid = fl_xid(window());

  XChangeProperty(fl_display, xid, atoms[0],
		  XA_STRING, 8, PropModeReplace,
		  (unsigned char*) data, strlen(data));
    
  snprintf(data, 512, "%s %d %d", "Color", 
    (int) BlackPixel(fl_display, DefaultScreen(fl_display)),
    (int) WhitePixel(fl_display, DefaultScreen(fl_display)));
    
  XChangeProperty(fl_display, xid, atoms[1],
		  XA_STRING, 8, PropModeReplace,
		  (unsigned char*) data, strlen(data));
}

void GsWidget::kill_gs() {
  int status;

  if (gs_pid > 0) {
    kill(gs_pid, SIGTERM);
    waitpid(gs_pid, &status, 0);
    gs_pid = 0;
  }
}

bool GsWidget::gs_active() {
  return gs_pid > 0 && gs_win;
}

GsWidget::GsWidget(int X,int Y,int W, int H) : Fl_Widget(X, Y, W, H) { 
  offscreen = 0;
  gs_pid = 0;
  page = 0;
  zoom_percent = 100;
  xdpi = 75 * zoom_percent / 100;
  ydpi = 75 * zoom_percent / 100;
  paper_x = 594; // DIN A4
  paper_y = 841; //
  initial_width = W;
  initial_height = H;
  in_fd = -1;
  reload_needed = 0;
}
  
GsWidget::~GsWidget() {
  kill_gs();
  if (offscreen) {
    fl_delete_offscreen(offscreen);
  }
}

int GsWidget::load(char *f) {
  int fd = open(f, O_RDONLY); 
  if (fd == -1) {
    perror("open");
    return 1;
  }
  return load(fd);
}

int GsWidget::load(int fd) {
  if (in_fd >= 0 && fd != in_fd) {
    close (in_fd);
  }

  in_fd = fd;

  fl_cursor(FL_CURSOR_WAIT);
  kill_gs();

  if (!offscreen) {
    reload_needed = 1;
    return 0;
  }

  setProps();
    
  pid_t pid = fork();
  if (pid == (pid_t) 0) {
    char *argv[16];
    char gvenv[256];
    int d_null = open("/dev/null", O_WRONLY);
      
    dup2(d_null, STDOUT_FILENO);
    close(d_null);
    dup2(in_fd, STDIN_FILENO);
    snprintf(gvenv, 256, "%d %d", (int) fl_xid(window()), (int) offscreen);

    setenv("GHOSTVIEW", gvenv, 1);
    argv[0] = "gs";
    argv[1] = "-dSAFER";
    argv[2] = "-dQUIET";
    argv[3] = "-sDEVICE=x11alpha";
    argv[4] = "-dNOPLATFONTS";
    argv[5] = "-";
    argv[6] = NULL;
    execvp(argv[0], argv);
    perror("exec");
    fprintf(stderr, "Please install ghostscript and make sure 'gs' "
	    "is in the PATH.\n");
    exit(1);
  } else {
    gs_pid = pid;
    page = 0;
  }
  
  return 0;
}

int GsWidget::reload() {
  int ret;

  if (in_fd >= 0) {
    ret = lseek(in_fd, 0L, SEEK_SET);
    if (ret == -1) {
      perror("lseek");
      return 1;
    }
    load(in_fd);
    return 0;
  } else {
    return 1;
  }
}
    
int GsWidget::next() {
  if (!gs_active()) {
    return 1;
  } else {
    fl_cursor(FL_CURSOR_WAIT);

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.display = fl_display;
    e.xclient.window = gs_win;
    e.xclient.message_type = atoms[2];
    e.xclient.format = 32;

    XSendEvent(fl_display, gs_win, false, 0, &e);
    XFlush(fl_display);
  }

  return 0;
}

int GsWidget::handleX11(int ev) {
  if (fl_xevent->type == ClientMessage) {
    gs_win = fl_xevent->xclient.data.l[0];
    
    if(fl_xevent->xclient.message_type == atoms[3]) {
      page++;                               // PAGE revceived
      damage(FL_DAMAGE_ALL);
      fl_cursor(FL_CURSOR_DEFAULT);
    } else if(fl_xevent->xclient.message_type == atoms[4] ) {
      reload();                            // go back to page 1 
    }
    return 1;
  }
  return 0;
}

int GsWidget::get_page() {
  return page;
}

int GsWidget::zoom(int p) {
  zoom_percent = p;

  kill_gs();
  
  if (offscreen) {
    // Clear widget with current size
    fl_begin_offscreen(offscreen);
    fl_color(FL_WHITE);
    fl_rectf(0, 0, w(), h());
    fl_end_offscreen();
    redraw();

    fl_delete_offscreen(offscreen);
    offscreen = 0;
  }

  w(initial_width * zoom_percent / 100);
  h(initial_height * zoom_percent / 100);

  xdpi = 75 * zoom_percent / 100;
  ydpi = 75 * zoom_percent / 100;
  reload();

  return 0;
}
