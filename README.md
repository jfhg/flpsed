# flpsed


flpsed is a PostScript annotator. You can't actually 
edit existing PostScript documents, but you can add arbitrary text lines to
existing documents.
It is useful for filling in forms etc. 

## Quick Start:
- Open an existing PostScript document.
- Click anywhere on the document and type a text line.
- The frame around the text shows, which text line has the focus.
- Click on the lower left corner of a text line to focus it or use the 
  Tab-key to cycle through the text lines on the current page. 
- Remove text, by hitting BackSpace.
- Move text lines by dragging them with the mouse or using the arrow keys.
- Navigate within the document with the Page->Next or Page->First menu buttons.
- Save your document and preview it with ghostview or something similar.
- If you reopen the document with flpsed, you can edit the added text lines.

## Features:
- Add arbitrary text to existing PostScript documents.
- Reedit text, that has been added with flpsed.
- The overall structure of the PostScript document is not
  modified. flpsed only adds the additional text.
- Lines can be given names ("tags"). The text of these lines can
  be replaced in batch mode (no X11 required).
 
## Restrictions:
- flpsed probably does not work on all existing PostScript documents.
  You simply have to test it for your documents.
- Zooming depends on the availability of scalable fonts on your X11 system.
  Font sizes might be wrong with zoom values other than 100% if these are 
  missing.
- flpsed is alpha software, so please backup your files, before trying to 
  modify them.

## Building:
- flpsed only works on X11-based systems.
- You need to have ghostscript installed.
- You need to have fltk-1.3.x from www.fltk.org installed.
- Unpack the tarball and type "./configure", "make", and "make install".



## Tags and Batch Mode:

To use batch mode, add text lines to your PostScript document as usual. 
Give all or some of the lines tag names (Tags->Edit Tag). 
Save the document. Now you can replace the text of the tagged line in batch
mode using the -t flag (see usage).
Example:
Lets assume you have added text lines with tags "name", and "street"
to your document letter.ps with flpsed in interactive mode and saved the
result in letter-templ.ps.
You can now call flpsed in batch mode to set the actual values:

flpsed -b -t name="Hans Meier" -t street="Haupstr. 14" letter-templ.ps out.ps

