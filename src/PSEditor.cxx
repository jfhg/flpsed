//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#include <errno.h>
#include <FL/fl_ask.H>
#include "PSEditor.H"
#include "Postscript.H"

PSEditor::PSEditor(int X,int Y,int W, int H) : PSEditWidget(X, Y, W, H) {
	loaded = 0;
	mod = 0;
	ps_level = 1;
	tmp_fd = -1;
}


int PSEditor::handle(int event) {
	int mark_x, mark_y;

	switch(event) {
		case FL_PUSH:    
			if (Fl::event_button() == 1) {
				if (!file_loaded()) {
					return 0;
				}

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

			break;
		case FL_DRAG:
			move(Fl::event_x()-x(), Fl::event_y()-y());
			return 1;
			break;
		case FL_KEYBOARD:
			{
				int del;
				int key = Fl::event_key();
				if (key == FL_BackSpace) {
					rm_char();  
				} else if (key == FL_Left) {
					rel_move(-1, 0);
				} else if (key == FL_Right) {
					rel_move(1, 0);
				} else if (key == FL_Up) {
					rel_move(0, 1);
				} else if (key == FL_Down) {
					rel_move(0, -1);
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

	return PSEditWidget::handle(event);
}


int PSEditor::open_file(FILE *fp) {
	if (tmp_fd >= 0) {
		close(tmp_fd);
	}

	tmp_fd = model->load(fp);

	if (tmp_fd < 0) {
		return 1;
	} else {
		mod = 0;
		loaded = 1;
		return GsWidget::open_file(tmp_fd);
	}
}

int PSEditor::open_file(const char *f) {
	FILE *fp;
	int ret;

	fp = fopen(f, "r");
	if (!fp) {
		perror("fopen");
		return 1;
	}

	ret = open_file(fp);
	fclose(fp);

	return ret;
}

int PSEditor::save(FILE *fp) {
	int ret;

	if (!file_loaded()) {
		return 1;
	}

	ret = model->save(fp, tmp_fd);

	if (ret == 0) {
		mod = 0;
	}

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

	ret = save(fp);
	fclose(fp);

	return ret;
}

int PSEditor::import(char *f) {
	FILE *fp;
	char linebuf[1024];
	PSParser *p2;

	if (!file_loaded()) {
		return 1;
	}

	fp = fopen(f, "r");
	if (!fp) {
		fprintf(stderr, "Could not open file %s.\n", f);
		return 1;
	}

	p2 = new PSParser_2(model);
	while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
		p2->parse(linebuf);
	}

	delete(p2);

	mod = 1;
	redraw();
	return 0;
}
