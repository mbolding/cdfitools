<html>
<head>
<title>Varian FID Reconstruction</title>
<link rel="stylesheet" href="styles.css" type="text/css">
</head>
<body bgcolor=#FFFFFF onload="javascript:on_load();">
<?
   //
   //   Header:
   //
?>
<table border=0 cellpadding=3 cellspacing=0 width=100%>
<tr style="backgroundColor=#FFFFFF"><td>
<a href="http://tesla.cdfi.uab.edu:8200"><img border=0 src="logo.gif"></a>
</td><td align=right><div class=banner>Varian FID Reconstruction</div></td>
</tr></table>
<br>
<script language="javascript">
  function clear_results()
  {
    if (confirm("Are you sure you want to clear the results?"))
    {
      document.clearform.submit();
    }
  }
<?
  $action = $_GET["action"];
  $id = $_GET["id"];

  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  if ($action == "clear")
  {
    mysql_query("UPDATE fid_files SET step_1=0,step_2=0,step_3=0,step_4=0,cmd1=\"\",cmd2=\"\",cmd3=\"\",cmd4=\"\",result1=\"\",result2=\"\",result3=\"\",result4=\"\",result_line_1=\"\",result_line_2=\"\",result_line_3=\"\",result_line_4=\"\" WHERE id=$id;");
  }

  $query = "SELECT * FROM fid_files WHERE id=$id";
  $q = mysql_query($query);
  if (!$q) {
     echo "query error: ".mysql_error();
     exit;
  }
  $row = mysql_fetch_array($q, MYSQL_ASSOC);

?>
</script>
<center>
<div class=nav>
<a href="index_fids.php?studyid=<? echo $row["studyid"]; ?>">View <? echo $row["studyid"]; ?></a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
<a href="index.php">View Studies</a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;<a href="upload_fid.php">Upload Fid</a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;<a href="fid/<?echo$id;?>/">View Files</a>
</div><br>

<?
  //<a href="javascript:clear_results();">clear</a>

  function statusString($arg)
  {
    if ($arg == 0)
      return "-";
    else if ($arg == 1)
      return "<font color='green'>done</font>";
    else if ($arg == 2)
      return "<font color='red'>error</font>";
    else
      return "???";
  }
?>
<br><br>
  <table cellpadding=1 cellspacing=0 border=0 width=85%>
  <tr><td bgcolor=#0000FF>
  <table cellpadding=3 cellspacing=1 border=0 bgcolor=#FFFFEE width=100%>
    <tr>
        <td width=25%>&nbsp;&nbsp;&nbsp;&nbsp;<b>processing step</b></td>
        <td align=center width=15%><b>status</b></td>
        <td align=center>&nbsp;&nbsp;<b>message</b>&nbsp;&nbsp;</td>
    </tr>
    <tr><td>&nbsp;1) fid -> raw</td><td align=center id=1><? echo statusString($row["step_1"]); ?></td>
        <td id=11>&nbsp;<? echo $row["result_line_1"]; ?></td></tr>
    <tr><td>&nbsp;2) to3d</td><td align=center id=2><? echo statusString($row["step_2"]); ?></td>
        <td id=12>&nbsp;<? echo $row["result_line_2"]; ?></td></tr>
    <tr><td>&nbsp;3) 3dAFNItoANALYZE</td><td align=center id=3><? echo statusString($row["step_3"]); ?></td>
        <td id=13>&nbsp;<? echo $row["result_line_3"]; ?></td></tr>
    <tr><td>&nbsp;4) fsl</td><td align=center id=4><? echo statusString($row["step_4"]); ?></td>
        <td id=14>&nbsp;<? echo $row["result_line_4"]; ?></td></tr>
    <tr><td>&nbsp;</td>
<? // SPAN tags are not surrounded by new paragraphs like DIV tags are: also SPAN fits tight around text, while
   // the DIV tag fills up the whole TD ?>
      <td align=center><span class="processbutton"><a href="javascript:run_all();">&nbsp;&nbsp;&nbsp;run all&nbsp;&nbsp;&nbsp;</a></span></td>
    </tr>
  </table>
  </td></tr></table>

<?
  $nextstep = 1;
  if ($row["step_1"] == 1) $nextstep = 2;
  if ($row["step_2"] == 1) $nextstep = 3;
  if ($row["step_3"] == 1) $nextstep = 4;
  if ($row["step_4"] == 1) $nextstep = 1; 

  $jpg_file = "fid/" . $id; // . "/Image1.pgm";
  if ($row["step_1"] == 1) 
    $jpg_file .= "/Image1.jpg";
  else
    $jpg_file = "monkey.gif";
  echo "<br><br><img id='mri_image' src=\"" . $jpg_file . "\">";

  $next_image = $row["images_processed"];
  if ($next_image >= $row["num_images"])
    $next_image = -1;
