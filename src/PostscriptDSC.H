//
// Copyright 2007-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef POSTSCRIPTDSC_H
#define POSTSCRIPTDSC_H

class PostscriptDSC {
	private:
		int bb_x, bb_y, bb_w, bb_h;
		size_t setup_len;
		int pages;

		size_t *page_off;
		size_t *page_len;

	public:
		PostscriptDSC();
		~PostscriptDSC();

		int parse(int fd);
		int get_pages();
		void get_bounding_box(int *x, int *y, int *w, int *h);
		size_t get_setup_len();
		size_t get_page_off(int p);
		size_t get_page_len(int p);
		void print();
};

#endif
