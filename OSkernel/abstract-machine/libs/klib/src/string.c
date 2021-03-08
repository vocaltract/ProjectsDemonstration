#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
size_t strlen(const char *s) {
  size_t count_str = 0;
  const char *temp = s;
  while(*temp != '\0'){
    count_str++;
    temp++;
  }
  return count_str;
}

char *strcpy(char* dst,const char* src) {
  if(dst==NULL || src==NULL){
  //  printf("NULL pointer error!");
    return NULL;
  }
  size_t i =0;
  for(;src[i]!='\0';i++){
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  if(dst==NULL || src==NULL){
  //  printf("NULL pointer error!");
    return NULL;
  }
  size_t count = 0;
  for(;count<n && src[count]!='\0';count++){
    dst[count] = src[count];
  }
  while(n>count){
    dst[count] ='\0';
    count++;
  }
  return dst;
}

char* strcat(char* dst, const char* src) {
  if(dst==NULL || src==NULL){
  //printf("NULL pointer error!");
  return NULL;
  }
  size_t dest_len = strlen(dst);
  size_t i = 0;
  for (;src[i] != '\0' ; i++) dst[dest_len + i] = src[i];
  dst[dest_len + i] = '\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  if(s1==NULL || s2==NULL){
  //  printf("NULL pointer error!");
    return 0;
  }
  size_t count = 0;
  for(;s1[count]!='\0' && s2[count]!='\0';count++){
    if(s1[count]>s2[count]){
      return 1;
    }
    else if(s1[count]<s2[count]){
      return -1;
    }
  }
  if(s1[count]!='\0'){
    return 1;
  }
  else if(s2[count]!='\0') 
    return -1;
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  if(s1==NULL || s2==NULL){
  //  printf("NULL pointer error!");
    return 0;
  }
  size_t count = 0;
  for(;count<n;count++){
    if(s1[count]>s2[count]){
      return 1;
    }
    else if(s1[count]<s2[count]){
      return -1;
    }
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  if(v == NULL){
    return v;
  }
  char* temp = v;
  while(n>0){
    temp[n-1] = (char)c;
    n--;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  if(out == NULL || in == NULL){
  //  printf("NULL pointer error!");
    return NULL;
  }
  char* out_type2 = (char*) out;
  char* in_type2  = (char*) in;
  for(;n>0;n--){
    out_type2[n-1] = in_type2[n-1];
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
    if(s1==NULL || s2==NULL){
//    printf("NULL pointer error!");
    return 0;
  }
  char* s1_type2 = (char*) s1;
  char* s2_type2 = (char*) s2;
  for(size_t count=0;count<n;count++){
    if(s1_type2[count] < s2_type2[count]){
      return -1;
    }
    else if(s1_type2[count] > s2_type2[count]){
      return 1;
    }
  }
  return 0;
}

#endif
