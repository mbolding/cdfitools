<?
  header("Content-type: text/xml");

//
//  this is an adaptation of process_fid.php...
//

  $id = $_GET["id"];

  $conn = @mysql_connect("legolas","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $query = "SELECT * FROM fid_files WHERE id=$id";
  $q = mysql_query($query);
  if (!$q) {
     echo "query error: ".mysql_error();
     exit;
  }

  $row = mysql_fetch_array($q, MYSQL_ASSOC);
 
  $next_image = $row[images_processed] + 1;

  if ($next_image >= $row["num_images"])
  {
    echo "done";
    exit();
  }

  $PATH = "/usr/local/apache/htdocs/mri_analysis";
  $outputdir = $PATH . "/fid/" . $id . "/";
  $matlab_file = $PATH . "/fid/$id/call_phase_correct_${id}.m";
  $eps_file = $outputdir . sprintf("image%03s.eps", $next_image); 
  $jpg_file = $outputdir . sprintf("image%03s.jpg", $next_image);
  $fp = fopen($matlab_file, "w");
  fwrite($fp, "addpath ${PATH}\n\n");
  fwrite($fp, "filenm = '${PATH}/fid/${id}/fid';\n");
  fwrite($fp, "outputdir = '${outputdir}';\n");
  fwrite($fp, "imageno = ${next_image};\n");
  fwrite($fp, "phase_correct(filenm, outputdir, imageno);\n");
  fwrite($fp, "pp = get(gcf, 'paperposition');\n");
  fwrite($fp, "incr=12/pp(4); pp([3 4]) = pp([3 4]) * incr;\n");
  fwrite($fp, "set(gcf, 'paperposition', pp);\n");
  fwrite($fp, "saveas(gcf, '${eps_file}', 'epsc2');\n");
  fclose($fp);

  $fi = popen("/usr/matlab/bin/matlab -nosplash < ${matlab_file}", "r");
  while (!feof($fi))
  {
    $s = fgets($fi);
  }

  if (file_exists($eps_file))
  {
    system("/usr/bin/convert -compress Lossless $eps_file $jpg_file");

    mysql_query("UPDATE fid_files SET images_processed=$next_image WHERE id=$id");

  }
//else
//    echo "ERROR: EPS file not found\n";

//echo $_GET["id"];

  echo sprintf("%03s", $next_image);

  exit();
?>