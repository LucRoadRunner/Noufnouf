<?php

/* Get the port for the WWW service. */
$service_port = 5678;
$command=$_GET['command'];
#echo "got command ".$command.'<BR>';

$address = gethostbyname('skywalker');

$socket=socket_create(AF_INET, SOCK_DGRAM, 0);

$result = socket_connect($socket, $address, $service_port);

$n=socket_sendto($socket, $command, 1, 0, "$address", $service_port);

socket_close($socket);
?>
