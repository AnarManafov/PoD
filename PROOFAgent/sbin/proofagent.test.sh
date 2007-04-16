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
    /home/anar/PROOFAgent/bin/proofagent  --instance server --pidfile /tmp/ --stop
    /home/anar/PROOFAgent/bin/proofagent  --instance client1 --pidfile /tmp/ --stop
    
      # close file descriptors
    exec 3<&-
    exec 4<&-
    
    [[ "$*" == "OK" ]] || exit 1
    exit 0
}

echo "Starting PROOFAgent Server:"
/home/anar/PROOFAgent/bin/proofagent --config /home/anar/PROOFAgent/etc/proofagent.cfg.xml  --instance server --pidfile /tmp/

echo "Starting PROOFAgent Client: "
/home/anar/PROOFAgent/bin/proofagent --config /home/anar/PROOFAgent/etc/proofagent.cfg.xml  --instance client1 --pidfile /tmp/

echo "processing socket tests..."
exec 3<>/dev/tcp/127.0.0.1/20001 || end_session "ERROR";
exec 4<>/dev/tcp/127.0.0.1/51511 || end_session "ERROR";


send3 "HELLO TEST from SERVER";
read4
echo "$* --> $REPLY"

[[ $REPLY == "HELLO TEST from SERVER" ]]  || end_session "ERROR";


send4 "HELLO TEST from CLIENT";
read3
echo "$* --> $REPLY"
[[ $REPLY == "HELLO TEST from CLIENT" ]]  || end_session "ERROR";


end_session "OK";
