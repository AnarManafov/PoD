#! /bin/bash


freeport () {
perl -e '
    use IO::Socket;
    my $port = $ARGV[0];
    for (; $port < $ARGV[1]; $port++) {
        $fh = IO::Socket::INET->new( Proto     => "tcp",
                                     LocalPort => $port,
                                     Listen    => SOMAXCONN,
                                     Reuse     => 0);
        if ($fh){
            $fh->close();
            print "$port";
            exit(0);
        }


    }
    die "%Error: Cant find free socket port\n";
    exit(1);
' $1 $2
}

FREE_XRD_PORT=`freeport 20000 22000`
FREE_XPROOF_PORT=`freeport 22001 25000`
echo "xrd port: $FREE_XRD_PORT; xproof port:$FREE_XPROOF_PORT"



exit 0