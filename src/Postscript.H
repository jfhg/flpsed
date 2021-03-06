//
// Copyright 2007-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
//
// This software may be used and distributed according to the terms
// of the GNU General Public License, incorporated herein by reference.

#ifndef POSTSCRIPT_H
#define POSTSCRIPT_H

#include <stdio.h>
#include "PSEditModel.H"

class PSParser {
	protected:
		PSEditModel *pse;

		int cur_size;

		PSEditColor cur_text_color;

		int page;

	public: 
		PSParser(PSEditModel *p);
		virtual ~PSParser() {};

		virtual int parse(char *line);
};

class PSParser_2 : public PSParser {
	private:
		int inside;

	public:
		PSParser_2(PSEditModel *p);

		int parse(char *line);
};

class PSWriter {
	protected:
		PSEditModel *pse;

		const char * pos_format;
		const char * size_format;
		const char * text_format;
		const char * color_format;
		const char * glyph_format;
		const char * tag_format;

		void write_string(FILE *out, char *s);

		void write_main_block(FILE *out);

		void write_internal_format(FILE *out);

		int write_text(FILE *out, PSEditText *t);

		virtual const char *ps_header();

		virtual const char *ps_trailer();

	public: 
		PSWriter(PSEditModel *p);
		virtual ~PSWriter() {};

		int write(FILE *in, FILE *out);
};

class PSLevel1Writer : public PSWriter {
	protected:
		const char *ps_header();

		const char *ps_trailer();

	public:
		PSLevel1Writer(PSEditModel *p);
};

#endif
