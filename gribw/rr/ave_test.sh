#!/bin/sh


yymm=199507


for hr in 00 03 06 09 12 15 18 21
do
   ./mk_ave_eta_fast /usr3/ebis/RR.$yymm.${hr}Z 24 \
       /var/ftp/pub/regional_reanl/analyses/merged_AWIP32/$yymm/AWIP3200.${yymm}??$hr.merged
done


./mk_ave_eta_fast /usr3/ebis/RR.$yymm 3 /usr3/ebis/RR.$yymm.*Z

