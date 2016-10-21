#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <unistd.h>
#include <pwd.h>



#include "list.h"
#include "main.h"
#include "wsutils.h"
#include "multiVis.h"

#include "jpeglib.h"

#define WRITE_BUF 	0x8000
int ret_size;
unsigned char *ret_ptr;
int *jpeg_size_ptr;

typedef unsigned long Pixel;


int Image_Size(XImage *image) {
 if(image->format != ZPixmap)
  return(image->bytes_per_line * image->height * image->depth);
 return(image->bytes_per_line * image->height);
}

int Get24bitDirectColors(XColor **colors) {
 int i, ncolors=256;
 XColor *tcol;

 *colors = tcol = (XColor *)malloc(sizeof(XColor) * ncolors);
 if(tcol <= 0) {
  errors = NOMEM;
  return -1;
 }
 for(i=0;i<ncolors;i++) {
  tcol[i].pixel = i << 16 | i <<8 | i;
  tcol[i].red = tcol[i].green = tcol[i].blue = i<<8 | i;
 }
 return ncolors;
}

int ReadColors(Display *dpy, Visual *vis, Colormap cmap, XColor ** colors) {
 int i, ncolors;
#define lobit(x) ((x) & (~(x)+1))
 ncolors = vis->map_entries;
 if(!(*colors = (XColor *)malloc(sizeof(XColor) * ncolors))) {
  errors = NOMEM;
  return -1;
 }

 if(vis->class == DirectColor || vis->class == TrueColor) {
  Pixel red, green, blue, red1, green1, blue1;
  red = green = blue = 0;
  red1 = lobit(vis->red_mask);
  green1=lobit(vis->green_mask);
  blue1=lobit(vis->blue_mask);
  for(i=0;i<ncolors;i++) {
   (*colors)[i].pixel = red|green|blue;
   (*colors)[i].pad = 0;
   red += red1;
   if(red>vis->red_mask)
    red = 0;
   green += green1;
   if(green>vis->green_mask)
    green = 0;
   blue+=blue1;
   if(blue>vis->blue_mask)
    blue = 0;
  }
 } else {
  for(i=0;i<ncolors;i++) {
   (*colors)[i].pixel = i;
   (*colors)[i].pad = 0;
  } 
 }
 XQueryColors(dpy, cmap, *colors, ncolors);
 return(ncolors);
}

int Get_XColors(Display *dpy, XWindowAttributes *win_info, XColor **colors) {
 int i, ncolors;
 Colormap cmap = win_info->colormap;
 if(!cmap)
  return 0;
 ncolors = ReadColors(dpy, win_info->visual, cmap, colors);
 return ncolors;
}

void init_mem_dest(j_compress_ptr cinfo) {
 // nothing to do 
}

boolean drain_mem(j_compress_ptr cinfo) {
 if((ret_ptr = realloc(ret_ptr, ret_size+WRITE_BUF))<=0) {
  errors = NOMEM;
  return FALSE;
 }
 ret_size+=WRITE_BUF;

 *jpeg_size_ptr+=WRITE_BUF;

 cinfo->dest->next_output_byte = ret_ptr + *jpeg_size_ptr;
 cinfo->dest->free_in_buffer = WRITE_BUF;
 return TRUE;
}

void term_mem(j_compress_ptr cinfo) {
 int len;
 len = WRITE_BUF - cinfo->dest->free_in_buffer;
 *jpeg_size_ptr+=len; 
}

