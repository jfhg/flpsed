// 
// "$Id: PSEditWidget.cxx,v 1.11 2004/06/29 20:05:39 hofmann Exp $"
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

#define PS_POS_FORMAT       "newpath %d %d moveto %% PSEditWidget\n"
#define PS_TEXT_FORMAT      "(%s) show %% PSEditWidget\n"
#define PS_FONT_SIZE_FORMAT "/HelveticaNeue-Roman findfont %d scalefont setfont %% PSEditWidget\n"
#define PS_GLYPH_FORMAT     "/%s glyphshow %% PSEditWidget\n"

static struct {
  const char *glyph;
  const char *c;
} glyph_char[] = {
  {"adieresis", "ä"}, 
  {"Adieresis", "Ä"}, 
  {"odieresis", "ö"}, 
  {"Odieresis", "Ö"}, 
  {"udieresis", "ü"}, 
  {"Udieresis", "Ü"}, 
  {"germandbls", "ß"}, 
  {"parenleft", "("}, 
  {"parenright", ")"}, 
  {"backslash", "\\"}, 
  {NULL, NULL}};

static const char * glyph_to_char(char *glyph) {
  int i=0;

  while(glyph_char[i].glyph != NULL) {
    if (strcmp(glyph_char[i].glyph, glyph) == 0) {
      return glyph_char[i].c;
      }
    i++;
  }
  
  return NULL;
}

static const char * char_to_glyph(char *c) {
  int i=0;
  
  while(glyph_char[i].glyph != NULL) {
    if (strncmp(glyph_char[i].c, c, 1) == 0) {
      return glyph_char[i].glyph;
    }
    i++;
  }
  
  return NULL;
}

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
  mod = 0;
  loaded = 0;
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
  GsWidget::next();
}  

void PSEditWidget::new_text(int x1, int y1, const char *s, int p) {
  cur_text = new PSText(this, x1, y1, s, cur_size);
  if (text[p]) {
    text[p]->append(cur_text);
  } else {
    text[p] = cur_text;
  }
  redraw();
  mod = 1;
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
    mod = 1;
    redraw();
  }
}

void PSEditWidget::move(int x1, int y1) {
  if (cur_text) {
    cur_text->move(x1, y1);
    mod = 1;
    redraw();
  }
}

void PSEditWidget::rm_char() {
  if (cur_text) {
    cur_text->rm_char();
    mod = 1;
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

int PSEditWidget::load(char *f) {
  FILE *fp = fopen(f, "r");
  char tmpname[256];
  char linebuf[1024];
  int p = 1;
  int x1, y1;
  char *s, *e, glyph[1024];
  int size, ret;
  
  strncpy(tmpname, "/tmp/PSEditWidgetXXXXXX", 256);
  tmp_fd = mkstemp(tmpname);
  if (tmp_fd < 0) {
    fprintf(stderr, "Could not create temporary file (errno %d).\n", errno);
    return 1;
  }
  unlink(tmpname);

  clear_text();

  while (fgets(linebuf, 1024, fp) != NULL) {
    
    if (strcmp(linebuf, "showpage\n") == 0) {
      p++;
    }
    
    if (strstr(linebuf, "% PSEditWidget")) {
      if (sscanf(linebuf, PS_FONT_SIZE_FORMAT, &size) == 1) {
	set_cur_size(size);
      } else if (sscanf(linebuf, PS_POS_FORMAT, &x1, &y1) == 2) {
	new_text(ps_to_display_x(x1), ps_to_display_y(y1), "", p);
      } else if (sscanf(linebuf, PS_GLYPH_FORMAT, glyph) == 1) {
	fprintf(stderr, "GLYPH %s\n", glyph);
	append_text(glyph_to_char(glyph));
      } else if ((s = strchr(linebuf, '(')) &&
		 (e = strrchr(linebuf, ')'))) {
	*e = '\0';
	s++;
	append_text(s);
      }
    } else {
      ret = write(tmp_fd, linebuf, strlen(linebuf));
      if (ret != strlen(linebuf)) {
	fprintf(stderr, "Error while writing to temporary file\n");
      }
    }
  }
  fclose(fp);
  lseek(tmp_fd, 0L, SEEK_SET);

  mod = 0;
  loaded = 1;
  return GsWidget::load(tmp_fd);
}

void PSEditWidget::to_ps(FILE *f, int p) {
  if (!text[p]) {
    return;
  }
  
  text[p]->to_ps(f);
}

int PSEditWidget::save(const char* savefile) {
  if (!file_loaded()) {
    return 1;
  }
  FILE *fp = fdopen(tmp_fd, "r");
  rewind(fp);
  FILE *sfp = fopen(savefile, "w");
  char linebuf[1024];
  int p = 1;
  
  while (fgets(linebuf, 1024, fp) != NULL) {
    if (strcmp(linebuf, "showpage\n") == 0) {
      if (p < max_pages) {
	to_ps(sfp, p);
      }
      p++;
    }
    fprintf(sfp, "%s", linebuf);
  }
  
  fclose(sfp);
  mod = 0;
  return 0;
}

int PSEditWidget::modified() {
  return mod;
}

int PSEditWidget::file_loaded() {
  return loaded;
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

void PSText::string_to_ps(FILE *f, char *s) {
  const char *glyph;
  
  if (strlen(s) == 0) {
    return;
  } else if ((glyph = char_to_glyph(s)) != NULL) {
    fprintf(f, PS_GLYPH_FORMAT, glyph);
    string_to_ps(f, &(s[1]));
    return;
  } else {
    for(int i=0; i<strlen(s); i++) {
      if ((glyph = char_to_glyph(&(s[i]))) != NULL) {
	char *s1 = strdup(s);
	s1[i] = '\0';
	fprintf(f, PS_TEXT_FORMAT, s1);
	free(s1);
	string_to_ps(f, &(s[i]));
	return;
	}
    }
    fprintf(f, PS_TEXT_FORMAT, s);
  }
  return;
}
  
void PSText::to_ps(FILE *f) {
  if (strcmp(s, "") != 0) {
    fprintf(f, PS_FONT_SIZE_FORMAT, size);
    fprintf(f, PS_POS_FORMAT, gsew->ps_x(x), gsew->ps_y(y));
    string_to_ps(f, s);
  }
  
  if (next) {
    next->to_ps(f);
  }
}



