//
// Copyright 2007-2011 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef PSEDITOR_H
#define PSEDITOR_H

#include "PSEditWidget.H"

class PSEditor : public PSEditWidget {
	private:
		int tmp_fd;
		int ps_level;

		int handle(int event);

	public: 
		PSEditor(int X,int Y,int W, int H);  

		int open_file(FILE *fp); 
		int open_file(const char *f); 
		int save(FILE *fp);
		int save(const char* savefile);
		int import(char *f); 
};

#endif
