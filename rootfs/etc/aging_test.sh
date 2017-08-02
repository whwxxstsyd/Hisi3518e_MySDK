#!/bin/sh
source /mnt/sdcard/aging_test_config.ini
echo $TIME
VAR=$TIME 
while [ 1 ]
do         
  
        gpio_cmd w 7 5 1
        sleep 1;        
        gpio_cmd w 7 5 0
        sleep 1;        
        gpio_cmd w 7 5 1
        sleep 1;        
                
        echo $VAR "tetsssssssssssssssssssssss!!!"
        cat /mnt/sdcard/wanna_run.wav > /tmp/g711       
        sleep 3;                          
                
        VAR=`expr $VAR - 1`;
        if [ $VAR -eq 0 ]; then
                echo "OVER!!"  
                return ;     
        fi              
done 


