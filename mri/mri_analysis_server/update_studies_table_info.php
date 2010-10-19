<?
  require("shared.inc");
  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];

  if (isset($_POST["studyid"]))
  {
    $data["studyid"] = $_POST["studyid"];
    $subj = $_POST["subject"];
    if (strlen($subj) < 1) $subj = $_POST["newsubject"];
    $data["subject"] = $subj;
    $data["description"] = $_POST["description"];
    $data["notes"] = $_POST["notes"];

    $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
    mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

    //update_study_table($data);
    add_or_update_study($data);

    if (strlen(mysql_error())) { echo mysql_error(); exit(); }

    header("Location: http://$host/mri_analysis/index_study.php?studyid=".$_POST["studyid"]);
    exit();    
  }

  header("Location: http://$host/mri_analysis/index.php");
  exit();    
?>