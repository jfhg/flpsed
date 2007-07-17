//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <FL/fl_draw.H>

#include "GsWidget.H"
#include "PostscriptDSC.H"

#define MIN(A,B) ((A)<(B)?(A):(B))

void GsWidget::draw() {
	if (!offscreen) {
		offscreen = fl_create_offscreen(w(), h());
		fl_begin_offscreen(offscreen);
		fl_color(FL_WHITE);
		fl_rectf(0, 0, w(), h());
		fl_end_offscreen();

		if (reload_needed) {
			reload();
			reload_needed = 0;
		}
	}
	fl_push_clip(x(), y(), w(), h());
	fl_copy_offscreen(x(), y(), w(), h(), offscreen, 0, 0); 
	fl_pop_clip();
}

void GsWidget::setProps() {
	char data[512];

	if (!offscreen) {
		offscreen = fl_create_offscreen(w(), h());
	}

	atoms[0] = XInternAtom(fl_display,"GHOSTVIEW" , false);
	atoms[1] = XInternAtom(fl_display,"GHOSTVIEW_COLORS" , false);
	atoms[2] = XInternAtom(fl_display,"NEXT" , false);
	atoms[3] = XInternAtom(fl_display,"PAGE" , false);
	atoms[4] = XInternAtom(fl_display,"DONE" , false);

	snprintf(data, sizeof(data), "%d %d %d %d %d %d %d.0 %d.0",
		0, 0, 0, 0, paper_x, paper_y, xdpi, ydpi);

	int xid = fl_xid(window());

	XChangeProperty(fl_display, xid, atoms[0],
		XA_STRING, 8, PropModeReplace,
		(unsigned char*) data, strlen(data));

	snprintf(data, sizeof(data), "%s %d %d", "Color", 
		(int) BlackPixel(fl_display, DefaultScreen(fl_display)),
		(int) WhitePixel(fl_display, DefaultScreen(fl_display)));

	XChangeProperty(fl_display, xid, atoms[1],
		XA_STRING, 8, PropModeReplace,
		(unsigned char*) data, strlen(data));

	XSync(fl_display, False);
}

void GsWidget::kill_gs() {
	int status;

	if (gs_pid > 0) {
		kill(gs_pid, SIGTERM);
		waitpid(gs_pid, &status, 0);
		gs_pid = 0;
	}
}

bool GsWidget::gs_active() {
	return gs_pid > 0 && gs_win;
}

GsWidget::GsWidget(int X,int Y,int W, int H) : Fl_Widget(X, Y, W, H) { 
	offscreen = 0;
	gs_pid = 0;
	page = 0;
	zoom_percent = 100;
	xdpi = 75 * zoom_percent / 100;
	ydpi = 75 * zoom_percent / 100;
	paper_x = 594; // DIN A4
	paper_y = 841; //
	initial_width = W;
	initial_height = H;
	in_fd = -1;
	reload_needed = 0;
	dsc = NULL;
	feeding = 0;
}

GsWidget::~GsWidget() {
	kill_gs();
	if (offscreen) {
		fl_delete_offscreen(offscreen);
	}
}

int GsWidget::open_file(char *f) {
	int fd = open(f, O_RDONLY); 
	if (fd == -1) {
		perror("open");
		return 1;
	}
	return open_file(fd);
}

int GsWidget::open_file(int fd) {
	if (in_fd >= 0 && fd != in_fd) {
		close (in_fd);
	}
	in_fd = fd;

	if (dsc) {
		delete(dsc);
	}
	dsc = new PostscriptDSC();

	if (dsc->parse(in_fd) == 0) {
		int bb_x, bb_y, bb_w, bb_h;

		dsc->get_bounding_box(&bb_x, &bb_y, &bb_w, &bb_h);
		paper_x = bb_w;
		paper_y = bb_h;
	} else {
		delete(dsc);
		dsc = NULL;
		paper_x = 594; // DIN A4
		paper_y = 841; //
	}

	page = 0;

	return 0;
}


int GsWidget::load() {
	pid_t pid;

	if (dsc) {
		return load_page(1);
	}

	if (in_fd < 0) {
		return 1;
	}

	lseek(in_fd, 0L, SEEK_SET);

	fl_cursor(FL_CURSOR_WAIT);
	kill_gs();

	if (!offscreen) {
		reload_needed = 1;
		return 0;
	}

	setProps();

	pid = fork();
	if (pid == (pid_t) 0) {
		dup2(in_fd, STDIN_FILENO);
		exec_gs();
	} else {
		gs_pid = pid;
		page = 0;
	}

	return 0;
}

