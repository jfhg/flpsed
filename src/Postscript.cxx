//
// Copyright 2007 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#include <stdlib.h>
#include <string.h>
#include "Postscript.H"

#define PS_POS_FORMAT   "newpath %d %d moveto\n"
#define PS_TEXT_FORMAT  "(%s) show\n"
#define PS_SIZE_FORMAT  "PSEditWidgetFontHelvetica %d scalefont setfont\n"
#define PS_COLOR_FORMAT  "%lf %lf %lf setrgbcolor\n"
#define PS_GLYPH_FORMAT "/%s glyphshow\n"
#define PS_TAG_FORMAT   ""


#define PSEDIT_BEGIN              "% PSEditWidget Begin\n"
#define PSEDIT_END                "% PSEditWidget End\n"

#define PSEDIT_PAGE_FORMAT        "%% PSEditWidget: PAGE %d\n"
#define PSEDIT_POS_FORMAT         "%% PSEditWidget: POS %d %d\n"
#define PSEDIT_TEXT_FORMAT_PRINT  "%% PSEditWidget: TEXT (%s)\n"
#define PSEDIT_TEXT_FORMAT_SCAN   "%% PSEditWidget: TEXT (%[^)])\n"
#define PSEDIT_SIZE_FORMAT        "%% PSEditWidget: SIZE %d\n"
#define PSEDIT_COLOR_FORMAT       "%% PSEditWidget: COLOR %lf %lf %lf\n"
#define PSEDIT_GLYPH_FORMAT       "%% PSEditWidget: GLYPH %s\n"
#define PSEDIT_TAG_FORMAT         "%% PSEditWidget: TAG %s\n"

#define PSEDIT_PAGE_MARKER "/PSEditWidgetPageCount %d def %% PSEditWidget\n"

