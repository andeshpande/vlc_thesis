#!/bin/bash
dir=
if [ $# -eq 1 ]
then
	scp $(pwd)/*.[ch] $1:/home/openvlc/$(basename $(pwd))
fi
