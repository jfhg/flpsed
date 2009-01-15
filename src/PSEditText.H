//
// Copyright 2007-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef PSEditText_H
#define PSEditText_H

#include <math.h>

class PSEditColor {
	public:
		double r, g, b;

		PSEditColor() {
			r = 0.0;
			g = 0.0;
			b = 0.0;
		}

		PSEditColor(double r1, double g1, double b1) {
			set(r1, g1, b1);
		}

		void set(unsigned char r1, unsigned char g1, unsigned char b1) {
			r = r1 / 256.0;
			g = g1 / 256.0;
			b = b1 / 256.0;
		}

		void set(double r1, double g1, double b1) {
			r = r1;
			g = g1;
			b = b1;
		}

		void set(const PSEditColor *c) {
			r = c->r;
			g = c->g;
			b = c->b;
		}

		unsigned char get_r() {
			return (unsigned char) rint(r * 256.0);
		}

		unsigned char get_g() {
			return (unsigned char) rint(g * 256.0);
		}

		unsigned char get_b() {
			return (unsigned char) rint(b * 256.0);
		}


};

class PSEditText;

class PSEditText {
	int x, y;

	char *s;

	char *tag;

	PSEditText *next;

	public:

	PSEditColor text_color;

	int size;

	PSEditText(int x1, int y1, const char *s1, int size1, PSEditColor *c);

	~PSEditText();

	void append_text(const char*s1);

	void rm_char();

	void move(int x1, int y1);

	void append(PSEditText *g);

	PSEditText *get_match(int x1, int y1);

	int set_text(const char *t);

	char *get_text();

	int is_empty();

	char *get_tag();

	int set_tag(const char *t);

	int get_size();

	PSEditText *get_next();

	int get_x();

	int get_y();
};

#endif
