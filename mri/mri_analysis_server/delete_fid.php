<?
  $id = $_GET["id"];
  $other_id = 0;
  if (isset($_REQUEST["other_id"])) $other_id = $_REQUEST["other_id"];
  //
  //  don't actually delete mysql entry, just flag it as deleted:
  //
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  if ($other_id > 0)
  {
    $q = mysql_query("SELECT * FROM other_files WHERE other_id=".$other_id);
    if (!$q) { echo mysql_error(); exit(); }
    $row = mysql_fetch_array($q, MYSQL_ASSOC);
    $q = mysql_query("DELETE FROM other_files WHERE other_id=".$other_id);
    if (!$q) { echo mysql_error(); exit(); }
 
    $fullname = $row["pathname"].$row["filename"];
    if ($row["file_type"] == "AFNI" || $row["file_type"] == "3dcalc")
    {
      unlink($fullname."+orig.HEAD");
      unlink($fullname."+orig.BRIK");
    }
  }
  else
  {

  $sql = "UPDATE fid_files SET deleted='Y' WHERE id=" . $id;
  $q = mysql_query($sql);

  //
  //  if this is the last entry for this study, then delete the studies table entry too
  //
  $q = mysql_query("SELECT * FROM fid_files WHERE id=".$id);
  $row = mysql_fetch_array($q);
  $q = mysql_query("SELECT id,studyid FROM fid_files WHERE deleted='N' AND studyid=\"".$row["studyid"]."\"");
  if (mysql_num_rows($q) == 0)
  {
    mysql_query("DELETE FROM studies WHERE studyid=\"".$row["studyid"]."\"");
  }

  //OLD:
  //  don't delete directory either, but change dir name do 'indicate' deleted
  //
  //OLD: rename("/usr/local/apache/htdocs/mri_analysis/fid/$id", "/usr/local/apache/htdocs/mri_analysis/fid/._$id");

  // DELETE EVERYTHING RELATED TO THIS fid_file ROW:
  $path = "/usr/local/apache/htdocs/mri_analysis/studies/".str_replace("/","_",$row["studyid"])."/".$id."/";
  $allfiles = glob($path."*");
  foreach($allfiles as $thisfile) unlink($thisfile);
  rmdir($path);

  }

  //
  //  NEW: redirect to index_study.php instead of index.php:
  //       redirect back to main page if returntoindex==1
  //
  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];
  //header("Location: http://$host/mri_analysis/index.php");

  if (isset($_GET["returntoindex"]) && $_GET["returntoindex"]==1)
    header("Location: http://$host/mri_analysis/index.php");
  else
    header("Location: http://$host/mri_analysis/index_study.php?studyid=".$_GET["studyid"]);
  exit();  
?>