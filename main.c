#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>

#include <sys/time.h>

#include <string.h>


#include "list.h"
#include "main.h"
#include "shot.h"
#include "lexxer.h"

#include "messages.h"

#include "album.h"


#define SPIN_DELAY	20000
#define RANDOM_LEN	10
#define FILENAME_LEN	6+RANDOM_LEN

album_t *albums = 0;
album_t *cats = 0;

int wait_sec = 0;
char nul[] = {" "};
char *album_name = 0;
char *cat_name = 0;
char *title = 0;
char *desc = 0;
char *tags = 0;
char *username = 0;
char *disp = NULL;
char *password = 0;
int timeout_sec = 20;
int uptimeout_sec = 100;
int upload_id = 0;
int album_id = 0;
int jpeg_quality = 100;
FILE *dump_file = 0;

#define START	\
 msg_buf_i = 0; \
 body_i = 0;

Display *dpy;
int screen;
Window target;

char hidden_token[MAXSTR];
char hidden_timestamp[MAXSTR];

int body_i;
unsigned char body_msg[MSG_BUFFER];

sem_t *spin_done;
pthread_t *spin_thread;
int spin_go = 0, spin_timeout = 0, spin_ack = 0;
const char spin_chars[] = {"\\|/-"};

int errors;
char errors_str[ERRORS_STR];

typedef struct {
 struct list_head node;
 char *data;
} cookie_t;

cookie_t *cookies = 0;

char errors_txt[][64] = {
 {"Can't get window attributes\n" },
 {"Can't translate window coords\n" },
 {"Can't get X11 image\n" },
 {"Can't get more memory\n" },
 {"Address longer then a long, this is a bug- report it\n"},
};

#define FATAL(X) \
{ \
 spin_go = 0; \
 sem_wait(spin_done); \
 switch(errors) { \
  case CHECK_ERRNO: \
   printf("<-%s\n", strerror(errno)); \
   break; \
  case NOTHING: \
   printf(X); \
   break; \
  case STR :\
   printf("<-%s\n", errors_str); \
   break; \
  default: \
   printf("<-%s\n", errors_txt[errors]); \
   break; \
 } \
 exit(0); \
}
void timeout(int sigraised) {
 spin_timeout = 1;
}

#define REPORT(X) \
{ \
 printf(X); \
 fflush(stdout); \
 spin_go = 1; \
}

#define FINISH(X) \
 alarm(0); \
 spin_go = 0; \
 sem_wait(spin_done); \
 printf(X); \
 fflush(stdout); \
 sem_post(spin_done);



pthread_t *new_thread(void *(*proc)(void *a)) {
 pthread_attr_t type;
 pthread_t *ret;
 if(pthread_attr_init(&type)!=0) 
  return 0;
 if((ret = (pthread_t *)malloc(sizeof(pthread_t)))<=0)
  return 0;
 pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);
 if(pthread_create(ret, &type, proc, NULL)!=0) 
  return 0;
 return ret;
}

sem_t *new_sem(int initial) {
 sem_t *ret;
 if((ret = (sem_t *)malloc(sizeof(sem_t)))<=0)
  return 0;
 if(sem_init(ret,0,initial)<0)
  return 0;
 return ret;
}

void *spinner(void *a) {
 unsigned int i,j;
 for(;;i++) {
  if(spin_timeout == 1) {
   errors = NOTHING;
   FATAL("<-timeout\n");
  }
  j = spin_go;
  if(j == 1) {
   spin_ack = 0;
   sem_wait(spin_done);
   putchar(spin_chars[i%4]);
   fflush(stdout); 
  }
  usleep(SPIN_DELAY);
  if(j == 1) {
   printf("\33[1D");
   sem_post(spin_done);
  }
  if(j == 0)
   spin_ack = 1;
 }
}

int find_cookie(char *data){
 int ret = 0;
 cookie_t *walker;
 if(cookies==0) return 0;
 
 walker = cookies;
 for(;;) {
  if(strncmp(data, walker->data, sizeof(walker->data))==0) return 1;
  if((int)(walker = (cookie_t *)walker->node.next) == (int)cookies) break;
 }
 return ret;
}

