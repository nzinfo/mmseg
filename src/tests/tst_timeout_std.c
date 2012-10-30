#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
/*
 * @zeromq mode tokenizer, the process might be gone. when this happen, the tokenizer should promot user continue or check processor..
 * */

int main(int argc, char **argv)
{
    /* Declare the variables you had ... */
    /*
    struct termios term;
    
    tcgetattr(0, &term);
    term.c_iflag &= ~ICANON;
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    */
    /* Now the rest of your code ... */
    {
      fd_set rfds;
      struct timeval tv;
      int retval;

      /* Watch stdin (fd 0) to see when it has input. */
      FD_ZERO(&rfds);
      FD_SET(0, &rfds);

      /* Wait up to 2 seconds. */
      tv.tv_sec = 2;
      tv.tv_usec = 0;

      retval = select(1, &rfds, NULL, NULL, &tv);

      if (retval == -1)
        perror("select()");
      else if (retval) {
        int c;
        printf("Data is available now.\n");
        while((c=getchar())!= 'e')      
             putchar(c);           
      }
      else
        printf("No data within five seconds.\n");

      exit(EXIT_SUCCESS);
   }
}
