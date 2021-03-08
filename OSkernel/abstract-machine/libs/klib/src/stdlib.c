#define a2imap(x) ((x)-48)

static unsigned long int next = 1;


int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int atoi(const char* nptr){
    int start=0, sign=1, res=0;
    /*If you want to send -1 when num is overflowed, cancel the comment.*/

/*    if((strcmp(nptr,"2147483647")>0 && strlen(nptr) >=10)|| 
    (nptr[0]=='-' && strlen(nptr) >=11 && strcmp(nptr, "-2147483648")>0))           return -1;*/
    if(a2imap(nptr[0]) == a2imap('-'))
    {
      start++;
      sign =-1;
    }
    for(;nptr[start]!='\0';start++)
    {
      res *= 10;
      res += (int)a2imap(nptr[start]);
    }
    return res*sign;
}