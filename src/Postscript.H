// 
// "$Id: Postscript.H,v 1.1 2004/07/09 16:13:35 hofmann Exp $"
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

#include "PSEditWidget.H"


class PSParser {
 protected:
  PSEditWidget *pse;
  int page;

 public: 
  PSParser(PSEditWidget *p);
  virtual int parse(char *line);
};
  
class PSParser_1 : public PSParser {
 public:
  PSParser_1(PSEditWidget *p);
  int parse(char *line);
};

class PSParser_2 : public PSParser {
 public:
  PSParser_2(PSEditWidget *p);
  int parse(char *line);
};

class PSWriter {
 protected:
  PSEditWidget *pse;
  void write_string(FILE *out, char *s);
  int write_text(FILE *out, PSText *t);
  virtual char *ps_header();
  virtual char *ps_trailer();

 public: 
  PSWriter(PSEditWidget *p);
  int write(FILE *in, FILE *out);
};

class PSLevel1Writer : public PSWriter {
 protected:
  char *ps_header();
  char *ps_trailer();
 public:
  PSLevel1Writer(PSEditWidget *p);
};

class PSLevel2Writer : public PSWriter {
 protected:
  char *ps_header();
  char *ps_trailer();
 public:
  PSLevel2Writer(PSEditWidget *p);
};


  
#endif
