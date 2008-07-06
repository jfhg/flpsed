//
// Copyright 2006-2008 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Hold_Browser.H>

#include "PSEditor.H"
#include "util.h"
#include "../config.h"

PSEditor *psed_p   = NULL;
Fl_Scroll *scroll = NULL;
Fl_Hold_Browser *page_sel = NULL;

int xev_handler(int ev) {
	if (psed_p)
		return psed_p->handleX11(ev);
	else
		return 0;
}

void save_cb();

int check_save(void) {
	if (!psed_p->modified())
		return 1;

	int r = fl_choice("The current file has not been saved.\n"
		"Would you like to save it now?",
		"Cancel", "Save", "Discard Changes");

	if (r == 1) {
		save_cb();
		return !psed_p->modified();
	}

	return (r == 2) ? 1 : 0;
}

static int
confirm_overwrite(const char *f) {
	struct stat sb;

	if (stat(f, &sb) == 0)
		return fl_choice("The file exists.\n", "Cancel", "Overwrite", NULL);
	else
		return 1;
}

char filename[1024] = "";

void page_sel_cb(Fl_Widget *w, void *) {
	int p = page_sel->value();
	if (p > 0)
		psed_p->load_page(page_sel->value());
}

void page_sel_fill() {
	char buf[128];
	int p = psed_p->get_pages();

	page_sel->clear();

	if (!p)
		return;

	for (int i = 1; i <= p; i++) {
		snprintf(buf, sizeof(buf), "%d", i);
		page_sel->add(buf);
	}

	page_sel->select(psed_p->get_page());
}

void open_file(char *file) {
	FILE *p;
	int status;
	char *args[32];
	pid_t pid;
	char *dot = strrchr(file, '.');

	if (dot && strcasecmp(dot, ".pdf") == 0) {
		// assume pdf and try to import it using pdftops
		args[0] = "pdftops";
		args[1] = file;
		args[2] = "-";
		args[3] = NULL;

		p = pexecvp("pdftops", args, &pid, "r");

		if (p) {
			psed_p->open_file(p);
			psed_p->load();
			fclose(p);
			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) == 127 || WEXITSTATUS(status) == 126) {
				fl_message("PDF import depends on pdftops from xpdf.\n"
					"Make sure pdftops is available on your system.\n");
			} else if (WEXITSTATUS(status) != 0) {
				fl_message("PDF import failed\n");
			}
		} else {
			perror("pexecvp");
		}
	} else {
		// assume postscript
		psed_p->open_file(file);
		psed_p->load();
	}

	page_sel_fill();
}

void open_cb() {
	char *file;

	if (!check_save())
		return;

	file = fl_file_chooser("Open File?", "*.{ps,pdf}", filename);
	if (file)
		open_file(file);
}

void export_pdf_cb() {
	char *file;
	FILE *p;
	int status;
	char *args[32];
	pid_t pid;

	file = fl_file_chooser("Open File?", "*.pdf", filename);
	if(file != NULL && confirm_overwrite(file)) {
		args[0] = "ps2pdf";
		args[1] = "-";
		args[2] = file;
		args[3] = NULL;

		signal(SIGPIPE, SIG_IGN);

		p = pexecvp("ps2pdf", args, &pid, "w");

		if (p) {
			psed_p->save(p);

			fclose(p);
			waitpid(pid, &status, 0); 
			if (WEXITSTATUS(status) == 127 || WEXITSTATUS(status) == 126) {
				fl_message("PDF export depends on ps2pdf from ghostscript.\n"
					"Make sure ps2pdf is available on your system.\n");
			} else if (WEXITSTATUS(status) != 0) {
				fl_message("PDF export failed\n");
			}
		} else {
			perror("pexecvp");
		}

		signal(SIGPIPE, SIG_DFL);
	}
}

void import_cb() {
	char *file = fl_file_chooser("Import Overlay from File?", "*.ps", filename);
	if (file)
		psed_p->import(file);
}

void first_cb() {
	psed_p->load();
	page_sel->select(psed_p->get_page());
}

void next_cb() {
	psed_p->next();
	page_sel->select(psed_p->get_page());
}