void add_cookie(char *data)  {
 int i;
 cookie_t *new;
 if(find_cookie(data)==1) return;
 if((new = (cookie_t *)malloc(sizeof(cookie_t)))<=0) {
  errors = NOMEM;
  FATAL(" ");
 }
 if(cookies == 0) {
  cookies = new;
  INIT_LIST_HEAD(&new->node);
 } else
  list_add(&new->node, &cookies->node);
 if(data !=0)
  for(i=0;;i++)
   if(data[i] == ';') {
    data[i] = 0;
    break;
   } 

 new->data = data;
}


char *moved_302(msg_t *msg_in) {
 char *moved_to = 0;
 head_t *walker;
 walker = msg_in->header;
 if(walker !=0) {
  for(;;) {
   if(walker->var !=0) {
    if(strncmp("Location", walker->var, strlen("Location"))==0)
     if(walker->value !=0) {
      moved_to = strdup(walker->value);
     }
   }
   if((int)(walker = (head_t *)walker->node.next) == (int)msg_in->header) break;
  }
 }
 return moved_to;
}

int print_header(msg_t *msg_in) {
 head_t *walker;
 printf("\n\n");
 walker = msg_in->header;
 if(walker !=0) {
  for(;;) {
   if(walker->var !=0) {
     if(walker->value !=0) {
       printf("%s:::%s\n", walker->var, walker->value);
     }
   }
   if((int)(walker = (head_t *)walker->node.next) == (int)msg_in->header) break;
  }
 }
}


int store_cookies(msg_t *msg_in) {
 int ret = 0;
 head_t *walker;
 walker = msg_in->header;
 if(walker !=0) {
  for(;;) {
   if(walker->var !=0) {
    if(strncmp("Set-Cookie", walker->var, strlen("Set-Cookie"))==0)
     if(walker->value !=0) {
       add_cookie(walker->value);
       ret = 1;
     }
   }
   if((int)(walker = (head_t *)walker->node.next) == (int)msg_in->header) break;
  }
 }
 return ret;
}

void msg_printf(char *format, ...) {
 int len;
 va_list ap;
 va_start(ap, format);
 len = vsnprintf(
   msg_buffer+msg_buf_i,MSG_BUFFER-msg_buf_i, format, ap);
 msg_buf_i+=len;
 msg_buffer[msg_buf_i+1] = 0;
 va_end(ap);
}

void body_printf(char *format, ...) {
 int len;
 va_list ap;
 va_start(ap, format);
 len = vsnprintf(
   body_msg+body_i,MSG_BUFFER-body_i, format, ap);
 body_i+=len;
 body_msg[body_i+1] = 0;
 va_end(ap);
}

void tack_body(void) {
 int i;
 for(i=0;i<body_i;i++)
  msg_buffer[msg_buf_i++] = body_msg[i];
}

void print_cookies(cookie_t *in) {
 cookie_t *walker;
 int i = 0;
 walker = in;
 
 if(walker !=0) {
  for(;;) {
   if(walker->data !=0) {
    if(++i == 1) msg_printf("Cookie: ");
    if(i!=1) msg_printf("; "); 
    msg_printf("%s", walker->data);

   }
   if((int)(walker = (cookie_t *)walker->node.next) == (int)in) break;
  }
  if(i !=0 )
   msg_printf("\r\n");
 }
}

int find_in(char *data, int len, char *match) {
 int i,j,k;
 int first = -1;
 k = strlen(match);
 j = 0;
 for(i=0;i<len;i++) {
  if(data[i] == match[j]) {
   if(first == -1)
    first = i;
   j++;
   if(j == k) return first;
  } else {
   j = 0;
   first = -1;
  }
 }
 return -1;
}

char *new_boundary(void) {
 char *ret;
 int i;
 ret = (char *)malloc(55);
 for(i=0;i<55;i++) { 
  if(i<27) 
   ret[i] = '-';
  else
   ret[i] = '0'+(random()%10);
 }
 ret[55] = 0;
 return ret;
}


void add_album(album_t **list, char *name, int id)  {
 album_t *new, *from;
 from = *list;
 if((new = (album_t *)malloc(sizeof(album_t)))<=0) {
  errors = NOMEM;
  FATAL(" ");
 }
 if(*list == 0) {
  *list = new;
  INIT_LIST_HEAD(&new->node);
 } else
  list_add(&new->node, &from->node);

 new->name = (char *)malloc(strlen(name)+1);
 memcpy(new->name, name, strlen(name)+1);
 new->id = id;
}

