//
// Copyright 2007-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#include <stdlib.h>
#include <string.h>
#include <FL/fl_utf8.h>
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

//
// Marker to set page number. This is necessary for viewers like ghostview
// to display single pages properly.
// 
#define PSEDIT_PAGE_MARKER "/PSEditWidgetPageCount %d def %% PSEditWidget\n"

static struct {
	const char *glyph;
	const char *c;
} glyph_char[] = {
	{"AE",          "\xC3\x86"},
	{"Aacute",      "\xC3\x81"},
	{"Acircumflex", "\xC3\x82"},
	{"Adieresis",  "\xC3\x84"},
	{"Agrave",      "\xC3\x80"},
	{"Aogonek", "\xC4\x84"},
	{"Aring",       "\xC3\x85"},
	{"Atilde",      "\xC3\x83"},
	{"Cacute", "\xC4\x86"},
	{"Ccaron", "\xC4\x8C"},
	{"Ccedilla",    "\xC3\x87"},
	{"Dcaron", "\xC4\x8E"},
	{"Dcroat", "\xC4\x90"},
	{"ETH",         "\xC3\x90"},
	{"Eacute",      "\xC3\x89"},
	{"Ecaron", "\xC4\x9A"},
	{"Ecircumflex", "\xC3\x8A"},
	{"Ediaeresis",  "\xC3\x8B"},
	{"Egrave",      "\xC3\x88"},
	{"Eogonek", "\xC4\x98"},
	{"Euro", "\xE2\x82\xAC"},
	{"Iacute",      "\xC3\x8D"},
	{"Icircumflex", "\xC3\x8E"},
	{"Idiaeresis",  "\xC3\x8F"},
	{"Igrave",      "\xC3\x8C"},
	{"Lslash", "\xC5\x81"},
	{"Nacute", "\xC5\x83"},
	{"Ncaron", "\xC5\x87"},
	{"Ntilde",      "\xC3\x91"},
	{"Oacute",      "\xC3\x93"},
	{"Ocircumflex", "\xC3\x94"},
	{"Odieresis",  "\xC3\x96"},
	{"Ograve",      "\xC3\x92"},
	{"Ooblique",    "\xC3\x98"},
	{"Otilde",      "\xC3\x95"},
	{"Rcaron", "\xC5\x98"},
	{"Sacute", "\xC5\x9A"},
	{"Scaron", "\xC5\xA0"},
	{"THORN",       "\xC3\x9E"},
	{"Tcaron", "\xC5\xA4"},
	{"Uacute",      "\xC3\x9A"},
	{"Ucircumflex", "\xC3\x9B"},
	{"Udieresis",  "\xC3\x9C"},
	{"Ugrave",      "\xC3\x99"},
	{"Uring", "\xC5\xAE"},
	{"Yacute",      "\xC3\x9D"},
	{"Zacute", "\xC5\xB9"},
	{"Zcaron", "\xC5\xBD"},
	{"Zdotaccent", "\xC5\xBB"},
	{"aacute",      "\xC3\xA1"},
	{"acircumflex", "\xC3\xA2"},
	{"acute",      "\xC2\xB4"},
	{"adieresis",  "\xC3\xA4"},
	{"ae",          "\xC3\xA6"},
	{"agrave",      "\xC3\xA0"},
	{"aogonek", "\xC4\x85"},
	{"aring",       "\xC3\xA5"},
	{"atilde",      "\xC3\xA3"},
	{"backslash",  "\\"},
	{"brokenbar",  "\xC2\xA6"},
	{"cacute", "\xC4\x87"},
	{"ccaron", "\xC4\x8D"},
	{"ccedilla",    "\xC3\xA7"},
	{"cedilla",    "\xC2\xB8"},
	{"cent",       "\xC2\xA2"},
	{"copyright",  "\xC2\xA9"},
	{"currency",   "\xC2\xA4"},
	{"dcaron", "\xC4\x8F"},
	{"dcroat", "\xC4\x91"},
	{"degree",     "\xC2\xB0"},
	{"diaeresis",  "\xC2\xA8"},
	{"division",    "\xC3\xB7"},
	{"eacute",      "\xC3\xA9"},
	{"ecaron", "\xC4\x9B"},
	{"ecircumflex", "\xC3\xAA"},
	{"ediaeresis",  "\xC3\xAB"},
	{"egrave",      "\xC3\xA8"},
	{"eogonek", "\xC4\x99"},
	{"eth",         "\xC3\xB0"},
	{"exclamdown", "\xC2\xA1"},
	{"germandbls", "\xC3\x9F"},
	{"guillemotleft","\xC2\xAB"},
	{"guillemotright","\xC2\xBB"},
	{"hyphen",     "\xC2\xAD"},
	{"iacute",      "\xC3\xAD"},
	{"icircumflex", "\xC3\xAE"},
	{"idiaeresis",  "\xC3\xAF"},
	{"igrave",      "\xC3\xAC"},
	{"lslash", "\xC5\x82"},
	{"macron",     "\xC2\xAF"},
	{"masculine",  "\xC2\xBA"},
	{"mu",         "\xC2\xB5"},
	{"multiply",    "\xC3\x97"},
	{"nacute", "\xC5\x84"},
	{"ncaron", "\xC5\x88"},
	{"notsign",    "\xC2\xAC"},
	{"ntilde",      "\xC3\xB1"},
	{"oacute",      "\xC3\xB3"},
	{"oacute", "\xC3\xB3"},
	{"ocircumflex", "\xC3\xB4"},
	{"odieresis",  "\xC3\xB6"},
	{"ograve",      "\xC3\xB2"},
	{"onehalf",    "\xC2\xBD"},
	{"onequarter", "\xC2\xBC"},
	{"onesuperior","\xC2\xB9"},
	{"ordfeminine","\xC2\xAA"},
	{"oslash",      "\xC3\xB8"},
	{"otilde",      "\xC3\xB5"},
	{"paragraph",  "\xC2\xB6"},
	{"parenleft",  "("},
	{"parenright", ")"},
	{"percent",    "%"},
	{"periodcentered","\xC2\xB7"},
	{"plusminus",  "\xC2\xB1"},
	{"questiondown","\xC2\xBF"},
	{"rcaron", "\xC5\x99"},
	{"registered", "\xC2\xAE"},
	{"sacute", "\xC5\x9B"},
	{"scaron", "\xC5\xA1"},
	{"section",    "\xC2\xA7"},
	{"ssharp",      "\xC3\x9F"},
	{"sterling",   "\xC2\xA3"},
	{"tcaron", "\xC5\xA5"},
	{"thorn",       "\xC3\xBE"},
	{"threequarters","\xC2\xBE"},
	{"threesuperior","\xC2\xB3"},
	{"twosuperior","\xC2\xB2"},
	{"uacute",      "\xC3\xBA"},
	{"ucircumflex", "\xC3\xBB"},
	{"udieresis",  "\xC3\xBC"},
	{"ugrave",      "\xC3\xB9"},
	{"uring", "\xC5\xAF"},
	{"yacute",      "\xC3\xBD"},
	{"ydiaeresis",  "\xC3\xBF"},
	{"yen",        "\xC2\xA5"},
	{"zacute", "\xC5\xBA"},
	{"zcaron", "\xC5\xBE"},
	{"zdotaccent", "\xC5\xBC"},
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
		if (strncmp(glyph_char[i].c, c, strlen(glyph_char[i].c)) == 0) {
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
	fprintf(out, "%s", ps_header());

	for (int i = 1; i<pse->get_max_pages(); i++) {
		if (pse->get_text(i)) {
			fprintf(out, "dup %d eq { \n", i);
			write_text(out, pse->get_text(i));
			fprintf(out, "} if\n");
		}
	}

	fprintf(out, "%s", ps_trailer());
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

static int utf8len(char *s) {
	int len;

	if (*s & 0x80) {
		fl_utf8decode(s, s + strlen(s), &len);
	} else {
		len = 1;
	}

	return len;
}

void PSWriter::write_string(FILE *out, char *s) {
	const char *glyph;

	if (strlen(s) == 0) {
		return;
	} else if ((glyph = char_to_glyph(s)) != NULL) {
		fprintf(out, glyph_format, glyph);
		write_string(out, &(s[utf8len(&(s[0]))]));
		return;
	} else {
		for(size_t i=0; i < strlen(s); i = i + utf8len(&(s[i]))) {
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

const char * PSWriter::ps_header() {
	return "";
}

const char * PSWriter::ps_trailer() {
	return "";
}

PSLevel1Writer::PSLevel1Writer(PSEditModel *p) : PSWriter(p) {};

const char * PSLevel1Writer::ps_header() {
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

const char * PSLevel1Writer::ps_trailer() {
	return  "grestore PSEditWidgetshowpage} def\n";
}
