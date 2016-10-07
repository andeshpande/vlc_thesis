#!/bin/bash
if [ $# -eq 1 ]
then
	scp /home/aniruddha/Dropbox/thesis/src/adcval/*.[ch] $1:/home/openvlc/adcval
	scp /home/aniruddha/Dropbox/thesis/src/adcval/Makefile $1:/home/openvlc/adcval
fi
