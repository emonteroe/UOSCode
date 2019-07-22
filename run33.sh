#!/bin/bash
#===============================================================================
#
#          FILE:  run33.sh
# 
#         USAGE:  ./run33.sh 
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
#       CREATED:  07/22/19 16:02:47 -03
#      REVISION:  ---
#===============================================================================

for i in {0..32..1}
do
	./waf --run "scratch/UOS-LTE-No-UABS" >> Simulations_Without_UABS
done
