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
#include <ctype.h>
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
  if (page_off) {
    free(page_off);
  }
  if (page_len) {
    free(page_len);
  }
}

int 
PostscriptDSC::parse(int fd) {
  FILE *fp;
  char linebuf[1024];
  int x, y, w, h;
  int p1 = 0, ps;
  int i = 0;
  int bb_read = 0;
  int max_pages = 2;
  
  bb_x = 0;
  bb_y = 0;
  bb_w = 594; // A4 
  bb_h = 841; //
  setup_len = 0;
  pages = 0;

  if (page_off) {
    free(page_off);
    page_off = NULL;
  }
  if(page_len) {
    free(page_len);
    page_len = NULL;
  }

  fp = fdopen(fd, "r");
  if (!fp) {
    perror("fdopen");
    return 1;
  }

  page_off = (size_t*) malloc(sizeof(size_t) * max_pages);
  memset(page_off, 0, sizeof(size_t) * max_pages);
  page_len = (size_t*) malloc(sizeof(size_t) * max_pages);
  memset(page_len, 0, sizeof(size_t) * max_pages);

  while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
    if (!bb_read &&
        sscanf(linebuf, "%%%%BoundingBox: %d %d %d %d", &x, &y, &w, &h) == 4) {
      bb_x = x; 
      bb_y = y; 
      bb_w = w; 
      bb_h = h; 
      bb_read++;
    } else if (strncmp(linebuf, "%%EndSetup", strlen("%%EndSetup")) == 0) {
      setup_len = ftello(fp);
    } else if (strncmp(linebuf, "%%Page: ", strlen("%%Page: ")) == 0) {
      char *p_str = &linebuf[strlen(linebuf)];

      // move p_str back to beginning of last number in linebuf
      while (p_str > linebuf && !isdigit(*p_str)) {
        p_str--;
      }
      while (p_str > linebuf && isdigit(*p_str)) {
        p_str--;
      }

      p1 = atoi(p_str);

      if (p1 < 1) {
        fprintf(stderr, "Page %d out of range\n", p1);
        return 1;
      } 

      if (p1 > max_pages) {
        page_off = (size_t*) realloc(page_off, 2 * max_pages * sizeof(size_t));
        memset(page_off + max_pages, 0, sizeof(size_t) * max_pages);
        page_len = (size_t*) realloc(page_len, 2 * max_pages * sizeof(size_t));
        memset(page_len + max_pages, 0, sizeof(size_t) * max_pages);
        max_pages = max_pages * 2;
      }

      if (page_off[p1 - 1] != 0) {
        fprintf(stderr, "Page %d already defined\n", p1);
        return 1;
      }
      if (p1 > 1 && page_off[p1 - 2] == NULL) {
        fprintf(stderr, "Page %d not yet defined\n", p1 - 1);
        return 1;
      }

      page_off[p1 - 1] = ftello(fp);
      if (p1 > 1) {
        page_len[p1 - 2] = page_off[p1 - 1] - page_off[p1 - 2];
      }

      if (p1 > pages) {
        pages = p1;
      }
    }      
  }
  
  if (page_len && page_off && p1 > 0 && p1 <= pages) {
    page_len[p1 - 1] = ftello(fp) - page_off[p1 - 1];

    for (int i=0; i<pages; i++) {
      if (page_off[i] == 0 || page_len[i] == 0) {
        fprintf(stderr, "Page %d not defined\n", i);
        return 1;
      }
    }
  } else {
    return 1;
  }

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
  if (p < 1 || p > pages) {
    return 0;
  } else {
    return page_off[p - 1];
  }
}

size_t
PostscriptDSC::get_page_len(int p) {
  if (p < 1 || p > pages) {
    return 0;
  } else {
    return page_len[p - 1];
  }
}

void
PostscriptDSC::print() {
  int i;

  printf("x %d, y %d, w %d, h %d\n", bb_x, bb_y, bb_w, bb_h);
  printf("setup_len %d\n", setup_len);
  for (i=0; i<pages; i++) {
    printf("p %d, off %d, len %d\n", i, page_off[i], page_len[i]);
  }
}