static struct {
	const char *glyph;
	const char *c;
} glyph_char[] = {
	{"adieresis",  "\xE4"}, 
	{"Adieresis",  "\xC4"}, 
	{"odieresis",  "\xF6"}, 
	{"Odieresis",  "\xD6"}, 
	{"udieresis",  "\xFC"}, 
	{"Udieresis",  "\xDC"}, 
	{"germandbls", "\xDF"}, 
	{"parenleft",  "("}, 
	{"parenright", ")"}, 
	{"percent",    "%"}, 
	{"backslash",  "\\"}, 
	{"exclamdown", "\xA1"},
	{"cent",       "\xA2"},
	{"sterling",   "\xA3"},
	{"currency",   "\xA4"},
	{"yen",        "\xA5"},
	{"brokenbar",  "\xA6"},
	{"section",    "\xA7"},
	{"diaeresis",  "\xA8"},
	{"copyright",  "\xA9"},
	{"ordfeminine","\xAA"},
	{"guillemotleft","\xAB"},
	{"notsign",    "\xAC"},
	{"hyphen",     "\xAD"},
	{"registered", "\xAE"},
	{"macron",     "\xAF"},
	{"degree",     "\xB0"},
	{"plusminus",  "\xB1"},
	{"twosuperior","\xB2"},
	{"threesuperior","\xB3"},
	{"acute",      "\xB4"},
	{"mu",         "\xB5"},
	{"paragraph",  "\xB6"},
	{"periodcentered","\xB7"},
	{"cedilla",    "\xB8"},
	{"onesuperior","\xB9"},
	{"masculine",  "\xBA"},
	{"guillemotright","\xBB"},
	{"onequarter", "\xBC"},
	{"onehalf",    "\xBD"},
	{"threequarters","\xBE"},
	{"questiondown","\xBF"},
	{"Agrave",      "\xC0"},
	{"Aacute",      "\xC1"},
	{"Acircumflex", "\xC2"},
	{"Atilde",      "\xC3"},
	{"Adiaeresis",  "\xC4"},
	{"Aring",       "\xC5"},
	{"AE",          "\xC6"},
	{"Ccedilla",    "\xC7"},
	{"Egrave",      "\xC8"},
	{"Eacute",      "\xC9"},
	{"Ecircumflex", "\xCA"},
	{"Ediaeresis",  "\xCB"},
	{"Igrave",      "\xCC"},
	{"Iacute",      "\xCD"},
	{"Icircumflex", "\xCE"},
	{"Idiaeresis",  "\xCF"},
	{"ETH",         "\xD0"},
	{"Ntilde",      "\xD1"},
	{"Ograve",      "\xD2"},
	{"Oacute",      "\xD3"},
	{"Ocircumflex", "\xD4"},
	{"Otilde",      "\xD5"},
	{"Odiaeresis",  "\xD6"},
	{"multiply",    "\xD7"},
	{"Ooblique",    "\xD8"},
	{"Ugrave",      "\xD9"},
	{"Uacute",      "\xDA"},
	{"Ucircumflex", "\xDB"},
	{"Udiaeresis",  "\xDC"},
	{"Yacute",      "\xDD"},
	{"THORN",       "\xDE"},
	{"ssharp",      "\xDF"},
	{"agrave",      "\xE0"},
	{"aacute",      "\xE1"},
	{"acircumflex", "\xE2"},
	{"atilde",      "\xE3"},
	{"adiaeresis",  "\xE4"},
	{"aring",       "\xE5"},
	{"ae",          "\xE6"},
	{"ccedilla",    "\xE7"},
	{"egrave",      "\xE8"},
	{"eacute",      "\xE9"},
	{"ecircumflex", "\xEA"},
	{"ediaeresis",  "\xEB"},
	{"igrave",      "\xEC"},
	{"iacute",      "\xED"},
	{"icircumflex", "\xEE"},
	{"idiaeresis",  "\xEF"},
	{"eth",         "\xF0"},
	{"ntilde",      "\xF1"},
	{"ograve",      "\xF2"},
	{"oacute",      "\xF3"},
	{"ocircumflex", "\xF4"},
	{"otilde",      "\xF5"},
	{"odiaeresis",  "\xF6"},
	{"division",    "\xF7"},
	{"oslash",      "\xF8"},
	{"ugrave",      "\xF9"},
	{"uacute",      "\xFA"},
	{"ucircumflex", "\xFB"},
	{"udiaeresis",  "\xFC"},
	{"yacute",      "\xFD"},
	{"thorn",       "\xFE"},
	{"ydiaeresis",  "\xFF"},
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

PSParser::PSParser(PSEditModel *p) {
	pse = p;
	cur_size = 12;
}

int PSParser::parse(char *line) {
	return 0;
}

PSParser_2::PSParser_2(PSEditModel *p) : PSParser(p) {
	page = 1;
	inside = 0;
}

int PSParser_2::parse(char *line) {
	int x1, y1, size, dummy;
	PSEditColor c;
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
		cur_size = size;
		return 1; 
	} else if (inside && sscanf(line, PSEDIT_COLOR_FORMAT, &c.r, &c.g, &c.b)
		== 3) {
		cur_text_color.set(&c);
		return 1; 
	} else if (inside && sscanf(line, PSEDIT_POS_FORMAT, &x1, &y1) == 2) {
		pse->new_text(x1, y1, "", cur_size, page, &cur_text_color);
		return 1;
	} else if (inside && sscanf(line, PSEDIT_GLYPH_FORMAT, buf) == 1) {
		pse->append_text(glyph_to_char(buf));
		return 1;
	} else if (inside && sscanf(line, PSEDIT_TEXT_FORMAT_SCAN, buf) == 1) {
		pse->append_text(buf);
		return 1;
	} else if (inside && sscanf(line, PSEDIT_TAG_FORMAT, buf) == 1) {
		pse->set_tag(buf);
		return 1;
	} else if (inside) {
		return 1;
	} else if (sscanf(line, PSEDIT_PAGE_MARKER, &dummy) == 1) {
		return 1; 
	} else {
		return 0; // line not recognized
	}
}