int album_getid(album_t *list, char *name) {
 album_t *walker;
 walker = list;
 if(walker !=0) {
  for(;;) {
   if(strncmp(name, walker->name, strlen(name))==0) break;
   if((int)(walker = (album_t *)walker->node.next) == (int)list) 
    return -1;
  }
 } else
  return -1;
 return walker->id;
}

void usage(void) {
 puts(
  "Usage: yesplz <arguments> \n"
  "\n"
  "** if ~/.yesplz.cfg doesn't exists at least -album -user and -title must\n"
  "** be specified. All arguments on the command-line overwrite what's in\n"
  "** the configuration file\n"
  "\n"
  " ARGUMENTS:\n"
  "\n"
  "-p             prompt for a password even if a stored password is\n"
  "               found"
  "\n"
  "-t <timeout>   this is the timeout in seconds for each operation,\n"
  "               the default is 20 seconds\n"
  "\n"
  "-up <timeout>  this is the timeout is seconds for the uploading part,\n"
  "               the default is 100 seconds\n"
  "\n"
  "-album         this is the album you wish to upload to, this must\n"
  "               be present\n"
  "\n"
  "-cat <name>    This is the category, if you have alternate categories\n"
  "               on your account. The default is \"no category\"\n"
  "\n"
  "-user <name>   this is the username of your account-- you need to\n"
  "               set-up an account on www.unixporn.ml first.\n"
  "\n"
  "-title <text>  this is the title of your screen-shot\n"
  "\n"
  "-desc  <text>  this is a description of your screen-shot (optional)\n"
  "\n"
  "-tag <text>    these are any tags you want to use (optional)\n"
  "\n"
  "-disp <name>   this is an alternate X display you want to take a\n"
  "               shot from, the default is what's in the environment\n"
  "               variable DISPLAY\n"
  "\n"
  "-qual <0-100>  This specifies the jpeg quality (upload size)\n"
  "\n"
  "-save          This saves any arguments you specify with this argument\n"
  "               as a configuration file in ~/.yesplz.cfg The next time\n"
  "               yesplz runs and if it finds that file it uses the arguments\n"
  "               you \"-save\"ed with as the defaults. For example, if you\n"
  "               type:\n"
  "\n"              
  "               $ yesplz -user sigflup -album \"nice album\" -save\n"
  "\n"
  "               from then on all you have to do is type:\n"
  "\n"
  "               $ yesplz -title \"I love pants\"\n"
  "\n"
  "               to upload a screen-shot\n"
  "\n"
  "-wait <time>  Wait a specified number of seconds before doing anything\n"
  "              this will give you a chance to make your desktop purty\n"
  "\n"
  "-dump <file>   dumps sessions to a file for use in debugging\n"
 );
 exit(0);
}

void load_config(void) {
 char path[MAXSTR];
 char *home;
 home = getenv("HOME");
 if(home<=0) {
  printf("$HOME not set, looking in current directory\n");
  snprintf(path, MAXSTR, ".yesplz.cfg");
 } else
  snprintf(path, MAXSTR, "%s/.yesplz.cfg", home);
 do_config(path);
}

