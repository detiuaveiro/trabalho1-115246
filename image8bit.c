/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec: 115246 Name: Daniela Silva
// 
// 
// 
// Date: 11/19/2023
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly-.

Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);

  //alocar memoria para nova imagem
  Image img = malloc(sizeof(struct image));
  //erro ao criar
  if (img == NULL) {
    errno = ENOMEM;;
    errCause = ENOMEM;
    return NULL;}

  //iniciar valores da imagem
  img -> width = width;
  img -> height = height;
  img -> maxval = (int) maxval;
  //alocar memoria pixeis
  img -> pixel = (uint8*)malloc(sizeof(uint8)*width*height);

  //erro ao criar
  if (img -> pixel == NULL) {
    free(img);
    errno = ENOMEM;
    errCause = ENOMEM;
    return NULL;
  }

  //iniciar os pixeis com valor 0
  for (int i = 0;i < width*height;i++){ 
    img->pixel[i]=0;
  }
  return img;

}




/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause?????.
void ImageDestroy(Image* imgp) { ///
  assert (*imgp != NULL);
  // Insert your code here!-----

  Image image = *imgp;
  // Libera a memória dos pixels
  free(image -> pixel);
  // Libera a memória da estrutura Image
  free(image);
  *imgp = NULL;
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  // Insert your code here!---
  // Variável para armazenar o valor do pixel
  uint8 pix;

  // Percorre cada pixel na imagem
  for (int x = 0;x <img -> width;x++){
    for(int y = 0;y<img->height;y++){
      // Valor do pixel na posição (x, y)
      pix = ImageGetPixel(img, x, y);
      // Atualiza o valor mínimo ou maior
      if (pix > *max) {*max = pix;}
      if (pix < *min) {*min = pix;}
    }
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  // Insert your code here!

  assert(x >= 0);
  assert(y >= 0);
  assert(w >= 0);
  assert(h >= 0);

  // Variável para indicar se a área retangular é válida
  int valid_rect = 0;

  if (x + w <= img -> width && y + h <= img -> height){valid_rect = 1;}
  return valid_rect; // 1 se a área retangular é válida, 0 caso contrário

}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index;
  // Insert your code here! -------

//For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].

  index = y * img -> width + x;

  assert (0 <= index && index < img->width*img->height);
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.

void ImageNegative(Image img) { ///
  assert (img != NULL);
  uint8 pix_og;// Variável para armazenar o valor original do pixel
  // Insert your code here!----
  for (int x = 0;x <img -> width;x++){
    for(int y = 0;y<img->height;y++){
      pix_og = ImageGetPixel(img,x,y);// Valor original do pixel na posição (x, y)
      ImageSetPixel(img,x,y,img -> maxval - pix_og); // Define o valor do pixel como o complemento em relação ao valor máximo
      }
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert (img != NULL);
// Insert your code here!----------
  for (int x = 0;x < img -> width;x++){
    for(int y = 0;y < img->height;y++){
      
      if (ImageGetPixel(img,x,y) >= thr){
        ImageSetPixel(img,x,y,img -> maxval);// Define o pixel como branco (maxval) se o valor for maior ou igual ao thr
      }
      else {
        ImageSetPixel(img,x,y,0);// Define o pixel como preto (0) se o valor for menor que thr
      }
    }
  
}
}

//pixmen pixops calltime time 

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert (img != NULL);
  assert (factor >= 0.0);
  assert (factor <= 1.0);
  // Insert your code here!---------
  uint8 pix_og,pix_new;// Variáveis para armazenar o valor original e o novo valor do pixel
  for (int x = 0;x < img -> width;x++){
    for(int y = 0;y < img->height;y++){
      pix_og = ImageGetPixel(img,x,y);// Valor original do pixel na posição (x, y)
      pix_new = pix_og * factor + 0.5;// Calcula o novo valor do pixel multiplicando pelo fator e adicionando 0.5 para arredondamento
      
      ImageSetPixel(img,x,y,(pix_new > img->maxval) ? img->maxval : pix_new);// Satura o novo valor do pixel em maxval, se for maior que maxval
    }
  }
}








/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees !counter! clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) { ///
  assert (img != NULL);
  // Insert your code here!-----------

    // Cria uma nova imagem com largura e altura trocadas
  Image img_new = ImageCreate(img->height, img->width, img->maxval);
  if(img_new == NULL){
    errCause = ENOMEM;
    errno = ENOMEM;
    return NULL;
    }

  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {
      uint8 pix_og = ImageGetPixel(img, x, y);   // Valor original do pixel na posição (x, y)
      ImageSetPixel(img_new, y, img->width - 1 - x, pix_og);// Define o pixel na nova imagem trocando as coordenadas x e y e invertendo a direção
    }
  }
  return img_new;
}




/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly-FALTA.
Image ImageMirror(Image img) { ///
  assert (img != NULL);
  // Insert your code here!-----------

  // Cria uma nova imagem com as mesmas dimensões
  Image img_new = ImageCreate(img -> width,img -> height,img -> maxval);
  if (img_new == NULL) { 
    errno = ENOMEM;
    errCause = ENOMEM;
    return NULL;}

  uint8 pix_og;// Variável para armazenar o valor original do pixel
  for (int x = 0;x < img -> width;x++){
    for(int y = 0;y < img->height;y++){
      pix_og = ImageGetPixel(img,x,y);
      ImageSetPixel(img_new,img -> width - x - 1,y,pix_og);// Define o pixel na nova imagem refletindo em relação ao eixo vertical
    }
  }
  return img_new;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));
  // Insert your code here!------

