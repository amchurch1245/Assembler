
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "writer.h"

// Error case if incorrect input source is given
static void usage(char *name)
{
  printf("Usage: %s [input source]\n\
where:\n\
\t[input source] is a file containing assembly source code.\n\
", name);
  exit(1);
}

// argv[0] = program name (for example, main.c)
// argv[1] = assembly source code file name (for example, example1-spaced.S)
// Usage example: ./main.c example1-spaced.S
int main( int argc, char *argv[] )
{
  struct line* llh; // Linked list for assembly line named "llh"
  uint32_t *text_segment, *data_segment; // Declared two 32b binary pointers, One for text segment, Other for data segment
  size_t prog_sz; // size_t represents size in bytes (?) of variable prog_sz

  if ( argc < 2 ) usage(argv[0]); // Error case

  llh = get_lines(argv[1]); // Calls get_lines from parser.c on assembly code
  
  /* Error case if there is a file error */

  if (!llh) {
    fprintf(stderr, "Error getting the lines of file: %s\n", argv[1]);
    exit(1);
  }

  // Prints description of assembly lines of llh
  print_lines(llh);

  // ENDED on 9/12/2021

  // // Allocates space for each binary variable
  // data_segment = malloc(sizeof(uint32_t)*DATA_SEGMENT_WORDS);
  // text_segment = malloc(sizeof(uint32_t)*TEXT_SEGMENT_WORDS);

  // // Error case
  // if (data_segment == NULL || text_segment == NULL) {
  //   fprintf(stderr, "Uh oh, looks like we ran out of memory!\n");
  //   exit(1);
  // }

  // /* TODO: convert the lines in llh into data and text segment binary
  //  * representations */

  // // write_bin_rep(llh, text_segment);
  // // write_bin_rep(llh, data_segment);

  // prog_sz = write_program("a.mxe", text_segment, data_segment); // Writing binary representations to file "a.mxe"
  // assert(prog_sz == DATA_SEGMENT_WORDS+TEXT_SEGMENT_WORDS); // Error code for checking prog_sz

  // free_lines(llh); // Empties linked list

  return 0;
}