// 
// "$Id: PSEditWidget.cxx,v 1.23 2004/10/26 17:22:45 hofmann Exp $"
//
// PSEditWidget routines.
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
#include <FL/fl_draw.H>
#include <FL/x.H>

#include "PSEditWidget.H"

void PSEditWidget::clear_text() {
  model->clear();
}

void PSEditWidget::draw() {
  GsWidget::draw();
  PSEditText *t = model->get_text(page);
  
  while (t) {
    fl_color((Fl_Color) t->get_color());
    fl_font(FL_HELVETICA, t->get_size());
    fl_draw(t->get_text(), t->get_x() + x(), t->get_y() + y());
    if (model->is_cur_text(t)) {
      fl_draw_box(FL_BORDER_FRAME, 
		  t->get_x()+x()-1, 
		  t->get_y()+y()-fl_height()+fl_descent(),
		  (int) fl_width(t->get_text())+2, 
		  fl_height(), 
		  FL_BLACK);
    }
    
    if (t->get_tag() && show_tags) {
      int text_height = fl_height() - fl_descent();
      fl_color(FL_BLUE);
      fl_font(FL_COURIER, 10);
      fl_draw(t->get_tag(), t->get_x() + x(), 
	      t->get_y() + y() - text_height - 1);
    }
  
    t = t->get_next();
  }
}

PSEditWidget::PSEditWidget(int X,int Y,int W, int H) : GsWidget(X, Y, W, H) {
  model = new PSEditModel(paper_x, paper_y, xdpi, ydpi);
  cur_size = 12;
  show_tags = 1;
}
  
int PSEditWidget::next() {
  model->set_page(page);
  return GsWidget::next();
}

void PSEditWidget::new_text(int x1, int y1, const char *s, int p) {
  PSEditText *t_old;
  
  t_old = model->get_cur_text();

  model->new_text(x1, y1, s, cur_size, p);

  fl_font(FL_HELVETICA, cur_size);
  damage(4, x1 - 10, y1 - fl_height() - 20, 
	 fl_width(s) + 20, fl_height() + 30);

  if (t_old) {
    fl_font(FL_HELVETICA, t_old->get_size());
    damage(4, t_old->get_x() - 10, t_old->get_y() - fl_height() - 20, 
	   fl_width(t_old->get_text()) + 20, fl_height() + 30);
  }
}

void PSEditWidget::new_text(int x1, int y1, const char *s) {
  new_text(x1, y1, s, page);
}

int PSEditWidget::set_cur_text(int x1, int y1) {
  PSEditText *t_new, *t_old;

  t_old = model->get_cur_text();

  if (model->set_cur_text(x1, y1, page) == 0) {

    t_new = model->get_cur_text();

    if (t_new) {
      fl_font(FL_HELVETICA, t_new->get_size());
      damage(4, t_new->get_x() - 10, t_new->get_y() - fl_height() - 20, 
	     fl_width(t_new->get_text()) + 20, fl_height() + 30);
    }
    if (t_old) {
      fl_font(FL_HELVETICA, t_old->get_size());
      damage(4, t_old->get_x() - 10, t_old->get_y() - fl_height() - 20, 
	     fl_width(t_old->get_text()) + 20, fl_height() + 30);
    }
    return 0;
  }
  return 1;
}

void PSEditWidget::append_text(const char *s) {
  PSEditText *t;

  model->append_text(s);

  t = model->get_cur_text();
  if (t) {
    fl_font(FL_HELVETICA, t->get_size());
    damage(4, t->get_x() - 10, t->get_y() - fl_height() - 20, fl_width(t->get_text()) + 20, fl_height() + 30);
  }
}

void PSEditWidget::move(int x1, int y1, int last_x, int last_y) {
  PSEditText *t;

  model->move(x1, y1);
  t = model->get_cur_text();
  if (t) {
    fl_font(FL_HELVETICA, t->get_size());
    damage(4, x1 - 10, y1 - fl_height() - 20, fl_width(t->get_text()) + 20, fl_height() + 30);
    damage(4, last_x - 10, last_y - fl_height() - 20, fl_width(t->get_text()) + 20, fl_height() + 30);

  }
}

void PSEditWidget::rm_char() {
  PSEditText *t;
  double width;

  t = model->get_cur_text();
  if (t) {
    fl_font(FL_HELVETICA, t->get_size());
    width = fl_width(t->get_text());
  }

  model->rm_char();
  
  if (t) {
    damage(4, t->get_x() - 10, t->get_y() - fl_height() - 20, width + 20, fl_height() + 30);
  }
}


int PSEditWidget::reload() {
  model->set_page(0);
  return GsWidget::reload();
}

void PSEditWidget::set_cur_size(int s) {
  cur_size = s;
}

void PSEditWidget::set_size(int s) {
  PSEditText *t;
  int old_size;

  t = model->get_cur_text();
  if (t) {
    old_size = t->get_size();
  }

  set_cur_size(s);
  model->set_size(s);

  if (t) {
    fl_font(FL_HELVETICA, t->get_size());
    damage(4, t->get_x() - 10, t->get_y() - fl_height() - 20, fl_width(t->get_text()) + 20, fl_height() + 30);
    fl_font(FL_HELVETICA, old_size);
    damage(4, t->get_x() - 10, t->get_y() - fl_height() - 20, fl_width(t->get_text()) + 20, fl_height() + 30);
  }

}

int PSEditWidget::get_size() {
  int s;

  s = model->get_size();
  if (s >= 0) {
    return s;
  } else {
    return cur_size;
  }
}

int PSEditWidget::get_max_pages() {
  return model->get_max_pages();
}

int PSEditWidget::get_show_tags() {
  return show_tags;
}

void PSEditWidget::set_show_tags(int s) {
  show_tags = s;
  redraw();
}

int PSEditWidget::set_tag(const char *t) {
  if (model->set_tag(t) == 0) {
    mod++;
    redraw();
    return 0;
  } else {
    return 1;
  }
}

char *PSEditWidget::get_tag() {
  return model->get_tag();
}

int PSEditWidget::modified() {
  return mod;
}

int PSEditWidget::file_loaded() {
  return loaded;
}

int PSEditWidget::replace_tag(char *tag, char *text) {
    return model->replace_tag(tag, text);
}

