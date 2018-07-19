#!/bin/sh

#config=/usr/app/conf.default
config=/config/cgminer.conf
bw_pool1url=
bw_pool1user=
bw_pool1pw=
bw_pool2url=
bw_pool2user=
bw_pool2pw=
bw_pool3url=
bw_pool3user=
bw_pool3pw=
bw_usefrequencyAll=
bw_freq=
bw_freq1=
bw_freq2=
bw_freq3=
bw_freq4=
bw_periodic_time=
bw_position=
bw_fan1_speed=
bw_fan2_speed=
bw_temperature_threshould=
bw_mhs_threshould=
bw_number=

case $1 in 
         0) 
           echo "cfg pool1"
	   bw_pool1url=$2;
           bw_pool1user=$3;
           bw_pool1pw=$4;	   
	   sed -i "s_\"pool1\":\".*\"_\"pool1\":\"${bw_pool1url//_/\\_}|${bw_pool1user}|${bw_pool1pw}\"_" ${config}
         ;; 
         1) 
           echo "cfg pool2"
	   bw_pool2url=$2;
           bw_pool2user=$3;
           bw_pool2pw=$4;	   
	   sed -i "s_\"pool2\":\".*\"_\"pool2\":\"${bw_pool2url//_/\\_}|${bw_pool2user}|${bw_pool2pw}\"_" ${config}
         ;; 
         2) 
           echo "cfg pool3"
	   bw_pool3url=$2;
           bw_pool3user=$3;
           bw_pool3pw=$4;	   
	   sed -i "s_\"pool3\":\".*\"_\"pool3\":\"${bw_pool3url//_/\\_}|${bw_pool3user}|${bw_pool3pw}\"_" ${config}
         ;; 
	 3)
	   echo "cfg usefrequencyAll"
	   bw_usefrequencyAll=$2;
	   sed -i "s_\"usefrequencyAll\":\(true\|false\)_\"usefrequencyAll\":${bw_usefrequencyAll}_" ${config}
       ;;
	 4)
	   echo "cfg frequency"
	   bw_freq=$2;
	   sed -i "s_\"frequency\":\".*\"_\"frequency\":\"${bw_freq}\"_" ${config}
         ;;
	 5)
	   echo "cfg frequency1"
	   bw_freq1=$2;
	   sed -i "s_\"frequency1\":\".*\"_\"frequency1\":\"${bw_freq1}\"_" ${config}
         ;;
	 6)
	   echo "cfg frequency2"
	   bw_freq2=$2;
	   sed -i "s_\"frequency2\":\".*\"_\"frequency2\":\"${bw_freq2}\"_" ${config}
         ;;
	 7)
	   echo "cfg frequency3"
	   bw_freq3=$2;
	   sed -i "s_\"frequency3\":\".*\"_\"frequency3\":\"${bw_freq3}\"_" ${config}
         ;;
	 8)
	   echo "cfg frequency4"
	   bw_freq4=$2;
	   sed -i "s_\"frequency4\":\".*\"_\"frequency4\":\"${bw_freq4}\"_" ${config}
         ;;
         9) 
           echo "cfg periodic_time"
	   bw_periodic_time=$2;	   
	   sed -i "s_\"periodic\_time\":\".*\"_\"periodic\_time\":\"${bw_periodic_time}\"_" ${config}
         ;; 
         10) 
           echo "cfg position"
	   bw_position=$2;	   
	   sed -i "s_\"position\":\".*\"_\"position\":\"${bw_position}\"_" ${config}
         ;; 
	 11)
	   echo "cfg fan1 speed"
	   bw_fan1_speed=$2;
	   sed -i "s_\"fan1\_speed\":\".*\"_\"fan1\_speed\":\"${bw_fan1_speed}\"_" ${config}
         ;;
	 12)
	   echo "cfg fan2 speed"
	   bw_fan2_speed=$2;
	   sed -i "s_\"fan2\_speed\":\".*\"_\"fan2\_speed\":\"${bw_fan2_speed}\"_" ${config}
         ;;
	 13)
	   echo "cfg temperature_threshould"
	   bw_temperature_threshould=$2;
	   sed -i "s_\"temperature\_threshould\":\".*\"_\"temperature\_threshould\":\"${bw_temperature_threshould}\"_" ${config}
         ;;
	 14)
	   echo "cfg mhs_threshould"
	   bw_mhs_threshould=$2;
	   sed -i "s_\"mhs\_threshould\":\".*\"_\"mhs\_threshould\":\"${bw_mhs_threshould}\"_" ${config}
         ;;
     15)
       echo "cfg number"
       bw_number=$2;
       sed -i "s_\"number\":\".*\"_\"number\":\"${bw_number}\"_" ${config}
       ;;
     *)
         echo "cfg others"
         ;; 
esac 
