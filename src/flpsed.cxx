// 
// "$Id: flpsed.cxx,v 1.25 2005/01/27 21:07:09 hofmann Exp $"
//
// flpsed program.
//
// Copyright 2004 by Johannes Hofmann
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//

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
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

#include "PSEditor.H"

PSEditor *gsw_p;

int xev_handler(int ev) {
  if (gsw_p) {
    return gsw_p->handleX11(ev);
  } else {
    return 0;
  }
}

void save_cb();

int check_save(void) {
  if (!gsw_p->modified()) return 1;

  int r = fl_choice("The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Discard");

  if (r == 1) {
    save_cb(); // Save the file...
    return !gsw_p->modified();
  }

  return (r == 2) ? 1 : 0;
}


char filename[256] = "";

void open_cb() {
  if (!check_save()) return;
  char *file = fl_file_chooser("Open File?", "*.ps", filename);
  if(file != NULL) {
    gsw_p->load(file);
  }  
}

void import_cb() {
  char *file = fl_file_chooser("Import Overlay from File?", "*.ps", filename);
  if(file != NULL) {
    gsw_p->import(file);
  }  
}

void first_cb() {
  gsw_p->reload();
}

void next_cb() {
  gsw_p->next();
}

void quit_cb() {
  if (!check_save()) return;
  delete gsw_p;
  exit(0);
}

void save_cb() {
  char *file = fl_file_chooser("Open File?", "*.ps", filename);
  if (file != NULL) {
    gsw_p->save(file);
  }
}

void print_cb() {
  char tmpname[256];
  char buf[256];
  int tmp_fd;

  int r = fl_ask("Print file?");
  if (r != 1) {
    return;
  }

  strncpy(tmpname, "/tmp/PSEditWidgetXXXXXX", 256);
  tmp_fd = mkstemp(tmpname);

  if (tmp_fd >= 0) {
    close(tmp_fd);
    if (gsw_p->save(tmpname) != 0) {
      fprintf(stderr, "Failed to print file\n");
    } else {
      snprintf(buf, 256, "lpr %s", tmpname);
      system(buf);
    }
    unlink(tmpname);
  } 
}

void about_cb() {
  fl_message("flpsed -- a pseudo PostScript editor\n"
	     "(c) Johannes Hofmann 2004\n\n"
	     "PostScript is a registered trademark of Adobe Systems");
}

void size_cb(Fl_Widget *w, void *) {
  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* m = mw->mvalue();
  if (m) {
    gsw_p->set_size(atoi(m->label()));
  }
}

void zoom_cb(Fl_Widget *w, void *) {
  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* m = mw->mvalue();
  if (m) {
    gsw_p->zoom(atoi(m->label()));
  }
}

void show_tags_cb(Fl_Widget* w, void*d) {
  gsw_p->set_show_tags((int) d);
}

void edit_tag_cb() {
  char *tag = gsw_p->get_tag();
  const char *new_tag;
  new_tag = fl_input("Tag Name", tag?tag:"");
  if (new_tag) {
    if (strcmp(new_tag, "") != 0) {
      gsw_p->set_tag(new_tag);
    } else {
      gsw_p->set_tag(NULL);
    }
  }
}

