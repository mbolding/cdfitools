<?
//
// discover_this.php:
//   Add the study located at post variable 'fulldir' to the database
//   2005 Dec 12 - Jon Grossmann 
//
  include('read_procpar.inc');
  include('shared.inc');

  $fulldir = '';
  if (isset($_GET["fulldir"]))
     $fulldir = $_GET["fulldir"];
  else  {
    echo "ERROR: fulldir not set in call to discover_this\n"; die;
  }
  $studyxml = $fulldir."/study.xml";
  if (!file_exists($studyxml))  {
    echo "ERROR: study.xml not found\n"; die;
  }

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $fullstudyid = "";  // each element should set this, hopefully all to the same value, need at end of page
  
  function process_fid_dir($fulldirname, $dirname)  // parser, name ,attrs
  {
    global $fulldir;
    global $fullstudyid;

    list($protocol,$runno) = split('[_.]', $dirname);  // e.g.: epidw _ 01
    // skip spuls; note that most sems are just setting power and so aren't useful
    //if ($protocol != 'epidw' && $protocol != 'gems' && strncmp($protocol, 'T1map2', 6) && $protocol != 'sems')
    //  return;

    $dataname = $protocol."_".$runno;  // rebuild last part without 'data/' prefix

    //print_r($a);

    $procparname = $fulldirname . "/procpar"; //$fulldir."/".$a['DATA'].".fid/procpar";
    $fd = fopen($procparname, "r"); 
    if ($fd == false) {
      ; //echo "could not open: ".$procparname; return;
    }
    else { ; /* echo "opened: ".$procparname; */ }
    $procvals = read_procpar($fd);
    fclose($fd);

    // make sure row not already uploaded first:  use studyid (s_200xxxxx/mark) and dataname (epidw_xx)
    // DECISION: for now, allow same study (same studyid+dataname) to be uploaded (which means one uploaded
    // from 'Upload Fid' and one from 'Discover Data'.  So, a duplicate would be same studyid+dataname and
    // also fulldir is not blank, which means this data has been discovered before:
    // REMINDER: studyid field in database is constructed from procpar studyid + '/' + operator !!!
    $fullstudyid = $procvals['studyid']."/".$procvals['operator'];
    $check = "SELECT id,pathname FROM fid_files WHERE deleted='N' AND studyid=\"".$fullstudyid."\" AND dataname=\"".$dataname."\"";

    $q = mysql_query($check) or die("discover_this: mysql select error: " . mysql_error());
    if (mysql_error() != "") { echo "mysql select error: ".mysql_error(); exit(); }
    
    if (mysql_num_rows($q) > 0) 
    {
      //echo "discover data found that studyid=".$procvals['studyid'].", dataname=".$dataname." is already in the database... ";
      while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
      {
        if ($row['pathname'] != "") 
        {
          //echo "skipping because fulldir is not blank\n";
          return;
        }
      }
      ; //echo "adding again.\n";
    }

    $sql = "INSERT INTO fid_files (when_uploaded,exp_date,shots,consecutive,num_images,num_slices,time_run,comment,studyid,dataname,pathname,run_type,run_no,n_no,tr_s,te_s,orient,slice_thickness,slice_offset,gap,mtx_ro,mtx_pe,fov_ro,fov_pe,ms_intlv,gain,power,rfcoil,fatsat,ky_order,fract_ky,epi_pc) VALUES (";
    $sql .= "NOW()";
    $sql .= ", \"" . $procvals['exp_date'] . "\"";
    $sql .= ", " . $procvals['nseg'];
    $sql .= ", \"" . $procvals['consec'] . "\"";
    $sql .= ", ".$procvals['num_images'];
    $sql .= ", ".$procvals['num_slices'];
    $sql .= ", \"" . $procvals['time_run'] . "\"";
    $sql .= ", \"" . mysql_escape_string($procvals['comment']) . "\"";
    $sql .= ", \"" . $fullstudyid . "\"";
    // no subject in procpar! : $sql .= ", \"" . $procvals['subject'] . "\"";
    $sql .= ", \"" . $dataname . "\"";
    $sql .= ", \"" . $fulldir . "\"";
    $sql .= ", \"" . $protocol . "\"";
    $sql .= ", \"" . $runno . "\"";
    $sql .= ", \"" . $procvals['actionid'] . "\"";     //a['ID'] . "\"";
    $sql .= ", ".$procvals['tr'];
    $sql .= ", ".$procvals['te'];

    $sql .= ", \"".$procvals['orient']."\"";
    $sql .= ", ".$procvals['thk'];
    $sql .= ", ".$procvals['pss0'];
    $sql .= ", ".$procvals['gap'];
    $sql .= ", ".$procvals['np'];
    $sql .= ", ".$procvals['nv'];
    $sql .= ", ".$procvals['lro'];
    $sql .= ", ".$procvals['lpe'];
    $sql .= ", \"".$procvals['ms_intlv']."\"";
    $sql .= ", ".$procvals['gain'];
    $sql .= ", ".$procvals['tpwr1'];
    $sql .= ", \"".$procvals['rfcoil']."\"";
    $sql .= ", \"".$procvals['fatsat']."\"";
    $sql .= ", \"".$procvals['ky_order']."\"";
    $sql .= ", ".(int)$procvals['fract_ky'];
    $sql .= ", \"".$procvals['epi_pc']."\"";

    $sql .= ")";
    //echo "\n".$sql."\n";
    //echo $n." = "; print_r($a); echo "\n";
    $q = mysql_query($sql) or die("discover_this: mysql insert error : " . mysql_error());
    $errstr = mysql_error();
    if ($errstr != "")  {
      echo "mysql insert error: " . $errstr; exit();
    }
    else
      ; //echo "insert successful ? " . mysql_insert_id() . "\n\n";

    // make sure studies table has this study
    $d["studyid"] = $fullstudyid;
    $dirparts = explode("/", $fulldir);    
    $d["description"] = $dirparts[count($dirparts)-1];
    $d["subject"] = "";
    $d["notes"] = "";
    $data = read_study_table($fullstudyid);
    if ($data["notfound"] == 0)
    {
      if (strlen($data["description"]))
        $d["description"] = $data["description"];
      $d["subject"] = $data["subject"];
      $d["notes"] = $data["notes"];
    }
    add_or_update_study($d);
  }

  foreach (glob($fulldir."/data/*.fid") as $thisfiddir)
  {
    if (!is_dir($thisfiddir)) continue;
    $dirparts = explode("/", $thisfiddir);
    process_fid_dir($thisfiddir, end($dirparts));
  }
  $host = strlen($_SERVER['HTTP_HOST'])?$_SERVER['HTTP_HOST']:$_SERVER['SERVER_NAME'];
  header("Location: http://$host/mri_analysis/index_study.php?studyid=".$fullstudyid);
  exit();  
?>