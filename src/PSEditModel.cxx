//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "Postscript.H"
#include "PSEditText.H"
#include "PSEditModel.H"


PSEditModel::PSEditModel() {
	max_pages = 32;
	page = 0;
	text = (PSEditText**) malloc(sizeof(PSEditText*) * max_pages);
	if (!text) {
		perror("malloc");
		exit(1);
	}
	for (int i = 0; i < max_pages; i++) {
		text[i] = NULL;
	}

	cur_text = NULL;
}

void PSEditModel::set_page(int p) {
	int old_max_pages;

	if (p >= max_pages) {
		old_max_pages = max_pages;

		max_pages = p + max_pages;
		text = (PSEditText**) realloc(text, sizeof(PSEditText*) * max_pages);
		if (!text) {
			perror("realloc");
			exit(1);
		}

		for (int i = old_max_pages; i < max_pages; i++) {
			text[i] = NULL;
		}
	}

	page = p;
	cur_text = NULL;
}

void PSEditModel::clear() {
	cur_text = NULL;
	for (int i = 0; i < max_pages; i++) {
		if (text[i]) {
			delete(text[i]);
			text[i] = NULL;
		}
	}
}

void PSEditModel::new_text(int x1, int y1, const char *s, 
	int size, int p, PSEditColor *c) {
	set_page(p);

	cur_text = new PSEditText(x1, y1, s, size, c);
	if (text[p]) {
		text[p]->append(cur_text);
	} else {
		text[p] = cur_text;
	}
}

int PSEditModel::set_cur_text(int x1, int y1, int p) {
	PSEditText *t;

	if (p < 0 || p >= max_pages) {
		return 1;
	}

	if (text[p]) {
		t = text[p]->get_match(x1, y1);
		if (t) {
			cur_text = t;
			return 0;
		}
	}
	return 1;
}

int PSEditModel::next_text(int p) {
	if (p < 0 || p >= max_pages) {
		return 1;
	}

	if (cur_text) {
		cur_text = cur_text->get_next();
	}

	if (cur_text) {
		return 0;
	} else if (text[p]) {
		cur_text = text[p];
		return 0;
	} else {
		return 1;
	}
}

int PSEditModel::is_cur_text(PSEditText *t) {
	return (t && t == cur_text);
}

PSEditText *PSEditModel::get_cur_text() {
	return cur_text;
}

void PSEditModel::append_text(const char *s) {
	if (cur_text && s) {
		cur_text->append_text(s);
	}
}


void PSEditModel::move(int x1, int y1) {
	if (cur_text) {
		cur_text->move(x1, y1);
	}
}


void PSEditModel::rm_char() {
	if (cur_text) {
		cur_text->rm_char();
	}
}


void PSEditModel::set_size(int s) {
	if (cur_text) {
		cur_text->size = s;
	}
}

int PSEditModel::get_size() {
	if (cur_text) {
		return cur_text->size;
	} else {
		return -1;
	}
}

void PSEditModel::set_color(PSEditColor *c) {
	if (cur_text) {
		cur_text->text_color.set(c->r, c->g, c->b); 
	}
}

int PSEditModel::get_color(PSEditColor *c) {
	if (cur_text) {
		c->set(cur_text->text_color.r, 
			cur_text->text_color.g,
			cur_text->text_color.b);
		return 0;
	} else {
		return -1;
	}
}


int PSEditModel::get_max_pages() {
	return max_pages;
}

int PSEditModel::set_tag(const char *t) {
	if (cur_text) {
		return cur_text->set_tag(t);
	} else {
		return 1;
	}
}

char *PSEditModel::get_tag() {
	if (cur_text) {
		return cur_text->get_tag();
	} else {
		return NULL;
	}
}

int PSEditModel::replace_tag(char *tag, char *txt) {
	PSEditText *t;
	int p, ret = 0;

	for (p = 0; p < max_pages; p++) {
		t = get_text(p);
		while (t) {
			if (t->get_tag() && strcmp(t->get_tag(), tag) == 0) {
				t->set_text(txt);
				ret++;
			}
			t = t->get_next();
		}
	}

	return ret;
}

int PSEditModel::dump_tags() {
	PSEditText *t;
	int p, ret = 0;

	for (p = 0; p < max_pages; p++) {
		t = get_text(p);
		while (t) {
			if (t->get_tag()) {
				printf("%s=%s\n", t->get_tag(), t->get_text());
				ret++;
			}
			t = t->get_next();
		}
	}

	return ret;
}

PSEditText *PSEditModel::get_text(int p) {
	if (p >= max_pages) {
		return 0;
	} else {
		return text[p];
	}
}

int PSEditModel::load(FILE *fp) {
	char tmpname[256];
	char linebuf[1024];
	ssize_t ret;
	PSParser *p2 = new PSParser_2(this);
	int tmp_fd;

	strncpy(tmpname, "/tmp/PSEditorXXXXXX", sizeof(tmpname));
	tmp_fd = mkstemp(tmpname);
	if (tmp_fd < 0) {
		fprintf(stderr, "Could not create temporary file (errno %d).\n", errno);
		return -1;
	}
	unlink(tmpname);

	clear();

	while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
		if (!p2->parse(linebuf)) {
			ret = write(tmp_fd, linebuf, strlen(linebuf));
			if (ret != (int) strlen(linebuf)) {
				fprintf(stderr, "Error while writing to temporary file\n");
			}
		}
	}

	lseek(tmp_fd, 0L, SEEK_SET);

	delete(p2);

	return tmp_fd;
}



int PSEditModel::save(FILE *sfp, int tmp_fd) {
	off_t pos = lseek(tmp_fd, 0, SEEK_CUR); // save current position

	if (pos == -1) {
		perror("lseek");
		return 1;
	}

	FILE *fp = fdopen(tmp_fd, "r");

	rewind(fp);

	PSWriter *pw;

	pw = new PSLevel1Writer(this);

	pw->write(fp, sfp);

	delete(pw);

	lseek(tmp_fd, pos, SEEK_SET);           // restore current position

	return 0;
}