void quit_cb() {
	if (!check_save())
		return;
	delete psed_p;
	exit(0);
}

void save_cb() {
	char *file = fl_file_chooser("Open File?", "*.ps", filename);
	if (file && confirm_overwrite(file))
		psed_p->save(file);
}

void print_cb() {
	char *pref_cmd;
	int r;
	const char *print_cmd;
	Fl_Preferences prefs(Fl_Preferences::USER,
		"Johannes.HofmannATgmx.de", "flpsed");

	prefs.get("printCommand", pref_cmd, "lpr");
	print_cmd = fl_input("Print Command, e.g. \"lpr\" or \"psnup -2 | lpr\"", pref_cmd);
	if (!print_cmd || print_cmd[0] == '\0')
		return;

	if (strcmp(print_cmd, pref_cmd))
		prefs.set("printCommand", print_cmd);

	FILE *fp = popen(print_cmd, "w");

	if (fp) {
		signal(SIGPIPE, SIG_IGN);
		r = psed_p->save(fp);
		pclose(fp);
		signal(SIGPIPE, SIG_DFL);
	} 

	if (!fp || r)
		fprintf(stderr, "Print Command %s failed\n", print_cmd);
}

void about_cb() {
	fl_message("flpsed -- a PostScript annotator\n"
		"Version %s\n\n"
		"(c) Johannes Hofmann 2004-2008\n\n"
		"PostScript is a registered trademark of Adobe Systems", VERSION);
}

Fl_Choice *size_c;
Fl_Button *color_b;

static struct {
	char *label;
	int size;
} text_sizes[] = {
	{"8", 8},
	{"10", 10},
	{"12", 12},
	{"14", 14},
	{"18", 18},
	{"24", 24},
	{NULL, 0}
};

void property_changed_cb() {
	PSEditColor c;
	int size;

	psed_p->get_color(&c);
	color_b->color(fl_rgb_color(c.get_r(), c.get_g(), c.get_b()));
	color_b->redraw();

	size = psed_p->get_size();
	for (int i=0; text_sizes[i].label != NULL; i++) {
		if (size == text_sizes[i].size) {
			size_c->value(i);
			size_c->redraw();
		}
	}
}

void size_cb(Fl_Widget *w, void *) {
	Fl_Menu_* mw = (Fl_Menu_*)w;
	const Fl_Menu_Item* m = mw->mvalue();
	if (m)
		psed_p->set_size(atoi(m->label()));
}

void color_cb(Fl_Widget *w, void *v) {
	uchar r, g, b;
	PSEditColor pc;

	psed_p->get_color(&pc);

	r = pc.get_r();
	g = pc.get_g();
	b = pc.get_b();

	if (!fl_color_chooser("Text Color", r, g, b))
		return;

	Fl_Button* button = (Fl_Button*)v;
	pc.set(r, g, b);
	psed_p->set_color(&pc);
	button->color(fl_rgb_color(r, g, b));
	button->parent()->redraw();
}

void zoom_cb(Fl_Widget *w, void *) {
	Fl_Menu_* mw = (Fl_Menu_*)w;
	const Fl_Menu_Item* m = mw->mvalue();
	if (m) {
		if (psed_p) {
			psed_p->zoom(atoi(m->label()));
			if (scroll) {
				scroll->position(0,0);
				scroll->redraw();
			}
		}
	}
}

void show_tags_cb(Fl_Widget* w, void*d) {
	psed_p->set_show_tags(d==NULL?0:1);
}

void edit_tag_cb() {
	char *tag = psed_p->get_tag();
	const char *new_tag;
	new_tag = fl_input("Tag Name", tag?tag:"");
	if (new_tag) {
		if (strcmp(new_tag, "") != 0) {
			psed_p->set_tag(new_tag);
		} else {
			psed_p->set_tag(NULL);
		}
	}
}

