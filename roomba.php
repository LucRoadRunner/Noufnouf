<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="style.css" type="text/css">
<title></title>
    
<script>
var ptz_type=0;
var PTZ_STOP='%20';
var PTZ_SAFE='a';
var PTZ_START='s';
var PTZ_DOCK='d';
var PTZ_LED='l';
var TILT_UP='i';
var TILT_DOWN=',';
var TILT_LEFT='j';
var TILT_ULEFT='u';
var TILT_DLEFT='n';
var TILT_RIGHT='k';
var TILT_URIGHT='o';
var TILT_DRIGHT=';';
function decoder_control(command)
{
	hal.location='/bikini/roomba_client.php?command='+command;
}
function stop()
{
	decoder_control(PTZ_STOP);
}
function up() 
{
	decoder_control(TILT_UP);
}
function down() 
{
	decoder_control(TILT_DOWN);
}
function left() 
{
	decoder_control(TILT_LEFT);
}

function uleft()
{
        decoder_control(TILT_ULEFT);
}

function dleft()
{
        decoder_control(TILT_DLEFT);
}


function right() 
{
	decoder_control(TILT_RIGHT);
}


function uright()
{
        decoder_control(TILT_URIGHT);
}


function dright()

{
        decoder_control(TILT_DRIGHT);
}




function key(e)
{
  var keynum;

    if(window.event) { // IE                    
      keynum = e.keyCode;
    } else if(e.which){ // Netscape/Firefox/Opera                   
      keynum = e.which;
    }
    //alert(keynum);
    switch(keynum) {
	case 38: decoder_control(TILT_UP);break;
	case 73: decoder_control(TILT_UP);break;
	case 37: decoder_control(TILT_LEFT);break;
	case 74: decoder_control(TILT_LEFT);break;
	case 85: decoder_control(TILT_ULEFT);break;
	case 78: decoder_control(TILT_DLEFT);break;
	case 40: decoder_control(TILT_DOWN);break;
	case 188: decoder_control(TILT_DOWN);break;
	case 39: decoder_control(TILT_RIGHT);break;
	case 75: decoder_control(TILT_RIGHT);break;
	case 79: decoder_control(TILT_URIGHT);break;
	case 190: decoder_control(TILT_DRIGHT);break;
	case 32: decoder_control(PTZ_STOP);break;
	case 83: decoder_control(PTZ_START),decoder_control(PTZ_SAFE);break;
	case 76: decoder_control(PTZ_LED);break;
	case 68: decoder_control(PTZ_DOCK);break;
	default: break;
    }
}
 

function body_onload()
{
 	decoder_control(PTZ_START);
 	decoder_control(PTZ_SAFE);
 	decoder_control(PTZ_LED);
}


</script>

</head>
<body bgcolor="#000000" onLoad="body_onload()">

<div>
<iframe name="video_zone"  style="position:absolute;left:0px;top:60px"src="/bikini/roomba_h264.html" width="750" height="520">
</iframe>
<div style="position:absolute;left:860px;top:60px" width="200" height="520" >
<img src="/hallogo2.gif" width="250" height="48" usemap="#CVLDI">
</div>
<iframe name="hal"  style="display:none"  width="900" height="1000" onkeydown=key(event) >
</iframe>


<div style="position:absolute;left:870px;top:250px" width="240" height="520" onkeydown="key(event)"   >
<img src="/halt.jpg" width="240" height="240" usemap="#halmap"  >
<map name="halmap">
			<area coords="0,0,80,80" href="javascript:void(0)" onClick="uleft()"> 
			<area coords="80,0,160,80" href="javascript:void(0)" onClick="up()" >
			<area coords="160,0,240,80" href="javascript:void(0)" onClick= "uright()">
		
			<area coords="0,80,80,160" href="javascript:void(0)" onClick="left()">
			<area coords="80,80,160,160" href="javascript:void(0)" onClick="stop()">
			<area coords="160,80,240,160" href="javascript:void(0)" onClick="right()">

			<area coords="0,160,80,240" href="javascript:void(0)" onClick="dleft()">
			<area coords="80,160,160,240" href="javascript:void(0)" onClick="down()">
			<area coords="160,160,240,240" href="javascript:void(0)" onClick="dright()">
</map>		
</div>
</body>
</html>