?>

<br><br>
<table cellpadding=1 cellspacing=1 border=0 width=90%>
<span class="cmd">
  <tr><td id=101>&nbsp;<? echo $row["cmd1"]; ?></td></tr>
  <tr><td style="color:#444488;"><div id=1001>&nbsp;<? echo $row["result1"]; ?></div></td></tr>
  <tr><td id=102>&nbsp;<? echo $row["cmd2"]; ?></td></tr>
  <tr><td style="color:#444488" id=1002>&nbsp;<? echo $row["result2"]; ?></td></tr>
  <tr><td id=103>&nbsp;<? echo $row["cmd3"]; ?></td></tr>
  <tr><td style="color:#444488" id=1003>&nbsp;<? echo $row["result3"]; ?></td></tr>
  <tr><td id=104>&nbsp;<? echo $row["cmd4"]; ?></td></tr>
  <tr><td style="color:#444488" id=1004>&nbsp;<? echo $row["result4"]; ?></td></tr>
</span>
</table>

<script language="javascript">

  var req = 0;  /* the XMLHttpRequest object */

  function on_load()
  {
<?
  //  if ($next_image > 0) {
  //    echo "    start_trans();\n";
  //  } else {
  //    echo "    /* BLANK: nothing left to process, or processing halted */\n";
  //  }
?>
  }

  var nextstep = <? echo $nextstep; ?>;
  var isrunning = 0;

  function run_all()
  {
    if (isrunning) { return; }
    isrunning = 1;
    var thisstep = document.getElementById(nextstep);
    thisstep.firstChild.nodeValue = "processing...";
    thisstep.style.color = "black";
    start_trans();
  }
  function process_req()
  {
//    try {
    if (req.readyState == 4) 
    {
      if (req.status == 200) 
      {
        isrunning = 0;
        var resultfield = req.responseXML.getElementsByTagName("result");
        var result = resultfield[0].firstChild.nodeValue;
        var stepfield = req.responseXML.getElementsByTagName("step");
        var step = stepfield[0].firstChild.nodeValue;
        var outputfield = req.responseXML.getElementsByTagName("poutput");
        var output_resp = outputfield[0].firstChild.nodeValue;
        var cmdfield = req.responseXML.getElementsByTagName("command");
        var cmd_resp = cmdfield[0].firstChild.nodeValue;
        var outputlinefield = req.responseXML.getElementsByTagName("outputline");
        var alloutput = "";
        for (var i=0; i<outputlinefield.length; i++) {
          alloutput += outputlinefield[i].firstChild.nodeValue;
          alloutput += "<br>";
        }
        alloutput += "<br>";

        var thisstep = document.getElementById(nextstep);
        var res_msg = document.getElementById(nextstep+10);  // can't use step+10 bec step is a string
        res_msg.innerHTML /*firstChild.nodeValue*/ = output_resp;

          var resfield = document.getElementById(nextstep+100);
          resfield.firstChild.nodeValue = cmd_resp;
          resfield = document.getElementById(nextstep+1000);
          resfield.innerHTML = "";          
          resfield.innerHTML = alloutput;          

        if (result == "1") 
        {
          thisstep.firstChild.nodeValue = "done";
          thisstep.style.color = "green";
          if (nextstep == 1)
          {
            var new_img = "fid/<? echo $id; ?>/Image1.jpg";
            document.getElementById('mri_image').src = new_img;
          }

          nextstep = nextstep + 1;  // PROBLEM: can's set nextstep=step+1 because step is a string
          run_all();
	}
        else 
        {
          thisstep.firstChild.nodeValue = "error";
          thisstep.style.color = "red";
	}
        //alert("result = " + result[0].firstChild.nodeValue + "\nstep = " + step[0].firstChild.nodeValue);
      }
    }
//} catch(e) {
//      alert("Server error:\n" + req.responseText);
//    }
  }
  function start_trans()
  {
    if (req) { delete req; }
    req = new XMLHttpRequest();
    req.onreadystatechange = process_req;
    req.open("GET", "xml_request.php?step="+nextstep+"&id="+<? echo $id; ?>, true);
    req.send(null);
  }
</script>

<? /* <a href="javascript:start_trans();">Start</a> */ ?>
<form name="clearform" action="edit_fid.php">
  <input type="hidden" name="action" value="clear">
  <input type="hidden" name="id" value="<? echo $id; ?>">
</form>
</body>
</html>