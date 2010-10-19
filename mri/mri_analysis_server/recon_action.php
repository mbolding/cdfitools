<?
  include('shared.inc');  // for: prepare_entire_output()

  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];

  $id = 0; if (isset($_GET["id"])) $id = $_GET["id"];  
  $other_id = 0; if (isset($_GET["other_id"])) $other_id = $_GET["other_id"];

  if ($id==0 && $other_id==0) die("Error: recon_action.php: did not specify either id or other_id");

  if (!isset($_GET["studyid"]) ||
      !isset($_GET["id"]) ||
      !isset($_GET["recon_action"]))
    die("Error: recon_action.php: did not specify studyid or recon_action");

  $studyid = $_GET["studyid"];
  $action  = $_GET["recon_action"];

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  // make sure directories exist, this is at this time the only place 
  // they are created:
  $studyid2 = str_replace("/","_",$studyid);
  $studydir = "/usr/local/apache/htdocs/mri_analysis/studies/" . $studyid2 . "/";
  mkdir($studydir);

  $fiddir = "";
  $row = array();
  if ($id != 0)
  {
    $fiddir = $studydir . $id . "/";
    mkdir($fiddir);

    // find row
    $q = mysql_query("SELECT * FROM fid_files WHERE id=" . $id);
    if (!$q) { echo mysql_error(); exit(); }
    if (mysql_num_rows($q) < 1) { echo "recon_action: fid row" . $id . " not found in db"; exit(); }
    $row = mysql_fetch_array($q, MYSQL_ASSOC);
  }
  else if ($other_id != 0)
  {
    $q = mysql_query("SELECT * FROM other_files WHERE other_id=" . $other_id);
    if (!$q) { echo mysql_error(); exit(); }
    if (mysql_num_rows($q) < 1) { echo "recon_action: other_id row" . $other_id . " not found in db"; exit(); }
    $row = mysql_fetch_array($q, MYSQL_ASSOC);
  }

//====================================================================================================
//
//  CONVERT FDF FILES (Varian Reconstruction) TO AFNI:
//
//====================================================================================================
  if ($action == "afni")
  {
    // apparently propar stores these params in the following units:
    //
    // FOV, gap, slice offset:  cm
    // slice thickness:         mm
    //
    $fdf_loc = $row["pathname"] . "/data/" . $row["dataname"] . ".img/*.fdf";
    $NSLICES = $row["num_slices"];
    $NIMAGES = $row["num_images"] - 1;
    $fovro = $row["fov_ro"] * 10 / 2;
    $fovpe = $row["fov_pe"] * 10 / 2;
    $zfov1 = ($NSLICES * $row["slice_thickness"] + ($NSLICES-1) * $row["gap"]*10.0) / 2.0;
    //$zfov1 = $NSLICES * ($row["slice_thickness"]+$row["gap"]*10) / 2;

    $zfov2 = sprintf("%.2f", $zfov1 + $row["slice_offset"]*10);
    $zfov1 = sprintf("%.2f", $zfov1 - $row["slice_offset"]*10);
    $TR    = $row["tr_s"] * $row["shots"];
    $prefix = str_replace("/", "_", $row["dataname"]."_fdf_".$row["studyid"]);
    // TODO: FOV params might not take negative numbers like -64R--64L which should be: 64L-64R
    $X1 = "L"; $X2 = "R"; $Y1 = "A"; $Y2 = "P"; $Z1 = "S"; $Z2 = "I";  // settings for transverse
    if ($row["orient"] == "cor" || $row["orient"] == "coronal") {
      $Y1 = "S"; $Y2 = "I"; $Z1 = "P"; $Z2 = "A";
    }
    else { // otherwise assume saggital
      $X1 = "L"; $X2 = "R"; $Y1 = "A"; $Y2 = "P"; $Z1 = "S"; $Z2 = "I";  // settings for transverse
    }

    $Z1t = $Z1; $Z2t = $Z2;
    if ($zfov1 < 0.0) {
      $Z1t = $Z2;
      $zfov1 *= -1;
    }
    if ($zfov2 < 0.0) {
      $Z2t = $Z1;
      $zfov2 *= -1;     
    }
    $Z1 = $Z1t; $Z2 = $Z2t;

    if ($NIMAGES < 1) // if 3D dataset then...
    {
      $cmd  = "/usr/local/afni/to3d -4swap -xFOV ".$fovro.$X1."-".$fovro;
      $cmd .= $X2." -yFOV ".$fovpe.$Y1."-".$fovpe.$Y2." -zFOV ".$zfov1.$Z1."-".$zfov2.$Z2." -session ".$fiddir;
      $cmd .= " -prefix ".$prefix;
      $cmd .= " 3Df:-1:0:".$row["mtx_ro"].":".$row["mtx_pe"].":1:".$fdf_loc." 2>&1";
    }
    else // if 4D dataset then...
    {
      $cmd  = "/usr/local/afni/to3d -4swap -time:tz $NIMAGES $NSLICES ".$TR."sec zero -xFOV ".$fovro.$X1."-".$fovro;
      $cmd .= $X2." -yFOV ".$fovpe.$Y1."-".$fovpe.$Y2." -zFOV ".$zfov1.$Z1."-".$zfov2.$Z2." -session ".$fiddir;
      $cmd .= " -prefix ".$prefix;
      $cmd .= " 3Df:-1:0:".$row["mtx_ro"].":".$row["mtx_pe"].":1:".$fdf_loc." 2>&1";
    }

    $lastline = exec($cmd, $output, $result);
    if ($result == 0) $result = "Y"; else $result = "N";
    $entire_output = prepare_entire_output($output);

    $qry = "UPDATE fid_files SET to3d_cmd=\"" . $cmd . "\", AFNI_files='" . $result . "', to3d_result=\"" . $entire_output . "\" WHERE id=" . $id;
    $q = mysql_query($qry);
    if (!$q) { echo $qry."\n".mysql_error(); exit(); }
  }
