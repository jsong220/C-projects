//Name: Myung Jin Song
//Description: cs344 assignment 4
//creates a key file according to specified length. 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//generates and returns a random letter 
  //limited to all capital letters of American alphabet and a space character
int pickRandLetter(){

  int upper = 91; //ASCII for "["
  int lower = 65; //ASCII for "A"
  int r = (rand () % (upper - lower + 1)) + lower; //works cited - https://www.geeksforgeeks.org/generating-random-number-range-c/
  char randomLetter = (char) r;

//replace "[" with space character to fit requirements 
  if (r == 91)
    randomLetter = ' '; 
  //printf("random generater letter..%c\n", randomLetter);

  return randomLetter;
}

int main(int argc, char *argv[]) {

  srand (time(0)); //seed time once for more randomness
  char* enteredNum = argv[1];
  int num = atoi (enteredNum); //works cited -  https://stackoverflow.com/questions/13145777/c-char-to-int-conversion
  char numLetters [num + 1];

  //printf("total output should be..%i\n", num);
  
  int i;
  for (i = 0; i < num; i++){
    numLetters[i] = pickRandLetter();
  }

  for (i =0; i< num; i++){
    printf("%c",numLetters[i]);
  } 

//last letter of key should be a newline per requirements 
  printf("\n");

  return 0;
}
