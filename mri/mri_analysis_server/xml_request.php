<?
  header("Content-type: text/xml");

  $step = $_GET["step"];
  $id = $_GET["id"];

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $query = "SELECT * FROM fid_files WHERE id=$id";
  $q = mysql_query($query);
  if (!$q) {
     echo "query error: ".mysql_error();
     exit;
  }

  $row = mysql_fetch_array($q, MYSQL_ASSOC);

  $cmd = "[no cmd]";
  $result = 0;  // 0==fail, 1=error
  $lastline = "[no output]";

  $NIMAGES = $row["num_images"] - 1;  // first image (phase correction scan) not written by raw2fid
  $NSLICES = $row["num_slices"];
  $TOTSLICES = $NIMAGES * $NSLICES;

  $DIR = "./fid/" . $id . "/";

//-------------------------------------------------------------------
//
//  1. fid -> raw
//
//-------------------------------------------------------------------
  if ($step == 1)
  {
    $fid = "/usr/local/apache/htdocs/mri_analysis/fid/" . $id;
    $cmd = "/usr/local/bin/fid2raw -nogui -img1 -info " . $fid; // . " 2>&1";
    $lastline = exec($cmd, $output, $result);
    if ($result == 0) {
      $result = 1;
      exec("/usr/bin/convert -geometry 850 ".$fid."/Image1.pgm ".$fid."/Image1.jpg 2>&1", $output);
      //unlink($fid."/Image1.pgm");

      foreach ($output as $line)
      {
        if (substr($line,0,2) == "DC") $lastline = $line;
      }
    }
    else 
      $result = 0;
  }
//-------------------------------------------------------------------
//
//  2. to3d
//
//-------------------------------------------------------------------
  else if ($step == 2)
  {
    $oldfile = $DIR . "to3d+orig.HEAD";
    if(file_exists($oldfile)) unlink($oldfile);
    $oldfile = $DIR . "to3d+orig.BRIK";
    if(file_exists($oldfile)) unlink($oldfile);

    $cmd = "/usr/local/afni/to3d -time:zt $NSLICES $NIMAGES 2sec zero -xFOV 64L-64R -yFOV 64A-64P -zFOV 6S-6I -session $DIR -prefix to3d 3Df:-1:0:64:64:${TOTSLICES}:${DIR}fid_phase_corrected.raw 2>&1";
    $lastline = exec($cmd, $output, $result);
    if ($result == 0) 
      $result = 1;
    else
      $result = 0;
  }
//-------------------------------------------------------------------
//
//  3. 3dAFNItoANALYZE
//
//-------------------------------------------------------------------
  else if ($step == 3)
  {
    $cmd = "/usr/local/afni/3dAFNItoANALYZE -4D -orient LPI ".$DIR."afni2analyze ".$DIR."to3d+orig. 2>&1";
    $lastline = exec($cmd, $output, $result);
    if ($result == 0) 
      $result = 1;
    else 
      $result = 0;

    if (strlen(trim($lastline)) == 0)
      $lastline = "[no output]";
  }
//-------------------------------------------------------------------
//
//  4. fsl
//
//-------------------------------------------------------------------
  else if ($step == 4)
  {
    $FSFDIR = $DIR."fsl/";
    mkdir($FSFDIR);
    copy("./fid/design_template.fsf", $FSFDIR."design.fsf");
    $h = popen("/bin/cat >> ".$FSFDIR."design.fsf", "w");
    fwrite($h, "\nset feat_files(1) \"/usr/local/apache/htdocs/mri_analysis/".$DIR."afni2analyze.hdr\"\n");
    fwrite($h, "\nset fmri(off1) ".$row["blocka"]."\n");
    fwrite($h, "\nset fmri(on1) ".$row["blockb"]."\n");
    fwrite($h, "\nset fmri(npts) ".$NIMAGES."\n");
    fwrite($h, "\nset fmri(z_thresh) ".$row["z_thresh"]."\n");
    fclose($h);

    $cmd = "FSLDIR=/usr/local/fsl PATH=/usr/local/fsl/bin:\$PATH /usr/local/fsl/bin/feat ".$FSFDIR."design.fsf 2>&1";
    $lastline = exec($cmd, $output, $result);
    if ($result == 0) 
      $result = 1;
    else 
      $result = 0;

    //$getnext = 0;
    //foreach ($output as $line)
    // {
    //  if ($getnext == 1)
    //  {
    $resulthtml = $output[count($output)-2];
    $resulthtml = str_replace("/usr/local/apache/htdocs", "", $resulthtml);
    $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];
    //$lastline = "&lt;a href=\"http://".$host.$resulthtml."\"&gt;view report&lt;/a&gt;";
    $lastline = "<a href=\"http://".$host.$resulthtml."\">view report</a>";
    //    $getnext = 0;
    //  }
    //  if (substr($line, 0, 23) == "To view the FEAT report")
    //  {
    //    $getnext = 1;
    //  }
    // }
  }

  // cmd has amperstand used to redirect stderr to stdout, but this causes stupid xml validation error, so fix:
  $cmd = str_replace("&", "&amp;", $cmd);
  //  saveResults($step, $result, $lastline, $output);
  //
  //  save results
  //
  $field_res      = "result" . $step;
  $field_res_line = "result_line_" . $step;
  $field_cmd      = "cmd" . $step;
  $field_rc       = "step_" . $step;
  $query  = "UPDATE fid_files SET ";
  $query .= $field_res_line."=\"".str_replace("\"", "\\\"", $lastline)."\"";
  $query .= ", ".$field_cmd."=\"".str_replace("\"", "\\\"", $cmd)."\"";
  $query .= ", ".$field_rc."=".$result;
  $entire_output = "";
  foreach ($output as $line)
  {
    if (strlen(trim($line)) > 0)
    {
      $line = str_replace("&", "&amp;", $line);
      $line = str_replace("\"", "\\\"", $line);  // escape double quotes
      $entire_output .= $line."<br>";
    }
  }
  $entire_output .= "<br>";
  $query .= ", ".$field_res . "=\"".$entire_output."\""; 
  $query .= " WHERE id=$id";
  $q = mysql_query($query);
  if (!$q)
  {
    $output[] = $query;
    $output[] = mysql_error();
  }

  $lastline = str_replace("<", "&lt;", $lastline);
  $lastline = str_replace(">", "&gt;", $lastline);

  echo "<?xml version=\"1.0\"?>\n";

// don't add a space to step and result because conditional expressions will fail
  echo "<contents>\n";
  echo "<step>".$step."</step>\n";
  echo "<command>".$cmd." </command>\n";
  echo "<result>".$result."</result>\n";
  echo "<poutput>".$lastline." </poutput>\n";
  foreach ($output as $line)
  {
  // an empty line yields an XML <outputline/> field that results in parse error on client
    if (strlen(trim($line)) > 0)
    {
      $line = str_replace("&", "&amp;", $line);
      echo "<outputline>".$line."</outputline>\n";
    }
  }
  echo "</contents>\n";
  exit();
?>

