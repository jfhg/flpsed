// 
// "$Id: PSEditWidget.cxx,v 1.24 2004/10/26 18:08:57 hofmann Exp $"
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

#define FLPSED_FONT FL_HELVETICA
#define FLPSED_TAG_FONT FL_COURIER
#define FLPSED_TAG_FONT_SIZE 10


void PSEditWidget::clear_text() {
  model->clear();
}

void PSEditWidget::draw() {
  GsWidget::draw();
  PSEditText *t = model->get_text(page);
  
  while (t) {
    fl_color((Fl_Color) t->get_color());
    fl_font(FLPSED_FONT, t->get_size());
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
      fl_font(FLPSED_TAG_FONT, FLPSED_TAG_FONT_SIZE);
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
  PSEditText *t_old, *t;
  
  t_old = model->get_cur_text();

  model->new_text(x1, y1, s, cur_size, p);

  t = model->get_cur_text();

  if (t) {
    damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
  }

  if (t_old) {
    damage(4, bb_x(t_old), bb_y(t_old), bb_w(t_old), bb_h(t_old));
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
      damage(4, bb_x(t_new), bb_y(t_new), bb_w(t_new), bb_h(t_new));
    }
    if (t_old) {
      damage(4, bb_x(t_old), bb_y(t_old), bb_w(t_old), bb_h(t_old));
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
    damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
  }
}

void PSEditWidget::move(int x1, int y1, int last_x, int last_y) {
  PSEditText *t;

  model->move(x1, y1);
  t = model->get_cur_text();
  if (t) {
    damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
    damage(4, last_x - 10, last_y - fl_height() - 20, bb_w(t), bb_h(t));
  }
}

void PSEditWidget::rm_char() {
  PSEditText *t;
  int width;

  t = model->get_cur_text();
  if (t) {
    fl_font(FLPSED_FONT, t->get_size());
    width =  bb_w(t);
  }

  model->rm_char();
  
  if (t) {
    damage(4, bb_x(t), bb_y(t), width, bb_h(t));
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
  int old_x, old_y, old_w, old_h;

  t = model->get_cur_text();
  if (t) {
    old_x = bb_x(t);
    old_y = bb_y(t);
    old_w = bb_w(t);
    old_h = bb_h(t);
  }

  set_cur_size(s);
  model->set_size(s);

  if (t) {
    damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
    damage(4, old_x, old_y, old_w, old_h);
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

int PSEditWidget::bb_x(PSEditText *t) {
  return t->get_x() - 10;
}

int PSEditWidget::bb_y(PSEditText *t) {
  fl_font(FLPSED_FONT, t->get_size());
  return t->get_y() - fl_height() - 20;
}

int PSEditWidget::bb_w(PSEditText *t) {
  int w, wt = 0;
  char *tag;
  fl_font(FLPSED_FONT, t->get_size());
  w = (int) fl_width(t->get_text()) + 20;

  tag = t->get_tag();
  if (tag) {
     fl_font(FLPSED_TAG_FONT, FLPSED_TAG_FONT_SIZE);
     wt = (int) fl_width(tag) + 20;
  }
  
  return w>=wt?w:wt;
}

int PSEditWidget::bb_h(PSEditText *t) {
  fl_font(FLPSED_FONT, t->get_size());
  return fl_height() + 30;
}

  
