%{
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include <ctype.h>


#include "list.h"
#include "main.h"
#include "lexxer.h"

#define BUFFER_SIZE	100
#define STRSIZE		2048
#define HOST_NAME_SIZE	0xff

#define LEN_ENCODING 	0
#define CHUNK_ENCODING	1

#define ALLOC_CHUNK	0x2000
int msg_data_alloc;

#define MODE_HTTP	0
#define MODE_ALBUM	1
#define MODE_CONFIG	2
#define MODE_TOKENS	3
#define MODE_ALBUM_ID	4
#define MODE_PID_ID	5

int lex_mode = MODE_HTTP;

char *pid_number;

int sock;
struct sockaddr_in server;
long server_addr;

int a,b;
int off_a, off_b;
char c[STRSIZE];
#define NAME_SIZE	300
char name[NAME_SIZE];
char number[NAME_SIZE];

char *d;
int i,j;

FILE *config;

msg_t *current_msg;
int msg_pos;

unsigned int body_len, body_pos;
int body_enc;
unsigned int chunk_len, chunk_pos;

int _just_head;
int lex_error;
msg_t *current_msg;

unsigned int msg_buf_i;
char msg_buffer[MSG_BUFFER];


int new_var(msg_t *msg, char *var, int var_len, char *value, int value_len);

#define LEX_ERROR \
 lex_error = 1; \
 yyterminate(); \

#define YY_INPUT(buf,result,max_size)\
{ \
 char c; \
 switch(lex_mode) { \
  case MODE_HTTP: \
   if(read(sock,&c,1)!=1) \
    result = YY_NULL; \
   else { \
    if((int)dump_file!=0) fputc(c, dump_file); \
    result = (buf[0] = c,1); \
   } \
   break; \
  case MODE_TOKENS: \
  case MODE_ALBUM: \
  case MODE_ALBUM_ID: \
  case MODE_PID_ID: \
   c = current_msg->data[msg_pos++]; \
   if(msg_pos > current_msg->len) \
    result = YY_NULL; \
   else \
    result = (buf[0] =c ,1); \
   break; \
  case MODE_CONFIG: \
   if(fread(&c,1,1,config)!=1) \
    result = YY_NULL; \
   else \
    result = (buf[0] = c,1); \
   break; \
 } \
}

#define SANE \
  if(body_pos >= msg_data_alloc) { \
   if((current_msg->data=realloc(current_msg->data,msg_data_alloc*2))<=0) { \
    errors = NOMEM; \
    LEX_ERROR; \
   } \
   msg_data_alloc*=2; \
  } 

#define SCAN_STRING(Q)\
 j = 0;\
 for(i=0;i<strlen(yytext);i++) {\
  if(yytext[i] == '=') {\
   while(yytext[++i]==' ') { }\
   if(yytext[i] == '\"') {\
    while(yytext[++i]!='\"') {\
     Q[j++] = yytext[i];\
    }\
    Q[j] = 0;\
    break;\
   } else {\
    while(yytext[i]!=' '&&yytext[i]!='\n') {\
     Q[j++] = yytext[i++];\
    }\
    Q[j] = 0;\
    break;\
   }\
  }\
 }

#define SCAN_NUMBERS\
 j = 0;\
 for(i=0;i<strlen(yytext);i++) {\
  if(yytext[i] == '=') {\
   i++;\
   while(yytext[i]!='\n'&&yytext[i]!=0)\
    if(yytext[i]!=',')\
     c[j++] = yytext[i++];\
    else {\
     i++;\
     c[j++] = ' ';\
    }\
   c[j] = 0;\
   break;\
  }\
 }



%}

%option noyywrap
%x head 
%x body
%x chunk_size
%x album_lex
%x token_lex
%x album_id_lex
%x pid_id_lex
%x cat_lex
%x cat_lex_b
%x config_file

