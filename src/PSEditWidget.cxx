// 
// "$Id: PSEditWidget.cxx,v 1.15 2004/10/12 17:14:16 hofmann Exp $"
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
  cur_text = NULL;
  for (int i = 0; i < max_pages; i++) {
    if (text[i]) {
      delete(text[i]);
      text[i] = NULL;
    }
  }
}

void PSEditWidget::draw() {
  GsWidget::draw();
  if (text[page]) {
    text[page]->draw(x() ,y());
  }
}

PSEditWidget::PSEditWidget(int X,int Y,int W, int H) : GsWidget(X, Y, W, H) {
  max_pages = 32;
  text = (PSText**) malloc(sizeof(PSText*) * max_pages);
  for (int i = 0; i < max_pages; i++) {
    text[i] = NULL;
  }
  cur_text = NULL;
  cur_size = 12;
}
  
int PSEditWidget::next() {
  if (page >= max_pages) {
    max_pages = max_pages * 2;
    text = (PSText**) realloc(text, sizeof(PSText*) * max_pages);
    for (int i = max_pages / 2; i < max_pages; i++) {
      text[i] = NULL;
    }
  }
  cur_text = NULL;
  return GsWidget::next();
}  

void PSEditWidget::new_text(int x1, int y1, const char *s, int p) {
  cur_text = new PSText(this, x1, y1, s, cur_size);
  if (text[p]) {
    text[p]->append(cur_text);
  } else {
    text[p] = cur_text;
  }
  redraw();
}

void PSEditWidget::new_text(int x1, int y1, const char *s) {
  new_text(x1, y1, s, page);
}

int PSEditWidget::set_cur_text(int x1, int y1) {
  if (text[page]) {
    cur_text = text[page]->get_match(x1, y1);
    if (cur_text) {
      redraw();
      return 0;
    }
  }
  return 1;
}

void PSEditWidget::append_text(const char *s) {
  if (cur_text && s) {
    cur_text->append_text(s);
    redraw();
  }
}

void PSEditWidget::move(int x1, int y1) {
  if (cur_text) {
    cur_text->move(x1, y1);
    redraw();
  }
}

void PSEditWidget::rm_char() {
  if (cur_text) {
    cur_text->rm_char();
    redraw();
  }
}

int PSEditWidget::ps_to_display_x(int x1) {
  return (int) ((float) x1 * xdpi / 72.0);
}

int PSEditWidget::ps_to_display_y(int y1) {
  return (int) ((float) (paper_y - y1) * xdpi / 72.0);
}

int PSEditWidget::ps_x(int x1) {
  return (int) ((float) x1 * 72.0 / xdpi);
}

int PSEditWidget::ps_y(int y1) {
  return paper_y - (int)((float) y1 * 72.0 / ydpi);
}

int PSEditWidget::reload() {
  cur_text = NULL;
  return GsWidget::reload();
}

void PSEditWidget::set_cur_size(int s) {
  cur_size = s;
}

void PSEditWidget::set_size(int s) {
  set_cur_size(s);
  if (cur_text) {
    cur_text->size = s;
    redraw();
  }
}

int PSEditWidget::get_size() {
  if (cur_text) {
    return cur_text->size;
  } else {
    return cur_size;
  }
}


int PSEditWidget::get_max_pages() {
  return max_pages;
}

PSText *PSEditWidget::get_text(int p) {
  if (p >= max_pages) {
    return 0;
  } else {
    return text[p];
  }
}



PSText::PSText(PSEditWidget *g, int x1, int y1, const char *s1, int size1) {
  x = x1;
  y = y1;
  s = strdup(s1);
  c = FL_BLACK;
  size = size1;
  next = NULL;
  gsew = g;
}

PSText::~PSText() {
  if (next) {
    delete(next);
  }
  if (s) {
    free(s);
  }
}
  
void PSText::append_text(const char*s1) {
  int len = (s?strlen(s):0) + strlen(s1) + 1;
  char *tmp = (char*) malloc(len);
  
  strncpy(tmp, s?s:"", len);
  strncat(tmp, s1, len - strlen(tmp));
  
  if (s) {
    free(s);
  }
  
  s = tmp;
}

void PSText::rm_char() {
  if (s && strlen(s) > 0) {
    s[strlen(s) - 1] = '\0';
  }
}

void PSText::move(int x1, int y1) {
  x = x1;
  y = y1;
}

void PSText::append(PSText *g) {
  PSText *p = this;
  while (p->next) {
    p = p->next;
  }
  p->next = g;
}

PSText *PSText::get_match(int x1, int y1) {
  if (abs(x - x1) < 10 && abs(y - y1) < 10) {
    return this;
  } else if (next) {
    return next->get_match(x1, y1);
  } else {
      return NULL;
  }
}

void PSText::draw(int off_x,int off_y) {
  PSText *p = this;
  fl_color(c);
  fl_font(FL_HELVETICA, size);
  fl_draw(s, x + off_x, y + off_y);
  if (gsew->cur_text == this) {
    fl_draw_box(FL_BORDER_FRAME, x+off_x-1, y+off_y-fl_height()+fl_descent(), (int) fl_width(s)+2, fl_height(), FL_BLACK);
  }
  if (p->next) {
    p->next->draw(off_x, off_y);
  }
}

char *PSText::get_text() {
  return s;
}

int PSText::get_size() {
  return size;
}

Fl_Color PSText::get_color() {
  return c;
}

PSText* PSText::get_next() {
  return next;
}

int PSText::get_x() {
  return x;
}

int PSText::get_y() {
  return y;
}
