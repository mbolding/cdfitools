<?
  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];
  
  if (!isset($_POST["studyid"]) ||
      !isset($_POST["id"]) ||
      !isset($_POST["new_comment"]))
  {
    header("Location: http://$host/mri_analysis/index.php");
    exit();      
  }
  $studyid = $_POST["studyid"];
  $id      = $_POST["id"];
  $comment = $_POST["new_comment"];
  $comment = str_replace("\"", "'", $comment);
  $comment = str_replace("\\", "/", $comment);  // replace backslash with fwd slash

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $q = mysql_query("UPDATE fid_files set comment=\"".$comment."\" WHERE id=".$id);
  if (!$q) { echo mysql_error(); exit(); }

  header("Location: http://$host/mri_analysis/index_study.php?studyid=".$studyid."&id=".$id);
  exit();
?>