// Cria uma nova imagem para armazenar a região recortada
  Image img_new = ImageCreate(w,h,img -> maxval);
  if (img_new == NULL) { 
    errno = ENOMEM;
    errCause = ENOMEM;
    return NULL;}

  uint8 pix_og;// Variável para armazenar o valor original do pixel

  // Itera sobre a região especificada na imagem original
  for (int i = 0;i < w;i++){
    for(int j = 0;j < h;j++){
      pix_og = ImageGetPixel(img,i + x,j + y);// Obtém o valor original do pixel na posição (i + x, j + y)
      ImageSetPixel(img_new,i,j,pix_og);// Define o pixel na nova imagem
    }
  }
  return img_new;
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!-----

  uint8 pix_img2;// Variável para armazenar o valor do pixel da segunda imagem
  
  // Itera sobre os pixels da segunda imagem
  for (int i = 0;i < img2 -> width;i++){
    for(int j = 0;j < img2 -> height;j++){
      pix_img2 = ImageGetPixel(img2,i,j);// Obtém o valor do pixel da segunda imagem na posição (i, j)
      ImageSetPixel(img1,i + x,j + y,pix_img2);// Define o pixel na primeira imagem na posição (i + x, j + y)
    }
  }
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!---------

  uint8 pix_img2,pix_img1,pix_blend;
  // Itera sobre os pixels da segunda imagem
  for (int i = 0;i < img2 -> width;i++){
    for(int j = 0;j < img2 -> height;j++){
      // Obtém os valores dos pixels das duas imagens nas posições correspondentes
      pix_img2 = ImageGetPixel(img2,i,j);
      pix_img1 = ImageGetPixel(img1,i + x,j + y);
      pix_blend = ((1.0 - alpha) * pix_img1 + alpha * pix_img2) + 0.5;// Calcula o pixel resultante da mistura ponderada, +0.5 para arrendondamentos

      ImageSetPixel(img1,i + x,j + y,pix_blend);// Define o pixel na primeira imagem na posição (i + x, j + y) como o resultado da mistura
    }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));
  // Insert your code here!

  int match = 1;// Inicializa a variável de correspondência como verdadeira
  uint8 pix_img2,pix_img1;
  // Itera sobre os pixels da segunda imagem
  for (int i = 0;i < img2 -> width;i++){
    for(int j = 0;j < img2 -> height;j++){
      // Obtém os valores dos pixels das duas imagens nas posições correspondentes
      pix_img1 = ImageGetPixel(img1,i + x,j + y);
      pix_img2 = ImageGetPixel(img2,i,j);
      // Verifica se os pixels são diferentes; se sim, a correspondência é falsa
      if (pix_img1 != pix_img2){match = 0;}
    }
  }
  return match;// 1 se verdadeira, 0 caso contrario

}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  // Insert your code here!

 int match = 0; // Inicializa a variável de correspondência como falsa

 // Itera sobre as posições possíveis da subimagem dentro da imagem principal
  for (int i = 0;i < img2 -> width;i++){
    for(int j = 0;j < img2 -> height;j++){

// Verifica se há uma correspondência entre as subimagens
      if(ImageMatchSubImage(img1,i,j,img2)){
        match = 1;// Configura a correspondência como verdadeira
        // Define a posição x e y da correspondência
        *px = i;
        *py = j; 
        break;// Sai do loop interno, pois encontrou uma correspondência
      }
      
    }
    // Sai do loop externo se uma correspondência já foi encontrada
    if (match) {
      break;
    }
  }
  return match; //1 se verdadeiro, 0 caso contrario
}


/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy) { ///
  // Insert your code here!
  assert (img != NULL);

  uint8_t pix_filter;// Variável para armazenar o valor do pixel após o desfoque
  // Cria uma cópia da imagem original para armazenar o resultado do desfoque.
  Image img_copia = ImageCreate(ImageWidth(img),ImageHeight(img),ImageMaxval(img));


// Itera sobre os pixels da imagem
  for (int i = 0; i < img->width; i++) {
    for (int j = 0; j < img->height; j++) {
      double sum = 0;
      double count = 0;

// Itera sobre a janela definida por (dx, dy) em torno do pixel atual
      for (int di = -dx; di <= dx; di++) {
        for (int dj = -dy; dj <= dy; dj++) {
          int ni = i + di;
          int nj = j + dj;
  // Verifica se a posição (ni, nj) está dentro dos limites da imagem
          if (ni >= 0 && ni < img->width && nj >= 0 && nj < img->height) {
            sum += ImageGetPixel(img, ni, nj);
            count++;
          }
        }
      }

     // Calcula o valor médio (+0.5 para arredondamentos) e define-o no pixel de saída
      if (count > 0) {
        pix_filter = (sum / count + 0.5) ;
        ImageSetPixel(img_copia, i, j, pix_filter);
      }
    }
  }
  // Substitui a imagem original pela imagem desfocada
  ImagePaste(img,0,0,img_copia);

}

