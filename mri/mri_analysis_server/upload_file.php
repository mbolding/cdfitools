<?
phpinfo(); exit();

// check for post variables
  if (!isset($_REQUEST["upload_file_type"]) || !isset($_REQUEST["studyid"]) ||
      !isset($_REQUEST["upload_file"]) || !isset($_REQUEST["upload_file2"]) ||
      !isset($_REQUEST["upload_comment"]))
    { echo "error: upload_file.php: upload_file_type, studyid, uploadfile[2], or upload_comment not specified\n"; exit(); }

  $filetype = $_REQUEST["upload_file_type"];
  $comment  = $_REQUEST["upload_comment"];
  $file1    = $_FILES["upload_file"]["tmp_name"];
  $name1    = $_FILES["upload_file"]["name"];
  $file2    = $_FILES["upload_file2"]["tmp_name"];
  $name2    = $_FILES["upload_file2"]["name"];

// make sure at least one file uploaded
  if ($_FILES["upload_file"]["error"] != 0) {
    echo "error: upload_file.php: first file upload error\n"; exit();
  }

// if type is AFNI or Analyze, make sure second file uploaded too
  $bSecondFile = ($_FILES["upload_file2"]["error"] != 0);
  if (($filetype=="afni" || $filetype=="analyze") && !$bSecondFile)  {
    echo "error: upload_file.php: need second file for 'afni' or 'analyze' uploads\n"; exit();
  }

  $studyid2 = str_replace("/", "_", $_REQUEST["studyid"]);
  $pathname = "/usr/local/apache/htdocs/mri_analysis/studies/".$studyid2."/3dcalc/";

// open database
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

// create new row in order to get new auto increment id
  $qry   = "INSERT INTO other_files SET studyid='".$_REQUEST["studyid"]."', file_type='".$filetype."', ";
  $qry  .= "pathname='".$pathname."', comment='".$comment."'";
  $q     = mysql_query($qry);
  $newid = mysql_insert_id();
  if (!$q) { echo mysql_error(); exit(); }

// new filenames are id _ old filename
  $filename1 = $newid."_".$file1;
  $filename2 = $newid."_".$file2;

// make sure directory for other files exists, then copy files from /tmp locations
  mkdir($pathname);
  copy($file1, $pathname.$filename1);
  if ($bSecondFile)
    copy($file2, $pathname.$filename2);

// update other_files table with file names and locations
  $qry = "UPDATE other_files SET pathname=
?>
