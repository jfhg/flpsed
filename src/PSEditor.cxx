// 
// "$Id: PSEditor.cxx,v 1.20 2004/11/08 18:10:34 hofmann Exp $"
//
// PSEditor routines.
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

#include <errno.h>
#include <FL/fl_ask.H>
#include "PSEditor.H"
#include "Postscript.H"

PSEditor::PSEditor(int X,int Y,int W, int H) : PSEditWidget(X, Y, W, H) {
  loaded = 0;
  mod = 0;
  ps_level = 1;
}


int PSEditor::handle(int event) {
  int mark_x, mark_y;

  switch(event) {
  case FL_PUSH:    
    if (Fl::event_button() == 1) {
      if (!file_loaded()) {
	fl_beep();
	return 0;
      }
      
      x_last = Fl::event_x()-x();
      y_last = Fl::event_y()-y();
   
      mark_x = Fl::event_x()-x();
      mark_y = Fl::event_y()-y();

      if (!set_cur_text(mark_x, mark_y) == 0) {
	new_text(mark_x, mark_y, "");
      }

      Fl::focus(this);
      return 1;
    }
    break;
  case FL_RELEASE:
    if (Fl::event_button() == 2) {
      Fl::paste(*this, 0);
      return 1;
    }

    x_last = -1;
    y_last = -1;

    break;
  case FL_DRAG:
    move(Fl::event_x()-x(), Fl::event_y()-y(), x_last-x(), y_last-y());

    x_last = Fl::event_x()-x();
    y_last = Fl::event_y()-y();

    return 1;
    break;
  case FL_KEYBOARD:
    {
      int del;
      int key = Fl::event_key();
      if (key == FL_BackSpace) {
	rm_char();  
      } else if (key == FL_Left) {
	PSEditText *t = model->get_cur_text();
	if (t) {
	  int x = t->get_x();
	  int y = t->get_y();
	  move(x - 1, y, x, y);
	}
      } else if (key == FL_Right) {
	PSEditText *t = model->get_cur_text();
	if (t) {
	  int x = t->get_x();
	  int y = t->get_y();
	  move(x + 1, y, x, y);
	}
      } else if (key == FL_Up) {
	PSEditText *t = model->get_cur_text();
	if (t) {
	  int x = t->get_x();
	  int y = t->get_y();
	  move(x, y - 1, x, y);
	}
      } else if (key == FL_Down) {
	PSEditText *t = model->get_cur_text();
	if (t) {
	  int x = t->get_x();
	  int y = t->get_y();
	  move(x, y + 1, x, y);
	}
      } else if (key == FL_Tab) {
	next_text();
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
    break;
  case FL_PASTE:
    append_text(Fl::event_text());
    return 1;
    break;
  case FL_FOCUS:
    return 1;
    break;
  case FL_UNFOCUS:
    return 0;
    break;
  }
  return 0;
}


int PSEditor::load(FILE *fp) {
  if (tmp_fd) {
    close(tmp_fd);
  }
  
  tmp_fd = model->load(fp);

  if (tmp_fd < 0) {
    return 1;
  } else {
    mod = 0;
    loaded = 1;
    return GsWidget::load(tmp_fd);
  }
}

int PSEditor::load(char *f) {
  FILE *fp;
  int ret;

  fp = fopen(f, "r");
  if (!fp) {
    perror("fopen");
    return 1;
  }

  ret = load(fp);
  fclose(fp);

  return ret;
}

int PSEditor::save(const char* savefile) {
  FILE *fp;
  int ret;

  if (!file_loaded()) {
    return 1;
  }
  
  fp = fopen(savefile, "w");
  if (!fp) {
    perror("fopen");
    return 1;
  }
  

  ret = model->save(fp, tmp_fd);

  if (ret == 0) {
    mod = 0;
  }

  fclose(fp);

  return ret;
}

int PSEditor::import(char *f) {
  FILE *fp;
  char linebuf[1024];
  PSParser *p1;
  PSParser *p2;

  if (!file_loaded()) {
    return 1;
  }
  
  fp = fopen(f, "r");
  if (!fp) {
    fprintf(stderr, "Could not open file %s.\n", f);
    return 1;
  }

  p1 = new PSParser_1(model);
  p2 = new PSParser_2(model);
  while (fgets(linebuf, 1024, fp) != NULL) {
    if (!p2->parse(linebuf)) {
      p1->parse(linebuf);
    }
  }

  delete(p1);
  delete(p2);
  
  mod = 1;
  redraw();
  return 0;
}
