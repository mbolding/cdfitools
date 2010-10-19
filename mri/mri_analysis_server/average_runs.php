<?
  $host = strlen($_SERVER['HTTP_HOST']) ? $_SERVER['HTTP_HOST'] : $_SERVER['SERVER_NAME'];

  // Check for expected request vars:
  if (!isset($_REQUEST["avg_what"]) ||
      !isset($_REQUEST["studyid"])) {
    echo "error: average_runs.php: 'avg_what' and/or 'studyid' is not set\n"; exit(); }

  // Make sure at least one row was selected:
  if (!isset($_REQUEST["sel_row"])) {
    $errmsg = "Average Runs: Please select which runs to average using the checkboxes";
    header("Location: http://$host/mri_analysis/index_study.php?view=files&studyid=".$_REQUEST["studyid"]."&errmsg=".urlencode($errmsg));
    exit();
  }

  include('shared.inc');  // for: prepare_entire_output()

  // open database:
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  // call 3dcalc to average the data:
  $studyid  = $_REQUEST["studyid"];
  $studyidmod = str_replace("/", "_", $studyid);
  $filename = "/usr/local/apache/htdocs/mri_analysis/studies/".$studyidmod."/";
  $type     = $_REQUEST["avg_what"];
  $row2chk  = "AFNI_files";  if ($type == "fid2raw") $row2chk = "fid2raw_AFNI_files";
  $fnstr    = "_fdf_";       if ($type == "fid2raw") $fnstr   = "_";
  $sel_row  = $_REQUEST["sel_row"];
  $letter   = "a";
  $midcmd   = "";
  $expr     = ' -expr "(';
  $nruns    = 0;
  $comment  = "Avg ";
  $prefix_suffix = "";
  $id = 0;  // save one of the id's in the other_files table for refernce (e.g. for calc'ing TR)
  $tr_s = 0;
  $num_images = 0;
  if ($row2chk == "AFNI_files") $comment .= "(fdf data) of "; else $comment .= "(fid2raw data) of ";
  foreach ($sel_row as $this_id => $val)
  {
    $q = mysql_query("SELECT * FROM fid_files WHERE id=" . $this_id);
    if (!$q) { echo mysql_error(); exit(); }
    if (mysql_num_rows($q) < 1) { echo "average_runs.php: fid row ".$this_id." not found in db"; exit(); }
    $row = mysql_fetch_array($q, MYSQL_ASSOC);  
    if ($row[$row2chk] == "Y")
    {
      $midcmd .= " -".$letter." ".$filename.$this_id."/".$row["dataname"].$fnstr.$studyidmod."+orig.";
      if ($nruns > 0) 
        { $expr .= "+"; $comment .= ", "; }
      else 
      { 
        $id = $row["id"]; 
        $tr_s = $row["shots"] * $row["tr_s"]; 
        $num_images = $row["num_images"];
      }
      $expr   .= $letter;
      $letter  = chr(ord($letter) + 1);
      $nruns++;
      $comment .= $row["dataname"];
      $prefix_suffix .= "_".$row["run_no"];
    }
    else
    {
      $errmsg = "Average Runs: Please only select runs that have AFNI files already generated.";
      header("Location: http://$host/mri_analysis/index_study.php?view=files&back=1&studyid=".$_REQUEST["studyid"]."&errmsg=".urlencode($errmsg));
      exit();
    }
  }

  // make sure '3dcalc' subdirectory of study directory exists:
  $newlocation = $filename."3dcalc/";
  mkdir($newlocation);

  // add a table entry for this pair (HEAD/BRIK) of files:
  $qry  = "INSERT INTO other_files SET id=".$id.", studyid='".$studyid."', file_type='3dcalc', pathname='";
  $qry .= $newlocation."', comment='".$comment."', tr_s=".$tr_s.", num_images=".$num_images;
  $q = mysql_query($qry);
  $newid = mysql_insert_id();
  if (!$q) { echo mysql_error(); exit(); }

  // Run 3dcalc:
  $expr .= ')/'.$nruns.'"';
  $prefix = "3dcalc".$newid.str_replace("s_", "_", $studyidmod).$prefix_suffix;
  $cmd = "/usr/local/afni/3dcalc ".$midcmd.$expr." -prefix ".$prefix." -session ".$newlocation." 2>&1";
  $lastline = exec($cmd, $output, $result);
  if ($result == 0) $result = "Y"; else $result = "N";
  $entire_output = prepare_entire_output($output);

  // If successful, update table with results; if fail, delete new table row
  if ($result == "Y")
  {
    $q = mysql_query("UPDATE other_files SET filename=\"".$prefix."\", command=\"".str_replace("\"","\\\"",$cmd)."\", cmd_output=\"".$entire_output."\" WHERE other_id=".$newid);
    if (!$q) { echo mysql_error(); exit(); }
  }
  else
  {
    $q = mysql_query("DELETE FROM other_files WHERE other_id=".$newid);
    if (!$q) { echo mysql_error(); exit(); }
  }
  
  header("Location: http://$host/mri_analysis/index_study.php?view=files&studyid=".$studyid."&other_id=".$newid);
  exit();
?>