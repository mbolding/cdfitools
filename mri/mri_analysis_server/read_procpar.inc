<?
//
//  read procpar file already opened by file descriptor $fd and
//  return assoc array with values of several parameters
//
function read_procpar($fp)
{
  // default values so sql statements have values for all fields
  $ans['nseg'] = 0;
  $ans['num_images'] = 0;
  $ans['consec'] = "";   // gems (e.g.) does not have param 'consec'

  $ans['orient'] = "";
  $ans['thk'] = 0;
  $ans['pss0'] = 0;
  $ans['gap'] = 0;
  $ans['np'] = 0;
  $ans['nv'] = 0;
  $ans['lro'] = 0;
  $ans['lpe'] = 0;
  $ans['ms_intlv'] = "";
  $ans['gain'] = 0;
  $ans['tpwr1'] = 0;
  $ans['rfcoil'] = "";
  $ans['fatsat'] = "";
  $ans['ky_order'] = "";
  $ans['fract_ky'] = 0;
  $ans['epi_pc'] = "";
  $ans['actionid'] = "";

  while (!feof($fp))
  {
    $line = fgets($fp);
    if (substr($line, 0, 4) == "nseg")
    {
      $line = fgets($fp);
      list($dum, $nseg) = explode(" ",$line);
      $ans['nseg'] = $nseg;
    }
    else if (substr($line, 0, 7) == "ms_intl")
    {
      $line = fgets($fp);
      $items = explode(" ",$line);
      if (substr($items[1],1,1) == "y")
        $consec = "slices";
      if (substr($items[1],1,1) == "n")
        $consec = "shots";
      $ans['consec'] = $consec;
    }
    else if (substr($line, 0, 4) == "date")
    {
      $line = fgets($fp);
      $exp_date = substr(trim($line), 3, -1);
      $ans['exp_date'] = $exp_date;
    }
    // jkg 2006apr18: bug bec ns matched ore than one line on gems, added space after 'ns' to fix 
    else if (substr($line, 0, 3) == "ns ")
    {
      $line = fgets($fp);
      list($dum,$num_slices) = explode(" ",$line);
      $ans['num_slices'] = $num_slices;
    }
    else if (substr($line, 0, 6) == "images")
    {
      $line = fgets($fp);
      list($dum,$num_images) = explode(" ",$line);
      $ans['num_images'] = $num_images;
    }
    else if (substr($line, 0, 8) == "time_run")
    {
      $line = fgets($fp);
      list($dum,$time_run) = explode(" ",$line);
      $time_run = ereg_replace("\"", "", $time_run);
      $ans['time_run'] = $time_run;
    }
    else if (substr($line, 0, 7) == "comment")
    {
      $line = fgets($fp);
      $comment = substr($line, 2, -1);
      $comment = ereg_replace("\"", "", $comment);
      $ans['comment'] = $comment;
    }
    else if (substr($line, 0, 9) == "operator_")
    {
      $line = fgets($fp);
      list($dum,$operator) = explode(" ",$line);
      $operator = ereg_replace("\"", "", $operator);
      $operator = ereg_replace("\n", "", $operator);
      $ans['operator'] = $operator;
    }
    else if (substr($line, 0, 8) == "studyid_") 
    {
      $line = fgets($fp);
      list($dum,$studyid) = explode(" ",$line);
      $studyid = ereg_replace("\"", "", $studyid);
      $studyid = ereg_replace("\n", "", $studyid);
      $ans['studyid'] = $studyid;
    }
    else if (substr($line, 0, 7) == "epi_pc ")
    {
      $line = fgets($fp);
      list($dum,$epi_pc) = explode(" ",$line);
      $epi_pc = str_replace("\"", "", $epi_pc);
      $epi_pc = str_replace("\n", "", $epi_pc);
      $ans['epi_pc'] = $epi_pc;
    }
    else if (substr($line, 0, 3) == "tr ")
    {
      $line = fgets($fp);
      list($dum,$tr) = explode(" ", $line);
      $ans['tr'] = $tr;
    }
    else if (substr($line, 0, 3) == "te ")
    {
      $line = fgets($fp);
      list($dum,$te) = explode(" ", $line);
      $ans['te'] = $te;      
    }
    else if (substr($line, 0, 7) == "orient ")
    {
      $line = fgets($fp);
      list($dum,$orient) = explode(" ", $line);
      $orient = ereg_replace("\"", "", $orient);
      $orient = ereg_replace("\n", "", $orient);
      if ($orient == "trans") $orient = "transverse";
      else if ($orient == "cor") $orient = "coronal";
      $ans['orient'] = $orient;
    }
    else if (substr($line, 0, 4) == "thk ")
    {
      $line = fgets($fp);
      list($dum,$thk) = explode(" ", $line);
      $ans['thk'] = $thk;
    }
    else if (substr($line, 0, 5) == "pss0 ")
    {
      $line = fgets($fp);
      list($dum,$pss0) = explode(" ", $line);
      $ans['pss0'] = $pss0;
    }
    else if (substr($line, 0, 4) == "gap ")
    {
      $line = fgets($fp);
      list($dum,$gap) = explode(" ", $line);
      $ans['gap'] = $gap;
    }
    else if (substr($line, 0, 3) == "np ")
    {
      $line = fgets($fp);
      list($dum,$np) = explode(" ", $line);
      $ans['np'] = $np / 2;
    }    
    else if (substr($line, 0, 3) == "nv ")
    {
      $line = fgets($fp);
      list($dum,$nv) = explode(" ", $line);
      $ans['nv'] = $nv;
    }
    else if (substr($line, 0, 4) == "lro ")
    {
      $line = fgets($fp);
      list($dum,$lro) = explode(" ", $line);
      $ans['lro'] = $lro;
    }    
    else if (substr($line, 0, 4) == "lpe ")
    {
      $line = fgets($fp);
      list($dum,$lpe) = explode(" ", $line);
      $ans['lpe'] = $lpe;
    }    
    else if (substr($line, 0, 9) == "ms_intlv ")
    {
      $line = fgets($fp);
      list($dum,$ms_intlv) = explode(" ", $line);
      $ms_intlv = ereg_replace("\"", "", $ms_intlv);
      $ms_intlv = ereg_replace("\n", "", $ms_intlv);
      $ans['ms_intlv'] = $ms_intlv;
    }    
    else if (substr($line, 0, 5) == "gain ")
    {
      $line = fgets($fp);
      list($dum,$gain) = explode(" ", $line);
      $ans['gain'] = $gain;
    }    
    else if (substr($line, 0, 6) == "tpwr1 ")
    {
      $line = fgets($fp);
      list($dum,$tpwr1) = explode(" ", $line);
      $ans['tpwr1'] = $tpwr1;
    }    
    else if (substr($line, 0, 7) == "rfcoil ")
    {
      $line = fgets($fp);
      list($dum,$rfcoil) = explode(" ", $line);
      $rfcoil = ereg_replace("\"", "", $rfcoil);
      $rfcoil = ereg_replace("\n", "", $rfcoil);
      $ans['rfcoil'] = $rfcoil;
    }    
    else if (substr($line, 0, 7) == "fatsat ")
    {
      $line = fgets($fp);
      list($dum,$fatsat) = explode(" ", $line);
      $fatsat = ereg_replace("\"", "", $fatsat);
      $fatsat = ereg_replace("\n", "", $fatsat);
      $ans['fatsat'] = $fatsat;
    }    
    else if (substr($line, 0, 9) == "ky_order ")
    {
      $line = fgets($fp);
      list($dum,$ky_order) = explode(" ", $line);
      $ky_order = ereg_replace("\"", "", $ky_order);
      $ky_order = ereg_replace("\n", "", $ky_order);
      $ans['ky_order'] = $ky_order;
    }    
    else if (substr($line, 0, 9) == "fract_ky ")
    {
      $line = fgets($fp);
      list($dum,$fract_ky) = explode(" ", $line);
      $ans['fract_ky'] = $fract_ky;
    }    
    else if (substr($line, 0, 9) == "actionid ")
    {
      $line = fgets($fp);
      list($dum,$actionid) = explode(" ", $line);
      $actionid = ereg_replace("\"", "", $actionid);
      $actionid = ereg_replace("\n", "", $actionid);
      $ans['actionid'] = $actionid;
    }
  }
  return $ans;
}
?>