HEAD	^\x20*
CRLF	\r\n
TAIL	\x20*{CRLF}
DIGIT	[0-9]
HEX	[0-9a-fA-F]
STRING	.+
COMMENT	("#".+\n)
IS	\x20*"="\x20*
CTAIL	(\x20*(\n|({COMMENT}*)|<EOF>))
CSTRING	(\".+\")|[^\0x20\n<EOF>]+


%%


"HTTP/1.1"\x20+{DIGIT}+\x20*{STRING}*{TAIL} {
 sscanf(yytext, "HTTP/1.1 %d ", &a);
 switch(a/100) {
  case 1:
   snprintf(errors_str, ERRORS_STR, "Not implemented %d", a);
   errors = STR;
   LEX_ERROR;
   break;
  case 2:
   if(a == 200) 
    BEGIN(head);
   else {
    snprintf(errors_str, ERRORS_STR, "Not implemented %d", a);
    errors = STR;
    LEX_ERROR;
   }
   break;
  case 3:
   BEGIN(head);
   break;
  case 4:
   switch(a) {
    case 400:
     snprintf(errors_str, ERRORS_STR, "400 Malformed request");
     errors = STR;
     LEX_ERROR;
     break;
    case 404:
     snprintf(errors_str, ERRORS_STR, "404 Not Found");
     errors = STR;
     LEX_ERROR;
     break;
    case 403:
     snprintf(errors_str, ERRORS_STR, "403 Forbidden");
     errors = STR;
     LEX_ERROR;
     break;
    default:
     snprintf(errors_str, ERRORS_STR, "Client error not implemented %d\n", a);
     errors = STR;
     LEX_ERROR; 
     break;
   }
   break;
  case 5:
   snprintf(errors_str, ERRORS_STR, "Server error %d\n", a);
   errors = STR;
   LEX_ERROR;
   break;
 }
}

<head>{STRING}*":"{STRING}*{TAIL} {
 for(a =0;;a++) 
  if(yytext[a] == ':') break;
 for(b=0;;b++)
  if(yytext[a+1+b] == '\r' ||
     yytext[a+1+b] == '\n') break;

 for(off_a = 0;;off_a++) 
  if(yytext[off_a] != ' ') break;

 for(off_b = 0;;off_b++)
  if(yytext[a+1+off_b] != ' ') break;

 if(new_var(current_msg, 
    yytext+off_a, a-off_a, 
    yytext+a+1+off_b, b-off_b)==0) {
  LEX_ERROR;
  }
}

<head>{TAIL} {
 if(_just_head == HEAD) 
  yyterminate();
 d = get_var(current_msg, "Content-Length");
 if( d != 0 ) {
  body_len = atoi(d);
  body_pos = 0;
  body_enc = LEN_ENCODING;
  current_msg->len = 0;
  BEGIN(body);
 } else { 
  d = get_var(current_msg, "Transfer-Encoding");
  if(d!=0) {
   if(strncmp("chunked", d, strlen("chunked"))==0) {
    body_enc = CHUNK_ENCODING;
    body_pos = 0;
    current_msg->len = 0;
    BEGIN(chunk_size);
   }
  } else {
   errors = STR;
   snprintf(errors_str, ERRORS_STR, "encoding not implemented\n");
   LEX_ERROR;
  }
 }
}

<chunk_size>({CRLF})?{HEX}+(;|\x20)?{CRLF} {
 j = 0;
 for(i=0;;i++) {
  if((yytext[i] == '\r')  ||
     (yytext[i] == '\n')) {
   if(j==0)
    c[i] = ' ';
   else {
    c[i] = 0;
    break;
   }
  }
  if((yytext[i] != '\r') &&
     (yytext[i] != '\n')) {
   j = 1;
   c[i] = tolower(yytext[i]);
  }
  
 }
 chunk_pos = 0;
 sscanf(c, "%x", &chunk_len);
 if(chunk_len == 0)
  yyterminate();
 BEGIN(body);
}

<body>.|\n {
 if(body_enc == LEN_ENCODING) {
  SANE;
  current_msg->data[body_pos++] = *yytext;
  current_msg->len++;
  if(body_pos==body_len) 
   yyterminate();
 } else {
  SANE;
  current_msg->data[body_pos++] = *yytext;
  current_msg->len++;
  chunk_pos++;
  if(chunk_pos==chunk_len) 
   BEGIN(chunk_size);
 }
}

<pid_id_lex>"displayimage.php?pid="{DIGIT}+ {
 pid_number = &yytext[21];
 return atoi(pid_number);
}

<album_id_lex>"<option value=\""{DIGIT}+"\" selected=\"selected\">* My gallery *</option>" {
 return -1;
}

<album_id_lex>"<option value=\""{DIGIT}+"\">* My gallery *</option>" {
 for(i=0;;i++) {
  if(yytext[15+i] == '\"') { number[i] = 0; break; }
  number[i] = yytext[15+i];
 }
 return atoi(number);
}

<token_lex>"name=\"form_token\" value=\""{STRING}\" {
 for(i=0;;i++) {
  if(yytext[25+i] == '\"') { hidden_token[i] = 0; break; }
  hidden_token[i] = yytext[25+i];
 }
}

<token_lex>"name=\"timestamp\" value=\""{STRING}\" {
 for(i=0;;i++) {
  if(yytext[24+i] == '\"') { hidden_timestamp[i] = 0; break; }
  hidden_timestamp[i] = yytext[24+i];
 }
}

