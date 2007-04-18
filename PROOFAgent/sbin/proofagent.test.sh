#!/bin/bash

# TODO: ADD HEADER

read3 ()                # Usage: read3
{
    read -r -u3              # read from fd=3
}

read4 ()
{
    read -r -u4   
}

send3 ()                # Usage: send3 [cmd...]
{
    echo "$*" 1>&3            # write to fd=3
}

send4 ()                # Usage: send4 [cmd...]
{
    echo "$*" 1>&4            # write to fd=4
}

end_session ()
{
    # shuting proofaget down
    /home/anar/PROOFAgent/bin/proofagent -d  --instance server --pidfile /tmp/ --stop
    /home/anar/PROOFAgent/bin/proofagent -d  --instance client1 --pidfile /tmp/ --stop
    
    # close file descriptors in both ways
    exec 3<&-
    exec 3>&-
    exec 4<&-
    exec 4>&-
    [[ "$*" == "OK" ]] || exit 1
    exit 0
}

echo "Starting PROOFAgent Server:"
/home/anar/PROOFAgent/bin/proofagent -d --config /home/anar/PROOFAgent/etc/proofagent.cfg.xml  --instance server --pidfile /tmp/

# Let's give PROOFAgent a chance to properly start
# TODO: Should be fixed. We don't need to sleep!
sleep 5


echo "Starting PROOFAgent Client: "
/home/anar/PROOFAgent/bin/proofagent -d --config /home/anar/PROOFAgent/etc/proofagent.cfg.xml  --instance client1 --pidfile /tmp/

# Let's give PROOFAgent a chance to properly start
# TODO: Should be fixed. We don't need to sleep!
sleep 5


echo "processing socket tests..."
exec 3<>/dev/tcp/127.0.0.1/20001
RET_VAL=$?
if [ "X$RET_VAL" = "X0" ]; then
  echo "Connection successful. Exit code: $RET_VAL"
  
else
  echo "Connection unsuccessful. Exit code: $RET_VAL"
  end_session "ERROR";
fi

exec 4<>/dev/tcp/127.0.0.1/51511
RET_VAL=$?
if [ "X$RET_VAL" = "X0" ]; then
  echo "Connection successful. Exit code: $RET_VAL"
  
else
  echo "Connection unsuccessful. Exit code: $RET_VAL"
  end_session "ERROR";
fi


: <<COMMENT
EOF
count=0
limit_count=20
exec 3<> /dev/tcp/127.0.0.1/20001
while [ "$*" != "0" ]
do
	sleep 2
	exec 3<> /dev/tcp/127.0.0.1/20001
	count=`expr $count + 1`
	[[ "$count" -lt "$limit_count" ]] || end_session "ERROR";
done

count=0
exec 4<> /dev/tcp/127.0.0.1/51511
while [ "$*" != "0" ]
do
        sleep 2
	exec 4<> /dev/tcp/127.0.0.1/51511
	count=`expr $count + 1`
        [[ "$count" -lt "$limit_count" ]] || end_session "ERROR";
done
COMMENT

count=0
limit_count=1000
while [ "$count" -lt "$limit_count" ]
do
  MSG_SERVER="HELLO TEST from SERVER: "$count
  MSG_CLIENT="HELLO TEST from CLIENT: "$count

  send3 $MSG_SERVER;
  read4
  echo "$* --> $REPLY"
  
  [[ $REPLY == $MSG_SERVER ]]  || end_session "ERROR";
  
  
  send4 $MSG_CLIENT;
  read3
  echo "$* --> $REPLY"
  [[ $REPLY == $MSG_CLIENT ]]  || end_session "ERROR";
  count=`expr $count + 1`
done

end_session "OK";