PSWriter::PSWriter(PSEditModel *p) {
	pse = p;
}

int PSWriter::write(FILE *in, FILE *out) {
	char linebuf[1024];
	int done=0, page = 1;

	while (fgets(linebuf, 1024, in) != NULL) {
		// Try to write main block before %%EndSetup or %%EndProlog
		if (!done && 
			(strncmp(linebuf, "%%EndSetup", strlen("%%EndSetup")) == 0 ||
			 strncmp(linebuf, "%%EndProlog", strlen("%%EndProlog")) == 0)) {
			done++;
			write_main_block(out);
		}

		fprintf(out, "%s", linebuf);

		// Try to write main block after %%BeginProlog
		if (!done && 
			(strncmp(linebuf, "%%BeginProlog", strlen("%%BeginProlog")) == 0)) {
			done++;
			write_main_block(out);
		}

		if (strncmp(linebuf, "%%Page:", strlen("%%Page:")) == 0) {
			fprintf(out, PSEDIT_PAGE_MARKER, page++);
		}
	}

	return 0;
}

void PSWriter::write_main_block(FILE *out) {
	fprintf(out, "\n");
	fprintf(out, "%s", PSEDIT_BEGIN);
	fprintf(out, "\n");

	write_internal_format(out);
	pos_format   = PS_POS_FORMAT;
	size_format  = PS_SIZE_FORMAT;
	color_format = PS_COLOR_FORMAT;
	text_format  = PS_TEXT_FORMAT;
	glyph_format = PS_GLYPH_FORMAT;
	tag_format   = PS_TAG_FORMAT;

	fprintf(out, "\n");
	fprintf(out, ps_header());

	for (int i = 1; i<pse->get_max_pages(); i++) {
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


void PSWriter::write_internal_format(FILE *out) {
	pos_format   = PSEDIT_POS_FORMAT;
	size_format  = PSEDIT_SIZE_FORMAT;
	color_format = PSEDIT_COLOR_FORMAT;
	text_format  = PSEDIT_TEXT_FORMAT_PRINT;
	glyph_format = PSEDIT_GLYPH_FORMAT;
	tag_format   = PSEDIT_TAG_FORMAT;

	for (int i = 1; i < pse->get_max_pages(); i++) {
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
		for (int i = 0; i < strlen(s); i++) {
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

int PSWriter::write_text(FILE *out, PSEditText *t) {
	char *s;

	if (!t) {
		return 0;
	}

	s = t->get_text();

	if (strcmp(s, "") != 0 || t->get_tag() != NULL) {
		fprintf(out, size_format, t->get_size());
		fprintf(out, color_format, 
			t->text_color.r,
			t->text_color.g,
			t->text_color.b);
		fprintf(out, pos_format, t->get_x(), t->get_y());
		if (t->get_tag()) {
			fprintf(out, tag_format, t->get_tag());
		}

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

PSLevel1Writer::PSLevel1Writer(PSEditModel *p) : PSWriter(p) {};

char * PSLevel1Writer::ps_header() {
	return
		"/PSEditWidgetPageCount 0 def\n"
		"/PSEditWidgetPC 0 def\n"
		"/PSEditWidgetFontHelvetica /Helvetica findfont def\n"
		"/PSEditWidgetshowpage /showpage load def\n"
		"/showpage {\n"	
		"gsave initgraphics\n"
		"PSEditWidgetPageCount 0 eq { \n"
		"/PSEditWidgetPC PSEditWidgetPC 1 add def PSEditWidgetPC\n"
		"} {\n"
		"PSEditWidgetPageCount\n"
		"} ifelse\n";
}

char * PSLevel1Writer::ps_trailer() {
	return  "grestore PSEditWidgetshowpage} def\n";
}