Fl_Menu_Item menuitems[] = {
  { "&File",              0, 0, 0, FL_SUBMENU },
    { "&Open File...",    FL_CTRL + 'o', (Fl_Callback *)open_cb },
    { "&Save File as...", FL_CTRL + 's', (Fl_Callback *)save_cb },
    { "I&mport Tags from File...",    FL_CTRL + 'm', (Fl_Callback *)import_cb },
    { "&Print...", FL_CTRL + 'p', (Fl_Callback *)print_cb, 0, FL_MENU_DIVIDER },
    { "&Quit", FL_CTRL + 'q', (Fl_Callback *)quit_cb, 0 },
    { 0 },

  { "&Page", 0, 0, 0, FL_SUBMENU },
    { "F&irst",        FL_CTRL + 'i', (Fl_Callback *)first_cb },
    { "&Next",       FL_CTRL + 'n', (Fl_Callback *)next_cb },
    { 0 },

  { "&Zoom (%)", 0, 0, 0, FL_SUBMENU },
    { "50",  0, (Fl_Callback *)zoom_cb },
    { "75",  0, (Fl_Callback *)zoom_cb },
    { "100",  0, (Fl_Callback *)zoom_cb },
    { "150",  0, (Fl_Callback *)zoom_cb },
    { "200",  0, (Fl_Callback *)zoom_cb },    
    { 0 },

  { "&Size", 0, 0, 0, FL_SUBMENU },
    { "8",  0, (Fl_Callback *)size_cb },
    { "10",  0, (Fl_Callback *)size_cb },
    { "12",  0, (Fl_Callback *)size_cb },    
    { "14",  0, (Fl_Callback *)size_cb },
    { "18",  0, (Fl_Callback *)size_cb },
    { "24",  0, (Fl_Callback *)size_cb },
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
  

void usage() {
  fprintf(stderr,
	  "usage: flpsed [-hbd] [-t <tag>=<value>] [<infile>] [<outfile>]\n"
	  "   -h                 print this message\n"
	  "   -b                 batch mode (no gui)\n"
	  "   -d                 dump tags and values from a document\n"
	  "                      to stdout (this implies -b)\n"
	  "   -t <tag>=<value>   set text to <value> where tag is <tag>\n"
	  "   <infile>           optional input file; in batch mode if no\n"
	  "                      input file is given, input is read from stdin\n"
	  "   <outfile>          optional output file for batch mode; if no\n"
	  "                      output file is given, output is written to stdout\n");
}

#define TV_LEN 256

int main(int argc, char** argv) {
  char c, *sep, *tmp, **my_argv;
  int err, bflag = 0, dflag = 0;
  Fl_Window *win;
  Fl_Menu_Bar *m;
  Fl_Int_Input *x_in, *y_in;
  Fl_Scroll *scroll;
  struct {char *tag; char *value;} tv[TV_LEN];
  int tv_idx = 0, my_argc;
  FILE *in_fp = NULL, *out_fp = NULL;
  
  err = 0;
  while ((c = getopt(argc, argv, "hdbt:")) != EOF) {
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
      if (!tmp) {
	perror("strdup");
	exit(1);
      }
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
    default:
      err++;
    }
  }
  
  if (err) {
    usage();
    exit(1);
  }

  my_argc = argc - optind;
  my_argv = argv + optind;

  if (my_argc >= 1) {
    in_fp = fopen(my_argv[0], "r");
    if (!in_fp) {
      perror("fopen");
      exit(1);
    }
  }
  
  if (bflag || dflag) {
    //
    // Batch Mode 
    //
    
    PSEditModel *m = new PSEditModel();
    int tmp_fd;

    if (!in_fp) {
      in_fp = stdin;
    }
 

    tmp_fd= m->load(in_fp);

    if (tmp_fd == -1) {
      fprintf(stderr, "Could not load %s\n", argv[argc - 2]);
      exit(1);
    }
    
    if (in_fp != stdin) {
      fclose(in_fp);
    }
    
    for(int i=0; i<tv_idx; i++) {
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
      
      if (out_fp != stdout) {
	fclose(out_fp);
      }
    } else { // dump tags
      m->dump_tags();
    }

  } else {
    // 
    // Interactive Mode
    //

    win = new Fl_Window(600,700);
    m = new Fl_Menu_Bar(0, 0, 600, 30);
    m->menu(menuitems);
    scroll = new Fl_Scroll(0, 30, win->w(), win->h()-30);
    gsw_p = new PSEditor(0, 0, 700, 900);
    scroll->end();
    
    fl_open_display();
    Fl::add_handler(xev_handler);
    
    win->resizable(scroll);
    
    win->end();
    win->callback((Fl_Callback *)quit_cb);
    win->show(1, argv); 
    
    if (in_fp) {
      gsw_p->load(in_fp);
      fclose(in_fp);
    }

    for(int i=0; i<tv_idx; i++) {
      gsw_p->replace_tag(tv[i].tag, tv[i].value);
      free(tv[i].tag);
      free(tv[i].value);
    }

    return Fl::run();
  }
}
