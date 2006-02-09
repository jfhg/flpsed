// 
// Copyright 2006 by Johannes Hofmann
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

#ifndef POSTSCRIPTDSC_H
#define POSTSCRIPTDSC_H

class PostscriptDSC {
  private:
    int bb_x, bb_y, bb_w, bb_h;
    size_t setup_len;
    int pages;
 
    size_t *page_off;
    size_t *page_len;

  public:
    PostscriptDSC();
    ~PostscriptDSC();
    
    int parse(int fd);
    int get_pages();
    void get_bounding_box(int *x, int *y, int *w, int *h);
    size_t get_setup_len();
    size_t get_page_off(int p);
    size_t get_page_len(int p);
    void print();
};

#endif
