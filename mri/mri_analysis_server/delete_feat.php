<?
  $feat_id = 0;  if (isset($_REQUEST["feat_id"])) $feat_id = $_REQUEST["feat_id"];
  $studyid = ""; if (isset($_REQUEST["studyid"])) $studyid = $_REQUEST["studyid"];
  $id = 0;       if (isset($_REQUEST["id"])) $id = $_REQUEST["id"];
  $other_id = 0; if (isset($_REQUEST["other_id"])) $other_id = $_REQUEST["other_id"];

  if ($feat_id == 0 || $studyid == "") die('Error: delete_feat.php: feat_id or studyid not set');

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $q = mysql_query("SELECT * FROM fsl_feat WHERE feat_id=".$feat_id);
  if (!$q) die(mysql_error());
  if (mysql_num_rows($q) < 1) die('Error: delete_feat.php: could not find row');
  $row = mysql_fetch_array($q, MYSQL_ASSOC);

  $dir = "/usr/local/apache/htdocs/".$row["feat_report"];
  if (substr($dir,-17) != ".feat/report.html") die('Error: delete_feat.php: feat_report field wrong');
  $dir = str_replace("report.html", "", $dir);
  $dir = str_replace("..", "", $dir); // make sure no double dots snuck in there

  foreach (glob($dir."*") as $thisfile) unlink($thisfile);
  unlink($dir.".fslcurrent");
  unlink($dir.".ramp.gif");
  foreach (glob($dir."mc/*") as $thisfile) unlink($thisfile);
  foreach (glob($dir."stats/*") as $thisfile) unlink($thisfile);
  foreach (glob($dir."tsplot/*") as $thisfile) unlink($thisfile);

  rmdir($dir."mc/");
  rmdir($dir."stats/");
  rmdir($dir."tsplot/");
  if (!rmdir($dir)) die("Error: delete_feat.php: could not delete directory: ".$dir);
  //rmdir($dir);

  $q = mysql_query("DELETE FROM fsl_feat WHERE feat_id=".$feat_id);
  if (!$q) die(mysql_error());

  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];
  header("Location: http://$host/mri_analysis/index_study.php?studyid=".$studyid."&id=".$id."&other_id=".$other_id."&view=fsl");
  exit();
?>