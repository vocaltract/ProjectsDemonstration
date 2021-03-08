// #include "klib.h"
// #include <stdarg.h>

// #if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// #define LEN_OF_CHECKLIST (sizeof(op_map)/sizeof(op_map[0]))
// #define INPUT_ERROR (LEN_OF_CHECKLIST+2)
// #define ADD_ZERO (LEN_OF_CHECKLIST+1)

// enum {TYPE_S, TYPE_D, TYPE_X};
// char* check_lists [3]={"s", "d", "x"};
// size_t op_map[3] = {2,2,2};

// size_t check_in_and_load_prefix (char** check_list, const char* str, char* prefix){
//   size_t count = 0;
//   while ('0'<=str[0] && str[0]<='9')
//   {
//     prefix[count] = str[0];
//     str++;
//     count++;
//   }
//   prefix[count] = '\0';
//   for(size_t i=0;i<LEN_OF_CHECKLIST;i++){
//     if(strncmp(str,check_list[i],op_map[i]-1)==0){
//       return i;
//     }
//   }  
//   return INPUT_ERROR;
// }


// //状态表 type_and_pos_and_status[2] 0是已经写到底了，1是刚好找到
// int write_and_fetch(const char* str, char** check_list, size_t* type_and_pos_and_status, char* dst, size_t* write, char* prefix){
//   size_t i=0;
//   size_t check_result;
//   if(str[i]=='\0'){
//     type_and_pos_and_status[2]=0;
//     return 1;
//   }
//   while(str[i]!='\0'){
//     if(str[i]=='%'){
//       check_result =check_in_and_load_prefix(check_list,&str[i+1],prefix);
//       if(check_result<INPUT_ERROR){
//         type_and_pos_and_status[0] = check_result;
//         type_and_pos_and_status[1] = i;
//         type_and_pos_and_status[2] = 1;
//         return 1;
//       }
//       else
//         return -1; 
//     }
//     else{
//       dst[i] = str[i];
//       (*write)++;
//     }  
//     i++;
//   }
//   type_and_pos_and_status[2]=0;
//   dst[i]='\0';
//   return 1;
// }


// void reverse_str(char* str){
//   char temp;
//   size_t len = strlen(str);
//   for(size_t i =0; i< len/2; i++){
//     temp = str[i];
//     str[i] = str[len-1-i];
//     str[len-1-i]=temp; 
//   }
// }


// void convert_int2str(int src, char* res){//不能处理最小负数
//   int temp=0;
//   size_t i = 0;
//   if(src==0){
//     res[0]='0';
//     res[1]='\0';
//     return;
//   }

//   if(src<0){
//     res[i] = '-';
//     src = -src;
//     i++;
//   }
//   while(src!=0){
//     temp = src-(src/10)*10;
//     res[i] = (char)(temp+48);
//     src = src/10;
//     i++;
//   }
//   res[i]='\0';
//   if(src<0){
//     reverse_str(&res[1]);
//   }
//   else{
//     reverse_str(&res[0]);
//   }
// }

// void str_move(char* str, int n){
//   if(str==NULL){
//     assert(0);
//   }
//   size_t len = strlen(str);
//   if(n>0){
//     for(int i= len;i>=0;i--)
//     {
//       str[i+n] = str[i];
//     }
//   }
//   else if(n ==0)
//   {
//     return;
//   }
//   else{
//     for(int i=n;i<0;i++){
//       str--;
//     }
//     for(int j=0;j<=len;j++){
//       str[j] = str[j-n];
//     }
//   }
// }

// void char_clear(char* str)
// {
//   for(size_t i=0; str[i]!='\0';i++)
//   {
//     str[i]='\0';
//   }
// }

// size_t convert_str2_number(char* src)//only support nonnegative number 
// {
//   size_t temp = 0;
//   size_t sum = 0;
//   for(size_t i= 0;i<strlen(src);i++)
//   {
//       temp = (size_t) (src[i])-48;
//       sum += 10*sum + temp;
//   }
//   return sum;
// }

// void add_entity(size_t num, char* src, char entity)
// {
//   if(num==0)
//     return;
//   str_move(src, num);
//   for(size_t i=0; i<num;i++)
//   {
//     src[i] = entity;
//   }
  
