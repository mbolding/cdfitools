<html>
<body style="background:#FFFFFF">
<div style="background:#222222; font-family:Veranda,Arial,Helvetica,sans-serif">
<table cellspacing=1 cellpadding=2 border=0 width='100%'>
<tr style="background:#FFFFFF">
<th width=14%>Sunday</th>
<th width=14%>Monday</th>
<th width=14%>Tuesday</th>
<th width=14%>Wednesday</th>
<th width=14%>Thursday</th>
<th width=14%>Friday</th>
<th width=14%>Saturday</th>
</tr>
<?
  $monthnames = array("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec");
  $dpm = array(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
  $month = 7;
  $day = 16;
  $wks = 24;
  $cols = array("#E4FFFF","#FFFFFF");
  $cols2 = array("#C4EEEE","#EEEEEE");
  $c = 0;
  for ($i=0; $i<$wks; $i++)
    {
    echo "<tr align=right valign=top>";
    for ($d=0; $d<7; $d++)
      {
      if ($d>0 && $d<6)
        echo "<td style='border:1 solid #000000; background:".$cols[$c]."'>";
      else
        echo "<td style='border:1 solid #000000; background:".$cols2[$c]."'>";
      if ($day == 1)
        echo "<span style='font-size:14; font-weight:bold'>".$monthnames[$month-1]."</span>&nbsp;&nbsp;";
      echo "<span style='font-size:12;'>".$day."</span>";
      echo "<br><br></td>";
      $day++;
      if ($day > $dpm[$month-1])
      {
        $day = 1;
        $month++;
        $c = 1 - $c;
      }
    }
    echo "</tr>\n";
  }
?>
</table>
</div>
</body>
</html>
