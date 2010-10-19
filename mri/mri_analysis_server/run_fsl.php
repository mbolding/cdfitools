<?
include('shared.inc');

  $id           = 0;
  $other_id     = 0;
  if (isset($_REQUEST["id"]))       $id = $_REQUEST["id"];
  if (isset($_REQUEST["other_id"])) $other_id = $_REQUEST["other_id"];
  if ($id==0 && $other_id==0) die("Error: run_fsl.php: called with neither id nor other_id set");

  $studyid      = $_REQUEST["studyid"];
  $studyidmod   = str_replace("/","_",$studyid);
  $blockA       = $_REQUEST["blockA"];
  $blockB       = $_REQUEST["blockB"];
  $phase        = $_REQUEST["phase"];
  $tr           = $_REQUEST["TR"];
  $motion_corr  = 0; 
  if (isset($_REQUEST["motion_correction"]) && $_REQUEST["motion_correction"]==1) $motion_corr = 1;
  $brain_ext    = 0; 
  if (isset($_REQUEST["brain_extraction"]) && $_REQUEST["brain_extraction"]==1) $brain_ext = 1;
  $fwhm         = $_REQUEST["spatial_smoothing"];
  $z_thresh     = $_REQUEST["z_threshold"];
  $registration = $_REQUEST["registration"];
  $dof          = $_REQUEST["dof"];
  $type         = "";
  if (isset($_REQUEST["type"])) $type = $_REQUEST["type"];
  if ($type!="fdf" && $type!="fid2raw" && $other_id==0) 
          { echo "ERROR; run_fsl.php: 'type' set to invalid value (".$type.")"; exit(); }

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $DIR = "";
  $hdrimgFN = "";
  $row = array();
  if ($id != 0)
  {
    $DIR = "/usr/local/apache/htdocs/mri_analysis/studies/".$studyidmod."/".$id."/";
    $q = mysql_query("SELECT * FROM fid_files WHERE id=" . $id);
    if (!$q) { echo mysql_error(); exit(); }
    if (mysql_num_rows($q) < 1) { echo "ERROR: run_fsl.php: fid row" . $id . " not found in db"; exit(); }
    $row = mysql_fetch_array($q, MYSQL_ASSOC);
    // to build filename, see recon_action.php
    if ($type == "fdf")
      $hdrimgFN = $row["dataname"]."_fdf_".$studyidmod;
    else
      $hdrimgFN = $row["dataname"]."_".$studyidmod;
  }
  else // other_id must not be zero
  {
    $q = mysql_query("SELECT * FROM other_files WHERE other_id=".$other_id);
    if (!$q) die(mysql_error());
    if (mysql_num_rows($q) < 1) die("Error: run_fsl.php: other_id ".$other_id." not found in db");
    $row = mysql_fetch_array($q, MYSQL_ASSOC);
    $DIR = $row["pathname"];
    $hdrimgFN = $row["filename"];
  }
  copy("/usr/local/apache/htdocs/mri_analysis/design_template.fsf", $DIR."design.fsf");
  $h = popen("/bin/cat >> ".$DIR."design.fsf", "w");
  fwrite($h, "\nset feat_files(1) \"".$DIR.$hdrimgFN.".hdr\"\n");
  fwrite($h, "\nset fmri(off1) ".     $blockA."\n");
  fwrite($h, "\nset fmri(on1) ".      $blockB."\n");
  fwrite($h, "\nset fmri(phase1) ".   $phase."\n");
  fwrite($h, "\nset fmri(npts) ".    ($row["num_images"]-1)."\n");
  fwrite($h, "\nset fmri(z_thresh) ". $z_thresh."\n");
  fwrite($h, "\nset fmri(tr) ".       $tr."\n");
  fwrite($h, "\nset fmri(smooth) ".   $fwhm."\n");
  fwrite($h, "\nset fmri(bet_yn) ".   $brain_ext."\n");
  fwrite($h, "\nset fmri(mc) ".       $motion_corr."\n");
  fclose($h);

  //
  // JKG 02Aug2006 Error:  try fix by adding USER=nobody to run cmd below
  //
  // FSLDIR=/usr/local/fsl PATH=/usr/local/fsl/bin:$PATH /usr/local/fsl/bin/feat /usr/local/apache/htdocs/mri_analysis/studies/s_20060731_01_mark/3dcalc/design.fsf 2>&1 can't read \"env(USER)\": no such variable
  // while executing
  // \"set USER $env(USER)\"
  // (file \"/usr/local/fsl/tcl/fslstart.tcl\" line 2)
  // invoked from within
  // \"source [ file dirname [ info script ] ]/fslstart.tcl\"
  // (file \"/usr/local/fsl/tcl/feat.tcl\" line 12)
  // invoked from within
  // \"source ${FSLDIR}/tcl/feat.tcl\"
  // (file \"/usr/local/fsl/bin/feat\" line 15)
  $cmd = "USER=nobody FSLDIR=/usr/local/fsl PATH=/usr/local/fsl/bin:\$PATH /usr/local/fsl/bin/feat ".$DIR."design.fsf 2>&1";
  $lastline = exec($cmd, $output, $result);
  if ($result == 0) $result = "Y"; else $result = "N";
  $entire_output = prepare_entire_output($output);  // in: shared.inc

  if ($result == "Y")
  {
    $comment  = "model=".$blockA."s/".$blockB."s"; if ($phase != 0) $comment .= " (phase=".$phase."s)";
    $comment .= ", FWHM=".$fwhm."mm, Z-thresh=".$z_thresh.", data type=".$type;
    $report   = str_replace("/usr/local/apache/htdocs", "", $output[count($output)-2]);
    $q = mysql_query("INSERT INTO fsl_feat SET id=".$id.", other_id=".$other_id.", comment=\"".$comment."\", blockA_s=".$blockA.", blockB_s=".$blockB.", phase_s=".$phase.", tr_s=".$tr.", smoothing_mm=".$fwhm.", z_thresh=".$z_thresh.", motion_correction='".$motion_corr."', brain_extraction='".$brain_ext."', data_type=\"".$type."\", entire_output=\"".$entire_output."\", feat_report=\"".$report."\"");
    if (!$q) { echo mysql_error(); exit(); }
  }
  else 
  {
    echo $cmd."\n";
    echo $entire_output;
    exit();
  }

  $host = strlen($_SERVER['HTTP_HOST']) ? $_SERVER['HTTP_HOST'] : $_SERVER['SERVER_NAME'];
  header("Location: http://$host/mri_analysis/index_study.php?view=fsl&studyid=".$studyid."&other_id=".$other_id."&id=".$id);
  exit();
?>