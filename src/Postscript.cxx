// 
// "$Id: Postscript.cxx,v 1.5 2004/07/09 22:51:39 hofmann Exp $"
//
// Postscript handling routines.
//
// Copyright 2004 by Johannes Hofmann
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//

#include "Postscript.H"

#define PS_POS_FORMAT       "newpath %d %d moveto %% PSEditWidget\n"
#define PS_TEXT_FORMAT      "(%s) show %% PSEditWidget\n"
#define PS_SIZE_FORMAT "/HelveticaNeue-Roman findfont %d scalefont setfont %% PSEditWidget\n"
#define PS_GLYPH_FORMAT     "/%s glyphshow %% PSEditWidget\n"

//
// PSEditWidget internal format as PostScript comments
//

#define PSEDIT_BEGIN "%% PSEditWidget Begin\n"
#define PSEDIT_END   "%% PSEditWidget End\n"

#define PSEDIT_PAGE_FORMAT  "%% PSEditWidget: PAGE %d\n"
#define PSEDIT_POS_FORMAT   "%% PSEditWidget: POS %d %d\n"
#define PSEDIT_TEXT_FORMAT_PRINT  "%% PSEditWidget: TEXT (%s)\n"
#define PSEDIT_TEXT_FORMAT_SCAN   "%% PSEditWidget: TEXT (%[^)])\n"
#define PSEDIT_SIZE_FORMAT  "%% PSEditWidget: SIZE %d\n"
#define PSEDIT_GLYPH_FORMAT "%% PSEditWidget: GLYPH %s\n"

#define PSEDIT_PAGE_MARKER  "/PSEditWidgetPageCount %d def %% PSEditWidget\n"

static struct {
  const char *glyph;
  const char *c;
} glyph_char[] = {
  {"adieresis", "ä"}, 
  {"Adieresis", "Ä"}, 
  {"odieresis", "ö"}, 
  {"Odieresis", "Ö"}, 
  {"udieresis", "ü"}, 
  {"Udieresis", "Ü"}, 
  {"germandbls", "ß"}, 
  {"parenleft", "("}, 
  {"parenright", ")"}, 
  {"backslash", "\\"}, 
  {NULL, NULL}};

static const char * glyph_to_char(char *glyph) {
  int i=0;

  while(glyph_char[i].glyph != NULL) {
    if (strcmp(glyph_char[i].glyph, glyph) == 0) {
      return glyph_char[i].c;
      }
    i++;
  }
  
  return NULL;
}

static const char * char_to_glyph(char *c) {
  int i=0;
  
  while(glyph_char[i].glyph != NULL) {
    if (strncmp(glyph_char[i].c, c, 1) == 0) {
      return glyph_char[i].glyph;
    }
    i++;
  }
  
  return NULL;
}

PSParser::PSParser(PSEditWidget *p) {
  pse = p;
}

int PSParser::parse(char *line) {
  return 0;
}


PSParser_1::PSParser_1(PSEditWidget *p) : PSParser(p) {
  page = 1;
}

int PSParser_1::parse(char *line) {
  int x1, y1, size;
  char *s, *e, glyph[1024];

  if (strcmp(line, "showpage\n") == 0) {
    page++;
  }
  
  if (strstr(line, "% PSEditWidget")) {
    if (sscanf(line, PS_SIZE_FORMAT, &size) == 1) {
      pse->set_cur_size(size);
      return 1; // line was recognized 
    } else if (sscanf(line, PS_POS_FORMAT, &x1, &y1) == 2) {
      pse->new_text(pse->ps_to_display_x(x1), pse->ps_to_display_y(y1), 
		    "", page);
      return 1;
    } else if (sscanf(line, PS_GLYPH_FORMAT, glyph) == 1) {
      pse->append_text(glyph_to_char(glyph));
      return 1;
    } else if ((s = strchr(line, '(')) &&
	       (e = strrchr(line, ')'))) {
      *e = '\0';
      s++;
      pse->append_text(s);
      return 1;
    }
    return 0; // line not recognized
  } else {
    return 0; 
  }
}

PSParser_2::PSParser_2(PSEditWidget *p) : PSParser(p) {
  page = 1;
  inside = 0;
}

int PSParser_2::parse(char *line) {
  int x1, y1, size, page, dummy;
  char buf[2028];
  
  if (!inside && strcmp(line, PSEDIT_BEGIN) == 0) {
    inside = 1;
    return 1; // line was recognized 
  } else if (inside && strcmp(line, PSEDIT_END) == 0) {
    inside = 0;
    return 1;
  } else if (inside && sscanf(line, PSEDIT_PAGE_FORMAT, &page) == 1) {
    return 1; 
  } else if (inside && sscanf(line, PSEDIT_SIZE_FORMAT, &size) == 1) {
    pse->set_cur_size(size);
    return 1; 
  } else if (inside && sscanf(line, PSEDIT_POS_FORMAT, &x1, &y1) == 2) {
    pse->new_text(pse->ps_to_display_x(x1), pse->ps_to_display_y(y1),"",page);
    return 1;
  } else if (inside && sscanf(line, PSEDIT_GLYPH_FORMAT, buf) == 1) {
    pse->append_text(glyph_to_char(buf));
    return 1;
  } else if (inside && sscanf(line, PSEDIT_TEXT_FORMAT_SCAN, buf) == 1) {
    pse->append_text(buf);
    return 1;
  } else if (inside) {
    return 1;
  } else if (sscanf(line, PSEDIT_PAGE_MARKER, &dummy) == 1) {
    return 1; 
  } else {
    return 0; // line not recognized
  }
}