unsigned char *shot(Display *dpy, int screen, Window window, int *jpeg_size, int quality) {
 unsigned char *scanline;
 unsigned char r,g,b;
 unsigned char red_shift, green_shift, blue_shift;
 unsigned long data;
 XColor *colors;
 unsigned buffer_size;
 int ncolors, i;
 XWindowAttributes win_info;
 XImage *image;
 int absx, absy, x, y;
 unsigned width, height;
 int dwidth, dheight;
 Window dummywin;
    
 int                 transparentOverlays , multiVis;
 int                 numVisuals;
 XVisualInfo         *pVisuals;
 int                 numOverlayVisuals;
 OverlayInfo         *pOverlayVisuals;
 int                 numImageVisuals;
 XVisualInfo         **pImageVisuals;
 list_ptr            vis_regions;    /* list of regions to read from */
 list_ptr            vis_image_regions ;
 Visual		vis_h,*vis ;
 int			allImage = 0 ;

 struct jpeg_compress_struct cinfo;
 struct jpeg_error_mgr jerr;
 JSAMPROW row_pointer[1];
 struct jpeg_destination_mgr store;

 if(!XGetWindowAttributes(dpy, window, &win_info)) {
  errors = WINATTR;
  return 0;
 }
 
 if(!XTranslateCoordinates(dpy, window, RootWindow(dpy,screen),0,0,
                           &absx, &absy, &dummywin)) {
  errors = TRANS;
  return 0;
 }
 
 win_info.x = absx;
 win_info.y = absy;
 width = win_info.width;
 height = win_info.height;

 dwidth = DisplayWidth(dpy,screen);
 dheight =DisplayHeight(dpy, screen);

 if(absx < 0) width += absx, absx = 0;
 if(absy < 0) height +=absy, absy = 0;
 if(absx+width > dwidth) width = dwidth - absx;
 if(absy+height > dheight) height = dheight - absy;

 x = absx - win_info.x;
 y = absy - win_info.y;

 multiVis = GetMultiVisualRegions(dpy,RootWindow(dpy,screen),
            absx, absy,
	    width,height,&transparentOverlays,&numVisuals,&pVisuals,
	    &numOverlayVisuals,&pOverlayVisuals,&numImageVisuals,
	    &pImageVisuals,&vis_regions,&vis_image_regions,&allImage);
 if(!multiVis)
  image = XGetImage(dpy,RootWindow(dpy,screen),absx,absy,
          width,height,AllPlanes,ZPixmap);
 else
  image = ReadAreaToImage(dpy,RootWindow(dpy,screen),absx,absy,
          width, height,
	  numVisuals,pVisuals,numOverlayVisuals,pOverlayVisuals,
	  numImageVisuals,pImageVisuals,vis_regions,
	  vis_image_regions,ZPixmap,allImage);

 if(!image) {
  errors = X11IMAGE;
  return 0;
 }

 buffer_size = Image_Size(image);

 if(!multiVis) {
  ncolors = Get_XColors(dpy, &win_info, &colors);
  vis = win_info.visual;
 } else {
  ncolors= Get24bitDirectColors(&colors);
  initFakeVisual(&vis_h);
  vis = &vis_h;
 }
 if(ncolors == -1)
  return 0;
 buffer_size = Image_Size(image);


 /* HERE */

 /* why aren't shifts in XImage? Are they? */
#define SHIFTY(X, Y) \
 if(image->X != 0) \
  for(Y = 0;;Y++) \
   if(((image->X >> Y)&1) == 1) break; 
 SHIFTY(red_mask, red_shift);
 SHIFTY(green_mask , green_shift);
 SHIFTY(blue_mask, blue_shift);

 cinfo.err= jpeg_std_error(&jerr);
 jpeg_create_compress(&cinfo);

 *jpeg_size = 0;
 jpeg_size_ptr = jpeg_size;

 if((ret_ptr = (unsigned char *)malloc(WRITE_BUF))<=0) {
  errors = NOMEM;
  return 0;
 }
 ret_size = WRITE_BUF;
 
 cinfo.dest = &store;
 store.init_destination = init_mem_dest;
 store.empty_output_buffer = drain_mem;
 store.term_destination = term_mem;
 store.next_output_byte = ret_ptr;
 store.free_in_buffer = WRITE_BUF;

 cinfo.image_width = image->width;
 cinfo.image_height= image->height;
 cinfo.input_components = 3;
 cinfo.in_color_space = JCS_RGB;

 jpeg_set_defaults(&cinfo);
 jpeg_set_quality(&cinfo, quality, TRUE);
 jpeg_start_compress(&cinfo, TRUE);

 if((scanline = (unsigned char *)malloc(image->width * 3))<=0) {
  errors = NOMEM;
  return 0;
 }
 row_pointer[0] = scanline;

 errors = -1;
 for(y=0;y<image->height;y++) {
  for(x=0;x<image->width;x++) {
   data = image->f.get_pixel(image,x,y);
   if(image->depth <= 8) {
    r = colors[ data % (1<<image->depth) ].red;
    g = colors[ data % (1<<image->depth) ].green;
    b = colors[ data % (1<<image->depth) ].blue;
   } else {
    r = (data & image->red_mask) >> red_shift;
    g = (data & image->green_mask) >> green_shift;
    b = (data & image->blue_mask) >> blue_shift;
   }
   scanline[x * 3] = r;
   scanline[(x*3)+1]=g;
   scanline[(x*3)+2]=b;
  }
  jpeg_write_scanlines(&cinfo, row_pointer, 1);
 }
 jpeg_finish_compress(&cinfo);
 jpeg_destroy_compress(&cinfo);
 if(errors != -1) 
  return 0;
 free(scanline);

 return ret_ptr;
}
