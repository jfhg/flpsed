//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

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
	int t_x, t_y;

	while (t) {
		t_x = ps_to_display_x(t->get_x());
		t_y = ps_to_display_y(t->get_y());

		fl_color(fl_rgb_color(t->text_color.get_r(),
				t->text_color.get_g(),
				t->text_color.get_b()));


		fl_font(FLPSED_FONT, t->get_size() * zoom_percent / 100);
		fl_draw(t->get_text(), t_x + x(), t_y + y());
		if (model->is_cur_text(t)) {
			fl_draw_box(FL_BORDER_FRAME, 
				t_x + x()-1, 
				t_y + y()-fl_height()+fl_descent(),
				(int) fl_width(t->get_text())+2, 
				fl_height(), 
				FL_BLACK);
		}

		if (t->get_tag() && show_tags) {
			int text_height = fl_height() - fl_descent();
			fl_color(FL_BLUE);
			fl_font(FLPSED_TAG_FONT, FLPSED_TAG_FONT_SIZE * zoom_percent / 100);
			fl_draw(t->get_tag(), t_x + x(), 
				t_y + y() - text_height - 1);
		}

		t = t->get_next();
	}
}

PSEditWidget::PSEditWidget(int X,int Y,int W, int H): GsWidget(X, Y, W, H) {
	model = new PSEditModel();
	cur_size = 12;
	cur_text_color.set(0.0, 0.0, 0.0);
	show_tags = 1;
	zoom_percent = 100;
	property_changed_cb = NULL;
}

int PSEditWidget::next() {
	model->set_page(page);  
	PSEditText *t_new;

	t_new = model->get_cur_text();

	if (t_new) {
		cur_text_color.set(&t_new->text_color);
		cur_size = t_new->size;

		if (property_changed_cb) {
			property_changed_cb();
		}
	}  

	return GsWidget::next();
}

void PSEditWidget::new_text(int x1, int y1, const char *s, int p) {
	PSEditText *t_old, *t;

	t_old = model->get_cur_text();

	model->new_text(ps_x(x1), ps_y(y1), s, cur_size, p, &cur_text_color);

	t = model->get_cur_text();

	if (t) {
		if (!t->is_empty()) {
			mod++;
		}
		damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
	}

	if (t_old) {
		damage(4, bb_x(t_old), bb_y(t_old), bb_w(t_old), bb_h(t_old));
	}
}

void PSEditWidget::new_text(int x1, int y1, const char *s) {
	new_text(x1, y1, s, get_page());
}

int PSEditWidget::set_cur_text(int x1, int y1) {
	PSEditText *t_new, *t_old;

	t_old = model->get_cur_text();

	if (model->set_cur_text(ps_x(x1), ps_y(y1), page) == 0) {

		t_new = model->get_cur_text();

		if (t_new) {
			cur_text_color.set(&t_new->text_color);
			cur_size = t_new->size;
			if (property_changed_cb) {
				property_changed_cb();
			}

			damage(4, bb_x(t_new), bb_y(t_new), bb_w(t_new), bb_h(t_new));
		}
		if (t_old) {
			damage(4, bb_x(t_old), bb_y(t_old), bb_w(t_old), bb_h(t_old));
		}

		return 0;
	}
	return 1;
}

int PSEditWidget::next_text() {
	PSEditText *t_new, *t_old;
	int ret;

	t_old = model->get_cur_text();

	ret = model->next_text(page);

	t_new = model->get_cur_text();

	if (t_new) {
		cur_text_color.set(&t_new->text_color);
		cur_size = t_new->size;
		if (property_changed_cb) {
			property_changed_cb();
		}

		damage(4, bb_x(t_new), bb_y(t_new), bb_w(t_new), bb_h(t_new));
	}

	if (t_old) {
		damage(4, bb_x(t_old), bb_y(t_old), bb_w(t_old), bb_h(t_old));
	}

	return ret;
}

void PSEditWidget::append_text(const char *s) {
	PSEditText *t;

	model->append_text(s);
	mod++;
	t = model->get_cur_text();
	if (t) {
		damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
	}
}