//====================================================================================================
//
//  CONVERT AFNI FILES TO ANALYZE (Varian fdf reconstruction)
//
//====================================================================================================
  else if ($action == "analyze")
  {
    $filename = $fiddir . $row["dataname"]."_fdf_".$studyid2;  // See also: download.php, what == 'head'/'brik'

    // use -4D for both 3d and 4d datasets
    $cmd = "/usr/local/afni/3dAFNItoANALYZE -4D -orient LPI ".$filename." ".$filename."+orig. 2>&1";
    $lastline = exec($cmd, $output, $result);

    // also silently make nifties:
    //$cmd2 = "/usr/local/afni/3dAFNItoNIFTI ".$filename."+orig. 2>&1";
    //exec($cmd2, $output2, $result2);

    if ($result == 0) $result = "Y"; else $result = "N";
    $entire_output = prepare_entire_output($output); // . "<br>" . prepare_entire_output($output2);

    $qry = "UPDATE fid_files SET analyze_cmd=\"" . $cmd . "\", hdrimg_files='" . $result . "', analyze_result=\"" . $entire_output . "\" WHERE id=" . $id;
    $q = mysql_query($qry);
    if (!$q) { echo $qry."\n".mysql_error(); exit(); }
  }
  else if ($action == "fsl")
  {
  }
  else if ($action == "fid2raw")
  {
    $origfid = $row["pathname"] . "/data/" . $row["dataname"] . ".fid/";
    if (!copy($origfid."fid", $fiddir."fid")) { echo "error: recon_action.php: copy(".$origfid."fid,".$fiddir."fid) failed"; exit(); }
    if (!copy($origfid."procpar", $fiddir."procpar")) { echo "error: recon_action.php: copy(".$origfid."procpar,".$fiddir."procpar) failed"; exit(); }
    $cmd = "/usr/local/bin/fid2raw -nogui " . $fiddir; // . " 2>&1";
    $lastline = exec($cmd, $output, $result);
    if ($result == 0)
    {
      $result = "Y";
      foreach ($output as $line)
        if (substr($line,0,2) == "DC")
          $lastline = $line;
    } 
    else 
      $result = "N";

    $entire_output = prepare_entire_output($output);

    $qry = "UPDATE fid_files SET result_line_1=\"" . str_replace("\"","'",$lastline) . "\", fid2raw_cmd=\"" . $cmd . "\", fid2raw_files='" . $result . "', result1=\"" . $entire_output . "\" WHERE id=" . $id;
    $q = mysql_query($qry);
    if (!$q) { echo $qry."\n".mysql_error(); exit(); }
  }
  //
  //  
  //
  else if ($action == "fid2raw_afni")
  {
    //$oldfile = $fiddir . "to3d+orig.HEAD";
    //if(file_exists($oldfile)) unlink($oldfile);
    //$oldfile = $fiddir . "to3d+orig.BRIK";
    //if(file_exists($oldfile)) unlink($oldfile);

    $NSLICES = $row["num_slices"];
    $NIMAGES = $row["num_images"] - 1;
    $TOTSLICES = $NSLICES * $NIMAGES;
    $fovro = $row["fov_ro"] * 10 / 2;
    $fovpe = $row["fov_pe"] * 10 / 2;
    $zfov1 = ($NSLICES * $row["slice_thickness"] + ($NSLICES-1) * $row["gap"]*10.0) / 2.0;
    $zfov2 = sprintf("%.2f", $zfov1 + $row["slice_offset"]*10.0);
    $zfov1 = sprintf("%.2f", $zfov1 - $row["slice_offset"]*10.0);
    $TR    = $row["tr_s"] * $row["shots"];
    $prefix = str_replace("/", "_", $row["dataname"]."_".$row["studyid"]);
    // TODO: FOV params might not take negative numbers like -64R--64L which should be: 64L-64R
    $X1 = "L"; $X2 = "R"; $Y1 = "A"; $Y2 = "P"; $Z1 = "S"; $Z2 = "I";  // settings for transverse
    if ($row["orient"] == "cor" || $row["orient"] == "coronal") {
      $Y1 = "S"; $Y2 = "I"; $Z1 = "P"; $Z2 = "A";
    }
    else { // otherwise assume saggital
      $X1 = "L"; $X2 = "R"; $Y1 = "A"; $Y2 = "P"; $Z1 = "S"; $Z2 = "I";  // settings for transverse
    }

    $Z1t = $Z1; $Z2t = $Z2;
    if ($zfov1 < 0.0) {
      $Z1t = $Z2;
      $zfov1 *= -1;
    }
    if ($zfov2 < 0.0) {
      $Z2t = $Z1;
      $zfov2 *= -1;     
    }
    $Z1 = $Z1t; $Z2 = $Z2t;

    $cmd  = "/usr/local/afni/to3d -time:zt $NSLICES $NIMAGES ".$TR."sec zero -xFOV ".$fovro.$X1."-".$fovro.$X2." ";
    $cmd .= "-yFOV ".$fovpe.$Y1."-".$fovpe.$Y2." -zFOV ".$zfov1.$Z1."-".$zfov2.$Z2." -session ".$fiddir;
    $cmd .= " -prefix ".$prefix;
    $cmd .= " 3Df:-1:0:".$row["mtx_ro"].":".$row["mtx_pe"].":".$TOTSLICES.":".$fiddir."fid_phase_corrected.raw 2>&1";

    $lastline = exec($cmd, $output, $result);
    if ($result == 0) $result = "Y"; else $result = "N";
    $entire_output = prepare_entire_output($output);

    $qry = "UPDATE fid_files SET result_line_2=\"" . str_replace("\"","'",$lastline) . "\", fid2raw_to3d_cmd=\"" . $cmd . "\", fid2raw_AFNI_files='" . $result . "', result2=\"" . $entire_output . "\" WHERE id=" . $id;
    $q = mysql_query($qry);
    if (!$q) { echo $qry."\n".mysql_error(); exit(); }
  }
