// 
// Postscript Document Structure Convention handling routines.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "PostscriptDSC.H"

PostscriptDSC::PostscriptDSC() {
  bb_x = 0;
  bb_y = 0;
  bb_w = 594; // A4 
  bb_h = 841; //
  setup_len = 0;
  pages = 0;
  page_off = NULL;
  page_len = NULL;
}

PostscriptDSC::~PostscriptDSC() {
}

int 
PostscriptDSC::parse(int fd) {
  FILE *fp;
  char linebuf[1024];
  int x, y, w, h;
  int p1, p2, ps;
  int i = 0;
  
  bb_x = 0;
  bb_y = 0;
  bb_w = 594; // A4 
  bb_h = 841; //
  setup_len = 0;
  pages = 0;
  page_off = NULL;
  page_len = NULL;

  fp = fdopen(fd, "r");
  if (!fp) {
    perror("fdopen");
    return 1;
  }

  while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
    if (sscanf(linebuf, "%%%%BoundingBox: %d %d %d %d", &x, &y, &w, &h) == 4) {
      bb_x = x; 
      bb_y = y; 
      bb_w = w; 
      bb_h = h; 
    } else if (strncmp(linebuf, "%%EndSetup", strlen("%%EndSetup")) == 0) {
      setup_len = ftello(fp);
    } else if (sscanf(linebuf, "%%%%Pages: %d", &ps) == 1) {
      if (pages != 0) {
        fprintf(stderr, "Multiple Pages tags found\n");
        free(page_off);
        free(page_len);
        return 1;
      }

      pages = ps;
      page_off = (size_t*) malloc(sizeof(size_t) * (pages + 1));
      memset(page_off, 0, sizeof(size_t) * (pages + 1));
      page_len = (size_t*) malloc(sizeof(size_t) * (pages + 1));
      memset(page_len, 0, sizeof(size_t) * (pages + 1));
    } else if (sscanf(linebuf, "%%%%Page: %d %d", &p1, &p2) == 2) {
      if (p1 > pages || p1 < 1) {
        fprintf(stderr, "Page %d out of range (0 - %d)\n", p1, pages);
        return 1;
      } 
      if (page_off[p1] != NULL) {
        fprintf(stderr, "Page %d already defined\n", p1);
        return 1;
      }
      if (p1 > 1 && page_off[p1 - 1] == NULL) {
        fprintf(stderr, "Page %d not yet defined\n", p1 - 1);
        return 1;
      }

      page_off[p1] = ftello(fp);
      if (p1 > 1) {
        page_len[p1 - 1] = page_off[p1] - page_off[p1 - 1];
      }
    }      
  }

  page_len[p1] = ftello(fp) - page_off[p1];

  return 0;
}

void
PostscriptDSC::get_bounding_box(int *x, int *y, int *w, int *h) {
  *x = bb_x;
  *y = bb_y;
  *w = bb_w;
  *h = bb_h;
}

int
PostscriptDSC::get_pages() {
  return pages;
}

size_t
PostscriptDSC::get_setup_len() {
  return setup_len;
}

size_t
PostscriptDSC::get_page_off(int p) {
  return page_off[p+1];
}

size_t
PostscriptDSC::get_page_len(int p) {
  return page_len[p+1];
}

void
PostscriptDSC::print() {
  int i;

  printf("x %d, y %d, w %d, h %d\n", bb_x, bb_y, bb_w, bb_h);
  printf("setup_len %d\n", setup_len);
  for (i=1; i<=pages; i++) {
    printf("p %d, off %d, len %d\n", i, page_off[i], page_len[i]);
  }
}

