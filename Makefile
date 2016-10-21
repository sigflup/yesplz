CC= gcc 
LEX= lex

PREFIX=/usr
CFLAGS =    -I/usr/include -I/usr/include/X11/  -I/usr/include -I/usr/include//  -I/usr/include -I/usr/include// -w -DPACKAGE_NAME=\"yesplz\" -DPACKAGE_TARNAME=\"yesplz\" -DPACKAGE_VERSION=\"dec_19_2011\" -DPACKAGE_STRING=\"yesplz\ dec_19_2011\" -DPACKAGE_BUGREPORT=\"\" 
LDFLAGS =    
LIBS = -lpthread -ljpeg -lX11  -lX11 -ljpeg -lpthread

OBJS=main.o shot.o list.o multiVis.o clientwin.o lex.yy.o 

yesplz: ${OBJS}
	${CC} -o yesplz ${OBJS} ${LDFLAGS} ${LIBS}

%.o: %.c
	${CC} -c ${CFLAGS} -o $@ $<

lex.yy.c: lexxer.l
	${LEX} lexxer.l

install: yesplz
	chmod +x ./install.sh
	./install.sh yesplz ${PREFIX}/bin/yesplz

uninstall:
	rm -f ${PREFIX}/bin/yesplz


clean:
	rm -f ${OBJS} yesplz lex.yy.c