//====================================================================================================
//
//  CONVERT AFNI FILES TO ANALYZE (Custom fid2raw reconstruction)
//
//====================================================================================================
  else if ($action == "fid2raw_analyze")
  {
    $filename = $fiddir . $row["dataname"]."_".$studyid2;  // See also: download.php and var '$prefix', just above

    // use -4D for both 3d and 4d datasets to don't get _0000 at end of filenames
    $cmd = "/usr/local/afni/3dAFNItoANALYZE -4D -orient LPI ".$filename." ".$filename."+orig. 2>&1";

    $lastline = exec($cmd, $output, $result);
    if ($result == 0) $result = "Y"; else $result = "N";
    $entire_output = prepare_entire_output($output);

    $qry = "UPDATE fid_files SET fid2raw_analyze_cmd=\"" . $cmd . "\", fid2raw_hdrimg_files='" . $result . "', result3=\"" . $entire_output . "\" WHERE id=" . $id;
    $q = mysql_query($qry);
    if (!$q) { echo $qry."\n".mysql_error(); exit(); }
  }
  else if ($action == "fid2raw_fsl")
  {
  }
  else if ($action == "other_analyze")
  {
//====================================================================================================
//
//  CONVERT 3dCalc or Uploaded AFNI FILES TO ANALYZE
//
//====================================================================================================

    $fullname = $row["pathname"].$row["filename"];

    // use -4D for both 3d and 4d datasets to don't get _0000 at end of filenames
    $cmd = "/usr/local/afni/3dAFNItoANALYZE -4D -orient LPI ".$fullname." ".$fullname."+orig. 2>&1";

    $lastline = exec($cmd, $output, $result);
    if ($result == 0) $result = "Y"; else $result = "N";
    $entire_output = prepare_entire_output($output);

    $qry = "UPDATE other_files SET hdrimg_cmd=\"" . $cmd . "\", hdrimg_files='" . $result . "', hdrimg_output=\"" . $entire_output . "\" WHERE other_id=" . $other_id;
    $q = mysql_query($qry);
    if (!$q) { echo $qry."\n".mysql_error(); exit(); }
  }

  header("Location: http://$host/mri_analysis/index_study.php?view=files&studyid=".$studyid."&id=".$id);
  exit();
?>