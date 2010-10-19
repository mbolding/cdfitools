<?
exit();
//
//  for each row in table, reopen procpar and get the operator and studyid fields to put 
//  into database studyid field
//
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $q = mysql_query("SELECT * FROM fid_files WHERE deleted='N' ORDER BY when_uploaded DESC");
  if (!$q)
    echo "query error: ".mysql_error();
  else
  {
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      $procparfile = "/usr/local/apache/htdocs/mri_analysis/fid/".$row["id"]."/procpar";
      $fp = fopen($procparfile, "r");
      while (!feof($fp))
      {
        $line = fgets($fp);
        if (substr($line, 0, 9) == "operator_")
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
      $sql = "UPDATE fid_files SET studyid=\"".$studyid."/".$operator."\" where id=".$row["id"];
      mysql_query($sql);
    }
  }

?>