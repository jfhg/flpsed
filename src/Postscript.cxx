// 
// "$Id: Postscript.cxx,v 1.3 2004/07/09 18:28:19 hofmann Exp $"
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
#define PS_FONT_SIZE_FORMAT "/HelveticaNeue-Roman findfont %d scalefont setfont %% PSEditWidget\n"
#define PS_GLYPH_FORMAT     "/%s glyphshow %% PSEditWidget\n"

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


//
// Postscript parser methods
//


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
    if (sscanf(line, PS_FONT_SIZE_FORMAT, &size) == 1) {
      pse->set_cur_size(size);
      return 1; // line was recognized 
    } else if (sscanf(line, PS_POS_FORMAT, &x1, &y1) == 2) {
      pse->new_text(pse->ps_to_display_x(x1), pse->ps_to_display_y(y1), 
		    "", page);
      return 1;
    } else if (sscanf(line, PS_GLYPH_FORMAT, glyph) == 1) {
      fprintf(stderr, "GLYPH %s\n", glyph);
      pse->append_text(glyph_to_char(glyph));
      return 1;
    } else if ((s = strchr(line, '(')) &&
	       (e = strrchr(line, ')'))) {
      *e = '\0';
      s++;
      pse->append_text(s);
      return 1;
    }
    return 2; // line should be removed, but passed to other parsers 
  } else {
    return 0; // line not recognized
  }
}

PSParser_2::PSParser_2(PSEditWidget *p) : PSParser(p) {
  page = 1;
}

int PSParser_2::parse(char *line) {
  return 0; // line not recognized
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

      fprintf(out, ps_header());

      for (int i=1;i<pse->get_max_pages();i++) {
	if (pse->get_text(i)) {
	  fprintf(out, "dup %d eq { \n", i);
	  write_text(out, pse->get_text(i));
	  fprintf(out, "} if\n");
	}
      }
      
      fprintf(out, ps_trailer());
    }

    fprintf(out, "%s", linebuf);

    if (strncmp(linebuf, "%%Page:", 7) == 0) {
      fprintf(out, "/PSEditWidgetPageCount %d def %% PSEditWidget\n", page++);
    }
  }
}

void PSWriter::write_string(FILE *out, char *s) {
  const char *glyph;
  
  if (strlen(s) == 0) {
    return;
  } else if ((glyph = char_to_glyph(s)) != NULL) {
    fprintf(out, PS_GLYPH_FORMAT, glyph);
    write_string(out, &(s[1]));
    return;
  } else {
    for(int i=0; i<strlen(s); i++) {
      if ((glyph = char_to_glyph(&(s[i]))) != NULL) {
	char *s1 = strdup(s);
	s1[i] = '\0';
	fprintf(out, PS_TEXT_FORMAT, s1);
	free(s1);
	write_string(out, &(s[i]));
	return;
	}
    }
    fprintf(out, PS_TEXT_FORMAT, s);
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
    fprintf(out, PS_FONT_SIZE_FORMAT, t->get_size());
    fprintf(out, PS_POS_FORMAT, 
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
    "%%\n" \
    "%% Begin PSEditWidget\n"	 \
    "%%\n" \
    "/PSEditWidgetPageCount 0 def\n"		\
    "/PSEditWidgetPC 0 def\n"			\
    "/PSEditWidgetshowpage /showpage load def\n"	\
    "/showpage {\n"							\
    "PSEditWidgetPageCount 0 eq { \n"					\
    "/PSEditWidgetPC PSEditWidgetPC 1 add def PSEditWidgetPC\n"		\
    "} {\n"								\
    "PSEditWidgetPageCount\n"						\
    "} ifelse\n";
}

char * PSLevel1Writer::ps_trailer() {
  return  "PSEditWidgetshowpage} def\n" \
    "%%\n" \
    "%% End PSEditWidget\n" \
    "%%\n";
}


PSLevel2Writer::PSLevel2Writer(PSEditWidget *p) : PSWriter(p) {};

char * PSLevel2Writer::ps_header() {
  return		  \
    "%%\n" \
    "%% Begin PSEditWidget\n"	 \
    "%%\n" \
    "/PSEditWidgetPageCount 0 def\n"		\
    "<< /EndPage {\n"				\
    "pop\n"								\
    "PSEditWidgetPageCount 0 eq { \n" \
    "1 add                        %% use showpage counter instead.\n"	\
    "} {\n"								\
    "PSEditWidgetPageCount\n"						\
    "} ifelse\n";

}

char * PSLevel2Writer::ps_trailer() {
  return  "true } >> setpagedevice\n" \
    "%%\n" \
    "%% End PSEditWidget\n" \
    "%%\n";
}
