#!/bin/sh
#
# Copyright (C)  Freescale Semiconductor, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
#  NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

usage()
{
	echo "usage: genassym [-o outfile] objfile"
	exit 1
}

outfile=/dev/stdout
while getopts "o:" option
do
	case "$option" in
	o)	outfile="$OPTARG";;
	*)	usage;;
	esac
done
shift $(($OPTIND - 1))
case $# in
1)	;;
*)	usage;;
esac

${NM:='nm'} "$1" | ${AWK:='awk'} '
/ C .*sign$/ {
	sign = substr($1, length($1) - 3, 4)
	sub("^0*", "", sign)
	if (sign != "")
		sign = "-"
}
/ C .*w0$/ {
	w0 = substr($1, length($1) - 3, 4)
}
/ C .*w1$/ {
	w1 = substr($1, length($1) - 3, 4)
}
/ C .*w2$/ {
	w2 = substr($1, length($1) - 3, 4)
}
/ C .*w3$/ {
	w3 = substr($1, length($1) - 3, 4)
	w = w3 w2 w1 w0
	sub("^0*", "", w)
	if (w == "")
		w = "0"
	sub("w3$", "", $3)
	# This still has minor problems representing INT_MIN, etc.  E.g.,
	# with 32-bit 2''s complement ints, this prints -0x80000000, which 
	# has the wrong type (unsigned int).
	printf("#define\t%s\t%s0x%s\n", $3, sign, w)
}
' 3>"$outfile" >&3 3>&-