Fl_Menu_Item menuitems[] = {
	{ "&File",              0, 0, 0, FL_SUBMENU },
	{ "&Open File...",    FL_CTRL + 'o', (Fl_Callback *)open_cb },
	{ "&Save File as...", FL_CTRL + 's', (Fl_Callback *)save_cb },
	{ "I&mport Tags from File...",    FL_CTRL + 'm', (Fl_Callback *)import_cb },
	{ "&Import PDF...", FL_CTRL + 'i', (Fl_Callback *)open_cb },
	{ "E&xport PDF...", FL_CTRL + 'x', (Fl_Callback *)export_pdf_cb },
	{ "&Print...", FL_CTRL + 'p', (Fl_Callback *)print_cb, 0, FL_MENU_DIVIDER },
	{ "&Quit", FL_CTRL + 'q', (Fl_Callback *)quit_cb, 0 },
	{ 0 },

	{ "&Page", 0, 0, 0, FL_SUBMENU },
	{ "&First",        FL_CTRL + 'f', (Fl_Callback *)first_cb },
	{ "&Next",       FL_CTRL + 'n', (Fl_Callback *)next_cb },
	{ 0 },

	{ "&Zoom", 0, 0, 0, FL_SUBMENU },
	{ "50 %",  0, (Fl_Callback *)zoom_cb },
	{ "75 %",  0, (Fl_Callback *)zoom_cb },
	{ "100 %",  0, (Fl_Callback *)zoom_cb },
	{ "150 %",  0, (Fl_Callback *)zoom_cb },
	{ "200 %",  0, (Fl_Callback *)zoom_cb },
	{ "250 %",  0, (Fl_Callback *)zoom_cb },
	{ 0 },

	{ "&Tags", 0, 0, 0, FL_SUBMENU },
	{ "Show &Tags", FL_CTRL + 't', (Fl_Callback *)show_tags_cb, (void *)1, FL_MENU_RADIO|FL_MENU_VALUE},
	{ "&Hide Tags", FL_CTRL + 'h', (Fl_Callback *)show_tags_cb, (void *)0, FL_MENU_RADIO},
	{ "&Edit Tag",  FL_CTRL + 'e', (Fl_Callback *)edit_tag_cb },
	{ 0 },

	{ "&Help", 0, 0, 0, FL_SUBMENU },
	{ "About",  0, (Fl_Callback *)about_cb },
	{ 0 },

	{ 0 }
};



Fl_Menu_Item size_menu[] = {
	{ text_sizes[0].label,  0, (Fl_Callback *)size_cb },
	{ text_sizes[1].label,  0, (Fl_Callback *)size_cb },
	{ text_sizes[2].label,  0, (Fl_Callback *)size_cb },    
	{ text_sizes[3].label,  0, (Fl_Callback *)size_cb },
	{ text_sizes[4].label,  0, (Fl_Callback *)size_cb },
	{ text_sizes[5].label,  0, (Fl_Callback *)size_cb },
	{ 0 }
};

void usage() {
	fprintf(stderr,
		"usage: flpsed [-hbdz] [-t <tag>=<value>] [<infile>] [<outfile>]\n"
		"   -h                 print this message\n"
		"   -b                 batch mode (no gui)\n"
		"   -d                 dump tags and values from a document\n"
		"                      to stdout (this implies -b)\n"
		"   -z <zoom>          set the zoom percentage (e.g. 200)\n"
		"   -t <tag>=<value>   set text to <value> where tag is <tag>\n"
		"   <infile>           optional input file; in batch mode if no\n"
		"                      input file is given, input is read from stdin\n"
		"   <outfile>          optional output file for batch mode; if no\n"
		"                      output file is given, output is written to stdout\n\n"
		"additionally flpsed supports the following FLTK standard options:\n"
		" -geometry WxH+X+Y\n"
		" -iconic\n"
		" -scheme string\n"
	);
}

#define TV_LEN 256

