// 
// "$Id: PSEditWidget.cxx,v 1.19 2004/10/21 20:12:36 hofmann Exp $"
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
  model->new_text(x1, y1, s, cur_size, p);
  redraw();
}

void PSEditWidget::new_text(int x1, int y1, const char *s) {
  new_text(x1, y1, s, page);
}

int PSEditWidget::set_cur_text(int x1, int y1) {
  if (model->set_cur_text(x1, y1, page) == 0) {
    redraw();
    return 0;
  }
  return 1;
}

void PSEditWidget::append_text(const char *s) {
  model->append_text(s);
  redraw();
}

void PSEditWidget::move(int x1, int y1) {
  model->move(x1, y1);
  redraw();
}

void PSEditWidget::rm_char() {
  model->rm_char();
  redraw();
}


int PSEditWidget::reload() {
  model->set_page(0);
  return GsWidget::reload();
}

void PSEditWidget::set_cur_size(int s) {
  cur_size = s;
}

void PSEditWidget::set_size(int s) {
  set_cur_size(s);
  model->set_size(s);
  redraw();
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
  fprintf(stderr, "%s => %s\n", tag, text);

  return 0;
}

