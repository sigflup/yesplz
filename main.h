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

#define CHECK_ERRNO	-2
#define NOTHING	-1
#define WINATTR	0
#define TRANS	1
#define X11IMAGE 2
#define NOMEM 3
#define LADDR	4
#define STR	5

#define MAXSTR		1024
#define ERRORS_STR	90

typedef struct {
 struct list_head node;
 char *name;
 int id;
} album_t;

extern album_t *albums;
extern FILE *dump_file;

extern char hidden_token[MAXSTR];
extern char hidden_timestamp[MAXSTR];

extern int errors;
extern char errors_str[ERRORS_STR];

extern sem_t *spin_done;
extern pthread_t *spin_thread;
extern int spin_go, spin_ack;

extern int wait_sec;
extern char nul[];
extern char *album_name;
extern char *cat_name;
extern char *title;
extern char *desc;
extern char *tags;
extern char *username;
extern char *disp;
extern char *password;
extern int timeout_sec;
extern int uptimeout_sec;
extern int upload_id;
extern int album_id;
extern int jpeg_quality;



void add_album(album_t **list, char *name, int id);
void choose_album(void);
/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.10 (OpenBSD)

iQEcBAEBAgAGBQJLvxzxAAoJEDC1QXa8JnD6/NYIAOdoI2EG9ocpCEaocLakt9Qa
slDMO8pyWzfMPbO9gGnPVPfmJwmWmelVYDBHx9M28dZwZICxvh+Fblfw2v+m53W1
JJ2umx3bEbPnCQpyt6KPcrmLEkWJK95l3Px/DfWp680ORmGH5u/UJXNv5ubvXMM2
eAOsLZao3g7503bWSk/o+4WnhZWtHBQs5jfuItC/VoRaYqY0VnnAOrdGNS/shhJS
D/PcvUH0+z0RMh8w5eyv6z8kCwI5oiznUlbw9QLtDCqsPqTuMZtmXgkbFInjyqwL
Q9e7RkLJVqBoNydZIbu86T+BxALfRhwWh0+I9PT1N2GyAEE34LSXISOhObmiLCU=
=ZAmw
-----END PGP SIGNATURE-----
*/