int main(int argc, char** argv) {
	char *sep, *tmp, **my_argv;
	int c, err, bflag = 0, dflag = 0;
	int i, n;
	Fl_Window *win;
	Fl_Menu_Bar *m;
	struct {char *tag; char *value;} tv[TV_LEN];
	int tv_idx = 0, my_argc;
	FILE *out_fp = NULL;
	int zoom_val = 0;

	err = 0;
	while ((c = getopt(argc, argv, "hdbt:z:g:d:f:i:s:")) != -1) {
		switch (c) {  
			case 'h':
				usage();
				exit(0);
				break;
			case 'b':
				bflag = 1;
				break;
			case 'd':
				dflag = 1;
				break;
			case 't':
				tmp = strdup(optarg);
				sep = strchr(tmp, '=');
				if (!sep) {
					fprintf(stderr, "Cannot parse %s\n", optarg);
					free(tmp);
					continue;
				}
				*sep = '\0';

				if (tv_idx >= TV_LEN) {
					fprintf(stderr, "More than %d tag/value pairs; ignoring %s->%s\n", 
						TV_LEN, tmp, sep+1);
				} else {
					tv[tv_idx].tag = strdup(tmp);
					tv[tv_idx].value = strdup(sep+1);
					tv_idx++;
				}
				free(tmp);
				break;
			case 'z':
				zoom_val = atoi(optarg);
				break;
			default:
				i = optind -1;
				n = Fl::arg(argc, argv, i);
				if (n == 0) {
					err++;
				} else {
					optind = i;
				}
		}
	}

	if (err) {
		usage();
		exit(1);
	}

	my_argc = argc - optind;
	my_argv = argv + optind;

	if (bflag || dflag) {
		//
		// Batch Mode 
		//

		PSEditModel *m = new PSEditModel();
		int tmp_fd;
		FILE *in_fp;

		if (my_argc >= 1) {
			in_fp = fopen(my_argv[0], "r");
			if (!in_fp) {
				perror("fopen");
				exit(1);
			}
		} else {
			in_fp = stdin;
		}

		tmp_fd = m->load(in_fp);

		if (tmp_fd == -1) {
			fprintf(stderr, "Could not load %s\n", argv[argc - 2]);
			exit(1);
		}

		if (in_fp != stdin)
			fclose(in_fp);

		for (int i = 0; i < tv_idx; i++) {
			m->replace_tag(tv[i].tag, tv[i].value);
			free(tv[i].tag);
			free(tv[i].value);
		}

		if (bflag) {
			if (my_argc >= 2) {
				out_fp = fopen(my_argv[1], "w");
				if (!in_fp) {
					perror("fopen");
					exit(1);
				}
			} else {
				out_fp = stdout;
			}

			m->save(out_fp, tmp_fd);

			if (out_fp != stdout)
				fclose(out_fp);

		} else {
			m->dump_tags();
		}

	} else {
		// 
		// Interactive Mode
		//

		win = new Fl_Window(600,700);
		m = new Fl_Menu_Bar(0, 0, 600, 30);
		m->menu(menuitems);

		Fl_Box props_box(FL_UP_BOX, 0, 30, 600, 25, "");
		Fl_Group props(0, 30, 600, 25);
		props.resizable(NULL);
		size_c = new Fl_Choice(30, 32, 50, 21, NULL);
		size_c->menu(size_menu); 
		size_c->callback(size_cb);
		size_c->tooltip("Text Size");

		color_b = new Fl_Button(100, 32, 21, 21);
		color_b->color(FL_BLACK);
		color_b->callback(color_cb, color_b);
		color_b->shortcut(FL_ALT + 'c');
		color_b->tooltip("Text Color");
		props.end();
		page_sel = new Fl_Hold_Browser(0, 55, 40, win->h()-55);
		page_sel->callback(page_sel_cb);
		page_sel->end();
		scroll = new Fl_Scroll(40, 55, win->w()-40, win->h()-55);
		psed_p = new PSEditor(40, 55, 700, 900);
		psed_p->property_changed_callback(property_changed_cb);
		scroll->end();

		fl_open_display();
		Fl::add_handler(xev_handler);

		win->resizable(scroll);

		win->end();
		win->callback((Fl_Callback *)quit_cb);
		win->show(1, argv); 

		if (zoom_val)
			psed_p->zoom(zoom_val);

		if (my_argc >= 1)
			open_file(my_argv[0]);

		for (int i = 0; i < tv_idx; i++) {
			psed_p->replace_tag(tv[i].tag, tv[i].value);
			free(tv[i].tag);
			free(tv[i].value);
		}

		return Fl::run();
	}
}
