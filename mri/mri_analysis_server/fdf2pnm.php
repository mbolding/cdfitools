<?
  $file = $_GET["file"];
  $fd = fopen($file, "r");
  while (!feof($fd))
  {
    $d = fread($fd, 1);
    if ($d == "\0")
      break;
  }
  header("Content-Type: image/pgm");  // png?
  echo "P2\n128 128\n255\n";  // I think P2 is ascii grayscale img (pgm)

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
  }
  $range = $max - $min;

  foreach($img as $val)
  {
    print (int)(($val-$min)*255/$range);
    print "\n";
  }
  exit();
?>