<album_lex>"<span class=\"albumName\">"{STRING}"&nbsp;<a href=\"upload.php?album="{DIGIT}+"\">" {

 
 for(i=0;;i++) {
  if(yytext[24+i] == '&') { name[i] = 0; break; }
  name[i] = yytext[24+i];
 }

 for(i=0;;i++) {
  if(yytext[24+strlen(name)+32+i] == '\"') { number[i] = 0; break; }
  number[i] =yytext[24+strlen(name)+32+i]; 
 }

 add_album(&albums, name, atoi(number));

}

<cat_lex>"<b>Select category</b>" {
 BEGIN(cat_lex_b);
}

<cat_lex_b>"<option value=\""{DIGIT}+\"((\x20*">")|(\x20*selected\x20*">")){STRING}"</option>" {
 b = 0;
 for(a=strlen(yytext);a!=0;a--) {
  if(yytext[a] == '>') b++;
  if(b==2) { a--; break; }
 }
 printf("PANTS:%s\n", &yytext[a]);
}

<cat_lex_b>"</select>" {
 yyterminate();
}

<config_file>{HEAD}{COMMENT} { }

<config_file>{HEAD}"title"{IS}{CSTRING}{CTAIL} {
 title = (char *)malloc(strlen(yytext));
 SCAN_STRING(title);
}

<config_file>{HEAD}"album"{IS}{CSTRING}{CTAIL} {
 album_name = (char *)malloc(strlen(yytext));
 SCAN_STRING(album_name);
}

<config_file>{HEAD}"password"{IS}{CSTRING}{CTAIL} {
 password = (char *)malloc(strlen(yytext));
 SCAN_STRING(password);
}

<config_file>{HEAD}"cat"{IS}{CSTRING}{CTAIL} {
 cat_name = (char *)malloc(strlen(yytext));
 SCAN_STRING(cat_name);
}

<config_file>{HEAD}"desc"{IS}{CSTRING}{CTAIL} {
 desc = (char *)malloc(strlen(yytext));
 SCAN_STRING(desc);
}

<config_file>{HEAD}"user"{IS}{CSTRING}{CTAIL} {
 username = (char *)malloc(strlen(yytext));
 SCAN_STRING(username);
}

<config_file>{HEAD}"timeout"{IS}{DIGIT}+{CTAIL} {
 SCAN_NUMBERS;
 sscanf(c, "%d", &a);
 timeout_sec = a;
}

<config_file>{HEAD}up_timeout{IS}{DIGIT}+{CTAIL} {
 SCAN_NUMBERS;
 sscanf(c, "%d", &a);
 uptimeout_sec = a;
}

<config_file>{HEAD}qual{IS}{DIGIT}+{CTAIL} {
 SCAN_NUMBERS;
 sscanf(c, "%d", &a);
 jpeg_quality = a;
 if(jpeg_quality > 100)
  jpeg_quality = 100;
 if(jpeg_quality < 1)
  jpeg_quality = 1;
}

<config_file>{HEAD}wait{IS}{DIGIT}+{CTAIL} {
 SCAN_NUMBERS;
 sscanf(c, "%d", &a);
 wait_sec = a;
}



<config_file>.|\n { }
<album_lex>.|\n	{ }
<token_lex>.|\n { } 
<album_id_lex>.|\n {  }
<pid_id_lex>.|\n {  }

<cat_lex>.|\n { }
<*>.|\n {
// errors = STR;
// snprintf(errors_str, ERRORS_STR, "Malformed response or parser is stupid");
// LEX_ERROR;
 return 0;
}


%%

int new_var(msg_t *msg, char *var, int var_len, char *value, int value_len) {
 head_t *new;
 if((new = (head_t *)malloc(sizeof(head_t)))<=0) {
  errors = NOMEM;
  return 0;
 }
 if(msg->header == 0)  {
  msg->header = new;
  INIT_LIST_HEAD(&msg->header->node);
 } else 
  list_add(&new->node, &msg->header->node);

 if((new->var = (char *)malloc(var_len+1))<=0) {
  errors = NOMEM;
  return 0;
 }
 if((new->value = (char *)malloc(value_len+1))<=0) {
  errors = NOMEM;
  return 0;
 }
 memcpy(new->var, var, var_len);
 memcpy(new->value, value, value_len);
 new->var[var_len] = 0;
 new->value[value_len] = 0;
 return 1;
}

void free_msg(msg_t *msg) {
 /* XXX probably is leaky */
 head_t *walker, *first, *store;
 if(msg->header !=0 ) {
  first = walker = msg->header;
  for(;;) {
   if(walker->var !=0 )
    free(walker->var);
   if(walker->value !=0)
    free(walker->value);
   if((int)walker->node.next == (int)first) { 
    free(walker);
    break;
   } else {
    store = walker;
    walker = (head_t *)walker->node.next;
    free(store);
   }
  }
 }
 free(msg);
}

