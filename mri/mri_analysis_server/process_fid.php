<html>
<head>
<link rel="stylesheet" href="styles.css" type="text/css">
</head>
<body>
<pre>
<?
  $id = $_GET["id"];

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $query = "SELECT * FROM fid_files WHERE id=$id";
  $q = mysql_query($query);
  if (!$q) {
     echo "query error: ".mysql_error();
     exit;
  }

  $row = mysql_fetch_array($q, MYSQL_ASSOC);

  $PATH = "/usr/local/apache/htdocs/mri_analysis";
  $outputdir = $PATH . "/fid/" . $id . "/";
  $matlab_file = $PATH . "/fid/$id/call_phase_correct_${id}.m";
  $eps_file = $outputdir . "image001.eps";
  $jpg_file = $outputdir . "image001.jpg";
  $fp = fopen($matlab_file, "w");
  fwrite($fp, "addpath ${PATH}\n\n");
  fwrite($fp, "filenm = '${PATH}/fid/${id}/fid';\n");
  fwrite($fp, "outputdir = '${outputdir}';\n");
  fwrite($fp, "phase_correct(filenm, outputdir);\n");
  fwrite($fp, "pp = get(gcf, 'paperposition');\n");
  fwrite($fp, "incr=12/pp(4); pp([3 4]) = pp([3 4]) * incr;\n");
  fwrite($fp, "set(gcf, 'paperposition', pp);\n");
  fwrite($fp, "saveas(gcf, '${eps_file}', 'epsc2');\n");
  fclose($fp);

$num_images = 0;
$num_slices = 0;
//$next_image = 0;

//**************************************************************
//
//  NEED TO RUN xhost localhost FOR THIS TO WORK!!!!!!!
//  (or else saved image will be blank)
//
//**************************************************************
  // this doesn't work:
  // exec('/usr/X11R6/bin/xhost localhost');
  // neither does adding an /etc/X0.hosts file with the entry 'localhost'

  $fi = popen("/usr/matlab/bin/matlab -nosplash < ${matlab_file}", "r");
  while (!feof($fi))
  {
    $s = fgets($fi);

    if (!strncmp($s, "nimages", 7)) 
      $num_images = substr($s, 8);

    if (!strncmp($s, "nslices", 7))
      $num_slices = substr($s, 8);

    $s = ereg_replace(">>", "", $s);
    echo $s;
    flush();
  }

  echo "</pre>\n";

  if (file_exists($eps_file))
  {
    system("/usr/bin/convert -compress Lossless $eps_file $jpg_file");

    mysql_query("UPDATE fid_files SET num_images=$num_images, num_slices=$num_slices, images_processed=1 WHERE id=$id");

    //if ($num_images > 1)
    //  $next_image = 2;
  }
  else
    echo "ERROR: EPS file not found\n";

  $nexturl = "edit_fid.php?id=" . $id;
//if ($next_image > 0)
//   $nexturl .= "&next=" . $next_image;

?>

<font size="+1" face="arial,helvetical,sans-serif" color=red>
<a href="<? echo $nexturl; ?>">click to continue...</a></font>

</body>
</html>