int
GsWidget::load_page(int p) {
	pid_t pid;
	int pdes[2];

	if (feeding || in_fd < 0) {
		return 1;
	}

	if (p < 1 || p > dsc->get_pages()) {
		fprintf(stderr, "Page %d not found in document\n", p);
		return 1;
	}

	fl_cursor(FL_CURSOR_WAIT);
	kill_gs();

	page = p;

	if (!offscreen) {
		reload_needed = 1;
		return 0;
	}

	if (pipe(pdes) < 0) {
		perror("pipe");
		return 1;
	}

	feeding = 1;

	lseek(in_fd, 0L, SEEK_SET);
	setProps();

	pid = fork();
	if (pid == (pid_t) 0) {
		close(in_fd);
		close(pdes[1]);
		dup2(pdes[0], STDIN_FILENO);
		exec_gs();
	} else {
		size_t len;

		gs_pid = pid;

		close(pdes[0]);

		lseek(in_fd, 0L, SEEK_SET);
		len = dsc->get_setup_len();
		if (fd_copy(pdes[1], in_fd, len) != 0) {
			close(pdes[1]);
			feeding = 0;
			return 1;
		}

		lseek(in_fd, dsc->get_page_off(p), SEEK_SET);
		len = dsc->get_page_len(p);
		if (fd_copy(pdes[1], in_fd, len) != 0) {
			close(pdes[1]);
			feeding = 0;
			return 1;
		}

		close(pdes[1]);
	}  

	feeding = 0;
	return 0;
}

int GsWidget::fd_copy(int to, int from, size_t len) {
	size_t n, r;
	char buf[1024];
	int ret = 0;

	signal(SIGPIPE, SIG_IGN); // don't die if gs has a problem

	n = 0;
	while(len > 0) {

		Fl::check(); // let fltk do its stuff 

		r = read(from, buf, MIN(sizeof(buf), len));

		if (r < 0) {
			perror("read");
			ret = 1;
			break;
		}

		write(to, buf, r);
		len -= r;
	}

	signal(SIGPIPE, SIG_DFL);

	return ret;
}

void GsWidget::exec_gs() {
	char *argv[16];
	char gvenv[256];
	int d_null = open("/dev/null", O_WRONLY);

	dup2(d_null, STDOUT_FILENO);

	snprintf(gvenv, sizeof(gvenv), "GHOSTVIEW=%d %d", 
		(int) fl_xid(window()), (int) offscreen);

	putenv(gvenv);
	argv[0] = "gs";
	argv[1] = "-dSAFER";
	argv[2] = "-dQUIET";
	argv[3] = "-sDEVICE=x11alpha";
	argv[4] = "-dNOPLATFONTS";
	argv[5] = "-dNOPAUSE";
	argv[6] = "-";
	argv[7] = NULL;
	execvp(argv[0], argv);
	perror("exec");
	fprintf(stderr, "Please install ghostscript and make sure 'gs' "
		"is in the PATH.\n");
	exit(1);
}

int GsWidget::reload() {
	if (in_fd >= 0) {
		if (dsc) {
			load_page(page);
		} else {
			load();
		}
		return 0;
	} else {
		return 1;
	}
}

int GsWidget::next() {
	if (dsc) {
		load_page(page + 1);
	} else {
		if (!gs_active()) {
			return 1;
		} else {
			fl_cursor(FL_CURSOR_WAIT);

			XEvent e;
			e.xclient.type = ClientMessage;
			e.xclient.display = fl_display;
			e.xclient.window = gs_win;
			e.xclient.message_type = atoms[2];
			e.xclient.format = 32;

			XSendEvent(fl_display, gs_win, false, 0, &e);
			XFlush(fl_display);
		}
	}

	return 0;
}

int GsWidget::prev() {
	if (dsc) {
		load_page(page - 1);
		return 0;
	} else {
		return 1;
	}
}

int GsWidget::handleX11(int ev) {
	if (fl_xevent->type == ClientMessage) {
		gs_win = fl_xevent->xclient.data.l[0];

		if(fl_xevent->xclient.message_type == atoms[3]) { // PAGE received
			damage(FL_DAMAGE_ALL);
			fl_cursor(FL_CURSOR_DEFAULT);
			if (dsc) {
				kill_gs();
			} else {
				page++;
			}
		} else if(fl_xevent->xclient.message_type == atoms[4] ) {
			if (!dsc) {
				reload();                            // go back to page 1 
			}
		}
		return 1;
	}
	return 0;
}

int GsWidget::get_page() {
	return page;
}

int GsWidget::zoom(int p) {
	zoom_percent = p;

	kill_gs();

	if (offscreen) {
		// Clear widget with current size
		fl_begin_offscreen(offscreen);
		fl_color(FL_WHITE);
		fl_rectf(0, 0, w(), h());
		fl_end_offscreen();
		redraw();

		fl_delete_offscreen(offscreen);
		offscreen = 0;
	}

	w(initial_width * zoom_percent / 100);
	h(initial_height * zoom_percent / 100);

	xdpi = 75 * zoom_percent / 100;
	ydpi = 75 * zoom_percent / 100;
	reload();

	return 0;
}

int
GsWidget::get_pages() {
	if (dsc) {
		return dsc->get_pages();
	} else {
		return 0;
	}
}
