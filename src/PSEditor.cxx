// 
// "$Id: PSEditor.cxx,v 1.6 2004/07/20 18:53:29 hofmann Exp $"
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
  switch(event) {
  case FL_PUSH:    
    if (Fl::event_button() == 1) {
      if (!file_loaded()) {
	fl_beep();
	return 0;
      }
      
      mark_x = Fl::event_x()-x();
      mark_y = Fl::event_y()-y();
      fprintf(stderr, "==> %d %d\n", mark_x, mark_y);    
      if (!set_cur_text(mark_x, mark_y) == 0) {
	new_text(mark_x, mark_y, "");
	mod++;
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
    break;
  case FL_DRAG:
    move(Fl::event_x()-x(), Fl::event_y()-y());
    mod++;
    return 1;
    break;
  case FL_KEYBOARD:
    {
      int del;
      int key = Fl::event_key();
      if (key == FL_BackSpace) {
	rm_char();  
	mod++;
      } else if (Fl::compose(del)) {
	if (del > 0) {
	  for (int i=0; i<del; i++) rm_char();
	  mod++;
	}
	if (Fl::event_length()) {
	  append_text(Fl::event_text());
	  mod++;
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


int PSEditor::modified() {
  return mod;
}

int PSEditor::file_loaded() {
  return loaded;
}

int PSEditor::load(char *f) {
  FILE *fp = fopen(f, "r");
  char tmpname[256];
  char linebuf[1024];
  int size, ret, ret1, ret2;
  PSParser *p1 = new PSParser_1(this);
  PSParser *p2 = new PSParser_2(this);
  
  strncpy(tmpname, "/tmp/PSEditorXXXXXX", 256);
  tmp_fd = mkstemp(tmpname);
  if (tmp_fd < 0) {
    fprintf(stderr, "Could not create temporary file (errno %d).\n", errno);
    return 1;
  }
  unlink(tmpname);

  clear_text();

  while (fgets(linebuf, 1024, fp) != NULL) {
    
    if (!p2->parse(linebuf) && !p1->parse(linebuf)) {
      ret = write(tmp_fd, linebuf, strlen(linebuf));
      if (ret != strlen(linebuf)) {
	fprintf(stderr, "Error while writing to temporary file\n");
      }
    }
  }
  fclose(fp);
  lseek(tmp_fd, 0L, SEEK_SET);

  delete(p1);
  delete(p2);

  mod = 0;
  loaded = 1;
  return GsWidget::load(tmp_fd);
}

int PSEditor::save(const char* savefile) {
  if (!file_loaded()) {
    return 1;
  }
  FILE *fp = fdopen(tmp_fd, "r");
  rewind(fp);
  FILE *sfp = fopen(savefile, "w");
  PSWriter *pw;
  
  if (ps_level == 2) {
    pw = new PSLevel2Writer(this);
  } else {
    pw = new PSLevel1Writer(this);
  }

  pw->write(fp, sfp);

  delete(pw);
   
  fclose(sfp);
  mod = 0;
  return 0;
}

int PSEditor::get_ps_level() {
  return ps_level;
}

void PSEditor::set_ps_level(int l) {
  if (l == 2) {
    ps_level = 2;
  } else {
    ps_level = 1;
  }
}
