<!doctype html>
<?php
session_start();
@include "baroload.php";
?>
<html>
<head>
<title>Barometer Config</title>
<meta name="viewport" content="width=device-width, initial-scale=1" />
<link rel="stylesheet" href="../jquery/jquery.mobile-1.4.3.min.css" />
<script src="../jquery/jquery-1.11.1.min.js"></script>
<script src="../jquery/jquery.mobile-1.4.3.min.js"></script>
</script>
</head>
<body>

<div data-role="page" id="wsdebug">
<div data-role="header">
<a href="../index.php" data-ajax="false" data-rel="back" data-transition="slide" class="ui-btn ui-corner-all ui-btn-inline">Go Back</a>
<h1>Barometer config</h1>
</div>

<form data-ajax="false" method="post" action="barosave.php">
<div role="main" class="ui-content">
<div>
Model:<br/> 
0: bmp085/bmp180</br>
1: ms5611<br/>
<br/>
Port: hex port (i.e. 0x77)
</div>

<div class="ui-field-contain">
<label for="c0_0">Baro model</label>
<input type="number" name="c0_0" id="c0_0" value="<?php echo $c[0][0];?>"/>
<label for="c0_1">Baro port</label>
<input type="text" name="c0_1" id="c0_1" value="<?php echo $c[0][1];?>"/>

<input type="submit" value="Save"/>

<div data-role="collapsible" data-collapsed="true">
<h3>Config view</h3>
<pre>
<?php
readfile($config_path."baro.config");
?>
</pre>
</div>
</div>
</form>
</div>
</body>
</html>