void args(int argc, char **argv, int save) {
 int i;
 int found;
 FILE *config;
 char path[MAXSTR];
 char *home;
 // XXX do this better-- strncmp is a silly function 

 for(i=1;i<argc;i++) {
  found = 0;
  if(strncmp(argv[i], "-t", MAXSTR)==0) {
   i++;
   timeout_sec = atoi(argv[i]);
   found = 1;
  }
  if(found ==0) 
   if(strncmp(argv[i], "-up", MAXSTR)==0) {
    if(++i>argc) usage();
    uptimeout_sec = atoi(argv[i]);
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-album", MAXSTR)==0) {
    if(++i>argc) usage();
    album_name = argv[i];
    found = 1; 
   }
  if(found == 0)
   if(strncmp(argv[i], "-user", MAXSTR)==0) {
    if(++i>argc) usage();
    username = argv[i];
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-title", MAXSTR)==0) {
    if(++i>argc) usage();
    title = argv[i]; 
    found = 1; 
   }
  if(found == 0)
   if(strncmp(argv[i], "-desc", MAXSTR)==0) {
    if(++i>argc) usage();
    desc = argv[i];
    found = 1;
   } 
  if(found == 0)
   if(strncmp(argv[i], "-tag", MAXSTR)==0) {
    if(++i>argc) usage();
    tags = argv[i];
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-disp", MAXSTR)==0) {
    if(++i>argc) usage();
    disp = argv[i];
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-qual", MAXSTR)==0) {
    if(++i>argc) usage();
    jpeg_quality = atoi(argv[i]);
    if(jpeg_quality <2)
     jpeg_quality = 2;
    if(jpeg_quality >100)
     jpeg_quality = 100;
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-dump", MAXSTR)==0) {
    if(++i>argc) usage();
    if((dump_file = fopen(argv[i], "wb")) <=0) {
     perror(argv[i]);
     dump_file = (FILE *)0;
    }
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-p", MAXSTR)==0) {
    password = (char *)0;
    found = 1;
   }
  if(found == 0)
   if(strncmp(argv[i], "-cat", MAXSTR)==0) {
    if(++i>argc) usage();
    //cat_name = argv[i]; 
    found = 1; 
   }
  if(found == 0)
   if(strncmp(argv[i], "-wait", MAXSTR)==0) {
    if(++i>argc) usage();
    wait_sec = atoi(argv[i]); 
    found = 1; 
   }

  if(found == 0)
   if((strncmp(argv[i], "-help", MAXSTR)==0) ||
      (strncmp(argv[i], "--help",MAXSTR)==0)) {
    usage(); 
   }
  if(found == 0 && save==0) usage();
 }

 if((title == 0 ||
     album_name == 0 ||
    username == 0)&&save==0) usage();
 if(desc == 0) desc = nul;
 if(tags == 0) tags = nul;

 if(save == 1) {
  home = getenv("HOME");
  if(home<=0) {
   printf("$HOME not set, saving in current directory\n");
   snprintf(path, MAXSTR, ".yesplz.cfg");
  } else
   snprintf(path, MAXSTR, "%s/.yesplz.cfg", home);
  if((config = fopen(path, "wb"))<=0) 
   perror(path);
  else {
   if(album_name!=0) fprintf(config, "album = \"%s\"\n", album_name); 
   if(cat_name!=0) fprintf(config, "cat = \"%s\"\n", cat_name); 
   if(title!=0) fprintf(config, "title = \"%s\"\n", title); 
   if(desc!=nul) fprintf(config, "desc = \"%s\"\n", desc); 
   if(username!=0) fprintf(config, "user = \"%s\"\n", username); 
   if(timeout_sec!=20) fprintf(config, "timeout = %d\n", timeout_sec); 
   if(uptimeout_sec!=100) fprintf(config, "up_timeout = %d\n", uptimeout_sec); 
   if(jpeg_quality!=100) fprintf(config, "qual = %d\n", jpeg_quality);
   if(wait_sec!=0) fprintf(config, "wait = %d\n", wait_sec);

   fclose(config);
   exit(0);
  } 
 }
}

#define PASSWORD_BUFFER		128

char *new_getpass(char *name) {
 char *ret;
 char data;
 unsigned int done, pos = 0;
 struct termios ios;
 tcflag_t old_lflag;
 tcgetattr(fileno(stdin), &ios);
 old_lflag = ios.c_lflag;
 ios.c_lflag &= ~ICANON;
 ios.c_lflag &= ~ECHO;
 tcsetattr(fileno(stdin), TCSANOW, &ios);

 ret = (char *)malloc(PASSWORD_BUFFER);
 *ret = 0;
 printf("%s's password (0_o):", name);
 fflush(stdout);
 for(done=0;done!=1;) 
  while(read(fileno(stdin), &data, 1) !=0) {
   if(data == '\n') {done = 1; break; }
   if(pos<PASSWORD_BUFFER)
    ret[pos++] = data;
   if(pos != PASSWORD_BUFFER)
    printf("\33[5D%s):", ((pos&1)==1 ? "o_O" : "O_o"));
   else
    printf("\33[5DO_O):");
   fflush(stdout);
  } 

 ret[pos] = 0;

 ios.c_lflag = old_lflag;
 tcsetattr(fileno(stdin), TCSANOW, &ios);
 return ret;
}



