//
// Copyright 2007-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef PSEditModel_H
#define PSEditModel_H

#include "PSEditText.H"

class PSEditModel {

	PSEditText **text;

	int max_pages;

	int page;

	PSEditText *cur_text;

	public:
	PSEditModel();

	~PSEditModel();

	void clear();

	void new_text(int x1, int y1,
		const char *s, int size, int p, PSEditColor *c);

	void append_text(const char *s);

	void move(int x1, int y1);

	void rm_char();

	void set_size(int s);

	int get_size();

	void set_color(PSEditColor *c);

	int get_color(PSEditColor *c);

	void set_page(int p);

	int get_page();

	char *get_tag();

	int set_tag(const char *t);

	int get_max_pages();

	int set_cur_text(int x1, int y1, int p);

	int next_text(int p);

	int is_cur_text(PSEditText *t);

	PSEditText *get_text(int p);

	PSEditText *get_cur_text();

	int replace_tag(char* tag, char* text);

	int dump_tags();

	int load(FILE *fp);  

	int save(FILE *sfp, int tmp_fd);
};

#endif
