// 
// "$Id: Postscript.H,v 1.6 2005/02/28 19:53:58 hofmann Exp $"
//
// X11 header file for the Fast Light Tool Kit (FLTK).
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

#ifndef POSTSCRIPT_H
#define POSTSCRIPT_H

#include <stdio.h>
#include "PSEditModel.H"

class PSParser {
 protected:
  PSEditModel *pse;

  int cur_size;

  int page;

 public: 
  PSParser(PSEditModel *p);

  virtual int parse(char *line);
};
  
class PSParser_1 : public PSParser {
 public:
  PSParser_1(PSEditModel *p);

  int parse(char *line);
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

  char * pos_format;

  char * size_format;

  char * text_format;

  char * glyph_format;

  char * tag_format;

  void write_string(FILE *out, char *s);

  void write_main_block(FILE *out);

  void write_internal_format(FILE *out);

  int write_text(FILE *out, PSEditText *t);

  virtual char *ps_header();

  virtual char *ps_trailer();

 public: 
  PSWriter(PSEditModel *p);

  int write(FILE *in, FILE *out);
};

class PSLevel1Writer : public PSWriter {
 protected:
  char *ps_header();

  char *ps_trailer();

 public:
  PSLevel1Writer(PSEditModel *p);
};

class PSLevel2Writer : public PSWriter {
 protected:
  char *ps_header();

  char *ps_trailer();

 public:
  PSLevel2Writer(PSEditModel *p);
};
  
#endif