int main(int argc, char **argv ) {
 int jpeg_size;
 int i,j;
 unsigned char *jpeg_data;
 msg_t *msg;
 char *str;
 char filename[FILENAME_LEN];

 char *jmp_location;
 struct timeval time;
 struct timezone timezone;

/* if(argc == 0) return 0;
 working_key = concr_getkey(&yesplz_method, concr_guessname(argv[0]));
*/

 j = 0;
 for(i=1;i<argc;i++) 
  if(strncmp(argv[i], "-save", strlen("-save"))==0) {
   j = 1; 
   break;
  }

 if(j == 0) load_config();
 args(argc, argv, j);

 alarm(0);
 signal(SIGALRM, timeout);

 if(password == (char *)0)
  password = new_getpass(username);

 if((spin_thread = new_thread(spinner))!=0)
  spin_done = new_sem(1);
 else  {
  printf("can't get a threads\n");
  exit(0);
 }

 while(wait_sec != 0) {
  printf("\r\33[Kand in %d...", wait_sec--);
  fflush(stdout);
  sleep(1);
 }

 if(gettimeofday(&time, &timezone)!=-1)
  srand((unsigned int)(time.tv_sec ^ time.tv_usec));

 alarm(timeout_sec);
 REPORT("\r\33[Kshot");
 if((dpy = XOpenDisplay(disp))<=0) 
  FATAL("<-failed\n"); 

 screen = XDefaultScreen(dpy);
 target = RootWindow(dpy,screen);
 jpeg_data = shot(dpy, screen, target, &jpeg_size, jpeg_quality);
 XCloseDisplay(dpy);
 FINISH(" ");

 alarm(timeout_sec);
 REPORT("\r\33[Kdelicious front page cookie");
 START;
 msg_printf(DELICIOUS_COOKIES);
 if((msg = ask("www.unixporn.ml", 80, HEAD)) == 0)
  FATAL("<-failed\n");
 if(store_cookies(msg) == 0) 
  FATAL("<-No cookies\n");
// free_msg(msg);
 FINISH(" "); 


 alarm(timeout_sec); 
 REPORT("\r\33[Kdelicious login cookie");
 START;
 msg_printf(LOGIN_A);
 if((msg = ask("www.unixporn.ml",80, BODY))==0)
  FATAL("<-failed\n");

 jmp_location = moved_302(msg);
 START;

 msg_printf("GET /%s HTTP/1.1\r\n"
            "Host: www.unixporn.ml\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Accept-Language: en-us,en;q=0.5\r\n"
            "Accept-Encoding: deflate\r\n"
            "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Connection: keep-alive\r\n", jmp_location);

 print_cookies(cookies);

 msg_printf("\r\n");

 if((msg = ask("www.unixporn.ml",80, HEAD))==0)
  FATAL("<-failed\n");
 
 if(store_cookies(msg) == 0) 
  FATAL("<-No cookies\n");

 FINISH(" ");
 

 alarm(timeout_sec);
 REPORT("\r\33[Klogin");
 START;
 msg_printf(LOGIN);
 print_cookies(cookies);

 body_printf("username=%s&password=%s&submitted=OK", username,password); 

 msg_printf(
   "Content-Type: application/x-www-form-urlencoded\r\n"
   "Content-Length: %d\r\n"
   "\r\n", body_i);
 tack_body();

 if((msg = ask("www.unixporn.ml", 80, BODY))==0)
  FATAL("<-failed\n");

 if( find_in(msg->data, msg->len, "Try again")>-1)
  FATAL("<-Couldn't login\n");  
 FINISH(" "); 


 alarm(timeout_sec);
 REPORT("\r\33[Kalbum list");
 START;
 msg_printf(ALBUM_B);
 print_cookies(cookies);
 msg_printf("\r\n\r\n");
 if((msg = ask("www.unixporn.ml", 80, BODY))==0)
  FATAL("<-failed\n");

 album_id = find_album_id(msg);

 if(album_id!=-1) {
  START;
  msg_printf(
   "GET /albmgr.php?cat=%d HTTP/1.1\r\n"
   "Host: www.unixporn.ml\r\n"
   "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
   "Accept-Language: en-us,en;q=0.5\r\n"
   "Accept-Encoding: deflate\r\n"
   "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
   "Connection: keep-alive\r\n", album_id);
  print_cookies(cookies);
  msg_printf("\r\n\r\n");
  if((msg = ask("www.unixporn.ml", 80, BODY))==0)
   FATAL("<-failed\n");
 }

 if(find_albums(msg) == 0) {
  errors = -1;
  FATAL("<-failed\n");
 }

 if((album_id = album_getid(albums, album_name))== -1)  {
  errors = -1;
  FATAL("<-couldn't find album\n");
 }

 FINISH(" ");

 alarm(timeout_sec);
 REPORT("\r\33[Kforum token and timestamp");
 START;

 msg_printf(UPLOAD_TOKEN);
 print_cookies(cookies);
 msg_printf("\r\n\r\n");

 if((msg = ask("www.unixporn.ml", 80, BODY))==0) 
  FATAL("<-failed\n");

 get_tokens(msg);

 FINISH(" ");


 alarm(uptimeout_sec);
 REPORT("\r\33[Kupload");
 START;
 msg_printf(UPLOAD_A);
 print_cookies(cookies);
 str = new_boundary();
 msg_printf(
   "Content-Type: multipart/form-data; boundary=%s\r\n", str);

 body_printf("--%s\r\n", str);
 body_printf("Content-Disposition: form-data; name=\"method\"\r\n"
             "\r\n"
	     "html_single\r\n");

 body_printf("--%s\r\n", str);
 body_printf("Content-Disposition: form-data; name=\"album\"\r\n"
             "\r\n"
	     "%d\r\n", album_id);


 for(i=0;i<RANDOM_LEN;i++) 
  filename[i] = 'a'+ (rand()%('z'-'a'));
 
 snprintf(&filename[RANDOM_LEN], FILENAME_LEN-RANDOM_LEN, ".jpg");

 body_printf("--%s\r\n", str);
 body_printf(
  "Content-Disposition: form-data; name=\"userpicture\"; filename=\"%s\"\r\n"
  "Content-Type: image/jpeg\r\n\r\n"
   , filename);
 for(i=0;i<jpeg_size;i++)
  body_msg[body_i++] = jpeg_data[i];

 body_printf("\r\n--%s\r\n", str);
 body_printf(
   "Content-Disposition: form-data; name=\"title\"\r\n\r\n%s\r\n",title); 

 body_printf("--%s\r\n", str);
 body_printf(
   "Content-Disposition: form-data; name=\"caption\"\r\n\r\n%s\r\n",desc); 

 body_printf("--%s\r\n", str);
 body_printf(
   "Content-Disposition: form-data; name=\"keywords\"\r\n\r\n\r\n"); 

 body_printf("--%s\r\n", str);
 body_printf(
   "Content-Disposition: form-data; name=\"event\"\r\n\r\npicture\r\n"); 

 body_printf("--%s\r\n", str);
 body_printf(
   "Content-Disposition: form-data; name=\"form_token\"\r\n\r\n%s\r\n",
   hidden_token); 

 body_printf("--%s\r\n", str);
 body_printf(
   "Content-Disposition: form-data; name=\"timestamp\"\r\n\r\n%s\r\n",
   hidden_timestamp); 

 body_printf("--%s--\r\n", str);
 
 msg_printf("Content-Length: %d\r\n\r\n", body_i);
 
 tack_body(); 

 if((msg = ask("www.unixporn.ml",80, UPLOAD))==0)
  FATAL("<-failed\n");

 free(str);

 if( find_in(msg->data, msg->len, "Your file was successfully added")==0) 
  FATAL("<-upload failed\n");  

 FINISH(" ");

 alarm(timeout_sec);
 REPORT("\r\33[Klogout");
 START;
 msg_printf(LOGOUT);
 print_cookies(cookies);
 msg_printf("\r\n");
 if(ask("www.unixporn.ml",80, HEAD)==0)
  FATAL("<-failed\n");
 free_msg(msg);
 FINISH("\r\33[K^>'_'<^\n");

 return 0;
}
