// 
// "$Id: flpsed.cxx,v 1.11 2004/06/29 17:26:20 hofmann Exp $"
//
// flpsed program.
//
// Copyright 2004 by Johannes Hofmann
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

#include "PSEditWidget.H"

class PSEditor : public PSEditWidget {
  int mark_x, mark_y;

  int handle(int event) {
    switch(event) {
    case FL_PUSH:
      if (!file_loaded()) {
	fl_beep();
	return 0;
      }
      
      mark_x = Fl::event_x()-x();
      mark_y = Fl::event_y()-y();

      if (!set_cur_text(mark_x, mark_y) == 0) {
	new_text(mark_x, mark_y, "");
      }

      Fl::focus(this);
      return 1;
    case FL_DRAG:
      move(Fl::event_x()-x(), Fl::event_y()-y());
      return 1;
    case FL_KEYBOARD:
      {
	int del;
	int key = Fl::event_key();
        if (key == FL_BackSpace) {
          rm_char();  
        } else if (Fl::compose(del)) {
	  if (del > 0) {
	    for (int i=0; i<del; i++) rm_char();
	  }
	  if (Fl::event_length()) {
	    append_text(Fl::event_text());
	  }
	} else {
	  return 0;
	}

	return 1;
      }
    case FL_FOCUS:
      return 1;
    case FL_UNFOCUS:
      return 0;
    }
    return 0;
  }

public: 
  PSEditor(int X,int Y,int W, int H) : PSEditWidget(X, Y, W, H) {}
};


//
//  Main Program
// 

PSEditor *gsw_p;

int xev_handler(int ev) {
  if (gsw_p) {
    return gsw_p->handleX11(ev);
  } else {
    return 0;
  }
}

void save_cb();

int check_save(void) {
  if (!gsw_p->modified()) return 1;

  int r = fl_choice("The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Discard");

  if (r == 1) {
    save_cb(); // Save the file...
    return !gsw_p->modified();
  }

  return (r == 2) ? 1 : 0;
}


char filename[256] = "";

void open_cb() {
  if (!check_save()) return;
  char *file = fl_file_chooser("Open File?", "*.ps", filename);
  if(file != NULL) {
    gsw_p->load(file);
  }  
}

void first_cb() {
  gsw_p->reload();
}

void next_cb() {
  gsw_p->next();
}

void quit_cb() {
  if (!check_save()) return;
  delete gsw_p;
  exit(0);
}

void save_cb() {
  char *file = fl_file_chooser("Open File?", "*.ps", filename);
  if (file != NULL) {
    gsw_p->save(file);
  }
}

void print_cb() {
  char tmpname[256];
  char buf[256];
  int tmp_fd;

  int r = fl_ask("Print file?");
  if (r != 1) {
    return;
  }

  strncpy(tmpname, "/tmp/PSEditWidgetXXXXXX", 256);
  tmp_fd = mkstemp(tmpname);

  if (tmp_fd >= 0) {
    close(tmp_fd);
    if (gsw_p->save(tmpname) != 0) {
      fprintf(stderr, "Failed to print file\n");
    } else {
      snprintf(buf, 256, "lpr %s", tmpname);
      system(buf);
    }
    unlink(tmpname);
  } 
}

void about_cb() {
  fl_message("flpsed -- a pseudo PostScript editor\n"
	     "(c) Johannes Hofmann 2004\n\n"
	     "PostScript is a registered trademark of Adobe Systems");
}


void size_cb(Fl_Widget *w, void *) {
  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* m = mw->mvalue();
  if (m) {
    gsw_p->set_size(atoi(m->label()));
  }
}



Fl_Menu_Item menuitems[] = {
  { "&File",              0, 0, 0, FL_SUBMENU },
    { "&Open File...",    FL_CTRL + 'o', (Fl_Callback *)open_cb },
    { "&Save File as...", FL_CTRL + 's', (Fl_Callback *)save_cb },
    { "&Print...", FL_CTRL + 'p', (Fl_Callback *)print_cb, 0, FL_MENU_DIVIDER },
    { "&Quit", FL_CTRL + 'q', (Fl_Callback *)quit_cb, 0 },
    { 0 },

  { "&Page", 0, 0, 0, FL_SUBMENU },
    { "F&irst",        FL_CTRL + 'i', (Fl_Callback *)first_cb },
    { "&Next",       FL_CTRL + 'n', (Fl_Callback *)next_cb },
    { 0 },

  { "&Size", 0, 0, 0, FL_SUBMENU },
    { "8",  0, (Fl_Callback *)size_cb },
    { "10",  0, (Fl_Callback *)size_cb },
    { "12",  0, (Fl_Callback *)size_cb },    
    { "14",  0, (Fl_Callback *)size_cb },
    { "18",  0, (Fl_Callback *)size_cb },
    { "24",  0, (Fl_Callback *)size_cb },
  { 0 },

  { "&Help", 0, 0, 0, FL_SUBMENU },
    { "About",  0, (Fl_Callback *)about_cb },
  { 0 },

  { 0 }
};
  
int main(int argc, char** argv) {
  Fl_Window window(600,700);
  Fl_Menu_Bar* m = new Fl_Menu_Bar(0, 0, 600, 30);
  m->menu(menuitems);

  Fl_Scroll scroll(0, 30, window.w(), window.h()-30);
  gsw_p = new PSEditor(0, 0, 700, 900);
  scroll.end();
  fl_open_display();
  Fl::add_handler(xev_handler);

  window.resizable(scroll);

  window.end();
  window.callback((Fl_Callback *)quit_cb);
  window.show(1, argv); 


  //  for (int i=0; i<100; i++) Fl::check();
  //  if (argc >= 2) {
  //    gsw_p->load(argv[1]);
  //  }

  return Fl::run();
}

