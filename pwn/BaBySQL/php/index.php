<!DOCTYPE html>
<html>
<title>Boogy's Blog</title>
    <body>
        <div>
<?php
$servername = "127.0.0.1";
$username = "root";
$password = "password";
$dbname = "mydb";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$id = @$_REQUEST['id'];
$sql = "SELECT id,firstname,lastname,email FROM MyGuests where id=". $id;
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    echo "<table><tr><th>ID</th><th>Name</th></tr>";
    while($row = $result->fetch_assoc()) {
        echo "<tr><td>".$row["id"]."</td><td>".$row["firstname"]." ".$row["lastname"]." ".$row['email']."</td></tr>";
    }
    echo "</table>";
} else {
    echo "<img src=./nope.gif>";
}
$conn->close();
?>
        </div>
    </body>
</html>
