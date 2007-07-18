//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef GsWidget_H
#define GsWidget_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>

#include "PostscriptDSC.H"

class GsWidget : public Fl_Widget {
	private:
		PostscriptDSC *dsc;
		int gs_win;
		Atom atoms[5];
		pid_t gs_pid;
		int display_x, display_y;
		int in_fd, reload_needed;
		int zoom_percent;
		int initial_width, initial_height;
		int feeding;
	
		void resize();
		void clear();

	protected:
		int page;
		Fl_Offscreen offscreen;
		int xdpi, ydpi;
		int paper_x, paper_y;

		void draw();

	public:
		GsWidget(int X,int Y,int W, int H);

		~GsWidget();

		int open_file(char *f);

		int open_file(int fd);

		int load();

		int load_page(int p);

		int reload();

		int next();

		int prev();

		int handleX11(int ev);

		int zoom(int p);

		int get_page();

		int get_pages();

	private:
		void setProps();

		void exec_gs();

		void kill_gs();

		int fd_copy(int to, int from, size_t len);

		bool gs_active();
};
#endif