void PSEditWidget::move(int x1, int y1) {
	PSEditText *t;
	int old_bbx, old_bby, old_bbw, old_bbh;

	t = model->get_cur_text();
	if (t) {
		old_bbx = bb_x(t);
		old_bby = bb_y(t);
		old_bbw = bb_w(t);
		old_bbh = bb_h(t);

		model->move(ps_x(x1), ps_y(y1));
		if (!t->is_empty() || t->get_tag()) {
			mod++;
		}

		damage(4, old_bbx, old_bby, old_bbw, old_bbh);
		damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
	}
}

void PSEditWidget::rel_move(int dx, int dy) {
	PSEditText *t;
	int old_bbx, old_bby, old_bbw, old_bbh;

	t = model->get_cur_text();
	if (t) {
		old_bbx = bb_x(t);
		old_bby = bb_y(t);
		old_bbw = bb_w(t);
		old_bbh = bb_h(t);

		model->move(t->get_x() + dx, t->get_y() + dy);
		mod++;

		damage(4, old_bbx, old_bby, old_bbw, old_bbh);
		damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
	}
}

void PSEditWidget::rm_char() {
	PSEditText *t;
	int width;

	t = model->get_cur_text();
	if (t) {
		fl_font(FLPSED_FONT, t->get_size() * zoom_percent / 100);
		width =  bb_w(t);
	}

	model->rm_char();
	mod++;

	if (t) {
		damage(4, bb_x(t), bb_y(t), width, bb_h(t));
	}
}


int PSEditWidget::reload() {
	model->set_page(0);

	if (property_changed_cb) {
		property_changed_cb();
	}

	return GsWidget::reload();
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

	cur_size = s;
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

void PSEditWidget::set_color(const PSEditColor *c) {
	PSEditText *t;
	uchar *p;

	t = model->get_cur_text();

	p = (uchar*) &c;
	cur_text_color.set(c);

	model->set_color(&cur_text_color);

	if (t) {
		damage(4, bb_x(t), bb_y(t), bb_w(t), bb_h(t));
	}
}

void PSEditWidget::get_color(PSEditColor *c) {
	c->set(&cur_text_color);
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

int PSEditWidget::zoom(int p) {
	zoom_percent = p;
	return GsWidget::zoom(zoom_percent);
}


int PSEditWidget::bb_x(PSEditText *t) {
	return ps_to_display_x(t->get_x()) + x() - 10;
}

int PSEditWidget::bb_y(PSEditText *t) {
	fl_font(FLPSED_FONT, t->get_size() * zoom_percent / 100);
	return ps_to_display_y(t->get_y()) - fl_height() + y() - 10;
}

int PSEditWidget::bb_w(PSEditText *t) {
	int w, wt = 0;
	char *tag;
	fl_font(FLPSED_FONT, t->get_size() * zoom_percent / 100);
	w = (int) fl_width(t->get_text()) + 20;

	tag = t->get_tag();
	if (tag) {
		fl_font(FLPSED_TAG_FONT, FLPSED_TAG_FONT_SIZE * zoom_percent / 100);
		wt = (int) fl_width(tag) + 20;
	}

	return w>=wt?w:wt;
}

int PSEditWidget::bb_h(PSEditText *t) {
	int ret;
	fl_font(FLPSED_FONT, t->get_size() * zoom_percent / 100);
	ret = fl_height() + 30;

	if (t->get_tag()) {
		fl_font(FLPSED_TAG_FONT, FLPSED_TAG_FONT_SIZE * zoom_percent / 100);
		ret = ret + fl_height();
	}

	return ret;
}

static int round_div(int a, int b) {
	int r;

	r = a / b;
	if (a % b > b / 2) {
		r++;
	}

	return r;
}

int PSEditWidget::ps_to_display_x(int x1) {
	return  round_div(x1 * xdpi,  72);
}

int PSEditWidget::ps_to_display_y(int y1) {
	return round_div((paper_y - y1) * ydpi, 72);
}

int PSEditWidget::ps_x(int x1) {
	return round_div(x1 * 72, xdpi);
}

int PSEditWidget::ps_y(int y1) {
	return paper_y - round_div(y1 * 72, ydpi);
}
