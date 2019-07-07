<?
$username="ce_webadmin";
$password="Armadillo2012";
$servername="mysqlv111";
$database="ce_webforum";

$user_name=$_POST['user_name'];
$user_email=$_POST['user_email'];
$user_twitter=$_POST['use_twitter'];
$user_facebook=$_POST['user_facebook'];
$user_web=$_POST['user_web'];
$user_message=$_POST['user_message'];

//mysql_connect($servername,$username,$password);
$conn = mysql_connect('205.178.146.112:3306',$username,$password);

// Check if the connection failed.
if (!$conn) {
    die('Could not connect: ' . mysql_error());
}

@mysql_select_db($database) or die( "Unable to select database");

// Add customer data to the table.
$query = "INSERT INTO ce_marketing VALUES ('', '$user_name', '$user_email', '', '$user_twitter', '$user_facebook', '$user_web', '$user_message')";


if(mysql_query($query)) 
{
    // Make mail.
    $to = $user_email;
    $from = "support@carexplorer.org";
    $subject = "Hello! This is the download link to Car Explorer Pack";
   
    $message  = "<html><body>";
    $message .= "<p>Hello,<br><br>This is the link to download <a href='http://www.carexplorer.org/Downloads/CE/Collections/AllCARFiles_2012_2008.zip'>CAR files</a> (2012-2008)</p>";
    $message .= "</body></html>";
    
    $headers  = "From: $from\r\n";
    $headers .= "Content-type: text/html\r\n";
    
    // Send mail. 
    if (mail($to, $subject, $message, $headers)) 
    {
        echo("<p>Message has been successfully sent. Please check your email!</p>");
    } 
    else 
    {
        echo("<p>Message delivery failed...</p>");
    }
} 
else
{
    echo 'Insertion failure: <br><br>';
    echo ''.mysql_errno($conn).'<br><br>';
    echo ''.mysql_error($conn).'';	
    // Insert failed, set error message.
    $errors[] = 'Error adding user to database, MySQL said:<br>
        ('.mysql_errno($conn).') '.mysql_error($conn).'</span>';
}

mysql_close();
?>