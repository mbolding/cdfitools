<?
  //ignore_user_abort(false);
  //exec("/bin/ps auwxr | /bin/grep httpd | /usr/bin/wc -l", $output);
  //if ($output[0] > 2) exit();

  function errorimg()
  {
    $defaultimg = "monkey.gif";
    header("Content-Type: image/gif");
    header("Content-Length: ".filesize($defaultimg));
    include($defaultimg);
    exit();
  }

  $file = $_GET["file"];
  $fd = @fopen($file, "r");
  if (!$fd)  errorimg();

  // get image dimensions from line like: "float matrix[] = {ro, pe};"
  while (!feof($fd))
  {
    $l = @fgets($fd, 1024);
    $parsed = preg_split("/[\s,={}]+/", $l);
    if ($parsed[1] == "matrix[]") 
    {
      $ro = $parsed[2];
      $pe = $parsed[3];
      break;
    }
  }

  while (!feof($fd))
  {
    $d = fread($fd, 1);
    if ($d == "\0")
      break;
  }
  header("Content-Type: image/gif");

  $f = fread($fd, 4);
  list(,$i) = unpack("f",strrev($f));
  $img[] = $i;
  $max = $i;
  $min = $i;
  while (!feof($fd))
  {
    $f = fread($fd, 4);
    list(,$i) = @unpack("f",strrev($f));  // suppress warning on last read
    if ($i > $max) $max = $i;
    if ($i < $min) $min = $i;
    $img[] = $i;

    //if (connection_aborted()) exit();
  }
  $range = $max - $min;

  $im = imagecreatetruecolor($ro, $pe);  // or imagecreatetruecolor
  $x = 0; $y = 0;
  foreach($img as $val)
  {
    $col = (int)(($val-$min)*255/$range);
    //$col = $col + $col*256 + $col*256*256;
    $col = $col | ($col << 8) | ($col << 16);
    imagesetpixel($im, $x, $y, $col);
    $x++;
    if ($x >= $ro) {
      $x = 0;
      $y++;
    }
    //print (int)(($val-$min)*255/$range);
    //print "\n";

    //if (connection_aborted()) exit();
  }
  imagegif($im);
  imagedestroy($im);

  //print "\n";
  exit();
?>