void dump_vars(msg_t *msg) {
 head_t *walker;
 if(msg != 0) 
  if(msg->header != 0) {
   walker = msg->header;
   for(;;) {
    printf("::%s::%s::\n", walker->var, walker->value);
    if((int)(walker = (head_t *)walker->node.next) == (int)msg->header) break;
   } 
  }
}

char *get_var(msg_t *msg, char *var) {
 head_t *walker;
 if(msg != 0) 
  if(msg->header != 0) {
   walker = msg->header;
   for(;;) {
    if(strncmp(walker->var, var, strlen(var))== 0)
     return walker->value;
    if((int)(walker = (head_t *)walker->node.next) == (int)msg->header) break;
   } 
  }
 return 0;
}

void do_config(char *filename) {
 if((config = fopen(filename, "rb"))<=0) return;

 yyrestart(0);
 BEGIN(config_file);
 lex_mode = MODE_CONFIG;
 yylex();
 fclose(config);
}

msg_t *ask(char *name, u_int16_t port, int just_head) {
 msg_t *ret;
 int i,j;
 int div, rem;
 int upload_pos;
 struct hostent *inf;
 if((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<=0) {
  errors = CHECK_ERRNO;
  return 0;
 }
 if((inf = gethostbyname(name))<=0) { 
  errors = CHECK_ERRNO;
  errno = h_errno;
  return 0;
 };
 if(inf->h_length <= sizeof(long))
  memcpy(&server_addr,inf->h_addr,inf->h_length);
 else  {
  errors = LADDR;
  return 0;
 }
 server.sin_addr.s_addr = server_addr;
 server.sin_port = htons(port);
 server.sin_family=AF_INET; 
 if(connect(sock, (struct sockaddr *)&server, sizeof(server))<0) {
  errors = CHECK_ERRNO;
  errno = h_errno;
  return 0;
 }

 if((ret = (msg_t *)malloc(sizeof(msg_t)))<=0) {
  errors = NOMEM;
  return 0;
 } 
 ret->header = 0;

 if(just_head == UPLOAD) {

  j = spin_go;
  spin_go = 0;
  while(spin_ack==0);

  div = msg_buf_i / 100;
  rem = msg_buf_i % 100;
  upload_pos = 0;

  printf("(00%%)");
  fflush(stdout);
  for(i=0;i<100;i++) {
   printf("\33[5D");
   fflush(stdout);
   write(sock, msg_buffer+upload_pos, div);
   if((int)dump_file!=0) 
    write(fileno(dump_file), msg_buffer+upload_pos, div); 
   upload_pos+=div;
   printf("(%02d%%)", i);
   fflush(stdout);
  }
  if(rem != 0) {
   write(sock, msg_buffer+upload_pos, rem); 
   if((int)dump_file!=0) 
    write(fileno(dump_file), msg_buffer+upload_pos, rem); 
  }
  printf("\33[5D\33[K");
  fflush(stdout);

  spin_go = j;
  just_head = BODY;
 } else {
  write(sock, msg_buffer, msg_buf_i);
  if((int)dump_file!=0) write(fileno(dump_file), msg_buffer, msg_buf_i); 
 }


 lex_error = 0; 
 if((_just_head = just_head) == BODY) {
  if((ret->data = (char *)malloc(ALLOC_CHUNK))<=0) {
   errors = NOMEM;
   return 0;
  }
  msg_data_alloc = ALLOC_CHUNK;
 } else
  ret->data = 0;


 current_msg = ret;
 yyrestart(0);
 BEGIN(0);
 lex_mode = MODE_HTTP;
 yylex();
 if(lex_error!=0)
  return 0;
 
 close(sock);
 return ret;
}

int find_cats(msg_t *msg) {
 current_msg = msg;
 msg_pos = 0;
 yyrestart(0);
 BEGIN(cat_lex);
 lex_mode = MODE_ALBUM;
 yylex();
 if(lex_error!=0)
  return 0;
 return 1;
}

int find_albums(msg_t *msg) {
 current_msg = msg;
 msg_pos = 0;
 yyrestart(0);
 BEGIN(album_lex);
 lex_mode = MODE_ALBUM;
 yylex();
 if(lex_error!=0)
  return 0;
 return 1;
}

int find_album_id(msg_t *msg) {
 current_msg = msg;
 msg_pos = 0;
 yyrestart(0);
 BEGIN(album_id_lex);
 lex_mode = MODE_ALBUM_ID;
 return yylex();
}

int find_pid(msg_t *msg) {
 current_msg = msg;
 msg_pos = 0;
 yyrestart(0);
 BEGIN(pid_id_lex);
 lex_mode = MODE_PID_ID;
 return yylex();
}

void get_tokens(msg_t *msg) {
 current_msg = msg;
 msg_pos = 0;
 yyrestart(0);
 BEGIN(token_lex);
 lex_mode = MODE_TOKENS;
 yylex();
}
