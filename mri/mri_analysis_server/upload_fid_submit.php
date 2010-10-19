<?
//echo $_FILES['fid_file']['name']."<br>";
//echo $_FILES['fid_file']['size']."<br>";
//echo $_FILES['fid_file']['tmp_name']."<br>";
//echo $_FILES['fid_file']['error']."<br>";
//echo "full filename = ".$_POST["full_filename"]."<br>";
//echo "client is = ".$_SERVER["REMOTE_ADDR"]." = ".gethostbyaddr($_SERVER["REMOTE_ADDR"])."<br>";
  $subject = $_POST["newsubject"];
  if (strlen($_POST["subject"]) > 0)
     $subject = $_POST["subject"];

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $nseg = 0;
  $num_slices = 0;
  $num_images = 0;
  $consec = "?";
  $exp_date = "?";
  $time_run = "?";
  $comment = "?";
  $operator = "?";
  $studyid = "?";
  $procpar = $_FILES["procpar_file"]["tmp_name"];
  $fp = fopen($procpar, "r");

  // 13 Dec 2005 jkg:
  // this loop has now been moved to read_propar function within include
  // file with same name.  code copied from here is now posted there
  // this code here should be replaced with the function and tested.
  // see discover_this.php for example using this fn.
  //  include('read_procpar.inc');

  while (!feof($fp))
  {
    $line = fgets($fp);
    if (substr($line, 0, 4) == "nseg")
    {
      $line = fgets($fp);
      list($dum, $nseg) = explode(" ",$line);
    }
    else if (substr($line, 0, 7) == "ms_intl")
    {
      $line = fgets($fp);
      $items = explode(" ",$line);
      if (substr($items[1],1,1) == "y")
        $consec = "slices";
      if (substr($items[1],1,1) == "n")
        $consec = "shots";
    }
    else if (substr($line, 0, 4) == "date")
    {
      $line = fgets($fp);
      $exp_date = substr(trim($line), 3, -1);
    }
    else if (substr($line, 0, 2) == "ns")
    {
      $line = fgets($fp);
      list($dum,$num_slices) = explode(" ",$line);
    }
    else if (substr($line, 0, 6) == "images")
    {
      $line = fgets($fp);
      list($dum,$num_images) = explode(" ",$line);
    }
    else if (substr($line, 0, 8) == "time_run")
    {
      $line = fgets($fp);
      list($dum,$time_run) = explode(" ",$line);
      $time_run = ereg_replace("\"", "", $time_run);
    }
    else if (substr($line, 0, 7) == "comment")
    {
      $line = fgets($fp);
      $comment = substr($line, 2, -1);
      $comment = ereg_replace("\"", "", $comment);
    }
    else if (substr($line, 0, 9) == "operator_")
    {
      $line = fgets($fp);
      list($dum,$operator) = explode(" ",$line);
      $operator = ereg_replace("\"", "", $operator);
      $operator = ereg_replace("\n", "", $operator);
    }
    else if (substr($line, 0, 8) == "studyid_") 
    {
      $line = fgets($fp);
      list($dum,$studyid) = explode(" ",$line);
      $studyid = ereg_replace("\"", "", $studyid);
      $studyid = ereg_replace("\n", "", $studyid);
    }
  }
  fclose($fp);

  // extract "epidw_XX" from the filename and store as 'dataname' field in db:
  $dataname = mysql_escape_string($_POST["full_filename"]);
  $dirlist  = split('/', $dataname);
  $dataname = $dirlist[count($dirlist)-2];
  $dataname = ereg_replace(".fid", "", $dataname);

  $sql = "INSERT INTO fid_files (filename,client_ip,file_size,when_uploaded,exp_date,description,shots,consecutive,num_images,num_slices,blocka,blockb,time_run,comment,studyid,subject,dataname) VALUES (";
  $sql .= " \"" . mysql_escape_string($_POST["full_filename"]) . "\"";
  $sql .= ", \"" . gethostbyaddr($_SERVER["REMOTE_ADDR"]) . "\"";
  $sql .= ", " . $_FILES["fid_file"]["size"];
  $sql .= ", NOW()";
  $sql .= ", \"" . $exp_date . "\"";
  $sql .= ", \"" . mysql_escape_string($_POST["description"]) . "\"";
  $sql .= ", " . $nseg;
  $sql .= ", \"" . $consec . "\"";
  $sql .= ", ".$num_images;
  $sql .= ", ".$num_slices;
  $sql .= ", ".$_POST["blocka"];
  $sql .= ", ".$_POST["blockb"];
  $sql .= ", \"" . $time_run . "\"";
  $sql .= ", \"" . mysql_escape_string($comment) . "\"";
  $sql .= ", \"" . $studyid . "/" . $operator . "\"";
  $sql .= ", \"" . $subject . "\"";
  $sql .= ", \"" . $dataname . "\"";
  $sql .= ")";
  // according to this page: us4.php.net/mysql_insert_id  need the "or die" or can miss some errors 
  // for some reason (?)
  $q = mysql_query($sql) or die("upload_fid_submit: mysql insert error : " . mysql_error());
  $errstr = mysql_error();
  if ($errstr != "")
  {
    echo "mysql insert error: ";
    echo $errstr;
    exit();
  }

  $id = mysql_insert_id();
  $newdir = "/usr/local/apache/htdocs/mri_analysis/fid/$id";
  mkdir($newdir);
  copy($_FILES["fid_file"]["tmp_name"], $newdir."/fid");
  copy($_FILES["procpar_file"]["tmp_name"], $newdir."/procpar");

  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];
  //header("Location: http://$host/mri_analysis/edit_fid.php?id=$id");
  header("Location: http://$host/mri_analysis/edit_fid.php?id=$id");
  exit();  
?> 