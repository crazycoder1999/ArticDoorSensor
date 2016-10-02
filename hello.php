<?php
  session_start();
?>
<html lang="en">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <link href="https://netdna.bootstrapcdn.com/bootstrap/3.0.1/css/bootstrap.min.css" rel="stylesheet">
    <title>Door Sensor</title>
    <script src="https://code.jquery.com/jquery-1.10.2.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.15.1/moment.min.js"></script>

</head>
<body>
   <h1>Door Sensor and ARTIK Cloud</h1>
   <p>Your login succeeded. Got your session token</p>
 <p> Last Update From the Sensor <span id="lastEvent">...</span></p>
   <p> Status <span id="lastStatus">...</span></p>

  <?php
  if (isset($_REQUEST['access_token'])) {
    $_SESSION['access_token'] = $_REQUEST['access_token'];
  }

  require('ArtikCloudProxy.php');
  $proxy = new ArtikCloudProxy();
  $proxy->setAccessToken($_SESSION['access_token']);
  ?> 

	<a class="btn getMessage">[Get last status]</a>

	<script>
	// Get a message using PHP
    $('.getMessage').click(function(ev){
        $.ajax({url:'get-message.php', dataType: "json", success:function(result){
    	    console.log(result);
			// Put code to validate result
			// Get the result and show it
			var message = result.data[0];
			var time = moment(message.ts).format("h:mm:ss DD-MM-YYYY");

			document.getElementById("lastEvent").innerHTML = time;
			if( message.data.open > message.data.close ) {
				document.getElementById("lastStatus").innerHTML = "The Door is OPEN!";
			} else {
				document.getElementById("lastStatus").innerHTML = "Door is closed, all OK.";
			}

            }
        });
    });
    </script>
	
</body>
</html>
