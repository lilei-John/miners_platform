#!/bin/sh

# minerclient
Release='false'

if [ $Release == 'true' ];then
CheckAppProcess()
{
  if [ "$1" = "" ];
  then
    return 1
  fi

  PROCESS_NUM=`ps -ef | grep "$1" | grep -v "grep" | wc -l` 
  if [ $PROCESS_NUM -eq 1 ];
  then
    return 0
  else
    return 1
  fi
}

while [ 1 ] ; do
 CheckAppProcess "minerclient"
 Check_Ret=$?
 if [ $Check_Ret -eq 1 ];then
 killall -9 minerclient
 exec /usr/app/minerclient &
 fi
 sleep 3
done

else
    exec /usr/app/minerclient &
fi
