//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef PSEditWidget_H
#define PSEditWidget_H

#include "GsWidget.H"
#include "PSEditModel.H"

typedef void (PSEditCallback)(); 

class PSText;

class PSEditWidget : public GsWidget {
private:
  int cur_size;

  PSEditColor cur_text_color;

  int show_tags;

  int zoom_percent;

  PSEditCallback *property_changed_cb;

protected:
  PSEditModel *model;

  int loaded;

  int mod;

  void clear_text();

  void draw();

public: 
  PSEditWidget(int X,int Y,int W, int H);  

  int next();

  void new_text(int x1, int y1, const char *s, int p);

  void new_text(int x1, int y1, const char *s);

  int set_cur_text(int x1, int y1);
  
  int next_text();
  
  void append_text(const char *s);

  void move(int x1, int y1);

  void rel_move(int dx, int dy);

  void rm_char();

  int reload();

  void set_size(int s);

  int get_size();

  void set_color(const PSEditColor *c);

  void get_color(PSEditColor *c);

  int get_max_pages();

  char *get_tag();

  int set_tag(const char *t);

  int get_show_tags();

  void set_show_tags(int s);

  PSText * get_text(int p);

  int modified();

  int file_loaded();

  int replace_tag(char* tag, char* text);

  int zoom(int p);

  void property_changed_callback(PSEditCallback *cb) {
    property_changed_cb = cb;};

 private:
  int bb_x(PSEditText *t);

  int bb_y(PSEditText *t);

  int bb_w(PSEditText *t);
  
  int bb_h(PSEditText *t);

  int ps_to_display_x(int x1);

  int ps_to_display_y(int y1);

  int ps_x(int x1);

  int ps_y(int y1);
};

#endif
