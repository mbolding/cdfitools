<?
  if (!isset($_GET["what"]) ||
      !isset($_GET["other_id"])) {
    echo "Error: download_other.php: what and/or id not set\n";
    exit();
  }
  $what = $_GET["what"];
  $other_id = $_GET["other_id"];
  $suggest = "";
  $filename = "";     // set this to the fullpath filename for download

  // find row
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $q = mysql_query("SELECT * FROM other_files WHERE other_id=" . $other_id);
  if (!$q) { echo mysql_error(); exit(); }
  if (mysql_num_rows($q) < 1) { echo "download_other: other_row=" . $other_id . " not found in db"; exit(); }
  $row = mysql_fetch_array($q, MYSQL_ASSOC);

  $studyid2 = str_replace("/","_",$row["studyid"]);
  $pathname = "/usr/local/apache/htdocs/mri_analysis/studies/".$studyid2."/3dcalc/";

  if ($what == "head")
  {
    $suggest = $row["filename"]."+orig.HEAD"; 
    $filename = $pathname . $suggest; 
  }
  else if ($what == "brik")
  {
    $suggest = $row["filename"]."+orig.BRIK"; 
    $filename = $pathname . $suggest; 
  }
  else if ($what == "hdr")
  {
    $suggest = $row["filename"].".hdr"; 
    $filename = $pathname . $suggest; 
  }
  else if ($what == "img")
  {
    $suggest = $row["filename"].".img"; 
    $filename = $pathname . $suggest; 
  }

  if (!strlen($filename)) {
    echo "Error: download_other.php: filename not set by any delegate\n";
    exit();
  }

  header('Content-Type: application/octet-stream');
  header('Content-Disposition: attachment; filename='.$suggest);
  header('Content-Transfer-Encoding: binary');
  readfile($filename);
  exit();
?>