#!/bin/bash
#===============================================================================
#
#          FILE:  teste.sh
# 
#         USAGE:  ./teste.sh 
# 
#   DESCRIPTION:  
# 
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR:   (), 
#       COMPANY:  
#       VERSION:  1.0
#       CREATED:  07/29/19 11:21:36 -03
#      REVISION:  ---
#===============================================================================

for i in {0..29..1}
do
	cat "APD_run_$i.plt" >> APDmean
done