//
// Postscript writer methods
//


PSWriter::PSWriter(PSEditWidget *p) {
  pse = p;
}

int PSWriter::write(FILE *in, FILE *out) {
  char linebuf[1024];
  int done=0, page = 1;
  
  while (fgets(linebuf, 1024, in) != NULL) {
    if (!done && strncmp(linebuf, "%%EndSetup", 10) == 0) {
      done++;

      fprintf(out, "\n");
      fprintf(out, "%s", PSEDIT_BEGIN);
      fprintf(out, "\n");

      write_internal_format(out);
      pos_format   = PS_POS_FORMAT;
      size_format  = PS_SIZE_FORMAT;
      text_format  = PS_TEXT_FORMAT;
      glyph_format = PS_GLYPH_FORMAT;

      fprintf(out, "\n");
      fprintf(out, ps_header());

      for (int i=1;i<pse->get_max_pages();i++) {
	if (pse->get_text(i)) {
	  fprintf(out, "dup %d eq { \n", i);
	  write_text(out, pse->get_text(i));
	  fprintf(out, "} if\n");
	}
      }
      
      fprintf(out, ps_trailer());
      fprintf(out, "\n");
      fprintf(out, "%s", PSEDIT_END);
      fprintf(out, "\n");
    }

    fprintf(out, "%s", linebuf);

    if (strncmp(linebuf, "%%Page:", 7) == 0) {
      fprintf(out, PSEDIT_PAGE_MARKER, page++);
    }
  }
}

void PSWriter::write_internal_format(FILE *out) {
  pos_format   = PSEDIT_POS_FORMAT;
  size_format  = PSEDIT_SIZE_FORMAT;
  text_format  = PSEDIT_TEXT_FORMAT_PRINT;
  glyph_format = PSEDIT_GLYPH_FORMAT;

  for (int i=1;i<pse->get_max_pages();i++) {
    if (pse->get_text(i)) {
      fprintf(out, PSEDIT_PAGE_FORMAT, i);
      write_text(out, pse->get_text(i));
    }
  }
}

void PSWriter::write_string(FILE *out, char *s) {
  const char *glyph;
  
  if (strlen(s) == 0) {
    return;
  } else if ((glyph = char_to_glyph(s)) != NULL) {
    fprintf(out, glyph_format, glyph);
    write_string(out, &(s[1]));
    return;
  } else {
    for(int i=0; i<strlen(s); i++) {
      if ((glyph = char_to_glyph(&(s[i]))) != NULL) {
	char *s1 = strdup(s);
	s1[i] = '\0';
	fprintf(out, text_format, s1);
	free(s1);
	write_string(out, &(s[i]));
	return;
	}
    }
    fprintf(out, text_format, s);
  }
  return;
}

int PSWriter::write_text(FILE *out, PSText *t) {
  char *s;

  if (!t) {
    return 0;
  }
  
  s = t->get_text();

  if (strcmp(s, "") != 0) {
    fprintf(out, size_format, t->get_size());
    fprintf(out, pos_format, 
	    pse->ps_x(t->get_x()), 
	    pse->ps_y(t->get_y()));
    write_string(out, s);
  }
  
  if (t->get_next()) {
    return write_text(out, t->get_next());
  }
  
  return 0;
}

char * PSWriter::ps_header() {
  return "";
}

char * PSWriter::ps_trailer() {
  return "";
}

PSLevel1Writer::PSLevel1Writer(PSEditWidget *p) : PSWriter(p) {};

char * PSLevel1Writer::ps_header() {
  return		  \
    "/PSEditWidgetPageCount 0 def\n"		\
    "/PSEditWidgetPC 0 def\n"			\
    "/PSEditWidgetshowpage /showpage load def\n"	\
    "/showpage {\n"							\
    "gsave initgraphics\n"                                              \
    "PSEditWidgetPageCount 0 eq { \n"					\
    "/PSEditWidgetPC PSEditWidgetPC 1 add def PSEditWidgetPC\n"		\
    "} {\n"								\
    "PSEditWidgetPageCount\n"						\
    "} ifelse\n";
}

char * PSLevel1Writer::ps_trailer() {
  return  "PSEditWidgetshowpage grestore} def\n";
}


PSLevel2Writer::PSLevel2Writer(PSEditWidget *p) : PSWriter(p) {};

char * PSLevel2Writer::ps_header() {
  return		  \
    "/PSEditWidgetPageCount 0 def\n"		\
    "<< /EndPage {\n"				\
    "gsave initgraphics\n"                                              \
    "pop\n"								\
    "PSEditWidgetPageCount 0 eq { \n" \
    "1 add                        %% use showpage counter instead.\n"	\
    "} {\n"								\
    "PSEditWidgetPageCount\n"						\
    "} ifelse\n";

}

char * PSLevel2Writer::ps_trailer() {
  return  "grestore true } >> setpagedevice\n";
}
