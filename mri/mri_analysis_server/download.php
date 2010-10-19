<?
  if (!isset($_GET["what"]) ||
      !isset($_GET["id"])) {
    echo "Error: download.php: what and/or id not set\n";
    exit();
  }
  $what = $_GET["what"];
  $id = $_GET["id"];
  $suggest = "";
  $filename = "";     // set this to the fullpath filename for download

  // find row
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $q = mysql_query("SELECT * FROM fid_files WHERE id=" . $id);
  if (!$q) { echo mysql_error(); exit(); }
  if (mysql_num_rows($q) < 1) { echo "recon_action: fid row" . $id . " not found in db"; exit(); }
  $row = mysql_fetch_array($q, MYSQL_ASSOC);

  $studyid2 = str_replace("/","_",$row["studyid"]);
  $pathname = "/usr/local/apache/htdocs/mri_analysis/studies/".$studyid2."/".$id."/";

  if ($what == "head")
  {
    $suggest = $row["dataname"]."_fdf_".$studyid2."+orig.HEAD";  // see recon_action.php, $prefix in action=='afni'
    $filename = $pathname . $suggest; 
  }
  else if ($what == "brik")
  {
    $suggest = $row["dataname"]."_fdf_".$studyid2."+orig.BRIK";
    $filename = $pathname . $suggest; 
  }
  else if ($what == "hdr")
  {
    $suggest = $row["dataname"]."_fdf_".$studyid2.".hdr";
    $filename = $pathname . $suggest;
  }
  else if ($what == "img")
  {
    $suggest = $row["dataname"]."_fdf_".$studyid2.".img";
    $filename = $pathname . $suggest;
  }
  else if ($what == "fid2rawhead")
  {
    $suggest  = $row["dataname"]."_".$studyid2."+orig.HEAD";
    $filename = $pathname . $suggest; 
  }
  else if ($what == "fid2rawbrik")
  {
    $suggest  = $row["dataname"]."_".$studyid2."+orig.BRIK";
    $filename = $pathname . $suggest; 
  }
  else if ($what == "fid2rawhdr")
  {
    $suggest = $row["dataname"]."_".$studyid2.".hdr";
    $filename = $pathname . $suggest;
  }
  else if ($what == "fid2rawimg")
  {
    $suggest = $row["dataname"]."_".$studyid2.".img";
    $filename = $pathname . $suggest;
  }

  if (!strlen($filename)) {
    echo "Error: download.php: filename not set by any delegate\n";
    exit();
  }

  header('Content-Type: application/octet-stream');
  header('Content-Disposition: attachment; filename='.$suggest);
  header('Content-Transfer-Encoding: binary');
  readfile($filename);
  exit();
?>