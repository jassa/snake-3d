/* File for "Textures" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */

#ifndef SNAKE_IMAGE_H_
#define SNAKE_IMAGE_H_

// Represents an image
class Image {
 public:
  Image(char* ps, int w, int h);
  ~Image();

  /* An array of the form (R1, G1, B1, R2, G2, B2, ...) indicating the
   * color of each pixel in image.  Color components range from 0 to 255.
   * The array starts the bottom-left pixel, then moves right to the end
   * of the row, then moves up to the next column, and so on.  This is the
   * format in which OpenGL likes images.
   */
  char* pixels;
  int width;
  int height;
};

#endif // SNAKE_IMAGE_H_
