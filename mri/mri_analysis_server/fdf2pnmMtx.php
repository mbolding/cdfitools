<?
  $file = $_GET["file"];
  $fd = fopen($file, "r");

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

  // single null char separates header from data, skip until find null
  while (!feof($fd))
  {
    $d = fread($fd, 1);
    if ($d == "\0") 
      break;
  }

  header("Content-Type: image/pgm");   
  // I think P2 is ascii grayscale img (pgm)
  echo "P2\n# CREATED by fdf2pnm.php\n".$ro." ".$pe."\n255\n";

  $f = fread($fd, 4);
  // use strrev bec assuming fdf's are always big endian
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
  }
  $range = $max - $min;

  foreach($img as $val)
  {
    print (int)(($val-$min)*255/$range);
    print "\n";
  }
  exit();
?>