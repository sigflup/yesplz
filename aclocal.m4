dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.


# 1 - library
# 2 - function
# 3 - error

AC_DEFUN(HRR_CHECK_LIB, [
 HRR_SWLIB=""
 HRR_SWDIR=""
LIB_SEARCH="/lib \
	    /lib \
            /usr/lib \
            /usr/unsupported/lib \
            /usr/local/lib \
            /usr/X386/lib \
            /usr/lib/X11R4 \
            /usr/X11R5/lib \
            /usr/lib/X11R5 \
            /usr/openwin/lib \
            /usr/X11/lib \
            /pub/X11R5/lib \
            /usr/local/X11R5/lib \
            /opt/lib \
            /opt/X11R6/lib \
            /opt/X11/lib \
            /usr/X11R6/lib \
            /usr/lib/X11R6 \
            /pub/X11R6/lib \
            /pub/lib \
            /usr/local/X11R6/lib \
            $HOME/local/lib "


 AC_SEARCH_LIBS($2,$1, HRR_SWLIB=-l$1, HRR_SWLIB="nope")
 if test "$HRR_SWLIB" = "nope"; then
  printf "searching for %s..." $1

  for i in $LIB_SEARCH ; do
   if test -r $i/lib$1.a; then
    HRR_SWDIR=-L$i
    HRR_SWLIB=-l$1
   fi
  done

  if test "$HRR_SWLIB" = "nope" ; then
   echo
   echo "****************"
   echo "$1" not found
   exit
  else
   echo "found"
  fi
 fi

 LDFLAGS=$LDFLAGS\ $HRR_SWDIR
 LIBS=$LIBS\ $HRR_SWLIB

])


# 1 - header_file
# 2 - sub directory  e.g. SDL/
# 3 - error

AC_DEFUN(HRR_CHECK_HEAD, [
HEADER_SEARCH="/include \
               /usr/include \
               /usr/unsupported/include \
               /usr/local/include \
               /usr/X386/include \
               /usr/include/X11R4 \
               /usr/X11R5/include \
               /usr/include/X11R5 \
               /usr/openwin/include \
               /usr/X11/include \
               /pub/X11R5/include \
               /usr/local/X11R5/include \
               /opt/include \
               /opt/X11R6/include \
               /opt/X11/include \
               /usr/X11R6/include \
               /usr/include/X11R6 \
               /pub/X11R6/include \
               /pub/include \
               /usr/local/X11R6/include \
               $HOME/local/include "


 printf "checking for %s... " $1
 HRR_INC="nope"
 AC_TRY_CPP([#include <$2$1>], , HRR_INC="nope")
 if test "$HRR_INC" = "nope"; then
  for i in $HEADER_SEARCH; do
   if test -r $i/$2$1; then
    HRR_INC=\ -I$i\ -I$i/$2
    break;
   fi
  done
 fi
if test "$HRR_INC" = "nope"; then
  echo " "
  echo "****************"
  echo "$1" not found
  exit;
else
  echo "found"
  CFLAGS=$CFLAGS\ $HRR_INC
  CPPFLAGS=$CPPFLAGS\ $HRR_INC
fi

])