// }
// void convert_int2x_str(int src, char* res){//不能处理最小负数
//   int new_bit=0;
//   size_t i = 0;
//   if(src==0){
//     res[0]='0';
//     res[1]='\0';
//     return;
//   }

//   if(src<0){
//     res[i] = '-';
//     src = -src;
//     i++;
//   }
//   while(src!=0){
//     new_bit = src & 0b1111;
//     src = src >> 4;
//     switch (new_bit)
//     {
//     case 10: res[i]='A';break;
//     case 11: res[i]='B';break;
//     case 12: res[i]='C';break;
//     case 13: res[i]='D';break;
//     case 14: res[i]='E';break;
//     case 15: res[i]='F';break;
//     default: res[i]=(char)(new_bit+48);break;
//     }
//     i++;
//   }
//   res[i]='\0';
//   if(src<0){
//     reverse_str(&res[1]);
//   }
//   else{
//     reverse_str(&res[0]);
//   }
// }


// int prepare(char* out ,const char* fmt, va_list p_para)
// {
//   char insert[1000];
//   size_t type_and_pos_and_status[3] = {0,0,1};
//   char* char_buffer;
//   int int_buffer;
//   int judge;
//   char* out_copy = out;
//   char prefix[30];
//   size_t count_direct_write = 0;
//   size_t prefix_len=0;
//   size_t len_of_insert=0;
//   while(type_and_pos_and_status[2]){
//       judge = write_and_fetch(fmt, check_lists, type_and_pos_and_status, out, &count_direct_write, prefix);
//       prefix_len = strlen(prefix);
//       if(judge==-1){
//         return -1;
//       }
//       if(type_and_pos_and_status[2]!=0){
//         switch (type_and_pos_and_status[0])
//         {
//         case TYPE_S:
//         {
//           char_buffer = va_arg(p_para, char*);
//           len_of_insert = strlen(char_buffer);
//           memcpy(&out[type_and_pos_and_status[1]],char_buffer, len_of_insert);
//         }
//           break;
//         case TYPE_D:
//         {
//           int_buffer = va_arg(p_para, int);
//           convert_int2str(int_buffer, insert);
//           len_of_insert =strlen(insert);
//           if(prefix[0]!='\0')
//           {
//             if(prefix[0]=='0')
//             {
//               size_t width = convert_str2_number(prefix);
//               if(width>len_of_insert)
//                 add_entity(width-len_of_insert,insert,'0');
//             }
//             else
//             {
//               size_t width = convert_str2_number(prefix);
//               if(width>len_of_insert)
//                 add_entity(width-len_of_insert,insert, ' ');
//             }
//           }
          
//           len_of_insert =strlen(insert);//更新
//           memcpy(&out[type_and_pos_and_status[1]],insert, len_of_insert);       
//         }
//           break;
//         case TYPE_X:
//         {
//           int_buffer = va_arg(p_para, int);
//           convert_int2x_str(int_buffer, insert);
//           len_of_insert =strlen(insert);//更新
//           memcpy(&out[type_and_pos_and_status[1]],insert, len_of_insert);       
//         }
//           break;          
//         default: assert(0);
//           break;
//         }
//         fmt = &fmt[op_map[type_and_pos_and_status[0]]+count_direct_write+prefix_len]; 
//         out[len_of_insert+type_and_pos_and_status[1]]='\0';
//         out = &out[len_of_insert+type_and_pos_and_status[1]];
//         count_direct_write = 0;
//         prefix_len = 0;
    
//       }
//     char_clear(prefix);
//     char_clear(insert);
//     }
//   return strlen(out_copy);
// }

// void true_print(char* str)
// {
//   for(size_t i=0;str[i]!='\0';i++) _putc(str[i]);
// }

// int printf(const char *fmt, ...) {
//   char store[999999];
//   va_list p_para;
//   va_start(p_para, fmt);
//   prepare(store, fmt, p_para);
//   va_end(p_para);
//   true_print(store);
//   return strlen(store);
// }

// int vsprintf(char *out, const char *fmt, va_list ap) {
//   return 0;
// }

// int sprintf(char *out, const char *fmt, ...){
//     char store[999999];
//     va_list p_para;
//     va_start(p_para, fmt);
//     prepare(store, fmt, p_para);
//     va_end(p_para);
//     strcpy(out,store);
//     return strlen(out);
// }

// int snprintf(char *out, size_t n, const char *fmt, ...) {
//   return 0;
// }

// #endif
