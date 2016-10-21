/*
-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

                                                    __
                                                  `    `.
                                                 /      |
                                                        | 
                                                 `     .
                                     _            `.  . 
                ........---       .`   `.      ..-`   `..._
                |          |     (      |     `            `
                .          |      )     /    |              |
                |  .-`w`   |     <       --..              / 
                | ` MMML.  |      \ /                     /
                |  . MMML  .       ` /                   /
                ._..  MMM``.--..  .-`                   /
                    `  MMI`-```--`                     ( 
                     . . \.       __                    \
                      `    ---````  L....................
                U B E R  L E E T  H A C K E R  F O R C E
 
           This is official Uber Leet Hacker Force source code
	    This is free software, read LICENSE for details
 
 http://pgp.mit.edu:11371/pks/lookup?op=get&search=0x34964E67B47CF3DB
*/ 
typedef struct {
 struct list_head node;
 char *var, *value;
} head_t;

typedef struct {
 head_t *header;
 unsigned char *data;
 int len; 
} msg_t;

extern int just_head;

#define BODY	0
#define HEAD	1
#define UPLOAD	2

/* big enough for a jpeg */
#define MSG_BUFFER	0x2ffffff

extern unsigned int msg_buf_i;
extern char msg_buffer[MSG_BUFFER];


void get_tokens(msg_t *msg);
int find_cats(msg_t *msg);
msg_t *ask(char *name, u_int16_t port, int just_head);
void free_msg(msg_t *msg);
char *get_var(msg_t *msg, char *var);
void dump_vars(msg_t *msg);
void do_config(char *filename);
int find_album_id(msg_t *msg);
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.10 (OpenBSD)

iQEcBAEBAgAGBQJLvx0XAAoJEDC1QXa8JnD6TVUH/jlbE6cx9lXf6ZfdJuMEY8aU
OHo0t4RlCW4/ZnWnW93nGPk2CGYbMv2a/Zyhqad650JTYG/JVjh2o0ngW5nuAl4P
orxr/OEJTR7CVHN13+oc+Pe4pP33PX7eodzlq2MAoFIjGfu8nDG4pT0Xcl4yGQ//
y4uwsMVi8sUX5N+Knp7ehqECQSVwXK5YaIF0dxnaa2tFFi/0OxDZGXbi2G8RtCWq
HF+zR6HGKZx3Iw9Qit3a+RoQWr/gMtaS2VGRRO1aCiDrsw80tBhGhEQMmtCS1ugd
xiOx1/1wdQXr66+pP0NxBJvVek5zsDeXMWE7elGaY/CsLb0D8+Ew7yDXOXiBOFg=
=tq6O
-----END PGP SIGNATURE-----
*/
