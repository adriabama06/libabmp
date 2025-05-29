#include "abmp.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

uint32_t abmp_get_pixel_raw_position(ABMP_BITMAP_HEADER header, uint32_t x, uint32_t y)
{
    // Note: It starts from bottom left, it means:
    /* x=0, y=0:
        |oooooo|
        |oooooo|
        |oooooo|
        |Xooooo|
    */
    /* x=1, y=0:
        |oooooo|
        |oooooo|
        |oooooo|
        |oXoooo|
    */
    /* x=4, y=0:
        |oooooo|
        |oooooo|
        |oooooo|
        |ooooXo|
    */
    /* x=1, y=1:
        |oooooo|
        |oooooo|
        |oXoooo|
        |oooooo|
    */
    /* x=3, y=3:
        |oooXoo|
        |oooooo|
        |oooooo|
        |oooooo|
    */

    // move down y times rows
    // + padding*y to skip y times the padding
    // + x in the current line
    return header.width * y * 3 + (header.width % 4) * y + 3 * x;
}

/* Why this function? As a human you view the image starting from top left, like when we read, but in bmp the data is stored in this order:
Image that we see in the screen:
    |ABCDE|
    |FGHIJ|
    |KLMNO|
    |PQRST|

In the bmp file is stored as:
    |PQRST + hidden padding|
    |KLMNO + hidden padding|
    |FGHIJ + hidden padding|
    |ABCDE + hidden padding|

From left to right the order stills the same, but the image is flipped vertically.
This function reverses the vertical order using (header.height - y)
*/
uint32_t abmp_get_pixel_position_from_top_left(ABMP_BITMAP_HEADER header, uint32_t x, uint32_t y)
{
    // Note: It starts from top left, it means:
    /* x=0, y=0:
        |Xooooo|
        |oooooo|
        |oooooo|
        |oooooo|
    */
    /* x=1, y=0:
        |oXoooo|
        |oooooo|
        |oooooo|
        |oooooo|
    */
    /* x=4, y=0:
        |ooooXo|
        |oooooo|
        |oooooo|
        |oooooo|
    */
    /* x=1, y=1:
        |oooooo|
        |oXoooo|
        |oooooo|
        |oooooo|
    */
    /* x=3, y=3:
        |oooooo|
        |oooooo|
        |oooooo|
        |oooXoo|
    */

    // Note: (header.height - y - 1); header.height=4,y=0 --> -1 because header.height=4 and you start conting from zero, making height=3 as height max height
    // this is only to reverse the count of the rows

    // move down y times rows
    // + padding*y to skip y times the padding
    // + x in the current line
    return header.width * (header.height - y - 1) * 3 + (header.width % 4) * (header.height - y - 1) + 3 * x;
}
