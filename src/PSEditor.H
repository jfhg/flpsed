// 
// "$Id: PSEditor.H,v 1.7 2004/10/26 16:58:49 hofmann Exp $"
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

#ifndef PSEDITOR_H
#define PSEDITOR_H

#include "PSEditWidget.H"

class PSEditor : public PSEditWidget {
 private:
  int tmp_fd;

  int ps_level;

  int x_last, y_last;

  int handle(int event);

public: 
  PSEditor(int X,int Y,int W, int H);  

  int load(FILE *fp); 

  int load(char *f); 

  int save(const char* savefile);

  int import(char *f); 
};


  
#endif
