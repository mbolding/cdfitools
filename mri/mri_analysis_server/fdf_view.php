<?
  function badimage()
  {
    $defaultimg = "monkey.gif";
    header("Content-Type: image/gif");
    header("Content-Length: ".filesize($defaultimg));
    include($defaultimg);
    exit();
  }
  if (!isset($_GET["file"])) badimage();

  $fd = fopen($_GET["file"], "r");
  if ($fd == false) badimage();  // not sure if checking for false works

  while (!feof($fd))
  {
    $b = fread($fd, 1);
    if ($b == "\0") break;  // look for single null char that divides header from data
  }
  if (feof($fd)) badimage();

  header("Content-Type: image/png");
  header("Content-Length: ".filesize($_GET["file"]));
  fpassthru($fd);
  fclose($fd);
  exit(0);
?>